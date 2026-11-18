#!/usr/bin/env python3
"""Supplemental integration checks for process-inventory production wiring."""
import re
from pathlib import Path

def c_function_body(source, signature):
    start = source.find(signature)
    while start >= 0:
        signature_end = start + len(signature)
        brace = source.find("{", signature_end)
        semicolon = source.find(";", signature_end)
        if brace >= 0 and (semicolon < 0 or brace < semicolon):
            break
        start = source.find(signature, semicolon + 1) if semicolon >= 0 else -1
    if start < 0:
        raise AssertionError("missing function definition: " + signature)
    brace = source.find("{", start + len(signature))
    if brace < 0:
        raise AssertionError("missing function body: " + signature)

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



root = Path(__file__).resolve().parents[1]
syslogd_source = (root / "trunk/user/busybox/busybox-1.24.x/sysklogd/syslogd.c").read_text()
klogd_source = (root / "trunk/user/busybox/busybox-1.24.x/sysklogd/klogd.c").read_text()
rstats_source = (root / "trunk/user/rc/src/rstats.c").read_text()
lldt_main_source = (root / "trunk/user/lldt/src/main.c").read_text()
lldt_osl_source = (root / "trunk/user/lldt/src/osl-linux.c").read_text()
shared_makefile = (root / "trunk/user/shared/src/Makefile").read_text()
shared_header = (root / "trunk/user/shared/src/shutils.h").read_text()
pids_source = (root / "trunk/user/shared/src/pids.c").read_text()
pids_presence_policy = (root / "trunk/user/shared/src/pids_presence_policy.h").read_text()
pids_presence_test = (root / "trunk/user/shared/src/tests/pids_presence_policy.c").read_text()
common_ex = (root / "trunk/user/rc/src/common_ex.c").read_text()
policy = (root / "trunk/user/rc/src/service_lifecycle_policy.h").read_text()
usb_services = (root / "trunk/user/rc/src/services_usb.c").read_text()
usb_modem = (root / "trunk/user/rc/src/usb_modem.c").read_text()
rc_header = (root / "trunk/user/rc/src/rc.h").read_text()
lifecycle_test = (root / "trunk/user/rc/src/tests/service_lifecycle_policy.c").read_text()
net_lan_notification = (root / "trunk/user/rc/src/net_lan.c").read_text()
assert "service_lifecycle_notification_allowed" in policy
assert "service_lifecycle_notify(" in policy
assert "service_lifecycle_preflight(targets, count, ops, context, 1)" in policy
assert "service_lifecycle_signal_targets(targets, count, ops" in policy
notify_body = policy.split("service_lifecycle_notify(char *service_names[]", 1)[1].split("service_lifecycle_probe_targets", 1)[0]
pidfile_stop_body = common_ex.split("kill_services_with_pidfiles(char *svc_name[]", 1)[1].split("\nint\ncreate_file(", 1)[0]
assert "service_lifecycle_release_targets(targets, count);" in notify_body
assert "free(targets)" not in notify_body
assert "service_lifecycle_release_targets(targets, count);" in pidfile_stop_body
assert "free(targets)" not in pidfile_stop_body
assert "kill_process_pidfile" not in common_ex
assert "kill_pidfile_s(" not in common_ex
assert "kill_pidfile(" not in common_ex
assert "kill_process_pidfile" not in rc_header
rc_source_dir = root / "trunk/user/rc/src"
legacy_pidfile_kill_callers = [
    source for source in rc_source_dir.glob("*.c")
    if "kill_process_pidfile(" in source.read_text()
]
assert legacy_pidfile_kill_callers == []
assert "test_pidfile_identity_change_blocks_escalation" in lifecycle_test
assert "test_pidfile_unknown_blocks_entire_preflight" in lifecycle_test
assert "test_force_only_pidfile_target_skips_term_and_joins_kill" in lifecycle_test
assert "signal_services_by_identity" in rc_header
assert "service_lifecycle_notify(svc_name, signal_number, &ops, NULL)" in common_ex
assert "service_lifecycle_notification_allowed(signal_number)" in common_ex
assert "signal_services_by_identity(httpd_services, SIGUSR1)" in net_lan_notification
assert 'doSystem("killall %s %s", "-SIGUSR1", "httpd")' not in net_lan_notification
assert "test_identity_bound_notification_preflights_all_targets" in lifecycle_test
assert "test_unknown_notification_inventory_sends_zero_signals" in lifecycle_test
net_source = (root / "trunk/user/rc/src/net.c").read_text()
storage_services = (root / "trunk/user/rc/src/services_stor.c").read_text()
rc_header = (root / "trunk/user/rc/src/rc.h").read_text()
firefly_config_header = (root / "trunk/user/rc/src/firefly_config_file.h").read_text()
firefly_config_test = (root / "trunk/user/rc/src/tests/firefly_config_file.c").read_text()
minidlna_config_header = (root / "trunk/user/rc/src/minidlna_config_file.h").read_text()
minidlna_config_test = (root / "trunk/user/rc/src/tests/minidlna_config_file.c").read_text()
igmpproxy_config_header = (root / "trunk/user/rc/src/igmpproxy_config_file.h").read_text()
igmpproxy_config_test = (root / "trunk/user/rc/src/tests/igmpproxy_config_file.c").read_text()
net_lan_notification = (root / "trunk/user/rc/src/net_lan.c").read_text()
pppoe_policy = (root / "trunk/user/rc/src/pppoe_relay_policy.h").read_text()

file_write_status_test = (root / "trunk/user/rc/src/tests/file_write_status.c").read_text()
file_write_status_header = (root / "trunk/user/shared/src/file_write_status.h").read_text()
process_workflow = (root / ".github/workflows/process-lifecycle-regression.yml").read_text()
xy_workflow = (root / ".github/workflows/xy-c1-firmware-validation.yml").read_text()
fput_string_body = common_ex.split("fput_string(const char *name, const char *value)", 1)[1].split(
    "\nint\nfput_int", 1
)[0]
assert "#include <file_write_status.h>" in common_ex
assert "return fput_string_stream(fp, value);" in fput_string_body
assert "fputs(value, fp)" not in fput_string_body
assert "fclose(fp)" not in fput_string_body
assert "fclose(stream) != 0" in file_write_status_header
assert "fputs(value, stream) == EOF" in file_write_status_header
assert "test_dev_full_reports_buffered_close_failure" in file_write_status_test
assert '"trunk/user/shared/src/file_write_status.h"' in process_workflow
assert '"trunk/user/rc/src/tests/file_write_status.c"' in process_workflow
assert "Compile and run RC file write/close status regression" in process_workflow
print("RC fput_string buffered-write and close-status integration: PASS")

storage_launch_contracts = (
    (
        "int run_dms(int force_rescan)",
        "void restart_dms(int force_rescan)",
        'launch_status = _eval(minidlna_argv, NULL, 0, NULL);',
        'if (!nvram_match("apps_dms", "1"))',
        "Media Server",
        "is_dms_run()",
    ),
    (
        "int run_itunes(void)",
        "void restart_itunes(void)",
        'launch_status = eval("/usr/bin/mt-daapd", "-c", conf_new);',
        'if (!nvram_match("apps_itunes", "1"))',
        "iTunes Server",
        "is_itunes_run()",
    ),
    (
        "int run_torrent(void)",
        "void restart_torrent(void)",
        'launch_status = eval("/usr/bin/transmission.sh", "start");',
        'if (!nvram_match("trmd_enable", "1"))',
        "Transmission",
        "is_torrent_run()",
    ),
    (
        "int run_aria(void)",
        "void restart_aria(void)",
        'launch_status = eval("/usr/bin/aria.sh", "start");',
        'if (!nvram_match("aria_enable", "1"))',
        "Aria",
        "is_aria_run()",
    ),
)
for start_marker, end_marker, launcher, enable_guard, service_name, presence_call in storage_launch_contracts:
    start_body = storage_services.split(start_marker, 1)[1].split(chr(10) + end_marker, 1)[0]
    assert enable_guard in start_body
    assert "int launch_status;" in start_body
    assert "if (process_state == PROCESS_PRESENCE_RUNNING)" in start_body
    assert "if (process_state != PROCESS_PRESENCE_ABSENT)" in start_body
    assert launcher in start_body
    assert "storage_log_daemon_start_result(apps_name, launch_status," in start_body
    assert start_body.index(launcher) < start_body.index("storage_log_daemon_start_result(apps_name, launch_status,")
    post_launch = start_body.split(launcher, 1)[1]
    assert "process_state = " + presence_call + ";" in post_launch
    assert service_name in start_body

start_result = storage_services.split("storage_log_daemon_start_result(", 1)[1].split(
    chr(10) + "#if defined (APP_FTPD)", 1
)[0]
assert "if (launch_status != 0)" in start_result
assert "#if defined (APP_MINIDLNA) || defined (APP_FIREFLY) || defined (APP_TRMD) || defined (APP_ARIA)" in storage_services
assert "PROCESS_PRESENCE_RUNNING" in start_result
assert "PROCESS_PRESENCE_ABSENT" in start_result
assert "cannot verify daemon startup (scan error=%d)" in start_result
assert "errno ? errno : EIO" in start_result
assert '"trunk/user/rc/src/services_stor.c"' in process_workflow
assert '"trunk/user/rc/src/services_stor.c"' in xy_workflow
assert '#include "firefly_config_file.h"' in storage_services
firefly_update = storage_services.split("static int\nupdate_firefly_conf(", 1)[1].split("\nint run_itunes(void)", 1)[0]
assert "return firefly_config_publish(conf_full, conf_full, link_path," in firefly_update
assert "ENAMETOOLONG" in firefly_update
itunes_config = storage_services.split("int run_itunes(void)", 1)[1].split("void restart_itunes(void)", 1)[0]
assert "if (mkdir(conf_path, 0755) != 0 && errno != EEXIST)" in itunes_config
assert "rename(conf_old, conf_new) != 0" in itunes_config
assert "config_status = update_firefly_conf(link_path, conf_path, conf_file);" in itunes_config
assert "configuration publication failed (status=%d)" in itunes_config
assert itunes_config.index("if (config_status != 0)") < itunes_config.index('launch_status = eval("/usr/bin/mt-daapd", "-c", conf_new);')
assert "mkstemp(temporary_path)" in firefly_config_header
assert "fflush(output)" in firefly_config_header
assert "fsync(fileno(output))" in firefly_config_header
assert "rename(temporary_path, path)" in firefly_config_header
assert "unlink(temporary_path)" in firefly_config_header
assert "ferror(source)" in firefly_config_header
assert "last-good-configuration" in firefly_config_test
assert "custom_setting = preserve-me" in firefly_config_test
assert "firefly_config_publish(path, directory" in firefly_config_test
assert "S_ISDIR(st.st_mode)" in firefly_config_test
assert "0600" in firefly_config_test
assert "trunk/user/rc/src/firefly_config_file.h" in process_workflow
assert "trunk/user/rc/src/tests/firefly_config_file.c" in process_workflow
assert "trunk/user/rc/src/firefly_config_file.h" in xy_workflow
print("Firefly configuration publication failure and last-good preservation contracts: PASS")

assert '#include "minidlna_config_file.h"' in storage_services
assert "int update_minidlna_conf(const char *link_path, const char *conf_path);" in rc_header
assert "void update_minidlna_conf(const char *link_path, const char *conf_path);" not in rc_header
assert "int update_minidlna_conf(const char *link_path, const char *conf_path)" in storage_services
minidlna_update = storage_services.split("int update_minidlna_conf(", 1)[1].split("\nint run_dms(int force_rescan)", 1)[0]
assert "minidlna_config_publish(conf_path, &values)" in minidlna_update
assert "configuration publication failed (status=%d)" in minidlna_update
dms_start = storage_services.split("int run_dms(int force_rescan)", 1)[1].split("void restart_dms(int force_rescan)", 1)[0]
assert "config_status = update_minidlna_conf(link_path, conf_path);" in dms_start
assert "if (config_status != 0)" in dms_start
minidlna_launcher = "launch_status = _eval(minidlna_argv, NULL, 0, NULL);"
assert "int launch_status;" in dms_start
assert minidlna_launcher in dms_start
minidlna_post_launch = dms_start.split(minidlna_launcher, 1)[1]
assert "process_state = is_dms_run();" in minidlna_post_launch
minidlna_logger = "storage_log_daemon_start_result(apps_name, launch_status,"
assert minidlna_logger in minidlna_post_launch
assert "(enum process_presence)process_state);" in minidlna_post_launch
assert minidlna_post_launch.index("process_state = is_dms_run();") < minidlna_post_launch.index(minidlna_logger)
assert dms_start.index("if (config_status != 0)") < dms_start.index("_eval(minidlna_argv, NULL, 0, NULL);")
assert "mkstemp(temporary_path)" in minidlna_config_header
assert "fflush(output)" in minidlna_config_header
assert "fsync(fileno(output))" in minidlna_config_header
assert "rename(temporary_path, path)" in minidlna_config_header
assert "unlink(temporary_path)" in minidlna_config_header
assert "minidlna_config_publish(directory_target, &values)" in minidlna_config_test
assert "old-incomplete-config" in minidlna_config_test
assert "0600" in minidlna_config_test
assert "trunk/user/rc/src/minidlna_config_file.h" in process_workflow
assert "trunk/user/rc/src/tests/minidlna_config_file.c" in process_workflow
assert "trunk/user/rc/src/minidlna_config_file.h" in xy_workflow
print("MiniDLNA configuration publication and fail-closed startup contracts: PASS")

mount_link = c_function_body(storage_services, "int create_mp_link(")
assert "storage_nfs_export_parse_mount_line(" in mount_link
assert "read_error = ferror(procpt) ? 1 : 0;" in mount_link
assert "close_status = fclose(procpt);" in mount_link
assert "close_error = close_status != 0;" in mount_link
assert "storage_nfs_export_inventory_complete(read_error, close_error" in mount_link
assert "if (link_created)" in mount_link and "unlink(link_path)" in mount_link
assert "incomplete /proc/mounts inventory; refusing storage link" in mount_link
assert "errno = EINVAL;" in mount_link
assert "errno = storage_nfs_export_errno_or_eio(first_error ? first_error : ENOENT);" in mount_link
assert "return link_created;" not in mount_link
assert "return 1;" in mount_link and "return 0;" in mount_link
mount_status = c_function_body(storage_services, "create_mp_link_status(")
assert "create_mp_link(search_dir, link_path, force_first_valid)" in mount_status
assert "status = storage_nfs_export_errno_or_eio(errno);" in mount_status
assert "errno = status;" in mount_status
assert "return status;" in mount_status
assert storage_services.index("static int create_mp_link_status(") < storage_services.index("int run_dms(int force_rescan)")
for signature in ("int run_dms(int force_rescan)", "int run_itunes(void)",
                  "int run_torrent(void)", "int run_aria(void)"):
    caller = c_function_body(storage_services, signature)
    assert "create_mp_link_status(" in caller
    assert "return link_status;" in caller
    assert "create_mp_link(" not in caller
print("Storage mount-link deterministic errno and media caller propagation contract: PASS")

pppoe_test = (root / "trunk/user/rc/src/tests/pppoe_relay_policy.c").read_text()

assert "process_inventory.o" in shared_makefile
assert '#include "process_inventory.h"' in shared_header
assert "process_inventory_scan(appname, &snapshot)" in pids_source
assert "PROCESS_PRESENCE_UNKNOWN" in pids_source
assert "return EXIT_FAILURE;" in pids_source
assert "return EXIT_SUCCESS;" in pids_source
assert "return count;" not in pids_source

assert "pids_presence(const char *appname)" in pids_source
assert "process_inventory_scan(appname, &snapshot)" in pids_source
assert "pids_presence_legacy_boolean(pids_presence(appname))" in pids_source
assert "presence != PROCESS_PRESENCE_ABSENT" in pids_presence_policy
for marker in (
    "test_running_remains_true",
    "test_confirmed_absence_remains_false",
    "test_unknown_is_never_downgraded_to_absent",
    "test_unrecognized_state_fails_closed",
):
    assert marker in pids_presence_test, marker
assert '#include <process_inventory.h>' in policy
assert "process_inventory_scan(service_name, snapshot)" in common_ex
assert "process_identity_validate(service_name, identity)" in common_ex
assert "kill(identity->pid, signal_number)" in common_ex
assert 'doSystem("killall %s %s", signal_option, service_name)' not in common_ex
assert "service_lifecycle_preflight" in policy
pppoe_confirm = net_source.split("static int\npppoe_relay_confirm_started(int launch_status)", 1)[1].split(
    "\nint\nset_passthrough_pppoe", 1
)[0]
assert "PPPOE_RELAY_START_WAIT_ATTEMPTS 4" in net_source
assert "pppoe_relay_process_presence(&scan_error)" in pppoe_confirm
assert "PROCESS_PRESENCE_RUNNING" in pppoe_confirm
assert "PROCESS_PRESENCE_UNKNOWN" in pppoe_confirm
assert "sleep(1);" in pppoe_confirm
assert "pppoe_relay_start_status(launch_status, presence)" in pppoe_confirm
udpxy_start = net_source.split("int\nstart_udpxy(char *wan_ifname)", 1)[1].split("\n#if defined(APP_XUPNPD)", 1)[0]
assert "if (port < 1024)" in udpxy_start
assert 'launch_status = eval("/usr/bin/msd_lite"' in udpxy_start
assert 'MSD Lite launch failed (status=%d)' in udpxy_start
assert 'launch_status = eval("/usr/sbin/udpxy"' in udpxy_start
assert 'udpxy launch failed (status=%d)' in udpxy_start
assert udpxy_start.count("if (launch_status != 0)") == 2
assert "return launch_status;" in udpxy_start
assert "int start_udpxy(char *wan_ifname);" in rc_header
xupnpd_start = net_source.split("int\nstart_xupnpd(char *wan_ifname)", 1)[1].split("\n#endif", 1)[0]
assert "if (!is_xupnpd_support())" in xupnpd_start
assert "if (xport < 1024)" in xupnpd_start
assert 'launch_status = eval("/usr/bin/xupnpd");' in xupnpd_start
assert 'launch_status = system("/usr/bin/xupnpd &");' in xupnpd_start
assert "launch_status = xupnpd_confirm_started(launch_status);" in xupnpd_start
assert '"xupnpd launch/readiness failed (status=%d errno=%d)"' in xupnpd_start
assert xupnpd_start.index('launch_status = eval("/usr/bin/xupnpd");') < xupnpd_start.index("xupnpd_confirm_started(launch_status);")
assert xupnpd_start.index('launch_status = system("/usr/bin/xupnpd &");') < xupnpd_start.index("xupnpd_confirm_started(launch_status);")
assert "return launch_status;" in xupnpd_start
assert "int start_xupnpd(char *wan_ifname);" in rc_header
assert "if (fp1 == NULL)" in xupnpd_start and "if (fp2 == NULL)" in xupnpd_start
assert '#include "xupnpd_config_file.h"' in net_source
assert "xupnpd_config_valid_interface(wan_ifname)" in xupnpd_start
assert "xupnpd_config_valid_ipv4(nvram_safe_get" in xupnpd_start
assert xupnpd_start.count("xupnpd_config_tempfile_open(tmp2, tmp1, sizeof(tmp1))") == 2
assert 'fopen(tmp1, "w")' not in xupnpd_start
xupnpd_config_header = (root / "trunk/user/rc/src/xupnpd_config_file.h").read_text()
xupnpd_config_test = (root / "trunk/user/rc/src/tests/xupnpd_config_file.c").read_text()
assert "mkstemp(temporary)" in xupnpd_config_header
assert "lstat(destination, &st)" in xupnpd_config_header
assert "fchmod(fd, mode)" in xupnpd_config_header
assert "xupnpd_config_source_open(tmp2)" in xupnpd_start
assert xupnpd_start.count("xupnpd_config_source_open(tmp2)") == 2
assert 'fopen(tmp2, "r")' not in xupnpd_start
assert "lstat(source_path, &path_stat)" in xupnpd_config_header
assert "O_NONBLOCK" in xupnpd_config_header and "O_NOFOLLOW" in xupnpd_config_header
assert "source_stat.st_dev != path_stat.st_dev" in xupnpd_config_header
assert "test_unique_exclusive_temp_and_symlink" not in xupnpd_config_test
assert "must-not-change" in xupnpd_config_test
assert "os.execute" in xupnpd_config_test
assert "test_source_rejects_symlinks_and_special_files" in xupnpd_config_test
assert 'xupnpd_config_directory_validate("/etc/storage")' in xupnpd_start
assert "xupnpd_config_ensure_directory(dir_dst, 0755)" in xupnpd_start
assert xupnpd_start.index("xupnpd_config_ensure_directory(dir_dst, 0755)") < xupnpd_start.index("for (i=0; xdir1[i]; i++)")
assert "xupnpd_config_subdirectory_validate(" in xupnpd_start
assert 'xupnpd_config_directory_validate(tmp2)' in xupnpd_start
assert "lstat(directory_path, &st)" in xupnpd_config_header
assert "S_ISLNK(st.st_mode)" in xupnpd_config_header
assert "S_ISDIR(st.st_mode)" in xupnpd_config_header
assert "test_storage_directory_rejects_symlinks_and_non_directories" in xupnpd_config_test
assert "mkfifo(fifo_path" in xupnpd_config_test

assert "ferror(fp2)" in xupnpd_start and "fflush(fp1)" in xupnpd_start
assert "fsync(fileno(fp1))" in xupnpd_start and "rename(tmp1, tmp2)" in xupnpd_start
assert "unlink(tmp1)" in xupnpd_start
assert xupnpd_start.index("if (has_daemon)") < xupnpd_start.index('launch_status = eval("/usr/bin/xupnpd");')
assert xupnpd_start.index('launch_status = system("/usr/bin/xupnpd &");') < xupnpd_start.index("if (launch_status != 0)")
igmpproxy_start = net_source.split("int\nstart_igmpproxy(char *wan_ifname)", 1)[1].split("\nint\nrestart_iptv", 1)[0]
assert '#include "service_start_status.h"' in net_source
assert "rc_first_service_start_status(start_status, start_udpxy(wan_ifname))" in igmpproxy_start
assert "rc_first_service_start_status(start_status, start_xupnpd(wan_ifname))" in igmpproxy_start
assert "rc_first_service_start_status(start_status, config_status)" in igmpproxy_start
assert 'eval("/bin/igmpproxy", igmpproxy_conf)' in igmpproxy_start
igmpproxy_start = net_source.split("int\nstart_igmpproxy(char *wan_ifname)", 1)[1].split("\n}\n", 1)[0]
assert '#include "igmpproxy_config_file.h"' in net_source
assert "igmpproxy_config_publish(igmpproxy_conf, &config_values)" in igmpproxy_start
assert 'igmpproxy config publication failed (status=%d)' in igmpproxy_start
assert igmpproxy_start.index("igmpproxy_config_publish(") < igmpproxy_start.index('eval("/bin/igmpproxy", igmpproxy_conf)')
assert "return rc_first_service_start_status(start_status," in igmpproxy_start
assert "if (config_status != 0)" in igmpproxy_start
assert "fopen(igmpproxy_conf" not in igmpproxy_start
assert "igmpproxy_config_printf(" in igmpproxy_config_header
assert "igmpproxy_config_token(" in igmpproxy_config_header
assert "igmpproxy_config_altnet_token(" in igmpproxy_config_header
assert "192.0.2.0/24 quickleave" in igmpproxy_config_test
assert "eth0.2 #comment" in igmpproxy_config_test
assert "br0\\tphyint" in igmpproxy_config_test
assert "mkstemp(" in igmpproxy_config_header
assert "fdopen(" in igmpproxy_config_header
assert "fflush(" in igmpproxy_config_header
assert "fsync(" in igmpproxy_config_header
assert "fclose(output)" in igmpproxy_config_header
assert "rename(temporary_path, path)" in igmpproxy_config_header
assert "unlink(temporary_path)" in igmpproxy_config_header
assert "last-good-after-write-failure" in igmpproxy_config_test
assert "target.conf.tmp." in igmpproxy_config_test
assert "quickleave" in igmpproxy_config_test
assert "igmpproxy_config_publish" in igmpproxy_config_test
assert '"trunk/user/rc/src/igmpproxy_config_file.h"' in process_workflow
assert '"trunk/user/rc/src/tests/igmpproxy_config_file.c"' in process_workflow
assert "Compile and run IGMP proxy atomic configuration publication regression" in process_workflow
assert '"trunk/user/rc/src/igmpproxy_config_file.h"' in xy_workflow
pppoe_start = net_source.split("int\nset_passthrough_pppoe(int is_on)", 1)[1].split("set_igmp_mld_version", 1)[0]
assert '#include "process_inventory.h"' in net_source
assert '#include "pppoe_relay_policy.h"' in net_source
pppoe_scan = net_source.split("static enum process_presence\npppoe_relay_process_presence(int *scan_error)", 1)[1].split(
    "\nstatic int\npppoe_relay_confirm_started", 1
)[0]
assert 'process_inventory_scan("pppoe-relay", &snapshot)' in pppoe_scan
assert "*scan_error = snapshot.error;" in pppoe_scan
assert pppoe_scan.index("*scan_error = snapshot.error;") < pppoe_scan.index("process_snapshot_release(&snapshot);")
assert "pppoe_relay_process_presence(&scan_error)" in pppoe_start
assert pppoe_start.index("pppoe_relay_process_presence(&scan_error)") < pppoe_start.index("pppoe_relay_should_start(presence)")
assert "if (pppoe_relay_should_start(presence))" in pppoe_start
assert "return start_status;" in pppoe_start
assert "start_status = pppoe_relay_confirm_started(start_status);" in pppoe_start
assert "launch/readiness failed" in pppoe_start
assert "return scan_error;" in pppoe_start
assert "return kill_services(svcs, 3, 1);" in pppoe_start
assert "int set_passthrough_pppoe(int is_on);" in rc_header
assert "void set_passthrough_pppoe(int is_on);" not in rc_header
assert "if (presence == PROCESS_PRESENCE_RUNNING)" in pppoe_start
assert "if (scan_error <= 0)" in pppoe_start
assert "errno = scan_error;" in pppoe_start
assert "strerror(scan_error)" in pppoe_start
assert "if (!pids(" not in pppoe_start and 'pids("pppoe-relay")' not in pppoe_start
assert pppoe_start.index("if (pppoe_relay_should_start(presence))") < pppoe_start.index('eval("/usr/sbin/pppoe-relay"')
assert "presence == PROCESS_PRESENCE_ABSENT" in pppoe_policy
assert "PROCESS_PRESENCE_UNKNOWN" in pppoe_test and "PROCESS_PRESENCE_RUNNING" in pppoe_test
assert "process inventory failed" in pppoe_start
assert "service_lifecycle_signal_targets" in policy
assert "SERVICE_LIFECYCLE_UNKNOWN" in policy

