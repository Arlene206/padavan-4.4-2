#!/usr/bin/env python3
"""Ensure XY-C1 builds every USB modem driver requested by the RC loader."""

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
CONFIG = (ROOT / "trunk/configs/boards/XY-C1/kernel-4.4.x.config").read_text()
KCONFIG = (ROOT / "trunk/linux-4.4.x/drivers/usb/serial/Kconfig").read_text()
SERIAL_MAKEFILE = (ROOT / "trunk/linux-4.4.x/drivers/usb/serial/Makefile").read_text()
MODEM_SOURCE = (ROOT / "trunk/user/rc/src/usb_modem.c").read_text()
OPTION_SOURCE = ROOT / "trunk/linux-4.4.x/drivers/usb/serial/option.c"


def function_body(source: str, signature: str) -> str:
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
    raise AssertionError(f"unterminated function: {signature}")


def require_module(symbol: str) -> None:
    assignments = re.findall(
        rf"^{re.escape(symbol)}=(\S+)$", CONFIG, flags=re.MULTILINE
    )
    disabled = re.findall(
        rf"^#\s*{re.escape(symbol)}\s+is not set\s*$",
        CONFIG,
        flags=re.MULTILINE,
    )
    assert assignments == ["m"] and not disabled, (
        f"{symbol} must be built as a loadable module; found "
        f"assignments={assignments}, disabled={bool(disabled)}"
    )


loader = function_body(
    MODEM_SOURCE, "reload_modem_modules(int modem_type, int reload)"
)
module_config = {
    "cdc_ether": "CONFIG_USB_NET_CDCETHER",
    "rndis_host": "CONFIG_USB_NET_RNDIS_HOST",
    "qmi_wwan": "CONFIG_USB_NET_QMI_WWAN",
    "cdc_ncm": "CONFIG_USB_NET_CDC_NCM",
    "huawei_cdc_ncm": "CONFIG_USB_NET_HUAWEI_CDC_NCM",
    "cdc_mbim": "CONFIG_USB_NET_CDC_MBIM",
    "sierra_net": "CONFIG_USB_SIERRA_NET",
    "cdc_acm": "CONFIG_USB_ACM",
    "qcserial": "CONFIG_USB_SERIAL_QUALCOMM",
    "sierra": "CONFIG_USB_SERIAL_SIERRAWIRELESS",
    "option": "CONFIG_USB_SERIAL_OPTION",
}

require_module("CONFIG_USB_USBNET")
require_module("CONFIG_USB_SERIAL")
require_module("CONFIG_USB_SERIAL_WWAN")

for module_name, symbol in module_config.items():
    assert f'module_smart_load("{module_name}",' in loader, (
        f"RC modem reload no longer requests expected module {module_name}"
    )
    require_module(symbol)

option_start = KCONFIG.index("config USB_SERIAL_OPTION\n")
option_end = KCONFIG.find("\nconfig ", option_start + 1)
option_kconfig = KCONFIG[option_start:option_end if option_end >= 0 else None]
assert re.search(r"^\s*select USB_SERIAL_WWAN\s*$", option_kconfig, re.MULTILINE)
assert re.search(
    r"^obj-\$\(CONFIG_USB_SERIAL_OPTION\)\s*\+=\s*option\.o\s*$",
    SERIAL_MAKEFILE,
    re.MULTILINE,
)
assert OPTION_SOURCE.is_file(), "USB serial option driver source is missing"

print(
    "XY-C1 USB modem driver configuration: PASS "
    f"({len(module_config)} runtime modules mapped to loadable kernel modules)"
)
