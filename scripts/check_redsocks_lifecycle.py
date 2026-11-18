#!/usr/bin/env python3
"""Source and regression contracts for Redsocks process lifecycle."""
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / "trunk/user/redsocks/redsocks.sh"
TEST = ROOT / "scripts/tests/redsocks_lifecycle.sh"
WORKFLOW = ROOT / ".github/workflows/redsocks-lifecycle.yml"
source = SOURCE.read_text()
test = TEST.read_text()
workflow = WORKFLOW.read_text()

def require(condition, message):
    if not condition:
        raise SystemExit(f"Redsocks lifecycle contract: FAIL: {message}")

def body(signature):
    match = re.search(rf"(?m)^{re.escape(signature)}\s*\{{(.*?)^\}}", source, re.S)
    require(match is not None, f"missing {signature}")
    return match.group(1)

state = body("redsocks_process_state()")
wait = body("redsocks_wait_stopped()")
wait_start = body("redsocks_wait_started()")
start_verify = body("redsocks_start_and_verify()")
start_offset = source.index("func_start(){")
start_end_offset = source.index("func_clean(){", start_offset)
start = source[start_offset:start_end_offset]
stop = body("func_stop()")
require('pidof "$BINARY_NAME"' in state and 'redsocks_pid_status' in state and
        'return 2' in state and 'return 1' in state and 'return 0' in state,
        "process inventory must distinguish RUNNING, confirmed ABSENT and UNKNOWN")
require('while [ "$redsocks_wait" -lt 5 ]; do' in wait and
        'redsocks_process_state' in wait and 'sleep 1 || return 2' in wait,
        "stop wait must use bounded tri-state polling and preserve sleep errors")
require('while [ "$redsocks_wait" -lt 5 ]; do' in wait_start and
        'redsocks_process_state' in wait_start and 'sleep 1 || return 2' in wait_start and
        'return 1' in wait_start,
        "detached startup must use bounded tri-state RUNNING confirmation")
require('func_redsocks' in start_verify and 'redsocks_launch_status' in start_verify and
        'return "$redsocks_launch_status"' in start_verify and
        'redsocks_wait_started' in start_verify and
        'startup timed out' in start_verify and 'startup state is UNKNOWN' in start_verify,
        "startup must preserve launcher errors and fail closed on timeout/UNKNOWN")
require(start.count('redsocks_start_and_verify') == 2 and
        'func_redsocks' not in start and
        start.count('logger -t $BINARY_NAME "STARTED."') == 1,
        "both configuration paths must verify detached readiness before STARTED")
require('killall -q "$BINARY_NAME"' in stop and
        'killall -q -9 "$BINARY_NAME"' in stop and
        stop.index('killall -q "$BINARY_NAME"') < stop.index('redsocks_wait_stopped') <
        stop.index('killall -q -9 "$BINARY_NAME"') < stop.rindex('redsocks_wait_stopped'),
        "TERM, bounded wait, KILL and final absence verification must be ordered")
require('TERM postcondition unknown; refusing KILL escalation' in stop and
        'pre-KILL process inventory unknown; refusing signal' in stop and
        'return 1' in stop,
        "UNKNOWN must block destructive escalation and propagate stop failure")
start_offset = source.index("func_start(){")
start_gate_offset = source.index("if ! func_stop; then", start_offset)
start_launch_offset = source.index("redsocks_start_and_verify", start_offset)
require('redsocks_process_state' in start and
        'if ! func_stop; then' in start and
        start_gate_offset < start_launch_offset,
        "start must refuse duplicate launch when existing daemon stop fails")
require('func_stop && func_start' in source,
        "restart must gate replacement start on successful stop")
require('if [ "${RED_SOCKS_TEST_MODE:-0}" != 1 ]; then' in source and
        'case "$ARG1" in' in source,
        "test mode must skip only the CLI dispatcher while preserving production function definitions")
for marker in ("UNKNOWN inventory", "UNKNOWN after TERM", "sleep failure",
               "force stop signal sequence incorrect", "stuck daemon was reported stopped",
               "replacement daemon launched after failed stop",
               "delayed detached child", "ABSENT startup timeout",
               "UNKNOWN startup inventory", "startup sleep failure",
               "launcher status was changed", "generated-config detached startup"):
    require(marker in test, f"missing regression marker {marker}")
paths = (
    ".github/workflows/redsocks-lifecycle.yml",
    "scripts/check_redsocks_lifecycle.py",
    "scripts/tests/redsocks_lifecycle.sh",
    "trunk/user/redsocks/redsocks.sh",
)
for block in (workflow.split("  push:",1)[1].split("  pull_request:",1)[0],
              workflow.split("  pull_request:",1)[1].split("permissions:",1)[0]):
    for path in paths:
        require(block.count(f'      - "{path}"') == 1, f"path filter missing or duplicated: {path}")
print("Redsocks lifecycle source and restart contracts: PASS")