stop_lpd = usb_services.split("stop_lpd(void)", 1)[1].split("#endif", 1)[0]
assert "kill_services(svcs, 3, 1)" in stop_lpd
assert 'unlink("/var/run/lpdparent.pid")' not in stop_lpd
print("process inventory and lifecycle integration contract: PASS")

lpd_source = (root / "trunk/user/LPRng/src/src/common/lpd.c").read_text()
pid_writer = lpd_source.split("static int update_pidfile(void)", 1)[1].split("static int read_lpd_pidfile_legacy", 1)[0]
assert 'process_pidfile_write("/var/run/lpdparent.pid", "lpd")' in pid_writer
assert "O_CREAT | O_EXCL" not in pid_writer
assert "if (update_pidfile() < 0)" in lpd_source

assert '#include <process_inventory.h>' in lpd_source
assert '#include <process_pidfile.h>' in lpd_source
pid_reader = lpd_source.split("static int read_lpd_pidfile(pid_t *pid)", 1)[1].split("static int lpd_pid_is_running", 1)[0]
assert 'process_pidfile_read_identity("/var/run/lpdparent.pid", "lpd",' in pid_reader
assert "if (errno != EINVAL)" in pid_reader
legacy_reader = lpd_source.split("static int read_lpd_pidfile_legacy(pid_t *pid)", 1)[1].split("static int read_lpd_pidfile(pid_t *pid)", 1)[0]
for token in ("O_NOFOLLOW | O_NONBLOCK | O_CLOEXEC", "fstat(fd, &st)",
              "S_ISREG(st.st_mode)", "st.st_uid != (uid_t)0",
              "st.st_nlink != 1", "(st.st_mode & 022) != 0",
              "lpd_pidfile_parse(buffer, used, pid)"):
    assert token in legacy_reader
pid_policy = (root / "trunk/user/LPRng/src/src/include/lpd_pidfile_policy.h").read_text()
assert "text[i] < '0' || text[i] > '9'" in pid_policy
assert "value <= 1 || value > (unsigned long)INT_MAX" in pid_policy
assert "static int lpd_pid_is_running(pid_t pid)" in lpd_source
assert 'process_inventory_scan("lpd", &snapshot)' in lpd_source
assert 'process_identity_validate("lpd", &snapshot.items[i])' in lpd_source
assert "fscanf(fp, \"%d\", &pid)" not in lpd_source
assert "if (pid == getpid())" in lpd_source
assert lpd_source.index("bind(sockfd") < lpd_source.index("update_pidfile() < 0")
assert lpd_source.index("update_pidfile() < 0") < lpd_source.index("listen(sockfd")


pidfile_source = (root / "trunk/user/shared/src/process_pidfile.c").read_text()
pidfile_header = (root / "trunk/user/shared/src/process_pidfile.h").read_text()
assert "process_pidfile.o" in shared_makefile
assert "process_pidfile_parse_record" in pidfile_source
assert "process_identity_validate(process_name, &snapshot.items[i])" in pidfile_source
assert "process_inventory_scan(process_name, &snapshot)" in pidfile_source
assert "process_pidfile_write" in pidfile_header
assert "process_pidfile_signal" in pidfile_header

detect_internet_source = (root / "trunk/user/rc/src/detect_internet.c").read_text()
httpd_source = (root / "trunk/user/httpd/src/web_ex.c").read_text()
assert "process_pidfile_write(DI_PID_FILE, \"detect_internet\")" in detect_internet_source
compact_detect_internet = "".join(detect_internet_source.split())
assert 'process_pidfile_signal(DI_PID_FILE,"detect_internet",SIGUSR1)' in compact_detect_internet
assert 'process_pidfile_signal(DI_PID_FILE,"detect_internet",SIGUSR2)' in compact_detect_internet
assert "kill_pidfile_s(DI_PID_FILE" not in detect_internet_source
assert 'process_pidfile_signal("/var/run/detect_internet.pid", "detect_internet", SIGHUP)' in httpd_source
assert 'kill_pidfile_s("/var/run/detect_internet.pid"' not in httpd_source

detect_link_source = (root / "trunk/user/rc/src/detect_link.c").read_text()
assert "process_pidfile_write(DL_PID_FILE, \"detect_link\")" in detect_link_source
assert "process_pidfile_signal(DL_PID_FILE, \"detect_link\", SIGUSR1)" in detect_link_source
assert "process_pidfile_signal(DL_PID_FILE, \"detect_link\", SIGHUP)" in detect_link_source
assert "kill_pidfile_s(DL_PID_FILE" not in detect_link_source

rstats_source = (root / "trunk/user/rc/src/rstats.c").read_text()
assert "process_pidfile_write(RSTATS_PID_FILE, \"rstats\")" in rstats_source
assert "process_pidfile_signal(RSTATS_PID_FILE, \"rstats\", SIGHUP)" in rstats_source
assert "kill_pidfile_s(RSTATS_PID_FILE" not in rstats_source
assert 'process_pidfile_signal(RSTATS_PID_FILE, "rstats", sig)' in httpd_source
assert "kill_pidfile_s(RSTATS_PID_FILE" not in httpd_source
assert 'process_pidfile_signal("/var/run/lpdparent.pid", "lpd", SIGUSR2)' in httpd_source
assert 'kill_pidfile_s("/var/run/lpdparent.pid", SIGUSR2)' not in httpd_source
assert 'refusing LPD remove signal because PID identity is unverified' in httpd_source

pppd_policy = (root / "trunk/user/pppd/ppp-2.4.7/pppd/pppd_pidfile_policy.h").read_text()
pppd_main = (root / "trunk/user/pppd/ppp-2.4.7/pppd/main.c").read_text()
pppd_writer = pppd_main.split("static void\ncreate_pidfile(pid)", 1)[1].split("void\ncreate_linkpidfile(pid)", 1)[0]
process_pidfile = (root / "trunk/user/shared/src/process_pidfile.c").read_text()
vpn_server = (root / "trunk/user/rc/src/vpn_server.c").read_text()
assert "pppd_pidfile_parse_start_time" in pppd_policy
assert "pppd_pidfile_format_record" in pppd_policy
assert "read_process_start_time(pid" in pppd_main
assert "write_pidfile_record(pidfilename, (pid_t)pid)" in pppd_writer
assert 'fprintf(pidfile, "%d\\n", pid)' not in pppd_writer
assert "process_pidfile_read_identity" in process_pidfile
assert "O_NOFOLLOW | O_CLOEXEC | O_NONBLOCK" in process_pidfile
assert "process_identity_validate(process_name, &snapshot.items[i])" in process_pidfile
assert 'process_pidfile_read_identity(pidfile, "pppd", &identity)' in vpn_server
assert "pid_text_file" not in vpn_server
print("PPPD PID-file start-time identity integration: PASS")

lifecycle_policy = (root / "trunk/user/rc/src/service_lifecycle_policy.h").read_text()
lifecycle_test = (root / "trunk/user/rc/src/tests/service_lifecycle_policy.c").read_text()
common_ex = (root / "trunk/user/rc/src/common_ex.c").read_text()
vpn_server = (root / "trunk/user/rc/src/vpn_server.c").read_text()
vpn_stop = vpn_server.split("static int\nstop_vpn_server_locked(void)", 1)[1].split("\n}\n", 1)[0]
assert "service_lifecycle_scan_target" in lifecycle_policy
assert "read_pidfile" in lifecycle_policy and "force_only" in lifecycle_policy
assert "service_lifecycle_stop_targets" in lifecycle_policy
assert "process_pidfile_read_identity" in common_ex
assert "kill_services_with_pidfiles" in common_ex
assert "test_pidfile_unknown_blocks_entire_preflight" in lifecycle_test
assert "test_pidfile_identity_change_blocks_escalation" in lifecycle_test
assert "test_force_only_pidfile_target_skips_term_and_joins_kill" in lifecycle_test
assert "kill_services_with_pidfiles(svcs, pidfile_paths, \"pppd\", 1, 5, 1)" in vpn_stop
assert "kill_pidfile_s" not in vpn_stop
assert "stop_status != 0" in vpn_server
print("PPP PID-file all-target stop transaction integration: PASS")

vpn_client = (root / "trunk/user/rc/src/vpn_client.c").read_text()
vpn_client_stop = vpn_client.split("static int\nstop_vpn_client_locked(void)", 1)[1].split("\n}\n", 1)[0]
vpn_client_stop_public = vpn_client.split("int\nstop_vpn_client(void)", 1)[1].split("\n}\n", 1)[0]
vpn_client_force = vpn_client.split("static int\nstop_vpn_client_force(void)", 1)[1].split("\n}\n", 1)[0]
assert "stop_vpn_client_ppp(0)" in vpn_client_stop
assert "if (stop_status != 0)" in vpn_client_stop
assert "vpn_server_lifecycle_lock_acquire()" in vpn_client_stop_public
assert "stop_vpn_client_locked()" in vpn_client_stop_public
assert "vpn_server_lifecycle_lock_release(lock_fd)" in vpn_client_stop_public
vpn_server_locked_wrapper = vpn_server.split(
    "int" + chr(10) + "stop_vpn_server_under_lifecycle_lock(void)", 1
)[1].split(chr(10) + "int" + chr(10) + "stop_vpn_server(void)", 1)[0]
assert "return stop_vpn_server_locked();" in vpn_server_locked_wrapper
rc_header_vpn = (root / "trunk/user/rc/src/rc.h").read_text()
assert "int stop_vpn_server_under_lifecycle_lock(void);" in rc_header_vpn
assert 'process_inventory_scan("xl2tpd", &snapshot)' in vpn_client_stop
assert "PROCESS_PRESENCE_START_ABORT" in vpn_client_stop
assert 'control_xl2tpd("d", "VPNC")' in vpn_client_stop
assert "stop_vpn_client_ppp(1)" in vpn_client_force
assert "process_pidfile_parse_link_record" in process_pidfile
assert "write_link_pidfile_record" in pppd_main
assert "pppd_pidfile_format_link_record" in pppd_policy
net_wan = (root / "trunk/user/rc/src/net_wan.c").read_text()
wan_ppp_stop = net_wan.split("static int\nstop_wan_ppp_locked(void)", 1)[1].split("\n}", 1)[0]
wan_full_stop = net_wan.split("static int\nstop_wan_locked(void)", 1)[1].split("\\n}", 1)[0]
wan_lock_runner = net_wan.split("run_wan_stop_under_vpn_lifecycle_lock(int (*operation)(void), const char *name)", 1)[1].split("\\n}", 1)[0]
for stop_body in (wan_ppp_stop, wan_full_stop):
    assert "stop_vpn_client_under_lifecycle_lock() != 0" in stop_body
    assert "stop_vpn_client() != 0" not in stop_body
    assert "kill_services(svcs_ppp, 6, 1)" in stop_body
assert wan_lock_runner.index("vpn_server_lifecycle_lock_acquire()") < wan_lock_runner.index("result = operation()")
assert wan_lock_runner.index("result = operation()") < wan_lock_runner.index("vpn_server_lifecycle_lock_release(lock_fd)")
assert "run_wan_stop_under_vpn_lifecycle_lock(stop_wan_ppp_locked" in net_wan
assert "run_wan_stop_under_vpn_lifecycle_lock(stop_wan_locked" in net_wan
print("VPN client PPP link PID-file stop and non-reentrant WAN lifecycle integration: PASS")

udhcpc_source = (root / "trunk/user/busybox/busybox-1.24.x/networking/udhcp/dhcpc.c").read_text()
udhcpc_writer = udhcpc_source.split("static int\nwrite_udhcpc_pidfile", 1)[1].split("\n#if BB_MMU", 1)[0]
assert '#include "process_pidfile_record.h"' in udhcpc_source
assert "process_pidfile_record_read_start_time(getpid(), &start_time)" in udhcpc_writer
assert "process_pidfile_record_format(getpid(), start_time" in udhcpc_writer
assert "mkstemp(temp_path)" in udhcpc_writer and "fsync(fd)" in udhcpc_writer
assert "rename(temp_path, path)" in udhcpc_writer
assert "write_pidfile(client_config.pidfile)" not in udhcpc_source
assert "remove_pidfile(client_config.pidfile)" not in udhcpc_source
assert "unlink-by-path can race a new publisher" in udhcpc_source
print("BusyBox udhcpc identity-bound PID-file producer integration: PASS")

services_source = (root / "trunk/user/rc/src/services.c").read_text()
logger_stop = services_source.split("int\nstop_logger(void)", 1)[1].split("\n}", 1)[0]
rc_source = (root / "trunk/user/rc/src/rc.c").read_text()
shutdown = rc_source.split("shutdown_router(int level)", 1)[1].split("\n}\n", 1)[0]
flash = rc_source.split("flash_firmware(void)", 1)[1].split(
    "\nstatic void\nstorage_load_time", 1
)[0]
assert "auth_stop_status = stop_8021x_all();" in flash
assert "int vpn_client_stop_status, vpn_server_stop_status, residual_stop_status;" in flash
assert "stop_vpn_client_under_lifecycle_lock()" in flash
assert "stop_vpn_server_under_lifecycle_lock()" in flash
assert "stop_vpn_client()" not in flash and "stop_vpn_server()" not in flash
assert "if (vpn_client_stop_status != 0 && stop_status == 0)" in flash
assert "if (vpn_server_stop_status != 0 && stop_status == 0)" in flash
assert "if (residual_stop_status != 0 && stop_status == 0)" in flash
assert "VPN client/server or residual L2TP/PPP services did not stop" in flash
assert flash.index("vpn_server_lifecycle_lock_acquire()") < flash.index("stop_vpn_client_under_lifecycle_lock()")
assert flash.index("stop_vpn_client_under_lifecycle_lock()") < flash.index("stop_vpn_server_under_lifecycle_lock()")
assert flash.index("stop_vpn_server_under_lifecycle_lock()") < flash.index("kill_services(svcs, 6, 1)")
assert flash.index("kill_services(svcs, 6, 1)") < flash.index("vpn_server_lifecycle_lock_release(lock_fd)")
assert "int script_status = doSystem(\"%s %d\", script_name, 0);" in flash
assert "if (script_status != 0)" in flash
assert "custom shutdown script failed" in flash
assert flash.index("if (script_status != 0)") < flash.index("stop_igmpproxy(NULL)")
assert "eapol_stop_status = stop_auth_eapol();" in flash
assert "kabinet_stop_status = stop_auth_kabinet();" in flash
assert "pppoe_stop_status = set_passthrough_pppoe(0);" in flash
assert "eapol_stop_status != 0 || kabinet_stop_status != 0" in flash
assert "auth_stop_status != 0 || pppoe_stop_status != 0" in flash
assert "wpa_cli" not in flash and "wpa_supplicant" not in flash
assert flash.index("set_passthrough_pppoe(0)") < flash.index("doSystem(\"%s %d\", script_name, 0)")
assert flash.index("auth_stop_status = stop_8021x_all();") < flash.index("doSystem(\"%s %d\", script_name, 0)")
assert flash.index("stop_igmpproxy(NULL)") < flash.index("stop_misc()")
assert flash.index("stop_misc()") < flash.index("storage_save_time(60)") < flash.index("write_storage_to_mtd()")
stop_misc_failure = flash.split("if (stop_misc() != 0)", 1)[1].split("storage_save_time(60)", 1)[0]
assert "watchdog_status = start_watchdog();" in stop_misc_failure
assert "firmware update abort could not restore watchdog" in stop_misc_failure
assert "firmware write failed and watchdog recovery also failed" in flash
assert "start_watchdog();" in stop_misc_failure
assert 'flash_status = eval("/tmp/mtd_write", "-r", "write", FW_IMG_NAME, FW_MTD_NAME);' in flash
assert "if (flash_status != 0)" in flash
flash_write_failure = flash.split("if (flash_status != 0)", 1)[1]
assert "int flash_errno = errno ? errno : EIO;" in flash_write_failure
assert "watchdog_status = start_watchdog();" in flash_write_failure
assert "errno = flash_errno;" in flash_write_failure
assert "return -1;" in flash_write_failure
assert "return 0;" in flash
syslog_restart = rc_source.split("RCN_RESTART_SYSLOG) == 0)", 1)[1].split("RCN_RESTART_WDG)", 1)[0]
time_restart = rc_source.split("RCN_RESTART_TIME) == 0)", 1)[1].split("RCN_RESTART_SYSCTL)", 1)[0]
assert "return kill_services(svcs, 3, 1);" in logger_stop
assert "int stop_logger(void);" in rc_header
assert "void stop_logger(void);" not in rc_header
assert "logger_stop_status = stop_logger();" in shutdown
assert "logger_stop_status == 0" in shutdown
assert shutdown.count("stop_logger();") == 1
assert "int shutdown_script_status = 0;" in shutdown
assert 'shutdown_script_status = doSystem("%s %d", script_name, level);' in shutdown
assert "if (shutdown_script_status != 0)" in shutdown
assert "custom shutdown script failed" in shutdown
assert "shutdown_script_status == 0" in shutdown
assert shutdown.count('doSystem("%s %d", script_name, level);') == 1
assert shutdown.index("shutdown_script_status = doSystem(") < shutdown.index("safe_remove_all_stor_devices(")
assert shutdown.index("shutdown_script_status = doSystem(") < shutdown.index("if (storage_remove_status == 0")
assert shutdown.index("logger_stop_status = stop_logger();") < shutdown.index("if (storage_remove_status == 0")
assert shutdown.index("if (storage_remove_status == 0") < shutdown.index("write_storage_to_mtd()")
assert "logger_stop_status != 0" in syslog_restart
assert syslog_restart.index("logger_stop_status != 0") < syslog_restart.index("start_logger(0)")
assert "action_result = -1;" in syslog_restart
assert "logger_stop_status == 0 && start_logger(0) != 0" in time_restart
assert "logger_stop_status != 0" in time_restart

logger_policy = (root / "trunk/user/rc/src/logger_start_policy.h").read_text()
logger_test = (root / "trunk/user/rc/src/tests/logger_start_policy.c").read_text()
logger_start = services_source.split("int\nstart_logger(int showinfo)", 1)[1].split("\n}\n", 1)[0]
assert '#include "logger_start_policy.h"' in services_source
assert "syslog_status = start_syslogd();" in logger_start
assert "klog_status = start_klogd();" in logger_start
assert logger_start.index("syslog_status = start_syslogd();") < logger_start.index("klog_status = start_klogd();")
assert 'logmessage(LOGNAME, "syslogd startup failed (status=%d)", syslog_status);' in logger_start
assert 'logmessage(LOGNAME, "klogd startup failed (status=%d)", klog_status);' in logger_start
assert "return rc_logger_start_status(syslog_status, klog_status);" in logger_start
assert "return 0;" not in logger_start
assert "return syslog_status != 0 ? syslog_status : klog_status;" in logger_policy
for contract in (
    "rc_logger_start_status(0, 0) == 0",
    "rc_logger_start_status(17, 0) == 17",
    "rc_logger_start_status(0, 23) == 23",
    "rc_logger_start_status(17, 23) == 17",
    "rc_logger_start_status(-1, 0) == -1",
    "rc_logger_start_status(0, -1) == -1",
):
    assert contract in logger_test
assert '"trunk/user/rc/src/logger_start_policy.h"' in process_workflow
assert '"trunk/user/rc/src/tests/logger_start_policy.c"' in process_workflow
assert '"trunk/user/rc/src/logger_start_policy.h"' in xy_workflow
assert '"trunk/user/rc/src/tests/logger_start_policy.c"' in xy_workflow
assert "Compile and run RC logger startup result policy regression" in process_workflow
print("RC logger startup status propagation and regression integration: PASS")


net_wan = (root / "trunk/user/rc/src/net_wan.c").read_text()
wan_stop = net_wan.split("static int\nstop_wan_locked(void)", 1)[1].split("\n}\n", 1)[0]
net_wan_dhcp = net_wan
net_lan_dhcp = (root / "trunk/user/rc/src/net_lan.c").read_text()
wan_dhcp_stop = net_wan_dhcp.split("static int\nstop_udhcpc_pidfile", 1)[1].split("\nint renew_udhcpc_wan", 1)[0]
lan_stop = net_lan_dhcp.split("int\nstop_lan(int is_ap_mode)", 1)[1].split("\nint\nfull_restart_lan", 1)[0]
lan_restart = net_lan_dhcp.split("int\nfull_restart_lan(void)", 1)[1]
wan_dhcp_noforce_stop = 'kill_services_with_pidfiles(NULL, pidfiles, "udhcpc", 0, 30, 0)'
assert wan_dhcp_noforce_stop in wan_dhcp_stop
assert 'process_pidfile_signal(pidfile, "udhcpc", SIGUSR1)' in net_wan_dhcp
assert 'process_pidfile_signal(pidfile, "udhcpc", SIGUSR2)' in net_wan_dhcp
assert "kill_pidfile(" not in net_wan_dhcp[net_wan_dhcp.index("static int\nstop_udhcpc_pidfile"):]
lan_dhcp_noforce_stop = 'kill_services_with_pidfiles(NULL, pidfiles, "udhcpc", 0, 30, 0)'
lan_daemon_stop = "kill_services(svcs, 3, 1)"
assert "status = stop_udhcpc_lan();" in lan_stop
assert lan_daemon_stop in lan_stop
assert lan_stop.index("notify_pause_detect_internet()") < lan_stop.index("status = stop_udhcpc_lan();")
assert lan_stop.index("status = stop_udhcpc_lan();") < lan_stop.index(lan_daemon_stop)
lan_pidfile_stop = net_lan_dhcp.split("stop_udhcpc_lan(void)", 1)[1].split("\n}", 1)[0]
assert '"/var/run/udhcpc_lan.pid"' in lan_pidfile_stop
assert 'process_pidfile_signal(pidfiles[0], "udhcpc", SIGUSR2)' in lan_pidfile_stop
assert lan_dhcp_noforce_stop in lan_pidfile_stop
assert "errno != ENOENT && errno != ESRCH" in lan_pidfile_stop
lan_abort = lan_stop.index("if (status != 0)")
lan_resume = lan_stop.index("notify_run_detect_internet(2)", lan_abort)
lan_return = lan_stop.index("return status;", lan_resume)
lan_bridge_down = lan_stop.index("ifconfig(IFNAME_BR, 0, NULL, NULL);")
assert lan_abort < lan_resume < lan_return < lan_bridge_down
assert "if (lan_status != 0)" in lan_restart
assert "LAN restart aborted" in lan_restart
print("WAN/LAN udhcpc PID-bound no-force stop and AP teardown-abort integration: PASS")

wan_start = net_wan.split("int\nstart_wan(void)", 1)[1].split(
    "\nstatic int\nstop_wan_ppp_locked", 1
)[0]
wan_up = net_wan.split("wan_up_internal(char *wan_ifname, int unit, int is_static)", 1)[1].split(
    "\nint\nwan_up(", 1
)[0]
wan_down = net_wan.split("wan_down_internal(char *wan_ifname, int unit, int is_static)", 1)[1].split(
    "\nint\nwan_down(", 1
)[0]
assert "auth_status = start_auth_eapol(wan_ifname, unit, wan_auth_mode - 2);" in wan_start
wan_eapol_failure = wan_start.split("if (auth_status != 0)", 1)[1].split("/* We are done configuration */", 1)[0]
assert "set_wan_unit_value_int(unit, \"err\", 1)" in wan_eapol_failure
assert 'ifconfig(wan_ifname, IFUP, "0.0.0.0", NULL)' in wan_eapol_failure
assert "continue;" in wan_eapol_failure
assert wan_start.index("if (auth_status != 0)") < wan_start.index("start_udhcpc_wan(wan_ifname, unit, 0)")
assert "route_status = start_auth_kabinet();" in wan_up
assert "status = route_status_accumulate(status, route_status);" in wan_up
assert "WAN kabinet authentication startup incomplete" in wan_up
assert "route_status = stop_auth_kabinet();" in wan_down
assert "status = route_status_accumulate(status, route_status);" in wan_down
assert "WAN kabinet authentication teardown incomplete" in wan_down
assert "passthrough_status = set_passthrough_pppoe(1);" in wan_start
assert "PPPoE passthrough startup incomplete" in wan_start
assert "stop_status = set_passthrough_pppoe(0);" in wan_stop
assert "refusing WAN interface teardown because PPPoE relay did not stop" in wan_stop
assert wan_stop.index("if (stop_status != 0)") < wan_stop.index("wan_down(man_ifname, unit, 1)")
assert "eapol_stop_status = stop_auth_eapol();" in wan_stop
assert "kabinet_stop_status = stop_auth_kabinet();" in wan_stop
assert "if (eapol_stop_status != 0 || kabinet_stop_status != 0)" in wan_stop
assert wan_stop.index("if (eapol_stop_status != 0 || kabinet_stop_status != 0)") < wan_stop.index(
    "set_passthrough_pppoe(0);"
)

