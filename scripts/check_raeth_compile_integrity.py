#!/usr/bin/env python3
"""Guard the RAETH source against duplicate or signature-drifted helper definitions."""

from pathlib import Path
import re

SOURCE = Path("trunk/linux-4.4.x/drivers/net/ethernet/raeth/raether.c")
EXPECTED_RETURN_TYPES = {
    "fe_chip_name_config": "int",
    "raeth_set_wol": "void",
    "raeth_arch_setting": "void",
    "fe_tx_rx_dec": "int",
    "fe_tx_rx_dec_release": "void",
}


def find_definitions(source: str, name: str):
    pattern = re.compile(
        r"(?m)^\s*(?:(?:static|inline)\s+)*"
        r"(void|int|u32|unsigned\s+int|long|bool)\s+"
        + re.escape(name)
        + r"\s*\([^;{}]*\)\s*\{"
    )
    return [match.group(1).replace("  ", " ") for match in pattern.finditer(source)]


def self_test() -> None:
    sample = "void helper(void);\nstatic void helper(void) { }\n"
    assert find_definitions(sample, "helper") == ["void"]
    duplicate = sample + "int helper(int value) { return value; }\n"
    assert len(find_definitions(duplicate, "helper")) == 2
    assert find_definitions("void helper(void);\n", "helper") == []
    assert find_definitions("static int status_helper(void) { return 0; }", "status_helper") == ["int"]


def main() -> None:
    self_test()
    source = SOURCE.read_text(encoding="utf-8")
    for name, expected_type in EXPECTED_RETURN_TYPES.items():
        definitions = find_definitions(source, name)
        assert len(definitions) == 1, (
            f"{name}: expected exactly one function definition, found {len(definitions)}"
        )
        assert definitions[0] == expected_type, (
            f"{name}: expected return type {expected_type}, got {definitions[0]}"
        )
    desc_start = source.index("static int fe_tx_rx_dec(")
    desc_end = source.index("\n\n\nstatic int rather_probe(", desc_start)
    desc = source[desc_start:desc_end]
    for property_name in (
        "gmac1_txq_num", "gmac1_txq_txd_num", "num_rx_desc",
        "gmac2_txq_num", "gmac2_txq_txd_num",
    ):
        read = 'ret = of_property_read_u32(node, "' + property_name + '",'
        read_at = desc.index(read)
        next_read = desc.find("ret = of_property_read_u32(", read_at + len(read))
        block = desc[read_at: next_read if next_read >= 0 else len(desc)]
        assert re.search(r"if \(ret\)\s*\{", block), (
            f"{property_name}: DT read error must be handled before the next read"
        )
    assert 'of_find_property(node, "gmac2_txq_num", NULL)' in desc
    assert 'of_find_property(node, "gmac2_txq_txd_num", NULL)' in desc
    assert "raeth_desc_policy_calculate(" in desc
    allocation_fields = (
        "free_skb", "free_txd_num", "free_txd_head", "free_txd_tail",
        "txd_pool_info", "skb_free", "rls_cnt", "netrx0_skb_data",
    )
    for field in allocation_fields:
        assert f"if (!ei_local->{field})" in desc, (
            f"{field}: allocation failure must be checked"
        )
    assert "if (!ei_local->netrx_skb_data[i])" in desc
    release_start = source.index("static void fe_tx_rx_dec_release(")
    release_end = source.index("\n}\n", release_start)
    release = source[release_start:release_end]
    for field in allocation_fields:
        assert f"kfree(ei_local->{field})" in release
        assert f"ei_local->{field} = NULL;" in release
    assert "for (i = 0; i < MAX_RX_RING_NUM; i++)" in release
    probe_start = source.index("static int rather_probe(")
    probe_end = source.index("\nstatic int raether_remove(", probe_start)
    probe = source[probe_start:probe_end]
    assert "ret = fe_tx_rx_dec(ei_local, pdev);" in probe
    assert "if (ret)\n\t\tgoto err_free_dev;" in probe
    assert "desc_initialized = 1;" in probe
    assert "if (desc_initialized)\n\t\tfe_tx_rx_dec_release(ei_local);" in probe

    print("RAETH helper definitions are unique and return types match the source contract: PASS")


if __name__ == "__main__":
    main()
