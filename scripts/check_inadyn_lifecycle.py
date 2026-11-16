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

start = body("start_ddns(int clear_cache)")
assert "stop_status = stop_ddns()" in start
assert start.index("if (stop_status != 0)") < start.index('eval("/bin/inadyn"')
stop = body("stop_ddns(void)")
assert "return kill_services(svcs, 3, 1)" in stop
assert "int stop_ddns(void);" in header
assert "INADYN_DAEMON_RELOAD" in policy
assert "state == PROCESS_PRESENCE_UNKNOWN" in policy
assert "sig != SIGHUP && sig != SIGUSR1" in policy
assert "process_identity_validate(\"inadyn\", identity)" in source
assert "kill(pid, signal_number)" in source
assert "test_config_preflight_requires_all_live_identities" in test
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
