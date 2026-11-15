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
    while start >= 0:
        signature_end = start + len(signature)
        brace = source.find("{", signature_end)
        semicolon = source.find(";", signature_end)
        if brace >= 0 and (semicolon < 0 or brace < semicolon):
            break
        start = source.find(signature, semicolon + 1) if semicolon >= 0 else -1
    require(start >= 0, "missing function definition: " + signature)
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


# A forward declaration must not bind the checker to the next function body.
_probe = "static int probe(void);\nstatic int probe(void) { return 1; }\n"
require("return 1;" in body(_probe, "probe(void)"),
        "function extractor must skip a forward declaration")


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
common_ex_source = read("trunk/user/rc/src/common_ex.c")
optware_policy = read("trunk/user/rc/src/storage_optware_start_policy.h")
optware_policy_test = read("trunk/user/rc/src/tests/storage_optware_start_policy.c")
policy = read("trunk/user/rc/src/storage_service_stop_policy.h")
ftp_policy = read("trunk/user/rc/src/ftp_daemon_policy.h")
ftp_test = read("trunk/user/rc/src/tests/ftp_daemon_policy.c")
ftp_policy = read("trunk/user/rc/src/ftp_daemon_policy.h")
ftp_test = read("trunk/user/rc/src/tests/ftp_daemon_policy.c")
workflow = read(".github/workflows/storage-hotplug-stability.yml")
xy_workflow = read(".github/workflows/xy-c1-firmware-validation.yml")
mtd_storage = read("trunk/user/scripts/files/sbin/mtd_storage.sh")
persistence_policy = read("trunk/user/rc/src/storage_persistence_policy.h")
persistence_test = read("trunk/user/rc/src/tests/storage_persistence_policy.c")
mtd_storage_test = read("trunk/user/scripts/tests/mtd_storage_save_retry.sh")
mtd_storage_restore_test = read("trunk/user/scripts/tests/mtd_storage_restore_transaction.sh")
mtd_storage_load_test = read("trunk/user/scripts/tests/mtd_storage_load_transaction.sh")
mtd_storage_backup_test = read("trunk/user/scripts/tests/mtd_storage_backup_failure.sh")

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
        "FTP, Transmission and Aria firewall refresh must use tri-state process state")
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
stop_body = body(storage, "stop_dms(void)")
require("storage_service_stop_complete(" in stop_body,
        "stop_dms must verify daemon absence after stop")
stop_body = body(storage, "stop_itunes(void)")
require("storage_service_stop_complete(" in stop_body,
        "stop_itunes must verify daemon absence after stop")
for signature, daemon, suffix, label in (
        ("static int stop_torrent_locked(void)", "transmission-daemon", "torrent", "Transmission"),
        ("static int stop_aria_locked(void)", "aria2c", "aria", "Aria")):
    locked_stop = body(storage, signature)
    require("enum process_presence process_state;" in locked_stop and
            "process_state = is_{}_run();".format(suffix) in locked_stop,
            "{} stop must preserve tri-state preflight".format(label))
    require("process_state == PROCESS_PRESENCE_ABSENT" in locked_stop and
            "process_state != PROCESS_PRESENCE_RUNNING" in locked_stop and
            "refusing name-wide stop because daemon presence is unknown" in locked_stop,
            "{} stop must not signal on UNKNOWN presence".format(label))
    expected_svcs = 'char *svcs[] = { "' + daemon + '", NULL };'
    require(expected_svcs in locked_stop and
            "command_status = kill_services(svcs," in locked_stop and
            "storage_service_stop_complete(command_status, is_{}_run())".format(suffix) in locked_stop,
            "{} stop must use identity-validated process lifecycle transaction".format(label))
    legacy_script = "transmission.sh" if suffix == "torrent" else "aria.sh"
    require('eval("/usr/bin/{}"'.format(legacy_script) not in locked_stop,
            "{} stop must not call legacy pidof/killall shell path".format(label))

stop_body = body(storage, "stop_torrent(void)")
require("storage_service_stop_complete(" in stop_body,
        "stop_torrent must verify daemon absence after stop")
