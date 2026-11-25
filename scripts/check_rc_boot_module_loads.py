#!/usr/bin/env python3
"""Verify boot module and service launch results reach init_router() health reporting."""

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RC = ROOT / "trunk/user/rc/src"


def body(source: str, signature: str) -> str:
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    for pos in range(brace, len(source)):
        if source[pos] == "{":
            depth += 1
        elif source[pos] == "}":
            depth -= 1
            if depth == 0:
                return source[brace + 1:pos]
    raise AssertionError(f"unterminated function: {signature}")


source = (RC / "rc.c").read_text()
policy = (RC / "wireless_module_load_policy.h").read_text()
common = (RC / "common_ex.c").read_text()

assert "rc_module_load_verified(module_name, module_param, &ops)" in source
assert "loaded = ops->is_loaded(module_name);" in policy
assert "if (loaded < 0)" in policy
assert "return -loaded;" in policy
assert "return loaded == 1 ? 0 : EIO;" in policy
assert '"/sys/module/%s"' in common and "opendir(mod_path)" in common
assert "if (errno == ENOENT)" in common
assert "return is_module_loaded_status(module_name) > 0;" in common

for signature in (
    "load_boot_module(const char *module_name, const char *module_param)",
    "load_wireless_module(const char *module_name)",
):
    loader = " ".join(body(source, signature).split())
    assert "module_smart_load, is_module_loaded_status" in loader, signature
    assert "module_smart_load, is_module_loaded)" not in loader, signature

for signature, calls in (
    ("load_usb_modules(void)", (
        'load_boot_module("usblp", NULL)',
        'load_boot_module("usb_storage", NULL)',
        'load_boot_module("xhci_hcd", xhci_param)',
        'load_boot_module("xhci_mtk", NULL)',
        'load_boot_module("ehci_hcd", NULL)',
        'load_boot_module("ohci_hcd", NULL)',
    )),
    ("load_ata_modules(void)", ('load_boot_module("ahci", NULL)',)),
    ("load_mmc_modules(void)", (
        'load_boot_module("mmc_block", NULL)',
        'load_boot_module("sdhci", NULL)',
        'load_boot_module("sdhci-pltfm", NULL)',
        'load_boot_module("mt7621-sd", NULL)',
    )),
    ("load_crypto_modules(void)", ('load_boot_module("mtk_aes", NULL)',)),
    ("load_ipset_modules(void)", (
        'load_boot_module("xt_set", NULL)',
        'load_boot_module("ip_set_hash_ip", NULL)',
        'load_boot_module("ip_set_hash_mac", NULL)',
        'load_boot_module("ip_set_hash_net", NULL)',
    )),
):
    function = body(source, signature)
    for call in calls:
        assert call in function, f"{signature}: missing checked call {call}"
    if "return start_status;" in function:
        assert "rc_first_service_start_status(start_status," in function, signature
    else:
        assert "return load_boot_module(" in function, signature

init = body(source, "init_router(void)")
assert "boot_modules_status = load_wireless_modules();" in init
for group in ("load_mmc_modules()", "load_usb_modules()", "load_ata_modules()",
              "load_crypto_modules()", "load_ipset_modules()"):
    assert f"boot_modules_status, {group});" in init, f"init_router must aggregate {group}"
assert "rwfs_mount_status = mount_rwfs_partition();" in init
assert "boot_services_status = rc_first_service_start_status(" in init
assert "rwfs_mount_status, boot_modules_status);" in " ".join(init.split())
assert "one or more unavailable hardware modules" in init

normalized_init = " ".join(init.split())
for name, call in (
    ("detect_link_start_status", "start_detect_link()"),
    ("detect_internet_start_status", "start_detect_internet(0)"),
    ("dnsmasq_start_status", "start_dns_dhcpd(is_ap_mode)"),
    ("wins_start_status", "start_wins()"),
    ("service_start_status", "start_services_once(is_ap_mode)"),
):
    assert f"{name} = {call};" in normalized_init, (
        f"init_router must capture {call}"
    )
    assert f"boot_services_status, {name});" in normalized_init, (
        f"init_router must aggregate {name}"
    )

assert normalized_init.count("logger_start_status = start_logger(1);") == 2
assert normalized_init.count(
    "boot_services_status, logger_start_status);"
) == 2, "both mutually exclusive logger paths must aggregate status"
assert "one or more boot service startup operations failed" in init

print("RC boot module and service result aggregation contracts: PASS")