wan_stop = net_wan.split("static int\nstop_wan_locked(void)", 1)[1].split("\n}\n", 1)[0]
usb_modem = (root / "trunk/user/rc/src/usb_modem.c").read_text()
modem_remove = usb_modem.split("int\nsafe_remove_usb_modem(void)", 1)[1].split("\n}\n", 1)[0]
assert '"udhcpc"' not in wan_stop.split("char* svcs_wan[]", 1)[1].split("};", 1)[0]
assert "process_pidfile_signal(udhcpc_pidfiles[0], \"udhcpc\", SIGUSR2)" in wan_stop
assert "process_pidfile_signal(udhcpc_pidfiles[1], \"udhcpc\", SIGUSR2)" in wan_stop
wan_dhcp_stop = 'kill_services_with_pidfiles(NULL, udhcpc_pidfiles, "udhcpc", 0, 30, 0)'
wan_daemon_stop = "kill_services(svcs_wan, 3, 1)"
assert wan_dhcp_stop in wan_stop
assert wan_daemon_stop in wan_stop
assert wan_stop.index('process_pidfile_signal(udhcpc_pidfiles[0], "udhcpc", SIGUSR2)') < wan_stop.index(wan_dhcp_stop)
assert wan_stop.index('process_pidfile_signal(udhcpc_pidfiles[1], "udhcpc", SIGUSR2)') < wan_stop.index(wan_dhcp_stop)
assert wan_stop.index(wan_dhcp_stop) < wan_stop.index(wan_daemon_stop)
assert "killall %s %s\", \"-SIGUSR2\", \"udhcpc\"" not in wan_stop
assert "stop_udhcpc_wan(0)" in modem_remove
assert '"/var/run/ppp-wan0.pid"' in modem_remove
assert "kill_services_with_pidfiles(NULL, pidfiles, \"pppd\", 0, 10, 1)" in modem_remove
assert "set_usb_modem_dev_wan(0, 0)" in modem_remove
assert modem_remove.index("if (stop_status != 0)") < modem_remove.index("set_usb_modem_dev_wan(0, 0)")
print("WAN and USB modem DHCP/PPP identity-bound teardown integration: PASS")

usbnet_stop = usb_modem.split("int\nstop_wan_usbnet(void)", 1)[1].split("\n}\n", 1)[0]
usbnet_stop_compact = " ".join(usbnet_stop.split())
assert "control_required = 0;" in usbnet_stop_compact
assert "ndis_control_network(ndis_ifname, modem_devnum, 0, &control_required)" in usbnet_stop_compact
assert "modem_ndis_control_failed(control_required, control_status)" in usbnet_stop_compact
assert 'ifdown_status = ifconfig(ndis_ifname, 0, "0.0.0.0", NULL)' in usbnet_stop
assert "ifdown_status != ENODEV" in usbnet_stop
assert 'is_usbnet_has_module(ndis_ifname, "cdc_mbim")' in usbnet_stop
assert "int  safe_remove_usb_modem(void);" in rc_header
assert "int  stop_wan_usbnet(void);" in rc_header

usb_modem_guard = usb_modem.split(
    "static int\nusb_modem_require_no_active_process(const char *process_name)", 1
)[1].split("\n}\n", 1)[0]
assert "process_inventory_scan(process_name, &snapshot)" in usb_modem_guard
assert "scan_error = snapshot.error;" in usb_modem_guard
assert usb_modem_guard.index("scan_error = snapshot.error;") < usb_modem_guard.index(
    "process_snapshot_release(&snapshot);"
)
assert usb_modem_guard.index("process_snapshot_release(&snapshot);") < usb_modem_guard.index(
    "if (presence == PROCESS_PRESENCE_ABSENT)"
)
assert "PROCESS_PRESENCE_RUNNING" in usb_modem_guard
assert "EBUSY" in usb_modem_guard and "scan_error ? scan_error : EIO" in usb_modem_guard

modem_remove = usb_modem.split("int\nsafe_remove_usb_modem(void)", 1)[1].split("\n}\n", 1)[0]
assert 'usb_modem_require_no_active_process("usb_modeswitch")' in modem_remove
assert 'usb_modem_require_no_active_process("eject")' in modem_remove
assert modem_remove.index('usb_modem_require_no_active_process("usb_modeswitch")') < modem_remove.index(
    "stop_udhcpc_wan(0)"
)
assert "killall" not in modem_remove
assert "stop_status = stop_wan_usbnet();" in modem_remove
assert "if (stop_status != 0)" in modem_remove
assert modem_remove.index("return stop_status;") < modem_remove.index("set_usb_modem_dev_wan(0, 0)")
assert "return 0;" in modem_remove

usb_remove = usb_services.split("void\nsafe_remove_usb_device(int port, const char *dev_name)", 1)[1].split("\n}\n", 1)[0]
assert "modem_remove_status = safe_remove_usb_modem();" in usb_remove
assert "if (modem_remove_status == 0)" in usb_remove
assert usb_remove.index("if (modem_remove_status == 0)") < usb_remove.index("try_wan_reconnect(0, 0)")
assert "skipping WAN reconnect after incomplete USB modem removal" in usb_remove

usbnet_stop_pos = wan_stop.index("stop_status = stop_wan_usbnet();")
wan_ifdown_pos = wan_stop.index("ifdown_status = wan_teardown_ifdown_interfaces(")
wan_failure_pos = wan_stop.index("if (stop_status != 0 || ifdown_status != 0)")
wan_clear_pos = wan_stop.index("clear_wan_state();")
assert usbnet_stop_pos < wan_ifdown_pos < wan_failure_pos < wan_clear_pos
assert "goto abort_resume_detect;" in wan_stop[wan_failure_pos:]
print("USB modem stop-result propagation and WAN restart gate: PASS")


lan_stop_transaction = net_lan_dhcp.split("int\nstop_lan(int is_ap_mode)", 1)[1].split("\n}\n", 1)[0]
lan_pause = lan_stop_transaction.index("notify_pause_detect_internet()")
lan_preflight = lan_stop_transaction.index("status = stop_udhcpc_lan();")
lan_preflight_failure = lan_stop_transaction.index("if (status != 0)", lan_preflight)
lan_preflight_resume = lan_stop_transaction.index("notify_run_detect_internet(2)", lan_preflight_failure)
lan_preflight_return = lan_stop_transaction.index("return status;", lan_preflight_failure)
lan_ifdown = lan_stop_transaction.index("ifconfig(IFNAME_BR, 0, NULL, NULL)")
lan_ifdown_resume = lan_stop_transaction.index("notify_run_detect_internet(2)", lan_ifdown)
lan_dhcp_stop = net_lan_dhcp.split("stop_udhcpc_lan(void)", 1)[1].split("\n}\n", 1)[0]
lan_release = lan_dhcp_stop.index('process_pidfile_signal(pidfiles[0], "udhcpc", SIGUSR2)')
lan_identity_gate = lan_dhcp_stop.index("errno != ENOENT && errno != ESRCH")
lan_release_wait = lan_dhcp_stop.index("usleep(300000)")
lan_noforce_stop = lan_dhcp_stop.index('kill_services_with_pidfiles(NULL, pidfiles, "udhcpc", 0, 30, 0)')
assert lan_release < lan_identity_gate < lan_release_wait < lan_noforce_stop
assert lan_pause < lan_preflight < lan_preflight_failure < lan_preflight_resume < lan_preflight_return < lan_ifdown
assert lan_ifdown < lan_ifdown_resume
assert "if (status != 0 && is_ap_mode)" in lan_stop_transaction
print("AP LAN detect_internet pause rollback on failed teardown: PASS")

lan_restart_transaction = net_lan_dhcp.split("int\nfull_restart_lan(void)", 1)[1].split("\n}\n", 1)[0]
lan_vpn_stop = lan_restart_transaction.index("vpn_status = stop_vpn_server()")
lan_vpn_failure = lan_restart_transaction.index("if (vpn_status != 0)", lan_vpn_stop)
lan_vpn_abort = lan_restart_transaction.index("return vpn_status;", lan_vpn_failure)
lan_first_other_stop = lan_restart_transaction.index('lan_restart_record_status(&restart_status, "stop LLTD", stop_lltd())')
assert lan_vpn_stop < lan_vpn_failure < lan_vpn_abort < lan_first_other_stop
assert lan_restart_transaction.count("stop_vpn_server();") == 1
lan_dhcp_failure = lan_restart_transaction.index("if (lan_status != 0)")
lan_dns_unlock = lan_restart_transaction.index("dnsmasq_lifecycle_lock_release(dnsmasq_lock)", lan_dhcp_failure)
lan_vpn_restore = lan_restart_transaction.index('lan_restart_record_status(&restart_status, "restore VPN after aborted LAN restart",', lan_dns_unlock)
assert lan_dhcp_failure < lan_dns_unlock < lan_vpn_restore
print("LAN restart VPN fail-closed stop and rollback integration: PASS")

wan_stop_transaction = net_wan.split("static int\nstop_wan_locked(void)", 1)[1].split("\n}\n", 1)[0]
wan_ppp_stop_transaction = net_wan.split("static int\nstop_wan_ppp_locked(void)", 1)[1].split("\n}\n", 1)[0]
assert "abort_resume_detect:" in wan_stop_transaction
assert "notify_run_detect_internet(2)" in wan_stop_transaction
assert "goto abort_resume_detect;" in wan_stop_transaction
assert "abort_resume_detect:" in wan_ppp_stop_transaction
assert "notify_run_detect_internet(2)" in wan_ppp_stop_transaction
assert "goto abort_resume_detect;" in wan_ppp_stop_transaction
print("WAN PPP/WAN teardown detect_internet pause rollback: PASS")

xy_workflow = (root / ".github/workflows/xy-c1-firmware-validation.yml").read_text()
xy_target_sources = (
    "trunk/user/busybox/busybox-1.24.x/networking/udhcp/dhcpc.c",
    "trunk/user/pppd/ppp-2.4.7/pppd/main.c",
    "trunk/user/pppd/ppp-2.4.7/pppd/pppd_pidfile_policy.h",
    "trunk/user/rc/src/common_ex.c",
    "trunk/user/rc/src/net.c",
    "trunk/user/rc/src/pppoe_relay_policy.h",
    "trunk/user/rc/src/net_lan.c",
    "trunk/user/rc/src/net_wan.c",
    "trunk/user/rc/src/rc.c",
    "trunk/user/rc/src/service_lifecycle_policy.h",
    "trunk/user/rc/src/services_ex.c",
    "trunk/user/rc/src/dnsmasq_reload_policy.h",
    "trunk/user/rc/src/usb_modem.c",
    "trunk/user/rc/src/vpn_client.c",
    "trunk/user/rc/src/vpn_server.c",
    "trunk/user/rc/src/inicd.c",
    "trunk/user/rc/src/net_wifi.c",
    "trunk/user/rc/src/detect_wan.c",
    "trunk/user/rc/src/detect_wan_udhcpc_policy.h",
    "trunk/user/shared/src/process_pidfile.c",
    "trunk/user/shared/src/process_pidfile.h",
    "trunk/user/shared/src/process_pidfile_record.h",
)
for path in xy_target_sources:
    assert '"{}"'.format(path) in xy_workflow, (
        "XY-C1 target-build workflow does not watch {}".format(path)
    )
print("XY-C1 MIPS/uClibc source-trigger coverage: PASS")

wan_source = (root / "trunk/user/rc/src/detect_wan.c").read_text()
wan_policy = (root / "trunk/user/rc/src/detect_wan_udhcpc_policy.h").read_text()
wan_test = (root / "trunk/user/rc/src/tests/detect_wan_udhcpc_policy.c").read_text()
process_workflow = (root / ".github/workflows/process-lifecycle-regression.yml").read_text()

wan_renew = wan_source.split("static int\nchk_udhcpc(int ap_mode)", 1)[1].split("\n}\n", 1)[0]
wan_poll = wan_source.split("static int\npoll_gateway(void)", 1)[1].split("\n}\n", 1)[0]
assert "process_inventory_scan(\"udhcpc\", &snapshot)" in wan_renew
assert "detect_wan_udhcpc_should_renew(presence)" in wan_renew
assert "scan_error = snapshot.error;" in wan_renew
assert wan_renew.index("scan_error = snapshot.error;") < wan_renew.index("process_snapshot_release(&snapshot);")
assert wan_renew.index("process_snapshot_release(&snapshot);") < wan_renew.index('process_pidfile_signal(detect_wan_udhcpc_pidfile(ap_mode),')
assert "if (renew_decision < 0)" in wan_renew
assert "errno = scan_error ? scan_error : EIO;" in wan_renew
assert "if (renew_decision == 0)" in wan_renew
assert 'process_pidfile_signal(detect_wan_udhcpc_pidfile(ap_mode), "udhcpc", SIGUSR1)' in " ".join(wan_renew.split())
assert 'doSystem("killall %s %s", "-SIGUSR1", "udhcpc")' not in wan_renew
assert 'return ap_mode ? "/var/run/udhcpc_lan.pid" : "/var/run/udhcpc0.pid";' in wan_policy
assert 'detect_wan_udhcpc_pidfile(0), "/var/run/udhcpc0.pid"' in wan_test
assert 'detect_wan_udhcpc_pidfile(1), "/var/run/udhcpc_lan.pid"' in wan_test
assert 'pids("udhcpc")' not in wan_renew
assert "return -1;" in wan_renew
assert "renew_status = chk_udhcpc(ap_mode);" in wan_poll
assert "if (renew_status != 0)" in wan_poll
assert wan_poll.index("if (renew_status != 0)") < wan_poll.index("return -1;")
assert all(state in wan_policy for state in ("PROCESS_PRESENCE_RUNNING", "PROCESS_PRESENCE_ABSENT", "PROCESS_PRESENCE_UNKNOWN"))
assert "errno = EIO;" in wan_policy and "return -1;" in wan_policy
assert "PROCESS_PRESENCE_UNKNOWN) == -1" in wan_test
assert "PROCESS_PRESENCE_ABSENT) == 0" in wan_test
assert "PROCESS_PRESENCE_RUNNING) == 1" in wan_test
assert "WAN DHCP renewal tri-state policy: PASS" in wan_test
for path in (
    "trunk/user/rc/src/detect_wan.c",
    "trunk/user/rc/src/net.c",
    "trunk/user/rc/src/pppoe_relay_policy.h",
    "trunk/user/rc/src/tests/pppoe_relay_policy.c",
    "trunk/user/rc/src/detect_wan_udhcpc_policy.h",
    "trunk/user/rc/src/tests/detect_wan_udhcpc_policy.c",
):
    assert '"{}"'.format(path) in process_workflow, "process workflow missing {}".format(path)
assert "WAN DHCP renewal presence policy regression" in process_workflow

dnsmasq_services = (root / "trunk/user/rc/src/services_ex.c").read_text()
dnsmasq_notify_policy = (root / "trunk/user/rc/src/dnsmasq_reload_policy.h").read_text()
dnsmasq_notify_test = (root / "trunk/user/rc/src/tests/dnsmasq_reload_policy.c").read_text()
wan_resolver = (root / "trunk/user/rc/src/net_wan.c").read_text()
dnsmasq_status_body = dnsmasq_services.split("dnsmasq_process_status(int *scan_error)", 1)[1].split(
    "\nint\nis_dhcpd_enabled", 1
)[0]
resolver_update_body = wan_resolver.split("update_resolvconf(int is_first_run, int do_not_notify)", 1)[1].split(
    "\nstatic void\nrefresh_resolvconf_nonfatal_internal", 1
)[0]
assert "dnsmasq_reload_notification_decision" in dnsmasq_status_body
assert "errno = (scan_error && *scan_error > 0) ? *scan_error : EIO;" in dnsmasq_status_body
assert "dnsmasq_reload_notification_decision(enum process_presence presence)" in dnsmasq_notify_policy
assert "PROCESS_PRESENCE_RUNNING" in dnsmasq_notify_policy
assert "PROCESS_PRESENCE_ABSENT" in dnsmasq_notify_policy
assert "return -1;" in dnsmasq_notify_policy
assert "dnsmasq_process_status(&dnsmasq_scan_error)" in resolver_update_body
assert "if (dnsmasq_status < 0)" in resolver_update_body
assert "if (dnsmasq_status > 0)" in resolver_update_body
assert "is_dns_dhcpd_run()" not in resolver_update_body
assert "resolver file published but DNSMasq process state is unknown" in resolver_update_body
assert "dnsmasq_reload_notification_decision(PROCESS_PRESENCE_RUNNING) == 1" in dnsmasq_notify_test
assert "dnsmasq_reload_notification_decision(PROCESS_PRESENCE_ABSENT) == 0" in dnsmasq_notify_test
assert "dnsmasq_reload_notification_decision(PROCESS_PRESENCE_UNKNOWN) == -1" in dnsmasq_notify_test
dnsmasq_workflow = (root / ".github/workflows/dnsmasq-lifecycle.yml").read_text()
for path in (
    "trunk/user/rc/src/net_wan.c",
    "trunk/user/rc/src/services_ex.c",
    "trunk/user/rc/src/rc.h",
    "trunk/user/rc/src/dnsmasq_reload_policy.h",
    "trunk/user/rc/src/tests/dnsmasq_reload_policy.c",
):
    assert '"{}"'.format(path) in dnsmasq_workflow, "DNSMasq workflow missing {}".format(path)
assert "Compile and run DNSMasq reload identity policy regression" in dnsmasq_workflow
print("DNSMasq resolver notification tri-state integration: PASS")

presence_policy = (root / "trunk/user/rc/src/process_presence_start_policy.h").read_text()
process_inventory_header = (root / "trunk/user/shared/src/process_inventory.h").read_text()
presence_test = (root / "trunk/user/rc/src/tests/process_presence_start_policy.c").read_text()
assert "process_presence_start_postcondition" in presence_policy
assert "PROCESS_PRESENCE_START_PROBE_ATTEMPTS 4" in presence_policy
assert "process_presence_start_postcondition(0, PROCESS_PRESENCE_RUNNING)" in presence_test
assert "process_presence_start_postcondition(0, PROCESS_PRESENCE_UNKNOWN)" in presence_test
detect_internet_source = (root / "trunk/user/rc/src/detect_internet.c").read_text()
detect_link_source = (root / "trunk/user/rc/src/detect_link.c").read_text()
di_notify = detect_internet_source.split(
    "void\nnotify_run_detect_internet(int delay_time)", 1
)[1].split("\n}\n", 1)[0]
dl_notify = detect_link_source.split(
    "void\nnotify_reset_detect_link(void)", 1
)[1].split("\n}\n", 1)[0]
wan_gateway_start = net_wan.split(
    "/* Start the gateway ARP checker only after a complete ABSENT scan. */", 1
)[1].split("if (wan_gate)", 1)[0]
for source in (detect_internet_source, detect_link_source, net_wan):
    assert '#include "process_presence_start_policy.h"' in source
    assert '#include <process_inventory.h>' in source
for body, process_name in ((di_notify, "detect_internet"), (dl_notify, "detect_link")):
    assert 'process_inventory_scan("{}"'.format(process_name) in body
    assert "scan_error = snapshot.error;" in body
    assert body.index("scan_error = snapshot.error;") < body.index("process_snapshot_release(&snapshot);")
    assert body.index("process_snapshot_release(&snapshot);") < body.index("process_presence_start_action_for_state(presence)")
    assert "PROCESS_PRESENCE_START_NEW" in body
    assert "PROCESS_PRESENCE_START_NOTIFY" in body
    assert "skipping start/notification because process inventory is unknown" in body
    assert "pids(" not in body
assert 'process_inventory_scan("detect_wan", &snapshot)' in wan_gateway_start
assert "scan_error = snapshot.error;" in wan_gateway_start
assert wan_gateway_start.index("scan_error = snapshot.error;") < wan_gateway_start.index("process_snapshot_release(&snapshot);")
assert wan_gateway_start.index("process_snapshot_release(&snapshot);") < wan_gateway_start.index("process_presence_start_action_for_state(presence)")
assert "PROCESS_PRESENCE_START_NEW" in wan_gateway_start
assert "PROCESS_PRESENCE_START_ABORT" in wan_gateway_start
assert 'pids("detect_wan")' not in wan_gateway_start
assert "PROCESS_PRESENCE_START_ABORT" in presence_policy
assert "PROCESS_PRESENCE_UNKNOWN" in process_inventory_header
for token in ("PROCESS_PRESENCE_ABSENT", "PROCESS_PRESENCE_RUNNING",
              "PROCESS_PRESENCE_UNKNOWN", "(enum process_presence)7"):
    assert token in presence_test
assert "Process presence start/notify and detached-daemon postcondition policy: PASS" in presence_test
for path in (
    "trunk/user/rc/src/detect_internet.c",
    "trunk/user/rc/src/detect_link.c",
    "trunk/user/rc/src/net_wan.c",
    "trunk/user/rc/src/process_presence_start_policy.h",
    "trunk/user/rc/src/tests/process_presence_start_policy.c",
):
    assert '"{}"'.format(path) in process_workflow, "process workflow missing {}".format(path)
    assert '"{}"'.format(path) in xy_workflow, "XY-C1 workflow missing {}".format(path)
print("detect_internet/detect_link/WAN gateway tri-state start-gate integration: PASS")

wifi_source = (root / "trunk/user/rc/src/net_wifi.c").read_text()
rc_source = (root / "trunk/user/rc/src/rc.c").read_text()
start_8021x_wl = wifi_source.split("int\nstart_8021x_wl(void)", 1)[1].split("\n}", 1)[0]
start_8021x_rt = wifi_source.split("int\nstart_8021x_rt(void)", 1)[1].split("\n}", 1)[0]
assert 'start_status = eval("rt2860apd", "-i", IFNAME_5G_MAIN);' in start_8021x_wl
assert 'if (start_status != 0)' in start_8021x_wl
assert '802.1X 5GHz authenticator launch failed (status=%d)' in start_8021x_wl
assert start_8021x_wl.index('start_status = eval("rt2860apd"') < start_8021x_wl.index(
    '802.1X 5GHz authenticator launch failed'
)
assert 'start_status = eval("rtinicapd", "-i", IFNAME_2G_MAIN);' in start_8021x_rt
assert 'if (start_status != 0)' in start_8021x_rt
assert '802.1X 2.4GHz authenticator launch failed (status=%d)' in start_8021x_rt
assert "return start_status;" in start_8021x_wl
assert "return start_status;" in start_8021x_rt
services_source = (root / "trunk/user/rc/src/services.c").read_text()
boot_services = services_source.split("start_services_once(int is_ap_mode)", 1)[1].split("\n}", 1)[0]
wl_boot = boot_services.split("int wl_auth_start_status = start_8021x_wl();", 1)[1].split(
    "int rt_auth_start_status = start_8021x_rt();", 1
)[0]
rt_boot = boot_services.split("int rt_auth_start_status = start_8021x_rt();", 1)[1].split(
    "start_status = rc_first_service_start_status(start_status, start_httpd(0));", 1
)[0]
for band, block, stop_auth, stop_radio, status_var in (
    ("5GHz", wl_boot, "stop_8021x_wl()", "stop_wifi_all_wl()", "wl_auth_start_status"),
    ("2.4GHz", rt_boot, "stop_8021x_rt()", "stop_wifi_all_rt()", "rt_auth_start_status"),
):
    assert f"{band} boot startup aborted: 802.1X authenticator launch failed" in block
    assert f"if ({status_var} != 0)" in block
    assert block.index(f"if ({status_var} != 0)") < block.index(stop_auth) < block.index(stop_radio)
    assert block.index(stop_radio) < block.index(
        f"rc_first_service_start_status(start_status, {status_var})"
    ), f"{band} boot status must be aggregated after rollback is attempted"
watchdog_start = services_source.split("int" + chr(10) + "start_watchdog_cpu(void)", 1)[1].split(chr(10) + "}", 1)[0]
common_source = (root / "trunk/user/rc/src/common_ex.c").read_text()
module_load_start = common_source.split("int" + chr(10) + "module_smart_load(", 1)[1].split(
    chr(10) + "}", 1
)[0]
assert "if (is_module_loaded(module_name))" in module_load_start
assert "return 0;" in module_load_start
assert "return (ret == 0) ? 1 : 0;" in module_load_start
assert watchdog_start.index('module_smart_load("rt_timer_wdg", NULL)') < watchdog_start.index(
    'module_status = is_module_loaded_status("rt_timer_wdg")'
)
assert "if (!module_smart_load(" not in watchdog_start
assert "if (module_status < 0)" in watchdog_start
assert "return probe_error;" in watchdog_start
assert "if (module_status != 1)" in watchdog_start
assert "CPU watchdog module startup failed" in watchdog_start
assert "return EIO;" in watchdog_start and "return 0;" in watchdog_start
assert "is_module_loaded_status(const char *module_name)" in common_source
assert "return is_module_loaded_status(module_name) > 0;" in common_source
assert "rc_first_service_start_status(start_status, start_watchdog_cpu())" in boot_services
assert start_8021x_rt.index('start_status = eval("rtinicapd"') < start_8021x_rt.index(
    '802.1X 2.4GHz authenticator launch failed'
)
stop_8021x_wl = wifi_source.split("int\nstop_8021x_wl(void)", 1)[1].split("\n}", 1)[0]
stop_8021x_rt = wifi_source.split("int\nstop_8021x_rt(void)", 1)[1].split("\n}", 1)[0]
stop_8021x_all = wifi_source.split("int\nstop_8021x_all(void)", 1)[1].split("\n}", 1)[0]
assert "return kill_services(svcs, 3, 1);" in stop_8021x_wl
assert "return 0;" in stop_8021x_wl
assert "return kill_services(svcs, 3, 1);" in stop_8021x_rt
assert "return kill_services(svcs, 3, 1);" in stop_8021x_all
restart_wifi_wl = wifi_source.split("restart_wifi_wl(int radio_on, int need_reload_conf)", 1)[1].split(
    "\nint\nrestart_wifi_rt", 1
)[0]
restart_wifi_rt = wifi_source.split("restart_wifi_rt(int radio_on, int need_reload_conf)", 1)[1].split(
    "\nint\n", 1
)[0]
assert "if (stop_8021x_wl() != 0)" in restart_wifi_wl
assert "if (need_reload_conf && stop_8021x_rt() != 0)" in restart_wifi_wl
assert restart_wifi_wl.index("if (stop_8021x_wl() != 0)") < restart_wifi_wl.index(
    'nvram_set_int_temp("reload_svc_wl", 1)'
)
assert "if (stop_8021x_rt() != 0)" in restart_wifi_rt
assert "if (need_reload_conf && stop_8021x_wl() != 0)" in restart_wifi_rt
assert restart_wifi_rt.index("if (stop_8021x_rt() != 0)") < restart_wifi_rt.index(
    'nvram_set_int_temp("reload_svc_rt", 1)'
)
assert "DBDC rollback could not stop the 5GHz 802.1X daemon" in restart_wifi_wl
assert "DBDC rollback could not stop the 2.4GHz 802.1X daemon" in restart_wifi_rt
assert "auth_start_status = start_8021x_wl();" in restart_wifi_wl
assert "auth_start_status = start_8021x_rt();" in restart_wifi_rt
assert "wireless restart aborted: 802.1X authenticator launch failed" in restart_wifi_wl
assert "wireless restart aborted: 802.1X authenticator launch failed" in restart_wifi_rt
for band, block in (("5GHz", restart_wifi_wl), ("2.4GHz", restart_wifi_rt)):
    assert 'int iappd_restart_status = system("/usr/bin/iappd.sh restart");' in block
    assert "if (iappd_restart_status != 0)" in block
    assert "Ralink IAPP restart after " + band + " wireless restart failed (wait status=%d)" in block
    assert "isolation_status = wireless_radio_restart_status(" in block
    assert "iappd_restart_status);" in block
    assert block.count('system("/usr/bin/iappd.sh restart")') == 1
    assert block.index("int iappd_restart_status = system(") < block.index("if (iappd_restart_status != 0)")
    assert block.index("if (iappd_restart_status != 0)") < block.index("wireless_radio_restart_status(") < block.index("return isolation_status;")