stop_body = body(storage, "stop_aria(void)")
require("storage_service_stop_complete(" in stop_body,
        "stop_aria must verify daemon absence after stop")
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
require("PROCESS_PRESENCE_UNKNOWN" in ftp_policy and "return -1;" in ftp_policy and "EIO" in ftp_policy,
        "FTP daemon wait must fail immediately on unknown process state")
require("PROCESS_PRESENCE_UNKNOWN" in ftp_test and "EACCES" in ftp_test,
        "FTP wait regression must cover process-inventory failures")

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

optional_identity_processes = {
    "scutclient": "bin_scutclient",
    "mentohust": "bin_mentohust",
    "vlmcsd": "vlmcsd",
    "iperf3": "iperf3",
}
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
    if name in optional_identity_processes:
        normalized = " ".join(stop_body.split())
        expected = ('return optional_daemon_stop_status("{0}", "{1}", '
                    'eval("{2}", "stop"));').format(
                        name, optional_identity_processes[name], script)
        require(expected in normalized,
                "identity-managed stop must preserve command status and verify daemon absence: " + name)
    else:
        require('return eval("' + script + '","stop");' in stop_body,
                "optional service stop must propagate its shell status: " + name)
    require("int stop_" + name + "(void);" in rc_header,
            "RC header must expose checked stop status: " + name)

optional_policy = read("trunk/user/rc/src/optional_daemon_policy.h")
optional_presence = body(services, "optional_daemon_presence(const char *process_name, int *scan_error)")
require("process_inventory_scan(process_name, &snapshot)" in optional_presence and
        "process_snapshot_release(&snapshot)" in optional_presence and
        "PROCESS_PRESENCE_UNKNOWN" in optional_presence,
        "optional daemon inventory must preserve UNKNOWN and release its snapshot")
optional_stop = body(services, "optional_daemon_stop_status(const char *service_name, const char *process_name,")
require("optional_daemon_presence(process_name, &scan_error)" in optional_stop and
        "optional_daemon_stop_complete(command_status, presence)" in optional_stop,
        "optional daemon stop must verify tri-state process absence after the command")
stop_policy = body(optional_policy, "optional_daemon_stop_complete(int command_status, enum process_presence state)")
require("if (command_status != 0)" in stop_policy and
        "return command_status;" in stop_policy and
        "state == PROCESS_PRESENCE_ABSENT ? 0 : -1" in stop_policy,
        "optional daemon stop policy must preserve command errors and require confirmed ABSENT")

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
rwfs_unmount = body(common_ex_source, "int umount_rwfs_partition(void)")
require("int umount_rwfs_partition(void);" in rc_header,
        "RWFS unmount must expose a checked status")
require("opt-umount.sh" in rwfs_unmount and "return EIO;" in rwfs_unmount,
        "Optware stop-hook failure must propagate")
require("umount(mp_rwfs)" in rwfs_unmount and "return operation_status;" in rwfs_unmount,
        "RWFS filesystem unmount failure must propagate")
require("rwfs_ubi_attachment_presence(mtd_rwfs, &ubi_before)" in rwfs_unmount and
        "rwfs_ubi_attachment_presence(mtd_rwfs, &ubi_after)" in rwfs_unmount,
        "UBI attachment must be verified before and after detach")
require("storage_optware_ubi_detach_complete(ubi_before, ubi_after)" in rwfs_unmount,
        "UBI detach must require confirmed absence")
ubi_presence = body(common_ex_source, "rwfs_ubi_attachment_presence(int mtd_rwfs, int *presence)")
require('"/sys/class/ubi/ubi%d"' in ubi_presence and '"%s/mtd_num"' in ubi_presence and
        'fscanf(fp, "%d", &attached_mtd)' in ubi_presence,
        "UBI identity must be read from its kernel mtd_num attribute")
require("storage_optware_ubi_detach_complete" in optware_policy and
        "STORAGE_OPTWARE_STOP_UNKNOWN" in optware_policy_test,
        "UBI detach policy must have regression coverage")
