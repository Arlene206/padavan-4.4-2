#!/usr/bin/env python3
"""Source contracts for Shadowsocks process shutdown and adblock publication."""
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


def read(path: str) -> str:
    return (ROOT / path).read_text(encoding="utf-8")


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def ordered(source: str, tokens: tuple[str, ...], label: str) -> None:
    position = -1
    for token in tokens:
        found = source.find(token, position + 1)
        require(found >= 0, f"{label}: missing or out-of-order {token!r}")
        position = found


script = read("trunk/user/shadowsocks/scripts/shadowsocks.sh")
monitor = read("trunk/user/shadowsocks/scripts/ss-monitor")
helper = read("trunk/user/shadowsocks/scripts/shadowsocks_process_lifecycle.sh")
makefile = read("trunk/user/shadowsocks/Makefile")
test = read("trunk/user/shadowsocks/tests/process_lifecycle.sh")
adblock_test = read("trunk/user/shadowsocks/tests/adblock_config.sh")
workflow = read(".github/workflows/shadowsocks-lifecycle.yml")
busybox_config = read("trunk/user/busybox/busybox-1.24.x/procps/Config.src")
busybox_pgrep = read("trunk/user/busybox/busybox-1.24.x/procps/pgrep.c")

require(". /usr/bin/shadowsocks_process_lifecycle.sh || exit 1" in monitor and
        "ss_stop_named_processes ssr-server" in monitor and
        'ss_stop_named_processes "$bin"' in monitor and
        "ss_stop_named_processes chinadns-ng" in monitor and
        "ss_stop_named_processes dns2tcp dnsproxy" in monitor and
        "killall -q -9 ssr-server" not in monitor and
        "killall -q -9 $bin" not in monitor and
        "kill -9 chinadns-ng" not in monitor,
        "watchcat restarts must use the shared tri-state lifecycle helper")
require("if ss_stop_named_processes ssr-server; then" in monitor and
        monitor.index("if ss_stop_named_processes ssr-server; then") < monitor.index("run_bin /usr/bin/ssr-server") and
        'if ss_stop_named_processes "$bin"; then' in monitor and
        monitor.index('if ss_stop_named_processes "$bin"; then') < monitor.index('run_bin "/usr/bin/$bin"') and
        "if ! ss_stop_named_processes chinadns-ng; then" in monitor and
        monitor.index("if ! ss_stop_named_processes chinadns-ng; then") < monitor.index("chinadns-ng -b 0.0.0.0") and
        monitor.count("if stop_dns_proxy; then") == 2 and
        "if ! stop_dns_proxy; then" in monitor and
        monitor.index("if ! stop_dns_proxy; then") < monitor.index("chinadns-ng -b 0.0.0.0"),
        "watchcat must verify daemon and DNS proxy absence before launching replacements")
require(". /usr/bin/shadowsocks_process_lifecycle.sh || exit 1" in script,
        "runtime script must load the installed lifecycle helper")
require("$(ROMFSINST) $(THISDIR)/scripts/ /usr/bin/" in makefile,
        "ROMFS must install the lifecycle helper beside shadowsocks.sh")
require("ss_stop_named_processes" in script and
        "ss_stop_process_pattern '/usr/bin/ss-monitor'" in script,
        "kill_process must stop all proxy and monitor process groups")
for name in ("v2ray", "xray", "ss-redir", "ssr-redir", "ss-local",
             "trojan", "ipt2socks", "srelay", "ssr-server",
             "chinadns-ng", "dns2tcp", "dnsproxy", "microsocks"):
    require(name in script, f"process inventory lost {name}")

require("kill -9" not in script and "pidof v2ray" not in script and
        "pidof ss-redir" not in script and "grep ss-monitor" not in script,
        "service shutdown must not signal stale PID snapshots")
require("ss_process_pid_state()" in helper and
        'ss_pid_output=$(pidof "$ss_process_name" 2>/dev/null)' in helper and
        'ss_validate_pid_list "$ss_pid_output" || return 2' in helper,
        "pidof must distinguish confirmed absence from lookup errors and malformed output")