radio_policy_test = (root / "trunk/user/rc/src/tests/wireless_radio_policy.c").read_text()
radio_policy_header = (root / "trunk/user/rc/src/wireless_radio_policy.h").read_text()
assert "wireless_radio_restart_status(int previous_status, int operation_status)" in radio_policy_header
assert "test_restart_failure_wait_status_is_normalized_and_sticky" in radio_policy_test
assert "wireless_radio_restart_status(0, 256) >= 0" in radio_policy_test
assert "wireless_radio_restart_status(-EACCES, 256) != -EACCES" in radio_policy_test
shutdown_router = rc_source.split("shutdown_router(int level)", 1)[1].split("\n}\n", 1)[0]
assert "auth_stop_status = stop_8021x_all();" in shutdown_router
assert "auth_stop_status == 0" in shutdown_router
assert "int wifi_wl_stop_status = 0, wifi_rt_stop_status = 0;" in shutdown_router
assert "wifi_wl_stop_status = stop_wifi_all_wl();" in shutdown_router
assert "wifi_rt_stop_status = stop_wifi_all_rt();" in shutdown_router
assert "wifi_wl_stop_status != 0" in shutdown_router
assert "wifi_rt_stop_status != 0" in shutdown_router
assert "wifi_wl_stop_status == 0" in shutdown_router
assert "wifi_rt_stop_status == 0" in shutdown_router
assert "int lan_stop_status = 0;" in shutdown_router
assert "lan_stop_status = stop_lan(is_ap_mode);" in shutdown_router
assert "if (lan_stop_status != 0)" in shutdown_router
assert "lan_stop_status == 0" in shutdown_router
assert 'LAN shutdown incomplete; continuing power-down without storage persistence' in shutdown_router
assert shutdown_router.count("stop_lan(is_ap_mode);") == 1
mtd_gate_pos = shutdown_router.index("if (storage_remove_status == 0")
assert shutdown_router.index("wifi_wl_stop_status = stop_wifi_all_wl();") < mtd_gate_pos
assert shutdown_router.index("wifi_rt_stop_status = stop_wifi_all_rt();") < mtd_gate_pos
lan_stop_pos = shutdown_router.index("lan_stop_status = stop_lan(is_ap_mode);")
assert shutdown_router.index("wifi_wl_stop_status = stop_wifi_all_wl();") < lan_stop_pos < mtd_gate_pos
assert lan_stop_pos < shutdown_router.index("rwfs_unmount_status = umount_rwfs_partition();")
assert shutdown_router.index("if (lan_stop_status != 0)") < mtd_gate_pos
assert shutdown_router.count("stop_lan(is_ap_mode);") == 1
assert shutdown_router.count("stop_wifi_all_wl();") == 1
assert shutdown_router.count("stop_wifi_all_rt();") == 1
assert shutdown_router.index("auth_stop_status = stop_8021x_all();") < shutdown_router.index(
    "auth_stop_status == 0"
)
assert '"trunk/user/rc/src/net_wifi.c"' in process_workflow
assert '"trunk/user/rc/src/rc.c"' in process_workflow

auth_source = (root / "trunk/user/rc/src/auth.c").read_text()
auth_stop_kabinet = auth_source.split("int stop_auth_kabinet(void)", 1)[1].split("\n}", 1)[0]
auth_stop_eapol = auth_source.split("int stop_auth_eapol(void)", 1)[1].split("\n}", 1)[0]
auth_start_kabinet = auth_source.split("int start_auth_kabinet(void)", 1)[1].split("\n}", 1)[0]
auth_start_eapol = auth_source.split("int start_auth_eapol(", 1)[1].split("\n}", 1)[0]
assert "return kill_services(svcs, 3, 1);" in auth_stop_kabinet
assert "return kill_services(svcs, 3, 1);" in auth_stop_eapol
assert "ret = stop_auth_kabinet();" in auth_start_kabinet
assert "if (ret != 0)" in auth_start_kabinet
assert auth_start_kabinet.index("ret = stop_auth_kabinet();") < auth_start_kabinet.index("if (ret != 0)") < auth_start_kabinet.index('eval("/usr/sbin/lanauth"')
assert "ret = stop_auth_eapol();" in auth_start_eapol
assert "if (ret != 0)" in auth_start_eapol
assert auth_start_eapol.index("ret = stop_auth_eapol();") < auth_start_eapol.index("if (ret != 0)") < auth_start_eapol.index('fopen(wpa_conf, "w")')
assert '"trunk/user/rc/src/auth.c"' in process_workflow
assert '"trunk/user/rc/src/auth.c"' in xy_workflow

watchdog_source = (root / "trunk/user/rc/src/watchdog.c").read_text()
httpd_probe = watchdog_source.split("static int\nhttpd_check_v2()", 1)[1].split(
    "\nstatic int\nrefresh_ntp(void)", 1
)[0]
busybox_config = (root / "trunk/configs/boards/busybox.config").read_text()
assert 'wget -q -T 1 http://127.0.0.1:%d/httpd_check.htm' in httpd_probe
assert 'pids("wget")' not in httpd_probe
assert 'killall wget' not in httpd_probe
assert "CONFIG_FEATURE_WGET_TIMEOUT=y" in busybox_config
watchdog_httpd = watchdog_source.split("static void httpd_process_check(void)", 1)[1].split(
    "/* Sometimes, dnsmasq crashed, try to re-run it */", 1
)[0]
watchdog_dnsmasq = watchdog_source.split("static void\ndnsmasq_process_check(void)", 1)[1].split(
    "\n}\n", 1
)[0]
watchdog_start = watchdog_source.split("int\nstart_watchdog(void)", 1)[1].split(
    "\n}\n", 1
)[0]
watchdog_notify = watchdog_source.split("void\nnotify_watchdog_time(void)", 1)[1].split(
    "\n}\n", 1
)[0]
for body, service in ((watchdog_httpd, "httpd"), (watchdog_dnsmasq, "dnsmasq"),
                      (watchdog_start, "watchdog")):
    assert 'process_inventory_scan("{}", &snapshot)'.format(service) in body
    assert "scan_error = snapshot.error;" in body
    assert body.index("scan_error = snapshot.error;") < body.index("process_snapshot_release(&snapshot);")
    assert body.index("process_snapshot_release(&snapshot);") < body.index("process_presence_start_action_for_state(presence)")
    assert "pids(" not in body
assert "is_dns_dhcpd_run()" not in watchdog_dnsmasq
assert "PROCESS_PRESENCE_START_ABORT" in watchdog_httpd
assert "restart_status = restart_httpd();" in watchdog_httpd
assert watchdog_httpd.count("restart_httpd();") == 1
assert "if (restart_status != 0)" in watchdog_httpd
assert "HTTPD recovery restart failed" in watchdog_httpd
assert "stop_httpd(" not in watchdog_httpd
assert "start_httpd(0)" not in watchdog_httpd
assert "killall -9 httpd" not in watchdog_httpd
assert "system(" not in watchdog_httpd

# Watchdog recovery must use the single-lock stop/start transaction, rather
# than reacquiring the process-associated record lock between public calls.
httpd_restart = c_function_body(services_source, "restart_httpd(void)")
assert 'httpd_lifecycle_lock_acquire_status("restart", &lock_fd)' in httpd_restart
assert httpd_restart.index("stop_httpd_locked();") < httpd_restart.index(
    "httpd_restart_can_start(stop_status)"
) < httpd_restart.index("start_httpd_locked(1);")
assert "httpd_lifecycle_lock_finish(lock_fd, status, \"restart\")" in httpd_restart
assert "PROCESS_PRESENCE_START_ABORT" in watchdog_dnsmasq
assert "PROCESS_PRESENCE_START_NEW" in watchdog_dnsmasq
assert "dnsmasq_missing = 0;" in watchdog_dnsmasq
assert "PROCESS_PRESENCE_START_NOTIFY" in watchdog_start
assert "PROCESS_PRESENCE_START_NEW" in watchdog_start
assert "PROCESS_PRESENCE_START_ABORT" in watchdog_start
assert "if (action == PROCESS_PRESENCE_START_NEW)" in watchdog_start
assert "launch_status = eval(\"/sbin/watchdog\")" in watchdog_start
assert "if (launch_status != 0)" in watchdog_start
assert "watchdog launcher failed (status=%d)" in watchdog_start
assert "return launch_status;" in watchdog_start
assert watchdog_start.index("if (action == PROCESS_PRESENCE_START_NEW)") < watchdog_start.index(
    "launch_status = eval(\"/sbin/watchdog\")"
) < watchdog_start.index("if (launch_status != 0)") < watchdog_start.index("return launch_status;")
assert "refusing watchdog start because process-presence policy is invalid" in watchdog_start
assert "start_watchdog()" in watchdog_notify
assert "pids(" not in watchdog_notify
assert "process_pidfile_signal(WD_PID_FILE, \"watchdog\", SIGHUP)" in watchdog_notify
assert '"trunk/user/rc/src/watchdog.c"' in process_workflow
assert '"trunk/user/rc/src/watchdog.c"' in xy_workflow
assert '"trunk/user/rc/src/process_presence_start_policy.h"' in process_workflow
assert '"trunk/user/rc/src/tests/process_presence_start_policy.c"' in process_workflow
print("watchdog process inventory tri-state recovery/start integration: PASS")


lltd_start = services_source.split("int\nstart_lltd(void)", 1)[1].split("\n}", 1)[0]
lltd_launcher = 'start_status = eval("/bin/lld2d", IFNAME_BR);'
assert 'if (nvram_invmatch("lltd_enable", "1"))' in lltd_start
assert lltd_start.index("return 1;") < lltd_start.index('optional_daemon_start_status("lld2d", "lld2d")')
assert "if (start_status > 0)" in lltd_start and "if (start_status < 0)" in lltd_start
assert lltd_launcher in lltd_start
assert 'logmessage(LOGNAME, "LLTD daemon launch failed (status=%d)", start_status);' in lltd_start
assert "process_presence_start_postcondition" in lltd_start
assert 'optional_daemon_presence("lld2d", &scan_error)' in lltd_start
assert "PROCESS_PRESENCE_START_INITIAL_WAIT_SECONDS" in lltd_start
assert "PROCESS_PRESENCE_START_PROBE_ATTEMPTS" in lltd_start
assert "PROCESS_PRESENCE_START_RETRY_WAIT_SECONDS" in lltd_start
assert "process_state == PROCESS_PRESENCE_UNKNOWN" in lltd_start
assert "errno = scan_error > 0 ? scan_error : EIO;" in lltd_start
assert "errno = ETIMEDOUT;" in lltd_start
lltd_preflight = lltd_start.index('optional_daemon_start_status("lld2d", "lld2d")')
lltd_launch_at = lltd_start.index(lltd_launcher)
lltd_wait_at = lltd_start.index("sleep(PROCESS_PRESENCE_START_INITIAL_WAIT_SECONDS);", lltd_launch_at)
lltd_probe_at = lltd_start.index("for (attempt = 0; attempt < PROCESS_PRESENCE_START_PROBE_ATTEMPTS; attempt++)", lltd_launch_at)
lltd_inventory_at = lltd_start.index('optional_daemon_presence("lld2d", &scan_error)', lltd_probe_at)
lltd_postcondition_at = lltd_start.index("process_presence_start_postcondition(start_status, process_state)", lltd_inventory_at)
assert lltd_preflight < lltd_launch_at < lltd_wait_at < lltd_probe_at < lltd_inventory_at < lltd_postcondition_at
assert "return process_presence_start_postcondition(start_status," in lltd_start

rstats_start = services_source.split("int\nstart_rstats(void)", 1)[1].split("\n}", 1)[0]
rstats_launcher = 'start_status = eval("/sbin/rstats");'
assert 'if (nvram_invmatch("rstats_enable", "1"))' in rstats_start
assert rstats_start.index("return 1;") < rstats_start.index('optional_daemon_start_status("rstats", "rstats")')
assert "if (start_status > 0)" in rstats_start and "if (start_status < 0)" in rstats_start
assert rstats_launcher in rstats_start
assert 'logmessage(LOGNAME, "rstats daemon launch failed (status=%d)", start_status);' in rstats_start
assert "process_presence_start_postcondition" in rstats_start
assert 'optional_daemon_presence("rstats", &scan_error)' in rstats_start
assert "PROCESS_PRESENCE_START_INITIAL_WAIT_SECONDS" in rstats_start
assert "PROCESS_PRESENCE_START_PROBE_ATTEMPTS" in rstats_start
assert "PROCESS_PRESENCE_START_RETRY_WAIT_SECONDS" in rstats_start
assert "process_state == PROCESS_PRESENCE_UNKNOWN" in rstats_start
assert "errno = scan_error > 0 ? scan_error : EIO;" in rstats_start
assert "errno = ETIMEDOUT;" in rstats_start
rstats_preflight = rstats_start.index('optional_daemon_start_status("rstats", "rstats")')
rstats_launch_at = rstats_start.index(rstats_launcher)
rstats_wait_at = rstats_start.index("sleep(PROCESS_PRESENCE_START_INITIAL_WAIT_SECONDS);", rstats_launch_at)
rstats_probe_at = rstats_start.index("for (attempt = 0; attempt < PROCESS_PRESENCE_START_PROBE_ATTEMPTS; attempt++)", rstats_launch_at)
rstats_inventory_at = rstats_start.index('optional_daemon_presence("rstats", &scan_error)', rstats_probe_at)
rstats_postcondition_at = rstats_start.index("process_presence_start_postcondition(start_status, process_state)", rstats_inventory_at)
assert rstats_preflight < rstats_launch_at < rstats_wait_at < rstats_probe_at < rstats_inventory_at < rstats_postcondition_at
assert "return process_presence_start_postcondition(start_status," in rstats_start

rstats_main = c_function_body(rstats_source, "int rstats_main(int argc, char *argv[])")
assert rstats_main.index("if (daemon(0, 0) < 0)") < rstats_main.index("if (alloc_rstats() < 0)")
assert rstats_main.index("if (alloc_rstats() < 0)") < rstats_main.index('process_pidfile_write(RSTATS_PID_FILE, "rstats")')
assert rstats_main.index('process_pidfile_write(RSTATS_PID_FILE, "rstats")') < rstats_main.index("while (!stopping)")

lldt_main = c_function_body(lldt_main_source, "main(int argc, char **argv)")
lldt_daemon = c_function_body(lldt_osl_source, "osl_become_daemon(osl_t *osl)")
assert lldt_main.index("if (!opt_debug)") < lldt_main.index("osl_become_daemon(g_osl);")
assert lldt_main.index("osl_become_daemon(g_osl);") < lldt_main.index("osl_write_pidfile(g_osl);") < lldt_main.index("event_mainloop();")
assert lldt_daemon.count("fork()") == 2 and "setsid()" in lldt_daemon
assert 'g_Progname = p+1;' in lldt_main_source
assert '"/var/run/%s-%.10s.pid", g_Progname, osl->responder_if' in lldt_osl_source
assert '"trunk/user/lldt/src/main.c"' in process_workflow
assert '"trunk/user/lldt/src/osl-linux.c"' in process_workflow
assert '"trunk/user/rc/src/rstats.c"' in process_workflow

assert '"trunk/user/rc/src/services.c"' in process_workflow
assert '"trunk/user/rc/src/services.c"' in xy_workflow
print("LLTD/rstats detached startup postconditions and daemonization contracts: PASS")

iappd_boot = services_source.split("int\nstart_services_once(int is_ap_mode)", 1)[1].split("\n}\n", 1)[0]
iappd_restart_at = iappd_boot.index('int iappd_restart_status = system("/usr/bin/iappd.sh restart");')
tproxy_load_at = iappd_boot.index('int tproxy_module_status = system("modprobe xt_TPROXY");')
iappd_test_at = iappd_boot.index('int iappd_test_status = system("/usr/bin/iappd.sh test");')
assert "Ralink IAPP restart command failed (wait status=%d)" in iappd_boot
assert "xt_TPROXY module load command failed (wait status=%d)" in iappd_boot
assert "Ralink IAPP post-start test failed (wait status=%d)" in iappd_boot
assert iappd_restart_at < tproxy_load_at < iappd_test_at
assert "if (iappd_restart_status != 0)" in iappd_boot
assert "if (tproxy_module_status != 0)" in iappd_boot
assert "if (iappd_test_status != 0)" in iappd_boot
assert not re.search(r'(?m)^\s*system\("/usr/bin/iappd\.sh restart"\);$', iappd_boot)
assert not re.search(r'(?m)^\s*system\("/usr/bin/iappd\.sh test"\);$', iappd_boot)
assert not re.search(r'(?m)^\s*system\("modprobe xt_TPROXY"\);$', iappd_boot)
print("IAPPD RC restart, TPROXY load and post-start test status propagation: PASS")

optional_boot_calls = (
    ("APP_SKIPDBV2", 'int skipdb_start_status = doSystem("/usr/sbin/skipd -d /etc/storage/db");',
     "skipdb_start_status", "SkipDBv2 boot startup failed (wait status=%d)"),
    ("APP_SCUT", "int scut_start_status = start_scutclient();",
     "scut_start_status", "SCUT client boot startup failed (status=%d)"),
    ("APP_DNSFORWARDER", "int dnsforwarder_start_status = start_dnsforwarder();",
     "dnsforwarder_start_status", "DNS Forwarder boot startup failed (status=%d)"),
    ("APP_VLMCSD", "int vlmcsd_start_status = start_vlmcsd();",
     "vlmcsd_start_status", "vlmcsd boot startup failed (status=%d)"),
    ("APP_IPERF3", "int iperf3_start_status = start_iperf3();",
     "iperf3_start_status", "iperf3 boot startup failed (status=%d)"),
    ("APP_MENTOHUST", "int mentohust_start_status = start_mentohust();",
     "mentohust_start_status", "MentoHUST boot startup failed (status=%d)"),
)
optional_call_positions = []
for feature, assignment, status_name, diagnostic in optional_boot_calls:
    feature_at = iappd_boot.index("#if defined(" + feature + ")")
    call_at = iappd_boot.index(assignment)
    check_at = iappd_boot.index("if (" + status_name + " != 0)", call_at)
    log_at = iappd_boot.index(diagnostic, check_at)
    assert feature_at < call_at < check_at < log_at
    assert "logmessage(LOGNAME," in iappd_boot[check_at:log_at]
    optional_call_positions.append(call_at)
assert optional_call_positions == sorted(optional_call_positions)
standalone_boot_calls = {line.strip() for line in iappd_boot.splitlines()}
for statement in (
    'doSystem("/usr/sbin/skipd -d /etc/storage/db");',
    "start_scutclient();",
    "start_dnsforwarder();",
    "start_vlmcsd();",
    "start_iperf3();",
    "start_mentohust();",
):
    assert statement not in standalone_boot_calls
print("Optional RC daemon startup status propagation and feature guards: PASS")

telnetd_start = services_source.split("int\nstart_telnetd(void)", 1)[1].split("\n}\n", 1)[0]
telnetd_impl = (root / "trunk/user/busybox/busybox-1.24.x/networking/telnetd.c").read_text()
assert "if (!(opt & OPT_FOREGROUND))" in telnetd_impl
assert "bb_daemonize_or_rexec(0 /*was DAEMON_CHDIR_ROOT*/, argv);" in telnetd_impl
assert telnetd_impl.index("bb_daemonize_or_rexec(0") < telnetd_impl.index("create_and_bind_stream_or_die")
assert 'if (!nvram_match("telnetd", "1"))' in telnetd_start
assert telnetd_start.index("return 0;") < telnetd_start.index('optional_daemon_start_status("telnetd", "telnetd")')
telnetd_start_compact = " ".join(telnetd_start.split())
assert 'logmessage(LOGNAME, "telnetd start aborted: process inventory unavailable (status=%d)", start_status);' in telnetd_start_compact
assert 'start_status = eval("telnetd");' in telnetd_start
assert 'logmessage(LOGNAME, "telnetd launcher failed (status=%d)", start_status);' in telnetd_start
assert "process_presence_start_postcondition" in telnetd_start
assert "PROCESS_PRESENCE_START_INITIAL_WAIT_SECONDS" in telnetd_start
assert "PROCESS_PRESENCE_START_PROBE_ATTEMPTS" in telnetd_start
assert 'optional_daemon_presence("telnetd", &scan_error)' in telnetd_start
assert "process_state == PROCESS_PRESENCE_UNKNOWN" in telnetd_start
assert "errno = scan_error > 0 ? scan_error : EIO;" in telnetd_start
assert "errno = ETIMEDOUT;" in telnetd_start
telnetd_launch = telnetd_start.index('start_status = eval("telnetd");')
telnetd_initial_wait = telnetd_start.index("sleep(PROCESS_PRESENCE_START_INITIAL_WAIT_SECONDS);", telnetd_launch)
telnetd_probe = telnetd_start.index("for (attempt = 0; attempt < PROCESS_PRESENCE_START_PROBE_ATTEMPTS; attempt++)", telnetd_launch)
telnetd_inventory = telnetd_start.index('optional_daemon_presence("telnetd", &scan_error)', telnetd_probe)
telnetd_postcondition = telnetd_start.index("process_presence_start_postcondition(start_status, process_state)", telnetd_inventory)
assert telnetd_launch < telnetd_initial_wait < telnetd_probe < telnetd_inventory < telnetd_postcondition

ttyd_start = services_source.split("int\nstart_ttyd(void)", 1)[1].split("\n}\n", 1)[0]
assert "int start_status, attempt, scan_error;" in ttyd_start
assert "enum process_presence process_state;" in ttyd_start
assert 'if (ttyd_mode != 1)' in ttyd_start
assert ttyd_start.index("return 0;") < ttyd_start.index('optional_daemon_start_status("ttyd", "ttyd")')
ttyd_start_compact = " ".join(ttyd_start.split())
assert 'logmessage(LOGNAME, "ttyd start aborted: process inventory unavailable (status=%d)", start_status);' in ttyd_start_compact
assert 'start_status = eval("/usr/bin/ttyd.sh", "start");' in ttyd_start
assert 'logmessage(LOGNAME, "ttyd launcher failed (status=%d)", start_status);' in ttyd_start
assert "process_presence_start_postcondition" in ttyd_start
assert "PROCESS_PRESENCE_START_INITIAL_WAIT_SECONDS" in ttyd_start
assert "PROCESS_PRESENCE_START_PROBE_ATTEMPTS" in ttyd_start
assert "PROCESS_PRESENCE_START_RETRY_WAIT_SECONDS" in ttyd_start
assert 'optional_daemon_presence("ttyd", &scan_error)' in ttyd_start
assert "process_state == PROCESS_PRESENCE_UNKNOWN" in ttyd_start
assert "errno = scan_error > 0 ? scan_error : EIO;" in ttyd_start
assert "errno = ETIMEDOUT;" in ttyd_start
ttyd_launch = ttyd_start.index('start_status = eval("/usr/bin/ttyd.sh", "start");')
ttyd_launch_failure = ttyd_start.index("if (start_status != 0)", ttyd_launch)
ttyd_launcher_return = ttyd_start.index("return process_presence_start_postcondition(start_status,", ttyd_launch_failure)
ttyd_initial_wait = ttyd_start.index("sleep(PROCESS_PRESENCE_START_INITIAL_WAIT_SECONDS);", ttyd_launch)
ttyd_probe = ttyd_start.index("for (attempt = 0; attempt < PROCESS_PRESENCE_START_PROBE_ATTEMPTS; attempt++)", ttyd_initial_wait)
ttyd_inventory = ttyd_start.index('optional_daemon_presence("ttyd", &scan_error)', ttyd_probe)
ttyd_postcondition = ttyd_start.index("process_presence_start_postcondition(start_status, process_state)", ttyd_inventory)
ttyd_unknown = ttyd_start.index("if (process_state == PROCESS_PRESENCE_UNKNOWN)", ttyd_postcondition)
ttyd_timeout = ttyd_start.index("errno = ETIMEDOUT;", ttyd_unknown)
assert ttyd_launch < ttyd_launch_failure < ttyd_launcher_return < ttyd_initial_wait < ttyd_probe < ttyd_inventory < ttyd_postcondition < ttyd_unknown < ttyd_timeout
assert ttyd_start.count("return process_presence_start_postcondition(start_status,") == 2
print("telnetd/ttyd detached-child startup postconditions and disabled sentinels: PASS")

optional_start_contracts = (
    ("start_adguardhome", "AdGuardHome"),
    ("start_ddnsto", "DDNS.to"),
    ("start_adbyby", "Adbyby"),
    ("start_aliddns", "AliDDNS"),
)
for function_name, log_label in optional_start_contracts:
    optional_start = services_source.split(function_name + "(void)", 1)[1].split("\n}", 1)[0]
    eval_at = optional_start.index("start_status = eval(")
    assert "int start_status;" in optional_start
    assert re.search(r"\bif\s*\(", optional_start[:eval_at])  # retain the original enabled/disabled guard
    failure_guard = optional_start.index("if (start_status != 0)", eval_at)
    failure_log = optional_start.index(f'logmessage(LOGNAME, "{log_label} launcher failed (status=%d)", start_status);', failure_guard)
    assert eval_at < failure_guard < failure_log
