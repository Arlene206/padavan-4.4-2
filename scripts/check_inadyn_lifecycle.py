#!/usr/bin/env python3
"""Verify Inadyn lifecycle and identity-bound config/manual-update signals."""
from pathlib import Path

root = Path(__file__).resolve().parents[1]
source = (root / "trunk/user/rc/src/services_ex.c").read_text()
header = (root / "trunk/user/rc/src/rc.h").read_text()
policy = (root / "trunk/user/rc/src/inadyn_daemon_policy.h").read_text()
test = (root / "trunk/user/rc/src/tests/inadyn_daemon_policy.c").read_text()
lock_header = (root / "trunk/user/rc/src/inadyn_lifecycle_lock.h").read_text()
lock_test = (root / "trunk/user/rc/src/tests/inadyn_lifecycle_lock.c").read_text()
workflow = (root / ".github/workflows/inadyn-lifecycle-stability.yml").read_text()
xy = (root / ".github/workflows/xy-c1-firmware-validation.yml").read_text()
webex = (root / "trunk/user/httpd/src/web_ex.c").read_text()
rc_source = (root / "trunk/user/rc/src/rc.c").read_text()
notify_header = (root / "trunk/user/shared/src/notify_rc.h").read_text()
config_file_header = (root / "trunk/user/rc/src/inadyn_config_file.h").read_text()
config_file_test = (root / "trunk/user/rc/src/tests/inadyn_config_file.c").read_text()

def body(signature):
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    state = "code"
    index = brace

    while index < len(source):
        current = source[index]
        following = source[index + 1] if index + 1 < len(source) else ""

        if state == "code":
            if current == "/" and following == "/":
                state = "line_comment"
                index += 2
                continue
            if current == "/" and following == "*":
                state = "block_comment"
                index += 2
                continue
            if current == '"':
                state = "string"
            elif current == "'":
                state = "character"
            elif current == "{":
                depth += 1
            elif current == "}":
                depth -= 1
                if depth == 0:
                    return source[start:index + 1]
        elif state in ("string", "character"):
            if current == chr(92):
                index += 2
                continue
            if (state == "string" and current == '"') or (
                    state == "character" and current == "'"):
                state = "code"
        elif state == "line_comment":
            if current == chr(10):
                state = "code"
        elif state == "block_comment":
            if current == "*" and following == "/":
                state = "code"
                index += 2
                continue

        index += 1

    raise AssertionError("unterminated function: " + signature)

notify = body("notify_ddns_update_locked(void)")
assert "process_inventory_scan(\"inadyn\", &snapshot)" in notify
assert "inadyn_action_for_presence(state)" in notify
assert "INADYN_DAEMON_ABORT" in notify and "INADYN_DAEMON_START" in notify
assert notify.index("inadyn_validate_snapshot(") < notify.index("write_inadyn_conf(DDNS_CONF_FILE)")
assert notify.index("write_inadyn_conf(DDNS_CONF_FILE)") < notify.index("inadyn_signal_snapshot(")
assert "process_snapshot_release(&snapshot)" in notify
assert 'doSystem("killall %s %s", "-SIGHUP", "inadyn")' not in notify
assert "pids(\"inadyn\")" not in notify

config_writer = body("write_inadyn_conf(const char *conf_file)")
assert config_writer.count("inadyn_config_file_write_quoted_setting(fp,") == 10
for value in ("ddns1_user", "ddns1_pass", "ddns1_svr", "ddns1_url",
              "ddns1_hname[0]", "ddns1_hname[1]", "ddns1_hname[2]",
              "ddns2_user", "ddns2_pass", "ddns2_hname"):
    assert value in config_writer
assert "goto write_failed;" in config_writer
assert "inadyn_config_file_discard(fp, staged_path)" in config_writer
assert "refusing unsafe or unwriteable Inadyn configuration value" in config_writer
assert "inadyn_config_file_write_escaped" in config_file_header
assert "case '\\\\':" in config_file_header
assert "case '\"':" in config_file_header
assert "case '\\n':" in config_file_header
assert "case '\\r':" in config_file_header
assert "case '\\t':" in config_file_header
assert "*cursor < 0x20 || *cursor == 0x7f" in config_file_header
assert "test_quoted_setting_escapes_configuration_delimiters" in config_file_test
assert "test_quoted_setting_rejects_control_bytes" in config_file_test
assert "test_rejected_value_discards_partial_config" in config_file_test
assert "trunk/user/rc/src/inadyn_config_file.h" in workflow
assert "trunk/user/rc/src/tests/inadyn_config_file.c" in workflow

