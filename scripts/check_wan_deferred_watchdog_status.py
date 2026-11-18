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
ordered = (
    'nvram_set_int_temp("deferred_wanup_t", deferred_up_time);',
    "status = start_watchdog();",
    "if (status != 0)",
    "cannot schedule deferred WAN reconnect because watchdog startup failed",
    "return status;",
    'logmessage(LOGNAME, "WAN up delay: %lds.", pause_in_seconds);',
    "return 0;",
)
positions = [deferred.index(token) for token in ordered]
require(positions == sorted(positions),
        "publish deferred intent, start watchdog, report and return failures before success")
require('nvram_set_int_temp("deferred_wanup_t", 0);' not in deferred,
        "retain deferred intent so a later watchdog recovery can retry")
print("Deferred WAN reconnect reports watchdog startup failure: PASS")