print("optional AdGuardHome/DDNS.to/Adbyby/AliDDNS launch diagnostics: PASS")

# Adbyby rule updates are RC notification actions too: propagate eval()'s
# wait status through PID 1 instead of discarding it in a void wrapper.
update_adb_body = services_source.split("int update_adb(void)", 1)[1].split("\n}", 1)[0]
assert 'return eval("/usr/bin/adbyby.sh","updateadb");' in update_adb_body
assert "int update_adb(void);" in rc_header
update_adb_notify = rc_source.split("RCN_RESTART_UPDATEADB) == 0)", 1)[1].split("\n\t\t}", 1)[0]
assert "rc_notify_merge_action_result(action_result, update_adb())" in update_adb_notify
print("Adbyby update action result propagation: PASS")


# MSD Lite config publication must finish before enabling or launching the daemon.
msd_config_header = (root / "trunk/user/rc/src/msd_lite_config_file.h").read_text()
msd_config_test = (root / "trunk/user/rc/src/tests/msd_lite_config_file.c").read_text()
udpxy_start = net_source.split("int\nstart_udpxy(char *wan_ifname)", 1)[1].split(
    "\n#if defined(APP_XUPNPD)", 1
)[0]
msd_lite_branch = udpxy_start.split("#if defined (APP_MSD_LITE)", 1)[1].split("#else", 1)[0]
assert '#include "msd_lite_config_file.h"' in net_source
assert "msd_lite_config_publish(src_conf, dst_conf, wan_ifname, port)" in msd_lite_branch
assert "if (config_status != 0)" in msd_lite_branch
assert msd_lite_branch.index("if (config_status != 0)") < msd_lite_branch.index('nvram_set("msd_lite_enable", "1")')
assert msd_lite_branch.index("if (config_status != 0)") < msd_lite_branch.index('eval("/usr/bin/msd_lite"')
assert 'fopen(dst_conf, "w")' not in msd_lite_branch
assert 'doSystem("cp -f %s %s", src_conf, dst_conf)' not in msd_lite_branch
assert "fopen(source_path, \"r\")" in msd_config_header
assert "fstat(fileno(source), &source_stat)" in msd_config_header
assert "lstat(destination_path, &destination_stat)" in msd_config_header
assert "mkstemp(temporary_path)" in msd_config_header
assert "fchmod(fd, mode)" in msd_config_header
assert "ferror(source)" in msd_config_header
assert "fflush(output)" in msd_config_header
assert "fsync(fileno(output))" in msd_config_header
assert "fclose(source)" in msd_config_header
assert "fclose(output)" in msd_config_header
assert "rename(temporary_path, destination_path)" in msd_config_header
assert "unlink(temporary_path)" in msd_config_header
assert "last-good-configuration" in msd_config_test
assert "RLIMIT_FSIZE" in msd_config_test
assert "symlink(symlink_target, symlink_path)" in msd_config_test
assert "eth3.2\\nquickleave" in msd_config_test
assert '"trunk/user/rc/src/msd_lite_config_file.h"' in process_workflow
assert '"trunk/user/rc/src/tests/msd_lite_config_file.c"' in process_workflow
assert '"trunk/user/rc/src/msd_lite_config_file.h"' in xy_workflow
assert '"trunk/user/rc/src/tests/msd_lite_config_file.c"' in xy_workflow
assert "Compile and run MSD Lite atomic configuration publication regression" in process_workflow
print("MSD Lite atomic config publication and launch gate integration: PASS")


service_start_policy = (root / "trunk/user/rc/src/service_start_status.h").read_text()
wireless_module_load_policy = (root / "trunk/user/rc/src/wireless_module_load_policy.h").read_text()
wireless_module_load_test = (root / "trunk/user/rc/src/tests/wireless_module_load_policy.c").read_text()
module_rc_source = (root / "trunk/user/rc/src/rc.c").read_text()
module_net_source = (root / "trunk/user/rc/src/net.c").read_text()
module_net_lan_source = (root / "trunk/user/rc/src/net_lan.c").read_text()
wireless_loader = rc_source.split("static int" + chr(10) + "load_wireless_module(", 1)[1].split(
    chr(10) + "#if defined (USE_USB_SUPPORT)", 1
)[0]
wireless_bulk_loader = wireless_loader.split(
    "static int" + chr(10) + "load_wireless_modules(void)", 1
)[1].split(chr(10) + "}", 1)[0]

service_start_test = (root / "trunk/user/rc/src/tests/service_start_status.c").read_text()
boot_start = services_source.split("int\nstart_services_once(int is_ap_mode)", 1)[1].split("\n}\n", 1)[0]
assert '#include "service_start_status.h"' in services_source
for call in (
    "start_httpd(0)", "start_telnetd()", "start_sshd()", "start_watchdog()",
    "start_gpio_btn()", "start_infosvr()", "start_upnp()", "start_lltd()",
    "start_udpxy(IFNAME_BR)", "start_xupnpd(IFNAME_BR)",
    "start_crond()", "start_networkmap(1)", "start_rstats()",
):
    assert call in boot_start
for call in (
    "start_httpd(0)", "start_telnetd()", "start_sshd()", "start_watchdog()",
    "start_gpio_btn()", "start_upnp()",
    "start_udpxy(IFNAME_BR)", "start_xupnpd(IFNAME_BR)",
):
    assert re.search(
        r"rc_first_service_start_status\(start_status,\s*" +
        re.escape(call) + r"\)",
        boot_start,
    ), "boot service status aggregation missing for " + call

ap_mode_start = boot_start.split("} else {", 1)[1].split("\n\t}", 1)[0]
assert "rc_first_service_start_status(start_status," in ap_mode_start
assert "start_udpxy(IFNAME_BR)" in ap_mode_start
assert "start_xupnpd(IFNAME_BR)" in ap_mode_start

# networkmap's status is captured for diagnostics before it is aggregated.
assert "int networkmap_start_status = start_networkmap(1);" in boot_start
assert 'logmessage(LOGNAME,' in boot_start
assert "rc_first_service_start_status(start_status, networkmap_start_status)" in boot_start
for service, flag in (
    ("start_infosvr()", "adsc_enable"),
    ("start_lltd()", "lltd_enable"),
    ("start_crond()", "crond_enable"),
    ("start_rstats()", "rstats_enable"),
):
    assert 'if (nvram_match("' + flag + '", "1"))' in boot_start
    assert "rc_first_service_start_status(start_status, " + service + ")" in boot_start
assert "return start_status;" in boot_start
assert "return first_status != 0 ? first_status : next_status;" in service_start_policy
for contract in (
    "rc_first_service_start_status(0, 0) == 0",
    "rc_first_service_start_status(0, -1) == -1",
    "rc_first_service_start_status(0, 17) == 17",
    "rc_first_service_start_status(-1, 0) == -1",
    "rc_first_service_start_status(17, -1) == 17",
    "rc_first_service_start_status(-5, 9) == -5",
    "rc_first_service_start_status(4, 0) == 4",
):
    assert contract in service_start_test
assert '#include "wireless_module_load_policy.h"' in rc_source
assert "(void)ops->load(module_name, module_param);" in wireless_module_load_policy
assert "loaded = ops->is_loaded(module_name);" in wireless_module_load_policy
assert "if (loaded < 0)" in wireless_module_load_policy
assert "return -loaded;" in wireless_module_load_policy
assert "return loaded == 1 ? 0 : EIO;" in wireless_module_load_policy
assert "return rc_module_load_verified(module_name, NULL, ops);" in wireless_module_load_policy
assert "if (!module_name || module_name[0] == '\\0' ||" in wireless_module_load_policy
module_loaders = (
    c_function_body(module_net_source, "reload_nat_modules(void)"),
    c_function_body(module_rc_source, "load_boot_module(const char *module_name, const char *module_param)"),
    c_function_body(module_rc_source, "load_wireless_module(const char *module_name)"),
    c_function_body(module_net_lan_source, "init_bridge_load_module_verified(const char *module_name, const char *module_param)"),
)
for loader_body in module_loaders:
    loader_code = " ".join(loader_body.split())
    assert "module_smart_load, is_module_loaded_status" in loader_code
    assert "module_smart_load, is_module_loaded)" not in loader_code
assert '"trunk/user/rc/src/net.c"' in process_workflow
assert '"trunk/user/rc/src/net_lan.c"' in process_workflow
assert '"trunk/user/rc/src/net.c"' in xy_workflow
assert '"trunk/user/rc/src/net_lan.c"' in xy_workflow
assert "rc_module_load_verified(module_name, NULL, &ops)" in wireless_loader
assert "module_smart_load(module_name, NULL)" not in wireless_loader
assert "is_module_loaded(module_name)" not in wireless_loader
assert "wireless driver module load failed or module is unavailable" in wireless_loader
assert "return status;" in wireless_loader
for module_name in (
    "rt2860v2_ap", "mt7628_ap", "mt76x2_ap", "mt_7615e",
    "rt3090_ap", "rt5392_ap", "mt76x3_ap", "rt5592_ap",
    "rt3593_ap", "mt7610_ap",
):
    assert 'load_wireless_module("' + module_name + '")' in wireless_bulk_loader
    assert "rc_first_service_start_status(start_status, load_wireless_module(" in wireless_bulk_loader
assert "return start_status;" in wireless_bulk_loader
assert "boot_modules_status = load_wireless_modules();" in rc_source
assert "Router initialization continuing with one or more unavailable hardware modules" in rc_source
wireless_boot_load = rc_source.index("boot_modules_status = load_wireless_modules();")
for module_group in (
    "boot_modules_status, load_mmc_modules());",
    "boot_modules_status, load_usb_modules());",
    "boot_modules_status, load_ata_modules());",
    "boot_modules_status, load_crypto_modules());",
    "boot_modules_status, load_ipset_modules());",
):
    assert wireless_boot_load < rc_source.index(module_group), module_group
assert "int boot_modules_status, boot_services_status;" in rc_source
normalized_rc_source = " ".join(rc_source.split())
assert "int rwfs_mount_status;" in rc_source
assert "rwfs_mount_status = mount_rwfs_partition();" in rc_source
assert (
    "boot_services_status = rc_first_service_start_status("
    " rwfs_mount_status, boot_modules_status);"
) in normalized_rc_source
assert "int service_start_status = start_services_once(is_ap_mode);" in rc_source
assert "rc_first_service_start_status(boot_services_status," in rc_source
for test_case in (
    "test_already_loaded_is_success",
    "test_newly_loaded_is_success",
    "test_failed_modprobe_is_not_success",
    "test_visible_module_state_wins_over_loader_status",
    "test_load_probe_error_is_reported",
    "test_required_module_set_probe_error_attempts_all_and_keeps_first_failure",
    "test_module_parameter_is_forwarded",
    "test_invalid_inputs_fail_before_side_effects",
    "test_unload_initial_probe_error_fails_closed",
    "test_unload_postcondition_probe_error_is_reported",
):
    assert test_case in wireless_module_load_test
assert "Compile and run RC wireless module post-load verification regression" in process_workflow
assert "test_unload_initial_probe_error_fails_closed" in wireless_module_load_test
assert "test_unload_postcondition_probe_error_is_reported" in wireless_module_load_test
assert "is_module_loaded_status(const char *module_name)" in common_ex
assert "opendir(\"/sys/module\")" in common_ex
assert "return is_module_loaded_status(module_name) > 0;" in common_ex
assert '"trunk/user/rc/src/wireless_module_load_policy.h"' in process_workflow
assert '"trunk/user/rc/src/tests/wireless_module_load_policy.c"' in process_workflow
assert "Compile and run RC module load/unload postcondition regression" in xy_workflow
assert '"trunk/user/rc/src/wireless_module_load_policy.h"' in xy_workflow
assert '"trunk/user/rc/src/tests/wireless_module_load_policy.c"' in xy_workflow
assert "int boot_modules_status, boot_services_status;" in rc_source
assert "rwfs_mount_status, boot_modules_status);" in " ".join(rc_source.split())
assert "int service_start_status = start_services_once(is_ap_mode);" in rc_source
assert "rc_first_service_start_status(boot_services_status," in rc_source
assert "one or more boot service startup operations failed (first status=%d)" in rc_source
assert "boot_services_status != 0" in rc_source
assert "Compile and run RC boot service status aggregation regression" in process_workflow
assert '"trunk/user/rc/src/service_start_status.h"' in process_workflow
assert '"trunk/user/rc/src/tests/service_start_status.c"' in process_workflow
assert '"trunk/user/rc/src/service_start_status.h"' in xy_workflow
assert '"trunk/user/rc/src/tests/service_start_status.c"' in xy_workflow
boot_launch = rc_source.split(
    "/* Install the firmware CA trust store before autostart can launch AliDDNS. */",
    1
)[1].split("return 0;", 1)[0]
assert "if (check_if_file_exist(\"/etc/storage/started_script.sh\"))" in boot_launch
assert 'int started_script_status = system("/etc/storage/started_script.sh &");' in boot_launch
assert "persistent user startup script launch failed (wait status=%d)" in boot_launch
assert 'int autostart_status = system("/usr/bin/autostart.sh &");' in boot_launch
assert "firmware optional-service autostart launch failed (wait status=%d)" in boot_launch
assert boot_launch.index("system(\"/usr/bin/copyscripts.sh\")") < boot_launch.index("started_script_status")
assert boot_launch.index("started_script_status") < boot_launch.index("autostart_status")
print("RC boot-service status aggregation and disabled-feature sentinel handling: PASS")
print("Optional boot launcher invocation failure diagnostics: PASS")


# AiDisk disable actions must always reach the RC stop handlers. The handlers
# own process-inventory policy and must propagate UNKNOWN rather than treating
# a failed legacy Boolean presence query as confirmed ABSENT.
aidisk_source = (root / "trunk/user/httpd/src/aidisk.c").read_text()
aidisk_handler_start = aidisk_source.index("int\nej_set_AiDisk_status")
aidisk_handler_end = aidisk_source.index("\nint\nej_get_all_accounts", aidisk_handler_start)
aidisk_handler = aidisk_source[aidisk_handler_start:aidisk_handler_end]
aidisk_handler_code = re.sub(r"/\*.*?\*/|//[^\n]*", "", aidisk_handler, flags=re.S)
assert not re.search(r"\bpids\s*\(", aidisk_handler_code)
assert 'result = system("/sbin/stop_samba");' in aidisk_handler
assert 'result = system("/sbin/stop_ftp");' in aidisk_handler
assert "SET_AIDISK_STATUS_SUCCESS:" not in aidisk_handler
print("AiDisk Samba/FTP disable uses tri-state RC stop handlers without legacy Boolean prechecks: PASS")


# CPU watchdog hot-restart notifications must return verified module-state
# failures to rc_notify instead of discarding them in a void helper.
watchdog_cpu_restart_policy = (
    root / "trunk/user/rc/src/watchdog_cpu_restart_policy.h"
).read_text()
watchdog_cpu_restart_test = (
    root / "trunk/user/rc/src/tests/watchdog_cpu_restart_policy.c"
).read_text()
watchdog_cpu_services = (root / "trunk/user/rc/src/services.c").read_text()
watchdog_cpu_rc = (root / "trunk/user/rc/src/rc.c").read_text()
watchdog_cpu_header = (root / "trunk/user/rc/src/rc.h").read_text()
watchdog_restart_body = watchdog_cpu_services.split(
    "int" + chr(10) + "restart_watchdog_cpu(void)", 1
)[1].split(chr(10) + "}", 1)[0]
assert '#include "watchdog_cpu_restart_policy.h"' in watchdog_cpu_services
assert 'rc_watchdog_cpu_restart_verified("rt_timer_wdg", should_load,' in watchdog_restart_body
assert "watchdog_cpu_restart_module_load" in watchdog_cpu_services
assert "watchdog_cpu_restart_module_unload" in watchdog_cpu_services
assert "watchdog_cpu_restart_module_is_loaded" in watchdog_cpu_services
assert "int restart_watchdog_cpu(void);" in watchdog_cpu_header
assert "void restart_watchdog_cpu(void);" not in watchdog_cpu_header
assert "action_result = rc_notify_merge_action_result(action_result, restart_watchdog_cpu());" in watchdog_cpu_rc
assert "probe_status = ops->is_loaded(module_name);" in watchdog_cpu_restart_policy
assert "if (probe_status < 0)" in watchdog_cpu_restart_policy
assert "return -probe_status;" in watchdog_cpu_restart_policy
assert "if (probe_status > 1)" in watchdog_cpu_restart_policy
assert "return should_load ? 0 : EBUSY;" in watchdog_cpu_restart_policy
assert "return should_load ? EIO : 0;" in watchdog_cpu_restart_policy
watchdog_start_body = c_function_body(
    watchdog_cpu_services, "int" + chr(10) + "start_watchdog_cpu(void)"
)
assert 'module_status = is_module_loaded_status("rt_timer_wdg");' in watchdog_start_body
assert "if (module_status < 0)" in watchdog_start_body
assert "return probe_error;" in watchdog_start_body
watchdog_probe_body = c_function_body(
    watchdog_cpu_services,
    "static int" + chr(10) + "watchdog_cpu_restart_module_is_loaded("
)
assert "return is_module_loaded_status(module_name);" in watchdog_probe_body
for test_case in (
    "test_enabled_module_already_loaded_is_success",
    "test_enabled_new_module_is_verified",
    "test_failed_load_is_not_success",
    "test_visible_module_state_wins_over_loader_status",
    "test_disabled_module_unload_is_verified",
    "test_disabled_already_absent_is_success",
    "test_busy_module_prevents_false_disable_success",
    "test_absent_postcondition_wins_over_unloader_status",
    "test_unknown_module_state_is_not_treated_as_absent",
    "test_invalid_probe_state_fails_closed",
    "test_invalid_inputs_fail_before_side_effects",
):
    assert test_case in watchdog_cpu_restart_test
assert "Compile and run RC watchdog module restart postcondition regression" in process_workflow
assert "Compile and run RC watchdog module restart postcondition regression" in xy_workflow
for required_path in (
    '"trunk/user/rc/src/watchdog_cpu_restart_policy.h"',
    '"trunk/user/rc/src/tests/watchdog_cpu_restart_policy.c"',
):
    assert required_path in process_workflow
    assert required_path in xy_workflow
print("RC CPU watchdog restart verifies module state and propagates notification failures: PASS")


# DDNS restart notifications must preserve stop/start failures. The start
# handler already performs its own stop-before-start operation; disabled or
# AP-mode configurations only need an explicit stop.
ddns_restart_policy = (root / "trunk/user/rc/src/ddns_restart_policy.h").read_text()
ddns_restart_test = (root / "trunk/user/rc/src/tests/ddns_restart_policy.c").read_text()
ddns_restart_rc = (root / "trunk/user/rc/src/rc.c").read_text()
ddns_restart_branch = ddns_restart_rc.split(
    'strcmp(entry->d_name, RCN_RESTART_DDNS) == 0', 1
)[1].split('else if (strcmp(entry->d_name, RCN_RESTART_DI) == 0', 1)[0]
assert '#include "ddns_restart_policy.h"' in ddns_restart_rc
assert "rc_ddns_restart_run(get_ap_mode()," in ddns_restart_branch
assert 'nvram_match("ddns_enable_x", "1")' in ddns_restart_branch
assert "start_ddns, stop_ddns" in ddns_restart_branch
assert "action_result = rc_notify_merge_action_result(action_result, ddns_status);" in ddns_restart_branch
assert "stop_ddns();" not in ddns_restart_branch
assert "start_ddns(1);" not in ddns_restart_branch
assert "if (ap_mode || !ddns_enabled)" in ddns_restart_policy
assert "return stop();" in ddns_restart_policy
assert "return start(1);" in ddns_restart_policy
for test_case in (
    "test_enabled_router_uses_one_start_transaction",
    "test_enabled_router_propagates_start_failure",
    "test_disabled_ddns_stops_without_starting",
    "test_disabled_ddns_propagates_stop_failure",
    "test_ap_mode_stops_even_when_ddns_is_enabled",
    "test_ap_mode_propagates_stop_failure",
    "test_invalid_callbacks_fail_without_side_effects",
):
    assert test_case in ddns_restart_test
assert "Compile and run RC DDNS restart result policy regression" in process_workflow
assert "Compile and run RC DDNS restart result policy regression" in xy_workflow
for required_path in (
    '"trunk/user/rc/src/ddns_restart_policy.h"',
    '"trunk/user/rc/src/tests/ddns_restart_policy.c"',
):
    assert required_path in process_workflow
    assert required_path in xy_workflow
print("RC DDNS restart notification preserves disabled/AP behavior and operation failures: PASS")


# VPN server restart must preserve failures through teardown, start, shared
# xl2tpd restoration and lifecycle-lock release. Both the PID-1 notification
# and direct rc applet return the same status to their caller.
vpn_restart_policy = (root / "trunk/user/rc/src/vpn_server_restart_policy.h").read_text()
vpn_restart_test = (root / "trunk/user/rc/src/tests/vpn_server_restart_policy.c").read_text()
vpn_restart_source = (root / "trunk/user/rc/src/vpn_server.c").read_text()
vpn_restart_rc = (root / "trunk/user/rc/src/rc.c").read_text()
vpn_restart_header = (root / "trunk/user/rc/src/rc.h").read_text()
vpn_restart_body = vpn_restart_source.split(
    "int" + chr(10) + "restart_vpn_server(void)", 1
)[1].split(chr(10) + "static void" + chr(10) + "reapply_vpn_server_locked", 1)[0]
assert '#include "vpn_server_restart_policy.h"' in vpn_restart_source
assert "int restart_vpn_server(void);" in vpn_restart_header
assert "void restart_vpn_server(void);" not in vpn_restart_header
assert 'restore_shared_xl2tpd_locked("after incomplete VPN teardown")' in vpn_restart_body
assert "vpn_server_restart_first_error(restart_status, start_status)" in vpn_restart_body
assert "vpn_server_restart_first_error(restart_status, restore_status)" in vpn_restart_body
assert "vpn_server_restart_first_error(restart_status, -1)" in vpn_restart_body
# A failed stop transaction may already have stopped shared xl2tpd before
# a later daemon/script/lease cleanup error. Keep the restore intent set before
# the transaction, and restore the WAN/VPN client daemon on both failure and
# normal restart paths while the lifecycle lock remains held.
vpn_stop_body = vpn_restart_source.split(
    "static int" + chr(10) + "stop_vpn_server_locked(void)", 1
)[1].split(chr(10) + "int" + chr(10) + "stop_vpn_server(void)", 1)[0]
vpn_shared_restore = vpn_restart_source.split(
    "static int" + chr(10) + "restore_shared_xl2tpd_locked(const char *phase)", 1
)[1].split(chr(10) + "int" + chr(10) + "restart_vpn_server(void)", 1)[0]
assert "xl2tpd_stop_attempted_vpns = xl2tpd_active;" in vpn_stop_body
assert vpn_stop_body.index("xl2tpd_stop_attempted_vpns = xl2tpd_active;") < vpn_stop_body.index("kill_services_with_pidfiles(")
assert "vpn_server_restart_should_restore_shared_xl2tpd(" in vpn_shared_restore
assert "restore_status = safe_start_xl2tpd_locked();" in vpn_shared_restore
assert 'restore_shared_xl2tpd_locked("after incomplete VPN teardown")' in vpn_restart_body
assert 'restore_shared_xl2tpd_locked("after VPN server restart")' in vpn_restart_body
assert vpn_restart_body.index("restore_shared_xl2tpd_locked(\"after incomplete VPN teardown\")") < vpn_restart_body.index("vpn_server_lifecycle_lock_release(lock_fd);")
assert "test_shared_xl2tpd_restoration_decision" in vpn_restart_test
assert "restore_status < 0" in vpn_restart_body
assert "restore_status != 0" not in vpn_restart_body
assert "release_status = vpn_server_lifecycle_lock_release(lock_fd);" in vpn_restart_body
assert "action_result = rc_notify_merge_action_result(action_result, restart_vpn_server());" in vpn_restart_rc
assert 'ret = restart_vpn_server();' in vpn_restart_rc
for test_case in (
    "test_success_stays_success",
    "test_start_failure_is_returned",
    "test_restore_failure_is_returned_when_start_succeeded",
    "test_first_failure_survives_later_restore_and_unlock_errors",
    "test_unlock_failure_is_returned_after_successful_operations",
    "test_positive_wait_status_is_not_lost",
):
    assert test_case in vpn_restart_test
assert "Compile and run VPN server restart status propagation regression" in process_workflow
assert "Compile and run VPN server restart status propagation regression" in xy_workflow
for required_path in (
    '"trunk/user/rc/src/vpn_server_restart_policy.h"',
    '"trunk/user/rc/src/tests/vpn_server_restart_policy.c"',
    '"trunk/user/rc/src/vpn_server.c"',
    '"trunk/user/rc/src/rc.h"',
):
    assert required_path in process_workflow
    assert required_path in xy_workflow
print("VPN server restart propagates teardown, start, restoration and unlock failures: PASS")


# Validate NVRAM LAN values and bound MiniUPnP presentation URL formatting.
upnp_url_source = (root / "trunk/user/rc/src/services_ex.c").read_text()
upnp_url_policy = (root / "trunk/user/rc/src/miniupnpd_presentation_url.h").read_text()
upnp_url_test = (root / "trunk/user/rc/src/tests/miniupnpd_presentation_url.c").read_text()
upnp_start_body = c_function_body(upnp_url_source, "static int" + chr(10) + "start_upnp_locked(void)")
assert '#include "miniupnpd_presentation_url.h"' in upnp_url_source
assert "is_valid_ipv4(lan_addr)" in upnp_start_body
assert "is_valid_ipv4(lan_mask)" in upnp_start_body
assert "miniupnpd_format_lan_url(lan_addr, ret, var, sizeof(var))" in upnp_start_body
assert "sprintf(var," not in upnp_start_body
assert "http_port < 0 || http_port > 65535" in upnp_url_policy
assert "snprintf(buffer, buffer_size" in upnp_url_policy
assert "errno = ENOSPC;" in upnp_url_policy
for test_case in (
    "test_default_ports_omit_suffix",
    "test_custom_port_is_appended",
    "test_oversized_address_fails_without_truncation",
    "test_invalid_port_and_null_address_fail",
    "test_invalid_output_buffer_fails",
    "test_short_output_buffer_is_cleared",
):
    assert test_case in upnp_url_test, test_case
