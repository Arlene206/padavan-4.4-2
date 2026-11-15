#!/usr/bin/env python3
"""Source integration checks for fail-closed storage eject before persistence."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def read(path):
    return (ROOT / path).read_text(encoding="utf-8")


def require(condition, message):
    if not condition:
        raise AssertionError(message)


def body(source, signature):
    start = source.find(signature)
    require(start >= 0, "missing function: " + signature)
    brace = source.find("{", start + len(signature))
    require(brace >= 0, "missing function body: " + signature)
    depth = 0
    state = "code"
    index = brace
    while index < len(source):
        char = source[index]
        nxt = source[index + 1] if index + 1 < len(source) else ""
        if state == "code":
            if char == "/" and nxt == "*":
                state = "block"
                index += 1
            elif char == "/" and nxt == "/":
                state = "line"
                index += 1
            elif char == '"':
                state = "string"
            elif char == "'":
                state = "char"
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return source[brace + 1:index]
        elif state in ("string", "char"):
            if char == "\\":
                index += 1
            elif (state == "string" and char == '"') or (state == "char" and char == "'"):
                state = "code"
        elif state == "block" and char == "*" and nxt == "/":
            state = "code"
            index += 1
        elif state == "line" and char == "\n":
            state = "code"
        index += 1
    raise AssertionError("unterminated function: " + signature)


libdisk = read("trunk/user/libdisk/src/disk_initial.c")
disk_policy = read("trunk/user/libdisk/src/disk_inventory_policy.h")
disk_policy_test = read("trunk/user/libdisk/src/tests/disk_inventory_policy.c")
disk_header = read("trunk/user/libdisk/src/disk_initial.h")
storage = read("trunk/user/rc/src/services_stor.c")
services = read("trunk/user/rc/src/services.c")
services_usb = read("trunk/user/rc/src/services_usb.c")
net = read("trunk/user/rc/src/net.c")
rc = read("trunk/user/rc/src/rc.c")
rc_header = read("trunk/user/rc/src/rc.h")
policy = read("trunk/user/rc/src/storage_service_stop_policy.h")
workflow = read(".github/workflows/storage-hotplug-stability.yml")
xy_workflow = read(".github/workflows/xy-c1-firmware-validation.yml")

checked = body(libdisk, "read_disk_data_checked(disk_info_t **disk_info_out)")
require("if (!disk_info_out)" in checked, "checked inventory must validate its output")
require("fopen(PROC_PARTITIONS_FILE" in checked and "ferror(fp)" in checked and "fclose(fp)" in checked,
        "checked inventory must detect open/read/close failures")
require("if (!create_disk(" in checked and "if (!create_partition(" in checked,
        "allocation/partition decode failures must not publish partial inventory")
require("*disk_info_out = disk_info_list;" in checked and "return 0;" in checked,
        "checked inventory must distinguish valid empty inventory from failure")
require("disk_inventory_parse_header_line(line)" in checked and
        "disk_inventory_line_is_blank(line)" in checked and
        "disk_inventory_parse_partition_line(" in checked,
        "checked inventory must reject malformed headers, rows and truncation")
require("disk_inventory_parse_decimal" in disk_policy and "UINT_MAX" in disk_policy and "ULLONG_MAX" in disk_policy,
        "procfs numeric fields must be strict decimal values with overflow checks")
require("1844674407370955161ULL" in disk_policy and "/ 10ULL" not in disk_policy and
        "/ 10UL" not in disk_policy,
        "strict disk parsing must avoid division so MIPS libdisk cannot import __udivdi3")
require(" -8 0 10 sda" in disk_policy_test and "18446744073709551616" in disk_policy_test and "ULLONG_MAX" in disk_policy_test,
        "parser tests must reject signed numbers and integer overflow")
legacy = body(libdisk, "read_disk_data(void)")
require("read_disk_data_checked(&disk_info_list)" in legacy and "return disk_info_list;" in legacy,
        "legacy read_disk_data API must remain a compatibility wrapper")
require("read_disk_data_checked" in disk_header and "read_disk_data(void)" in disk_header,
        "both checked and legacy inventory APIs must be exported")

count = body(storage, "count_stor_mountpoint(void)")
require("storage_stage_unmount_mounts(&mounts)" in count and
        "free_storage_unmount_mounts(mounts)" in count,
        "final mount count must use the complete parsed mount inventory")

eject = body(storage, "safe_remove_all_stor_devices(int do_spindown)")
require("read_disk_data_checked(&disks_info)" in eject and
        eject.index("read_disk_data_checked(&disks_info)") < eject.index("stop_stor_apps()"),
        "obtain a complete inventory before stopping services")
require("path_status = disks_info ? umount_stor_path(disks_info, 0, NULL, do_spindown) : 0" in eject,
        "a valid empty inventory must still reach generic mount cleanup")
require("storage_operation_lock_acquire(file_lock)" in eject and "stop_stor_apps()" in eject,
        "eject must retain operation lock and storage-service stop preflight")
require("read_disk_data_checked(&disks_info)" in eject,
        "eject must refuse an unknown /proc/partitions snapshot")
require(eject.index("read_disk_data_checked(&disks_info)") < eject.index("umount_stor_path("),
        "eject must not unmount before the complete disk inventory")
require("umount_all_storage()" in eject and "count_stor_mountpoint()" in eject,
        "eject must verify fallback result and final mount inventory")
require("storage_safe_remove_complete(path_status, fallback_status, mount_count)" in eject,
        "eject must use the fail-closed postcondition")
require("return result;" in eject, "eject must return operation status")
require("int safe_remove_all_stor_devices(int do_spindown);" in rc_header,
        "RC header must expose the checked eject result")
require('ret = safe_remove_all_stor_devices(1);' in rc,
        "ejall must propagate a failed eject status to its caller")
complete = body(policy, "storage_safe_remove_complete(")
require("path_status != 0" in complete and "fallback_status != 0" in complete and "remaining_mounts != 0" in complete,
        "safe eject must reject unmount failures and residual/unknown mounts")

# Printer daemon stop results must reach the firmware flash gate.
for declaration in ("int stop_u2ec(void);", "int stop_lpd(void);",
                    "int stop_p910nd(void);", "int stop_usb_printer_spoolers(void);"):
    require(declaration in rc_header, "RC header must expose checked printer stop API: " + declaration)
for signature in ("int\nstop_u2ec(void)", "int\nstop_lpd(void)", "int\nstop_p910nd(void)"):
    stop_fn = body(services_usb, signature)
    require("return kill_services(" in stop_fn,
            "printer stop must propagate kill_services result: " + signature)
printer_stop = body(services_usb, "int\nstop_usb_printer_spoolers(void)")
for daemon in ("u2ec", "lpd", "p910nd"):
    require('"' + daemon + '"' in printer_stop,
            "printer stop aggregate must include " + daemon)
require("record_usb_printer_stop_status" in printer_stop and "return stop_status;" in printer_stop,
        "printer group must aggregate every stop result")
for signature in ("restart_usb_printer_spoolers(void)", "try_start_usb_printer_spoolers(void)"):
    starter = body(services_usb, signature)
    require("stop_usb_printer_spoolers() != 0" in starter and
            "return;" in starter and "exec_printer_daemons(" in starter,
            "printer restart must abort launch after a failed stop: " + signature)
stop_all = body(services, "int\nstop_services(int stopall)")
for daemon in ("p910nd", "lpd", "u2ec", "IPTV proxies",
               "scutclient", "mentohust", "vlmcsd", "iperf3",
               "AdGuardHome", "Shadowsocks", "Shadowsocks tunnel",
               "Adbyby", "DDNSTO", "SQM", "AliyunDrive WebDAV",
               "WireGuard", "SmartDNS", "AliDDNS", "ttyd", "FRP"):
    require('record_service_stop_status(&stop_status, "' + daemon + '"' in stop_all,
            "stop_services must aggregate stop status: " + daemon)

for name, script in (
    ("scutclient", "/usr/bin/scutclient.sh"),
    ("mentohust", "/usr/bin/mentohust.sh"),
    ("ttyd", "/usr/bin/ttyd.sh"),
    ("ss", "/usr/bin/shadowsocks.sh"),
    ("ss_tunnel", "/usr/bin/ss-tunnel.sh"),
    ("vlmcsd", "/usr/bin/vlmcsd.sh"),
    ("iperf3", "/usr/bin/iperf3.sh"),
    ("adguardhome", "/usr/bin/adguardhome.sh"),
    ("ddnsto", "/usr/bin/ddnsto.sh"),
    ("sqm", "/usr/lib/sqm/run.sh"),
    ("aldriver", "/usr/bin/aliyundrive-webdav.sh"),
    ("wireguard", "/usr/bin/wireguard.sh"),
    ("adbyby", "/usr/bin/adbyby.sh"),
    ("smartdns", "/usr/bin/smartdns.sh"),
    ("aliddns", "/usr/bin/aliddns.sh"),
):
    stop_body = body(services, "int stop_" + name + "(void)")
    require('return eval("' + script + '","stop");' in stop_body,
            "optional service stop must propagate its shell status: " + name)
    require("int stop_" + name + "(void);" in rc_header,
            "RC header must expose checked stop status: " + name)
frp_stop = body(services, "int stop_frp(void)")
require("return status;" in frp_stop and 'eval("/usr/bin/frp.sh","stop")' in frp_stop,
        "FRP stop must propagate the launcher status")
require("int stop_frp(void);" in rc_header,
        "RC header must expose checked FRP stop status")
for name in ("scutclient", "mentohust", "ttyd", "ss", "ss_tunnel",
             "vlmcsd", "iperf3", "sqm", "aldriver", "wireguard", "smartdns"):
    restart_body = body(services, "void restart_" + name + "(void)")
    require("stop_" + name + "() != 0" in restart_body and "start_" + name + "();" in restart_body,
            "manual stop/start wrapper must not launch after failed stop: " + name)
require(stop_all.count('record_service_stop_status(&stop_status, "WireGuard"') == 1,
        "stop_services must only stop WireGuard once")
igmp_stop = body(net, "int\nstop_igmpproxy(char *wan_ifname)")
require("return 0;" in igmp_stop and "return kill_services(" in igmp_stop,
        "IPTV proxy stop must report success only for intentional no-op or verified stop")
igmp_start = body(net, "void\nstart_igmpproxy(char *wan_ifname)")
require(igmp_start.index("stop_igmpproxy(wan_ifname) != 0") < igmp_start.index("start_udpxy(wan_ifname)"),
        "IPTV restart must not launch replacements after a failed stop")
iptv_restart = body(net, "void\nrestart_iptv(int is_ap_mode)")
require('stop_igmpproxy(NULL) != 0' in iptv_restart and
        iptv_restart.index('stop_igmpproxy(NULL) != 0') < iptv_restart.index("start_udpxy(IFNAME_BR)"),
        "AP-mode IPTV restart must abort replacements after a failed stop")
require("int stop_igmpproxy(char *wan_ifname);" in rc_header,
        "RC header must export checked IPTV stop status")

flash = body(rc, "flash_firmware(void)")
require("stop_usb_printer_spoolers() != 0" in flash and "stop_igmpproxy(NULL) != 0" in flash,
        "firmware write must be blocked by failed printer or IPTV proxy stop")
flash_gate = flash.index("if (safe_remove_all_stor_devices(0) != 0)")
require(flash_gate < flash.index("storage_save_time(") < flash.index("write_storage_to_mtd(") <
        flash.index('eval("/tmp/mtd_write"'),
        "firmware persistence and flash must follow successful storage eject")
shutdown = body(rc, "shutdown_router(int level)")
require("misc_stop_status = stop_misc();" in shutdown and
        "service_stop_status = stop_services(use_halt);" in shutdown and
        "printer_stop_status = stop_usb_printer_spoolers();" in shutdown and
        "lan_wan_stop_status = stop_services_lan_wan();" in shutdown,
        "shutdown must collect all checked service-stop results")
require("service shutdown incomplete; continuing system power-down" in shutdown and
        "LAN/WAN service shutdown incomplete; continuing power-down without storage persistence" in shutdown,
        "safe reboot/halt must proceed even when daemon stop fails")
shutdown_gate = shutdown.index("storage_remove_status = safe_remove_all_stor_devices(use_halt)")
persist_gate = shutdown.index("if (storage_remove_status == 0 && misc_stop_status == 0")
require(shutdown_gate < persist_gate < shutdown.index("storage_save_time(") <
        shutdown.index("write_storage_to_mtd("),
        "shutdown must power down but gate storage persistence on all service-stop/eject results")

push_at = workflow.index("  push:\n    paths:\n")
pr_at = workflow.index("  pull_request:\n    paths:\n")
jobs_at = workflow.index("\n\njobs:")
push_filters = workflow[push_at:pr_at]
pull_filters = workflow[pr_at:jobs_at]
for path in ("trunk/user/libdisk/src/disk_initial.c",
             "trunk/user/libdisk/src/disk_initial.h",
             "trunk/user/libdisk/src/disk_inventory_policy.h",
             "trunk/user/libdisk/src/tests/disk_inventory_policy.c",
             "scripts/check_storage_safe_remove.py"):
    require('"' + path + '"' in push_filters, "storage push filter missing " + path)
    require('"' + path + '"' in pull_filters, "storage pull_request filter missing " + path)
for path in ("trunk/user/libdisk/src/disk_initial.c",
             "trunk/user/libdisk/src/disk_initial.h",
             "trunk/user/libdisk/src/disk_inventory_policy.h",
             "trunk/user/rc/src/storage_service_stop_policy.h"):
    require('"' + path + '"' in xy_workflow, "XY-C1 target build does not watch " + path)

print("Storage eject inventory, failure propagation and persistence gates: PASS")
