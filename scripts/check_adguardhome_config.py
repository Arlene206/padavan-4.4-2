#!/usr/bin/env python3
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
script_path = ROOT / "trunk/user/adguardhome/adguardhome.sh"
test_path = ROOT / "trunk/user/adguardhome/tests/adguardhome_config_security.sh"
package_makefile_path = ROOT / "trunk/user/adguardhome/Makefile"
user_makefile_path = ROOT / "trunk/user/Makefile"
services_path = ROOT / "trunk/user/rc/src/services.c"
rc_header_path = ROOT / "trunk/user/rc/src/rc.h"
rc_path = ROOT / "trunk/user/rc/src/rc.c"
workflow_path = ROOT / ".github/workflows/adguardhome-config-security.yml"
variables_path = ROOT / "trunk/user/httpd/src/variables.c"
web_ex_path = ROOT / "trunk/user/httpd/src/web_ex.c"
busybox_config_path = ROOT / "trunk/configs/boards/busybox.config"
busybox_flock_path = ROOT / "trunk/user/busybox/busybox-1.24.x/util-linux/flock.c"
kernel_addrtype_path = ROOT / "trunk/linux-4.4.x/net/netfilter/xt_addrtype.c"
kernel_netfilter_makefile_path = ROOT / "trunk/linux-4.4.x/net/netfilter/Makefile"
kernel_netfilter_kconfig_path = ROOT / "trunk/linux-4.4.x/net/netfilter/Kconfig"
user_addrtype_path = ROOT / "trunk/user/iptables/iptables-1.8.7/extensions/libxt_addrtype.c"
iptables_package_makefile_path = ROOT / "trunk/user/iptables/Makefile"

script = script_path.read_text()
test = test_path.read_text()
package_makefile = package_makefile_path.read_text()
user_makefile = user_makefile_path.read_text()
services = services_path.read_text()
rc_header = rc_header_path.read_text()
rc = rc_path.read_text()
workflow = workflow_path.read_text()
variables = variables_path.read_text()
web_ex = web_ex_path.read_text()
busybox_config = busybox_config_path.read_text()
busybox_flock = busybox_flock_path.read_text()
kernel_addrtype = kernel_addrtype_path.read_text()
kernel_netfilter_makefile = kernel_netfilter_makefile_path.read_text()
kernel_netfilter_kconfig = kernel_netfilter_kconfig_path.read_text()
user_addrtype = user_addrtype_path.read_text()
iptables_package_makefile = iptables_package_makefile_path.read_text()


def require(condition: bool, message: str) -> None:
    if not condition:
        raise SystemExit(f"AdGuardHome config security contract: FAIL: {message}")


def function_body(source: str, signature: str) -> str:
    start = source.find(signature)
    require(start >= 0, f"missing function {signature}")
    opening = source.find("{", start)
    require(opening >= 0, f"missing opening brace for {signature}")
    closing = source.find(chr(10) + "}", opening + 1)
    require(closing >= 0, f"missing closing brace for {signature}")
    return source[opening + 1:closing]


config = function_body(script, "getconfig(){")
dns_update = function_body(script, "adg_update_dns_config() {")
change_dns = function_body(script, "change_dns() {")
del_dns = function_body(script, "del_dns() {")
start = function_body(script, "start_adg(){")
remove_redirect = function_body(script, "adg_remove_redirect_rule()" + chr(10) + "{")
set_redirect = function_body(script, "set_iptable()" + chr(10) + "{")
clear_redirect = function_body(script, "clear_iptable()" + chr(10) + "{")
wait_stopped = function_body(script, "adg_wait_stopped() {")
stop_daemon = function_body(script, "adg_stop_daemon() {")
stop = function_body(script, "stop_adg(){")
lock_path = function_body(script, "adg_lifecycle_lock_path() {")
lock_prepare = function_body(script, "adg_lifecycle_lock_prepare() {")
lock_execute = function_body(script, "adg_lifecycle_lock_execute() {")
locked_action = function_body(script, "adg_lifecycle_locked_action() {")
lock_run = function_body(script, "adg_lifecycle_lock_run() {")
require('while [ "$adg_remove_count" -lt 64 ]; do' in remove_redirect and
        'case "$adg_check_status" in' in remove_redirect and
        '"$adg_iptables" -t nat -C PREROUTING "$@"' in remove_redirect and
        '"$adg_iptables" -t nat -D PREROUTING "$@"' in remove_redirect and
        'return "$adg_check_status"' in remove_redirect,
        "redirect cleanup must bound duplicate deletion and distinguish absent rules from xtables errors")
