#!/usr/bin/env python3
"""Verify Inadyn lifecycle and identity-bound config/manual-update signals."""
from pathlib import Path

root = Path(__file__).resolve().parents[1]
source = (root / "trunk/user/rc/src/services_ex.c").read_text()
header = (root / "trunk/user/rc/src/rc.h").read_text()
policy = (root / "trunk/user/rc/src/inadyn_daemon_policy.h").read_text()
test = (root / "trunk/user/rc/src/tests/inadyn_daemon_policy.c").read_text()
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
    for index in range(brace, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[start:index + 1]
    raise AssertionError("unterminated function: " + signature)

notify = body("notify_ddns_update(void)")
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
assert "trunk/user/rc/src/inadyn_config_file.h" in workflow
assert "trunk/user/rc/src/tests/inadyn_config_file.c" in workflow

start = body("start_ddns(int clear_cache)")
assert "stop_status = stop_ddns()" in start
assert start.index("if (stop_status != 0)") < start.index('eval("/bin/inadyn"')
assert 'launch_status = eval("/bin/inadyn"' in start
assert "inadyn_confirm_started(launch_status)" in start
assert "inadyn_prepare_cache_directory(" in start
assert start.index("inadyn_prepare_cache_directory(") < start.index("write_inadyn_conf(DDNS_CONF_FILE)")
assert start.index("write_inadyn_conf(DDNS_CONF_FILE)") < start.index('eval("/bin/inadyn"')
assert "cache_status != 0" in start
assert "inadyn_cache_clear_cb" in source and "inadyn_cache_lstat_cb" in source
assert 'doSystem("rm -rf %s", path)' in source
assert "inadyn_cache_command_status(command_status, errno)" in source
assert "inadyn_prepare_cache_directory" in policy
assert "command_status == -1 && command_errno > 0" in policy
assert "S_ISDIR(st.st_mode)" in policy
assert "test_cache_prepare_status" in test
assert "doSystem(\"rm -rf %s\", DDNS_CACHE_DIR)" not in start
assert "mkdir(DDNS_CACHE_DIR, 0777)" not in start
assert "ret = start_ddns(1);" in rc_source
assert "return start_ddns(0);" in source
assert "return ret;" in rc_source
confirm = body("inadyn_confirm_started(int launch_status)")
assert "inadyn_process_presence(&scan_error)" in confirm
assert "INADYN_START_WAIT_ATTEMPTS" in confirm and "sleep(1)" in confirm
presence = body("inadyn_process_presence(int *scan_error)")
assert 'process_inventory_scan("inadyn", &snapshot)' in presence
assert "inadyn_start_status(launch_status, state, scan_error)" in confirm
stop = body("stop_ddns(void)")
assert "return kill_services(svcs, 3, 1)" in stop
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

manual_update = body("notify_ddns_manual_update(void)")
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