print("MiniUPnP LAN URL validation and bounded formatting contract: PASS")


# UPnP restart refuses to reapply firewall rules when stop_upnp() cannot
# prove that the old daemon stopped; preserve that status to PID 1.
upnp_restart_policy = (root / "trunk/user/rc/src/upnp_restart_policy.h").read_text()
upnp_restart_test = (root / "trunk/user/rc/src/tests/upnp_restart_policy.c").read_text()
upnp_restart_source = (root / "trunk/user/rc/src/services_ex.c").read_text()
upnp_restart_rc = (root / "trunk/user/rc/src/rc.c").read_text()
upnp_restart_header = (root / "trunk/user/rc/src/rc.h").read_text()
upnp_restart_body = upnp_restart_source.split(
    "int" + chr(10) + "restart_upnp(void)", 1
)[1].split("int" + chr(10) + "ddns_updated_main", 1)[0]
assert '#include "upnp_restart_policy.h"' in upnp_restart_source
assert "int restart_upnp(void);" in upnp_restart_header
assert "void restart_upnp(void);" not in upnp_restart_header
assert "rc_upnp_restart_run(stop_upnp_locked, restart_firewall)" in upnp_restart_body
assert 'return miniupnpd_lifecycle_lock_finish(lock_fd, ret, "restart");' in upnp_restart_body
assert "return ret;" not in upnp_restart_body
assert "action_result = rc_notify_merge_action_result(action_result, restart_upnp());" in upnp_restart_rc
for test_case in (
    "test_success_stops_then_restarts_firewall",
    "test_stop_failure_aborts_firewall_restart_and_propagates",
    "test_firewall_refresh_failure_is_propagated",
    "test_stop_error_wins_when_firewall_is_not_attempted",
    "test_invalid_callbacks_fail_without_side_effects",
):
    assert test_case in upnp_restart_test
assert "Compile and run RC UPnP restart stop-result regression" in process_workflow
assert "Compile and run RC UPnP restart stop-result regression" in xy_workflow
for required_path in (
    '"trunk/user/rc/src/upnp_restart_policy.h"',
    '"trunk/user/rc/src/tests/upnp_restart_policy.c"',
    '"trunk/user/rc/src/services_ex.c"',
    '"trunk/user/rc/src/rc.h"',
):
    assert required_path in process_workflow
    assert required_path in xy_workflow
print("RC UPnP restart preserves daemon stop failure in PID 1 result: PASS")


# full_restart_lan() is a single PID-1 notification transaction: retain first
# failure from status-bearing teardown, recovery, reload and start operations.
lan_source = (root / "trunk/user/rc/src/net_lan.c").read_text()
lan_test = (root / "trunk/user/rc/src/tests/service_start_status.c").read_text()
lan_rc = (root / "trunk/user/rc/src/rc.c").read_text()
assert '#include "service_start_status.h"' in lan_source
lan_start = lan_source.index("int" + chr(10) + "full_restart_lan(void)")
lan_brace = lan_source.index("{", lan_start)
lan_depth = 0
for lan_index in range(lan_brace, len(lan_source)):
    if lan_source[lan_index] == "{":
        lan_depth += 1
    elif lan_source[lan_index] == "}":
        lan_depth -= 1
        if lan_depth == 0:
            lan_body = lan_source[lan_start:lan_index + 1]
            break
else:
    raise AssertionError("unterminated full_restart_lan()")
for contract in (
    'lan_restart_record_status(&restart_status, "stop logger", stop_logger())',
    'lan_restart_record_status(&restart_status, "stop LLTD", stop_lltd())',
    'lan_restart_record_status(&restart_status, "stop infosvr", stop_infosvr())',
    'lan_restart_record_status(&restart_status, "stop networkmap", stop_networkmap())',
    'lan_restart_record_status(&restart_status, "stop miniupnpd", stop_upnp())',
    'lan_restart_record_status(&restart_status, "stop DNSMasq", dnsmasq_status)',
    'lan_restart_record_status(&restart_status, "stop LAN services", lan_status)',
    'lan_restart_record_status(&restart_status, "start logger", start_logger(0))',
    'lan_restart_record_status(&restart_status, "start VPN server after WAN restart",',
    'lan_restart_record_status(&restart_status, "restart VPN server",',
    'lan_restart_record_status(&restart_status, "restart IPTV services",',
    'lan_restart_record_status(&restart_status, "reload NFS exports", reload_nfsd())',
    'lan_restart_record_status(&restart_status, "reload NetBIOS services", reload_nmbd())',
    'lan_restart_record_status(&restart_status, "start infosvr", start_infosvr())',
    'lan_restart_record_status(&restart_status, "start LLTD", start_lltd())',
    'lan_restart_record_status(&restart_status, "start networkmap", start_networkmap(1))',
    'lan_restart_record_status(&restart_status, "restart IAPP daemon",',
    'lan_restart_record_status(&restart_status, "notify HTTPD logout",',
    'return restart_status;',
):
    assert contract in lan_body, contract
assert "return dnsmasq_status;" not in lan_body
assert "rc_notify_merge_action_result(action_result, full_restart_lan())" in lan_rc
assert "test_first_error_survives_lan_restart_recovery" in lan_test
assert "test_negative_and_positive_statuses_are_preserved" in lan_test
print("RC full LAN restart first-error propagation and PID 1 integration: PASS")


# Router-mode bridge STP and forward-delay configuration are fallible sysfs
# writes. Both operations must be attempted and the first failure retained.
bridge_source = (root / "trunk/user/rc/src/services.c").read_text()
bridge_start = bridge_source.split("start_services_once(int is_ap_mode)", 1)[1].split(
    "stop_services(int stopall)", 1
)[0]
for status_name, call in (
    ("bridge_stp_status", "br_set_stp(IFNAME_BR, 1)"),
    ("bridge_forward_delay_status", "br_set_fd(IFNAME_BR, 15)"),
):
    assert f"{status_name} = {call};" in bridge_start, status_name
    assert f"start_status, {status_name});" in bridge_start, status_name
assert "LAN bridge STP enable failed" in bridge_start
assert "LAN bridge forward-delay configuration failed" in bridge_start
assert "return start_status;" in bridge_start
bridge_test = (root / "trunk/user/rc/src/tests/service_start_status.c").read_text()
assert "test_bridge_parameters_are_both_attempted_and_first_error_survives" in bridge_test
assert "Compile and run RC boot service status aggregation regression" in process_workflow
assert "Compile and run RC LAN restart first-error aggregation regression" in xy_workflow
print("RC bridge STP and forward-delay result propagation: PASS")

# RCN_RESTART_SYSCTL must preserve the errno-style PPPoE relay postcondition.
sysctl_common = (root / "trunk/user/rc/src/common_ex.c").read_text()
sysctl_header = (root / "trunk/user/rc/src/rc.h").read_text()
sysctl_rc = (root / "trunk/user/rc/src/rc.c").read_text()
sysctl_body = c_function_body(sysctl_common, "restart_all_sysctl(void)")
assert "int restart_all_sysctl(void)" in " ".join(sysctl_common.split())
assert '#include "service_start_status.h"' in sysctl_common
assert "passthrough_status = set_passthrough_pppoe(1);" in sysctl_body
sysctl_body_compact = "".join(sysctl_body.split())
assert "rc_first_service_start_status(restart_status,passthrough_status)" in sysctl_body_compact
assert "return restart_status;" in sysctl_body
assert "int restart_all_sysctl(void);" in sysctl_header
sysctl_dispatch = " ".join(sysctl_rc.split("RCN_RESTART_SYSCTL) == 0)", 1)[1].split(";", 1)[0].split())
assert "rc_notify_merge_action_result(" in sysctl_dispatch
assert "restart_all_sysctl()" in sysctl_dispatch
sysctl_regression = (root / "trunk/user/rc/src/tests/service_start_status.c").read_text()
assert "test_sysctl_pppoe_relay_failure_keeps_notification_pending" in sysctl_regression
assert "rc_notify_merge_action_result(action_result, restart_status)" in sysctl_regression
print("RC sysctl PPPoE relay status reaches PID 1 notification result: PASS")


# stop_lan() previously discarded clear_if_route4() status. Preserve its
# failure after conntrack cleanup while still attempting IPv6/interface teardown.
lan6_source = (root / "trunk/user/rc/src/net_lan6.c").read_text()
clear_lan_addr6 = c_function_body(lan6_source, "clear_lan_addr6(void)")
assert "return clear_if_addr6(IFNAME_BR);" in clear_lan_addr6
lan_teardown = c_function_body(lan_source, "stop_lan(int is_ap_mode)")
lan_teardown_compact = "".join(lan_teardown.split())
assert "intconntrack_status=0,route_status=0,ipv6_status=0;" in lan_teardown_compact
assert "route_status=clear_if_route4(IFNAME_BR);" in lan_teardown_compact
assert 'if(route_status!=0)logmessage(LOGNAME,"LANroutecleanupfailed(status=%d)",route_status);' in lan_teardown_compact
assert "ipv6_status=clear_lan_addr6();" in lan_teardown_compact
assert "returnrc_first_service_start_status(rc_first_service_start_status(rc_first_service_start_status(conntrack_status,route_status),ipv6_status),status);" in lan_teardown_compact
route_flush_at = lan_teardown_compact.index("route_status=clear_if_route4(IFNAME_BR);")
ipv6_cleanup_at = lan_teardown_compact.index("ipv6_status=clear_lan_addr6();")
bridge_down_at = lan_teardown_compact.index("ifconfig(IFNAME_BR,0,NULL,NULL);")
assert route_flush_at < ipv6_cleanup_at < bridge_down_at
assert "test_router_lan_teardown_preserves_route_failure_and_continues" in lan_test
assert "test_router_lan_teardown_preserves_route_failure_and_continues();" in lan_test
assert "test_router_lan_teardown_preserves_ipv6_clear_failure_and_continues" in lan_test
assert "test_router_lan_teardown_preserves_ipv6_clear_failure_and_continues();" in lan_test
print("LAN route flush errno reaches stop_lan() result without skipping later cleanup: PASS")


net6_source = (root / "trunk/user/rc/src/net6.c").read_text()
rc_boot_source = (root / "trunk/user/rc/src/rc.c").read_text()
rc_header_source = (root / "trunk/user/rc/src/rc.h").read_text()
ipv6_init_body = c_function_body(net6_source, "int init_ipv6(void)")
assert "control_if_ipv6_all((ipv6_type == IPV6_DISABLED) ? 0 : 1)" in ipv6_init_body
assert "if (conf_status != 0)" in ipv6_init_body
assert ipv6_init_body.index("reset_lan6_vars();") < ipv6_init_body.index("return conf_status;")
assert "int init_ipv6(void);" in rc_header_source
assert "void init_ipv6(void);" not in rc_header_source

router_init_body = c_function_body(rc_boot_source, "init_router(void)")
router_init_compact = re.sub(r"\s+", " ", router_init_body)
assert "int ipv6_init_status = init_ipv6();" in router_init_compact
assert (
    "boot_services_status = rc_first_service_start_status( "
    "boot_services_status, ipv6_init_status);"
) in router_init_compact, "init_router must preserve IPv6 global sysctl failure in its first-error boot status"
assert router_init_body.index("ipv6_init_status = init_ipv6()") < router_init_body.index("set_cpu_affinity();")
assert '"trunk/user/rc/src/net6.c"' in process_workflow
assert '"trunk/user/rc/src/rc.c"' in process_workflow
assert '"trunk/user/rc/src/rc.h"' in process_workflow
assert '"trunk/user/rc/src/net6.c"' in xy_workflow
assert '"trunk/user/rc/src/rc.c"' in xy_workflow
assert '"trunk/user/rc/src/rc.h"' in xy_workflow
print("IPv6 boot sysctl initialization status aggregation: PASS")
shutdown_router = rc_source.split("shutdown_router(int level)", 1)[1].split("\n}\n", 1)[0]
assert "auth_stop_status = stop_8021x_all();" in shutdown_router
assert "auth_stop_status == 0" in shutdown_router
assert "int wifi_wl_stop_status = 0, wifi_rt_stop_status = 0;" in shutdown_router
assert "wifi_wl_stop_status = stop_wifi_all_wl();" in shutdown_router
assert "wifi_rt_stop_status = stop_wifi_all_rt();" in shutdown_router
assert "wifi_wl_stop_status != 0" in shutdown_router
assert "wifi_rt_stop_status != 0" in shutdown_router
assert "wifi_wl_stop_status == 0" in shutdown_router
assert "wifi_rt_stop_status == 0" in shutdown_router
assert "int lan_stop_status = 0;" in shutdown_router
assert "lan_stop_status = stop_lan(is_ap_mode);" in shutdown_router
assert "if (lan_stop_status != 0)" in shutdown_router
assert "lan_stop_status == 0" in shutdown_router
assert 'LAN shutdown incomplete; continuing power-down without storage persistence' in shutdown_router
assert shutdown_router.count("stop_lan(is_ap_mode);") == 1
mtd_gate_pos = shutdown_router.index("if (storage_remove_status == 0")
assert shutdown_router.index("wifi_wl_stop_status = stop_wifi_all_wl();") < mtd_gate_pos
assert shutdown_router.index("wifi_rt_stop_status = stop_wifi_all_rt();") < mtd_gate_pos
lan_stop_pos = shutdown_router.index("lan_stop_status = stop_lan(is_ap_mode);")
assert shutdown_router.index("wifi_wl_stop_status = stop_wifi_all_wl();") < lan_stop_pos < mtd_gate_pos
assert lan_stop_pos < shutdown_router.index("rwfs_unmount_status = umount_rwfs_partition();")
assert shutdown_router.index("if (lan_stop_status != 0)") < mtd_gate_pos
assert shutdown_router.count("stop_lan(is_ap_mode);") == 1
assert shutdown_router.count("stop_wifi_all_wl();") == 1
assert shutdown_router.count("stop_wifi_all_rt();") == 1
assert shutdown_router.index("auth_stop_status = stop_8021x_all();") < shutdown_router.index(
    "auth_stop_status == 0"
)
assert '"trunk/user/rc/src/net_wifi.c"' in process_workflow
assert '"trunk/user/rc/src/rc.c"' in process_workflow

auth_source = (root / "trunk/user/rc/src/auth.c").read_text()
auth_stop_kabinet = auth_source.split("int stop_auth_kabinet(void)", 1)[1].split("\n}", 1)[0]
auth_stop_eapol = auth_source.split("int stop_auth_eapol(void)", 1)[1].split("\n}", 1)[0]
auth_start_kabinet = auth_source.split("int start_auth_kabinet(void)", 1)[1].split("\n}", 1)[0]
auth_start_eapol = auth_source.split("int start_auth_eapol(", 1)[1].split("\n}", 1)[0]
assert "return kill_services(svcs, 3, 1);" in auth_stop_kabinet
assert "return kill_services(svcs, 3, 1);" in auth_stop_eapol
assert "ret = stop_auth_kabinet();" in auth_start_kabinet
assert "if (ret != 0)" in auth_start_kabinet
assert auth_start_kabinet.index("ret = stop_auth_kabinet();") < auth_start_kabinet.index("if (ret != 0)") < auth_start_kabinet.index('eval("/usr/sbin/lanauth"')
assert "ret = stop_auth_eapol();" in auth_start_eapol
assert "if (ret != 0)" in auth_start_eapol
assert auth_start_eapol.index("ret = stop_auth_eapol();") < auth_start_eapol.index("if (ret != 0)") < auth_start_eapol.index('fopen(wpa_conf, "w")')
assert '"trunk/user/rc/src/auth.c"' in process_workflow
assert '"trunk/user/rc/src/auth.c"' in xy_workflow

watchdog_source = (root / "trunk/user/rc/src/watchdog.c").read_text()
httpd_probe = watchdog_source.split("static int\nhttpd_check_v2()", 1)[1].split(
    "\nstatic int\nrefresh_ntp(void)", 1
)[0]
busybox_config = (root / "trunk/configs/boards/busybox.config").read_text()
assert 'wget -q -T 1 http://127.0.0.1:%d/httpd_check.htm' in httpd_probe
assert 'pids("wget")' not in httpd_probe
assert 'killall wget' not in httpd_probe
assert "CONFIG_FEATURE_WGET_TIMEOUT=y" in busybox_config
watchdog_httpd = watchdog_source.split("static void httpd_process_check(void)", 1)[1].split(
    "/* Sometimes, dnsmasq crashed, try to re-run it */", 1
)[0]
watchdog_dnsmasq = watchdog_source.split("static void\ndnsmasq_process_check(void)", 1)[1].split(
    "\n}\n", 1
)[0]
watchdog_start = watchdog_source.split("int\nstart_watchdog(void)", 1)[1].split(
    "\n}\n", 1
)[0]
watchdog_notify = watchdog_source.split("void\nnotify_watchdog_time(void)", 1)[1].split(
    "\n}\n", 1
)[0]
for body, service in ((watchdog_httpd, "httpd"), (watchdog_dnsmasq, "dnsmasq"),
                      (watchdog_start, "watchdog")):
    assert 'process_inventory_scan("{}", &snapshot)'.format(service) in body
    assert "scan_error = snapshot.error;" in body
    assert body.index("scan_error = snapshot.error;") < body.index("process_snapshot_release(&snapshot);")
    assert body.index("process_snapshot_release(&snapshot);") < body.index("process_presence_start_action_for_state(presence)")
    assert "pids(" not in body
assert "is_dns_dhcpd_run()" not in watchdog_dnsmasq
assert "PROCESS_PRESENCE_START_ABORT" in watchdog_httpd
assert "restart_status = restart_httpd();" in watchdog_httpd
assert watchdog_httpd.count("restart_httpd();") == 1
assert "if (restart_status != 0)" in watchdog_httpd
assert "HTTPD recovery restart failed" in watchdog_httpd
assert "stop_httpd(" not in watchdog_httpd
assert "start_httpd(0)" not in watchdog_httpd
assert "killall -9 httpd" not in watchdog_httpd
assert "system(" not in watchdog_httpd

# Watchdog recovery must use the single-lock stop/start transaction, rather
# than reacquiring the process-associated record lock between public calls.
httpd_restart = c_function_body(services_source, "restart_httpd(void)")
assert 'httpd_lifecycle_lock_acquire_status("restart", &lock_fd)' in httpd_restart
assert httpd_restart.index("stop_httpd_locked();") < httpd_restart.index(
    "httpd_restart_can_start(stop_status)"
) < httpd_restart.index("start_httpd_locked(1);")
assert "httpd_lifecycle_lock_finish(lock_fd, status, \"restart\")" in httpd_restart
assert "PROCESS_PRESENCE_START_ABORT" in watchdog_dnsmasq
assert "PROCESS_PRESENCE_START_NEW" in watchdog_dnsmasq
assert "dnsmasq_missing = 0;" in watchdog_dnsmasq
assert "PROCESS_PRESENCE_START_NOTIFY" in watchdog_start
assert "PROCESS_PRESENCE_START_NEW" in watchdog_start
assert "PROCESS_PRESENCE_START_ABORT" in watchdog_start
assert "if (action == PROCESS_PRESENCE_START_NEW)" in watchdog_start
assert "launch_status = eval(\"/sbin/watchdog\")" in watchdog_start
assert "if (launch_status != 0)" in watchdog_start
assert "watchdog launcher failed (status=%d)" in watchdog_start
assert "return launch_status;" in watchdog_start
assert watchdog_start.index("if (action == PROCESS_PRESENCE_START_NEW)") < watchdog_start.index(
    "launch_status = eval(\"/sbin/watchdog\")"
) < watchdog_start.index("if (launch_status != 0)") < watchdog_start.index("return launch_status;")
assert "refusing watchdog start because process-presence policy is invalid" in watchdog_start
assert "start_watchdog()" in watchdog_notify
assert "pids(" not in watchdog_notify
assert "process_pidfile_signal(WD_PID_FILE, \"watchdog\", SIGHUP)" in watchdog_notify
assert '"trunk/user/rc/src/watchdog.c"' in process_workflow
assert '"trunk/user/rc/src/watchdog.c"' in xy_workflow
assert '"trunk/user/rc/src/process_presence_start_policy.h"' in process_workflow
assert '"trunk/user/rc/src/tests/process_presence_start_policy.c"' in process_workflow
print("watchdog process inventory tri-state recovery/start integration: PASS")


lltd_start = services_source.split("int\nstart_lltd(void)", 1)[1].split("\n}", 1)[0]
lltd_launcher = 'start_status = eval("/bin/lld2d", IFNAME_BR);'
assert 'if (nvram_invmatch("lltd_enable", "1"))' in lltd_start
assert lltd_start.index("return 1;") < lltd_start.index('optional_daemon_start_status("lld2d", "lld2d")')
assert "if (start_status > 0)" in lltd_start and "if (start_status < 0)" in lltd_start
assert lltd_launcher in lltd_start
assert 'logmessage(LOGNAME, "LLTD daemon launch failed (status=%d)", start_status);' in lltd_start
assert "process_presence_start_postcondition" in lltd_start
assert 'optional_daemon_presence("lld2d", &scan_error)' in lltd_start
assert "PROCESS_PRESENCE_START_INITIAL_WAIT_SECONDS" in lltd_start
assert "PROCESS_PRESENCE_START_PROBE_ATTEMPTS" in lltd_start
assert "PROCESS_PRESENCE_START_RETRY_WAIT_SECONDS" in lltd_start
assert "process_state == PROCESS_PRESENCE_UNKNOWN" in lltd_start
assert "errno = scan_error > 0 ? scan_error : EIO;" in lltd_start
assert "errno = ETIMEDOUT;" in lltd_start
lltd_preflight = lltd_start.index('optional_daemon_start_status("lld2d", "lld2d")')
lltd_launch_at = lltd_start.index(lltd_launcher)
lltd_wait_at = lltd_start.index("sleep(PROCESS_PRESENCE_START_INITIAL_WAIT_SECONDS);", lltd_launch_at)
lltd_probe_at = lltd_start.index("for (attempt = 0; attempt < PROCESS_PRESENCE_START_PROBE_ATTEMPTS; attempt++)", lltd_launch_at)
lltd_inventory_at = lltd_start.index('optional_daemon_presence("lld2d", &scan_error)', lltd_probe_at)
lltd_postcondition_at = lltd_start.index("process_presence_start_postcondition(start_status, process_state)", lltd_inventory_at)
assert lltd_preflight < lltd_launch_at < lltd_wait_at < lltd_probe_at < lltd_inventory_at < lltd_postcondition_at
assert "return process_presence_start_postcondition(start_status," in lltd_start

rstats_start = services_source.split("int\nstart_rstats(void)", 1)[1].split("\n}", 1)[0]
rstats_launcher = 'start_status = eval("/sbin/rstats");'
assert 'if (nvram_invmatch("rstats_enable", "1"))' in rstats_start
assert rstats_start.index("return 1;") < rstats_start.index('optional_daemon_start_status("rstats", "rstats")')
assert "if (start_status > 0)" in rstats_start and "if (start_status < 0)" in rstats_start
assert rstats_launcher in rstats_start
assert 'logmessage(LOGNAME, "rstats daemon launch failed (status=%d)", start_status);' in rstats_start
assert "process_presence_start_postcondition" in rstats_start
assert 'optional_daemon_presence("rstats", &scan_error)' in rstats_start
assert "PROCESS_PRESENCE_START_INITIAL_WAIT_SECONDS" in rstats_start
assert "PROCESS_PRESENCE_START_PROBE_ATTEMPTS" in rstats_start
assert "PROCESS_PRESENCE_START_RETRY_WAIT_SECONDS" in rstats_start
assert "process_state == PROCESS_PRESENCE_UNKNOWN" in rstats_start
assert "errno = scan_error > 0 ? scan_error : EIO;" in rstats_start
assert "errno = ETIMEDOUT;" in rstats_start
rstats_preflight = rstats_start.index('optional_daemon_start_status("rstats", "rstats")')
rstats_launch_at = rstats_start.index(rstats_launcher)
rstats_wait_at = rstats_start.index("sleep(PROCESS_PRESENCE_START_INITIAL_WAIT_SECONDS);", rstats_launch_at)
rstats_probe_at = rstats_start.index("for (attempt = 0; attempt < PROCESS_PRESENCE_START_PROBE_ATTEMPTS; attempt++)", rstats_launch_at)
rstats_inventory_at = rstats_start.index('optional_daemon_presence("rstats", &scan_error)', rstats_probe_at)
rstats_postcondition_at = rstats_start.index("process_presence_start_postcondition(start_status, process_state)", rstats_inventory_at)
assert rstats_preflight < rstats_launch_at < rstats_wait_at < rstats_probe_at < rstats_inventory_at < rstats_postcondition_at
assert "return process_presence_start_postcondition(start_status," in rstats_start

rstats_main = c_function_body(rstats_source, "int rstats_main(int argc, char *argv[])")
assert rstats_main.index("if (daemon(0, 0) < 0)") < rstats_main.index("if (alloc_rstats() < 0)")
assert rstats_main.index("if (alloc_rstats() < 0)") < rstats_main.index('process_pidfile_write(RSTATS_PID_FILE, "rstats")')
assert rstats_main.index('process_pidfile_write(RSTATS_PID_FILE, "rstats")') < rstats_main.index("while (!stopping)")

lldt_main = c_function_body(lldt_main_source, "main(int argc, char **argv)")
lldt_daemon = c_function_body(lldt_osl_source, "osl_become_daemon(osl_t *osl)")
assert lldt_main.index("if (!opt_debug)") < lldt_main.index("osl_become_daemon(g_osl);")
assert lldt_main.index("osl_become_daemon(g_osl);") < lldt_main.index("osl_write_pidfile(g_osl);") < lldt_main.index("event_mainloop();")
assert lldt_daemon.count("fork()") == 2 and "setsid()" in lldt_daemon
assert 'g_Progname = p+1;' in lldt_main_source
assert '"/var/run/%s-%.10s.pid", g_Progname, osl->responder_if' in lldt_osl_source
assert '"trunk/user/lldt/src/main.c"' in process_workflow
assert '"trunk/user/lldt/src/osl-linux.c"' in process_workflow
assert '"trunk/user/rc/src/rstats.c"' in process_workflow