require("if ! clear_iptable; then" in set_redirect and
        "if [ \"$(nvram get adg_redirect)\" = 2 ]; then" in set_redirect,
        "redirect setup must first clear stale rules and respect the configured redirect mode")
require("-m addrtype --dst-type LOCAL" in set_redirect and
        "! -d 127.0.0.0/8" in set_redirect and
        "! -d ::1/128" in set_redirect and "! -d fe80::/10" in set_redirect,
        "IPv4/IPv6 redirects must follow local route state while excluding loopback/link-local")
require('command -v ip6tables' in set_redirect and
        '检查IPv6 DNS NAT表失败' in set_redirect and
        'command -v ip6tables' in clear_redirect and
        '检查IPv6 DNS NAT表失败' in clear_redirect,
        "a missing IPv6 binary may be skipped, but a present/unusable nat table must fail closed")
require("ifconfig" not in set_redirect and "inet addr" not in set_redirect,
        "redirect setup must not snapshot current addresses into firewall rules")
require("adg_remove_redirect_rule iptables -p tcp" in clear_redirect and
        "adg_remove_redirect_rule iptables -p udp" in clear_redirect and
        "adg_remove_redirect_rule ip6tables -p tcp" in clear_redirect and
        "adg_remove_redirect_rule ip6tables -p udp" in clear_redirect,
        "redirect cleanup must cover TCP/UDP in both address families")
require("clear_iptable" in set_redirect and
        "IPv6 UDP DNS重定向失败" in set_redirect,
        "partial redirect publication must roll back both address families")

require('MODULE_ALIAS("ip6t_addrtype")' in kernel_addrtype and
        '#if IS_ENABLED(CONFIG_IP6_NF_IPTABLES)' in kernel_addrtype and
        "addrtype_mt6" in kernel_addrtype,
        "Linux 4.4 addrtype implementation must retain IPv6 matching")
require("config NETFILTER_XT_MATCH_ADDRTYPE" in kernel_netfilter_kconfig and
        "obj-$(CONFIG_NETFILTER_XT_MATCH_ADDRTYPE) += xt_addrtype.o" in kernel_netfilter_makefile,
        "kernel Kconfig/Makefile must build the addrtype match")
require("--dst-type" in user_addrtype and '"LOCAL"' in user_addrtype,
        "iptables userspace extension must expose LOCAL addrtype matching")
require("PKG_VERSION:=1.8.7" in iptables_package_makefile and
        "xtables-legacy-multi" in iptables_package_makefile,
        "ROMFS iptables package must install the legacy xtables command family")

board_kernel_configs = sorted((ROOT / "trunk/configs/boards").glob("*/kernel-4.4.x.config"))
require(bool(board_kernel_configs), "no per-board Linux 4.4 configs were found")
missing_addrtype = [
    str(path.relative_to(ROOT)) for path in board_kernel_configs
    if "CONFIG_NETFILTER_XT_MATCH_ADDRTYPE=y" not in path.read_text()
    or "CONFIG_IP6_NF_IPTABLES=y" not in path.read_text()
]
require(not missing_addrtype,
        "all board profiles must retain IPv4/IPv6 addrtype support: " + ", ".join(missing_addrtype))

require('adg_file="${ADG_CONFIG_FILE:-/etc/storage/adg.sh}"' in config,
        "testable config path must retain the production default")
