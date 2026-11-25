#!/usr/bin/env python3
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
script_path = ROOT / "trunk/user/ddnsto/ddnsto.sh"
test_path = ROOT / "trunk/user/ddnsto/tests/ddnsto_lifecycle.sh"
makefile_path = ROOT / "trunk/user/ddnsto/Makefile"
services_path = ROOT / "trunk/user/rc/src/services.c"
rc_header_path = ROOT / "trunk/user/rc/src/rc.h"
rc_path = ROOT / "trunk/user/rc/src/rc.c"
workflow_path = ROOT / ".github/workflows/ddnsto-lifecycle.yml"
xy_workflow_path = ROOT / ".github/workflows/xy-c1-firmware-validation.yml"

script = script_path.read_text()
test = test_path.read_text()
makefile = makefile_path.read_text()
services = services_path.read_text()
rc_header = rc_header_path.read_text()
rc = rc_path.read_text()
workflow = workflow_path.read_text()
shared_makefile = (ROOT / "trunk/user/shared/Makefile").read_text()
xy_workflow = xy_workflow_path.read_text()


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit(f"DDNSTO lifecycle source contract: FAIL: {message}")


def function_body(source: str, signature: str) -> str:
    match = re.search(rf"(?m)^{re.escape(signature)}[ \t]*\{{(.*?)^[ \t]*\}}", source, re.S)
    require(match is not None, f"missing function {signature}")
    return match.group(1)


state_body = function_body(script, "ddnsto_process_state()")
wait_body = function_body(script, "ddnsto_wait_stopped()")
wait_started_body = function_body(script, "ddnsto_wait_started()")
kill_body = function_body(script, "kill_z()")
signal_body = function_body(script, "ddnsto_signal_validated_processes()")
absence_body = function_body(script, "ddnsto_confirm_absent()")
start_body = function_body(script, "start_zero()")
stop_body = function_body(script, "stop_zero()")
lock_path_body = function_body(script, "ddnsto_lifecycle_lock_path()")
lock_prepare_body = function_body(script, "ddnsto_lifecycle_lock_prepare()")
lock_execute_body = function_body(script, "ddnsto_lifecycle_lock_execute()")
locked_action_body = function_body(script, "ddnsto_lifecycle_locked_action()")
enable_state_body = function_body(script, "ddnsto_read_enable_state()")
lock_run_body = function_body(script, "ddnsto_lifecycle_lock_run()")

require('pidof ddnsto' in state_body and 'ddnsto_pidof_status' in state_body and
        'return 2' in state_body and 'return 1' in state_body and 'return 0' in state_body,
        "DDNSTO process inventory must distinguish RUNNING, confirmed ABSENT and UNKNOWN")
require('while [ "$ddnsto_wait" -lt 5 ]; do' in wait_body and
        'ddnsto_process_state' in wait_body and 'sleep 1' in wait_body and
        'return 2' in wait_body,
        "stop wait must use five bounded tri-state polls and preserve UNKNOWN")
require('DDNSTO_START_WAIT_ATTEMPTS=5' in script and
        'while [ "$ddnsto_start_wait" -lt "$DDNSTO_START_WAIT_ATTEMPTS" ]; do' in wait_started_body and
        'ddnsto_process_state' in wait_started_body and
        '1) ;;' in wait_started_body and '*) return 2 ;;' in wait_started_body and
        'if ! sleep 1; then' in wait_started_body and 'return 1' in wait_started_body,
        "startup must retry only confirmed ABSENT and fail closed on UNKNOWN/wait errors")
require('[ ! -x "$DDNSTO_BIN" ]' in start_body and
        'ddnsto_start_id=$(nvram get ddnsto_id)' in start_body and
        '[ -z "$ddnsto_start_id" ]' in start_body and
        'ddnsto_wait_started' in start_body and
        'ddnsto_start_wait_status' in start_body and
        start_body.index('[ ! -x "$DDNSTO_BIN" ]') < start_body.index('if ! kill_z; then') <
        start_body.index('"$DDNSTO_BIN" -u "$ddnsto_start_id"') <
        start_body.index('ddnsto_wait_started'),
        "start must preflight executable and identity before stopping, then verify RUNNING")
require("stat -L -c '%d:%i'" in state_body and
        'stat -c %u "/proc/$ddnsto_process_pid"' in state_body and
        'stat -L -c \'%d:%i\' "/proc/$ddnsto_process_pid/exe"' in state_body and
        '[ "$ddnsto_actual_exe" = "$ddnsto_expected_exe" ]' in state_body and
        '[ "$ddnsto_actual_uid" = "$ddnsto_expected_uid" ]' in state_body,
        "RUNNING must bind each candidate to the installed executable inode and service UID")
