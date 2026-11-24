#!/usr/bin/env python3
"""Guard RAETH compile-time declaration and definition integrity."""
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
RAETH = ROOT / "trunk/linux-4.4.x/drivers/net/ethernet/raeth"
source = (RAETH / "raether.c").read_text()
mac_header = (RAETH / "ra_mac.h").read_text()
ether_header = (RAETH / "raether.h").read_text()
mac_source = (RAETH / "ra_mac.c").read_text()

for name in (
    "fe_chip_name_config",
    "raeth_set_wol",
    "raeth_arch_setting",
    "fe_tx_rx_dec",
):
    definitions = re.findall(
        r"(?m)^[ \t]*void[ \t]+" + re.escape(name) + r"\s*\(", source
    )
    assert len(definitions) == 1, (
        f"{name} must have exactly one definition in raether.c; found {len(definitions)}"
    )

for name in ("set_ge2_gmii", "set_ge0_gmii"):
    declaration = re.compile(
        r"(?m)^[ \t]*int[ \t]+" + re.escape(name) + r"\(void\);$"
    )
    stale = re.compile(
        r"(?m)^[ \t]*void[ \t]+" + re.escape(name) + r"\(void\);$"
    )
    assert declaration.search(mac_header), f"ra_mac.h must declare status-returning {name}"
    assert declaration.search(ether_header), f"raether.h declaration for {name} drifted"
    assert not stale.search(mac_header), f"ra_mac.h must not discard {name} status"
    assert re.search(
        r"(?m)^[ \t]*int[ \t]+" + re.escape(name) + r"\(void\)", mac_source
    ), f"ra_mac.c definition for {name} must return status"

print("RAETH function uniqueness and GMII status declaration contract: PASS")
