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
lldt_pidfile_policy = (root / "trunk/user/lldt/src/lldt_pidfile_policy.h").read_text()
lldt_pidfile_test = (root / "trunk/user/lldt/src/tests/lldt_pidfile_policy.c").read_text()
lldt_config_path_policy = (root / "trunk/user/lldt/src/lldt_config_path_policy.h").read_text()
lldt_config_parse_policy = (root / "trunk/user/lldt/src/lldt_config_parse_policy.h").read_text()
lldt_config_path_test = (root / "trunk/user/lldt/src/tests/lldt_config_path_policy.c").read_text()
shared_makefile = (root / "trunk/user/shared/src/Makefile").read_text()
netutils_source = (root / "trunk/user/shared/src/netutils.c").read_text()
net_ifstats_policy = (root / "trunk/user/shared/src/net_ifstats_policy.h").read_text()
net_ifstats_test = (root / "trunk/user/shared/src/tests/net_ifstats_policy.c").read_text()
shared_header = (root / "trunk/user/shared/src/shutils.h").read_text()
shared_source = (root / "trunk/user/shared/src/shutils.c").read_text()
shared_pidfile_policy = (root / "trunk/user/shared/src/shutils_pidfile_policy.h").read_text()
shared_pidfile_test = (root / "trunk/user/shared/src/tests/shutils_pidfile_policy.c").read_text()
shared_pidfile_open_test = (root / "trunk/user/shared/tests/shutils_pidfile_open.c").read_text()
pids_source = (root / "trunk/user/shared/src/pids.c").read_text()
process_inventory = (root / "trunk/user/shared/src/process_inventory.c").read_text()
process_inventory_test = (root / "trunk/user/shared/src/tests/process_inventory.c").read_text()
pids_presence_policy = (root / "trunk/user/shared/src/pids_presence_policy.h").read_text()
pids_presence_test = (root / "trunk/user/shared/src/tests/pids_presence_policy.c").read_text()
common_ex = (root / "trunk/user/rc/src/common_ex.c").read_text()
module_refcount_policy = (root / "trunk/user/rc/src/module_refcount_policy.h").read_text()
module_refcount_test = (root / "trunk/user/rc/src/tests/module_refcount_policy.c").read_text()
char_to_ascii_policy = (root / "trunk/user/rc/src/char_to_ascii_policy.h").read_text()
char_to_ascii_test = (root / "trunk/user/rc/src/tests/char_to_ascii_policy.c").read_text()
policy = (root / "trunk/user/rc/src/service_lifecycle_policy.h").read_text()
usb_services = (root / "trunk/user/rc/src/services_usb.c").read_text()
usb_modem = (root / "trunk/user/rc/src/usb_modem.c").read_text()
rc_header = (root / "trunk/user/rc/src/rc.h").read_text()
lifecycle_test = (root / "trunk/user/rc/src/tests/service_lifecycle_policy.c").read_text()
net_lan_notification = (root / "trunk/user/rc/src/net_lan.c").read_text()
bridge_init_regression = (root / "trunk/user/rc/src/tests/bridge_init_policy.c").read_text()
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
legacy_pidfile_body = c_function_body(
    shared_source, "kill_pidfile_s(char *pidfile, int sig)"
)
assert "#include \"shutils_pidfile_policy.h\"" in shared_source
assert "shutils_pidfile_open(pidfile)" in legacy_pidfile_body
assert "O_NOFOLLOW" in shared_pidfile_policy
assert "O_NONBLOCK" in shared_pidfile_policy
assert "fstat(fd, &st)" in legacy_pidfile_body
assert "shutils_pidfile_metadata_is_safe(&st, geteuid())" in legacy_pidfile_body
assert "S_ISREG(st.st_mode)" not in legacy_pidfile_body
for token in ("(status->st_mode & S_IFMT) == S_IFREG", "status->st_uid == owner", "status->st_nlink == 1", "(status->st_mode & 022) == 0"):
    assert token in shared_pidfile_policy, token
assert "metadata_policy_rejects_unsafe_files" in shared_pidfile_open_test
assert "fdopen(fd, \"r\")" in legacy_pidfile_body
assert "shutils_pidfile_parse_record(buf, length, &pid)" in legacy_pidfile_body
assert "fgets(" not in legacy_pidfile_body
assert "strtoul(" not in legacy_pidfile_body
assert "value > (unsigned long)INT_MAX" in shared_pidfile_policy
assert 'expect_valid("052", 3' in shared_pidfile_test
assert 'expect_invalid("2junk", 5, EINVAL)' in shared_pidfile_test
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
        'if (strcmp(apps_enabled, "1") != 0)',
        "Media Server",
        "is_dms_run()",
    ),
    (
        "int run_itunes(void)",
        "void restart_itunes(void)",
        'launch_status = eval("/usr/bin/mt-daapd", "-c", conf_new);',
        'storage_service_enabled_status(\n\t\t"apps_itunes", "iTunes Server", &service_enabled);',
        "iTunes Server",
        "is_itunes_run()",
    ),
    (
        "int run_torrent(void)",
        "void restart_torrent(void)",
        'launch_status = eval("/usr/bin/transmission.sh", "start");',
        'storage_service_enabled_status(\n\t\t"trmd_enable", "Transmission", &service_enabled);',
        "Transmission",
        "is_torrent_run()",
    ),
    (
        "int run_aria(void)",
        "void restart_aria(void)",
        'launch_status = eval("/usr/bin/aria.sh", "start");',
        'storage_service_enabled_status(\n\t\t"aria_enable", "Aria", &service_enabled);',
        "Aria",
        "is_aria_run()",
    ),
)
for start_marker, end_marker, launcher, enable_guard, service_name, presence_call in storage_launch_contracts:
    start_body = storage_services.split(start_marker, 1)[1].split(chr(10) + end_marker, 1)[0]
    assert enable_guard in start_body
    normalized_enable_start = " ".join(start_body.split())
    assert "if (enable_status != 0)" in start_body
    assert "return enable_status;" in start_body
    if start_marker == "int run_dms(int force_rescan)":
        dms_enable_read = 'storage_read_string_status( "apps_dms", apps_enabled, sizeof(apps_enabled));'
        assert dms_enable_read in normalized_enable_start
        assert normalized_enable_start.index("if (enable_status != 0)") < normalized_enable_start.index('strcmp(apps_enabled, "1")') < normalized_enable_start.index("if (!is_dms_support())")
    else:
        assert "if (!service_enabled)" in start_body
        assert normalized_enable_start.index("storage_service_enabled_status(") < normalized_enable_start.index("if (enable_status != 0)")
        assert normalized_enable_start.index("if (enable_status != 0)") < normalized_enable_start.index("if (!service_enabled)") < normalized_enable_start.index("if (!is_")
    assert "int launch_status;" in start_body


    assert "if (process_state == PROCESS_PRESENCE_RUNNING)" in start_body
    assert "if (process_state != PROCESS_PRESENCE_ABSENT)" in start_body
    assert launcher in start_body
    normalized_start_body = " ".join(start_body.split())
    normalized_launcher = " ".join(launcher.split())
    start_result_call = "storage_log_daemon_start_result( apps_name, launch_status,"
    assert start_result_call in normalized_start_body
    assert normalized_start_body.index(normalized_launcher) < normalized_start_body.index(start_result_call)
    post_launch = start_body.split(launcher, 1)[1]
    assert "process_state = " + presence_call + ";" in post_launch
    assert service_name in start_body

dms_body = storage_services.split("int run_dms(int force_rescan)", 1)[1].split(
    chr(10) + "int restart_dms(int force_rescan)", 1
)[0]
dms_compact = "".join(dms_body.split())
dms_enable_read = 'storage_read_string_status("apps_dms",apps_enabled,sizeof(apps_enabled))'
dms_enable_error = "if(enable_status!=0)"
dms_enable_match = 'if(strcmp(apps_enabled,"1")!=0)'
dms_enable_failure_return = "returnenable_status;"
assert dms_enable_read in dms_compact
assert dms_enable_error in dms_compact
assert dms_enable_match in dms_compact
assert dms_enable_failure_return in dms_compact
assert dms_compact.index(dms_enable_read) < dms_compact.index(dms_enable_error)
assert dms_compact.index(dms_enable_error) < dms_compact.index(dms_enable_match)
assert dms_compact.index(dms_enable_failure_return) < dms_compact.index("is_dms_support()")
assert 'nvram_match("apps_dms","1")' not in dms_compact

start_result = storage_services.split("storage_log_daemon_start_result(", 1)[1].split(
    chr(10) + "#if defined (APP_FTPD)", 1
)[0]
assert "if (launch_status != 0)" in start_result
assert "#if defined (APP_MINIDLNA) || defined (APP_FIREFLY) || defined (APP_TRMD) || defined (APP_ARIA)" in storage_services
assert "PROCESS_PRESENCE_RUNNING" in start_result
assert "PROCESS_PRESENCE_ABSENT" in start_result
assert "cannot verify daemon startup (scan error=%d)" in start_result
assert "process_error > 0 ? process_error : EIO" in start_result
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
normalized_minidlna_post_launch = " ".join(minidlna_post_launch.split())
minidlna_logger = "storage_log_daemon_start_result( apps_name, launch_status,"
assert minidlna_logger in normalized_minidlna_post_launch
assert "(enum process_presence)process_state, process_error);" in normalized_minidlna_post_launch
assert 'storage_wait_for_daemon("minidlna")' in normalized_minidlna_post_launch
assert "process_error == ETIMEDOUT" in normalized_minidlna_post_launch
assert normalized_minidlna_post_launch.index("process_state = is_dms_run();") < normalized_minidlna_post_launch.index(minidlna_logger)
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
firefly_start = storage_services.split("int run_itunes(void)", 1)[1].split("void restart_itunes(void)", 1)[0]
normalized_firefly_start = " ".join(firefly_start.split())
assert 'storage_wait_for_daemon("mt-daapd")' in normalized_firefly_start
assert "process_error == ETIMEDOUT" in normalized_firefly_start
assert "storage_log_daemon_start_result( apps_name, launch_status," in normalized_firefly_start
assert "(enum process_presence)process_state, process_error);" in normalized_firefly_start
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
assert 'strchr("RSDZTtXxKWPI", cursor[0])' in process_inventory
assert "state == 'Z' || state == 'X' || state == 'x'" in process_inventory
assert "test_proc_state_codes_are_validated" in process_inventory_test

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
assert "service_wait_seconds(1)" in pppoe_confirm
assert "wait_status = errno > 0 ? errno : EIO;" in pppoe_confirm
assert "return wait_status;" in pppoe_confirm
assert "sleep(1);" not in pppoe_confirm
assert "pppoe_relay_start_status(launch_status, presence)" in pppoe_confirm
udpxy_start = net_source.split("int\nstart_udpxy(char *wan_ifname)", 1)[1].split("\n#if defined(APP_XUPNPD)", 1)[0]
assert "rc_wan_nvram_copy_value(net_nvram_get_checked, NULL," in udpxy_start
assert '"udpxy_enable_x"' in udpxy_start
assert "rc_wan_nvram_parse_int_value(port_text, &port)" in udpxy_start
assert 'nvram_get_int("udpxy_enable_x")' not in udpxy_start
checked_port = udpxy_start.index('"udpxy_enable_x"')
disabled_gate = udpxy_start.index("if (port < 1024)")
process_preflight = udpxy_start.index("iptv_proxy_process_presence((void *)process_name, &scan_error)")
assert checked_port < disabled_gate < process_preflight
assert "cannot read IPTV proxy port selector" in udpxy_start
assert "if (port > 65535)" in udpxy_start
assert 'process_name = "msd_lite";' in udpxy_start
assert 'process_name = "udpxy";' in udpxy_start
assert "iptv_proxy_process_presence((void *)process_name, &scan_error)" in udpxy_start
assert "process_state == PROCESS_PRESENCE_UNKNOWN" in udpxy_start
assert "process_state != PROCESS_PRESENCE_ABSENT" in udpxy_start
assert 'launch_status = eval("/usr/bin/msd_lite"' in udpxy_start
assert 'launch_status = eval("/usr/sbin/udpxy"' in udpxy_start
# APP_MSD_LITE must publish its enable state successfully before daemon launch.
assert 'marker_api_result = nvram_set("msd_lite_enable", "1");' in udpxy_start
assert "rc_wan_nvram_write_status(marker_api_result, marker_errno)" in udpxy_start
assert "if (marker_status != 0)" in udpxy_start
assert "cannot publish MSD Lite enable state; refusing daemon startup" in udpxy_start
assert "return marker_status;" in udpxy_start
assert udpxy_start.index('nvram_set("msd_lite_enable", "1")') < udpxy_start.index('eval("/usr/bin/msd_lite"')
assert udpxy_start.index('if (marker_status != 0)') < udpxy_start.index('eval("/usr/bin/msd_lite"')
assert "launch_status = iptv_proxy_confirm_started(process_name, launch_status);" in udpxy_start
assert '"%s launch/readiness failed (status=%d errno=%d)"' in udpxy_start
assert udpxy_start.count("if (launch_status != 0)") == 1
preflight = udpxy_start.index("iptv_proxy_process_presence((void *)process_name, &scan_error)")
assert preflight < udpxy_start.index("msd_lite_config_publish(")
assert preflight < udpxy_start.index('eval("/usr/bin/msd_lite"')
assert preflight < udpxy_start.index('eval("/usr/sbin/udpxy"')
assert "return launch_status;" in udpxy_start
assert "int start_udpxy(char *wan_ifname);" in rc_header
udpxy_start = c_function_body(net_source, "start_udpxy(char *wan_ifname)")
assert "errno = 0;" in udpxy_start
assert udpxy_start.index("errno = 0;") < udpxy_start.index("eval(")

rollback_helper = c_function_body(net_source, "iptv_proxy_rollback_failed_start(")
rollback_stop = c_function_body(net_source, "iptv_proxy_process_stop(")
assert "detached_daemon_start_rollback_with_ops(" in rollback_helper
assert "iptv_proxy_process_presence" in rollback_helper
assert "PROCESS_PRESENCE_ABSENT" in rollback_helper
assert "kill_services(service_names, 3, 1)" in rollback_stop
assert "service_names[0] = (char *)context;" in rollback_stop
for signature, confirm_name, process_name in (
    ("start_udpxy(char *wan_ifname)", "iptv_proxy_confirm_started(", "process_name"),
    ("start_xupnpd(char *wan_ifname)", "xupnpd_confirm_started(", '"xupnpd"'),
):
    body = c_function_body(net_source, signature)
    assert confirm_name in body
    assert "iptv_proxy_rollback_failed_start(" in body
    assert process_name in body

# IGMPproxy's lifecycle now lives in the snapshot-aware helper; the public
# wrapper only validates arguments, takes one NVRAM snapshot, and delegates.
igmpproxy_start = c_function_body(
    net_source, "start_igmpproxy_with_settings(char *wan_ifname")