require('DDNSTO_PROCESS_LIFECYCLE_LIB' in absence_body and
        'pl_process_state ddnsto' in absence_body and
        '[ "$ddnsto_procfs_status" -eq 1 ]' in absence_body,
        "pidof status 1 must be confirmed ABSENT by the shared procfs scanner")
require('killall' not in script and
        'ddnsto_signal_validated_processes TERM' in kill_body and
        'ddnsto_signal_validated_processes KILL' in kill_body and
        kill_body.index('ddnsto_signal_validated_processes TERM') < kill_body.index('ddnsto_wait_stopped') < kill_body.index('ddnsto_signal_validated_processes KILL') < kill_body.rindex('ddnsto_wait_stopped') and
        'kill "-$ddnsto_signal" "$ddnsto_signal_pid"' in signal_body and
        'case " $ddnsto_process_pids " in' in signal_body,
        "TERM/KILL must use fresh exact-PID snapshots in bounded TERM-before-KILL order")
require('return 1' in kill_body and 'ddnsto_process_state' in kill_body and
        'ddnsto_wait_status' in kill_body and 'TERM后进程状态未知' in kill_body and
        'if ! kill_z; then' in start_body and
        start_body.index('if ! kill_z; then') < start_body.index('"$DDNSTO_BIN" -u "$ddnsto_start_id"'),
        "UNKNOWN must block TERM/KILL escalation and replacement daemon launch")
require('if ! kill_z; then' in start_body and start_body.index('if ! kill_z; then') <
        start_body.index('"$DDNSTO_BIN" -u "$ddnsto_start_id"'), "start must fail closed before launching a replacement")
require('9>&- &' in start_body and 'DDNSTO_BIN="/usr/bin/ddnsto"' in script,
        "detached daemon launch must use the fixed ROMFS binary and close the lifecycle lock FD")
require("return kill_z" in stop_body or "kill_z" in stop_body,
        "stop action must propagate kill_z status")

require("'/tmp/.ddnsto.lifecycle.lock'" in lock_path_body,
        "DDNSTO must use one stable boot-lifetime lock path")
for marker in ("stat -c '%u %a'", "stat -c '%u %a %h'", "set -C",
               '[ -L "$ddnsto_lock_file" ]', '[ "$2" != 600 ]', '[ "$3" != 1 ]'):
    require(marker in lock_prepare_body,
            f"DDNSTO lock validation omits {marker}")
for marker in ("flock -n 9", "ddnsto_wait_seconds", "exec 9>&-",
               "生命周期锁FD 9已被占用", "等待生命周期锁超时"):
    require(marker in lock_execute_body,
            f"DDNSTO lock execution omits {marker}")
require(lock_execute_body.index("flock -n 9") <
        lock_execute_body.index('"$@"') <
        lock_execute_body.rindex("exec 9>&-"),
        "DDNSTO lock must enclose the complete operation and release after it")
require("ddnsto_lifecycle_lock_prepare" in lock_run_body and
        "command -v flock" in lock_run_body and
        "ddnsto_lifecycle_lock_execute" in lock_run_body and
        "start|stop|restart)" in lock_run_body,
        "all DDNSTO external lifecycle actions must fail closed behind flock")
require('if ddnsto_enable_value=$(nvram get ddnsto_enable)' in enable_state_body and
        'ddnsto_enable_status=$?' in enable_state_body and
        'return "$ddnsto_enable_status"' in enable_state_body,
        "enable-state NVRAM failure must preserve the original command status")
require('case "$ddnsto_enable_value" in' in enable_state_body and
        "''|0|1)" in enable_state_body and
        "DDNSTO enable selector is invalid; refusing to change process state" in enable_state_body and
        "return 22" in enable_state_body,
        "DDNSTO start/restart must reject malformed non-empty enable selectors before process changes")
require("DDNSTO invalid enable selectors cannot stop or launch a daemon: PASS" in test,
        "DDNSTO regression must cover malformed enable selectors for both start and restart")
require(locked_action_body.count("ddnsto_read_enable_state || return $?") == 2,
        "start and restart must each use one checked enable-state preflight")
start_action_body = locked_action_body.split("start)", 1)[1].split(";;", 1)[0]
restart_action_body = locked_action_body.split("restart)", 1)[1].split("*)", 1)[0]
require(start_action_body.index("ddnsto_read_enable_state || return $?") <
        start_action_body.index("start_zero"),
        "start must complete enable-state preflight before daemon launch")
require(restart_action_body.index("ddnsto_read_enable_state || return $?") <
        restart_action_body.index("stop_zero || return $?") <
        restart_action_body.index("start_zero"),
        "restart must read enable state before stopping or launching a daemon")
