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
require('if ! kill_z; then' in start_body and start_body.index('kill_z || {') <
        start_body.index('/usr/bin/ddnsto'), "start must fail closed before launching a replacement")
require('kill_z' in stop_body, "stop action must propagate kill_z status")
require(not re.search(r'\bkill\s+-9\s+"?\$ddnsto_process', script),
        "stale pidof snapshot must never be force-signalled")
require('$(pidof ddnsto)' not in script, "no cached numeric pidof snapshot is permitted")
require('$(ROMFSINST) -p +x $(THISDIR)/ddnsto.sh /usr/bin/ddnsto.sh' in makefile,
        "ROMFS must install the exercised DDNSTO launcher")
require('eval("/usr/bin/ddnsto.sh","stop")' in services and
        'eval("/usr/bin/ddnsto.sh","start")' in services,
        "RC service wrappers must retain DDNSTO stop/start integration")
require('void restart_ddnsto(void)' in services and 'stop_ddnsto();' in services and
        'start_ddnsto();' in services, "restart caller graph changed unexpectedly")
require('void restart_ddnsto(void);' in rc_header and 'restart_ddnsto();' in rc,
        "RC declaration / notification caller must remain wired")
require('TERM' in test and 'force-exits' in test and 'stuck' in test and
        'start' in test and 'numeric-kill.log' in test,
        "regression must cover graceful stop, escalation, failure and restart gate")

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
