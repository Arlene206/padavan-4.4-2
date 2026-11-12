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
start = function_body(script, "start_adg(){")
remove_redirect = function_body(script, "adg_remove_redirect_rule() {")
set_redirect = function_body(script, "set_iptable()\n{")
clear_redirect = function_body(script, "clear_iptable()\n{")
wait_stopped = function_body(script, "adg_wait_stopped() {")
stop_daemon = function_body(script, "adg_stop_daemon() {")
stop = function_body(script, "stop_adg(){")
require('while "$adg_iptables" -t nat -C PREROUTING "$@"' in remove_redirect and
        '"$adg_iptables" -t nat -D PREROUTING "$@"' in remove_redirect,
        "redirect cleanup must remove every exact duplicate and stop on failed deletion")
require("if ! clear_iptable; then" in set_redirect and
        "if [ \"$(nvram get adg_redirect)\" = 2 ]; then" in set_redirect,
        "redirect setup must first clear stale rules and respect the configured redirect mode")
require("-m addrtype --dst-type LOCAL" in set_redirect and
        "! -d 127.0.0.0/8" in set_redirect and
        "! -d ::1/128" in set_redirect and "! -d fe80::/10" in set_redirect,
        "IPv4/IPv6 redirects must follow local route state while excluding loopback/link-local")
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
require('if ! getconfig; then' in start and
        start.index('if ! getconfig; then') < start.index('change_dns') <
        start.index('set_iptable') < start.index('eval "AdGuardHome'),
        "unsafe config preparation must stop before DNS/firewall changes or launch")

require('if pidof AdGuardHome >/dev/null 2>&1; then' in start and
        start.index('if pidof AdGuardHome') < start.index('getconfig') <
        start.index('change_dns') < start.index('set_iptable') <
        start.index('eval "AdGuardHome'),
        "duplicate starts must be detected before config, DNS, firewall or daemon side effects")
require('while [ "$adg_wait" -lt 5 ]; do' in wait_stopped and
        'sleep 1' in wait_stopped and 'pidof AdGuardHome' in wait_stopped,
        "daemon absence polling must use the live process name and a five-second bound")
require(stop_daemon.index('killall AdGuardHome') <
        stop_daemon.index('adg_wait_stopped') <
        stop_daemon.index('killall -9 AdGuardHome') <
        stop_daemon.rindex('adg_wait_stopped'),
        "daemon stop must verify TERM grace before name-based KILL and post-verify absence")
require('if ! adg_stop_daemon; then' in stop and
        stop.index('if ! adg_stop_daemon; then') < stop.index('rm -rf /tmp/AdGuardHome') <
        stop.index('del_dns') < stop.index('clear_iptable'),
        "workdir, DNS and firewall cleanup must be gated on confirmed daemon exit")
require('pidof AdGuardHome' in stop_daemon and 'killall -9 AdGuardHome' in stop_daemon and
        'pidof AdGuardHome' in wait_stopped,
        "stop must re-enumerate the process by name, not cache a numeric PID")

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
    "failed stop removed runtime state or traffic redirection",
    "address-independent IPv4/IPv6 DNS redirects, duplicate cleanup and rollback: PASS",
    "repeated start left duplicate IPv4 redirect rules",
    "redirect rules still depend on enumerated interface address snapshots",
    "failed IPv6 publication did not roll back IPv4/IPv6 rules",
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

print("AdGuardHome config privacy, atomic publication and RC integration contracts: PASS")
