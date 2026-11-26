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
enabled_policy = (root / "trunk/user/rc/src/ddns_enabled_policy.h").read_text()
restart_policy = (root / "trunk/user/rc/src/ddns_restart_policy.h").read_text()
restart_test = (root / "trunk/user/rc/src/tests/ddns_restart_policy.c").read_text()

def body(signature):
    # Skip forward declarations: source.index("{", start) can silently select
    # the next unrelated function body and make this contract test misleading.
    search_from = 0
    while True:
        start = source.find(signature, search_from)
        if start < 0:
            raise AssertionError("function definition not found: " + signature)
        after_signature = start + len(signature)
        brace = source.find("{", after_signature)
        declaration_end = source.find(";", after_signature)
        if brace >= 0 and (declaration_end < 0 or brace < declaration_end):
            break
        search_from = after_signature
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

config_snapshot_header = (root / "trunk/user/rc/src/inadyn_config_file.h").read_text()
config_snapshot_test = (root / "trunk/user/rc/src/tests/inadyn_config_file.c").read_text()

rollback = body("inadyn_rollback_failed_start(int start_status, int start_errno,")
assert "inadyn_failed_start_may_leave_daemon(presence)" in rollback
assert rollback.index("inadyn_failed_start_may_leave_daemon(presence)") < rollback.index(
    "inadyn_config_file_snapshot_restore(")
assert "inadyn_restore_previous_instance(previous_running)" in rollback
assert "previous config snapshot retained for manual recovery" in rollback

start = body("start_ddns_locked(int clear_cache)")
assert start.index("inadyn_config_file_snapshot_begin(") < start.index(
    "write_inadyn_conf(DDNS_CONF_FILE)")
assert start.index("inadyn_confirm_started(launch_status)") < start.index(
    "inadyn_config_file_snapshot_commit(")
assert start.index("inadyn_rollback_failed_start(") < start.index(
    "inadyn_config_file_snapshot_commit(")
assert "previous_running, &config_snapshot" in start

assert "link(destination, snapshot->backup_path)" in config_snapshot_header
assert "backup.st_dev != original.st_dev" in config_snapshot_header
assert "backup.st_ino != original.st_ino" in config_snapshot_header
assert "O_DIRECTORY | O_NOFOLLOW" in config_snapshot_header
assert "inadyn_config_file_snapshot_restore" in config_snapshot_test
assert "lstat(destination, &current)" in config_snapshot_header
assert "current.st_ino == snapshot->inode" in config_snapshot_header
assert "unlink(snapshot->backup_path)" in config_snapshot_header

assert "inadyn_config_file_snapshot_commit" in config_snapshot_test
assert "test_config_snapshot_rejects_symlink" in config_snapshot_test

notify = body("notify_ddns_update_locked(void)")
assert "inadyn_process_inventory_scan(&snapshot)" in notify
assert "inadyn_action_for_presence(state)" in notify
assert "INADYN_DAEMON_ABORT" in notify and "INADYN_DAEMON_START" in notify
assert notify.index("inadyn_validate_snapshot(") < notify.index("write_inadyn_conf(DDNS_CONF_FILE)")
assert notify.index("write_inadyn_conf(DDNS_CONF_FILE)") < notify.index("inadyn_signal_snapshot(")
assert "process_snapshot_release(&snapshot)" in notify
assert 'doSystem("killall %s %s", "-SIGHUP", "inadyn")' not in notify
assert "pids(\"inadyn\")" not in notify
enabled_read = notify.index('enabled_api_result = nvram_get_checked("ddns_enable_x", &enabled_value)')
enabled_status = notify.index("enabled_status = inadyn_ddns_enabled_status(", enabled_read)
enabled_gate = notify.index("inadyn_update_should_run(ddns_enabled, is_ap_mode)", enabled_status)
inventory_scan = notify.index("inadyn_process_inventory_scan(&snapshot)")
assert enabled_read < enabled_status < enabled_gate < inventory_scan
assert "return enabled_status;" in notify
assert "test_ddns_enabled_read_status" in test
assert "inadyn_ddns_enabled_status" in enabled_policy
assert '#include "ddns_enabled_policy.h"' in source

assert "static inline int" in restart_policy
assert "rc_ddns_restart_run(int ap_mode, int ddns_enabled," in restart_policy
assert "(ap_mode != 0 && ap_mode != 1)" in restart_policy
assert "(ddns_enabled != 0 && ddns_enabled != 1)" in restart_policy
assert "if (ap_mode || !ddns_enabled)" in restart_policy
for case in (
    "test_enabled_router_uses_one_start_transaction",
    "test_enabled_router_propagates_start_failure",
    "test_disabled_ddns_stops_without_starting",
    "test_disabled_ddns_propagates_stop_failure",
    "test_ap_mode_stops_even_when_ddns_is_enabled",
    "test_ap_mode_propagates_stop_failure",
    "test_invalid_callbacks_fail_without_side_effects",
    "test_invalid_mode_values_fail_without_side_effects",
):
    assert case in restart_test
