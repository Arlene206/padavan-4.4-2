#!/usr/bin/env python3
"""Check fail-closed SSH listener handling after public firewall refresh errors."""

from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
RC = ROOT / "trunk/user/rc/src"
SERVICES = (RC / "services.c").read_text()
POLICY = (RC / "sshd_daemon_policy.h").read_text()
TEST = (RC / "tests/sshd_daemon_policy.c").read_text()


def body(source: str, signature: str) -> str:
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    for pos in range(brace, len(source)):
        if source[pos] == "{":
            depth += 1
        elif source[pos] == "}":
            depth -= 1
            if depth == 0:
                return source[brace + 1:pos]
    raise AssertionError("unterminated function: " + signature)


def compact(source: str) -> str:
    return re.sub(r"\s+", "", source)


assert "sshd_firewall_failure_requires_stop" in POLICY
policy_body = body(POLICY, "sshd_firewall_failure_requires_stop(")
assert "firewall_status!=0" in compact(policy_body)
assert "after_state!=PROCESS_PRESENCE_ABSENT" in compact(policy_body)

restart = body(SERVICES, "restart_sshd(void)")
assert "sshd_firewall_refresh_required(before_state, after_state)" in restart
assert 'nvram_match("sshd_wopen", "1")' in restart
assert 'nvram_match("fw_enable_x", "1")' in restart
assert "errno = 0;" in restart
assert "firewall_status = restart_firewall();" in restart
assert "firewall_errno = errno > 0 ? errno :" in restart
assert "sshd_firewall_failure_requires_stop(firewall_status, after_state)" in restart
assert "firewall_stop_status = stop_sshd();" in restart
assert "firewall_stop_state = sshd_process_presence(&firewall_stop_scan_error);" in restart
assert "if (firewall_stop_state == PROCESS_PRESENCE_ABSENT)" in restart
assert "after_state = PROCESS_PRESENCE_ABSENT;" in restart
assert "SSH listener confirmed stopped after firewall refresh failure" in restart
assert "SSH listener may remain active after firewall refresh failure" in restart
assert "if (firewall_status != 0)\n\t\terrno = firewall_errno;" in restart

order = [
    restart.index("firewall_status = restart_firewall();"),
    restart.index("if (sshd_firewall_failure_requires_stop("),
    restart.index("firewall_stop_status = stop_sshd();"),
    restart.index("firewall_stop_state = sshd_process_presence("),
    restart.index("if (firewall_stop_state == PROCESS_PRESENCE_ABSENT)"),
    restart.index("if (start_status != 0)"),
]
assert order == sorted(order)

assert "test_firewall_failure_requires_listener_shutdown();" in TEST
test = body(TEST, "test_firewall_failure_requires_listener_shutdown(void)")
assert test.count("sshd_firewall_failure_requires_stop(") == 6
for state in ("PROCESS_PRESENCE_ABSENT","PROCESS_PRESENCE_RUNNING",
              "PROCESS_PRESENCE_UNKNOWN","(enum process_presence)9"):
    assert state in test

print("SSH public-firewall failure listener shutdown contract: PASS")

# SSH/Dropbear detach from their launcher: verify the child, not eval(), before
# reporting success. Also gate the broad stop wrapper on a known initial state.
start = body(SERVICES, "int start_sshd(void)")
start_policy = body(POLICY, "sshd_start_postcondition(")
restart_preflight_policy = body(POLICY, "sshd_restart_before_state_allows_stop(")
restart_stop_policy = body(POLICY, "sshd_restart_requires_stop(")
restart = body(SERVICES, "int restart_sshd(void)")

assert "process_presence_start_postcondition" in start_policy
assert "PROCESS_PRESENCE_ABSENT" in restart_preflight_policy
assert "PROCESS_PRESENCE_RUNNING" in restart_preflight_policy
assert "process_state == PROCESS_PRESENCE_RUNNING" in restart_stop_policy
assert "test_restart_initial_inventory_gates_name_based_stop();" in TEST
assert "test_detached_start_requires_running_child();" in TEST