igmpproxy_wrapper = c_function_body(net_source, "start_igmpproxy(char *wan_ifname)")
assert 'iptv_proxy_confirm_started("igmpproxy", launch_status)' in igmpproxy_start
assert "iptv_proxy_rollback_failed_start(" in igmpproxy_start
assert '"igmpproxy"' in igmpproxy_start
assert "net_iptv_proxy_nvram_settings_read(&settings)" in igmpproxy_wrapper
assert "start_igmpproxy_with_settings(wan_ifname, &settings)" in igmpproxy_wrapper

xupnpd_start = net_source.split("int\nstart_xupnpd(char *wan_ifname)", 1)[1].split("\n#endif", 1)[0]
assert "if (!is_xupnpd_support())" in xupnpd_start
assert "char tmp1[128], tmp2[128], line[256], lan_addr[16], xport_text[16];" in xupnpd_start
assert "rc_wan_nvram_parse_int_value(xport_text, &xport)" in xupnpd_start
assert "xupnpd_process_presence(NULL, &scan_error)" in xupnpd_start
assert "rc_wan_nvram_copy_value(net_nvram_get_checked, NULL," in xupnpd_start
assert '"xupnpd_enable_x"' in xupnpd_start
assert "rc_wan_nvram_parse_int_value(xport_text, &xport)" in xupnpd_start
assert 'nvram_get_int("xupnpd_enable_x")' not in xupnpd_start
checked_xupnpd_port = xupnpd_start.index('"xupnpd_enable_x"')
xupnpd_disabled_gate = xupnpd_start.index("if (xport < 1024)")
xupnpd_lan_snapshot = xupnpd_start.index('"lan_ipaddr_t"')
xupnpd_preflight = xupnpd_start.index("xupnpd_process_presence(NULL, &scan_error)")
assert checked_xupnpd_port < xupnpd_disabled_gate < xupnpd_lan_snapshot < xupnpd_preflight
assert "cannot read xupnpd enable-port selector" in xupnpd_start
assert "if (xport > 65535)" in xupnpd_start
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
assert "rc_wan_nvram_copy_value(net_nvram_get_checked, NULL," in xupnpd_start
assert '"lan_ipaddr_t"' in xupnpd_start
assert "xupnpd_config_valid_ipv4(lan_addr)" in xupnpd_start
lua_udpxy = xupnpd_start.split('else if (strstr(line, "cfg.udpxy_url="))', 1)[1].split('else if (strstr(line, "cfg.daemon")', 1)[0]
assert '"udpxy_enable_x"' in lua_udpxy
assert "rc_wan_nvram_parse_int_value(udpxy_port_text, &udpxy_port)" in lua_udpxy
assert '"xupnpd_udpxy"' in lua_udpxy
assert 'strcmp(udpxy_enable_text, "1") == 0' in lua_udpxy
assert 'nvram_get_int("udpxy_enable_x")' not in lua_udpxy
assert 'nvram_match("xupnpd_udpxy", "1")' not in lua_udpxy
assert "cannot read IPTV proxy port while preparing xupnpd configuration" in lua_udpxy
assert "cannot read xupnpd IPTV proxy integration setting" in lua_udpxy
assert lua_udpxy.index('"udpxy_enable_x"') < lua_udpxy.index("rc_wan_nvram_parse_int_value")
assert lua_udpxy.index("rc_wan_nvram_parse_int_value") < lua_udpxy.index('"xupnpd_udpxy"')
assert 'nvram_safe_get("lan_ipaddr_t")' not in xupnpd_start
xupnpd_start_compact = " ".join(xupnpd_start.split())
assert "xupnpd_config_tempfile_open(common_path, common_stage, sizeof(common_stage))" in xupnpd_start_compact
assert "xupnpd_config_tempfile_open(runtime_path, runtime_stage, sizeof(runtime_stage))" in xupnpd_start_compact
assert "xupnpd_config_tempfile_open(tmp2, tmp1, sizeof(tmp1))" not in xupnpd_start
assert 'fopen(tmp1, "w")' not in xupnpd_start
xupnpd_config_header = (root / "trunk/user/rc/src/xupnpd_config_file.h").read_text()
xupnpd_config_test = (root / "trunk/user/rc/src/tests/xupnpd_config_file.c").read_text()
assert "mkstemp(temporary)" in xupnpd_config_header
assert "lstat(destination, &st)" in xupnpd_config_header
assert "fchmod(fd, mode)" in xupnpd_config_header
assert "xupnpd_config_source_open(common_path)" in xupnpd_start
assert "xupnpd_config_source_open(runtime_path)" in xupnpd_start
assert "xupnpd_config_source_open(tmp2)" not in xupnpd_start
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
assert "fsync(fileno(fp1))" in xupnpd_start and "fsync(fileno(fp3))" in xupnpd_start
assert "dnsmasq_file_publish_pair(" in xupnpd_start
assert "rename(tmp1, tmp2)" not in xupnpd_start
assert "dnsmasq_file_discard(common_stage)" in xupnpd_start
assert "dnsmasq_file_discard(runtime_stage)" in xupnpd_start
assert '#include "dnsmasq_file.h"' in net_source
assert xupnpd_start.index("if (has_daemon)") < xupnpd_start.index('launch_status = eval("/usr/bin/xupnpd");')
assert xupnpd_start.index('launch_status = system("/usr/bin/xupnpd &");') < xupnpd_start.index("if (launch_status != 0)")
assert '#include "service_start_status.h"' in net_source
assert "rc_first_service_start_status(start_status, start_udpxy(wan_ifname))" in igmpproxy_start
assert "rc_first_service_start_status(start_status, start_xupnpd(wan_ifname))" in igmpproxy_start
assert "rc_first_service_start_status(start_status, config_status)" in igmpproxy_start
assert 'eval("/bin/igmpproxy", igmpproxy_conf)' in igmpproxy_start
assert 'iptv_proxy_process_presence((void *)"igmpproxy", &scan_error)' in igmpproxy_start
assert "PROCESS_PRESENCE_UNKNOWN" in igmpproxy_start
assert "process_state != PROCESS_PRESENCE_ABSENT" in igmpproxy_start
assert igmpproxy_start.index("iptv_proxy_process_presence(") < igmpproxy_start.index("igmpproxy_config_publish(")
assert "iptv_proxy_confirm_started(\"igmpproxy\", launch_status)" in igmpproxy_start
assert "return rc_first_service_start_status(start_status, launch_status);" in igmpproxy_start
assert "net_iptv_proxy_nvram_settings_read(&settings)" in igmpproxy_wrapper
assert "start_igmpproxy_with_settings(wan_ifname, &settings)" in igmpproxy_wrapper
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
wan_full_stop = net_wan.split("static int\nstop_wan_locked(void)", 1)[1].split("\n}", 1)[0]
wan_lock_runner = net_wan.split("run_wan_stop_under_vpn_lifecycle_lock(int (*operation)(void), const char *name)", 1)[1].split("\n}", 1)[0]
for stop_body in (wan_ppp_stop, wan_full_stop):
    assert "pause_status = notify_pause_detect_internet();" in stop_body
    pause_call = stop_body.index("pause_status = notify_pause_detect_internet();")
    pause_gate = stop_body.index("if (pause_status != 0)", pause_call)
    pause_return = stop_body.index("return pause_status;", pause_gate)
    callback_fence = stop_body.index("invalidate_wan_ppp_callback_session()")
    assert pause_call < pause_gate < pause_return < callback_fence
    assert "stop_vpn_client() != 0" not in stop_body
    assert "kill_services(svcs_ppp, 6, 1)" in stop_body
assert "stop_status = stop_vpn_client_under_lifecycle_lock();" in wan_ppp_stop
assert "stop_status = kill_services(svcs_ppp, 6, 1);" in wan_ppp_stop
assert wan_ppp_stop.index("stop_status = stop_vpn_client_under_lifecycle_lock();") < wan_ppp_stop.index("stop_status = kill_services(svcs_ppp, 6, 1);")
assert "stop_status = invalidate_wan_ppp_callback_session();" in wan_full_stop
assert "stop_status = stop_vpn_client_under_lifecycle_lock();" in wan_full_stop
assert "stop_status = kill_services(svcs_ppp, 6, 1);" in wan_full_stop
assert "if (stop_status != 0)" in wan_full_stop
assert wan_full_stop.index("stop_status = invalidate_wan_ppp_callback_session();") < wan_full_stop.index("stop_status = stop_vpn_client_under_lifecycle_lock();") < wan_full_stop.index("stop_status = kill_services(svcs_ppp, 6, 1);")
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
assert shutdown.index("shutdown_script_status = doSystem(") < shutdown.index("storage_remove_status == 0")
assert shutdown.index("logger_stop_status = stop_logger();") < shutdown.index("storage_remove_status == 0")
assert shutdown.index("storage_remove_status == 0") < shutdown.index("write_storage_to_mtd()")
assert "logger_stop_status != 0" in syslog_restart
assert syslog_restart.index("logger_stop_status != 0") < syslog_restart.index("start_logger(0)")
assert "action_result = -1;" in syslog_restart
assert "logger_stop_status == 0 && start_logger(0) != 0" in time_restart
assert "logger_stop_status != 0" in time_restart

