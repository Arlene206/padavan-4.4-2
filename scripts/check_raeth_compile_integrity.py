#!/usr/bin/env python3
"""Guard the RAETH source against duplicate or signature-drifted helper definitions."""

from pathlib import Path
import re

SOURCE = Path("trunk/linux-4.4.x/drivers/net/ethernet/raeth/raether.c")
EXPECTED_RETURN_TYPES = {
    "fe_chip_name_config": "int",
    "raeth_set_wol": "void",
    "raeth_arch_setting": "void",
    "fe_tx_rx_dec": "void",
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
    print("RAETH helper definitions are unique and return types match the source contract: PASS")


if __name__ == "__main__":
    main()