require('if [ -L "$adg_file" ]; then' in config,
        "symlink config destinations must be rejected before writes")
require('if [ -e "$adg_file" ] && [ ! -f "$adg_file" ]; then' in config,
        "non-regular config destinations must be rejected")
require('mktemp "${adg_file}.tmp.XXXXXX"' in config,
        "default config must use a unique same-directory temporary")
require('cat > "$adg_tmp" <<-' in config and 'umask 077' in config,
        "default config contents must be written to a private stage")
require('chmod 600 "$adg_tmp"' in config and 'mv -f "$adg_tmp" "$adg_file"' in config,
        "private stage must be permissioned before atomic publication")
require('chmod 600 "$adg_file"' in config and 'chmod 755 "$adg_file"' not in config,
        "all existing and generated config files must be mode 0600")
require('rm -f "$adg_tmp"' in config and 'return 1' in config,
        "failed staging, chmod or rename must clean up and fail closed")
require('auth_pass: adguardhome' in config and 'private_key: ""' in config,
        "source contract must recognize secret-bearing config fields")
require('if ! getconfig; then' in start and 'if ! set_iptable; then' in start and
        start.index('if ! getconfig; then') < start.index('if ! set_iptable; then') <
        start.index('if ! change_dns; then') < start.index('eval "AdGuardHome'),
        "config, firewall and resolver failures must stop before daemon launch")
require('mktemp "${adg_dns_conf}.adg-new.XXXXXX"' in dns_update and
        'mktemp "${adg_dns_conf}.adg-old.XXXXXX"' in dns_update and
        'cp -p "$adg_dns_conf" "$adg_dns_tmp"' in dns_update and
        'cp -p "$adg_dns_conf" "$adg_dns_backup"' in dns_update and
        'mv -f "$adg_dns_tmp" "$adg_dns_conf"' in dns_update and
        'if adg_restart_dhcpd; then' in dns_update and
        'mv -f "$adg_dns_backup" "$adg_dns_conf"' in dns_update and
        dns_update.index('mv -f "$adg_dns_tmp" "$adg_dns_conf"') <
        dns_update.index('if adg_restart_dhcpd; then') <
        dns_update.index('mv -f "$adg_dns_backup" "$adg_dns_conf"'),
        "resolver updates must stage beside the live file, verify reload and restore the previous file on failure")
require('adg_update_dns_config enable' in change_dns and 'return 1' in change_dns and
        'adg_update_dns_config disable' in del_dns,
        "change_dns/del_dns must propagate resolver transaction failures")
require('if ! change_dns; then' in start and 'if ! clear_iptable; then' in start and
        start.index('if ! change_dns; then') < start.index('if ! clear_iptable; then'),
        "failed resolver setup must roll back firewall rules before daemon launch")

require('if pidof AdGuardHome >/dev/null 2>&1; then' in start and
        start.index('if pidof AdGuardHome') < start.index('getconfig') <
        start.index('if ! set_iptable; then') < start.index('change_dns') <
        start.index('eval "AdGuardHome'),
        "duplicate starts must be detected before config, firewall, DNS or daemon side effects")
require('while [ "$adg_wait" -lt 5 ]; do' in wait_stopped and
        'sleep 1' in wait_stopped and 'pidof AdGuardHome' in wait_stopped,
        "daemon absence polling must use the live process name and a five-second bound")
require(stop_daemon.index('killall AdGuardHome') <
        stop_daemon.index('adg_wait_stopped') <
        stop_daemon.index('killall -9 AdGuardHome') <
        stop_daemon.rindex('adg_wait_stopped'),
        "daemon stop must verify TERM grace before name-based KILL and post-verify absence")