require("rwfs_unmount_status = umount_rwfs_partition();" in shutdown and
        "rwfs_unmount_status == 0" in shutdown,
        "shutdown must gate MTD persistence on RWFS teardown")
rwfs_call = shutdown.index("rwfs_unmount_status = umount_rwfs_partition();")
require(rwfs_call < persist_gate < shutdown.index("storage_save_time(") <
        shutdown.index("write_storage_to_mtd("),
        "RWFS teardown must finish before MTD persistence")
require(shutdown.count("umount_rwfs_partition(") == 1,
        "shutdown must not unmount RWFS after MTD persistence")

push_at = workflow.index("  push:\n    paths:\n")
pr_at = workflow.index("  pull_request:\n    paths:\n")
jobs_at = workflow.index("\n\njobs:")
push_filters = workflow[push_at:pr_at]
pull_filters = workflow[pr_at:jobs_at]
for path in ("trunk/user/libdisk/src/disk_initial.c",
             "trunk/user/libdisk/src/disk_initial.h",
             "trunk/user/libdisk/src/disk_inventory_policy.h",
             "trunk/user/libdisk/src/tests/disk_inventory_policy.c",
             "scripts/check_storage_safe_remove.py",
             "trunk/user/rc/src/optional_daemon_policy.h",
             "trunk/user/shared/src/process_inventory.h"):
    require('"' + path + '"' in push_filters, "storage push filter missing " + path)
    require('"' + path + '"' in pull_filters, "storage pull_request filter missing " + path)
for path in ("trunk/user/libdisk/src/disk_initial.c",
             "trunk/user/libdisk/src/disk_initial.h",
             "trunk/user/libdisk/src/disk_inventory_policy.h",
             "trunk/user/rc/src/storage_service_stop_policy.h",
             "trunk/user/rc/src/storage_optware_start_policy.h",
             "trunk/user/rc/src/optional_daemon_policy.h"):
    require('"' + path + '"' in xy_workflow, "XY-C1 target build does not watch " + path)


# A failed flash must not advance the MD5 checkpoint and suppress identical retries.
mtd_save_start = mtd_storage.index("func_save()")
mtd_save_end = mtd_storage.index("\nfunc_backup()", mtd_save_start)
mtd_save = mtd_storage[mtd_save_start:mtd_save_end]
require('md5sum "$tmp" > "$hash_tmp"' in mtd_save and
        'mv -f "$hash_tmp" "$hsh"' in mtd_save,
        "MTD save must stage its content hash")
require(mtd_save.index("if mtd_write write") <
        mtd_save.index('mv -f "$hash_tmp" "$hsh"'),
        "MTD hash checkpoint must be published only after flash success")
require("MTD write succeeded but storage hash checkpoint failed" in mtd_save and
        'rm -f "$tmp" "$tbz" "$hash_tmp"' in mtd_save,
        "MTD save must surface checkpoint failures and clean scratch files")
require("MTD_WRITE_FAIL=1" in mtd_storage_test and
        "expected three MTD attempts" in mtd_storage_test and
        "unchanged save unexpectedly flashed again" in mtd_storage_test,
        "MTD retry test must cover write failure, identical retry and unchanged skip")

# Boot-time MTD load must reject incomplete archives before publishing.
mtd_load_start = mtd_storage.index("func_load()")
mtd_load_end = mtd_storage.index("\nfunc_tarb()", mtd_load_start)
mtd_load = mtd_storage[mtd_load_start:mtd_load_end]
for token in ('if ! bzcat "$mtd_part_dev" > "$tmp" 2>/dev/null; then',
              'if ! tar xf "$tmp" -C "$load_storage" 2>/dev/null; then',
              'if ! md5sum "$tmp" > "$hash_tmp"; then',
              'mv "$dir_storage" "$backup_storage"',
              'mv "$load_storage" "$dir_storage"',
              'mv "$backup_storage" "$dir_storage"',
              'if ! mv -f "$hash_tmp" "$hsh"; then',
              'if ! rm -f "$slk"; then'):
    require(token in mtd_load, "MTD load misses " + token)
