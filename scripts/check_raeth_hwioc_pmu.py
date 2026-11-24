#!/usr/bin/env python3
"""Check fail-closed bounds and MMIO handling in the RAETH HW-IOC proc API."""

from pathlib import Path

SOURCE = Path("trunk/linux-4.4.x/drivers/net/ethernet/raeth/ra_dbg_hwioc.c")
WORKFLOW = Path(".github/workflows/switch-cli-stability.yml")
POLICY = Path("trunk/linux-4.4.x/drivers/net/ethernet/raeth/ra_dbg_hwioc_policy.h")
TEST = Path("trunk/linux-4.4.x/drivers/net/ethernet/raeth/tests/ra_dbg_hwioc_policy.c")


def body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    for position in range(opening, len(source)):
        if source[position] == "{":
            depth += 1
        elif source[position] == "}":
            depth -= 1
            if depth == 0:
                return source[opening + 1:position]
    raise AssertionError("unterminated function: " + signature)


def main() -> None:
    source = SOURCE.read_text(encoding="utf-8")
    workflow = WORKFLOW.read_text(encoding="utf-8")
    policy = POLICY.read_text(encoding="utf-8")
    test = TEST.read_text(encoding="utf-8")

    assert '#include "ra_dbg_hwioc_policy.h"' in source

    counter = body(source, "int ioc_pmu_cnt_config(")
    assert counter.index("raeth_hwioc_pmu_counter_args_valid(") < counter.index("ioremap(")
    assert counter.index("if (!reg_pmu_cnt)") < counter.index("sys_reg_write(")
    assert "iounmap(reg_pmu_cnt);" in counter

    control = body(source, "int ioc_pmu_ctl_config(")
    assert control.index("raeth_hwioc_pmu_enable_valid(") < control.index("ioremap(")
    assert control.index("if (!reg_pmu_ctl)") < control.index("sys_reg_write(")
    assert "iounmap(reg_pmu_ctl);" in control

    parser = body(source, "static int ioc_next_hex_arg(")
    assert 'strsep(cursor, " \\t\\r\\n")' in parser
    assert "kstrtol(token, 16, value)" in parser
    assert "return ret;" in parser

    writer = body(source, "ssize_t ioc_pmu_write(")
    assert "count == 0" in writer
    assert "count >= sizeof(buf)" in writer
    assert "raeth_hwioc_pmu_operation_valid(args[0])" in writer
    assert writer.index("raeth_hwioc_pmu_operation_valid(args[0])") < writer.index("iocoherent_set_func[args[0]]")
    assert "raeth_hwioc_pmu_enable_valid(args[1])" in writer
    assert "raeth_hwioc_pmu_counter_args_valid(args[1], args[2], args[3])" in writer
    assert "ioc_no_more_hex_args(&cursor)" in writer
    assert "ret = iocoherent_set_func[args[0]]" in writer
    assert "if (ret != 0)" in writer
    assert "iocoherent_set_func[arg0]" not in writer

    reader = body(source, "int ioc_pmu_read(")
    mappings = (
        "reg_virt_0 = ioremap(MCSI_A_PMU_EVN_SEL0, 0x10);",
        "reg_virt_1 = ioremap(MCSI_A_PMU_EVN_SEL1, 0x10);",
        "reg_virt_2 = ioremap(MCSI_A_PMU_EVN_SEL2, 0x10);",
        "reg_virt_3 = ioremap(MCSI_A_PMU_EVN_SEL3, 0x10);",
    )
    assert all(reader.count(item) == 1 for item in mappings)
    assert reader.count("if (!reg_virt_") == 4
    assert max(reader.index(item) for item in mappings) < reader.index("seq_printf(")
    assert reader.count("iounmap(reg_virt_0);") == 4
    assert reader.count("iounmap(reg_virt_1);") == 3
    assert reader.count("iounmap(reg_virt_2);") == 2
    assert reader.count("iounmap(reg_virt_3);") == 1

    assert "RAETH_HWIOC_PMU_OPERATION_COUNT 3" in policy
    assert "RAETH_HWIOC_PMU_COUNTER_COUNT 4" in policy
    assert "RAETH_HWIOC_PMU_INTERFACE_COUNT 9" in policy
    assert "assert(!raeth_hwioc_pmu_operation_valid(-1));" in test
    assert "assert(!raeth_hwioc_pmu_counter_args_valid(4, 0, 0));" in test
    assert "assert(!raeth_hwioc_pmu_counter_args_valid(0, 0, 0x1f));" in test

    push, pr = workflow.split("  pull_request:", 1)
    pr = pr.split("  workflow_dispatch:", 1)[0]
    required_paths = (
        "trunk/linux-4.4.x/drivers/net/ethernet/raeth/ra_dbg_hwioc.c",
        "trunk/linux-4.4.x/drivers/net/ethernet/raeth/ra_dbg_hwioc_policy.h",
        "trunk/linux-4.4.x/drivers/net/ethernet/raeth/tests/ra_dbg_hwioc_policy.c",
        "scripts/check_raeth_hwioc_pmu.py",
    )
    for path in required_paths:
        assert f'      - "{path}"' in push, f"missing push path: {path}"
        assert f'      - "{path}"' in pr, f"missing pull_request path: {path}"
    assert "Validate RAETH HW-IOC PMU proc bounds and MMIO failures" in workflow
    assert "python3 scripts/check_raeth_hwioc_pmu.py" in workflow

    print("RAETH HW-IOC PMU bounds, MMIO guards, partial-map cleanup and status propagation: PASS")


if __name__ == "__main__":
    main()