assert '"trunk/user/rc/src/services.c"' in process_workflow
assert '"trunk/user/rc/src/services.c"' in xy_workflow
print("LLTD/rstats detached startup postconditions and daemonization contracts: PASS")

iappd_boot = services_source.split("int\nstart_services_once(int is_ap_mode)", 1)[1].split("\n}\n", 1)[0]
iappd_restart_at = iappd_boot.index('int iappd_restart_status = system("/usr/bin/iappd.sh restart");')
tproxy_load_at = iappd_boot.index('int tproxy_module_status = system("modprobe xt_TPROXY");')
iappd_test_at = iappd_boot.index('int iappd_test_status = system("/usr/bin/iappd.sh test");')
assert "Ralink IAPP restart command failed (wait status=%d)" in iappd_boot
assert "xt_TPROXY module load command failed (wait status=%d)" in iappd_boot
assert "Ralink IAPP post-start test failed (wait status=%d)" in iappd_boot
assert iappd_restart_at < tproxy_load_at < iappd_test_at
assert "if (iappd_restart_status != 0)" in iappd_boot
assert "if (tproxy_module_status != 0)" in iappd_boot
assert "if (iappd_test_status != 0)" in iappd_boot
assert not re.search(r'(?m)^\s*system\("/usr/bin/iappd\.sh restart"\);$', iappd_boot)
assert not re.search(r'(?m)^\s*system\("/usr/bin/iappd\.sh test"\);$', iappd_boot)
assert not re.search(r'(?m)^\s*system\("modprobe xt_TPROXY"\);$', iappd_boot)
print("IAPPD RC restart, TPROXY load and post-start test status propagation: PASS")

optional_boot_calls = (
    ("APP_SKIPDBV2", 'int skipdb_start_status = doSystem("/usr/sbin/skipd -d /etc/storage/db");',
     "skipdb_start_status", "SkipDBv2 boot startup failed (wait status=%d)"),
    ("APP_SCUT", "int scut_start_status = start_scutclient();",
     "scut_start_status", "SCUT client boot startup failed (status=%d)"),
    ("APP_DNSFORWARDER", "int dnsforwarder_start_status = start_dnsforwarder();",
     "dnsforwarder_start_status", "DNS Forwarder boot startup failed (status=%d)"),
    ("APP_VLMCSD", "int vlmcsd_start_status = start_vlmcsd();",
     "vlmcsd_start_status", "vlmcsd boot startup failed (status=%d)"),
    ("APP_IPERF3", "int iperf3_start_status = start_iperf3();",
     "iperf3_start_status", "iperf3 boot startup failed (status=%d)"),
    ("APP_MENTOHUST", "int mentohust_start_status = start_mentohust();",
     "mentohust_start_status", "MentoHUST boot startup failed (status=%d)"),
)
optional_call_positions = []
for feature, assignment, status_name, diagnostic in optional_boot_calls:
    feature_at = iappd_boot.index("#if defined(" + feature + ")")
    call_at = iappd_boot.index(assignment)
    check_at = iappd_boot.index("if (" + status_name + " != 0)", call_at)
    log_at = iappd_boot.index(diagnostic, check_at)
    assert feature_at < call_at < check_at < log_at
    assert "logmessage(LOGNAME," in iappd_boot[check_at:log_at]
    optional_call_positions.append(call_at)
assert optional_call_positions == sorted(optional_call_positions)
standalone_boot_calls = {line.strip() for line in iappd_boot.splitlines()}
for statement in (
    'doSystem("/usr/sbin/skipd -d /etc/storage/db");',
    "start_scutclient();",
    "start_dnsforwarder();",
    "start_vlmcsd();",
    "start_iperf3();",
    "start_mentohust();",
):
    assert statement not in standalone_boot_calls
print("Optional RC daemon startup status propagation and feature guards: PASS")

telnetd_start = services_source.split("int\nstart_telnetd(void)", 1)[1].split("\n}\n", 1)[0]
telnetd_impl = (root / "trunk/user/busybox/busybox-1.24.x/networking/telnetd.c").read_text()
assert "if (!(opt & OPT_FOREGROUND))" in telnetd_impl
assert "bb_daemonize_or_rexec(0 /*was DAEMON_CHDIR_ROOT*/, argv);" in telnetd_impl
assert telnetd_impl.index("bb_daemonize_or_rexec(0") < telnetd_impl.index("create_and_bind_stream_or_die")
assert 'if (!nvram_match("telnetd", "1"))' in telnetd_start
assert telnetd_start.index("return 0;") < telnetd_start.index('optional_daemon_start_status("telnetd", "telnetd")')
telnetd_start_compact = " ".join(telnetd_start.split())
assert 'logmessage(LOGNAME, "telnetd start aborted: process inventory unavailable (status=%d)", start_status);' in telnetd_start_compact
assert 'start_status = eval("telnetd");' in telnetd_start
assert 'logmessage(LOGNAME, "telnetd launcher failed (status=%d)", start_status);' in telnetd_start
assert "process_presence_start_postcondition" in telnetd_start
assert "PROCESS_PRESENCE_START_INITIAL_WAIT_SECONDS" in telnetd_start
assert "PROCESS_PRESENCE_START_PROBE_ATTEMPTS" in telnetd_start
assert 'optional_daemon_presence("telnetd", &scan_error)' in telnetd_start
assert "process_state == PROCESS_PRESENCE_UNKNOWN" in telnetd_start
assert "errno = scan_error > 0 ? scan_error : EIO;" in telnetd_start
assert "errno = ETIMEDOUT;" in telnetd_start
telnetd_launch = telnetd_start.index('start_status = eval("telnetd");')
telnetd_initial_wait = telnetd_start.index("sleep(PROCESS_PRESENCE_START_INITIAL_WAIT_SECONDS);", telnetd_launch)
telnetd_probe = telnetd_start.index("for (attempt = 0; attempt < PROCESS_PRESENCE_START_PROBE_ATTEMPTS; attempt++)", telnetd_launch)
telnetd_inventory = telnetd_start.index('optional_daemon_presence("telnetd", &scan_error)', telnetd_probe)
telnetd_postcondition = telnetd_start.index("process_presence_start_postcondition(start_status, process_state)", telnetd_inventory)
assert telnetd_launch < telnetd_initial_wait < telnetd_probe < telnetd_inventory < telnetd_postcondition

ttyd_start = services_source.split("int\nstart_ttyd(void)", 1)[1].split("\n}\n", 1)[0]
assert "int start_status, attempt, scan_error;" in ttyd_start
assert "enum process_presence process_state;" in ttyd_start
assert 'if (ttyd_mode != 1)' in ttyd_start
assert ttyd_start.index("return 0;") < ttyd_start.index('optional_daemon_start_status("ttyd", "ttyd")')
ttyd_start_compact = " ".join(ttyd_start.split())
assert 'logmessage(LOGNAME, "ttyd start aborted: process inventory unavailable (status=%d)", start_status);' in ttyd_start_compact
assert 'start_status = eval("/usr/bin/ttyd.sh", "start");' in ttyd_start
assert 'logmessage(LOGNAME, "ttyd launcher failed (status=%d)", start_status);' in ttyd_start
assert "process_presence_start_postcondition" in ttyd_start
assert "PROCESS_PRESENCE_START_INITIAL_WAIT_SECONDS" in ttyd_start
assert "PROCESS_PRESENCE_START_PROBE_ATTEMPTS" in ttyd_start
assert "PROCESS_PRESENCE_START_RETRY_WAIT_SECONDS" in ttyd_start
assert 'optional_daemon_presence("ttyd", &scan_error)' in ttyd_start
assert "process_state == PROCESS_PRESENCE_UNKNOWN" in ttyd_start
assert "errno = scan_error > 0 ? scan_error : EIO;" in ttyd_start
assert "errno = ETIMEDOUT;" in ttyd_start
ttyd_launch = ttyd_start.index('start_status = eval("/usr/bin/ttyd.sh", "start");')
ttyd_launch_failure = ttyd_start.index("if (start_status != 0)", ttyd_launch)
ttyd_launcher_return = ttyd_start.index("return process_presence_start_postcondition(start_status,", ttyd_launch_failure)
ttyd_initial_wait = ttyd_start.index("sleep(PROCESS_PRESENCE_START_INITIAL_WAIT_SECONDS);", ttyd_launch)
ttyd_probe = ttyd_start.index("for (attempt = 0; attempt < PROCESS_PRESENCE_START_PROBE_ATTEMPTS; attempt++)", ttyd_initial_wait)
ttyd_inventory = ttyd_start.index('optional_daemon_presence("ttyd", &scan_error)', ttyd_probe)
ttyd_postcondition = ttyd_start.index("process_presence_start_postcondition(start_status, process_state)", ttyd_inventory)
ttyd_unknown = ttyd_start.index("if (process_state == PROCESS_PRESENCE_UNKNOWN)", ttyd_postcondition)
ttyd_timeout = ttyd_start.index("errno = ETIMEDOUT;", ttyd_unknown)
assert ttyd_launch < ttyd_launch_failure < ttyd_launcher_return < ttyd_initial_wait < ttyd_probe < ttyd_inventory < ttyd_postcondition < ttyd_unknown < ttyd_timeout
assert ttyd_start.count("return process_presence_start_postcondition(start_status,") == 2
print("telnetd/ttyd detached-child startup postconditions and disabled sentinels: PASS")

optional_start_contracts = (
    ("start_adguardhome", "AdGuardHome"),
    ("start_ddnsto", "DDNS.to"),
    ("start_adbyby", "Adbyby"),
    ("start_aliddns", "AliDDNS"),
)
for function_name, log_label in optional_start_contracts:
    optional_start = services_source.split(function_name + "(void)", 1)[1].split("\n}", 1)[0]
    eval_at = optional_start.index("start_status = eval(")
    assert "int start_status;" in optional_start
    assert re.search(r"\bif\s*\(", optional_start[:eval_at])  # retain the original enabled/disabled guard
    failure_guard = optional_start.index("if (start_status != 0)", eval_at)
    failure_log = optional_start.index(f'logmessage(LOGNAME, "{log_label} launcher failed (status=%d)", start_status);', failure_guard)
    assert eval_at < failure_guard < failure_log
print("optional AdGuardHome/DDNS.to/Adbyby/AliDDNS launch diagnostics: PASS")

# Adbyby rule updates are RC notification actions too: propagate eval()'s
# wait status through PID 1 instead of discarding it in a void wrapper.
update_adb_body = services_source.split("int update_adb(void)", 1)[1].split("\n}", 1)[0]
assert 'return eval("/usr/bin/adbyby.sh","updateadb");' in update_adb_body
assert "int update_adb(void);" in rc_header
update_adb_notify = rc_source.split("RCN_RESTART_UPDATEADB) == 0)", 1)[1].split("\n\t\t}", 1)[0]
assert "rc_notify_merge_action_result(action_result, update_adb())" in update_adb_notify
print("Adbyby update action result propagation: PASS")


# MSD Lite config publication must finish before enabling or launching the daemon.
msd_config_header = (root / "trunk/user/rc/src/msd_lite_config_file.h").read_text()
msd_config_test = (root / "trunk/user/rc/src/tests/msd_lite_config_file.c").read_text()
udpxy_start = net_source.split("int\nstart_udpxy(char *wan_ifname)", 1)[1].split(
    "\n#if defined(APP_XUPNPD)", 1
)[0]
msd_lite_branch = udpxy_start.split("#if defined (APP_MSD_LITE)", 1)[1].split("#else", 1)[0]
assert '#include "msd_lite_config_file.h"' in net_source
assert "msd_lite_config_publish(src_conf, dst_conf, wan_ifname, port)" in msd_lite_branch
assert "if (config_status != 0)" in msd_lite_branch
assert msd_lite_branch.index("if (config_status != 0)") < msd_lite_branch.index('nvram_set("msd_lite_enable", "1")')
assert msd_lite_branch.index("if (config_status != 0)") < msd_lite_branch.index('eval("/usr/bin/msd_lite"')
assert 'fopen(dst_conf, "w")' not in msd_lite_branch
assert 'doSystem("cp -f %s %s", src_conf, dst_conf)' not in msd_lite_branch
assert "fopen(source_path, \"r\")" in msd_config_header
assert "fstat(fileno(source), &source_stat)" in msd_config_header
assert "lstat(destination_path, &destination_stat)" in msd_config_header
assert "mkstemp(temporary_path)" in msd_config_header
assert "fchmod(fd, mode)" in msd_config_header
assert "ferror(source)" in msd_config_header
assert "fflush(output)" in msd_config_header
assert "fsync(fileno(output))" in msd_config_header
assert "fclose(source)" in msd_config_header
assert "fclose(output)" in msd_config_header
assert "rename(temporary_path, destination_path)" in msd_config_header
assert "unlink(temporary_path)" in msd_config_header
assert "last-good-configuration" in msd_config_test
assert "RLIMIT_FSIZE" in msd_config_test
assert "symlink(symlink_target, symlink_path)" in msd_config_test
assert "eth3.2\\nquickleave" in msd_config_test
assert '"trunk/user/rc/src/msd_lite_config_file.h"' in process_workflow
assert '"trunk/user/rc/src/tests/msd_lite_config_file.c"' in process_workflow
assert '"trunk/user/rc/src/msd_lite_config_file.h"' in xy_workflow
assert '"trunk/user/rc/src/tests/msd_lite_config_file.c"' in xy_workflow
assert "Compile and run MSD Lite atomic configuration publication regression" in process_workflow
print("MSD Lite atomic config publication and launch gate integration: PASS")


service_start_policy = (root / "trunk/user/rc/src/service_start_status.h").read_text()
wireless_module_load_policy = (root / "trunk/user/rc/src/wireless_module_load_policy.h").read_text()
wireless_module_load_test = (root / "trunk/user/rc/src/tests/wireless_module_load_policy.c").read_text()
module_rc_source = (root / "trunk/user/rc/src/rc.c").read_text()
module_net_source = (root / "trunk/user/rc/src/net.c").read_text()
module_net_lan_source = (root / "trunk/user/rc/src/net_lan.c").read_text()
wireless_loader = rc_source.split("static int" + chr(10) + "load_wireless_module(", 1)[1].split(
    chr(10) + "#if defined (USE_USB_SUPPORT)", 1
)[0]
wireless_bulk_loader = wireless_loader.split(
    "static int" + chr(10) + "load_wireless_modules(void)", 1
)[1].split(chr(10) + "}", 1)[0]

service_start_test = (root / "trunk/user/rc/src/tests/service_start_status.c").read_text()
boot_start = services_source.split("int\nstart_services_once(int is_ap_mode)", 1)[1].split("\n}\n", 1)[0]
assert '#include "service_start_status.h"' in services_source
for call in (
    "start_httpd(0)", "start_telnetd()", "start_sshd()", "start_watchdog()",
    "start_gpio_btn()", "start_infosvr()", "start_upnp()", "start_lltd()",
    "start_udpxy(IFNAME_BR)", "start_xupnpd(IFNAME_BR)",
    "start_crond()", "start_networkmap(1)", "start_rstats()",
):
    assert call in boot_start
for call in (
    "start_httpd(0)", "start_telnetd()", "start_sshd()", "start_watchdog()",
    "start_gpio_btn()", "start_upnp()",
    "start_udpxy(IFNAME_BR)", "start_xupnpd(IFNAME_BR)",
):
    assert re.search(
        r"rc_first_service_start_status\(start_status,\s*" +
        re.escape(call) + r"\)",
        boot_start,
    ), "boot service status aggregation missing for " + call

ap_mode_start = boot_start.split("} else {", 1)[1].split("\n\t}", 1)[0]
assert "rc_first_service_start_status(start_status," in ap_mode_start
assert "start_udpxy(IFNAME_BR)" in ap_mode_start
assert "start_xupnpd(IFNAME_BR)" in ap_mode_start

# networkmap's status is captured for diagnostics before it is aggregated.
assert "int networkmap_start_status = start_networkmap(1);" in boot_start
assert 'logmessage(LOGNAME,' in boot_start
assert "rc_first_service_start_status(start_status, networkmap_start_status)" in boot_start
for service, flag in (
    ("start_infosvr()", "adsc_enable"),
    ("start_lltd()", "lltd_enable"),
    ("start_crond()", "crond_enable"),
    ("start_rstats()", "rstats_enable"),
):
    assert 'if (nvram_match("' + flag + '", "1"))' in boot_start
    assert "rc_first_service_start_status(start_status, " + service + ")" in boot_start
assert "return start_status;" in boot_start
assert "return first_status != 0 ? first_status : next_status;" in service_start_policy
for contract in (
    "rc_first_service_start_status(0, 0) == 0",
    "rc_first_service_start_status(0, -1) == -1",
    "rc_first_service_start_status(0, 17) == 17",
    "rc_first_service_start_status(-1, 0) == -1",
    "rc_first_service_start_status(17, -1) == 17",
    "rc_first_service_start_status(-5, 9) == -5",
    "rc_first_service_start_status(4, 0) == 4",
):
    assert contract in service_start_test
assert '#include "wireless_module_load_policy.h"' in rc_source
assert "(void)ops->load(module_name, module_param);" in wireless_module_load_policy
assert "loaded = ops->is_loaded(module_name);" in wireless_module_load_policy
assert "if (loaded < 0)" in wireless_module_load_policy
assert "return -loaded;" in wireless_module_load_policy
assert "return loaded == 1 ? 0 : EIO;" in wireless_module_load_policy
assert "return rc_module_load_verified(module_name, NULL, ops);" in wireless_module_load_policy
assert "if (!module_name || module_name[0] == '\\0' ||" in wireless_module_load_policy
module_loaders = (
    c_function_body(module_net_source, "reload_nat_modules(void)"),
    c_function_body(module_rc_source, "load_boot_module(const char *module_name, const char *module_param)"),
    c_function_body(module_rc_source, "load_wireless_module(const char *module_name)"),
    c_function_body(module_net_lan_source, "init_bridge_load_module_verified(const char *module_name, const char *module_param)"),
)
for loader_body in module_loaders:
    loader_code = " ".join(loader_body.split())
    assert "module_smart_load, is_module_loaded_status" in loader_code
    assert "module_smart_load, is_module_loaded)" not in loader_code
assert '"trunk/user/rc/src/net.c"' in process_workflow
assert '"trunk/user/rc/src/net_lan.c"' in process_workflow
assert '"trunk/user/rc/src/net.c"' in xy_workflow
assert '"trunk/user/rc/src/net_lan.c"' in xy_workflow
assert "rc_module_load_verified(module_name, NULL, &ops)" in wireless_loader
assert "module_smart_load(module_name, NULL)" not in wireless_loader
assert "is_module_loaded(module_name)" not in wireless_loader
assert "wireless driver module load failed or module is unavailable" in wireless_loader
assert "return status;" in wireless_loader
for module_name in (
    "rt2860v2_ap", "mt7628_ap", "mt76x2_ap", "mt_7615e",
    "rt3090_ap", "rt5392_ap", "mt76x3_ap", "rt5592_ap",
    "rt3593_ap", "mt7610_ap",
):
    assert 'load_wireless_module("' + module_name + '")' in wireless_bulk_loader
    assert "rc_first_service_start_status(start_status, load_wireless_module(" in wireless_bulk_loader
assert "return start_status;" in wireless_bulk_loader
assert "boot_modules_status = load_wireless_modules();" in rc_source
assert "Router initialization continuing with one or more unavailable hardware modules" in rc_source
wireless_boot_load = rc_source.index("boot_modules_status = load_wireless_modules();")
for module_group in (
    "boot_modules_status, load_mmc_modules());",
    "boot_modules_status, load_usb_modules());",
    "boot_modules_status, load_ata_modules());",
    "boot_modules_status, load_crypto_modules());",
    "boot_modules_status, load_ipset_modules());",
):
    assert wireless_boot_load < rc_source.index(module_group), module_group
assert "int boot_modules_status, boot_services_status;" in rc_source
normalized_rc_source = " ".join(rc_source.split())
assert "int rwfs_mount_status;" in rc_source
assert "rwfs_mount_status = mount_rwfs_partition();" in rc_source
assert (
    "boot_services_status = rc_first_service_start_status("
    " rwfs_mount_status, boot_modules_status);"
) in normalized_rc_source
assert "int service_start_status = start_services_once(is_ap_mode);" in rc_source
assert "rc_first_service_start_status(boot_services_status," in rc_source
for test_case in (
    "test_already_loaded_is_success",
    "test_newly_loaded_is_success",
    "test_failed_modprobe_is_not_success",
    "test_visible_module_state_wins_over_loader_status",
    "test_load_probe_error_is_reported",
    "test_required_module_set_probe_error_attempts_all_and_keeps_first_failure",
    "test_module_parameter_is_forwarded",
    "test_invalid_inputs_fail_before_side_effects",
    "test_unload_initial_probe_error_fails_closed",
    "test_unload_postcondition_probe_error_is_reported",
):
    assert test_case in wireless_module_load_test
assert "Compile and run RC wireless module post-load verification regression" in process_workflow
assert "test_unload_initial_probe_error_fails_closed" in wireless_module_load_test
assert "test_unload_postcondition_probe_error_is_reported" in wireless_module_load_test
assert "is_module_loaded_status(const char *module_name)" in common_ex
assert "opendir(\"/sys/module\")" in common_ex
assert "return is_module_loaded_status(module_name) > 0;" in common_ex
assert '"trunk/user/rc/src/wireless_module_load_policy.h"' in process_workflow
assert '"trunk/user/rc/src/tests/wireless_module_load_policy.c"' in process_workflow
assert "Compile and run RC module load/unload postcondition regression" in xy_workflow
assert '"trunk/user/rc/src/wireless_module_load_policy.h"' in xy_workflow
assert '"trunk/user/rc/src/tests/wireless_module_load_policy.c"' in xy_workflow
assert "int boot_modules_status, boot_services_status;" in rc_source
assert "rwfs_mount_status, boot_modules_status);" in " ".join(rc_source.split())
assert "int service_start_status = start_services_once(is_ap_mode);" in rc_source
assert "rc_first_service_start_status(boot_services_status," in rc_source
assert "one or more boot service startup operations failed (first status=%d)" in rc_source
assert "boot_services_status != 0" in rc_source
assert "Compile and run RC boot service status aggregation regression" in process_workflow
assert '"trunk/user/rc/src/service_start_status.h"' in process_workflow
assert '"trunk/user/rc/src/tests/service_start_status.c"' in process_workflow
assert '"trunk/user/rc/src/service_start_status.h"' in xy_workflow
assert '"trunk/user/rc/src/tests/service_start_status.c"' in xy_workflow
boot_launch = rc_source.split(
    "/* Install the firmware CA trust store before autostart can launch AliDDNS. */",
    1
)[1].split("return 0;", 1)[0]
assert "if (check_if_file_exist(\"/etc/storage/started_script.sh\"))" in boot_launch
assert 'int started_script_status = system("/etc/storage/started_script.sh &");' in boot_launch
assert "persistent user startup script launch failed (wait status=%d)" in boot_launch
assert 'int autostart_status = system("/usr/bin/autostart.sh &");' in boot_launch
assert "firmware optional-service autostart launch failed (wait status=%d)" in boot_launch
assert boot_launch.index("system(\"/usr/bin/copyscripts.sh\")") < boot_launch.index("started_script_status")
assert boot_launch.index("started_script_status") < boot_launch.index("autostart_status")
print("RC boot-service status aggregation and disabled-feature sentinel handling: PASS")
print("Optional boot launcher invocation failure diagnostics: PASS")


# AiDisk disable actions must always reach the RC stop handlers. The handlers
# own process-inventory policy and must propagate UNKNOWN rather than treating
# a failed legacy Boolean presence query as confirmed ABSENT.
aidisk_source = (root / "trunk/user/httpd/src/aidisk.c").read_text()
aidisk_handler_start = aidisk_source.index("int\nej_set_AiDisk_status")
aidisk_handler_end = aidisk_source.index("\nint\nej_get_all_accounts", aidisk_handler_start)
aidisk_handler = aidisk_source[aidisk_handler_start:aidisk_handler_end]
aidisk_handler_code = re.sub(r"/\*.*?\*/|//[^\n]*", "", aidisk_handler, flags=re.S)
assert not re.search(r"\bpids\s*\(", aidisk_handler_code)
assert 'result = system("/sbin/stop_samba");' in aidisk_handler
assert 'result = system("/sbin/stop_ftp");' in aidisk_handler
assert "SET_AIDISK_STATUS_SUCCESS:" not in aidisk_handler
print("AiDisk Samba/FTP disable uses tri-state RC stop handlers without legacy Boolean prechecks: PASS")