require(mtd_load.index('if ! bzcat') < mtd_load.index('if ! tar xf') <
        mtd_load.index('if ! md5sum') < mtd_load.index('mv "$dir_storage" "$backup_storage"') <
        mtd_load.index('mv "$load_storage" "$dir_storage"') <
        mtd_load.index('if ! mv -f "$hash_tmp" "$hsh"') <
        mtd_load.index('if ! rm -f "$slk"; then'),
        "MTD load must validate and stage the full tree before publication")
require('tar xf "$tmp" -C "$dir_storage"' not in mtd_load and
        'rm -rf "$dir_storage"' not in mtd_load,
        "MTD load must never extract over or delete live config")
require("BZCAT_STATUS=2" in mtd_storage_load_test and
        "TAR_FAIL=1" in mtd_storage_load_test and
        "MV_FAIL_AT=2" in mtd_storage_load_test and
        "partial-extract.conf" in mtd_storage_load_test and
        'if [ -n "${BUSYBOX_BIN:-}" ]; then' in mtd_storage_load_test and
        '"$BUSYBOX_BIN" ash "$sandbox/mtd_storage.sh" load' in mtd_storage_load_test and
        'BUSYBOX_BIN="$busybox_bin" sh trunk/user/scripts/tests/mtd_storage_load_transaction.sh' in workflow,
        "MTD load fixture must cover decompression, extraction, rollback, BusyBox ash and success")

backup_start = mtd_storage.index("func_backup()")
backup_end = mtd_storage.index("\\nfunc_restore()", backup_start)
backup = mtd_storage[backup_start:backup_end]
for token in ('if ! bzip2 -9 "$tmp" 2>/dev/null; then',
              'stat -c %s "$tbz"',
              'if ! bzcat "$tbz" > "$tmp" 2>/dev/null ||',
              'tar tf "$tmp" >/dev/null 2>&1',
              'rm -f "$tmp" "$tbz"'):
    require(token in backup, "MTD backup validation misses " + token)
backup_dispatch = mtd_storage[mtd_storage.index("backup)"):mtd_storage.index("restore)")]
require("func_get_mtd" in backup_dispatch and
        backup_dispatch.index("func_get_mtd") < backup_dispatch.index("func_tarb") < backup_dispatch.index("func_backup"),
        "MTD backup must read the actual target partition size before compression")
require("BZIP2_MODE=fail" in mtd_storage_backup_test and
        "BZIP2_MODE=oversized" in mtd_storage_backup_test and
        "BZCAT_MODE=fail" in mtd_storage_backup_test and
        "BZCAT_MODE=invalid" in mtd_storage_backup_test and
        "MTD storage backup failure and integrity regression" in mtd_storage_backup_test,
        "MTD backup fixture must cover compression, size, stream, tar and successful archive")

writer = body(rc, "write_storage_to_mtd(void)")
require('return doSystem("/sbin/mtd_storage.sh %s", "save");' in writer and
        "int write_storage_to_mtd(void);" in rc_header,
        "RC MTD writer must propagate the storage script status")
time_save = body(rc, "storage_save_time(time_t delta)")
for operation in ("mkstemp(temp_path)", 'fdopen(fd, "w")',
                  "fchmod(fd, 0644)", "fflush(fp)", "fsync(fileno(fp))",
                  "fclose(fp)", 'rename(temp_path, "/etc/storage/system_time")'):
    require(operation in time_save, "timestamp save misses checked operation: " + operation)
require(time_save.index("fflush(fp)") < time_save.index("fsync(fileno(fp))") <
        time_save.index("if (fclose(fp) != 0)") <
        time_save.index('rename(temp_path, "/etc/storage/system_time")'),
        "timestamp temp file must be flushed, synced, closed and renamed in order")
require("(void)unlink(temp_path);" in time_save and
        "errno = saved_errno;" in time_save,
        "timestamp save must clean temporary files and preserve errno")

require("storage_persistence_mtd_may_run" in persistence_policy and
        "storage_persistence_complete" in persistence_policy and
        "storage_persistence_complete(-1, 0)" in persistence_test and
        "storage_persistence_complete(0, -1)" in persistence_test,
        "firmware persistence gate policy needs failure cases")