start = body("start_ddns_locked(int clear_cache)")
assert "stop_status = stop_ddns_locked()" in start
assert start.index("if (stop_status != 0)") < start.index('eval("/bin/inadyn"')
assert 'launch_status = eval("/bin/inadyn"' in start
assert "inadyn_confirm_started(launch_status)" in start
assert "int launch_errno;" in start
assert "int start_status;" in start
assert "int start_errno;" in start
assert start.index("errno = 0;") < start.index('launch_status = eval("/bin/inadyn"')
assert "launch_errno = errno;" in start
assert start.count("inadyn_rollback_failed_start(") == 2
assert 'inadyn_rollback_failed_start(\n\t\t\tlaunch_status, start_errno, "launcher")' in start
assert 'inadyn_rollback_failed_start(\n\t\t\tstart_status, start_errno, "startup confirmation")' in start
assert start.index("inadyn_confirm_started(launch_status)") < start.index("inadyn_rollback_failed_start(\n\t\t\tstart_status")
rollback = body("inadyn_rollback_failed_start(int start_status, int start_errno,")
assert "stop_status = stop_ddns_locked();" in rollback
assert "presence = inadyn_process_presence(&scan_error);" in rollback
assert "inadyn_failed_start_may_leave_daemon(presence)" in rollback
assert "errno = start_errno > 0 ? start_errno : EIO;" in rollback
assert rollback.index("stop_ddns_locked()") < rollback.index("inadyn_process_presence(&scan_error)")
assert rollback.index("inadyn_process_presence(&scan_error)") < rollback.index("inadyn_failed_start_may_leave_daemon")
assert rollback.index("inadyn_failed_start_may_leave_daemon") < rollback.index("errno = start_errno > 0")
assert "inadyn_failed_start_may_leave_daemon(enum process_presence state)" in policy
assert "return state != PROCESS_PRESENCE_ABSENT;" in policy
assert "test_failed_start_may_leave_daemon" in test

restore = body("inadyn_restore_previous_instance(int previous_running)")
assert "inadyn_restore_action_for_state(previous_running, state)" in restore
assert "lstat(DDNS_CONF_FILE, &config_stat)" in restore
assert "S_ISREG(config_stat.st_mode)" in restore
assert "config_stat.st_uid != geteuid()" in restore
assert "config_stat.st_nlink != 1" in restore
assert "lstat(DDNS_CACHE_DIR, &cache_stat)" in restore
assert "S_ISDIR(cache_stat.st_mode)" in restore
assert "cache_stat.st_uid != geteuid()" in restore
assert "cache_stat.st_mode & 0777" in restore
assert "(config_stat.st_mode & 077) != 0" in restore
assert 'eval("/bin/inadyn"' in restore
assert "inadyn_confirm_started(launch_status)" in restore
assert "restored previous Inadyn daemon" in restore
assert "inadyn_process_presence(&scan_error)" in start
assert start.index("inadyn_process_presence(&scan_error)") < start.index("stop_status = stop_ddns_locked()")
assert start.count("inadyn_restore_previous_instance(previous_running)") == 2
assert "test_restore_previous_instance_action" in test
assert "INADYN_RESTORE_LAUNCH" in policy and "INADYN_RESTORE_ABORT" in policy

