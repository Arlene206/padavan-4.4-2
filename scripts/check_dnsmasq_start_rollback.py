#!/usr/bin/env python3
"""Verify DNSMasq failed-start cleanup remains serialized and fail-closed."""

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RC = ROOT / "trunk/user/rc/src"
SERVICES = (RC / "services_ex.c").read_text()
POLICY = (RC / "dnsmasq_restart_policy.h").read_text()
TEST = (RC / "tests/dnsmasq_restart_policy.c").read_text()
RC_H = (RC / "rc.h").read_text()


def body(source: str, signature: str) -> str:
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    state = "code"
    position = opening
    while position < len(source):
        ch = source[position]
        nxt = source[position + 1] if position + 1 < len(source) else ""
        if state == "code":
            if ch == '"':
                state = "string"
            elif ch == "'":
                state = "char"
            elif ch == "/" and nxt == "*":
                state = "block"
                position += 1
            elif ch == "/" and nxt == "/":
                state = "line"
                position += 1
            elif ch == "{":
                depth += 1
            elif ch == "}":
                depth -= 1
                if depth == 0:
                    return source[opening + 1:position]
        elif state in ("string", "char"):
            if ch == "\\":
                position += 1
            elif state == "string" and ch == '"':
                state = "code"
            elif state == "char" and ch == "'":
                state = "code"
        elif state == "block" and ch == "*" and nxt == "/":
            state = "code"
            position += 1
        elif state == "line" and ch == "\n":
            state = "code"
        position += 1
    raise AssertionError("unterminated function: " + signature)


def compact(source: str) -> str:
    return " ".join(source.split())


rollback = compact(body(
    SERVICES,
    "static int\ndnsmasq_rollback_failed_start(int start_status, const char *operation)",
))
assert "start_errno = start_status > 0 ? start_status :" in rollback
assert "rollback_status = stop_dns_dhcpd_locked()" in rollback
assert "presence = dnsmasq_process_presence(&scan_error)" in rollback
assert "dnsmasq_failed_start_may_leave_daemon(presence)" in rollback
assert "errno = start_errno;" in rollback
assert rollback.index("stop_dns_dhcpd_locked()") < rollback.index(
    "dnsmasq_process_presence(&scan_error)"
) < rollback.index("dnsmasq_failed_start_may_leave_daemon")
assert rollback.index("dnsmasq_failed_start_may_leave_daemon") < rollback.index(
    "errno = start_errno;"
)

start = compact(body(SERVICES, "int\nstart_dns_dhcpd_locked(int is_ap_mode)"))
assert start.index("errno = 0;") < start.index('eval("/usr/sbin/dnsmasq")')
assert "status = dnsmasq_confirm_started(launch_status)" in start
assert "status = dnsmasq_rollback_failed_start(status, \"start\")" in start
assert start.index("dnsmasq_confirm_started(launch_status)") < start.index(
    "dnsmasq_rollback_failed_start(status, \"start\")"
)

restart = compact(body(SERVICES, "static int\nrestart_dhcpd_locked(void)"))
assert restart.index("stop_dns_dhcpd_locked()") < restart.index(
    'eval("/usr/sbin/dnsmasq")'
)
assert restart.index("errno = 0;") < restart.index('eval("/usr/sbin/dnsmasq")')
assert "start_status = dnsmasq_confirm_started(launch_status)" in restart
assert "start_status = dnsmasq_rollback_failed_start(start_status, \"restart\")" in restart
assert restart.index("dnsmasq_confirm_started(launch_status)") < restart.index(
    "dnsmasq_rollback_failed_start(start_status, \"restart\")"
)

for public, internal, operation in (
    ("start_dns_dhcpd(int is_ap_mode)", "start_dns_dhcpd_locked(is_ap_mode)", "start"),
    ("restart_dhcpd(void)", "restart_dhcpd_locked()", "restart"),
):
    entry = body(SERVICES, "int\n" + public)
    assert entry.index("dnsmasq_lifecycle_lock_acquire()") < entry.index(internal)
    assert entry.index(internal) < entry.index("dnsmasq_lifecycle_lock_finish(")
    assert f'"{operation}"' in entry

assert "dnsmasq_failed_start_may_leave_daemon(enum process_presence state)" in POLICY
assert "return state != PROCESS_PRESENCE_ABSENT;" in POLICY
assert "dnsmasq_failed_start_may_leave_daemon(PROCESS_PRESENCE_UNKNOWN)" in TEST
assert "dnsmasq_failed_start_may_leave_daemon((enum process_presence)7)" in TEST
assert "int start_dns_dhcpd_locked(int is_ap_mode);" in RC_H
assert "int stop_dns_dhcpd_locked(void);" in RC_H

print("DNSMasq failed-start rollback and lifecycle-lock contract: PASS")
