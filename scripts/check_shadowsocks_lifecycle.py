#!/usr/bin/env python3
"""Source contract for bounded Shadowsocks proxy-process shutdown."""
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
ordered(helper, ['pidof "$ss_process_name"',
                 'killall -q "$ss_process_name"',
                 'while ss_any_named_process_running "$@"',
                 'pidof "$ss_process_name"',
                 'killall -q -KILL "$ss_process_name"',
                 'if ss_any_named_process_running "$@"'],
        "named process TERM/wait/fresh-name KILL/postcondition")
ordered(helper, ['pgrep -f "$ss_process_pattern"',
                 'pkill -TERM -f "$ss_process_pattern"',
                 'while ss_any_process_pattern_running "$ss_process_pattern"',
                 'pgrep -f "$ss_process_pattern"',
                 'pkill -KILL -f "$ss_process_pattern"',
                 'if ss_any_process_pattern_running "$ss_process_pattern"'],
        "monitor TERM/wait/fresh-command KILL/postcondition")
require('[ "$ss_wait_count" -lt 3 ]' in helper and
        '[ "$ss_wait_count" -lt 1 ]' in helper,
        "process shutdown waits must remain bounded")
require("config PGREP\n\tbool \"pgrep\"\n\tdefault y" in busybox_config and
        "config PKILL\n\tbool \"pkill\"\n\tdefault y" in busybox_config and
        '"\\n\\t-f\\tMatch against entire command line"' in busybox_pgrep,
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
