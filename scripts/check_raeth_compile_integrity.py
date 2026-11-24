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
        r"(?m)^[ \t]*(?:static[ \t]+)?(?:void|int)[ \t]+" + re.escape(name) + r"\s*\([^;]*\)\s*\{, source
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


chip_name = function_body(source, "static int fe_chip_name_config(")
arch_setting = function_body(source, "void raeth_arch_setting(")
probe = function_body(source, "static int rather_probe(")
require(chip_name.index("if (ret)") < chip_name.index("strcasecmp(pm") and
        "return ret;" in chip_name and "return -EINVAL;" in chip_name,
        "chip-name parsing must reject failed or unsupported compatible data before comparing it")
require(all("!ret" in line.split("strcasecmp(pm,", 1)[0]
            for line in arch_setting.splitlines() if "strcasecmp(pm," in line),
        "every device-tree selector comparison must be dominated by its latest read status")

require("ret = fe_chip_name_config(ei_local, pdev);" in probe and
        "if (ret)\n\t\tgoto err_free_dev;" in probe and
        "if (dev_raether == netdev)\n\t\tdev_raether = NULL;" in probe,
        "probe must propagate chip-name parse failures and clear its unpublished global device pointer")

print("RAETH function uniqueness and GMII status declaration contract: PASS")