logger_policy = (root / "trunk/user/rc/src/logger_start_policy.h").read_text()
logger_test = (root / "trunk/user/rc/src/tests/logger_start_policy.c").read_text()
logger_start = services_source.split("int\nstart_logger(int showinfo)", 1)[1].split("\n}\n", 1)[0]
syslogd_start = services_source.split("int\nstart_syslogd(void)", 1)[1].split("\nint\nstart_klogd(void)", 1)[0]
assert '#include "logger_start_policy.h"' in services_source
assert "syslog_status = start_syslogd();" in logger_start
assert "klog_status = start_klogd();" in logger_start
assert logger_start.index("syslog_status = start_syslogd();") < logger_start.index("klog_status = start_klogd();")
assert "nvram_transaction_begin(&log_transaction)" in syslogd_start
assert 'nvram_transaction_get(&log_transaction, "log_ipaddr", &nvram_value)' in syslogd_start
assert 'nvram_transaction_get(&log_transaction, "log_port", &nvram_value)' in syslogd_start
assert "rc_logger_nvram_address_status(" in syslogd_start
assert "rc_logger_nvram_port_status(" in syslogd_start
assert "nvram_transaction_end(&log_transaction);" in syslogd_start
assert syslogd_start.index('"log_ipaddr"') < syslogd_start.index('"log_port"')
assert syslogd_start.index("nvram_transaction_end(&log_transaction);") < syslogd_start.index("_eval(syslogd_argv")
assert "if (remote_config_status != 0)" in syslogd_start
assert "log_ipaddr[0] = '\\0';" in syslogd_start
assert "return remote_config_status;" in syslogd_start
assert syslogd_start.index("process_presence_start_postcondition") < syslogd_start.index("return remote_config_status;")
assert 'nvram_safe_get("log_ipaddr")' not in syslogd_start
assert 'nvram_safe_get_int("log_port"' not in syslogd_start
assert 'logmessage(LOGNAME, "syslogd startup/configuration incomplete (status=%d)", syslog_status);' in logger_start
assert 'logmessage(LOGNAME, "klogd startup failed (status=%d)", klog_status);' in logger_start
assert "return rc_logger_start_status_with_wait(syslog_status, settle_status, klog_status);" in logger_start
assert "service_wait_microseconds(300000U)" in logger_start
assert "usleep(300000)" not in logger_start
assert "settle_status = errno > 0 ? errno : EIO;" in logger_start
assert "rc_logger_start_status_with_wait" in logger_policy
assert "rc_logger_nvram_address_status" in logger_policy
assert "rc_logger_nvram_port_status" in logger_policy
assert "return 0;" not in logger_start
assert "return syslog_status != 0 ? syslog_status : klog_status;" in logger_policy
for contract in (
    "rc_logger_start_status(0, 0) == 0",
    "rc_logger_start_status(17, 0) == 17",
    "rc_logger_start_status(0, 23) == 23",
    "rc_logger_start_status(17, 23) == 17",
    "rc_logger_start_status(-1, 0) == -1",
    "rc_logger_start_status(0, -1) == -1",
    'rc_logger_nvram_address_status(-1, EACCES, "ignored"',
    'rc_logger_nvram_port_status(-1, EIO, "1514"',
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
assert "pause_status = notify_pause_detect_internet();" in lan_stop
lan_pause_call = lan_stop.index("pause_status = notify_pause_detect_internet();")
lan_pause_failure = lan_stop.index("if (pause_status != 0)", lan_pause_call)
lan_pause_return = lan_stop.index("return pause_status;", lan_pause_failure)
lan_dhcp_stop_call = lan_stop.index("status = stop_udhcpc_lan();")
assert lan_pause_call < lan_pause_failure < lan_pause_return < lan_dhcp_stop_call
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
man_deconfig = c_function_body(
    net_wan, "udhcpc_deconfig(char *wan_ifname, int is_zcip)"
)
assert 'rc_wan_nvram_copy_value(' in man_deconfig
assert 'wan_nvram_checked_get_callback, NULL, "wanx_ipaddr"' in man_deconfig
assert 'nvram_safe_get("wanx_ipaddr")' not in man_deconfig
assert "int man_addr_status;" in man_deconfig
assert "conntrack_status = man_addr_status;" in man_deconfig
assert "cannot read MAN old address for conntrack cleanup" in man_deconfig
man_deconfig_compact = "".join(man_deconfig.split())
assert (
    "route_status_accumulate(route_status_accumulate("
    "route_status_accumulate(proto_status,interface_status),conntrack_status),cleanup_status)"
) in man_deconfig_compact
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
wan_start_compact = " ".join(wan_start.split())
assert "auth_status = nvram_status;" in wan_start_compact
assert "if (nvram_status == 0 && wan_auth_mode > 1)" in wan_start_compact
assert "auth_status = start_auth_eapol(" in wan_start_compact
assert (
    wan_start_compact.index("auth_status = nvram_status;")
    < wan_start_compact.index("auth_status = start_auth_eapol(")
    < wan_start_compact.index("if (auth_status != 0)")
)
wan_eapol_failure = wan_start.split("if (auth_status != 0)", 1)[1].split("/* We are done configuration */", 1)[0]
wan_eapol_compact = "".join(wan_eapol_failure.split())
assert "vlan_result=set_wan_unit_error_status(unit,1);" in wan_eapol_compact
assert "wan_start_status=rc_first_service_start_status(wan_start_status,vlan_result);" in wan_eapol_compact
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

usb_modem_policy = (root / "trunk/user/shared/src/usb_modem_wan_policy.h").read_text()
assert '#include "usb_modem_wan_policy.h"' in netutils_source
assert "usb_modem_wan_devnum_matches_status(" in usb_modem_policy
modem_identity_reader = c_function_body(
    netutils_source,
    "get_usb_modem_dev_wan_status(int unit, int devnum, int *is_selected)"
)
legacy_modem_identity_reader = c_function_body(
    netutils_source, "get_usb_modem_dev_wan(int unit, int devnum)"
)
assert "nvram_get_checked(key, &value)" in modem_identity_reader
assert "usb_modem_wan_devnum_matches_status(" in modem_identity_reader
assert "errno = status;" in modem_identity_reader and "return status;" in modem_identity_reader
assert "get_usb_modem_dev_wan_status(unit, devnum, &is_selected)" in legacy_modem_identity_reader
assert "nvram_get_int(" not in legacy_modem_identity_reader

usb_remove = usb_services.split("int\nsafe_remove_usb_device(int port, const char *dev_name)", 1)[1].split("\n}\n", 1)[0]
assert "storage_hotplug_parse_decimal_token(dev_name, 1, INT_MAX" in usb_remove
assert "if (!has_modem_port)" in usb_remove
assert "modem_identity_status = get_usb_modem_dev_wan_status(" in usb_remove
assert "if (modem_identity_status != 0)" in usb_remove
assert "errno = modem_identity_status;" in usb_remove
assert "if (!is_selected_modem)" in usb_remove
assert "cannot verify selected USB modem identity before eject" in usb_remove
identity_read = usb_remove.index("modem_identity_status = get_usb_modem_dev_wan_status(")
identity_error = usb_remove.index("if (modem_identity_status != 0)", identity_read)
identity_mismatch = usb_remove.index("if (!is_selected_modem)", identity_error)
modem_remove_call = usb_remove.index("modem_remove_status = safe_remove_usb_modem();")
assert identity_read < identity_error < identity_mismatch < modem_remove_call
assert "if (modem_remove_status != 0)" in usb_remove
modem_failure_gate = usb_remove.index("if (modem_remove_status != 0)")
modem_failure_return = usb_remove.index("return -1;", modem_failure_gate)
assert modem_failure_gate < modem_failure_return < usb_remove.index("try_wan_reconnect(0, 0)")
assert "return safe_remove_stor_device(port, port, dev_name, 1);" in usb_remove
assert "return safe_remove_stor_device(1, BOARD_NUM_USB_PORTS, dev_name, 1);" in usb_remove
assert "skipping WAN reconnect after incomplete USB modem removal" in usb_remove
reconnect_call = usb_remove.index("wan_reconnect_status = try_wan_reconnect(0, 0);")
reconnect_gate = usb_remove.index("if (wan_reconnect_status != 0)", reconnect_call)
reconnect_failure_return = usb_remove.index("return -1;", reconnect_gate)
reconnect_success_return = usb_remove.index("return 0;", reconnect_failure_return)
assert usb_remove.rfind("errno = 0;", 0, reconnect_call) > usb_remove.index(
    "modem_remove_status = safe_remove_usb_modem();"
)
assert "errno = wan_reconnect_status;" in usb_remove[reconnect_gate:reconnect_failure_return]
assert "else if (!errno)" in usb_remove[reconnect_gate:reconnect_failure_return]
assert "errno = EIO;" in usb_remove[reconnect_gate:reconnect_failure_return]
assert reconnect_call < reconnect_gate < reconnect_failure_return < reconnect_success_return
assert "USB modem was removed but WAN reconnect failed" in usb_remove
wan_reconnect_body = c_function_body(
    net_wan, "try_wan_reconnect(int try_use_modem, long pause_in_seconds)"
)
assert "status = stop_wan();" in wan_reconnect_body
assert "if (status != 0)" in wan_reconnect_body and "return status;" in wan_reconnect_body
assert "status = start_wan();" in wan_reconnect_body
assert "return merge_l2tp_server_start_status(status);" in wan_reconnect_body

usbnet_stop_pos = wan_stop.index("stop_status = stop_wan_usbnet();")
wan_ifdown_pos = wan_stop.index("ifdown_status = wan_teardown_ifdown_interfaces(")
wan_failure_pos = wan_stop.index("stop_status = wan_teardown_first_error_status(")
wan_failure_gate_pos = wan_stop.index("if (stop_status != 0)", wan_failure_pos)
wan_clear_pos = wan_stop.index("stop_status = clear_wan_state_status();")
wan_clear_failure_pos = wan_stop.index("if (stop_status != 0)", wan_clear_pos)
assert usbnet_stop_pos < wan_ifdown_pos < wan_failure_pos < wan_failure_gate_pos < wan_clear_pos < wan_clear_failure_pos
assert "stop_status, proto_status, ifdown_status, callback_cleanup_status" in wan_stop[wan_failure_pos:wan_failure_gate_pos]
assert wan_clear_failure_pos < wan_stop.index("control_wan_led_isp_state(0, 0);", wan_clear_failure_pos)
assert "callback_cleanup_status = remove_cb_links();" in wan_stop
assert "goto abort_resume_detect;" in wan_stop[wan_failure_pos:]
assert "goto abort_resume_detect;" in wan_stop[wan_clear_failure_pos:]
print("USB modem stop-result propagation and WAN restart gate: PASS")


lan_stop_transaction = net_lan_dhcp.split("int\nstop_lan(int is_ap_mode)", 1)[1].split("\n}\n", 1)[0]
lan_pause = lan_stop_transaction.index("notify_pause_detect_internet()")
lan_preflight = lan_stop_transaction.index("ap_dhcp_stop_status = stop_udhcpc_lan();")
lan_preflight_failure = lan_stop_transaction.index("if (status != 0)", lan_preflight)
lan_preflight_resume = lan_stop_transaction.index("notify_run_detect_internet(2)", lan_preflight_failure)
lan_preflight_return = lan_stop_transaction.index("return status;", lan_preflight_failure)
lan_ifdown = lan_stop_transaction.index("ifconfig(IFNAME_BR, 0, NULL, NULL)")
lan_ap_restore = net_lan_dhcp.split("lan_ap_teardown_abort_restore(", 1)[1].split("\n}\n", 1)[0]
assert "start_lan(1, 1)" in lan_ap_restore
assert "notify_run_detect_internet(2)" in lan_ap_restore
assert "if (restore_status != 0)" in lan_ap_restore
assert "if (is_ap_mode && rc_lan_ap_teardown_recovery_required(" in lan_stop_transaction
lan_stop_compact = "".join(lan_stop_transaction.split())
assert "teardown_status=lan_ap_teardown_abort_restore(teardown_status," in lan_stop_compact
lan_dhcp_stop = net_lan_dhcp.split("stop_udhcpc_lan(void)", 1)[1].split("\n}\n", 1)[0]
lan_release = lan_dhcp_stop.index('process_pidfile_signal(pidfiles[0], "udhcpc", SIGUSR2)')
lan_identity_gate = lan_dhcp_stop.index("errno != ENOENT && errno != ESRCH")
lan_release_wait = lan_dhcp_stop.index("service_wait_microseconds(300000U)")
lan_noforce_stop = lan_dhcp_stop.index('kill_services_with_pidfiles(NULL, pidfiles, "udhcpc", 0, 30, 0)')
assert lan_release < lan_identity_gate < lan_release_wait < lan_noforce_stop
assert "if (wait_status != 0)" in lan_dhcp_stop
assert "refusing LAN DHCP stop: deconfig grace wait failed" in lan_dhcp_stop
assert "usleep(300000)" not in lan_dhcp_stop
assert lan_pause < lan_preflight < lan_preflight_failure < lan_preflight_resume < lan_preflight_return < lan_ifdown

lan_stop_compact = "".join(lan_stop_transaction.split())
assert "intgateway_status;" in lan_stop_compact
assert 'rc_wan_nvram_copy_value(lan_nvram_get_checked,NULL,"lan_gateway_t",ap_gateway,sizeof(ap_gateway))' in lan_stop_compact
ap_gateway_read = lan_stop_compact.index('"lan_gateway_t"')
ap_gateway_error = lan_stop_compact.index("if(gateway_status!=0)", ap_gateway_read)
ap_gateway_return = lan_stop_compact.index("returngateway_status;", ap_gateway_error)
assert ap_gateway_read < ap_gateway_error < ap_gateway_return < lan_stop_compact.index("notify_pause_detect_internet()")
assert 'nvram_safe_get("lan_gateway_t")' not in lan_stop_transaction
assert 'rc_wan_nvram_copy_value(lan_nvram_get_checked,NULL,"lan_ipaddr_t",lan_ip,sizeof(lan_ip))' in lan_stop_compact
assert 'nvram_safe_get("lan_ipaddr_t")' not in lan_stop_transaction
router_ip_read = lan_stop_compact.index('"lan_ipaddr_t"')
router_ip_error = lan_stop_compact.index("if(lan_ip_status!=0)", router_ip_read)
router_ip_record = lan_stop_compact.index("conntrack_status=lan_ip_status;", router_ip_error)
router_route_cleanup = lan_stop_compact.index("route_status=clear_if_route4(IFNAME_BR);", router_ip_record)
router_interface_down = lan_stop_compact.index("ifconfig(IFNAME_BR,0,NULL,NULL);", router_route_cleanup)
assert router_ip_read < router_ip_error < router_ip_record < router_route_cleanup < router_interface_down
assert "test_stop_lan_ap_gateway_snapshot_survives_cache_reuse" in bridge_init_regression
assert "test_stop_lan_router_address_read_failure_is_preserved" in bridge_init_regression
assert lan_preflight_failure < lan_preflight_resume < lan_preflight_return < lan_ifdown
print("AP LAN detect_internet pause rollback on failed teardown: PASS")

lan_restart_transaction = net_lan_dhcp.split("int\nfull_restart_lan(void)", 1)[1].split("\n}\n", 1)[0]
lan_vpn_stop = lan_restart_transaction.index("vpn_status = stop_vpn_server()")
lan_vpn_failure = lan_restart_transaction.index("if (vpn_status != 0)", lan_vpn_stop)
lan_vpn_abort = lan_restart_transaction.index("return vpn_status;", lan_vpn_failure)
lan_first_other_stop = lan_restart_transaction.index("lltd_stop_status = stop_lltd();")
assert lan_vpn_stop < lan_vpn_failure < lan_vpn_abort < lan_first_other_stop
assert lan_restart_transaction.count("stop_vpn_server();") == 1
lan_restart_compact = "".join(lan_restart_transaction.split())
lan_mode_read = lan_restart_compact.index(
    "ap_mode_status=get_ap_mode_status(&is_ap_mode);"
)
lan_mode_error = lan_restart_compact.index(
    "if(ap_mode_status!=0)", lan_mode_read
)
wan_error_read = lan_restart_compact.index(
    'get_wan_unit_value_int_status(0,"err",&is_wan_err)'
)
lan_stp_read = lan_restart_compact.index(
    'nvram_get_checked("lan_stp",&lan_stp_value)'
)
lan_first_side_effect = lan_restart_compact.index(
    "vpn_status=stop_vpn_server();"
)
assert lan_mode_read < lan_mode_error < wan_error_read < lan_stp_read < lan_first_side_effect
assert "get_ap_mode()" not in lan_restart_compact
assert 'get_wan_unit_value_int(0,"err")' not in lan_restart_compact
assert "rc_wan_nvram_read_int_status(" in lan_restart_compact
assert "LAN restart aborted because AP/router mode cannot be verified" in lan_restart_transaction
assert "LAN restart aborted because WAN error state cannot be read" in lan_restart_transaction
assert "LAN restart aborted because bridge STP state cannot be read" in lan_restart_transaction

lan_dhcp_failure = lan_restart_transaction.index("if (lan_status != 0)")
lan_router_restore = lan_restart_transaction.index("start_lan(0, 1)", lan_dhcp_failure)
lan_dns_unlock = lan_restart_transaction.index("dnsmasq_lifecycle_lock_release(dnsmasq_lock)", lan_dhcp_failure)
lan_vpn_restore = lan_restart_transaction.index('lan_restart_record_status(&restart_status, "restore VPN after aborted LAN restart",', lan_dns_unlock)
assert lan_dhcp_failure < lan_router_restore < lan_dns_unlock < lan_vpn_restore
for stop_status, start_call in (
    ("logger_stop_status", "start_logger(0)"),
    ("lltd_stop_status", "start_lltd()"),
    ("infosvr_stop_status", "start_infosvr()"),
    ("networkmap_stop_status", "start_networkmap(1)"),
):
    stop_call = lan_restart_transaction.index(stop_status + " = stop_")
    restore_gate = lan_restart_transaction.index("rc_service_restart_after_stop(" + stop_status + ")")
    restore = lan_restart_transaction.index(start_call, restore_gate)
    assert lan_vpn_abort < stop_call < lan_dhcp_failure
    assert lan_vpn_restore < restore_gate < restore

switch_vlan_restart = c_function_body(net_lan_dhcp, "restart_switch_config_vlan(void)")
switch_vlan_mode = switch_vlan_restart.index("get_ap_mode_status(&is_ap_mode)")
switch_vlan_error = switch_vlan_restart.index("if (ap_mode_status != 0)", switch_vlan_mode)
switch_vlan_mutation = switch_vlan_restart.index("notify_reset_detect_link();")
assert switch_vlan_mode < switch_vlan_error < switch_vlan_mutation
assert "return ap_mode_status;" in switch_vlan_restart[switch_vlan_error:switch_vlan_mutation]
assert "get_ap_mode()" not in switch_vlan_restart

lan_status_update = c_function_body(net_lan_dhcp, "update_lan_status(int is_auto)")
lan_status_mode = lan_status_update.index("get_ap_mode_status(&is_ap_mode)")
lan_status_error = lan_status_update.index("if (ap_mode_status != 0)", lan_status_mode)
lan_status_snapshot = lan_status_update.index("lan_dhcp_runtime_snapshot_copy(&snapshot)")
assert lan_status_mode < lan_status_error < lan_status_snapshot
assert "return ap_mode_status;" in lan_status_update[lan_status_error:lan_status_snapshot]
assert "get_ap_mode()" not in lan_status_update

hosts_retry = c_function_body(
    net_lan_dhcp,
    "lan_dhcp_publish_hosts_and_schedule_retry(const char *lease_ip,"
)
hosts_retry_mode = hosts_retry.index("get_ap_mode_status(&is_ap_mode)")
hosts_retry_error = hosts_retry.index("if (ap_mode_status != 0)", hosts_retry_mode)
hosts_retry_notify = hosts_retry.index("notify_rc_status(RCN_RESTART_DHCPD)")
assert hosts_retry_mode < hosts_retry_error < hosts_retry_notify
assert "errno = saved_errno;" in hosts_retry[hosts_retry_error:hosts_retry_notify]

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
    "trunk/user/802.1x/src/rtdot1x.c",
    "trunk/user/802.1x/src/auth_startup_handshake.h",
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
lan_bridge_setup = c_function_body(net_wan, "config_soft_bridge_lan(void)")
assert 'rc_wan_nvram_copy_value(\n\t\t\twan_nvram_checked_get_callback, NULL, "lan_hwaddr"' in lan_bridge_setup
assert 'nvram_safe_get("lan_hwaddr")' not in lan_bridge_setup
assert lan_bridge_setup.index('rc_wan_nvram_copy_value(') < lan_bridge_setup.index('create_vlan_iface_tracked(')

vlan_setup = c_function_body(net_wan, "config_vinet_wan(void)")
assert 'get_wan_unit_value_copy_status(\n\t\t0, "viflast"' in vlan_setup
assert "previous WAN VLAN cleanup identity is invalid" in vlan_setup
assert 'get_wan_unit_value(0, "viflast")' not in vlan_setup
assert vlan_setup.index('get_wan_unit_value_copy_status(') < vlan_setup.index("hw_vlan_tx_map(")
assert vlan_setup.index('wan_ifname_copy(vinet_iflast') < vlan_setup.index("hw_vlan_tx_map(")
vlan_setup_compact = "".join(vlan_setup.split())
assert 'rc_wan_nvram_copy_value(wan_nvram_checked_get_callback,NULL,"wan_hwaddr",wan_hwaddr,sizeof(wan_hwaddr))' in vlan_setup_compact
assert 'nvram_safe_get("wan_hwaddr")' not in vlan_setup
assert vlan_setup.index('rc_wan_nvram_copy_value(') < vlan_setup.index("hw_vlan_tx_map(")

resolver_body = c_function_body(
    net_wan, "update_resolvconf(int is_first_run, int do_not_notify)"
)
assert "rc_wan_nvram_copy_snapshot(" in resolver_body
assert all(f'"wan_dns{i}_x"' in resolver_body for i in range(1, 4))
assert 'get_wan_unit_value_copy_status(\n\t\t\t0, "dns"' in resolver_body
assert 'wan_nvram_checked_get_callback, NULL, "wanx_dns"' in resolver_body
assert 'get_wan_unit_value_copy_status(\n\t\t\t0, "dns6"' in resolver_body
assert "preserving existing resolver file" in resolver_body
assert "int wan_dns_config_status = 0;" in resolver_body
resolver_body_compact = "".join(resolver_body.split())
assert "file_unlock(lock);errno=wan_dns_config_status;returnwan_dns_config_status;" in resolver_body_compact

assert "dns_servers_error = 0;" not in resolver_body

assert 'nvram_safe_get(dns_name_x)' not in resolver_body
assert 'get_wan_unit_value(0, "dns")' not in resolver_body
assert 'nvram_safe_get("wanx_dns")' not in resolver_body
assert 'get_wan_unit_value(0, "dns6")' not in resolver_body

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
    "int\nnotify_run_detect_internet(int delay_time)", 1
)[1].split("\n}\n", 1)[0]
dl_notify = detect_link_source.split(
    "int\nnotify_reset_detect_link(void)", 1
)[1].split("\n}\n", 1)[0]
assert "detect_link_notification_status(status, saved_errno)" in dl_notify
assert "return status;" in dl_notify
wan_gateway_start = net_wan.split(
    "A failed gateway-ARP enable read is not equivalent to a disabled setting:", 1
)[1].split("if (gateway_valid)", 1)[0]
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
assert "return operation_error;" in di_notify
assert "operation_error = detect_internet_operation_error(-1, scan_error);" in di_notify
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

