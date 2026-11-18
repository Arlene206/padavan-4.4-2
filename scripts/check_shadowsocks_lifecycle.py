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
        helper.count('sleep 1 || return 2') == 2 and
        'ss_wait_named_processes 3 "$@"' in helper and
        'ss_wait_named_processes 1 "$@"' in helper and
        'ss_wait_process_pattern 3 "$ss_process_pattern"' in helper and
        'ss_wait_process_pattern 1 "$ss_process_pattern"' in helper,
        "named and pattern shutdown waits must remain bounded and fail closed on sleep errors")
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
require('SS_START_STATUS="$status"' in startup and
        '[ "$SS_START_STATUS" -eq 0 ] || return 0' in startup and
        'Optional Shadowsocks update scheduling failed' in startup,
        "startup must preserve first required error and keep cron scheduling best-effort")
require('[ -n "$local_server" ] && [ "$local_server" != "nil" ]' in startup and
        'if [ "$GLOBAL_SERVER" != "nil" ]; then' in startup,
        "disabled local-only/global server paths must not become false startup-stage failures")
require(startup.rfind('[ "$SS_START_STATUS" -ne 0 ] && return "$SS_START_STATUS"') >
        startup.index('if ssp_run_pppoe_detect; then'),
        "return the first required failure after WAN reconciliation")
require(script.count("ssp_start || exit $?") == 2,
        "start/restart CLI actions must preserve ssp_start status before cache dropping")
require("FAIL_TCP=31" in startup_test and "FAIL_UDP=32" in startup_test and
        "FAIL_CGROUPS=30" in startup_test and "FAIL_DNS=35" in startup_test and
        "FAIL_LOCAL=36" in startup_test and "FAIL_WATCHCAT=37" in startup_test and
        "FAIL_RULES=12" in startup_test and "FAIL_NVRAM_SET=41" in startup_test and
        "FAIL_DETECT=42" in startup_test and "FAIL_AUTO_UPDATE=55" in startup_test and
        "Shadowsocks required-stage startup status regression: PASS" in startup_test,
        "startup status regression must cover required failures and disabled semantics")
print("Shadowsocks required-stage startup status source contract: PASS")

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
