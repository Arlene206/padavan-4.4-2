#!/usr/bin/env python3
from pathlib import Path


ROOT = Path(__file__).resolve().parents[1]


def function_body(source: str, signature: str) -> str:
    start = source.index(signature)
    brace = source.index("{", start)
    depth = 0
    for index in range(brace, len(source)):
        if source[index] == "{":
            depth += 1
        elif source[index] == "}":
            depth -= 1
            if depth == 0:
                return source[brace + 1:index]
    raise AssertionError(f"unterminated function: {signature}")


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


source = (ROOT / "trunk/user/frp/frp.sh").read_text()
test = (ROOT / "trunk/user/frp/tests/frp_lifecycle.sh").read_text()
workflow = (ROOT / ".github/workflows/frp-lifecycle.yml").read_text()

check = function_body(source, "check_frp ()")
require(check.count("frp_start") == 1,
        "check_frp must dispatch no more than one restart per watchdog pass")
require(check.count("need_start=1") == 2,
        "check_frp must aggregate the frpc and frps missing states")
require(check.index('if [ "$frpc_enable" = "1" ]') <
        check.index('if [ "$frps_enable" = "1" ]') <
        check.index('if [ "$need_start" = "1" ]') <
        check.index("frp_start"),
        "the two service states must be checked before a single restart")
require('if [ "$result_net" = "1" ]' in check,
        "FRP restart must remain gated on successful WAN reachability")

cron = function_body(source, "frp_manage_watchdog_cron()")
for marker in ("add|remove", "A-Za-z0-9_.-", "FRP_CRONTAB_DIR", "FRP_CRON_ROOT",
               "mktemp", "umask 077", "awk -v managed=", "chmod 0600", "mv -f"):
    require(marker in cron, f"managed FRP crontab transaction omits {marker}")
require("sed -i '/frp/d'" not in source and "cat >> /etc/storage/cron/crontabs/" not in source,
        "FRP must not delete arbitrary cron entries using substring matching")
require('FRP_WATCHDOG_CRON=' in source and
        'awk -v managed="$FRP_WATCHDOG_CRON"' in cron,
        "FRP watchdog cleanup must match only its exact managed line")
start = function_body(source, "frp_start ()")
require(start.index("frp_run_script") < start.index("frp_manage_watchdog_cron add"),
        "do not publish a watchdog when the configured FRP start script fails")
close = function_body(source, "frp_close ()")
require("frp_manage_watchdog_cron remove" in close,
        "disabling both services must remove the exact managed watchdog")
cron_test = (ROOT / "trunk/user/frp/tests/frp_crontab.sh").read_text()
for marker in ("frp_start", "frp_close", "operator", "path-traversal",
               "symlink crontab", "atomic rename failure", "mode 0600",
               "failed start script", "overlong username", "removing a missing watchdog"):
    require(marker in cron_test, f"FRP managed crontab regression omits {marker}")

network = function_body(source, "check_net()")
require("if /bin/ping " in network and "== " not in network,
        "WAN probe must use a POSIX if/status check")
require(network.index('logger -t "frp"') < network.index("return 2"),
        "WAN failure log must not be unreachable after return")

for marker in (
    'expect_start_count "both enabled services missing" 1',
    'expect_start_count "client missing" 1',
    'expect_start_count "server missing" 1',
    'expect_start_count "both services alive" 0',
    'expect_start_count "all services disabled" 0',
    'expect_start_count "WAN unavailable" 0',
    "Deliberately leave both PIDs absent",
):
    require(marker in test, f"missing asynchronous lifecycle regression: {marker}")

paths = (
    ".github/workflows/frp-lifecycle.yml",
    "trunk/user/frp/frp.sh",
    "trunk/user/frp/tests/frp_lifecycle.sh",
    "trunk/user/frp/tests/frp_crontab.sh",
    "scripts/check_frp_lifecycle.py",
)
filters = workflow.split("jobs:", 1)[0]
push_filters, pull_filters = filters.split("  pull_request:", 1)
for path in paths:
    token = f'      - "{path}"'
    require(push_filters.count(token) == 1 and pull_filters.count(token) == 1,
            f"{path} must appear exactly once in push and pull_request path filters")

print("FRP asynchronous restart dispatch and workflow contracts: PASS")