# LAN STP is a boot-time bridge mutation: an unreadable selector must not be
# collapsed into the legacy "not disabled" result and must skip both writes.
boot_services_compact = "".join(boot_services.split())
lan_stp_read = 'lan_stp_read_status=service_nvram_match_status("lan_stp","0",0,&lan_stp_disabled);'
assert boot_services_compact.count(lan_stp_read) == 1
lan_stp_read_at = boot_services_compact.index(lan_stp_read)
lan_stp_error_at = boot_services_compact.index("if(lan_stp_read_status!=0)", lan_stp_read_at)
lan_stp_enable_at = boot_services_compact.index("elseif(!lan_stp_disabled)", lan_stp_error_at)
lan_stp_config_at = boot_services_compact.index("br_set_stp(IFNAME_BR,1)", lan_stp_enable_at)
lan_forward_delay_at = boot_services_compact.index("br_set_fd(IFNAME_BR,15)", lan_stp_config_at)
assert lan_stp_read_at < lan_stp_error_at < lan_stp_enable_at < lan_stp_config_at < lan_forward_delay_at
lan_stp_error_block = boot_services.split("if (lan_stp_read_status != 0)", 1)[1].split(
    "else if (!lan_stp_disabled)", 1
)[0]
assert "cannot read LAN STP selector; refusing bridge STP changes" in lan_stp_error_block
lan_stp_error_block_compact = "".join(lan_stp_error_block.split())
assert "rc_first_service_start_status(start_status,lan_stp_read_status)" in lan_stp_error_block_compact
assert 'nvram_match("lan_stp", "0")' not in boot_services
watchdog_start = services_source.split("int" + chr(10) + "start_watchdog_cpu(void)", 1)[1].split(chr(10) + "}", 1)[0]
common_source = (root / "trunk/user/rc/src/common_ex.c").read_text()
module_load_policy = (root / "trunk/user/rc/src/wireless_module_load_policy.h").read_text()
module_load_test = (root / "trunk/user/rc/src/tests/wireless_module_load_policy.c").read_text()
module_load_start = common_source.split("int" + chr(10) + "module_smart_load(", 1)[1].split(
    chr(10) + "}", 1
)[0]
module_probe_pos = module_load_start.index("module_status = is_module_loaded_status(module_name)")
module_preflight_pos = module_load_start.index("if (!rc_module_load_probe_allows_attempt(module_status))")
module_modprobe_pos = module_load_start.index('argv[argc++] = "modprobe"')
assert module_probe_pos < module_preflight_pos < module_modprobe_pos
assert "is_module_loaded(module_name)" not in module_load_start
assert "rc_module_load_probe_allows_attempt" in module_load_policy
assert "return module_status == 0;" in module_load_policy
assert "test_load_preflight_requires_confirmed_absence" in module_load_test
assert "rc_module_load_probe_allows_attempt(-EIO) == 0" in module_load_test
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
    iappd_failure = block.index("if (iappd_restart_status != 0)")
    iappd_merge = block.index("isolation_status = wireless_radio_restart_status(", iappd_failure)
    return_status = block.index("return isolation_status;", iappd_merge)
    assert iappd_failure < iappd_merge < return_status
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
mtd_gate_pos = shutdown_router.index("storage_remove_status == 0")
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
assert '"trunk/user/rc/src/common_ex.c"' in process_workflow
assert '"trunk/user/rc/src/rc.h"' in process_workflow
assert '"trunk/user/rc/src/char_to_ascii_policy.h"' in process_workflow
assert '"trunk/user/rc/src/tests/char_to_ascii_policy.c"' in process_workflow

auth_source = (root / "trunk/user/rc/src/auth.c").read_text()
auth_stop_kabinet = auth_source.split("int stop_auth_kabinet(void)", 1)[1].split("\n}", 1)[0]
auth_stop_eapol = auth_source.split("int stop_auth_eapol(void)", 1)[1].split("\n}", 1)[0]
auth_start_eapol = auth_source.split("int start_auth_eapol(", 1)[1].split("\n}", 1)[0]
auth_start_kabinet = auth_source.split("int start_auth_kabinet(void)", 1)[1].split("\n}", 1)[0]
auth_nvram_get = auth_source.split("auth_kabinet_nvram_get(void *context", 1)[1].split("\n}", 1)[0]
auth_read_value = auth_source.split("auth_kabinet_read_value(void *context", 1)[1].split("\n}", 1)[0]
auth_start_service = auth_source.split("auth_kabinet_start_service(void *context", 1)[1].split("\n}", 1)[0]
auth_policy = (root / "trunk/user/rc/src/auth_kabinet_restart_policy.h").read_text()
assert "return kill_services(svcs, 3, 1);" in auth_stop_kabinet
assert "return kill_services(svcs, 3, 1);" in auth_stop_eapol
assert "auth_kabinet_restart_with_ops(" in auth_start_kabinet
assert "nvram_safe_get(" not in auth_start_kabinet
assert "nvram_get_checked(name, value)" in auth_nvram_get
assert "rc_wan_nvram_copy_value(auth_kabinet_nvram_get" in auth_read_value
assert 'eval("/usr/sbin/lanauth", "-f", credential_path)' in auth_start_service
assert 'eval("/usr/sbin/lanauth", "-s", gateway, "-f", credential_path)' in auth_start_service
assert '"-p", password' not in auth_start_service
auth_credential_writer = auth_source.split("auth_kabinet_write_password_file(", 1)[1].split("\n}", 1)[0]
assert "mkstemp(temporary_path)" in auth_credential_writer
assert "fchmod(fd, S_IRUSR | S_IWUSR)" in auth_credential_writer
assert "write(fd, password + written" in auth_credential_writer
assert "fsync(fd)" in auth_credential_writer
assert "memset(credential_path, 0, sizeof(credential_path))" in auth_source
lanauth_source = (root / "trunk/user/lanauth/src/lanauth.c").read_text()
lanauth_policy = (root / "trunk/user/lanauth/src/lanauth_credential_file_policy.h").read_text()
assert 'getopt(argc, argv, "iv:b:ns:p:f:l:u:g:")' in lanauth_source
assert "case 'f':" in lanauth_source
assert "lanauth_read_private_password_file(" in lanauth_source
assert "O_NOFOLLOW" in lanauth_policy
assert "(opened_stat.st_mode & 07777) != 0600" in lanauth_policy
assert "opened_stat.st_nlink != 1" in lanauth_policy
assert "opened_stat.st_uid != geteuid()" in lanauth_policy
assert "lstat(path, &named_stat)" in lanauth_policy
assert "unlink(path)" in lanauth_policy
auth_policy_body = auth_policy.split("auth_kabinet_restart_with_ops(", 1)[1].split("\n}", 1)[0]
assert auth_policy_body.index("AUTH_KABINET_GATEWAY_KEY") < auth_policy_body.index("AUTH_KABINET_PASSWORD_KEY")
assert auth_policy_body.index("AUTH_KABINET_PASSWORD_KEY") < auth_policy_body.index("stop_service(context)") < auth_policy_body.index("start_service(context, gateway, password)")
assert "memset(password, 0, password_size)" in auth_policy_body
assert "memset(gateway, 0, gateway_size)" in auth_policy_body
assert 'get_wan_unit_value_copy_status(' in auth_start_eapol
assert '"auth_user"' in auth_start_eapol and '"auth_pass"' in auth_start_eapol
assert "write_wpa_config_temp(" in auth_start_eapol
auth_start_begin = auth_source.index("int start_auth_eapol(")
auth_start_close = auth_source.index("\n}\n", auth_start_begin) + 3
auth_next_function = auth_source.index("int wpacli_main(", auth_start_close)
assert not auth_source[auth_start_close:auth_next_function].strip(), (
    "unexpected tokens after start_auth_eapol before wpacli_main"
)
assert "ret = stop_auth_eapol();" in auth_start_eapol
assert "if (ret != 0)" in auth_start_eapol
auth_user_read = auth_start_eapol.index('"auth_user"')
auth_pass_read = auth_start_eapol.index('"auth_pass"')
auth_temp_write = auth_start_eapol.index("write_wpa_config_temp(")
auth_stop = auth_start_eapol.index("ret = stop_auth_eapol();")
auth_stop_gate = auth_start_eapol.index("if (ret != 0)", auth_stop)
auth_publish = auth_start_eapol.index("rename(wpa_conf_temp, wpa_conf)")
auth_launch = auth_start_eapol.index("_eval(wpa_argv, NULL, 0, NULL)")
assert auth_user_read < auth_pass_read < auth_temp_write < auth_stop < auth_stop_gate < auth_publish < auth_launch
assert 'get_wan_unit_value(unit, "auth_user")' not in auth_start_eapol
assert 'get_wan_unit_value(unit, "auth_pass")' not in auth_start_eapol
assert 'fopen(wpa_conf, "w")' not in auth_start_eapol
auth_config_writer = auth_source.split("static int\nwrite_wpa_config_temp(", 1)[1].split("\n}\n", 1)[0]
assert "mkstemp(temporary_path)" in auth_config_writer
assert "fchmod(fd, S_IRUSR | S_IWUSR)" in auth_config_writer
assert "fsync(fileno(fp))" in auth_config_writer
assert "wpa_eapol_config_write_quoted(fp, username)" in auth_config_writer
assert "wpa_eapol_config_write_quoted(fp, password)" in auth_config_writer
assert "ret = _eval(cli_argv, NULL, 0, NULL);" in auth_start_eapol
assert "wpa_cli event handler failed to start" in auth_start_eapol
assert '"trunk/user/rc/src/auth.c"' in process_workflow
assert '"trunk/user/rc/src/auth.c"' in xy_workflow

watchdog_source = (root / "trunk/user/rc/src/watchdog.c").read_text()
reboot_schedule = watchdog_source.split(
    "int timecheck_reboot(char *activeSchedule)", 1
)[1].split("\n}\n", 1)[0]
assert '"timezone_env_policy.h"' in watchdog_source
assert 'nvram_get_checked("time_zone_x", &time_zone)' in reboot_schedule
assert "rc_timezone_env_format_status(" in reboot_schedule
assert 'setenv("TZ", timezone_env + 3, 1)' in reboot_schedule
assert "tzset();" in reboot_schedule
assert 'nvram_safe_get("time_zone_x")' not in reboot_schedule
timezone_read = reboot_schedule.index('nvram_get_checked("time_zone_x", &time_zone)')
timezone_validate = reboot_schedule.index("rc_timezone_env_format_status(")
timezone_apply = reboot_schedule.index('setenv("TZ", timezone_env + 3, 1)')
clock_read = reboot_schedule.index("time(&now)")
assert timezone_read < timezone_validate < timezone_apply < clock_read
assert "cannot evaluate scheduled action because timezone is unavailable" in reboot_schedule
assert "time(&now) == (time_t)-1" in reboot_schedule
assert "timezone_env_policy.h" in process_workflow
assert "timezone_env_policy.h" in xy_workflow