# CPU watchdog hot-restart notifications must return verified module-state
# failures to rc_notify instead of discarding them in a void helper.
watchdog_cpu_restart_policy = (
    root / "trunk/user/rc/src/watchdog_cpu_restart_policy.h"
).read_text()
watchdog_cpu_restart_test = (
    root / "trunk/user/rc/src/tests/watchdog_cpu_restart_policy.c"
).read_text()
watchdog_cpu_services = (root / "trunk/user/rc/src/services.c").read_text()
watchdog_cpu_rc = (root / "trunk/user/rc/src/rc.c").read_text()
watchdog_cpu_header = (root / "trunk/user/rc/src/rc.h").read_text()
watchdog_restart_body = watchdog_cpu_services.split(
    "int" + chr(10) + "restart_watchdog_cpu(void)", 1
)[1].split(chr(10) + "}", 1)[0]
assert '#include "watchdog_cpu_restart_policy.h"' in watchdog_cpu_services
assert 'rc_watchdog_cpu_restart_verified("rt_timer_wdg", should_load,' in watchdog_restart_body
assert "watchdog_cpu_restart_module_load" in watchdog_cpu_services
assert "watchdog_cpu_restart_module_unload" in watchdog_cpu_services
assert "watchdog_cpu_restart_module_is_loaded" in watchdog_cpu_services
assert "int restart_watchdog_cpu(void);" in watchdog_cpu_header
assert "void restart_watchdog_cpu(void);" not in watchdog_cpu_header
assert "action_result = rc_notify_merge_action_result(action_result, restart_watchdog_cpu());" in watchdog_cpu_rc
assert "probe_status = ops->is_loaded(module_name);" in watchdog_cpu_restart_policy
assert "if (probe_status < 0)" in watchdog_cpu_restart_policy
assert "return -probe_status;" in watchdog_cpu_restart_policy
assert "if (probe_status > 1)" in watchdog_cpu_restart_policy
assert "return should_load ? 0 : EBUSY;" in watchdog_cpu_restart_policy
assert "return should_load ? EIO : 0;" in watchdog_cpu_restart_policy
watchdog_start_body = c_function_body(
    watchdog_cpu_services, "int" + chr(10) + "start_watchdog_cpu(void)"
)
assert 'module_status = is_module_loaded_status("rt_timer_wdg");' in watchdog_start_body
assert "if (module_status < 0)" in watchdog_start_body
assert "return probe_error;" in watchdog_start_body
watchdog_probe_body = c_function_body(
    watchdog_cpu_services,
    "static int" + chr(10) + "watchdog_cpu_restart_module_is_loaded("
)
assert "return is_module_loaded_status(module_name);" in watchdog_probe_body
for test_case in (
    "test_enabled_module_already_loaded_is_success",
    "test_enabled_new_module_is_verified",
    "test_failed_load_is_not_success",
    "test_visible_module_state_wins_over_loader_status",
    "test_disabled_module_unload_is_verified",
    "test_disabled_already_absent_is_success",
    "test_busy_module_prevents_false_disable_success",
    "test_absent_postcondition_wins_over_unloader_status",
    "test_unknown_module_state_is_not_treated_as_absent",
    "test_invalid_probe_state_fails_closed",
    "test_invalid_inputs_fail_before_side_effects",
):
    assert test_case in watchdog_cpu_restart_test
assert "Compile and run RC watchdog module restart postcondition regression" in process_workflow
assert "Compile and run RC watchdog module restart postcondition regression" in xy_workflow
for required_path in (
    '"trunk/user/rc/src/watchdog_cpu_restart_policy.h"',
    '"trunk/user/rc/src/tests/watchdog_cpu_restart_policy.c"',
):
    assert required_path in process_workflow
    assert required_path in xy_workflow
print("RC CPU watchdog restart verifies module state and propagates notification failures: PASS")


# DDNS restart notifications must preserve stop/start failures. The start
# handler already performs its own stop-before-start operation; disabled or
# AP-mode configurations only need an explicit stop.
ddns_restart_policy = (root / "trunk/user/rc/src/ddns_restart_policy.h").read_text()
ddns_restart_test = (root / "trunk/user/rc/src/tests/ddns_restart_policy.c").read_text()
ddns_restart_rc = (root / "trunk/user/rc/src/rc.c").read_text()
ddns_restart_branch = ddns_restart_rc.split(
    'strcmp(entry->d_name, RCN_RESTART_DDNS) == 0', 1
)[1].split('else if (strcmp(entry->d_name, RCN_RESTART_DI) == 0', 1)[0]
assert '#include "ddns_restart_policy.h"' in ddns_restart_rc
assert "rc_ddns_restart_run(get_ap_mode()," in ddns_restart_branch
assert 'nvram_match("ddns_enable_x", "1")' in ddns_restart_branch
assert "start_ddns, stop_ddns" in ddns_restart_branch
assert "action_result = rc_notify_merge_action_result(action_result, ddns_status);" in ddns_restart_branch
assert "stop_ddns();" not in ddns_restart_branch
assert "start_ddns(1);" not in ddns_restart_branch
assert "if (ap_mode || !ddns_enabled)" in ddns_restart_policy
assert "return stop();" in ddns_restart_policy
assert "return start(1);" in ddns_restart_policy
for test_case in (
    "test_enabled_router_uses_one_start_transaction",
    "test_enabled_router_propagates_start_failure",
    "test_disabled_ddns_stops_without_starting",
    "test_disabled_ddns_propagates_stop_failure",
    "test_ap_mode_stops_even_when_ddns_is_enabled",
    "test_ap_mode_propagates_stop_failure",
    "test_invalid_callbacks_fail_without_side_effects",
):
    assert test_case in ddns_restart_test
assert "Compile and run RC DDNS restart result policy regression" in process_workflow
assert "Compile and run RC DDNS restart result policy regression" in xy_workflow
for required_path in (
    '"trunk/user/rc/src/ddns_restart_policy.h"',
    '"trunk/user/rc/src/tests/ddns_restart_policy.c"',
):
    assert required_path in process_workflow
    assert required_path in xy_workflow
print("RC DDNS restart notification preserves disabled/AP behavior and operation failures: PASS")


# VPN server restart must preserve failures through teardown, start, shared
# xl2tpd restoration and lifecycle-lock release. Both the PID-1 notification
# and direct rc applet return the same status to their caller.
vpn_restart_policy = (root / "trunk/user/rc/src/vpn_server_restart_policy.h").read_text()
vpn_restart_test = (root / "trunk/user/rc/src/tests/vpn_server_restart_policy.c").read_text()
vpn_restart_source = (root / "trunk/user/rc/src/vpn_server.c").read_text()
vpn_restart_rc = (root / "trunk/user/rc/src/rc.c").read_text()
vpn_restart_header = (root / "trunk/user/rc/src/rc.h").read_text()
vpn_restart_body = vpn_restart_source.split(
    "int" + chr(10) + "restart_vpn_server(void)", 1
)[1].split(chr(10) + "static void" + chr(10) + "reapply_vpn_server_locked", 1)[0]
assert '#include "vpn_server_restart_policy.h"' in vpn_restart_source
assert "int restart_vpn_server(void);" in vpn_restart_header
assert "void restart_vpn_server(void);" not in vpn_restart_header
assert 'restore_shared_xl2tpd_locked("after incomplete VPN teardown")' in vpn_restart_body
assert "vpn_server_restart_first_error(restart_status, start_status)" in vpn_restart_body
assert "vpn_server_restart_first_error(restart_status, restore_status)" in vpn_restart_body
assert "vpn_server_restart_first_error(restart_status, -1)" in vpn_restart_body
# A failed stop transaction may already have stopped shared xl2tpd before
# a later daemon/script/lease cleanup error. Keep the restore intent set before
# the transaction, and restore the WAN/VPN client daemon on both failure and
# normal restart paths while the lifecycle lock remains held.
vpn_stop_body = vpn_restart_source.split(
    "static int" + chr(10) + "stop_vpn_server_locked(void)", 1
)[1].split(chr(10) + "int" + chr(10) + "stop_vpn_server(void)", 1)[0]
vpn_shared_restore = vpn_restart_source.split(
    "static int" + chr(10) + "restore_shared_xl2tpd_locked(const char *phase)", 1
)[1].split(chr(10) + "int" + chr(10) + "restart_vpn_server(void)", 1)[0]
assert "xl2tpd_stop_attempted_vpns = xl2tpd_active;" in vpn_stop_body
assert vpn_stop_body.index("xl2tpd_stop_attempted_vpns = xl2tpd_active;") < vpn_stop_body.index("kill_services_with_pidfiles(")
assert "vpn_server_restart_should_restore_shared_xl2tpd(" in vpn_shared_restore
assert "restore_status = safe_start_xl2tpd_locked();" in vpn_shared_restore
assert 'restore_shared_xl2tpd_locked("after incomplete VPN teardown")' in vpn_restart_body
assert 'restore_shared_xl2tpd_locked("after VPN server restart")' in vpn_restart_body
assert vpn_restart_body.index("restore_shared_xl2tpd_locked(\"after incomplete VPN teardown\")") < vpn_restart_body.index("vpn_server_lifecycle_lock_release(lock_fd);")
assert "test_shared_xl2tpd_restoration_decision" in vpn_restart_test
assert "restore_status < 0" in vpn_restart_body
assert "restore_status != 0" not in vpn_restart_body
assert "release_status = vpn_server_lifecycle_lock_release(lock_fd);" in vpn_restart_body
assert "action_result = rc_notify_merge_action_result(action_result, restart_vpn_server());" in vpn_restart_rc
assert 'ret = restart_vpn_server();' in vpn_restart_rc
for test_case in (
    "test_success_stays_success",
    "test_start_failure_is_returned",
    "test_restore_failure_is_returned_when_start_succeeded",
    "test_first_failure_survives_later_restore_and_unlock_errors",
    "test_unlock_failure_is_returned_after_successful_operations",
    "test_positive_wait_status_is_not_lost",
):
    assert test_case in vpn_restart_test
assert "Compile and run VPN server restart status propagation regression" in process_workflow
assert "Compile and run VPN server restart status propagation regression" in xy_workflow
for required_path in (
    '"trunk/user/rc/src/vpn_server_restart_policy.h"',
    '"trunk/user/rc/src/tests/vpn_server_restart_policy.c"',
    '"trunk/user/rc/src/vpn_server.c"',
    '"trunk/user/rc/src/rc.h"',
):
    assert required_path in process_workflow
    assert required_path in xy_workflow
print("VPN server restart propagates teardown, start, restoration and unlock failures: PASS")


# Validate NVRAM LAN values and bound MiniUPnP presentation URL formatting.
upnp_url_source = (root / "trunk/user/rc/src/services_ex.c").read_text()
upnp_url_policy = (root / "trunk/user/rc/src/miniupnpd_presentation_url.h").read_text()
upnp_url_test = (root / "trunk/user/rc/src/tests/miniupnpd_presentation_url.c").read_text()
upnp_start_body = c_function_body(upnp_url_source, "static int" + chr(10) + "start_upnp_locked(void)")
assert '#include "miniupnpd_presentation_url.h"' in upnp_url_source
assert "is_valid_ipv4(lan_addr)" in upnp_start_body
assert "is_valid_ipv4(lan_mask)" in upnp_start_body
assert "miniupnpd_format_lan_url(lan_addr, ret, var, sizeof(var))" in upnp_start_body
assert "sprintf(var," not in upnp_start_body
assert "http_port < 0 || http_port > 65535" in upnp_url_policy
assert "snprintf(buffer, buffer_size" in upnp_url_policy
assert "errno = ENOSPC;" in upnp_url_policy
for test_case in (
    "test_default_ports_omit_suffix",
    "test_custom_port_is_appended",
    "test_oversized_address_fails_without_truncation",
    "test_invalid_port_and_null_address_fail",
    "test_invalid_output_buffer_fails",
    "test_short_output_buffer_is_cleared",
):
    assert test_case in upnp_url_test, test_case
print("MiniUPnP LAN URL validation and bounded formatting contract: PASS")


# UPnP restart refuses to reapply firewall rules when stop_upnp() cannot
# prove that the old daemon stopped; preserve that status to PID 1.
upnp_restart_policy = (root / "trunk/user/rc/src/upnp_restart_policy.h").read_text()
upnp_restart_test = (root / "trunk/user/rc/src/tests/upnp_restart_policy.c").read_text()
upnp_restart_source = (root / "trunk/user/rc/src/services_ex.c").read_text()
upnp_restart_rc = (root / "trunk/user/rc/src/rc.c").read_text()
upnp_restart_header = (root / "trunk/user/rc/src/rc.h").read_text()
upnp_restart_body = upnp_restart_source.split(
    "int" + chr(10) + "restart_upnp(void)", 1
)[1].split("int" + chr(10) + "ddns_updated_main", 1)[0]
assert '#include "upnp_restart_policy.h"' in upnp_restart_source
assert "int restart_upnp(void);" in upnp_restart_header
assert "void restart_upnp(void);" not in upnp_restart_header
assert "rc_upnp_restart_run(stop_upnp_locked, restart_firewall)" in upnp_restart_body
assert 'return miniupnpd_lifecycle_lock_finish(lock_fd, ret, "restart");' in upnp_restart_body
assert "return ret;" not in upnp_restart_body
assert "action_result = rc_notify_merge_action_result(action_result, restart_upnp());" in upnp_restart_rc
for test_case in (
    "test_success_stops_then_restarts_firewall",
    "test_stop_failure_aborts_firewall_restart_and_propagates",
    "test_firewall_refresh_failure_is_propagated",
    "test_stop_error_wins_when_firewall_is_not_attempted",
    "test_invalid_callbacks_fail_without_side_effects",
):
    assert test_case in upnp_restart_test
assert "Compile and run RC UPnP restart stop-result regression" in process_workflow
assert "Compile and run RC UPnP restart stop-result regression" in xy_workflow
for required_path in (
    '"trunk/user/rc/src/upnp_restart_policy.h"',
    '"trunk/user/rc/src/tests/upnp_restart_policy.c"',
    '"trunk/user/rc/src/services_ex.c"',
    '"trunk/user/rc/src/rc.h"',
):
    assert required_path in process_workflow
    assert required_path in xy_workflow
print("RC UPnP restart preserves daemon stop failure in PID 1 result: PASS")


# full_restart_lan() is a single PID-1 notification transaction: retain first
# failure from status-bearing teardown, recovery, reload and start operations.
lan_source = (root / "trunk/user/rc/src/net_lan.c").read_text()
lan_test = (root / "trunk/user/rc/src/tests/service_start_status.c").read_text()
lan_rc = (root / "trunk/user/rc/src/rc.c").read_text()
assert '#include "service_start_status.h"' in lan_source
lan_start = lan_source.index("int" + chr(10) + "full_restart_lan(void)")
lan_brace = lan_source.index("{", lan_start)
lan_depth = 0
for lan_index in range(lan_brace, len(lan_source)):
    if lan_source[lan_index] == "{":
        lan_depth += 1
    elif lan_source[lan_index] == "}":
        lan_depth -= 1
        if lan_depth == 0:
            lan_body = lan_source[lan_start:lan_index + 1]
            break
else:
    raise AssertionError("unterminated full_restart_lan()")
for contract in (
    'lan_restart_record_status(&restart_status, "stop logger", stop_logger())',
    'lan_restart_record_status(&restart_status, "stop LLTD", stop_lltd())',
    'lan_restart_record_status(&restart_status, "stop infosvr", stop_infosvr())',
    'lan_restart_record_status(&restart_status, "stop networkmap", stop_networkmap())',
    'lan_restart_record_status(&restart_status, "stop miniupnpd", stop_upnp())',
    'lan_restart_record_status(&restart_status, "stop DNSMasq", dnsmasq_status)',
    'lan_restart_record_status(&restart_status, "stop LAN services", lan_status)',
    'lan_restart_record_status(&restart_status, "start logger", start_logger(0))',
    'lan_restart_record_status(&restart_status, "start VPN server after WAN restart",',
    'lan_restart_record_status(&restart_status, "restart VPN server",',
    'lan_restart_record_status(&restart_status, "restart IPTV services",',
    'lan_restart_record_status(&restart_status, "reload NFS exports", reload_nfsd())',
    'lan_restart_record_status(&restart_status, "reload NetBIOS services", reload_nmbd())',
    'lan_restart_record_status(&restart_status, "start infosvr", start_infosvr())',
    'lan_restart_record_status(&restart_status, "start LLTD", start_lltd())',
    'lan_restart_record_status(&restart_status, "start networkmap", start_networkmap(1))',
    'lan_restart_record_status(&restart_status, "restart IAPP daemon",',
    'lan_restart_record_status(&restart_status, "notify HTTPD logout",',
    'return restart_status;',
):
    assert contract in lan_body, contract
assert "return dnsmasq_status;" not in lan_body
assert "rc_notify_merge_action_result(action_result, full_restart_lan())" in lan_rc
assert "test_first_error_survives_lan_restart_recovery" in lan_test
assert "test_negative_and_positive_statuses_are_preserved" in lan_test
print("RC full LAN restart first-error propagation and PID 1 integration: PASS")


# Router-mode bridge STP and forward-delay configuration are fallible sysfs
# writes. Both operations must be attempted and the first failure retained.
bridge_source = (root / "trunk/user/rc/src/services.c").read_text()
bridge_start = bridge_source.split("start_services_once(int is_ap_mode)", 1)[1].split(
    "stop_services(int stopall)", 1
)[0]
for status_name, call in (
    ("bridge_stp_status", "br_set_stp(IFNAME_BR, 1)"),
    ("bridge_forward_delay_status", "br_set_fd(IFNAME_BR, 15)"),
):
    assert f"{status_name} = {call};" in bridge_start, status_name
    assert f"start_status, {status_name});" in bridge_start, status_name
assert "LAN bridge STP enable failed" in bridge_start
assert "LAN bridge forward-delay configuration failed" in bridge_start
assert "return start_status;" in bridge_start
bridge_test = (root / "trunk/user/rc/src/tests/service_start_status.c").read_text()
assert "test_bridge_parameters_are_both_attempted_and_first_error_survives" in bridge_test
assert "Compile and run RC boot service status aggregation regression" in process_workflow
assert "Compile and run RC LAN restart first-error aggregation regression" in xy_workflow
print("RC bridge STP and forward-delay result propagation: PASS")

# RCN_RESTART_SYSCTL must preserve the errno-style PPPoE relay postcondition.
sysctl_common = (root / "trunk/user/rc/src/common_ex.c").read_text()
sysctl_header = (root / "trunk/user/rc/src/rc.h").read_text()
sysctl_rc = (root / "trunk/user/rc/src/rc.c").read_text()
sysctl_body = c_function_body(sysctl_common, "restart_all_sysctl(void)")
assert "int restart_all_sysctl(void)" in " ".join(sysctl_common.split())
assert '#include "service_start_status.h"' in sysctl_common
assert "passthrough_status = set_passthrough_pppoe(1);" in sysctl_body
sysctl_body_compact = "".join(sysctl_body.split())
assert "rc_first_service_start_status(restart_status,passthrough_status)" in sysctl_body_compact
assert "return restart_status;" in sysctl_body
assert "int restart_all_sysctl(void);" in sysctl_header
sysctl_dispatch = " ".join(sysctl_rc.split("RCN_RESTART_SYSCTL) == 0)", 1)[1].split(";", 1)[0].split())
assert "rc_notify_merge_action_result(" in sysctl_dispatch
assert "restart_all_sysctl()" in sysctl_dispatch
sysctl_regression = (root / "trunk/user/rc/src/tests/service_start_status.c").read_text()
assert "test_sysctl_pppoe_relay_failure_keeps_notification_pending" in sysctl_regression
assert "rc_notify_merge_action_result(action_result, restart_status)" in sysctl_regression
print("RC sysctl PPPoE relay status reaches PID 1 notification result: PASS")


# stop_lan() previously discarded clear_if_route4() status. Preserve its
# failure after conntrack cleanup while still attempting IPv6/interface teardown.
lan6_source = (root / "trunk/user/rc/src/net_lan6.c").read_text()
clear_lan_addr6 = c_function_body(lan6_source, "clear_lan_addr6(void)")
assert "return clear_if_addr6(IFNAME_BR);" in clear_lan_addr6
lan_teardown = c_function_body(lan_source, "stop_lan(int is_ap_mode)")
lan_teardown_compact = "".join(lan_teardown.split())
assert "intconntrack_status=0,route_status=0,ipv6_status=0;" in lan_teardown_compact
assert "route_status=clear_if_route4(IFNAME_BR);" in lan_teardown_compact
assert 'if(route_status!=0)logmessage(LOGNAME,"LANroutecleanupfailed(status=%d)",route_status);' in lan_teardown_compact
assert "ipv6_status=clear_lan_addr6();" in lan_teardown_compact
assert "returnrc_first_service_start_status(rc_first_service_start_status(rc_first_service_start_status(conntrack_status,route_status),ipv6_status),status);" in lan_teardown_compact
route_flush_at = lan_teardown_compact.index("route_status=clear_if_route4(IFNAME_BR);")
ipv6_cleanup_at = lan_teardown_compact.index("ipv6_status=clear_lan_addr6();")
bridge_down_at = lan_teardown_compact.index("ifconfig(IFNAME_BR,0,NULL,NULL);")
assert route_flush_at < ipv6_cleanup_at < bridge_down_at
assert "test_router_lan_teardown_preserves_route_failure_and_continues" in lan_test
assert "test_router_lan_teardown_preserves_route_failure_and_continues();" in lan_test
assert "test_router_lan_teardown_preserves_ipv6_clear_failure_and_continues" in lan_test
assert "test_router_lan_teardown_preserves_ipv6_clear_failure_and_continues();" in lan_test
print("LAN route flush errno reaches stop_lan() result without skipping later cleanup: PASS")


net6_source = (root / "trunk/user/rc/src/net6.c").read_text()
rc_boot_source = (root / "trunk/user/rc/src/rc.c").read_text()
rc_header_source = (root / "trunk/user/rc/src/rc.h").read_text()
ipv6_init_body = c_function_body(net6_source, "int init_ipv6(void)")
assert "control_if_ipv6_all((ipv6_type == IPV6_DISABLED) ? 0 : 1)" in ipv6_init_body
assert "if (conf_status != 0)" in ipv6_init_body
assert ipv6_init_body.index("reset_lan6_vars();") < ipv6_init_body.index("return conf_status;")
assert "int init_ipv6(void);" in rc_header_source
assert "void init_ipv6(void);" not in rc_header_source

router_init_body = c_function_body(rc_boot_source, "init_router(void)")
router_init_compact = re.sub(r"\s+", " ", router_init_body)
assert "int ipv6_init_status = init_ipv6();" in router_init_compact
assert (
    "boot_services_status = rc_first_service_start_status( "
    "boot_services_status, ipv6_init_status);"
) in router_init_compact, "init_router must preserve IPv6 global sysctl failure in its first-error boot status"
assert router_init_body.index("ipv6_init_status = init_ipv6()") < router_init_body.index("set_cpu_affinity();")
assert '"trunk/user/rc/src/net6.c"' in process_workflow
assert '"trunk/user/rc/src/rc.c"' in process_workflow
assert '"trunk/user/rc/src/rc.h"' in process_workflow
assert '"trunk/user/rc/src/net6.c"' in xy_workflow
assert '"trunk/user/rc/src/rc.c"' in xy_workflow
assert '"trunk/user/rc/src/rc.h"' in xy_workflow
print("IPv6 boot sysctl initialization status aggregation: PASS")

# BusyBox syslogd/klogd detach before service operation; verify the child.
syslogd_start = services_source.split("int\nstart_syslogd(void)", 1)[1].split("\n}", 1)[0]
klogd_start = services_source.split("int\nstart_klogd(void)", 1)[1].split("\n}", 1)[0]

assert 'start_status = optional_daemon_start_status("syslogd", "syslogd");' in syslogd_start
assert "start_status = _eval(syslogd_argv, NULL, 0, NULL);" in syslogd_start
assert "PROCESS_PRESENCE_START_INITIAL_WAIT_SECONDS" in syslogd_start
assert "PROCESS_PRESENCE_START_PROBE_ATTEMPTS" in syslogd_start
assert 'optional_daemon_presence("syslogd", &scan_error)' in syslogd_start
assert "process_presence_start_postcondition(start_status, process_state)" in syslogd_start
assert "errno = scan_error > 0 ? scan_error : EIO;" in syslogd_start
assert "errno = ETIMEDOUT;" in syslogd_start
assert "if (start_status > 0)" in syslogd_start and "if (start_status < 0)" in syslogd_start
syslog_preflight = syslogd_start.index('optional_daemon_start_status("syslogd", "syslogd")')
syslog_launch = syslogd_start.index("start_status = _eval(syslogd_argv, NULL, 0, NULL);")
syslog_wait = syslogd_start.index("sleep(PROCESS_PRESENCE_START_INITIAL_WAIT_SECONDS);", syslog_launch)
syslog_probe = syslogd_start.index("for (attempt = 0; attempt < PROCESS_PRESENCE_START_PROBE_ATTEMPTS; attempt++)", syslog_launch)
syslog_inventory = syslogd_start.index('optional_daemon_presence("syslogd", &scan_error)', syslog_probe)
syslog_result = syslogd_start.index("process_presence_start_postcondition(start_status, process_state)", syslog_inventory)
assert syslog_preflight < syslog_launch < syslog_wait < syslog_probe < syslog_inventory < syslog_result
assert "if (!(opts & OPT_nofork))" in syslogd_source
assert syslogd_source.index("bb_daemonize_or_rexec(DAEMON_CHDIR_ROOT, argv)") < syslogd_source.index('write_pidfile(CONFIG_PID_FILE_PATH "/syslogd.pid")') < syslogd_source.index("do_syslogd()")
assert '"-n"' not in syslogd_start

assert 'start_status = optional_daemon_start_status("klogd", "klogd");' in klogd_start
assert 'start_status = eval("/sbin/klogd");' in klogd_start
assert "PROCESS_PRESENCE_START_INITIAL_WAIT_SECONDS" in klogd_start
assert "PROCESS_PRESENCE_START_PROBE_ATTEMPTS" in klogd_start
assert 'optional_daemon_presence("klogd", &scan_error)' in klogd_start
assert "process_presence_start_postcondition(start_status, process_state)" in klogd_start
assert "errno = scan_error > 0 ? scan_error : EIO;" in klogd_start
assert "errno = ETIMEDOUT;" in klogd_start
assert "if (start_status > 0)" in klogd_start and "if (start_status < 0)" in klogd_start
klog_preflight = klogd_start.index('optional_daemon_start_status("klogd", "klogd")')
klog_launch = klogd_start.index('start_status = eval("/sbin/klogd");')
klog_wait = klogd_start.index("sleep(PROCESS_PRESENCE_START_INITIAL_WAIT_SECONDS);", klog_launch)
klog_probe = klogd_start.index("for (attempt = 0; attempt < PROCESS_PRESENCE_START_PROBE_ATTEMPTS; attempt++)", klog_launch)
klog_inventory = klogd_start.index('optional_daemon_presence("klogd", &scan_error)', klog_probe)
klog_result = klogd_start.index("process_presence_start_postcondition(start_status, process_state)", klog_inventory)
assert klog_preflight < klog_launch < klog_wait < klog_probe < klog_inventory < klog_result
assert "bb_daemonize_or_rexec(DAEMON_CHDIR_ROOT, argv)" in klogd_source
assert klogd_source.index("bb_daemonize_or_rexec(DAEMON_CHDIR_ROOT, argv)") < klogd_source.index('openlog("kernel"')
print("syslogd/klogd detached startup postconditions and BusyBox contracts: PASS")