require('if ! clear_iptable; then' in stop and
        'if ! del_dns; then' in stop and 'if ! adg_stop_daemon; then' in stop and
        stop.index('if ! clear_iptable; then') < stop.index('if ! del_dns; then') <
        stop.index('if ! adg_stop_daemon; then') < stop.index('rm -rf /tmp/AdGuardHome'),
        "stop must verify firewall cleanup and resolver reload before stopping daemon and removing its workdir")
require('pidof AdGuardHome' in stop_daemon and 'killall -9 AdGuardHome' in stop_daemon and
        'pidof AdGuardHome' in wait_stopped,
        "stop must re-enumerate the process by name, not cache a numeric PID")
require('echo /tmp/.adguardhome.lifecycle.lock' in lock_path and
        'never unlink this file' in script and
        '[ -L "$adg_lock_file" ]' in lock_prepare and
        'set -C' in lock_prepare and
        "stat -c '%u %a %h'" in lock_prepare and
        '[ "$2" != 600 ]' in lock_prepare and
        '[ "$3" != 1 ]' in lock_prepare,
        "lifecycle lock must use a private stable inode and reject symlink, mode and hardlink attacks")
require('flock -n 9' in lock_execute and
        'exec 9>>"$adg_lock_file"' in lock_execute and
        'while ! flock -n 9; do' in lock_execute and
        '[ "$adg_lock_attempt" -ge "$adg_wait_seconds" ]' in lock_execute and
        'exec 9>&-' in lock_execute,
        "lifecycle operations must share a bounded kernel flock and release FD 9 on all outcomes")
require('if ! stop_adg; then' in locked_action and
        'if [ "$(nvram get adg_enable)" = 1 ]; then' in locked_action and
        locked_action.index('if ! stop_adg; then') <
        locked_action.index('nvram get adg_enable') <
        locked_action.index('start_adg', locked_action.index('if ! stop_adg; then')),
        "restart must hold one lock across stop and conditional start")
require('adg_lifecycle_lock_prepare "$adg_lock_file"' in lock_run and
        'adg_lifecycle_lock_execute "$adg_lock_file"' in lock_run and
        'start|stop|restart' in lock_run,
        "all launcher service actions must pass through the same checked lock")
require('( exec 9>&-; eval "AdGuardHome -c $adg_file -w /tmp/AdGuardHome -v" ) &' in start and
        '/sbin/restart_dhcpd 9>&-' in script,
        "detached AdGuardHome and dnsmasq restarts must not retain the launcher lock descriptor")
require('CONFIG_FLOCK=y' in busybox_config and
        'fd = xatoi_positive(argv[0]);' in busybox_flock and
        'if (flock(fd, mode) != 0)' in busybox_flock,
        "the common BusyBox target config must build descriptor-mode flock used by the shell launcher")

require('$(INSTALL_BIN) ./adguardhome.sh $(ROMFSDIR)/usr/bin/adguardhome.sh' in package_makefile,
        "ROMFS must install the audited launcher")
require('$(INSTALL_BIN) $(PKG_BUILD_DIR)/AdGuardHome $(ROMFSDIR)/usr/bin/AdGuardHome' in package_makefile,
        "ROMFS must install the daemon inspected by the lifecycle caller")
require('dir_$(CONFIG_FIRMWARE_INCLUDE_ADGUARDHOME)' in user_makefile and
        '+= adguardhome' in user_makefile,
        "user package graph must retain the firmware configuration gate")
require('eval("/usr/bin/adguardhome.sh","stop")' in services and
        'eval("/usr/bin/adguardhome.sh","start")' in services and
        'restart_adguardhome(void)' in services,
        "RC service wrapper must call the ROMFS-installed launcher")
require('eval("/usr/bin/adguardhome.sh","restart")' in services and
        'void restart_adguardhome(void){\n\t/* Keep stop + conditional start inside one launcher lifecycle lock. */' in services,
        "RC restart wrapper must use one atomic launcher restart instead of separate stop/start locks")