require('nvram get ddnsto_enable' not in locked_action_body,
        "unchecked enable-state command substitutions must not remain in the dispatcher")
require('kill_z' in stop_body, "stop action must propagate kill_z status")
require(not re.search(r'\bkill\s+-9\s+"?\$ddnsto_process', script),
        "stale pidof snapshot must never be force-signalled")
require('$(pidof ddnsto)' not in script, "no cached numeric pidof snapshot is permitted")
require('$(ROMFSINST) -p +x $(THISDIR)/ddnsto.sh /usr/bin/ddnsto.sh' in makefile,
        "ROMFS must install the exercised DDNSTO launcher")
require('$(INSTALL_BIN) $(CURDIR)/scripts/process_lifecycle.sh $(ROMFSDIR)/usr/lib/process_lifecycle.sh' in shared_makefile,
        "shared package must install the procfs absence helper used by DDNSTO")
require('eval("/usr/bin/ddnsto.sh","stop")' in services and
        'eval("/usr/bin/ddnsto.sh","start")' in services,
        "RC service wrappers must retain DDNSTO stop/start integration")
require('int restart_ddnsto(void)' in services and
        'eval("/usr/bin/ddnsto.sh","restart")' in services,
        "RC restart must use one serialized launcher operation")
restart_body = function_body(services, "int restart_ddnsto(void)")
require('return eval("/usr/bin/ddnsto.sh","restart");' in restart_body and
        'eval("/usr/bin/ddnsto.sh","stop")' not in restart_body and
        'eval("/usr/bin/ddnsto.sh","start")' not in restart_body,
        "RC must not split DDNSTO restart across independently locked commands")
require('int restart_ddnsto(void);' in rc_header and 'rc_notify_merge_action_result(action_result, restart_ddnsto())' in rc,
        "RC declaration / notification caller must remain wired")
for marker in ("spoofed executable", "foreign UID", "unreadable executable metadata",
               "pidof absence is verified against procfs",
               "UNKNOWN process inventory", "UNKNOWN after TERM", "sleep failure",
               "missing DDNSTO executable", "NVRAM identity read failure",
               "NVRAM enable-state read failure", "after an NVRAM read failure",
               "empty DDNSTO identity", "immediate-exit daemon",
               "exactly five bounded polls", "UNKNOWN post-launch process inventory",
               "failed startup wait primitive", "delayed but confirmed daemon",
               "private inode", "unsafe paths", "bounds lock contention",
               "inherited FD 9", "concurrent start", "TERM-END",
               "detached daemons do not retain FD 9", "serialized restart",
               "numeric PID"):
    require(marker in test, f"regression omits lifecycle lock contract {marker}")
require('flock -n 8' in test and 'TERM-BEGIN' in test and
        'awk ' in test, "regression must exercise two independent launchers and stop/start ordering")
require('echo $$ > "$TEST_DAEMON_PID"' in test and
        'if [ -s "$TEST_DAEMON_PID" ]; then' in test,
        "daemon fixtures must publish their actual numeric PID to process inventory")
require('start_after_stop_attempt' in test and
        '[ "$(grep -Fxc START "$TEST_OPS" || :)" -lt 2 ]' in test,
        "concurrent start ordering must wait for its asynchronous launch marker")

paths = [
    '.github/workflows/ddnsto-lifecycle.yml',
    'scripts/check_ddnsto_lifecycle.py',
    'trunk/user/ddnsto/ddnsto.sh',
    'trunk/user/ddnsto/tests/ddnsto_lifecycle.sh',
    'trunk/user/shared/scripts/process_lifecycle.sh',
    'trunk/user/shared/Makefile',
    'trunk/user/ddnsto/Makefile',
    'trunk/user/rc/src/services.c',
    'trunk/user/rc/src/rc.h',
    'trunk/user/rc/src/rc.c',
]
push = workflow.split('  push:', 1)[1].split('  pull_request:', 1)[0]
pull = workflow.split('  pull_request:', 1)[1].split('permissions:', 1)[0]
for path in paths:
    token = f'      - "{path}"'
    require(push.count(token) == 1 and pull.count(token) == 1,
            f"{path} must appear exactly once in push and pull_request path filters")

xy_push = xy_workflow.split('  push:', 1)[1].split('  workflow_dispatch:', 1)[0]
require(xy_push.count('      - "trunk/user/ddnsto/ddnsto.sh"') == 1 and
        xy_push.count('      - "trunk/user/shared/scripts/process_lifecycle.sh"') == 1 and
        xy_push.count('      - "trunk/user/shared/Makefile"') == 1,
        "XY-C1 firmware workflow must trigger for DDNSTO and shared process packaging changes")

print("DDNSTO lifecycle, RC integration and XY-C1 packaging contracts: PASS")