flash = body(rc, "flash_firmware(void)")
flash_time = flash.index("storage_time_status = storage_save_time(60)")
flash_mtd = flash.index("storage_mtd_status = storage_persistence_mtd_may_run(storage_time_status)")
flash_gate = flash.index("if (!storage_persistence_complete(storage_time_status, storage_mtd_status))")
flash_image = flash.index('eval("/tmp/mtd_write", "-r", "write", FW_IMG_NAME, FW_MTD_NAME)')
require(flash_time < flash_mtd < flash_gate < flash_image and
        "watchdog_status = start_watchdog();" in flash[flash_gate:flash_image],
        "firmware image write must require successful storage persistence and recover watchdog on abort")
startup = body(rc, "init_router(void)")
require("storage_save_status = write_storage_to_mtd();" in startup and
        "if (storage_save_status != 0)" in startup and
        startup.index("if (storage_save_status != 0)") < startup.index("restart_crond()"),
        "startup must report snapshot failure and still restart crond")
shutdown_writer = body(rc, "shutdown_router(int level)")
shutdown_time = shutdown_writer.index("storage_time_status = storage_save_time(10)")
shutdown_mtd = shutdown_writer.index("storage_mtd_status = storage_persistence_mtd_may_run(storage_time_status)")
shutdown_gate = shutdown_writer.index("if (!storage_persistence_complete(storage_time_status, storage_mtd_status))")
require(shutdown_time < shutdown_mtd < shutdown_gate and
        "continuing power-down" in shutdown_writer[shutdown_gate:],
        "shutdown must report persistence errors without aborting power-down")
require("int storage_save_time(time_t delta);" in rc_header,
        "RC header must expose timestamp status")

for path in ("trunk/user/scripts/files/sbin/mtd_storage.sh",
             "trunk/user/scripts/tests/mtd_storage_save_retry.sh",
             "trunk/user/scripts/tests/mtd_storage_restore_transaction.sh",
             "trunk/user/scripts/tests/mtd_storage_load_transaction.sh",
             "trunk/user/scripts/tests/mtd_storage_backup_failure.sh",
             "trunk/user/scripts/tests/mtd_storage_load_transaction.sh",
             "trunk/user/rc/src/storage_persistence_policy.h",
             "trunk/user/rc/src/tests/storage_persistence_policy.c"):
    require('"' + path + '"' in push_filters and '"' + path + '"' in pull_filters,
            "storage-hotplug must trigger for persistence file " + path)
for path in ("trunk/user/scripts/files/sbin/mtd_storage.sh",
             "trunk/user/rc/src/storage_persistence_policy.h"):
    require('"' + path + '"' in xy_workflow,
            "XY-C1 must trigger for product persistence dependency " + path)

restore_start = mtd_storage.index("func_restore()")
restore_end = mtd_storage.index("\nfunc_erase()", restore_start)
restore = mtd_storage[restore_start:restore_end]
require("mktemp -d" in restore and 'cp -a "$tmp_storage"/. "$stage_storage"/' in restore and
        'chmod 755 "$stage_storage"' in restore,
        "storage restore must prepare a complete same-filesystem staging tree")
require(restore.index("cp -a") < restore.index("func_stop_apps") <
        restore.index('mv "$dir_storage" "$rollback_storage"') <
        restore.index('mv "$stage_storage" "$dir_storage"'),
        "storage restore must stage before stopping apps and replacing live settings")
require('mv "$rollback_storage" "$dir_storage"' in restore and
        "restore install and rollback both failed" in restore and
        "Cannot prepare complete storage restore tree" in restore,
        "storage restore must preserve and recover the previous directory on install failures")
require("RESTORE_FAIL_CP=1" in mtd_storage_restore_test and
        "RESTORE_FAIL_MV_AT=2" in mtd_storage_restore_test and
        "rollback" in mtd_storage_restore_test,
        "storage restore regression must inject copy and install-rename failures")

print("Storage eject inventory, failure propagation and persistence gates: PASS")
