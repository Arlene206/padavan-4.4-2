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