timezone_policy = (root / "trunk/user/rc/src/timezone_env_policy.h").read_text()
timezone_test = (root / "trunk/user/rc/src/tests/timezone_env_policy.c").read_text()
# Kernel timezone retries must not be suppressed after a failed settimeofday().
kernel_timezone = c_function_body(rc_source, "setkernel_tz(void)")
assert '#include "timezone_env_policy.h"' in rc_source
assert "rc_timezone_kernel_cache_status(" in kernel_timezone
assert kernel_timezone.index("settimeofday(tvp, &tz)") < kernel_timezone.index(
    "rc_timezone_kernel_cache_status("
)
assert "tz_minuteswest_last = tz.tz_minuteswest" not in kernel_timezone
assert "static int tz_minuteswest_valid = 0;" in kernel_timezone
assert "rc_timezone_kernel_cache_needs_update(" in kernel_timezone
assert "tz_minuteswest_valid = 1;" in kernel_timezone
assert "rc_timezone_kernel_cache_needs_update(int cache_valid" in timezone_policy
assert "test_cache_validity_does_not_collide_with_negative_one_offset();" in timezone_test
assert "cannot update kernel timezone" in kernel_timezone
assert "if (syscall_status != 0)" in timezone_policy
assert "*cached_minuteswest = requested_minuteswest;" in timezone_policy
assert "test_kernel_timezone_cache_changes_only_after_success();" in timezone_test

timezone_env = c_function_body(rc_source, "setenv_tz(void)")
assert "intsetenv_tz(void)" in "".join(rc_source.split())
assert "return status;" in timezone_env
assert "return 0;" in timezone_env
assert "return status;" in kernel_timezone
assert "time(&now) == (time_t)-1" in kernel_timezone
assert "gmtime_r(&now, &gm)" in kernel_timezone and "!gmtime_r" in kernel_timezone
assert "localtime_r(&now, &local)" in kernel_timezone and "!localtime_r" in kernel_timezone
assert "gm_time = mktime(&gm)" in kernel_timezone
assert "local_time = mktime(&local)" in kernel_timezone
assert kernel_timezone.index("gmtime_r(&now, &gm)") < kernel_timezone.index("gm.tm_isdst = local.tm_isdst")
assert kernel_timezone.index("localtime_r(&now, &local)") < kernel_timezone.index("local.tm_isdst")
assert "rc_timezone_conversion_status(0, errno)" in kernel_timezone
assert "test_time_conversion_failures_have_deterministic_status();" in timezone_test
assert "rc_timezone_conversion_status(int conversion_succeeded, int saved_errno)" in timezone_policy
timezone_update = c_function_body(rc_source, "set_timezone(void)")
assert "status = setenv_tz();" in timezone_update
assert "kernel_status = setkernel_tz();" in timezone_update
assert "if (status == 0)" in timezone_update
assert "return status;" in kernel_timezone
boot_router = c_function_body(rc_source, "init_router(void)")
assert "int timezone_status = set_timezone();" in boot_router
assert "boot_services_status = rc_first_service_start_status(" in boot_router
assert "boot_services_status, timezone_status" in boot_router
assert boot_router.index("storage_load_time();") < boot_router.index(
    "int timezone_status = set_timezone();"
), "kernel timezone offset must be initialized after persisted clock restore"
time_restart = rc_source.split(
    "strcmp(entry->d_name, RCN_RESTART_TIME) == 0", 1
)[1].split("RCN_RESTART_SYSCTL", 1)[0]
assert "int timezone_status = set_timezone();" in time_restart
assert "action_result, timezone_status" in time_restart