require('{"adg_enable", "", NULL, EVM_RESTART_ADGUARDHOME}' in variables and
        '{"AdguardHomeConf",\t\tvariables_AdguardHomeConf}' in variables and
        'notify_rc(events_desc[i].notify_cmd)' in web_ex and
        'RCN_RESTART_ADGUARDHOME' in rc and 'restart_adguardhome();' in rc,
        "AdGuardHome UI NVRAM changes must flow through the RC restart notification")
require('void restart_adguardhome(void);' in rc_header and
        'RCN_RESTART_ADGUARDHOME' in rc and 'restart_adguardhome();' in rc,
        "RC prototype and notification dispatch must remain connected")

for marker in (
    "world-readable mode is repaired",
    "symlink config destination was accepted",
    "FIFO config destination was accepted",
    "rename failure did not clean the private stage",
    "chmod failure published a config",
    "startup changed DNS/firewall before config validation",
    "duplicate start changed config/DNS/firewall state or launched again",
    "stop reported success while AdGuardHome remained alive after SIGKILL",
    "failed stop removed the workdir or skipped pre-stop resolver cleanup",
    "address-independent IPv4/IPv6 DNS redirects, duplicate cleanup and rollback: PASS",
    "repeated start left duplicate IPv4 redirect rules",
    "redirect rules still depend on enumerated interface address snapshots",
    "failed IPv6 publication did not roll back IPv4/IPv6 rules",
    "redirect cleanup ignored the 64-rule safety bound",
    "cleanup treated an xtables -C error as an absent rule",
    "set_iptable ignored an IPv6 NAT table probe failure",
    "clear_iptable ignored an IPv6 NAT table probe failure",
    "stop reported success after firewall cleanup failed",
    "start_adg ignored a firewall setup failure",
    "dnsmasq resolver publication, status and rollback: PASS",
    "dnsmasq resolver update did not preserve the previous config on failure",
    "start continued after resolver reload failure",
    "stop reported success after resolver removal failed",
    "lifecycle flock policy, serialization or stale-owner recovery failed",
    "AdGuardHome lifecycle lock stable inode, permissions and stale-owner recovery: PASS",
    "AdGuardHome lifecycle lock excludes concurrent transitions and releases after failed actions: PASS",
    "restart did not keep stop and conditional start in a single locked action",
    "AdGuardHome restart is a single locked stop/start transaction: PASS",
):
    require(marker in test, f"host regression missing case: {marker}")

paths = [
    ".github/workflows/adguardhome-config-security.yml",
    "scripts/check_adguardhome_config.py",
    "trunk/user/adguardhome/adguardhome.sh",
    "trunk/user/adguardhome/tests/adguardhome_config_security.sh",
    "trunk/user/adguardhome/Makefile",
    "trunk/user/Makefile",
    "trunk/user/rc/src/services.c",
    "trunk/user/rc/src/rc.h",
    "trunk/user/rc/src/rc.c",
    "trunk/user/httpd/src/variables.c",
    "trunk/user/httpd/src/web_ex.c",
    "trunk/configs/boards/busybox.config",
    "trunk/user/busybox/busybox-1.24.x/util-linux/flock.c",
    "trunk/configs/boards/*/kernel-4.4.x.config",
    "trunk/linux-4.4.x/net/netfilter/xt_addrtype.c",
    "trunk/linux-4.4.x/net/netfilter/Makefile",
    "trunk/linux-4.4.x/net/netfilter/Kconfig",
    "trunk/user/iptables/iptables-1.8.7/extensions/libxt_addrtype.c",
    "trunk/user/iptables/Makefile",
]
push = workflow.split("  push:", 1)[1].split("  pull_request:", 1)[0]
pull = workflow.split("  pull_request:", 1)[1].split("permissions:", 1)[0]
for path in paths:
    token = f'      - "{path}"'
    require(push.count(token) == 1 and pull.count(token) == 1,
            f"{path} must appear exactly once in push and pull_request path filters")

print("AdGuardHome config privacy, bounded firewall cleanup and RC integration contracts: PASS")