require("ss_process_pattern_state()" in helper and
        'ss_pid_output=$(pgrep -f "$ss_process_pattern" 2>/dev/null)' in helper and
        "ss_named_processes_state()" in helper,
        "pgrep and aggregate process inventories must retain tri-state status")
ordered(helper, ['ss_named_processes_state "$@"',
                 'ss_process_pid_state "$ss_process_name"',
                 'killall -q "$ss_process_name"',
                 'ss_wait_named_processes 3 "$@"',
                 'killall -q -KILL "$ss_process_name"',
                 'ss_wait_named_processes 1 "$@"'],
        "named preflight/TERM/wait/fresh-name KILL/postcondition")
ordered(helper, ['ss_process_pattern_state "$ss_process_pattern"',
                 'pkill -TERM -f "$ss_process_pattern"',
                 'ss_wait_process_pattern 3 "$ss_process_pattern"',
                 'pkill -KILL -f "$ss_process_pattern"',
                 'ss_wait_process_pattern 1 "$ss_process_pattern"'],
        "monitor preflight/TERM/wait/fresh-command KILL/postcondition")
require("unknown after TERM blocks KILL" in test and "pidof error blocks named stop" in test and
        "pgrep error blocks monitor stop" in test and
        "unknown after monitor TERM blocks KILL" in test,
        "regression must prove unknown inventory prevents signals and escalation")
require(helper.count('ss_wait_limit=$1') == 2 and
        helper.count('[ "$ss_wait_count" -lt "$ss_wait_limit" ]') == 2 and
        helper.count('sleep 1 || return 2') == 5 and
        'ss_wait_named_processes 3 "$@"' in helper and
        'ss_wait_named_processes 1 "$@"' in helper and
        'ss_wait_process_pattern 3 "$ss_process_pattern"' in helper and
        'ss_wait_process_pattern 1 "$ss_process_pattern"' in helper,
        "named and pattern shutdown waits must remain bounded and fail closed on sleep errors")
require("ss_process_pid_count()" in helper and
        "ss_wait_named_process_count()" in helper and
        "awk '{ count += NF } END { print count + 0 }'" in helper,
        "startup readiness must count validated PIDs and preserve UNKNOWN")

main_run_start = script.index("run_bin() {")
main_run_end = script.index("\n\n# Keep filesystem operations", main_run_start)
main_run = script[main_run_start:main_run_end]
require('if [ "$1" = "--wait-for" ]' in main_run and
        'process_name="$2"' in main_run and
        'process_name="${1##*/}"' in main_run and
        'expected_count=$((process_count + 1))' in main_run and
        'ss_wait_named_process_count "$process_name" "$expected_count" 5' in main_run and
        'exec "$@"' in main_run,
        "main launcher must verify the selected daemon process after exec")

for label, start_token, end_token in (
    ("TCP", "start_redir_tcp() {", "\n\nstart_redir_udp() {"),
    ("UDP", "start_redir_udp() {", "\n\nstop_dns_proxy() {"),
    ("local SOCKS5", "start_local() {", "\n\nrules() {"),
):
    begin = script.index(start_token)
    section = script[begin:script.index(end_token, begin)]
    launch_lines = [line.strip() for line in section.splitlines()
                    if line.strip().startswith("run_bin ")]
    require(launch_lines and all("|| return $?" in line for line in launch_lines),
            f"{label} startup must propagate every readiness failure")

require('run_bin --wait-for ipt2socks lua /etc_ro/ss/gensocks.lua' in script,
        "Lua generator must be checked against its ipt2socks child")

monitor_run_start = monitor.index("run_bin() {")
monitor_run_end = monitor.index("\n\nstop_dns_proxy() {", monitor_run_start)
monitor_run = monitor[monitor_run_start:monitor_run_end]
require('ss_process_pid_count "$process_name"' in monitor_run and
        'ss_wait_named_process_count "$process_name" "$expected_count" 5' in monitor_run and
        'exec "$@"' in monitor_run and
        'if run_bin /usr/bin/ssr-server' in monitor and
        'if run_bin "/usr/bin/$bin"' in monitor,
        "watchcat replacement launches must verify new child readiness")

