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
ftp_policy = read("trunk/user/rc/src/ftp_daemon_policy.h")
ftp_test = read("trunk/user/rc/src/tests/ftp_daemon_policy.c")
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
require("storage_service_process_presence_changed" in policy and
        "before < 0 || after < 0" in policy,
        "unknown daemon state must force conservative firewall reconciliation")
require("storage_service_process_presence_changed" in storage,
        "FTP, Transmission and Aria firewall refresh must use tri-state presence")
presence = body(storage, "storage_daemon_presence(const char *daemon, int *scan_error)")
require("process_inventory_scan(daemon, &snapshot)" in presence and
        "process_snapshot_release(&snapshot);" in presence and
        "PROCESS_PRESENCE_UNKNOWN" in presence,
        "storage daemon query must preserve process-inventory UNKNOWN")
for daemon, signature in (
    ("vsftpd", "is_ftp_run(void)"),
    ("minidlna", "int is_dms_run(void)"),
    ("mt-daapd", "int is_itunes_run(void)"),
    ("transmission-daemon", "int is_torrent_run(void)"),
    ("aria2c", "int is_aria_run(void)"),
):
    state_fn = body(storage, signature)
    require('storage_daemon_presence("' + daemon + '", NULL)' in state_fn,
            "storage daemon query must use tri-state inventory: " + daemon)
ftp_stop = body(storage, "stop_ftp_locked(void)")
require("storage_service_stop_complete(status, is_ftp_run())" in ftp_stop,
        "FTP stop must verify daemon absence before eject")
ftp_start = body(storage, "run_ftp(void)")
require("process_state != PROCESS_PRESENCE_ABSENT" in ftp_start and
        "PROCESS_PRESENCE_RUNNING" in ftp_start,
        "FTP start must refuse unknown daemon presence")
for signature in ("void run_dms(int force_rescan)", "void run_itunes(void)",
                  "void run_torrent(void)", "void run_aria(void)"):
    run_body = body(storage, signature)
    require("process_state != PROCESS_PRESENCE_ABSENT" in run_body and
            "PROCESS_PRESENCE_RUNNING" in run_body,
            "storage daemon start must fail closed on unknown presence: " + signature)
require("PROCESS_PRESENCE_UNKNOWN" in ftp_policy and
        "return -1;" in ftp_policy and "EIO" in ftp_policy,
        "FTP daemon wait must fail immediately on unknown process state")
require("PROCESS_PRESENCE_UNKNOWN" in ftp_test and "EACCES" in ftp_test,
        "FTP wait regression must cover process-inventory failures")


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
               "WireGuard", "SmartDNS", "ZeroTier", "AliDDNS", "ttyd", "FRP"):
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
zerotier_stop = body(services, "int stop_zerotier(void)")
require('return eval("/usr/bin/zerotier.sh","stop");' in zerotier_stop and
        "int stop_zerotier(void);" in rc_header,
        "ZeroTier stop must propagate launcher status")
zerotier_restart = " ".join(body(services, "int restart_zerotier(void)").split())
require("int stop_status = stop_zerotier();" in zerotier_restart and
        "if (stop_status != 0)" in zerotier_restart and
        "return stop_status;" in zerotier_restart and
        "start_zerotier();" in zerotier_restart and
        zerotier_restart.index("return stop_status;") < zerotier_restart.index("start_zerotier();") < zerotier_restart.index("return 0;"),
        "ZeroTier restart must propagate stop failure and gate the replacement start")
require('record_service_stop_status(&stop_status, "ZeroTier", stop_zerotier())' in stop_all,
        "stop_services must aggregate ZeroTier stop status")

frp_stop = body(services, "int stop_frp(void)")
require("return status;" in frp_stop and 'eval("/usr/bin/frp.sh","stop")' in frp_stop,
        "FRP stop must propagate the launcher status")
require("int stop_frp(void);" in rc_header,
        "RC header must expose checked FRP stop status")
for name in ("scutclient", "mentohust", "ttyd", "ss", "ss_tunnel",
             "vlmcsd", "iperf3", "sqm", "aldriver", "wireguard", "smartdns"):
    restart_body = body(services, "int restart_" + name + "(void)")
    require("int stop_status = stop_" + name + "();" in restart_body and "return stop_status;" in restart_body and "start_" + name + "();" in restart_body,
            "manual stop/start wrapper must not launch after failed stop: " + name)
require(stop_all.count('record_service_stop_status(&stop_status, "WireGuard"') == 1,
        "stop_services must only stop WireGuard once")
igmp_stop = body(net, "int\nstop_igmpproxy(char *wan_ifname)")
require("return 0;" in igmp_stop and "return kill_services(" in igmp_stop,
        "IPTV proxy stop must report success only for intentional no-op or verified stop")
igmp_start = body(net, "int\nstart_igmpproxy(char *wan_ifname)")
require("return stop_status;" in igmp_start and
        igmp_start.index("return stop_status;") < igmp_start.index("start_udpxy(wan_ifname)") and
        'return eval("/bin/igmpproxy", igmpproxy_conf);' in igmp_start,
        "IPTV start must propagate stop and daemon launch status before replacements")
iptv_restart = body(net, "int\nrestart_iptv(int is_ap_mode)")
require("return start_igmpproxy(wan_ifname);" in iptv_restart and
        "stop_status = stop_igmpproxy(NULL);" in iptv_restart and
        "return stop_status;" in iptv_restart and
        iptv_restart.index("return stop_status;") < iptv_restart.index("start_udpxy(IFNAME_BR)") and
        "return 0;" in iptv_restart,
        "IPTV restart must propagate non-AP/AP stop outcomes and abort failed replacements")
require("int restart_iptv(int is_ap_mode);" in rc_header and
        "int start_igmpproxy(char *wan_ifname);" in rc_header,
        "IPTV start/restart declarations must return checked status")
iptv_dispatch = body(rc, 'else if (!strcmp(entry->d_name, RCN_RESTART_IPTV))')
require("rc_notify_merge_action_result(action_result, restart_iptv(is_ap_mode))" in iptv_dispatch,
        "PID 1 IPTV notification must return restart status to queue")
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
        "wan_stop_status = stop_wan();" in shutdown and
        "lan_wan_stop_status = stop_services_lan_wan();" in shutdown,
        "shutdown must collect all checked service-stop results")
require("service shutdown incomplete; continuing system power-down" in shutdown and
        "LAN/WAN service shutdown incomplete; continuing power-down without storage persistence" in shutdown,
        "safe reboot/halt must proceed even when daemon stop fails")
shutdown_gate = shutdown.index("storage_remove_status = safe_remove_all_stor_devices(use_halt)")
persist_gate = shutdown.index("if (storage_remove_status == 0 && misc_stop_status == 0")
require("wan_stop_status == 0" in shutdown[persist_gate:],
        "shutdown must not persist storage when WAN stop failed")
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
