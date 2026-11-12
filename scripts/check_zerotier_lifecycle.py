#!/usr/bin/env python3
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[1]
script_path = ROOT / "trunk/user/zerotier/zerotier.sh"
test_path = ROOT / "trunk/user/zerotier/tests/zerotier_lifecycle.sh"
makefile_path = ROOT / "trunk/user/zerotier/Makefile"
services_path = ROOT / "trunk/user/rc/src/services.c"
rc_header_path = ROOT / "trunk/user/rc/src/rc.h"
rc_path = ROOT / "trunk/user/rc/src/rc.c"
workflow_path = ROOT / ".github/workflows/zerotier-lifecycle.yml"

script = script_path.read_text()
test = test_path.read_text()
makefile = makefile_path.read_text()
services = services_path.read_text()
rc_header = rc_header_path.read_text()
rc = rc_path.read_text()
workflow = workflow_path.read_text()


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit(f"ZeroTier lifecycle source contract: FAIL: {message}")


def function_body(source: str, signature: str) -> str:
    match = re.search(rf"(?m)^{re.escape(signature)}[ \t]*\{{(.*?)^[ \t]*\}}", source, re.S)
    require(match is not None, f"missing function {signature}")
    return match.group(1)


wait_body = function_body(script, "zerotier_wait_stopped()")
kill_body = function_body(script, "kill_z()")
start_body = function_body(script, "start_zero()")
stop_body = function_body(script, "stop_zero()")

require('while [ "$zerotier_wait" -lt 5 ]; do' in wait_body and
        'pidof zerotier-one' in wait_body and 'sleep 1' in wait_body,
        "daemon absence must be checked through five bounded one-second live-PID polls")
require(kill_body.count('killall zerotier-one') == 1 and
        kill_body.count('killall -9 zerotier-one') == 1,
        "stop must send one TERM and at most one process-name SIGKILL")
require(kill_body.index('killall zerotier-one') < kill_body.index('zerotier_wait_stopped') <
        kill_body.index('killall -9 zerotier-one') <
        kill_body.rindex('zerotier_wait_stopped'),
        "TERM grace period and force-stop absence postcondition are out of order")
require('return 1' not in kill_body or 'zerotier_wait_stopped' in kill_body,
        "stop failure must propagate the bounded absence result")
require('if ! kill_z; then' in start_body and
        start_body.index('if ! kill_z; then') < start_body.index("start_instance 'zerotier'"),
        "start must not launch a new daemon unless the previous daemon is absent")
require('if ! kill_z; then' in stop_body and
        stop_body.index('if ! kill_z; then') < stop_body.index('del_rules') <
        stop_body.index('zero_route "del"') < stop_body.index('rm -rf "$config_path"'),
        "failed stop must preserve firewall/routing state and persistent identity/config")
require(not re.search(r'\bkill\s+-9\s+"?\$zerotier_process', script),
        "stale numeric PID must never be force-signalled")
require('$(pidof zerotier-one)' not in script,
        "numeric pidof snapshots must not be cached")

require('$(ROMFSINST) -p +x $(THISDIR)/zerotier.sh /usr/bin/zerotier.sh' in makefile,
        "ROMFS must install the exercised ZeroTier launcher")
require('eval("/usr/bin/zerotier.sh","stop")' in services and
        'eval("/usr/bin/zerotier.sh","start")' in services,
        "RC service wrappers must retain ZeroTier stop/start integration")
restart = function_body(services, "void restart_zerotier(void)")
require('stop_zerotier();' in restart and 'start_zerotier();' in restart,
        "RC ZeroTier restart caller graph changed unexpectedly")
require('void restart_zerotier(void);' in rc_header and 'restart_zerotier();' in rc,
        "RC declaration and notification caller must remain wired")

for token in ['graceful', '-9-exits', 'stuck', 'start_zero', 'stop_zero',
              'identity.secret', 'numeric-kill.log', 'five one-second polls']:
    require(token in test, f"regression coverage missing {token}")

paths = [
    '.github/workflows/zerotier-lifecycle.yml',
    'scripts/check_zerotier_lifecycle.py',
    'trunk/user/zerotier/zerotier.sh',
    'trunk/user/zerotier/tests/zerotier_lifecycle.sh',
    'trunk/user/zerotier/Makefile',
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

print("ZeroTier bounded lifecycle, persistent-config and RC integration contracts: PASS")
