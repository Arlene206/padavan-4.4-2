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

script = script_path.read_text()
test = test_path.read_text()
makefile = makefile_path.read_text()
services = services_path.read_text()
rc_header = rc_header_path.read_text()
rc = rc_path.read_text()
workflow = workflow_path.read_text()


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit(f"DDNSTO lifecycle source contract: FAIL: {message}")


def function_body(source: str, signature: str) -> str:
    match = re.search(rf"(?m)^{re.escape(signature)}[ \t]*\{{(.*?)^[ \t]*\}}", source, re.S)
    require(match is not None, f"missing function {signature}")
    return match.group(1)


wait_body = function_body(script, "ddnsto_wait_stopped()")
kill_body = function_body(script, "kill_z()")
start_body = function_body(script, "start_zero()")
stop_body = function_body(script, "stop_zero()")
lock_path_body = function_body(script, "ddnsto_lifecycle_lock_path()")
lock_prepare_body = function_body(script, "ddnsto_lifecycle_lock_prepare()")
lock_execute_body = function_body(script, "ddnsto_lifecycle_lock_execute()")
locked_action_body = function_body(script, "ddnsto_lifecycle_locked_action()")
lock_run_body = function_body(script, "ddnsto_lifecycle_lock_run()")

require('if ! pidof ddnsto >/dev/null 2>&1; then' in wait_body,
        "wait loop must query live process identity rather than cached numeric PIDs")
require('while [ "$ddnsto_wait" -lt 5 ]; do' in wait_body and 'sleep 1' in wait_body,
        "stop wait must be bounded to five one-second polls")
require(kill_body.count('killall ddnsto') == 1 and kill_body.count('killall -9 ddnsto') == 1,
        "stop must send TERM then a name-re-enumerated SIGKILL only once")
require(kill_body.index('killall ddnsto') < kill_body.index('ddnsto_wait_stopped') <
        kill_body.index('killall -9 ddnsto'), "TERM grace period must precede force kill")
require(kill_body.rfind('ddnsto_wait_stopped') > kill_body.index('killall -9 ddnsto'),
        "force kill must have a bounded process-absence postcondition")
require('return 1' in kill_body, "failed stop postcondition must propagate failure")
require('if ! kill_z; then' in start_body and start_body.index('if ! kill_z; then') <
        start_body.index('"$DDNSTO_BIN"'), "start must fail closed before launching a replacement")
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
require("stop_zero || return $?" in locked_action_body and
        'nvram get ddnsto_enable' in locked_action_body and
        "start_zero" in locked_action_body,
        "start/restart must refresh the enable state while holding one lock")
require('kill_z' in stop_body, "stop action must propagate kill_z status")
require(not re.search(r'\bkill\s+-9\s+"?\$ddnsto_process', script),
        "stale pidof snapshot must never be force-signalled")
require('$(pidof ddnsto)' not in script, "no cached numeric pidof snapshot is permitted")
require('$(ROMFSINST) -p +x $(THISDIR)/ddnsto.sh /usr/bin/ddnsto.sh' in makefile,
        "ROMFS must install the exercised DDNSTO launcher")
require('eval("/usr/bin/ddnsto.sh","stop")' in services and
        'eval("/usr/bin/ddnsto.sh","start")' in services,
        "RC service wrappers must retain DDNSTO stop/start integration")
require('void restart_ddnsto(void)' in services and
        'eval("/usr/bin/ddnsto.sh","restart")' in services,
        "RC restart must use one serialized launcher operation")
restart_body = function_body(services, "void restart_ddnsto(void)")
require('eval("/usr/bin/ddnsto.sh","stop")' not in restart_body and
        'eval("/usr/bin/ddnsto.sh","start")' not in restart_body,
        "RC must not split DDNSTO restart across independently locked commands")
require('void restart_ddnsto(void);' in rc_header and 'restart_ddnsto();' in rc,
        "RC declaration / notification caller must remain wired")
for marker in ("private inode", "unsafe paths", "bounds lock contention",
               "inherited FD 9", "concurrent start", "TERM-END",
               "detached daemons do not retain FD 9", "serialized restart",
               "numeric PID signalling"):
    require(marker in test, f"regression omits lifecycle lock contract {marker}")
require('flock -n 8' in test and 'TERM-BEGIN' in test and
        'awk ' in test, "regression must exercise two independent launchers and stop/start ordering")

paths = [
    '.github/workflows/ddnsto-lifecycle.yml',
    'scripts/check_ddnsto_lifecycle.py',
    'trunk/user/ddnsto/ddnsto.sh',
    'trunk/user/ddnsto/tests/ddnsto_lifecycle.sh',
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

print("DDNSTO lifecycle stop/start and RC integration contracts: PASS")
