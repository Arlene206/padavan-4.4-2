#!/usr/bin/env python3
"""Source and call-path contract for watchdog NTP single-instance refresh."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def read(relative):
    return (ROOT / relative).read_text()


def function_body(source, signature):
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    pos = brace
    state = "code"
    while pos < len(source):
        ch = source[pos]
        next_ch = source[pos + 1] if pos + 1 < len(source) else ""
        if state == "code":
            if ch == '"':
                state = "string"
            elif ch == "'":
                state = "char"
            elif ch == "/" and next_ch == "*":
                state = "block_comment"
                pos += 1
            elif ch == "/" and next_ch == "/":
                state = "line_comment"
                pos += 1
            elif ch == "{":
                depth += 1
            elif ch == "}":
                depth -= 1
                if depth == 0:
                    return source[brace + 1:pos]
        elif state == "string" or state == "char":
            if ch == "\\":
                pos += 1
            elif state == "string" and ch == '"':
                state = "code"
            elif state == "char" and ch == "'":
                state = "code"
        elif state == "block_comment":
            if ch == "*" and next_ch == "/":
                state = "code"
                pos += 1
        elif state == "line_comment" and ch == "\n":
            state = "code"
        pos += 1
    raise AssertionError("unterminated function: " + signature)


watchdog = read("trunk/user/rc/src/watchdog.c")
policy = read("trunk/user/rc/src/watchdog_ntp_policy.h")
test = read("trunk/user/rc/src/tests/watchdog_ntp_policy.c")
common = read("trunk/user/rc/src/common_ex.c")
lifecycle = read("trunk/user/rc/src/service_lifecycle_policy.h")
lifecycle_test = read("trunk/user/rc/src/tests/service_lifecycle_policy.c")
workflow = read(".github/workflows/watchdog-signal-stability.yml")

assert '#include "watchdog_ntp_policy.h"' in watchdog
assert "return previous_stop_status == 0;" in policy
for token in (
    "watchdog_ntp_refresh_should_launch(0)",
    "watchdog_ntp_refresh_should_launch(-1)",
    "watchdog_ntp_refresh_should_launch(1)",
    "watchdog_ntp_refresh_should_launch(EBUSY)",
    "watchdog_ntp_refresh_should_launch(EIO)",
    "watchdog NTP stop-before-start policy: PASS",
):
    assert token in test, token

refresh = function_body(watchdog, "refresh_ntp(void)")
stop_call = refresh.index("stop_status = kill_services(svcs, 3, 1);")
gate = refresh.index("if (!watchdog_ntp_refresh_should_launch(stop_status))")
failure_body = function_body(
    refresh, "if (!watchdog_ntp_refresh_should_launch(stop_status))"
)
server_read = refresh.index('ntp_addr[0] = nvram_safe_get("ntp_server0");')
server_advance = refresh.index("ntpc_server_idx = (ntpc_server_idx + 1) % 2;")
launch = refresh.index('result = eval("/usr/sbin/ntpd"')
assert stop_call < gate < server_read < server_advance < launch
assert "Previous ntpd instance did not stop" in failure_body
assert "return stop_status;" in failure_body
assert "eval(" not in failure_body
assert refresh.count("kill_services(svcs, 3, 1)") == 1

stop = function_body(
    common, "kill_services(char* svc_name[], int wtimeout, int forcekill)"
)
assert "result = service_lifecycle_stop(svc_name, wtimeout, forcekill, &ops, NULL);" in stop
assert "return result;" in stop
assert "SERVICE_LIFECYCLE_FORCE_WAIT_SECONDS 3" in lifecycle
assert "service_lifecycle_wait_for_exit" in lifecycle
assert "if (!forcekill || result != SERVICE_LIFECYCLE_TIMEOUT)" in lifecycle
assert "test_nonforced_timeout_blocks_followup" in lifecycle_test
assert "service_lifecycle_stop(names, 1, 0, &fake_ops, &context) ==" in lifecycle_test
assert "SERVICE_LIFECYCLE_TIMEOUT" in lifecycle_test
assert "SERVICE_LIFECYCLE_UNKNOWN" in lifecycle_test
assert "test_unknown_before_kill_blocks_kill" in lifecycle_test

paths = (
    ".github/workflows/watchdog-signal-stability.yml",
    "trunk/user/rc/src/watchdog.c",
    "trunk/user/rc/src/watchdog_signal_policy.h",
    "trunk/user/rc/src/tests/watchdog_signal_policy.c",
    "trunk/user/rc/src/watchdog_ntp_policy.h",
    "trunk/user/rc/src/tests/watchdog_ntp_policy.c",
    "trunk/user/rc/src/common_ex.c",
    "trunk/user/rc/src/service_lifecycle_policy.h",
    "trunk/user/rc/src/tests/service_lifecycle_policy.c",
    "trunk/user/rc/src/rc.c",
    "trunk/user/rc/src/rc.h",
    "trunk/user/rc/src/services.c",
    "trunk/user/rc/src/net_wifi.c",
    "trunk/user/rc/src/Makefile",
    "trunk/user/rc/Makefile",
    "scripts/check_watchdog_ntp_lifecycle.py",
)
filters = workflow.split("jobs:", 1)[0]
push_filters, pr_filters = filters.split("  pull_request:", 1)
for path in paths:
    quoted = '"' + path + '"'
    assert push_filters.count(quoted) == 1, "push path count: " + path
    assert pr_filters.count(quoted) == 1, "pull_request path count: " + path

print("Watchdog NTP stop-before-start and lifecycle call-path contracts: PASS")


# NTP step completion and persistent readiness publication must fail closed.
from pathlib import Path as _WatchdogPath
_ntp_root = _WatchdogPath(__file__).resolve().parents[1]
_ntp_rc = _ntp_root / "trunk/user/rc/src"
_ntp_watchdog = (_ntp_rc / "watchdog.c").read_text()
_ntp_dispatcher = (_ntp_rc / "rc.c").read_text()
_ntp_router_policy = (_ntp_rc / "init_router_result_policy.h").read_text()
_ntp_router_test = (_ntp_rc / "tests/init_router_result_policy.c").read_text()
_ntp_policy = (_ntp_rc / "watchdog_ntp_policy.h").read_text()
_ntp_test = (_ntp_rc / "tests/watchdog_ntp_policy.c").read_text()
assert "watchdog_ntp_step_has_offset" in _ntp_policy
assert "watchdog_ntp_ready_retry_required" in _ntp_policy
assert "test_ntp_step_requires_nonempty_offset" in _ntp_test
assert "test_ntp_ready_publication_retries_only_after_a_real_step" in _ntp_test
assert _ntp_watchdog.index("if (!watchdog_ntp_step_has_offset(offset))") < _ntp_watchdog.index('counter_status = nvram_set_int_temp("ntpc_counter"')
assert _ntp_watchdog.index('counter_status = nvram_set_int_temp("ntpc_counter"') < _ntp_watchdog.index('ready_status = nvram_set_int("ntp_ready", 1)')
assert "return ready_status;" in _ntp_watchdog
assert "ntpc_ready_handler();" in _ntp_watchdog
assert _ntp_watchdog.rindex("ntpc_ready_handler();") < _ntp_watchdog.rindex("svc_timecheck();")
assert "ntp_ready_publish_error != status" in _ntp_watchdog
assert "NTP readiness persistence recovered" in _ntp_watchdog

# Boot must invalidate the previous boot's persistent NTP-ready flag and
# temporary NTP-step counter before watchdog or user autostart can trust time.
_ntp_init_router = function_body(_ntp_dispatcher, "init_router(void)")
_counter_reset = _ntp_init_router.index('ntpc_counter_reset_status = nvram_set_temp("ntpc_counter", "0000000000");')
_ready_reset = _ntp_init_router.index('ntp_ready_reset_status = nvram_set_int("ntp_ready", 0);')
_reset_gate = _ntp_init_router.index("if (!init_router_ntp_state_reset_succeeded(")
_ready_section = _ntp_init_router.index("// system ready")
_autostart = _ntp_init_router.index('system("/usr/bin/copyscripts.sh")')
assert _counter_reset < _ready_reset < _reset_gate < _ready_section < _autostart
assert "return INIT_ROUTER_RESULT_FATAL;" in _ntp_init_router[_reset_gate:_ready_section]
assert "init_router_ntp_state_reset_succeeded" in _ntp_router_policy
assert "init_router_ntp_state_reset_succeeded" in _ntp_router_test
assert _ntp_dispatcher.count('nvram_set_int("ntp_ready", 0)') == 1
assert "#endif /* PADAVAN_INIT_ROUTER_RESULT_POLICY_H */" in _ntp_router_policy