require("run_bin child exits before readiness" in test and
        "run_bin unknown process inventory" in test and
        "pre-existing sibling" in test and
        "gensocks.lua starts ipt2socks" in test,
        "readiness regression must cover immediate exit, UNKNOWN, siblings and Lua child")

require("ss_process_pattern_count()" in helper and
        "ss_wait_process_pattern_count()" in helper and
        'pgrep -f "$ss_count_pattern"' in helper,
        "pattern startup readiness must validate and count pgrep PIDs")
require('if ss_pid_output=$(pidof "$ss_process_name" 2>/dev/null); then' in helper and
        'if ss_count_pid_output=$(pidof "$ss_count_process_name" 2>/dev/null); then' in helper and
        'if ss_pid_output=$(pgrep -f "$ss_process_pattern" 2>/dev/null); then' in helper and
        'if ss_pattern_pid_output=$(pgrep -f "$ss_count_pattern" 2>/dev/null); then' in helper,
        "pidof/pgrep exit statuses must be captured conditionally under caller errexit")
require("ABSENT pidof count was" in test and "ABSENT pgrep count was" in test and
        "Lua-generated ipt2socks readiness" in test,
        "host regression must cover absent inventory and detached-child startup")


watch_start = script[script.index("start_watchcat() {"):script.index("\n\nauto_update() {")]
require('ss_process_pattern_count "/usr/bin/ss-monitor"' in watch_start and
        'ss_launch_watchcat "$server_count"' in watch_start and
        'ss_wait_process_pattern_count "/usr/bin/ss-monitor" "$watchcat_expected_count" 5' in watch_start and
        'return "$watchcat_status"' in watch_start,
        "watchcat startup must fail closed and verify detached monitor readiness")

require("watchcat readiness timeout" in test and
        "watchcat scheduling failure" in test and
        "watchcat UNKNOWN preflight" in test and
        "already-running monitor" in test,
        "watchcat host regression must cover readiness, failure, UNKNOWN and duplicate cases")

require("config PGREP\n\tbool \"pgrep\"\n\tdefault y" in busybox_config and
        "config PKILL\n\tbool \"pkill\"\n\tdefault y" in busybox_config and
        busybox_pgrep.count("Match against entire command line") >= 2,
        "target BusyBox must include full-command-line pgrep/pkill")

close = script[script.index("ssp_close() {"):script.index("\n\n\nclear_iptable() {")]
ordered(close, ["if ! kill_process; then", "return 1", "rm -rf /tmp/cdn",
                "$SS_RULES -f", "cgroups_cleanup"],
        "preserve firewall/cgroup state until processes stop")
for action in ("stop)", "restart)", "reserver)"):
    start = script.index(action, script.index("case $1 in"))
    end = script.find(";;", start)
    require("if ! ssp_close; then" in script[start:end] and "exit 1" in script[start:end],
            f"{action[:-1]} must abort follow-up cleanup/start on stop failure")

require("exit_on_term" in test and "exit_on_kill" in test and
        "never_exit" in test and "Shadowsocks process lifecycle regression: PASS" in test,
        "host regression must cover graceful, forced and failed shutdown")
for path in ("trunk/user/shadowsocks/scripts/shadowsocks.sh",
             "trunk/user/shadowsocks/scripts/ss-monitor",
             "trunk/user/shadowsocks/scripts/shadowsocks_process_lifecycle.sh",
             "trunk/user/shadowsocks/tests/process_lifecycle.sh",
             "trunk/user/shadowsocks/tests/start_status.sh",
             "trunk/user/shadowsocks/Makefile",
             "trunk/user/busybox/busybox-1.24.x/procps/Config.src",
             "trunk/user/busybox/busybox-1.24.x/procps/pgrep.c",
             "scripts/check_shadowsocks_lifecycle.py",
             ".github/workflows/shadowsocks-lifecycle.yml"):
    require(path in workflow, f"workflow path filter missing {path}")