# Persisted clock state is a privileged boot input: reject links, non-regular
# files, unsafe ownership/mode, incomplete reads and non-decimal timestamps.
storage_time_load = c_function_body(rc_source, "storage_load_time(void)")
storage_time_policy = (root / "trunk/user/rc/src/storage_time_file_policy.h").read_text()
storage_time_test = (root / "trunk/user/rc/src/tests/storage_time_file_policy.c").read_text()
assert '#include "storage_time_file_policy.h"' in rc_source
assert 'open("/etc/storage/system_time"' in storage_time_load
assert "O_NOFOLLOW" in storage_time_load and "O_NONBLOCK" in storage_time_load
assert "fstat(fd, &st)" in storage_time_load and "S_ISREG(st.st_mode)" in storage_time_load
assert "st_uid != geteuid()" in storage_time_load and "st_nlink != 1" in storage_time_load
assert "S_IWGRP | S_IWOTH" in storage_time_load
storage_time_load_compact = "".join(storage_time_load.split())
assert "rc_storage_time_parse_status(buffer,length,&storage_time)" in storage_time_load_compact
assert "strtoul(buf, NULL, 0)" not in storage_time_load
assert "if (stime(&adjusted_time) != 0)" in storage_time_load
assert "test_overflow_and_invalid_arguments_fail_closed();" in storage_time_test
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
watchdog_notify = watchdog_source.split("int\nnotify_watchdog_time(void)", 1)[1].split(
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
assert "watchdog_rollback_failed_start(" in watchdog_start
assert watchdog_start.count("watchdog_rollback_failed_start(") == 6
assert "return launch_status;" not in watchdog_start
assert watchdog_start.index("if (action == PROCESS_PRESENCE_START_NEW)") < watchdog_start.index(
    "launch_status = eval(\"/sbin/watchdog\")"
) < watchdog_start.index("if (launch_status != 0)")
watchdog_start_stop = c_function_body(
    watchdog_source, "watchdog_start_rollback_stop(void *context)"
)
watchdog_start_scan = c_function_body(
    watchdog_source, "watchdog_start_rollback_scan(void *context, int *scan_error)"
)
watchdog_start_rollback = c_function_body(
    watchdog_source, "watchdog_rollback_failed_start(int start_status, int start_errno,"
)
assert "detached_daemon_start_rollback_with_ops(" in watchdog_start_rollback
assert "watchdog_start_rollback_stop" in watchdog_start_rollback
assert "watchdog_start_rollback_scan" in watchdog_start_rollback
assert "errno = saved_errno;" in watchdog_start_rollback
assert "return result;" in watchdog_start_rollback
assert "kill_services(service_names, 3, 1)" in watchdog_start_stop
assert "process_inventory_scan((const char *)context, &snapshot)" in watchdog_start_scan
assert "process_snapshot_release(&snapshot)" in watchdog_start_scan
for marker in (
    'launch_status, launch_errno, "launcher"',
    'wait_status, wait_errno, "initial readiness wait"',
    '-1, scan_error, "process inventory"',
    '-1, EIO, "invalid process inventory state"',
    'wait_status, wait_errno, "retry readiness wait"',
    '-1, ETIMEDOUT, "readiness timeout"',
):
    assert marker in watchdog_start, marker
assert '#include "service_wait_policy.h"' in watchdog_source
assert "watchdog_start_probe_wait(unsigned int seconds)" in watchdog_source
assert "watchdog_start_probe_wait(" in watchdog_start
assert "PROCESS_PRESENCE_START_INITIAL_WAIT_SECONDS" in watchdog_start
assert "PROCESS_PRESENCE_START_RETRY_WAIT_SECONDS" in watchdog_start
assert "PROCESS_PRESENCE_START_PROBE_ATTEMPTS" in watchdog_start
assert watchdog_start.count('process_inventory_scan("watchdog", &snapshot)') == 2
assert "process_presence_start_postcondition(launch_status, presence)" in watchdog_start
assert "presence == PROCESS_PRESENCE_UNKNOWN" in watchdog_start
assert "cannot verify detached watchdog startup" in watchdog_start
assert "watchdog daemon did not remain running after launch" in watchdog_start
assert "refusing watchdog start because process-presence policy is invalid" in watchdog_start
assert "start_watchdog()" in watchdog_notify
assert "pids(" not in watchdog_notify
assert "process_pidfile_signal(WD_PID_FILE, \"watchdog\", SIGHUP)" in watchdog_notify
assert "watchdog_time_notification_error(" in watchdog_notify
assert "return operation_error;" in watchdog_notify
assert '"trunk/user/rc/src/watchdog.c"' in process_workflow
assert '"trunk/user/rc/src/watchdog.c"' in xy_workflow
assert '"trunk/user/rc/src/process_presence_start_policy.h"' in process_workflow
assert '"trunk/user/rc/src/tests/process_presence_start_policy.c"' in process_workflow
print("watchdog process inventory tri-state recovery/start integration: PASS")


lltd_start = services_source.split("int\nstart_lltd(void)", 1)[1].split("\n}", 1)[0]
lltd_launcher = 'start_status = eval("/bin/lld2d", IFNAME_BR);'
assert '"lltd_enable", "1", 0, &enabled' in lltd_start
assert "if (config_status != 0)" in lltd_start
assert "if (!enabled)" in lltd_start
assert 'nvram_invmatch("lltd_enable", "1")' not in lltd_start
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
lltd_wait_at = lltd_start.index('service_start_probe_wait("LLTD", PROCESS_PRESENCE_START_INITIAL_WAIT_SECONDS);', lltd_launch_at)
lltd_probe_at = lltd_start.index("for (attempt = 0; attempt < PROCESS_PRESENCE_START_PROBE_ATTEMPTS; attempt++)", lltd_launch_at)
lltd_inventory_at = lltd_start.index('optional_daemon_presence("lld2d", &scan_error)', lltd_probe_at)
lltd_postcondition_at = lltd_start.index("process_presence_start_postcondition(start_status, process_state)", lltd_inventory_at)
assert lltd_preflight < lltd_launch_at < lltd_wait_at < lltd_probe_at < lltd_inventory_at < lltd_postcondition_at
assert "if (process_presence_start_postcondition(start_status, process_state) == 0)" in lltd_start
assert "return 0;" in lltd_start[lltd_postcondition_at:]

rstats_start = services_source.split("int\nstart_rstats(void)", 1)[1].split("\n}", 1)[0]
rstats_launcher = 'start_status = eval("/sbin/rstats");'
assert '"rstats_enable", "1", 0, &enabled' in rstats_start
assert "if (config_status != 0)" in rstats_start
assert "if (!enabled)" in rstats_start
assert 'nvram_invmatch("rstats_enable", "1")' not in rstats_start
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
rstats_wait_at = rstats_start.index('service_start_probe_wait("rstats", PROCESS_PRESENCE_START_INITIAL_WAIT_SECONDS);', rstats_launch_at)
rstats_probe_at = rstats_start.index("for (attempt = 0; attempt < PROCESS_PRESENCE_START_PROBE_ATTEMPTS; attempt++)", rstats_launch_at)
rstats_inventory_at = rstats_start.index('optional_daemon_presence("rstats", &scan_error)', rstats_probe_at)
rstats_postcondition_at = rstats_start.index("process_presence_start_postcondition(start_status, process_state)", rstats_inventory_at)
assert rstats_preflight < rstats_launch_at < rstats_wait_at < rstats_probe_at < rstats_inventory_at < rstats_postcondition_at
assert "if (process_presence_start_postcondition(start_status, process_state) == 0)" in rstats_start
assert "return 0;" in rstats_start[rstats_postcondition_at:]

# start_lltd()/start_rstats() already implement a checked enable read and
# return zero when disabled. Re-reading the same key with nvram_match() here
# used to swallow a real start/configuration error when that second read failed.
lltd_restart = c_function_body(services_source, "restart_lltd(void)")
assert "return start_lltd();" in lltd_restart
assert 'nvram_match("lltd_enable", "1")' not in lltd_restart
assert "return stop_status;" in lltd_restart

rstats_restart = c_function_body(services_source, "restart_rstats(void)")
assert "return start_rstats();" in rstats_restart
assert 'nvram_match("rstats_enable", "1")' not in rstats_restart
assert "return stop_status;" in rstats_restart

rstats_main = c_function_body(rstats_source, "int rstats_main(int argc, char *argv[])")
assert rstats_main.index("if (daemon(0, 0) < 0)") < rstats_main.index("if (alloc_rstats() < 0)")
assert rstats_main.index("if (alloc_rstats() < 0)") < rstats_main.index('process_pidfile_write(RSTATS_PID_FILE, "rstats")')
assert rstats_main.index('process_pidfile_write(RSTATS_PID_FILE, "rstats")') < rstats_main.index("while (!stopping)")

lldt_conf_reader = c_function_body(lldt_main_source, "init_from_conf_file()")
assert '#include "lldt_config_path_policy.h"' in lldt_main_source
assert '#include "lldt_config_parse_policy.h"' in lldt_main_source
assert '#include <ctype.h>' in lldt_main_source
assert "lldt_config_parse_assignment(line, (size_t)numread," in lldt_conf_reader
assert "sscanf(" not in lldt_conf_reader and "consumed" not in lldt_conf_reader
assert "memchr(" not in lldt_conf_reader and "(size_t)numread" in lldt_conf_reader
assert lldt_conf_reader.count("lldt_config_path_copy(val, path, path_capacity)") == 2
assert lldt_conf_reader.count("size_t path_capacity = strlen(val) + 6") == 2
assert "strncpy(" not in lldt_conf_reader
assert "memchr(line, '\\0', length)" in lldt_config_parse_policy
assert "name_length >= name_capacity || value_length >= value_capacity" in lldt_config_parse_policy
assert "memcpy(name, line + name_start, name_length)" in lldt_config_parse_policy
assert "memcpy(path + prefix_len, value, value_len + 1)" in lldt_config_path_policy
for marker in ("LLTD bounded config parser and icon path regression", "embedded_nul",
               "ENAMETOOLONG", "100001", "strlen(long_path) == 260"):
    assert marker in lldt_config_path_test, marker

lldt_main = c_function_body(lldt_main_source, "main(int argc, char **argv)")
lldt_daemon = c_function_body(lldt_osl_source, "osl_become_daemon(osl_t *osl)")
assert lldt_main.index("if (!opt_debug)") < lldt_main.index("osl_become_daemon(g_osl);")
assert lldt_main.index("osl_become_daemon(g_osl);") < lldt_main.index("osl_write_pidfile(g_osl);") < lldt_main.index("event_mainloop();")
assert lldt_daemon.count("fork()") == 2 and "setsid()" in lldt_daemon
assert 'g_Progname = p+1;' in lldt_main_source
lldt_pidfile_writer = c_function_body(lldt_osl_source, "osl_write_pidfile(osl_t *osl)")
assert "lldt_pidfile_format_path(pidfile, sizeof(pidfile)," in lldt_pidfile_writer
assert '"/var/run/%s-%.10s.pid"' not in lldt_pidfile_writer
assert "LLDT_INTERFACE_NAME_MAX 15U" in lldt_pidfile_policy
assert "lldt_validate_interface_name(interface)" in lldt_pidfile_policy
assert "lldt_pidfile_format_path" in lldt_pidfile_policy
assert "test_interface_name_and_pidfile_path_identity" in lldt_pidfile_test
assert '#include "lldt_pidfile_policy.h"' in lldt_osl_source
assert "lldt_pidfile_parse_pid(pidbuf, pidbuf_length, &pid)" in lldt_pidfile_writer
assert "pidbuf_length == sizeof(pidbuf)" in lldt_pidfile_writer
assert "O_NOFOLLOW" in lldt_pidfile_writer and "O_NONBLOCK" in lldt_pidfile_writer
assert lldt_pidfile_writer.count("fstat(fd, &st)") == 2
lldt_pidfile_read_validation = lldt_pidfile_writer.index(
    "lldt_pidfile_validate_regular_file(&st, geteuid())")
lldt_pidfile_write_validation = lldt_pidfile_writer.rindex(
    "lldt_pidfile_validate_regular_file(&st, geteuid())")
assert lldt_pidfile_read_validation < lldt_pidfile_writer.index("io_status = read")
assert lldt_pidfile_write_validation < lldt_pidfile_writer.index("ftruncate(fd, 0)")
lldt_pidfile_lock = c_function_body(lldt_osl_source,
                                    "osl_open_pidfile_lock(const char *pidfile)")
assert lldt_pidfile_lock.index("fstat(fd, &lock_stat)") < lldt_pidfile_lock.index(
    "lldt_pidfile_validate_regular_file(&lock_stat, geteuid())")
assert lldt_pidfile_lock.index(
    "lldt_pidfile_validate_regular_file(&lock_stat, geteuid())") < lldt_pidfile_lock.index(
    "lldt_pidfile_try_exclusive_lock(fd)")
for marker in ("S_ISREG(st->st_mode)", "st->st_uid != expected_uid",
               "st->st_nlink != 1",
               "(st->st_mode & (S_IWGRP | S_IWOTH))"):
    assert marker in lldt_pidfile_policy, marker
assert "test_pidfile_descriptor_validation" in lldt_pidfile_test
assert "LLTD PID-file descriptor ownership/mode/hard-link regression: PASS" in lldt_pidfile_test
assert "if (saved_errno != ESRCH)" in lldt_pidfile_writer
assert "if (ret == 0)" in lldt_pidfile_writer
assert "ftruncate(fd, 0)" in lldt_pidfile_writer
assert "while (written < pidbuf_length)" in lldt_pidfile_writer
assert "strtol(" not in lldt_pidfile_writer
assert "#define LLDT_PIDFILE_MAX_TEXT 11U" in lldt_pidfile_policy
assert "value > ((unsigned long)INT_MAX - digit) / 10UL" in lldt_pidfile_policy
for marker in ("expect_valid(no_nul", "expect_valid(maximum", "expect_invalid(overflow",
               "expect_invalid(embedded_nul", "LLTD bounded PID-file parser regression"):
    assert marker in lldt_pidfile_test, marker
assert '"trunk/user/lldt/src/lldt_pidfile_policy.h"' in process_workflow
assert '"trunk/user/lldt/src/tests/lldt_pidfile_policy.c"' in process_workflow
assert '"trunk/user/lldt/src/lldt_config_path_policy.h"' in process_workflow
assert '"trunk/user/lldt/src/lldt_config_parse_policy.h"' in process_workflow
assert '"trunk/user/lldt/src/tests/lldt_config_path_policy.c"' in process_workflow
assert "Compile and run LLTD PID-file parser regression" in process_workflow
assert '"trunk/user/lldt/src/main.c"' in process_workflow
assert '"trunk/user/lldt/src/osl-linux.c"' in process_workflow
assert '"trunk/user/rc/src/rstats.c"' in process_workflow

assert '"trunk/user/rc/src/services.c"' in process_workflow
assert '"trunk/user/rc/src/services.c"' in xy_workflow
assert '"trunk/user/lldt/src/osl-linux.c"' in xy_workflow
assert '"trunk/user/lldt/src/lldt_pidfile_policy.h"' in xy_workflow
assert '"trunk/user/lldt/src/main.c"' in xy_workflow
assert '"trunk/user/lldt/src/lldt_config_path_policy.h"' in xy_workflow
assert '"trunk/user/lldt/src/lldt_config_parse_policy.h"' in xy_workflow
assert '"trunk/user/shared/src/shutils_pidfile_policy.h"' in xy_workflow
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
    ("APP_SKIPDBV2", "int skipdb_start_status = start_skipdbv2();",
     "skipdb_start_status", "SkipDBv2 boot startup failed (status=%d)"),
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
assert "service_nvram_match_status(" in telnetd_start
assert '"telnetd", "1", 0, &telnet_enabled' in telnetd_start
telnet_enable_read = telnetd_start.index('"telnetd", "1", 0, &telnet_enabled')
telnet_config_failure = telnetd_start.index("if (config_status != 0)", telnet_enable_read)
telnet_disabled = telnetd_start.index("if (!telnet_enabled)", telnet_config_failure)
telnet_start_gate = telnetd_start.index('optional_daemon_start_status("telnetd", "telnetd")')
assert "return config_status;" in telnetd_start[telnet_config_failure:telnet_disabled]
assert "return 0;" in telnetd_start[telnet_disabled:telnet_start_gate]
assert telnet_config_failure < telnet_disabled < telnet_start_gate
assert 'nvram_match("telnetd", "1")' not in telnetd_start
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
telnetd_initial_wait = telnetd_start.index('service_start_probe_wait("telnetd", PROCESS_PRESENCE_START_INITIAL_WAIT_SECONDS);', telnetd_launch)
telnetd_probe = telnetd_start.index("for (attempt = 0; attempt < PROCESS_PRESENCE_START_PROBE_ATTEMPTS; attempt++)", telnetd_launch)
telnetd_inventory = telnetd_start.index('optional_daemon_presence("telnetd", &scan_error)', telnetd_probe)
telnetd_postcondition = telnetd_start.index("process_presence_start_postcondition(start_status, process_state)", telnetd_inventory)
assert telnetd_launch < telnetd_initial_wait < telnetd_probe < telnetd_inventory < telnetd_postcondition

ttyd_start = c_function_body(
    services_source, "start_ttyd_with_mode(int ttyd_mode)"
)
ttyd_start_wrapper = c_function_body(services_source, "start_ttyd(void)")
ttyd_restart_wrapper = c_function_body(services_source, "restart_ttyd(void)")
assert "int start_status, attempt, scan_error, start_errno;" in ttyd_start
assert "enum process_presence process_state;" in ttyd_start
assert "if (ttyd_mode != 1)" in ttyd_start
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
ttyd_launch = ttyd_start_compact.index('start_status = eval("/usr/bin/ttyd.sh", "start");')
ttyd_launch_failure = ttyd_start_compact.index("if (start_status != 0)", ttyd_launch)
ttyd_launcher_errno = ttyd_start_compact.index("start_errno = errno > 0 ? errno :", ttyd_launch_failure)
ttyd_launcher_log = ttyd_start_compact.index('logmessage(LOGNAME, "ttyd launcher failed (status=%d)", start_status);', ttyd_launcher_errno)
ttyd_launcher_rollback = ttyd_start_compact.index('return optional_daemon_rollback_failed_start( "ttyd", start_status, start_errno);', ttyd_launcher_log)
ttyd_initial_wait = ttyd_start_compact.index('service_start_probe_wait("ttyd", PROCESS_PRESENCE_START_INITIAL_WAIT_SECONDS);', ttyd_launch)
ttyd_probe = ttyd_start_compact.index("for (attempt = 0; attempt < PROCESS_PRESENCE_START_PROBE_ATTEMPTS; attempt++)", ttyd_initial_wait)
ttyd_inventory = ttyd_start_compact.index('optional_daemon_presence("ttyd", &scan_error)', ttyd_probe)
ttyd_postcondition = ttyd_start_compact.index("process_presence_start_postcondition(start_status, process_state) == 0", ttyd_inventory)
ttyd_unknown = ttyd_start_compact.index("if (process_state == PROCESS_PRESENCE_UNKNOWN)", ttyd_postcondition)
ttyd_timeout = ttyd_start_compact.index("errno = ETIMEDOUT;", ttyd_unknown)
ttyd_timeout_rollback = ttyd_start_compact.index('return optional_daemon_rollback_failed_start("ttyd", -1, ETIMEDOUT);', ttyd_timeout)
assert ttyd_launch < ttyd_launch_failure < ttyd_launcher_errno < ttyd_launcher_log < ttyd_launcher_rollback < ttyd_initial_wait < ttyd_probe < ttyd_inventory < ttyd_postcondition < ttyd_unknown < ttyd_timeout < ttyd_timeout_rollback
assert ttyd_start_compact.count("return optional_daemon_rollback_failed_start(") == 5
assert ttyd_start_compact.count("process_presence_start_postcondition(start_status, process_state)") == 1
assert "start_errno = scan_error > 0 ? scan_error : EIO;" in ttyd_start

ttyd_read = 'service_enabled_mode_status("ttyd", "ttyd_enable", &ttyd_mode);'
ttyd_read_compact = "".join(ttyd_read.split())
ttyd_start_wrapper_compact = "".join(ttyd_start_wrapper.split())
ttyd_restart_wrapper_compact = "".join(ttyd_restart_wrapper.split())
assert ttyd_read_compact in ttyd_start_wrapper_compact
assert ttyd_start_wrapper_compact.index(ttyd_read_compact) < ttyd_start_wrapper_compact.index("if(config_status!=0)")
assert ttyd_start_wrapper_compact.index("if(config_status!=0)") < ttyd_start_wrapper_compact.index("returnstart_ttyd_with_mode(ttyd_mode);")
assert ttyd_read_compact in ttyd_restart_wrapper_compact
assert ttyd_restart_wrapper_compact.index(ttyd_read_compact) < ttyd_restart_wrapper_compact.index("if(config_status!=0)")
assert ttyd_restart_wrapper_compact.index("if(config_status!=0)") < ttyd_restart_wrapper_compact.index("stop_status=stop_ttyd();")
assert ttyd_restart_wrapper_compact.index("stop_status=stop_ttyd();") < ttyd_restart_wrapper_compact.index("if(stop_status!=0)")
assert ttyd_restart_wrapper_compact.index("if(stop_status!=0)") < ttyd_restart_wrapper_compact.index("returnstart_ttyd_with_mode(ttyd_mode);")
assert "nvram_get_int(" not in ttyd_start_wrapper
assert "nvram_safe_get(" not in ttyd_start_wrapper
assert "nvram_get_int(" not in ttyd_restart_wrapper
assert "nvram_safe_get(" not in ttyd_restart_wrapper
print("telnetd/ttyd detached-child startup postconditions and disabled sentinels: PASS")

optional_start_contracts = (
    ("start_infosvr(void)", "adsc_enable"),
    ("start_lltd(void)", "lltd_enable"),
    ("start_rstats(void)", "rstats_enable"),
)
for signature, flag in optional_start_contracts:
    service_body = c_function_body(services_source, signature)
    service_body_compact = "".join(service_body.split())
    config_read = service_body_compact.index(
        'service_nvram_match_status("' + flag + '","1",0,&enabled);'
    )
    config_error = service_body_compact.index("if(config_status!=0)", config_read)
    disabled = service_body_compact.index("if(!enabled)", config_error)
    process_probe = service_body_compact.index("optional_daemon_start_status(", disabled)
    assert config_read < config_error < disabled < process_probe, (
        signature + " checked enable read must precede process preflight"
    )
    assert "returnconfig_status;" in service_body_compact[config_error:disabled], (
        signature + " must propagate enable-read errors"
    )
    assert "return0;" in service_body_compact[disabled:process_probe], (
        signature + " disabled path must be success"
    )
    assert 'nvram_invmatch("' + flag + '","1")' not in service_body_compact
infosvr_start = c_function_body(services_source, "start_infosvr(void)")
infosvr_start_compact = "".join(infosvr_start.split())
assert infosvr_start_compact.index(
    'service_nvram_match_status("adsc_enable","1",0,&enabled);'
) < infosvr_start_compact.index(
    'optional_daemon_start_status("infosvr","infosvr")'
)
infosvr_launch = infosvr_start.index('start_status = eval("/usr/sbin/infosvr", IFNAME_BR);')
infosvr_failure = infosvr_start.index("if (start_status != 0)", infosvr_launch)
infosvr_failure_branch = infosvr_start[infosvr_failure:].split("\n\t}", 1)[0]
assert "errno = EIO;" in infosvr_failure_branch
assert "start_errno = errno;" in infosvr_failure_branch
assert 'optional_daemon_rollback_failed_start("infosvr", -1, start_errno)' in infosvr_failure_branch
assert "return start_status;" not in infosvr_failure_branch
infosvr_policy = (root / "trunk/user/rc/src/infosvr_daemon_policy.h").read_text()
infosvr_test = (root / "trunk/user/rc/src/tests/infosvr_daemon_policy.c").read_text()
infosvr_postcondition = c_function_body(infosvr_policy, "infosvr_start_postcondition(")
assert "return -1;" in infosvr_postcondition
assert "return launch_status;" not in infosvr_postcondition
assert "test_start_normalizes_launcher_failure_for_pid1" in infosvr_test
boot_services = c_function_body(services_source, "start_services_once(int is_ap_mode)")
for call in ("start_infosvr()", "start_lltd()", "start_rstats()"):
    assert call in boot_services, "missing optional daemon startup: " + call
print("optional infosvr/LLTD/rstats disabled-start status aggregation: PASS")



# WAN0 defaults/runtime identity must be checked before PID 1 permits boot
# services to start. A single failed write is fatal instead of leaving a
# partially initialized wan0_ tuple while initialization reports success.
wan0_get = c_function_body(rc_source, "rc_wan0_nvram_get_owned(")
assert "nvram_get_checked(param_name, &cached_value)" in wan0_get
assert "rc_wan_nvram_read_status(api_result, saved_errno)" in wan0_get
assert "malloc(strlen(cached_value) + 1)" in wan0_get
assert "memcpy(*value, cached_value, strlen(cached_value) + 1)" in wan0_get
wan0_set_default = c_function_body(rc_source, "rc_wan0_nvram_set_default(")
assert "nvram_set_temp(param_name, value)" in wan0_set_default
assert "rc_wan_nvram_write_status(api_result, saved_errno)" in wan0_set_default
wan0_init = c_function_body(rc_source, "set_wan0_vars(void)")
assert "rc_wan_nvram_copy_missing_default(" in wan0_init
assert "rc_wan0_nvram_get_owned" in wan0_init
assert "rc_wan0_nvram_set_default" in wan0_init
assert "nvram_get(" not in wan0_init
assert "if (status != 0)" in wan0_init
assert "return status;" in wan0_init
assert "rc_wan_nvram_set_required_values(" in wan0_init
for field in (
    '"unit"', '"primary"', '"ifname"', '"uptime"', '"dltime"',
    '"bytes_rx"', '"bytes_tx"', '"ifname_t"',
):
    assert field in wan0_init, "WAN0 initialization field missing: " + field

char_to_ascii_impl = c_function_body(
    common_ex, "char_to_ascii(char *output, size_t output_size, const char *input)"
)
assert '#include "char_to_ascii_policy.h"' in common_ex
assert '#include "char_to_ascii_policy.h"' in rc_source
assert 'int char_to_ascii(char *output, size_t output_size, const char *input);' in rc_header
assert "void char_to_ascii(char *output, char *input);" not in rc_header
assert "#include <stddef.h>" in rc_header
assert "return rc_char_to_ascii_encode(output, output_size, input);" in char_to_ascii_impl
assert "sprintf(" not in char_to_ascii_impl and "strcpy(" not in char_to_ascii_impl
assert "const unsigned char *source" in char_to_ascii_policy
assert "input_length == RC_WLAN_SSID_MAX_LEN" in char_to_ascii_policy
assert "required > output_size" in char_to_ascii_policy
assert "hex[value >> 4]" in char_to_ascii_policy
assert "hex[value & 0x0f]" in char_to_ascii_policy
assert "errno = EMSGSIZE" in char_to_ascii_policy
assert "errno = ENOSPC" in char_to_ascii_policy
assert "RC_CHAR_TO_ASCII_MAX_OUTPUT_SIZE (RC_WLAN_SSID_MAX_LEN * 3U + 1U)" in char_to_ascii_policy
for marker in (
    "Bounded WLAN SSID percent-encoding regression: PASS",
    "RC_WLAN_SSID_MAX_LEN * 3U",
    '"ssid%20with%2525"',
    '"%C3%A9"',
    "errno == EMSGSIZE",
    "errno == ENOSPC",
):
    assert marker in char_to_ascii_test, marker

misc_init = c_function_body(rc_source, "nvram_convert_misc_values(void)")
assert misc_init.count("char_to_ascii(buff, sizeof(buff), nvram_wlan_get(") == 2
assert "char buff[RC_CHAR_TO_ASCII_MAX_OUTPUT_SIZE];" in misc_init
assert misc_init.count("if (char_to_ascii(") == 2
assert "return status;" in misc_init

assert "status = set_wan0_vars();" in misc_init
assert misc_init.index("status = set_wan0_vars();") < misc_init.index("set_usb_modem_dev_wan(0, 0);")
assert "if (status != 0) return status;" in re.sub(r"\s+", " ", misc_init)
net_wan_source = (root / "trunk/user/rc/src/net_wan.c").read_text()
wan_reset_definition = c_function_body(net_wan_source, "reset_wan_vars_status(void)")
assert "static int\nreset_wan_vars_status(void)" in net_wan_source
assert "return proto_status;" in wan_reset_definition
wan_reset_getter = c_function_body(net_wan_source, "get_wan_vars_reset_status(void)")
assert "return wan_vars_reset_status;" in wan_reset_getter
assert "int get_wan_vars_reset_status(void);" in rc_header

wan_reset_at = misc_init.index("reset_wan_vars();")
wan_reset_status_at = misc_init.index("status = get_wan_vars_reset_status();")
wan_reset_gate = misc_init[wan_reset_status_at:].split(
    "if (status != 0)", 1
)[1].split("/* Setup LAN identity", 1)[0]
assert "return status;" in wan_reset_gate
assert "reset_wan_vars_status();" not in misc_init
assert wan_reset_at < wan_reset_status_at < misc_init.index("reset_lan_vars_status();")

# LAN boot identity and runtime snapshot are checked operations. The cached
# MAC/address strings are copied before a write can invalidate NVRAM storage.
lan_get = c_function_body(net_lan_notification, "lan_nvram_get_checked(")
assert "nvram_get_checked(param_name, value)" in lan_get
lan_set = c_function_body(net_lan_notification, "lan_nvram_set_value_status(")
assert "nvram_set_temp(param_name, value)" in lan_set
assert "nvram_set(param_name, value)" in lan_set
assert "rc_wan_nvram_write_status(api_result, saved_errno)" in lan_set
lan_mac_reset = c_function_body(net_lan_notification, "reset_lan_vars_status(void)")
assert '"il0macaddr"' in lan_mac_reset
assert "rc_wan_nvram_copy_value(lan_nvram_get_checked" in lan_mac_reset
assert '"lan_hwaddr"' in lan_mac_reset
assert "rc_wan_nvram_set_required_values(" in lan_mac_reset
lan_temp_reset = c_function_body(net_lan_notification, "reset_lan_temp_status(void)")
for key in ('"lan_ipaddr"', '"lan_netmask"'):
    assert key in lan_temp_reset
assert "rc_wan_nvram_copy_snapshot(" in lan_temp_reset
assert 'snapshot_fields[0].param_name = "lan_ipaddr";' in lan_temp_reset
assert 'snapshot_fields[1].param_name = "lan_netmask";' in lan_temp_reset
assert "lan_dhcp_snapshot_begin" in lan_temp_reset
assert "lan_dhcp_snapshot_get" in lan_temp_reset
assert "lan_dhcp_snapshot_end" in lan_temp_reset
for key in (
    '"lan_ipaddr_t"', '"lan_netmask_t"', '"lan_gateway_t"',
    '"lan_domain_t"', '"lan_dns_t"',
):
    assert key in lan_temp_reset
assert "rc_wan_nvram_set_required_values(" in lan_temp_reset
assert "if (status != 0)" in lan_temp_reset

lan_mac_at = misc_init.index("status = reset_lan_vars_status();")
lan_mac_gate = misc_init[lan_mac_at:].split("if (status != 0)", 1)[1].split(
    "status = reset_lan_temp_status();", 1
)[0]
assert "return status;" in lan_mac_gate
lan_temp_at = misc_init.index("status = reset_lan_temp_status();")
lan_temp_gate = misc_init[lan_temp_at:].split("if (status != 0)", 1)[1].split(
    "time_zone_x_mapping();", 1
)[0]
assert "return status;" in lan_temp_gate
assert lan_mac_at < lan_temp_at < misc_init.index("time_zone_x_mapping();")
assert "int reset_lan_vars_status(void);" in rc_header
assert "int reset_lan_temp_status(void);" in rc_header

lan_nvram_test = (root / "trunk/user/rc/src/tests/wan_nvram_status_policy.c").read_text()
assert "test_lan_boot_runtime_snapshot_stops_at_each_failed_write" in lan_nvram_test
assert "test_lan_boot_runtime_snapshot_stops_at_each_failed_write();" in lan_nvram_test
assert '"trunk/user/rc/src/tests/wan_nvram_status_policy.c"' in process_workflow
assert "Compile and run WAN0 NVRAM initialization failure regression" in process_workflow
print("LAN MAC/default/runtime NVRAM fail-closed boot gate: PASS")

router_init = c_function_body(rc_source, "init_router(void)")
assert "nvram_setup_status = nvram_convert_misc_values();" in router_init
assert "if (nvram_setup_status != 0)" in router_init
failure_gate = router_init.split("if (nvram_setup_status != 0)", 1)[1].split("init_gpio_leds_buttons();", 1)[0]
assert "return INIT_ROUTER_RESULT_FATAL;" in failure_gate
assert router_init.index("nvram_setup_status = nvram_convert_misc_values();") < router_init.index("init_gpio_leds_buttons();")

wan_nvram_test = (root / "trunk/user/rc/src/tests/wan_nvram_status_policy.c").read_text()
wan_nvram_policy_header = (root / "trunk/user/rc/src/wan_nvram_status_policy.h").read_text()
assert "test_wan0_boot_values_stop_at_each_nvram_failure" in wan_nvram_test
assert "test_wan0_boot_values_stop_at_each_nvram_failure();" in wan_nvram_test
for case in (
    "test_wan0_default_copy_preserves_empty_existing_override",
    "test_wan0_default_copy_uses_checked_source_value",
    "test_wan0_default_copy_uses_factory_fallback_when_source_is_absent",
    "test_wan0_default_copy_stops_on_target_read_error",
    "test_wan0_default_copy_stops_on_source_read_error",
    "test_wan0_default_copy_propagates_write_error",
    "test_wan0_default_copy_rejects_invalid_arguments",
):
    assert case in wan_nvram_test
    assert case + "();" in wan_nvram_test
assert "rc_wan_nvram_copy_missing_default" in wan_nvram_policy_header
assert '"trunk/user/rc/src/tests/wan_nvram_status_policy.c"' in process_workflow
assert "Compile and run WAN0 NVRAM initialization failure regression" in process_workflow
print("WAN0 NVRAM defaults/runtime identity fail-closed boot gate: PASS")

shared_src = root / "trunk/user/shared/src"
netutils_source = (shared_src / "netutils.c").read_text()
netutils_header = (shared_src / "netutils.h").read_text()
ap_mode_policy = (shared_src / "ap_mode_read_policy.h").read_text()
ap_mode_test = (shared_src / "tests/ap_mode_read_policy.c").read_text()
rc_source = (root / "trunk/user/rc/src/rc.c").read_text()
net_source = (root / "trunk/user/rc/src/net.c").read_text()
wan_source = (root / "trunk/user/rc/src/net_wan.c").read_text()

ap_mode_reader = c_function_body(netutils_source, "get_ap_mode_status(int *is_ap_mode)")
assert 'nvram_get_checked("wan_route_x", &route_mode)' in ap_mode_reader
assert "rc_ap_mode_read_status(api_result, saved_errno, route_mode" in ap_mode_reader
assert "extern int       get_ap_mode_status(int *is_ap_mode);" in netutils_header
assert "rc_ap_mode_read_status" in ap_mode_policy
legacy_ap_mode = c_function_body(netutils_source, "get_ap_mode(void)")
assert "get_ap_mode_status(&is_ap_mode)" in legacy_ap_mode
assert "rc_ap_mode_read_legacy_fallback(status, is_ap_mode)" in legacy_ap_mode

usb_modules = c_function_body(rc_source, "load_usb_modules(void)")
assert "ap_mode_status = get_ap_mode_status(&is_ap_mode);" in usb_modules
assert "if (ap_mode_status != 0)" in usb_modules
assert "skipping modem-specific module selection" in usb_modules
assert "get_ap_mode()" not in usb_modules

# XHCI must use the checked integer reader and retain its legacy fallback
# without hiding NVRAM backend or parse failures from boot status.
# Normalize C whitespace before comparing calls: production formatting may
# wrap function arguments without changing the checked-read contract.
usb_modules_compact = " ".join(usb_modules.split())
usb3_read = 'nvram_status = rc_boot_nvram_read_int_status("usb3_disable", 0, &usb3_disable);'
assert usb3_read in usb_modules_compact
usb3_read_at = usb_modules_compact.index(usb3_read)
usb3_error_at = usb_modules_compact.index("if (nvram_status != 0)", usb3_read_at)
usb3_fallback_at = usb_modules_compact.index("usb3_disable = 0;", usb3_error_at)
usb3_error_block = usb_modules_compact[usb3_error_at:usb3_fallback_at]
assert "cannot read USB 3.0 disable setting during boot" in usb3_error_block
assert "rc_first_service_start_status" in usb3_error_block
usb3_param_at = usb_modules_compact.index("snprintf(xhci_param", usb3_fallback_at)
xhci_load_at = usb_modules_compact.index('load_boot_module("xhci_hcd", xhci_param)')
xhci_mtk_load_at = usb_modules_compact.index('load_boot_module("xhci_mtk", NULL)')
assert usb3_read_at < usb3_error_at < usb3_fallback_at < usb3_param_at < xhci_load_at < xhci_mtk_load_at
assert 'nvram_get_int("usb3_disable")' not in usb_modules_compact
print("USB XHCI checked NVRAM parameter and boot-status propagation: PASS")

router_init = c_function_body(rc_source, "init_router(void)")
assert 'nvram_get_checked("log_ipaddr", &log_remote_nvram_value)' in router_init
assert "firewall_nvram_string_value_status(" in router_init
assert "rc_logger_remote_start_deferred(" in router_init
assert 'nvram_invmatch("log_ipaddr", "")' not in router_init
assert router_init.index("rc_logger_remote_start_deferred(") < router_init.index("if (!log_remote)")
assert router_init.index("start_lan(is_ap_mode, 0)") < router_init.index("if (log_remote)")
assert "cannot determine remote syslog startup phase" in router_init
assert "rc_logger_remote_start_deferred" in logger_policy
assert "rc_logger_remote_start_deferred(EIO, NULL, &defer_until_lan) == EIO" in logger_test
mode_read = router_init.index("ap_mode_status = get_ap_mode_status(&is_ap_mode);")
mode_failure = router_init.index("if (ap_mode_status != 0)", mode_read)
mode_fallback = router_init.index("is_ap_mode = 1;", mode_failure)
mode_aggregate = router_init.index(
    "boot_services_status = rc_first_service_start_status(",
    mode_fallback
)
bridge_start = router_init.index("init_bridge(is_ap_mode, &switch_vlan_status);")
assert mode_read < mode_failure < mode_fallback < mode_aggregate < bridge_start
assert "blocking WAN startup" in router_init

firewall_restart = c_function_body(net_source, "restart_firewall(void)")
firewall_mode = firewall_restart.index("get_ap_mode_status(&is_ap_mode)")
firewall_failure = firewall_restart.index("if (ap_mode_status != 0)", firewall_mode)
firewall_mutation = firewall_restart.index("start_firewall_ex();")
assert firewall_mode < firewall_failure < firewall_mutation
assert "return ap_mode_status;" in firewall_restart[firewall_failure:firewall_mutation]

for signature in (
    "full_restart_wan(void)",
    "try_wan_reconnect(int try_use_modem, long pause_in_seconds)",
    "manual_wan_disconnect(void)",
    "get_wan_ip4(int only_broadband_wan)",
):
    function = c_function_body(wan_source, signature)
    assert "get_ap_mode_status(&is_ap_mode)" in function, signature
    assert "get_ap_mode()" not in function, signature
    if signature == "get_wan_ip4(int only_broadband_wan)":
        # A failed mode lookup must not masquerade as AP mode / no WAN address.
        assert "if (status != 0)" in function, signature
        status_failure = function.index("if (status != 0)")
        status_errno = function.index("errno = status;", status_failure)
        assert status_failure < status_errno
        assert "return INADDR_ANY;" in function, signature
    else:
        assert "if (ap_mode_status != 0)" in function, signature
        assert "return ap_mode_status;" in function, signature

for case in (
    "test_missing_and_routed_settings_keep_router_mode",
    "test_bridged_setting_selects_access_point_mode_exactly",
    "test_backend_failures_preserve_output_and_errno",
    "test_inconsistent_results_are_rejected_without_changing_output",
):
    assert case in ap_mode_test and case + "();" in ap_mode_test
assert "AP/router mode checked NVRAM read regression: PASS" in ap_mode_test
assert "test_legacy_boolean_fallback_blocks_router_mode_on_read_error();" in ap_mode_test
assert '"trunk/user/shared/src/ap_mode_read_policy.h"' in process_workflow
assert '"trunk/user/shared/src/tests/ap_mode_read_policy.c"' in process_workflow
assert "Compile and run AP/router mode checked-read regression" in process_workflow
xy_workflow = (root / ".github/workflows/xy-c1-firmware-validation.yml").read_text()
assert '"trunk/user/shared/src/tests/ap_mode_read_policy.c"' in xy_workflow
assert '"trunk/user/rc/src/rc.c"' in xy_workflow
assert '"trunk/user/rc/src/common_ex.c"' in xy_workflow
assert '"trunk/user/rc/src/rc.h"' in xy_workflow
assert '"trunk/user/rc/src/char_to_ascii_policy.h"' in xy_workflow
print("Checked AP/router mode NVRAM read and fail-closed boot/WAN/firewall gates: PASS")

shutdown = c_function_body(rc_source, "shutdown_router(int level)")
assert "get_ap_mode_status(&is_ap_mode)" in shutdown
assert "cannot determine AP/router mode during shutdown" in shutdown
assert "ap_mode_status == 0" in shutdown
assert "int is_ap_mode = 1;" in shutdown
notification_dispatch = c_function_body(rc_source, "handle_notifications(void)")
assert "get_ap_mode()" not in notification_dispatch
assert notification_dispatch.count("get_ap_mode_status(&is_ap_mode)") >= 6
for marker in (
    "IPv6 restart skipped because AP/router mode cannot be verified",
    "IPTV restart skipped because AP/router mode cannot be verified",
    "DDNS restart skipped because AP/router mode cannot be verified",
    "Internet detection skipped because AP/router mode cannot be verified",
    "DHCP service restart skipped because AP/router mode cannot be verified",
    "bridge configuration skipped because AP/router mode cannot be verified",
):
    assert marker in notification_dispatch, marker

nat_reload = c_function_body(net_source, "reload_nat_modules(void)")
assert "get_ap_mode_status(&is_ap_mode)" in nat_reload
assert "aborting NAT helper reload because AP/router mode cannot be verified" in nat_reload
assert "get_ap_mode()" not in nat_reload

internet_state = c_function_body(wan_source, "notify_on_internet_state_changed(int has_internet, long elapsed)")
assert "get_ap_mode_status(&is_ap_mode)" in internet_state
assert "router Internet-loss actions suppressed because AP/router mode cannot be verified" in internet_state
assert "get_ap_mode()" not in internet_state

services_source = (root / "trunk/user/rc/src/services_ex.c").read_text()
dnsmasq_restart = c_function_body(services_source, "restart_dhcpd_locked(void)")
assert "get_ap_mode_status(&is_ap_mode)" in dnsmasq_restart
assert "prepare_dns_dhcpd(is_ap_mode, &service_enabled)" in dnsmasq_restart
assert "DNSMasq restart aborted because AP/router mode cannot be verified" in dnsmasq_restart
samba_header = c_function_body(services_source, "write_smb_conf_header(FILE *fp)")
assert "get_ap_mode_status(&is_ap_mode)" in samba_header
assert "is_dhcpd_enabled_status(is_ap_mode, &dhcp_enabled)" in samba_header
samba_nvram_get = c_function_body(
    services_source, "samba_ex_nvram_get(void *context, const char *param_name, char **value)"
)
assert "return nvram_get_checked(param_name, value);" in samba_nvram_get
samba_nvram_copy = c_function_body(
    services_source, "samba_ex_copy_nvram_value(const char *param_name, char *destination,"
)
assert "rc_wan_nvram_copy_value(" in samba_nvram_copy
samba_header_compact = " ".join(samba_header.split())
for key, destination in (
    ("st_samba_workgroup", "workgroup"),
    ("dhcp_wins_x", "wins_server"),
):
    assert 'samba_ex_copy_nvram_value("' + key + '",' in samba_header_compact
    assert destination + ", sizeof(" + destination + ")" in samba_header_compact
assert "rc_wan_nvram_copy_text(get_our_hostname()," in samba_header_compact
assert "p_workgroup = workgroup;" in samba_header
assert "p_computer_name = computer_name;" in samba_header
assert 'nvram_safe_get("st_samba_workgroup")' not in samba_header
assert 'nvram_safe_get("dhcp_wins_x")' not in samba_header
samba_nvram_regression = (
    root / "trunk/user/rc/src/tests/wan_nvram_status_policy.c"
).read_text()
for test_case in (
    "test_samba_workgroup_and_wins_values_survive_shared_cache_overwrite",
    "test_samba_wins_read_error_preserves_workgroup_copy",
):
    assert test_case + "();" in samba_nvram_regression

miniupnpd_nvram_get = c_function_body(
    services_source,
    "miniupnpd_nvram_get_checked(void *context, const char *param_name,",
)
assert "return nvram_get_checked(param_name, value);" in miniupnpd_nvram_get
miniupnpd_nvram_copy = c_function_body(
    services_source, "miniupnpd_copy_nvram_value(const char *param_name, char *destination,"
)
assert "rc_wan_nvram_copy_value(" in miniupnpd_nvram_copy
miniupnpd_start = c_function_body(services_source, "start_upnp_locked(void)")
miniupnpd_start_compact = "".join(miniupnpd_start.split())
for key, destination in (
    ("lan_ipaddr", "lan_addr"),
    ("lan_netmask", "lan_mask"),
    ("lan_hwaddr", "lan_hwaddr"),
):
    assert 'miniupnpd_copy_nvram_value("' + key + '",' in miniupnpd_start_compact
    assert destination + ",sizeof(" + destination + "))" in miniupnpd_start_compact
assert "ether_atoe(lan_hwaddr,lan_mac);" in miniupnpd_start_compact
assert 'nvram_safe_get("lan_ipaddr")' not in miniupnpd_start
assert 'nvram_safe_get("lan_netmask")' not in miniupnpd_start
assert 'ether_atoe(nvram_safe_get("lan_hwaddr")' not in miniupnpd_start
assert miniupnpd_start_compact.index(
    'miniupnpd_copy_nvram_value("lan_ipaddr",'
) < miniupnpd_start_compact.index("miniupnpd_config_publish(")
for test_case in (
    "test_miniupnpd_lan_identity_survives_shared_cache_overwrite",
    "test_miniupnpd_lan_identity_read_failure_stops_snapshot",
):
    assert test_case + "();" in samba_nvram_regression
ddns_start = c_function_body(services_source, "start_ddns_locked(int clear_cache)")
assert "get_ap_mode_status(&is_ap_mode)" in ddns_start
assert "return ap_mode_status;" in ddns_start
ddns_update = c_function_body(services_source, "notify_ddns_update_locked(void)")
assert "get_ap_mode_status(&is_ap_mode)" in ddns_update
assert "is_ap_mode" in ddns_update
wan_param_setter = c_function_body(
    wan_source, "set_wan_unit_param_status(int unit, const char *param_name)"
)
assert "if (unit < 0 || !param_name || !param_name[0])" in wan_param_setter
assert wan_param_setter.index("if (unit < 0") < wan_param_setter.index("nvram_transaction_begin")

# Router hostname publication must not hide failures from the running kernel.
wan_router_hosts = c_function_body(wan_source, "update_hosts_router(const char *lan_ipaddr)")
wan_hostname_setter = c_function_body(
    wan_source, "wan_sethostname_status(const char *name, size_t length)"
)
wan_domain_setter = c_function_body(
    wan_source, "wan_setdomainname_status(const char *name, size_t length)"
)
assert '#include "lan_kernel_name_policy.h"' in wan_source
for setter, syscall in (
    (wan_hostname_setter, "sethostname(name, length)"),
    (wan_domain_setter, "setdomainname(name, length)"),
):
    assert syscall in setter
    assert "return errno ? errno : EIO;" in setter
assert "lan_kernel_names_apply(lan_hname, lan_dname, &wan_kernel_name_ops)" in " ".join(wan_router_hosts.split())
assert "sethostname(" not in wan_router_hosts
assert "setdomainname(" not in wan_router_hosts
assert wan_router_hosts.index("dnsmasq_file_publish_pair(") < wan_router_hosts.index("lan_kernel_names_apply(")
assert wan_router_hosts.index("lan_kernel_names_apply(") < wan_router_hosts.index("dnsmasq_lifecycle_lock_release(")
assert "return saved_errno;" in wan_router_hosts
assert "first_error=%d" in wan_router_hosts
lan_kernel_policy = (root / "trunk/user/rc/src/lan_kernel_name_policy.h").read_text()
lan_kernel_test = (root / "trunk/user/rc/src/tests/lan_kernel_name_policy.c").read_text()
assert "ops->set_hostname(hostname, strlen(hostname))" in lan_kernel_policy
assert "ops->set_domainname(domainname, strlen(domainname))" in lan_kernel_policy
for case in (
    "test_hostname_failure_does_not_skip_domain_update",
    "test_domain_failure_is_reported_after_hostname_update",
    "test_invalid_inputs_fail_before_system_calls",
):
    assert case + "();" in lan_kernel_test
assert 'lan_kernel_names_apply("router", "home.arpa", &mock_ops) == EPERM' in lan_kernel_test

print("Remaining AP/router mode consumers use checked NVRAM reads: PASS")

# Standalone OpenSSH stop must use the shared bounded lifecycle transaction.
openssh_script = (root / "trunk/user/openssh/sshd.sh").read_text()
openssh_makefile = (root / "trunk/user/openssh/Makefile").read_text()
openssh_lifecycle_test = (root / "trunk/user/openssh/tests/sshd_pidfile_lifecycle.sh").read_text()
openssh_stop = openssh_script.split("func_stop()\n{", 1)[1].split("\n}\n", 1)[0]
openssh_workflow = (root / ".github/workflows/process-lifecycle-regression.yml").read_text()
assert 'PROCESS_LIFECYCLE_LIB="${PROCESS_LIFECYCLE_LIB:-/usr/lib/process_lifecycle.sh}"' in openssh_script
assert '. "$PROCESS_LIFECYCLE_LIB"' in openssh_stop
assert "pl_stop_named sshd 3 2" in openssh_stop
assert "killall" not in openssh_stop
assert "shared process lifecycle helper is missing or unsafe" in openssh_stop
assert '$(THISDIR)/../shared/scripts/process_lifecycle.sh /usr/lib/process_lifecycle.sh' in openssh_makefile
assert "func_stop()" in openssh_lifecycle_test
assert 'MOCK_STOP_STATUS=37' in openssh_lifecycle_test
assert "missing helper did not fail closed" in openssh_lifecycle_test
assert "trunk/user/openssh/Makefile" in openssh_workflow
assert "trunk/user/openssh/tests/sshd_pidfile_lifecycle.sh" in openssh_workflow

# The AP relay monitor must retain its lock until both independent workers exit.
ap_script = (root / "trunk/user/scripts/files/usr/bin/ap_script.sh").read_text()
ap_test = (root / "trunk/user/scripts/tests/ap_script_lock_cleanup.sh").read_text()
ap_monitor_start = ap_script.index("ap_script_num=$(ps")
ap_monitor_end = ap_script.index("\nfi\n", ap_monitor_start) + len("\nfi")
ap_monitor_gate = ap_script[ap_monitor_start:ap_monitor_end]
assert 'if [[ $ap_script_num == "0" && $sh_ezscript_num == "0" ]]; then' in ap_monitor_gate
assert 'if [[ $ap_script_num == "0" && $ap_script_num == "0" ]]; then' not in ap_monitor_gate
for test_case in ("run_case helper_only keep", "run_case script_only keep",
                  "run_case both keep", "run_case none remove"):
    assert test_case in ap_test
process_workflow = Path(".github/workflows/process-lifecycle-regression.yml").read_text()
xy_workflow = Path(".github/workflows/xy-c1-firmware-validation.yml").read_text()
for changed_path in ("trunk/user/scripts/files/usr/bin/ap_script.sh",
                     "trunk/user/scripts/tests/ap_script_lock_cleanup.sh"):
    assert changed_path in process_workflow and changed_path in xy_workflow

# A module unload must fail closed when the sysfs refcount read is incomplete.
module_refcount_reader = c_function_body(
    common_ex, "get_module_refcount(const char *module_name)"
)
module_unloader = c_function_body(
    common_ex, "module_smart_unload(const char *module_name, int recurse_unload)"
)
assert "ferror(fp)" in module_refcount_reader
assert "fgetc(fp)" in module_refcount_reader
assert "trailing_char != EOF" in module_refcount_reader
assert "fclose(fp) != 0" in module_refcount_reader
assert "module_refcount_parse(mod_refval, &refcount)" in module_refcount_reader
assert module_refcount_reader.index("fgets(") < module_refcount_reader.index("ferror(fp)")
assert module_refcount_reader.index("ferror(fp)") < module_refcount_reader.index("fgetc(fp)")
assert module_refcount_reader.index("fgetc(fp)") < module_refcount_reader.index("fclose(fp)")
assert module_unloader.index("get_module_refcount(module_name)") < module_unloader.index("if (refcount < 0)")
assert module_unloader.index("if (refcount < 0)") < module_unloader.index("_eval(")
for token in (
    "module_refcount_parse",
    "INT_MAX",
    "digits >= 10",
):
    assert token in module_refcount_policy
for test_case in ("test_valid_refcounts", "test_invalid_or_partial_records"):
    assert "CHECK(" + test_case + "() == 0);" in module_refcount_test

# Page-cache reclaim must not mask NVRAM reads or kernel sysctl write failures.
pagecache_policy = (root / "trunk/user/rc/src/pagecache_reclaim_policy.h").read_text()
pagecache_test = (root / "trunk/user/rc/src/tests/pagecache_reclaim_policy.c").read_text()
pagecache_apply = c_function_body(common_ex, "int\nset_pagecache_reclaim(void)")
assert '#include "pagecache_reclaim_policy.h"' in common_ex
assert "nvram_get_checked(\"pcache_reclaim\", &pagecache_reclaim_value)" in pagecache_apply
assert "nvram_get_int(\"pcache_reclaim\")" not in pagecache_apply
assert pagecache_apply.index("rc_pagecache_reclaim_read_status(") < pagecache_apply.index('fput_int("/proc/sys/vm/pagecache_ratio"')
assert "rc_pagecache_reclaim_write_status(status, saved_errno)" in pagecache_apply
assert "errno = status;" in pagecache_apply
assert "rc_pagecache_reclaim_ratio" in pagecache_policy
for test_case in (
    "test_missing_setting_keeps_legacy_default",
    "test_present_values_keep_legacy_atoi_and_ratio_mapping",
    "test_read_failures_never_publish_a_default",
    "test_write_failures_are_preserved",
):
    assert "CHECK(" + test_case + "() == 0);" in pagecache_test

pagecache_boot = c_function_body(rc_source, "init_router(void)")
assert "int pagecache_status = set_pagecache_reclaim();" in pagecache_boot
assert "boot_services_status = rc_first_service_start_status(" in pagecache_boot
hddtune_event = rc_source.split(
    "strcmp(entry->d_name, RCN_RESTART_HDDTUNE) == 0", 1
)[1].split("#if defined(APP_FTPD)", 1)[0]
assert "int pagecache_status = set_pagecache_reclaim();" in hddtune_event
assert "action_result = rc_notify_merge_action_result(" in hddtune_event
assert "return status;" in pagecache_apply
print("RC page-cache reclaim NVRAM and sysctl failure propagation: PASS")