assert "rc_notify_merge_action_result(0, start_status) == -1" in restart_test
assert "rc_notify_merge_action_result(0, stop_status) == -1" in restart_test
assert "trunk/user/rc/src/ddns_restart_policy.h" in workflow
assert "trunk/user/rc/src/tests/ddns_restart_policy.c" in workflow
assert "Compile DDNS restart dispatch validation regression" in workflow
assert 'ASAN_OPTIONS=detect_leaks=1 "$RUNNER_TEMP/ddns-restart-policy"' in workflow


assert "inadyn_config_file_read_integer_status" in config_file_header
assert "inadyn_config_file_clear_sensitive" in config_file_header
assert "volatile unsigned char *cursor" in config_file_header
assert "test_sensitive_snapshot_clear_wipes_every_byte" in config_file_test
assert "test_checked_integer_status_preserves_defaults_and_rejects_errors" in config_file_test

config_writer = body("write_inadyn_conf(const char *conf_file)")
assert config_writer.count("inadyn_config_file_clear_sensitive(") == 2
assert config_writer.count("free(config_storage);") == 2
assert config_writer.index("inadyn_config_file_clear_sensitive(") < config_writer.index("free(config_storage);")
assert config_writer.rindex("inadyn_config_file_clear_sensitive(") < config_writer.rindex("free(config_storage);")
snapshot_call = config_writer.index("rc_wan_nvram_copy_snapshot(")
parse_first = config_writer.index('inadyn_parse_config_integer("ddns_period"')
config_open = config_writer.index("inadyn_config_file_open(")
assert snapshot_call < parse_first < config_open
assert "INADYN_CFG_FIELD_COUNT" in config_writer
assert "INADYN_CONFIG_VALUE_SIZE" in config_writer
assert "inadyn_nvram_snapshot_begin" in config_writer
assert "inadyn_nvram_snapshot_get" in config_writer
assert "inadyn_nvram_snapshot_end" in config_writer
assert "inadyn_read_nvram_integer(" not in source
assert "nvram_safe_get(" not in config_writer
assert "nvram_safe_get_int(" not in config_writer
config_names = source.split("inadyn_config_nvram_names", 1)[1].split("};", 1)[0]
for key in ("ddns_period", "ddns_forced", "ddns_source", "ddns_ipv6",
            "ddns_ssl", "ddns_wildcard_x", "ddns2_ssl", "ddns2_wildcard_x",
            "ddns_checkip", "ddns2_checkip", "ddns_server_x", "ddns_hostname_x",
            "ddns_hostname2_x", "ddns_hostname3_x", "ddns_username_x",
            "ddns_passwd_x", "ddns_cst_svr", "ddns_cst_url", "ddns2_server",
            "ddns2_hname", "ddns2_user", "ddns2_pass"):
    assert '"' + key + '"' in config_names, "snapshot omits " + key
assert 'i_ddns1_wildcard ? "true" : "false"' in config_writer
assert 'i_ddns2_ssl ? "true" : "false"' in config_writer
assert 'i_ddns2_wildcard ? "true" : "false"' in config_writer
assert config_writer.count("inadyn_config_file_write_quoted_setting(fp,") == 10

