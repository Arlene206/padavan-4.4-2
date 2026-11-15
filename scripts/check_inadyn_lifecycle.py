#!/usr/bin/env python3
"""Verify Inadyn lifecycle decisions and identity-bound configuration reload."""
from pathlib import Path

root = Path(__file__).resolve().parents[1]
source = (root / "trunk/user/rc/src/services_ex.c").read_text()
header = (root / "trunk/user/rc/src/rc.h").read_text()
policy = (root / "trunk/user/rc/src/inadyn_daemon_policy.h").read_text()
test = (root / "trunk/user/rc/src/tests/inadyn_daemon_policy.c").read_text()
workflow = (root / ".github/workflows/inadyn-lifecycle-stability.yml").read_text()
xy = (root / ".github/workflows/xy-c1-firmware-validation.yml").read_text()

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
assert "sig != SIGHUP" in policy
assert "process_identity_validate(\"inadyn\", identity)" in source
assert "kill(pid, signal_number)" in source
assert "test_config_preflight_requires_all_live_identities" in test
assert "test_identity_bound_hup_signal" in test
assert "UNKNOWN" in test and "SIGHUP" in test
assert "trunk/user/rc/src/inadyn_daemon_policy.h" in xy
assert "trunk/user/rc/src/services_ex.c" in workflow
print("Inadyn lifecycle tri-state and PID identity integration: PASS")