assert "inadyn_prepare_cache_directory(" in start
assert start.index("inadyn_prepare_cache_directory(") < start.index("write_inadyn_conf(DDNS_CONF_FILE)")
assert start.index("write_inadyn_conf(DDNS_CONF_FILE)") < start.index('eval("/bin/inadyn"')
assert "cache_status != 0" in start
assert "inadyn_cache_clear_cb" in source and "inadyn_cache_lstat_cb" in source
assert "inadyn_cache_chmod_cb" in source
assert "inadyn_cache_chmod_cb, NULL" in source
assert 'doSystem("rm -rf %s", path)' in source
assert "inadyn_cache_command_status(command_status, errno)" in source
assert "inadyn_prepare_cache_directory" in policy
assert "command_status == -1 && command_errno > 0" in policy
assert "S_ISDIR(st.st_mode)" in policy
assert "st.st_uid != geteuid()" in policy
assert "chmod_fn(context, path, 0700)" in policy
assert "st.st_mode & 0777" in policy
assert "test_cache_prepare_status" in test
assert "doSystem(\"rm -rf %s\", DDNS_CACHE_DIR)" not in start
assert "mkdir(DDNS_CACHE_DIR, 0777)" not in start
assert "ret = start_ddns(1);" in rc_source
assert "return start_ddns_locked(0);" in source
assert "return ret;" in rc_source
confirm = body("inadyn_confirm_started(int launch_status)")
assert "inadyn_process_presence(&scan_error)" in confirm
assert "INADYN_START_WAIT_ATTEMPTS" in confirm
assert 'service_start_wait_one_second("Inadyn")' in confirm
assert "sleep(1)" not in confirm
presence = body("inadyn_process_presence(int *scan_error)")
assert 'process_inventory_scan("inadyn", &snapshot)' in presence
assert "inadyn_start_status(launch_status, state, scan_error)" in confirm
stop = body("stop_ddns_locked(void)")
assert "return kill_services(svcs, 3, 1)" in stop
for entry in (body("stop_ddns(void)"), body("start_ddns(int clear_cache)"),
              body("notify_ddns_update(void)"), body("notify_ddns_manual_update(void)")):
    assert "inadyn_lifecycle_lock_acquire_status(" in entry
    assert "inadyn_lifecycle_lock_finish(" in entry
assert "start_ddns_locked(0)" in notify
assert '#include "inadyn_lifecycle_lock.h"' in source
assert "O_NOFOLLOW" in lock_header and "F_SETLK" in lock_header
assert "st.st_nlink != 1" in lock_header and "(st.st_mode & 077) != 0" in lock_header
assert "INADYN_LIFECYCLE_LOCK_WAIT_SECONDS 10" in lock_header
assert "test_exclusive_process_lock" in lock_test
assert "test_invalid_paths_fail_closed" in lock_test
assert "errno == EBUSY" in lock_test and "errno == ELOOP" in lock_test
assert "trunk/user/rc/src/inadyn_lifecycle_lock.h" in workflow
assert "trunk/user/rc/src/tests/inadyn_lifecycle_lock.c" in workflow
assert "Compile Inadyn exclusive lifecycle lock regression" in workflow
assert "trunk/user/rc/src/inadyn_lifecycle_lock.h" in xy
assert "trunk/user/rc/src/tests/inadyn_lifecycle_lock.c" in xy
assert "int stop_ddns(void);" in header
assert "INADYN_DAEMON_RELOAD" in policy
assert "state == PROCESS_PRESENCE_UNKNOWN" in policy
assert "sig != SIGHUP && sig != SIGUSR1" in policy
assert "process_identity_validate(\"inadyn\", identity)" in source
assert "kill(pid, signal_number)" in source
assert "test_config_preflight_requires_all_live_identities" in test
assert "test_start_postcondition_status" in test
assert "test_identity_bound_hup_signal" in test
assert "test_identity_bound_usr1_signal" in test
assert "UNKNOWN" in test and "SIGHUP" in test and "SIGUSR1" in test
assert "trunk/user/rc/src/inadyn_daemon_policy.h" in xy
assert "trunk/user/rc/src/services_ex.c" in workflow

manual_update = body("notify_ddns_manual_update_locked(void)")
assert 'process_inventory_scan("inadyn", &snapshot)' in manual_update
assert "PROCESS_PRESENCE_UNKNOWN" in manual_update
assert "PROCESS_PRESENCE_ABSENT" in manual_update
assert "inadyn_validate_snapshot(" in manual_update
assert "inadyn_signal_snapshot(&snapshot, SIGUSR1" in manual_update
assert "process_snapshot_release(&snapshot)" in manual_update
ddns_start = webex.index('else if (strcmp(name,"ddnsclient")==0)')
ddns_end = webex.index('else if (strcmp(name,"hostname_check")', ddns_start)
ddns_button = webex[ddns_start:ddns_end]
assert "notify_rc(RCN_MANUAL_DDNS_UPDATE)" in ddns_button
assert "pids(" not in ddns_button and "killall" not in ddns_button
assert '#define RCN_MANUAL_DDNS_UPDATE' in notify_header
assert "strcmp(entry->d_name, RCN_MANUAL_DDNS_UPDATE)" in rc_source
assert "notify_ddns_manual_update()" in rc_source
assert "int notify_ddns_manual_update(void);" in header

print("Inadyn lifecycle tri-state and PID identity integration: PASS")