rules_start = script.index("rules() {")
rules_end = script.index("\n\ncase $1 in", rules_start)
rules = script[rules_start:rules_end]
require(
    'if start_rules; then' in rules and 'status=$?' in rules
    and 'return "$status"' in rules,
    "rules() must preserve the original ss-rules exit status",
)

startup = script[script.index("ssp_record_start_failure() {"):script.index("ssp_close() {")]
startup_test = read("trunk/user/shadowsocks/tests/start_status.sh")
ordered(startup, [
    'SS_START_STATUS=0',
    'if rules; then',
    'if cgroups_init; then',
    'if start_redir_tcp; then',
    'if start_redir_udp; then',
    'if start_dns; then',
    'nvram get socks5_enable',
    'if start_local; then',
    'if start_watchcat; then',
    'if ENABLE_SERVER=$(nvram get global_server); then',
    'nvram set check_mode=0',
    'if ssp_run_pppoe_detect; then',
], "required startup-stage status aggregation")
require('if [ "$SS_START_STATUS" -eq 0 ]; then' in startup and
        'SS_START_STATUS="$status"' in startup and
        'Optional Shadowsocks update scheduling failed' in startup,
        "startup must preserve the first required error and keep cron scheduling best-effort")
require('[ -n "$local_server" ] && [ "$local_server" != "nil" ]' in startup and
        'if [ "$GLOBAL_SERVER" != "nil" ]; then' in startup,
        "disabled local-only/global server paths must not become false startup-stage failures")
require(startup.rfind('[ "$SS_START_STATUS" -ne 0 ] && return "$SS_START_STATUS"') >
        startup.index('if ssp_run_pppoe_detect; then'),
        "return the first required failure after WAN reconciliation")
require(script.count("ssp_start || exit $?") == 2,
        "start/restart CLI actions must preserve ssp_start status before cache dropping")
require("FAIL_TCP=31" in startup_test and "FAIL_UDP=32" in startup_test and
        "FAIL_CGROUP_MKDIR=30" in startup_test and
        "FAIL_CGROUP_CPU_WRITE=38" in startup_test and
        "FAIL_CGROUP_MEM_WRITE=39" in startup_test and
        "FAIL_CGROUP_READ=40" in startup_test and "FAIL_DNS=35" in startup_test and
        "FAIL_LOCAL=36" in startup_test and "FAIL_WATCHCAT=37" in startup_test and
        "FAIL_RULES=12" in startup_test and "FAIL_NVRAM_SET=41" in startup_test and
        "FAIL_DETECT=42" in startup_test and "FAIL_AUTO_UPDATE=55" in startup_test and
        "Shadowsocks required-stage startup status regression: PASS" in startup_test,
        "startup status regression must cover required failures and disabled semantics")
print("Shadowsocks required-stage startup status source contract: PASS")


dns_stop_start = script.index("stop_dns_proxy() {")
dns_start_start = script.index("start_dns_proxy() {", dns_stop_start)
dns_start_end = script.index("\n\nstart_dns() {", dns_start_start)
dns_stop = script[dns_stop_start:dns_start_start]
dns_start = script[dns_start_start:dns_start_end]
require("ss_stop_named_processes dns2tcp dnsproxy" in dns_stop,
        "DNS proxy teardown must use fail-closed process inventory")
ordered(dns_start, [
    'command -v "$dns_process"',
    'dns2tcp -L"127.0.0.1#5353"',
    'dnsproxy -d -p 5353',
    'ss_wait_named_process_started "$dns_process" 5',
    'ss_stop_named_processes "$dns_process"',
    'return "$launch_status"',
], "DNS proxy startup readiness and rollback")
require('if stop_dns_proxy; then' in script and
        'if start_dns_proxy; then' in script,
        "router/GFW DNS startup must propagate cleanup and launch failures")