assert "if (state == PROCESS_PRESENCE_RUNNING)" in start
assert "if (!sshd_start_allowed(state))" in start
assert 'start_status = eval("/usr/bin/sshd.sh", "start", "-s");' in start
assert 'start_status = eval("/usr/bin/sshd.sh", "start");' in start
assert "if (start_status != 0)" in start
assert "PROCESS_PRESENCE_START_INITIAL_WAIT_SECONDS" in start
assert "PROCESS_PRESENCE_START_PROBE_ATTEMPTS" in start
assert "PROCESS_PRESENCE_START_RETRY_WAIT_SECONDS" in start
assert "sshd_process_presence(&scan_error)" in start
assert "sshd_start_postcondition(start_status, state) == 0" in start
assert "if (state == PROCESS_PRESENCE_UNKNOWN)" in start
assert "errno = scan_error > 0 ? scan_error : EIO;" in start
assert "errno = ETIMEDOUT;" in start
assert "sshd_start_postcondition(start_status, PROCESS_PRESENCE_ABSENT)" in start
start_order = (
    start.index("if (!sshd_start_allowed(state))"),
    start.index('start_status = eval("/usr/bin/sshd.sh", "start"'),
    start.index("if (start_status != 0)"),
    start.index('service_start_probe_wait("SSH", PROCESS_PRESENCE_START_INITIAL_WAIT_SECONDS);'),
    start.index("for (attempt = 0; attempt < PROCESS_PRESENCE_START_PROBE_ATTEMPTS; attempt++)"),
    start.index("state = sshd_process_presence(&scan_error)", start.index("for (attempt")),
    start.index("sshd_start_postcondition(start_status, state) == 0"),
)
assert start_order == tuple(sorted(start_order))

assert "before_state = sshd_process_presence(&scan_error);" in restart
assert "if (!sshd_restart_before_state_allows_stop(before_state))" in restart
assert "if (sshd_restart_requires_stop(before_state))" in restart
assert "stop_status = 0;" in restart
assert "stop_status = stop_sshd();" in restart
assert "stopped_state = sshd_process_presence(&scan_error);" in restart
assert "sshd_restart_can_start(stop_status, stopped_state)" in restart
restart_order = (
    restart.index("before_state = sshd_process_presence(&scan_error);"),
    restart.index("if (!sshd_restart_before_state_allows_stop(before_state))"),
    restart.index("if (sshd_restart_requires_stop(before_state))"),
    restart.index("stop_status = stop_sshd();"),
    restart.index("stopped_state = sshd_process_presence(&scan_error);"),
)
assert restart_order == tuple(sorted(restart_order))


# A stop operation must use the same executable-to-process mapping as
# inventory and the identity-bound lifecycle engine; shell killall success
# only proves a signal was sent, not that the daemon exited.
process_name_policy = body(POLICY, "sshd_process_name_for_binaries(")
assert 'return "dropbear";' in process_name_policy
assert 'return "sshd";' in process_name_policy
assert "return NULL;" in process_name_policy
presence = body(SERVICES, "sshd_process_presence(int *scan_error)")
stop = body(SERVICES, "int stop_sshd(void)")
assert "sshd_process_name_for_binaries(" in presence
assert "sshd_process_name_for_binaries(" in stop
assert 'check_if_file_exist("/usr/bin/dropbearmulti")' in stop
assert 'check_if_file_exist("/usr/sbin/sshd")' in stop
assert "service_names[0] = (char *)process_name;" in stop
assert "service_names[1] = NULL;" in stop
assert "kill_services(service_names, 3, 1)" in stop
assert 'eval("/usr/bin/sshd.sh", "stop")' not in stop
assert "killall" not in stop
assert "test_process_name_selection_matches_inventory_backend();" in TEST

print("SSH detached-child startup, tri-state restart preflight and firewall rollback: PASS")