# Optional ENOENT remains a success in load_user_config(), but an actual
# overlay copy/read/flush/close error must discard the whole Inadyn stage.
user_overlay_load = config_writer.index(
    'user_config_status = load_user_config(fp, INADYN_USER_DIR, "inadyn.conf", NULL);'
)
user_overlay_failure = config_writer.index("if (user_config_status != 0)", user_overlay_load)
user_overlay_publish = config_writer.index(
    "inadyn_config_file_publish(fp, staged_path, conf_file)", user_overlay_load
)
user_overlay_guard = config_writer[user_overlay_failure:user_overlay_publish]
assert user_overlay_load < user_overlay_failure < user_overlay_publish
assert "saved_errno = user_config_status > 0 ? user_config_status : EIO;" in user_overlay_guard
assert "cannot copy user Inadyn configuration overlay" in user_overlay_guard
assert "goto failed;" in user_overlay_guard
assert "inadyn_config_file_discard(fp, staged_path)" in config_writer

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
start_enable_read = start.index('enabled_api_result = nvram_get_checked("ddns_enable_x", &enabled_value)')
start_enable_policy = start.index("enabled_status = inadyn_ddns_enabled_status(", start_enable_read)
start_enable_error = start.index("if (enabled_status != 0)", start_enable_policy)
start_enable_return = start.index("return enabled_status;", start_enable_error)
start_inventory = start.index("inadyn_process_presence(&scan_error)", start_enable_return)
start_stop = start.index("stop_status = stop_ddns_locked()", start_inventory)
start_disabled_gate = start.index("if (!ddns_enabled)", start_stop)
start_cache = start.index("inadyn_prepare_cache_directory(", start_disabled_gate)
assert start_enable_read < start_enable_policy < start_enable_error < start_enable_return
assert start_enable_return < start_inventory < start_stop < start_disabled_gate < start_cache
assert 'nvram_match("ddns_enable_x", "1")' not in start
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
assert 'inadyn_rollback_failed_start(\n\t\t\tlaunch_status, start_errno, "launcher",\n\t\t\tprevious_running, &config_snapshot)' in start
assert 'inadyn_rollback_failed_start(\n\t\t\tstart_status, start_errno, "startup confirmation",\n\t\t\tprevious_running, &config_snapshot)' in start
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
assert start.count("inadyn_restore_previous_instance(previous_running)") == 3
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
assert "inadyn_process_inventory_scan(&snapshot)" in presence
assert "inadyn_start_status(launch_status, state, scan_error)" in confirm
stop = body("stop_ddns_locked(void)")
assert "service_lifecycle_stop(svcs, 3, 1," in stop
assert "&inadyn_service_lifecycle_ops" in stop
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
assert "trunk/user/rc/src/ddns_enabled_policy.h" in workflow
assert "trunk/user/rc/src/tests/inadyn_lifecycle_lock.c" in workflow
assert "Compile Inadyn exclusive lifecycle lock regression" in workflow
assert "trunk/user/rc/src/inadyn_lifecycle_lock.h" in xy
assert "trunk/user/rc/src/tests/inadyn_lifecycle_lock.c" in xy
assert "int stop_ddns(void);" in header
assert "INADYN_DAEMON_RELOAD" in policy
assert "state == PROCESS_PRESENCE_UNKNOWN" in policy
assert "sig != SIGHUP && sig != SIGUSR1" in policy
assert "process_identity_validate(\"inadyn\", identity)" in source
assert 'inadyn_executable_identity_validate(' in body("inadyn_validate_identity(void *context,")
assert '"/bin/inadyn"' in body("inadyn_validate_identity(void *context,")
assert 'readlink(proc_path, path, path_size - 1)' in body("inadyn_read_executable_path(void *context, pid_t pid,")
assert "inadyn_process_inventory_scan(snapshot)" in body("inadyn_lifecycle_scan(void *context, const char *service_name,")
assert "signal_number != SIGTERM && signal_number != SIGKILL" in body("inadyn_lifecycle_send_signal(void *context,")
assert "service_lifecycle_policy.h" in source
assert "inadyn_executable_identity_validate" in policy
assert " (deleted)" in policy
assert "test_executable_bound_inadyn_identity" in test
assert "kill(pid, signal_number)" in source
assert "test_config_preflight_requires_all_live_identities" in test
assert "test_start_postcondition_status" in test
assert "test_identity_bound_hup_signal" in test
assert "test_identity_bound_usr1_signal" in test
assert "UNKNOWN" in test and "SIGHUP" in test and "SIGUSR1" in test
assert "trunk/user/rc/src/inadyn_daemon_policy.h" in xy
assert "trunk/user/rc/src/services_ex.c" in workflow

manual_update = body("notify_ddns_manual_update_locked(void)")
assert "inadyn_process_inventory_scan(&snapshot)" in manual_update
assert "PROCESS_PRESENCE_UNKNOWN" in manual_update
assert "PROCESS_PRESENCE_ABSENT" in manual_update
assert "inadyn_validate_snapshot(" in manual_update
assert "inadyn_signal_snapshot(&snapshot, SIGUSR1" in manual_update
assert "process_snapshot_release(&snapshot)" in manual_update
ddns_start = rc_source.index('else if (strcmp(entry->d_name, RCN_RESTART_DDNS) == 0)')
ddns_end = rc_source.index('else if (strcmp(entry->d_name, RCN_RESTART_DI)', ddns_start)
ddns_restart = rc_source[ddns_start:ddns_end]
ddns_read = ddns_restart.index('ddns_enabled_api_result = nvram_get_checked("ddns_enable_x",')
ddns_check = ddns_restart.index("inadyn_ddns_enabled_status(", ddns_read)
ddns_error = ddns_restart.index("if (ddns_enabled_status != 0)", ddns_check)
ddns_error_result = ddns_restart.index("ddns_status = ddns_enabled_status;", ddns_error)
ddns_restart_call = ddns_restart.index("rc_ddns_restart_run(is_ap_mode,", ddns_error_result)
assert ddns_read < ddns_check < ddns_error < ddns_error_result < ddns_restart_call
assert 'nvram_match("ddns_enable_x", "1")' not in ddns_restart
assert '#include "ddns_enabled_policy.h"' in rc_source
ddns_start = webex.index('else if (strcmp(name,"ddnsclient")==0)')
ddns_end = webex.index('else if (strcmp(name,"hostname_check")', ddns_start)
ddns_button = webex[ddns_start:ddns_end]
assert "notify_rc(RCN_MANUAL_DDNS_UPDATE)" in ddns_button
assert "pids(" not in ddns_button and "killall" not in ddns_button
assert '#define RCN_MANUAL_DDNS_UPDATE' in notify_header
assert "strcmp(entry->d_name, RCN_MANUAL_DDNS_UPDATE)" in rc_source
assert "notify_ddns_manual_update()" in rc_source
assert "int notify_ddns_manual_update(void);" in header

print("Inadyn last-known-good config rollback and PID identity integration: PASS")