require("ss_wait_named_process_started() {" in helper and
        'case "$ss_start_state" in' in helper and
        '*) return 2 ;;' in helper,
        "DNS readiness must distinguish unknown process inventory from absence")
require("DNS proxy child launch failure" in read("trunk/user/shadowsocks/tests/process_lifecycle.sh") and
        "DNS proxy unknown startup inventory" in read("trunk/user/shadowsocks/tests/process_lifecycle.sh"),
        "DNS readiness regression must cover child exit and unknown inventory")
print("Shadowsocks DNS proxy readiness source contract: PASS")
print("Shadowsocks process lifecycle source contract: PASS")


adblock_start = script.index("ss_publish_adblock_config() {")
adblock_end = script.index("\nstart_AD() {", adblock_start)
adblock_helper = script[adblock_start:adblock_end]
adblock_action_start = script.index("start_AD() {", adblock_end)
adblock_action_end = script.index("\n\n# ========== 启动 Socks5 代理", adblock_action_start)
adblock_action = script[adblock_action_start:adblock_action_end]
require("mktemp" in adblock_helper and "chmod 644" in adblock_helper and
        'mv -f "$ss_adblock_tmp" "$ss_adblock_output"' in adblock_helper,
        "adblock publication must use a complete same-directory atomic replacement")
require("'^[[:space:]]*address='" in adblock_helper and
        "'^[|][|][A-Za-z0-9][A-Za-z0-9.-]*\\^$'" in adblock_helper,
        "adblock parser must distinguish dnsmasq configs from simple domain rules")
require("check =" not in adblock_action and "grep -wq" not in adblock_action,
        "adblock detection must not use a command as a shell assignment")
ordered(adblock_action, ["curl -fsS", "ss_publish_adblock_config",
                         "去广告文件下载并发布成功"],
        "download then validate/publish adblock config")
require("address=/ads.example.com/0.0.0.0" in adblock_test and
        "unsupported-only Adblock input" in adblock_test and
        "last known-good" in adblock_test,
        "adblock regression must cover conversion and fail-safe replacement")
print("Shadowsocks adblock configuration source contract: PASS")


require("'^[[:space:]]*(#|address=|$)'" in adblock_helper,
        "downloaded dnsmasq configs must whitelist comments and address rules")
require("dhcp-script=/tmp/attacker.sh" in adblock_test and
        "conf-file=/tmp/attacker.conf" in adblock_test,
        "adblock regression must reject arbitrary dnsmasq directive injection")
print("Downloaded dnsmasq directive whitelist contract: PASS")

cgroup_helpers = script[script.index("ss_cgroup_mkdir() {"):script.index("cgroups_cleanup() {")]
cgroup_init = cgroup_helpers[cgroup_helpers.index("cgroups_init() {"):]
require(
    "mkdir -p \"$1\"" in cgroup_helpers
    and 'printf \'%s\\n\' "$1" > "$2"' in cgroup_helpers
    and 'cat "$1"' in cgroup_helpers,
    "cgroup filesystem operations must be isolated behind testable helpers",
)
for operation in (
    "ss_cgroup_mkdir \"$cpu_dir\"",
    "ss_cgroup_mkdir \"$memory_dir\"",
    "ss_cgroup_write_value \"$cpu_limit\" \"$cpu_dir/cpu.shares\"",
    "ss_cgroup_write_value \"$mem_limit\" \"$memory_dir/memory.limit_in_bytes\"",
    "ss_cgroup_read_value \"$memory_dir/memory.limit_in_bytes\"",
):
    require(operation in cgroup_init, f"cgroups_init is missing {operation}")
require(cgroup_init.count("status=$?") >= 5,
        "cgroups_init must preserve mkdir/write/read child exit codes")
for marker in (
    "resource-limit directory creation failure",
    "CPU shares write failure",
    "memory limit write failure",
    "memory limit readback failure",
):
    require(marker in startup_test, f"cgroup regression is missing {marker}")
print("Shadowsocks cgroup initialization status source contract: PASS")

