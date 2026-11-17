#!/usr/bin/env python3
"""Ensure XY-C1 builds every optional USB modem driver requested by RC."""

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
CONFIG = (ROOT / "trunk/configs/boards/XY-C1/kernel-4.4.x.config").read_text()
KCONFIG = (ROOT / "trunk/linux-4.4.x/drivers/usb/serial/Kconfig").read_text()
SERIAL_MAKEFILE = (ROOT / "trunk/linux-4.4.x/drivers/usb/serial/Makefile").read_text()
MODEM_SOURCE = (ROOT / "trunk/user/rc/src/usb_modem.c").read_text()
MODULE_POLICY = (ROOT / "trunk/user/rc/src/modem_module_reload_policy.h").read_text()
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


def string_array(source: str, array_name: str) -> list[str]:
    match = re.search(
        rf"static\s+const\s+char\s+\*const\s+{re.escape(array_name)}"
        rf"\s*\[\]\s*=\s*\{{(.*?)\}};",
        source,
        flags=re.DOTALL,
    )
    assert match, f"missing RC modem driver array: {array_name}"
    return re.findall(r'"([^"]+)"', match.group(1))


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


reload_body = function_body(
    MODEM_SOURCE, "reload_modem_modules(int modem_type, int reload)"
)
group_loader = function_body(
    MODEM_SOURCE, "load_modem_module_group(const char *group_name,"
)

ndis_config = {
    "cdc_ether": "CONFIG_USB_NET_CDCETHER",
    "rndis_host": "CONFIG_USB_NET_RNDIS_HOST",
    "qmi_wwan": "CONFIG_USB_NET_QMI_WWAN",
    "cdc_ncm": "CONFIG_USB_NET_CDC_NCM",
    "huawei_cdc_ncm": "CONFIG_USB_NET_HUAWEI_CDC_NCM",
    "cdc_mbim": "CONFIG_USB_NET_CDC_MBIM",
    "sierra_net": "CONFIG_USB_SIERRA_NET",
}
ras_config = {
    "cdc_acm": "CONFIG_USB_ACM",
    "qcserial": "CONFIG_USB_SERIAL_QUALCOMM",
    "sierra": "CONFIG_USB_SERIAL_SIERRAWIRELESS",
    "option": "CONFIG_USB_SERIAL_OPTION",
}

# Driver names are held in alternative-family arrays and passed to a verified
# generic loader; they are not direct module_smart_load() call literals here.
assert string_array(reload_body, "ndis_modules") == list(ndis_config), (
    "NDIS candidate array no longer matches the supported USB driver family"
)
assert string_array(reload_body, "ras_modules") == list(ras_config), (
    "RAS candidate array no longer matches the supported serial driver family"
)
assert string_array(reload_body, "serial_aux_modules") == [
    "qcserial", "sierra", "option"
], "NDIS supplemental AT/control driver candidates changed unexpectedly"

params_match = re.search(
    r"static\s+const\s+char\s+\*const\s+ndis_params\s*\[\]\s*=\s*\{(.*?)\};",
    reload_body,
    flags=re.DOTALL,
)
assert params_match, "missing NDIS per-driver parameter array"
params = re.findall(r'NULL|"([^"]*)"', params_match.group(1))
assert params == ["", "", "", "prefer_mbim=0", "", "", ""], (
    f"unexpected NDIS module parameters: {params}"
)

compact_reload = re.sub(r"\s+", " ", reload_body)
assert re.search(
    r'if\s*\(modem_type\s*==\s*3\).*?'
    r'load_modem_module_group\("NDIS",\s*ndis_modules,\s*ndis_params,',
    compact_reload,
), "NDIS driver family is no longer selected for modem_type 3"
assert re.search(
    r'else\s*\{.*?load_modem_module_group\("RAS",\s*ras_modules,\s*NULL,',
    compact_reload,
), "RAS driver family is no longer selected for non-NDIS modem types"

# The generic loader attempts every optional candidate and probes observable
# /sys/module state; the legacy modprobe return value is not treated as proof.
assert "module_smart_load" in group_loader
assert "is_module_loaded_status" in group_loader
assert "is_module_loaded(module_name)" not in group_loader
assert "rc_modem_module_load_group_verified(module_names, module_params" in group_loader
load_index = MODULE_POLICY.index("(void)ops->load(module_names[i]")
probe_index = MODULE_POLICY.index("probe_status = ops->is_loaded(module_names[i])")
assert load_index < probe_index
assert "if (probe_status == 1)" in MODULE_POLICY
assert "probe_status < 0" in MODULE_POLICY
assert "probe_status > 1" in MODULE_POLICY
assert "first_probe_error" in MODULE_POLICY
assert "return first_probe_error;" in MODULE_POLICY
assert "return ENODEV;" in MODULE_POLICY

for symbol in (
    "CONFIG_USB_USBNET",
    "CONFIG_USB_SERIAL",
    "CONFIG_USB_SERIAL_WWAN",
    *ndis_config.values(),
    *ras_config.values(),
):
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
    f"({len(ndis_config)} NDIS + {len(ras_config)} RAS optional modules mapped "
    "to loadable kernel modules)"
)
