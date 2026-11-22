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
priority_write = "priority_status = wan_nvram_int_status("
priority_key = '"modem_prio",'
priority_gate = "if (priority_status != 0)"
priority_log = "cannot publish modem priority before automatic reconnect"
priority_queue = 'queue_internet_lost_action("auto_wan_reconnect",'
write_pos = notification.index(priority_write)
key_pos = notification.index(priority_key, write_pos)
gate_pos = notification.index(priority_gate, key_pos)
log_pos = notification.index(priority_log, gate_pos)
queue_pos = notification.index(priority_queue, log_pos)
require(write_pos < key_pos < gate_pos < log_pos < queue_pos,
        "automatic USB-WAN reconnect must be gated on checked modem-priority publication")
require("nvram_set_int(\"modem_prio\"" not in notification,
        "modem reconnect priority must not use the unchecked direct NVRAM setter")
require("break;" in notification[gate_pos:queue_pos],
        "failed modem-priority publication must skip the automatic reconnect enqueue")

priority_helper = body(WAN, "wan_nvram_int_status(const char *param_name, int value)")
require("api_result = nvram_set_int(param_name, value);" in priority_helper
        and "saved_errno = errno;" in priority_helper
        and "rc_wan_nvram_write_status(api_result, saved_errno)" in priority_helper,
        "persistent modem-priority writes must preserve the original setter semantics and API errno")

reconnect = body(WAN, "try_wan_reconnect(int try_use_modem, long pause_in_seconds)")
require(
    any(line.strip().split() == ["int", "start_watchdog(void);"]
        for line in HEADER.splitlines()),
    "watchdog startup must have a status-returning public contract",
)
watchdog_start = body(WATCHDOG, "start_watchdog(void)")
watchdog_start_compact = "".join(watchdog_start.split())
require(
    "PROCESS_PRESENCE_START_ABORT" in watchdog_start
    and 'returnwatchdog_rollback_failed_start(launch_status,launch_errno,"launcher");'
    in watchdog_start_compact
    and 'returnwatchdog_rollback_failed_start(-1,ETIMEDOUT,"readinesstimeout");'
    in watchdog_start_compact,
    "watchdog inventory, launcher and readiness failures must remain visible",
)

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
pause_reader = "".join(body(WAN, "get_nvram_int_checked_status(const char *param_name, int *value)").split())
require("rc_wan_nvram_read_int_status(api_result,saved_errno,stored_value,0,value)" in pause_reader,
        "deferred reconnect delay must retain the legacy missing-key zero default")

pause = body(WAN, "auto_wan_reconnect_pause(void)")
pause_read = 'status = get_nvram_int_checked_status("di_recon_pause", &pause_seconds);'
pause_gate = "if (status != 0)"
pause_abort = "aborting automatic WAN reconnect because delay configuration is unavailable"
pause_return = "return status;"
pause_start = "return try_wan_reconnect(1, pause_seconds);"
pause_positions = (
    pause.index(pause_read),
    pause.index(pause_gate),
    pause.index(pause_abort),
    pause.index(pause_return, pause.index(pause_gate)),
    pause.index(pause_start),
)
require(pause_positions == tuple(sorted(pause_positions)),
        "automatic reconnect must abort before stop/start when its delay NVRAM read fails")
require('nvram_get_int("di_recon_pause")' not in pause,
        "automatic reconnect delay must not collapse NVRAM errors into immediate reconnect")

print("Deferred WAN reconnect reports publication and watchdog failures: PASS")
