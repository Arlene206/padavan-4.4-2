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
helper = read("trunk/user/shadowsocks/scripts/shadowsocks_process_lifecycle.sh")
makefile = read("trunk/user/shadowsocks/Makefile")
test = read("trunk/user/shadowsocks/tests/process_lifecycle.sh")
adblock_test = read("trunk/user/shadowsocks/tests/adblock_config.sh")
workflow = read(".github/workflows/shadowsocks-lifecycle.yml")
busybox_config = read("trunk/user/busybox/busybox-1.24.x/procps/Config.src")
busybox_pgrep = read("trunk/user/busybox/busybox-1.24.x/procps/pgrep.c")

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
             "trunk/user/shadowsocks/scripts/shadowsocks_process_lifecycle.sh",
             "trunk/user/shadowsocks/tests/process_lifecycle.sh",
             "trunk/user/shadowsocks/Makefile",
             "trunk/user/busybox/busybox-1.24.x/procps/Config.src",
             "trunk/user/busybox/busybox-1.24.x/procps/pgrep.c",
             "scripts/check_shadowsocks_lifecycle.py",
             ".github/workflows/shadowsocks-lifecycle.yml"):
    require(path in workflow, f"workflow path filter missing {path}")

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
