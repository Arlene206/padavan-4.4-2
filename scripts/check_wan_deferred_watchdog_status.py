#!/usr/bin/env python3
"""Verify watchdog startup failures are propagated when WAN reconnect is deferred."""

from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
RC = ROOT / "trunk/user/rc/src"
WAN = (RC / "net_wan.c").read_text()
WATCHDOG = (RC / "watchdog.c").read_text()
HEADER = (RC / "rc.h").read_text()


def body(source, signature):
    start = source.index(signature)
    opening = source.index("{", start)
    depth = 0
    state = "code"
    quote = ""
    index = opening
    while index < len(source):
        char = source[index]
        nxt = source[index + 1] if index + 1 < len(source) else ""
        if state == "code":
            if char == "/" and nxt == "*":
                state = "block"
                index += 2
                continue
            if char == "/" and nxt == "/":
                state = "line"
                index += 2
                continue
            if char in ("'", '"'):
                state = "quote"
                quote = char
            elif char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return source[opening + 1:index]
        elif state == "quote":
            if char == chr(92):
                index += 2
                continue
            if char == quote:
                state = "code"
        elif state == "line" and char == chr(10):
            state = "code"
        elif state == "block" and char == "*" and nxt == "/":
            state = "code"
            index += 2
            continue
        index += 1
    raise AssertionError("unterminated function: " + signature)


def require(condition, message):
    if not condition:
        raise SystemExit("deferred WAN watchdog status contract: FAIL: " + message)


notification = body(WAN, "notify_on_internet_state_changed(int has_internet, long elapsed)")
priority_write = "priority_status = wan_nvram_int_temp_status("
priority_key = '"modem_prio",'
priority_gate = "if (priority_status != 0)"
priority_log = "cannot publish modem priority before automatic WAN reconnect"
priority_queue = 'queue_internet_lost_action("auto_wan_reconnect",'
write_pos = notification.index(priority_write)
key_pos = notification.index(priority_key, write_pos)
gate_pos = notification.index(priority_gate, key_pos)
log_pos = notification.index(priority_log, gate_pos)
queue_pos = notification.index(priority_queue, log_pos)
require(write_pos < key_pos < gate_pos < log_pos < queue_pos,
        "automatic USB-WAN reconnect must be gated on checked modem-priority publication")
require("nvram_set_int(\"modem_prio\"" not in notification,
        "modem reconnect priority must not use the unchecked persistent NVRAM setter")
require("break;" in notification[gate_pos:queue_pos],
        "failed modem-priority publication must skip the automatic reconnect enqueue")

priority_helper = body(WAN, "wan_nvram_int_temp_status(const char *param_name, int value)")
require("api_result = nvram_set_int_temp(param_name, value);" in priority_helper
        and "saved_errno = errno;" in priority_helper
        and "rc_wan_nvram_write_status(api_result, saved_errno)" in priority_helper,
        "modem-priority writes must preserve the NVRAM API status/errno")

reconnect = body(WAN, "try_wan_reconnect(int try_use_modem, long pause_in_seconds)")
require(
    any(line.strip().split() == ["int", "start_watchdog(void);"]
        for line in HEADER.splitlines()),
    "watchdog startup must have a status-returning public contract",
)
watchdog_start = body(WATCHDOG, "start_watchdog(void)")
require("return launch_status;" in watchdog_start
        and "PROCESS_PRESENCE_START_ABORT" in watchdog_start,
        "watchdog launcher and unknown-presence errors must remain visible")

deferred_start = reconnect.index("if (pause_in_seconds > 0)")
deferred = reconnect[deferred_start:]
schedule_call = "status = schedule_deferred_wan_reconnect_status("
watchdog_call = "status = start_watchdog();"
schedule_pos = deferred.index(schedule_call)
schedule_gate = deferred.index("if (status != 0)", schedule_pos)
schedule_error = deferred.index("cannot publish deferred WAN reconnect deadline", schedule_gate)
schedule_return = deferred.index("return status;", schedule_error)
watchdog_pos = deferred.index(watchdog_call, schedule_return)
watchdog_gate = deferred.index("if (status != 0)", watchdog_pos)
watchdog_error = deferred.index(
    "cannot schedule deferred WAN reconnect because watchdog startup failed",
    watchdog_gate,
)
watchdog_return = deferred.index("return status;", watchdog_error)
success_log = deferred.index(
    'logmessage(LOGNAME, "WAN up delay: %lds.", pause_in_seconds);',
    watchdog_return,
)
success_return = deferred.index("return 0;", success_log)
positions = (
    schedule_pos,
    schedule_gate,
    schedule_error,
    schedule_return,
    watchdog_pos,
    watchdog_gate,
    watchdog_error,
    watchdog_return,
    success_log,
    success_return,
)
require(positions == tuple(sorted(positions)),
        "checked deadline publication and watchdog errors must precede reconnect success")
require('nvram_set_int_temp("deferred_wanup_t"' not in WAN,
        "deferred reconnect must use checked NVRAM publication")
policy = (RC / "wan_deferred_reconnect_policy.h").read_text()
require("wan_deferred_reconnect_schedule" in policy and "INT_MAX" in policy,
        "deferred reconnect policy must enforce the watchdog integer deadline domain")
print("Deferred WAN reconnect reports publication and watchdog failures: PASS")
