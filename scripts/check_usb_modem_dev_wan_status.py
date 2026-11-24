#!/usr/bin/env python3
"""Guard the USB-modem WAN-selection NVRAM failure-propagation contract."""

from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


def function_body(source: str, signature: str) -> str:
    start = source.index(signature)
    brace = source.index("{", start)
    terminator = source.find(";", start, brace)
    if terminator >= 0:
        raise AssertionError(f"matched a call/prototype rather than definition: {signature}")

    depth = 0
    for position in range(brace, len(source)):
        if source[position] == "{":
            depth += 1
        elif source[position] == "}":
            depth -= 1
            if depth == 0:
                return source[brace + 1:position]
    raise AssertionError(f"unterminated function: {signature}")


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def require_status_propagation(source: str, signature: str, description: str) -> None:
    body = function_body(source, signature)
    assignment = body.find("status = select_usb_modem_to_wan();")
    check = body.find("if (status != 0)", assignment)
    returned = body.find("return status;", check)
    require(assignment >= 0 and check > assignment and returned > check,
            f"{description} must stop when USB modem selection publication fails")


def main() -> None:
    shared = ROOT / "trunk/user/shared/src"
    rc = ROOT / "trunk/user/rc/src"
    netutils = (shared / "netutils.c").read_text()
    netutils_header = (shared / "netutils.h").read_text()
    rc_source = (rc / "rc.c").read_text()
    usb_modem = (rc / "usb_modem.c").read_text()
    net_wan = (rc / "net_wan.c").read_text()
    rc_header = (rc / "rc.h").read_text()
    services_usb = (rc / "services_usb.c").read_text()
    workflow = (ROOT / ".github/workflows/wan-dhcp-callback-stability.yml").read_text()

    require('#include "usb_modem_wan_policy.h"' in netutils,
            "shared NVRAM publisher must use the tested status policy")
    write_callback = function_body(
        netutils,
        "usb_modem_wan_set_temp_int(void *context, const char *name, int value)",
    )
    require("nvram_set_int_temp(name, value)" in write_callback,
            "the checked publisher must wrap the actual temporary NVRAM API")
    publisher = function_body(netutils, "set_usb_modem_dev_wan(int unit, int devnum)")
    publisher = " ".join(publisher.split())
    require("usb_modem_wan_publish_nvram( unit, devnum," in publisher and
            "errno = status;" in publisher,
            "set_usb_modem_dev_wan must return the policy result and preserve errno")
    require("extern int       set_usb_modem_dev_wan(int unit, int devnum);" in netutils_header,
            "shared header must expose the checked setter")
    require("int select_usb_modem_to_wan(void);" in rc_header,
            "WAN selection entrypoint must expose its status")

    wan_setup = function_body(rc_source, "nvram_convert_misc_values(void)")
    assignment = wan_setup.find("status = set_usb_modem_dev_wan(0, 0);")
    check = wan_setup.find("if (status != 0)", assignment)
    returned = wan_setup.find("return status;", check)
    reset = wan_setup.find("reset_wan_vars();", assignment)
    require(assignment >= 0 and check > assignment and returned > check and reset > returned,
            "WAN NVRAM setup must stop before WAN reset if modem selection cannot be cleared")

    boot = function_body(rc_source, "init_router(void)")
    setup_call = boot.find("nvram_setup_status = nvram_convert_misc_values();")
    setup_check = boot.find("if (nvram_setup_status != 0)", setup_call)
    setup_return = boot.find("return INIT_ROUTER_RESULT_FATAL;", setup_check)
    require(setup_call >= 0 and setup_check > setup_call and setup_return > setup_check,
            "PID 1 must abort startup when WAN NVRAM setup fails")

    remove = function_body(usb_modem, "safe_remove_usb_modem(void)")
    assignment = remove.find("publish_status = set_usb_modem_dev_wan(0, 0);")
    check = remove.find("if (publish_status != 0)", assignment)
    returned = remove.find("return publish_status;", check)
    require(assignment >= 0 and check > assignment and returned > check,
            "USB modem removal must report a failed WAN-selection reset")

    require('#include "modem_wan_start_policy.h"' in net_wan,
            "WAN modem selection must include its static-inline policy implementation")
    require("modem_wan_select_device_status(" in net_wan,
            "WAN modem selection must use the checked device-priority policy")
    select = function_body(net_wan, "select_usb_modem_to_wan(void)")
    assignment = select.find("status = set_usb_modem_dev_wan(0, modem_devnum);")
    check = select.find("if (status != 0)", assignment)
    returned = select.find("return status;", check)
    require(assignment >= 0 and check > assignment and returned > check,
            "WAN selection must return the NVRAM publication failure")
    require_status_propagation(net_wan, "full_restart_wan(void)", "full_restart_wan")
    require_status_propagation(
        net_wan, "try_wan_reconnect(int try_use_modem, long pause_in_seconds)",
        "try_wan_reconnect",
    )
    require_status_propagation(net_wan, "deferred_wan_connect(void)", "deferred_wan_connect")

    require("modem_remove_status = safe_remove_usb_modem();" in services_usb and
            "if (modem_remove_status != 0)" in services_usb,
            "USB hotplug must keep handling safe_remove_usb_modem failures")

    for path in (
        "scripts/check_usb_modem_dev_wan_status.py",
        "trunk/user/shared/src/usb_modem_wan_policy.h",
        "trunk/user/rc/src/tests/usb_modem_wan_policy.c",
        "trunk/user/rc/src/rc.c",
        "trunk/user/rc/src/usb_modem.c",
    ):
        require(f'"{path}"' in workflow,
                f"WAN workflow path filters must include {path}")
    require("python3 scripts/check_usb_modem_dev_wan_status.py" in workflow,
            "WAN workflow must execute the source contract")

    print("USB modem WAN status propagation source contract: PASS")


if __name__ == "__main__":
    main()
