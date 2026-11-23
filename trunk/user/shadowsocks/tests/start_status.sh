#!/bin/sh
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
SCRIPT="$HERE/../scripts/shadowsocks.sh"
TMP_DIR=$(mktemp -d)
CALL_LOG="$TMP_DIR/calls.log"
LOG_FILE="$TMP_DIR/ss.log"
trap 'rm -rf "$TMP_DIR"' EXIT HUP INT TERM

DNSMASQ_HELPER="$HERE/../scripts/dnsmasq_config_lifecycle.sh"
. "$DNSMASQ_HELPER"

sed -n \
	-e '/^ss_cgroup_mkdir() {/,/^}/p' \
	-e '/^ss_cgroup_write_value() {/,/^}/p' \
	-e '/^ss_cgroup_read_value() {/,/^}/p' \
	-e '/^cgroups_init() {/,/^}/p' \
	-e '/^rules() {/,/^}/p' \
	-e '/^ssp_record_start_failure() {/,/^}/p' \
	-e '/^ssp_run_pppoe_detect() {/,/^}/p' \
	-e '/^ssp_start() {/,/^}/p' \
	-e '/^ressp() {/,/^}/p' \
	-e '/^ss_dns_proxy_config_read() {/,/^}/p' \
	-e '/^ss_restart_dns_proxy() {/,/^}/p' \
	-e '/^ss_publish_and_restart_dnsmasq() {/,/^}/p' \
	-e '/^ss_restart_dhcpd() {/,/^}/p' \
	-e '/^ss_run_detect() {/,/^}/p' \
	-e '/^ssp_close() {/,/^}/p' \
	"$SCRIPT" > "$TMP_DIR/start-functions.sh"
. "$TMP_DIR/start-functions.sh"

MOCK_BIN="$TMP_DIR/mock-bin"
mkdir -p "$MOCK_BIN"
for mock_dns_binary in dnsproxy dns2tcp; do
	printf '#!/bin/sh\nexit 0\n' > "$MOCK_BIN/$mock_dns_binary"
	chmod 755 "$MOCK_BIN/$mock_dns_binary"
done
PATH="$MOCK_BIN:$PATH"
export PATH

fail() { echo "Shadowsocks startup status regression: FAIL: $*" >&2; exit 1; }
record() { printf '%s\n' "$*" >> "$CALL_LOG"; }
log() { record "log:$*"; printf '%s\n' "$*" >> "$LOG_FILE"; }

nvram() {
	case "$1:$2" in
		get:ss_enable) echo 1 ;;
		get:ss_cgroups) record cgroups_enabled_probe; echo "${MOCK_SS_CGROUPS:-0}" ;;
		get:ss_cgoups_cpu_s) echo "${MOCK_CPU_LIMIT:-128}" ;;
		get:ss_cgoups_mem_s) echo "${MOCK_MEM_LIMIT:-1048576}" ;;
		get:global_server) echo "${MOCK_CONFIGURED_GLOBAL_SERVER:-$GLOBAL_SERVER}" ;;
		get:socks5_enable) echo "$SOCKS5_ENABLE" ;;
		get:pdnsd_enable)
			record get:pdnsd_enable
			[ "${FAIL_PDNSD_READ:-0}" -eq 0 ] || return "$FAIL_PDNSD_READ"
			echo "${MOCK_PDNSD_ENABLE:-0}"
			;;
		get:tunnel_forward)
			record get:tunnel_forward
			[ "${FAIL_TUNNEL_READ:-0}" -eq 0 ] || return "$FAIL_TUNNEL_READ"
			echo "${MOCK_TUNNEL_FORWARD:-8.8.8.8#53}"
			;;
		get:backup_server)
			[ "${FAIL_BACKUP_READ:-0}" -eq 0 ] || return "$FAIL_BACKUP_READ"
			echo "${MOCK_BACKUP_SERVER:-backup-node}"
			;;
		get:udp_relay_server) echo "${MOCK_UDP_RELAY_SERVER:-nil}" ;;
		set:check_mode=0) record "nvram:set:$2"; return "${FAIL_NVRAM_SET:-0}" ;;
		*) return 90 ;;
	esac
}
ss_cgroup_mkdir() {
	record "cgroup_mkdir:$1"
	return "${FAIL_CGROUP_MKDIR:-0}"
}
ss_cgroup_write_value() {
	record "cgroup_write:$2"
	case "$2" in
		*/cpu.shares) return "${FAIL_CGROUP_CPU_WRITE:-0}" ;;
		*/memory.limit_in_bytes) return "${FAIL_CGROUP_MEM_WRITE:-0}" ;;
	esac
	return 0
}
ss_cgroup_read_value() {
	record cgroup_read
	[ "${FAIL_CGROUP_READ:-0}" -eq 0 ] || return "${FAIL_CGROUP_READ}"
	printf '%s' "${MOCK_MEM_LIMIT_BYTES:-1048576}"
}
start_rules() { record rules; record "rules_for:$GLOBAL_SERVER:$UDP_RELAY_SERVER"; return "${FAIL_RULES:-0}"; }
start_redir_tcp() { record start_redir_tcp; record "tcp_server:$GLOBAL_SERVER"; return "${FAIL_TCP:-0}"; }
start_redir_udp() { record start_redir_udp; record "udp_server:$GLOBAL_SERVER:$UDP_RELAY_SERVER"; return "${FAIL_UDP:-0}"; }
start_dns() { record start_dns; return "${FAIL_DNS:-0}"; }
start_local() { record start_local; record "local_global_server:$GLOBAL_SERVER"; return "${FAIL_LOCAL:-0}"; }
start_watchcat() { record start_watchcat; return "${FAIL_WATCHCAT:-0}"; }
stop_dns_proxy() { record stop_dns_proxy; return "${FAIL_DNS_PROXY_STOP:-0}"; }
start_dns_proxy() { record "start_dns_proxy:$SS_DNS_PROXY_PROCESS:$SS_DNS_PROXY_SERVER"; return "${FAIL_DNS_PROXY_START:-0}"; }
auto_update() { record auto_update; return "${FAIL_AUTO_UPDATE:-0}"; }
ssp_run_pppoe_detect() { record pppoe_detect; return "${FAIL_DETECT:-0}"; }

reset() {
	: > "$CALL_LOG"
	: > "$LOG_FILE"
	GLOBAL_SERVER=node1
	NAME=shadowsocksr
	MOCK_SS_CGROUPS=0
	lancons="test LAN policy"
	SOCKS5_ENABLE=local-node
	pppoemwan=1
	SS_START_STATUS=0
	SS_DNS_PROXY_PROCESS=""
	SS_DNS_PROXY_SERVER=""
	dnsserver=""
	pdnsd_enable_flag=""
	unset FAIL_RULES FAIL_CGROUPS FAIL_TCP FAIL_UDP FAIL_DNS FAIL_LOCAL
	unset FAIL_WATCHCAT FAIL_AUTO_UPDATE FAIL_NVRAM_SET FAIL_DETECT
	unset FAIL_CGROUP_MKDIR FAIL_CGROUP_CPU_WRITE FAIL_CGROUP_MEM_WRITE FAIL_CGROUP_READ
	unset MOCK_BACKUP_SERVER MOCK_UDP_RELAY_SERVER MOCK_CONFIGURED_GLOBAL_SERVER FAIL_BACKUP_READ
	unset FAIL_PDNSD_READ FAIL_TUNNEL_READ FAIL_DNS_PROXY_STOP FAIL_DNS_PROXY_START
	unset MOCK_PDNSD_ENABLE MOCK_TUNNEL_FORWARD
}
expect_ressp_status() {
	expected=$1
	label=$2
	if ressp; then actual=0; else actual=$?; fi
	[ "$actual" -eq "$expected" ] || fail "$label returned $actual, expected $expected"
}
expect_dns_proxy_restart_status() {
	expected=$1
	label=$2
	if ss_restart_dns_proxy; then actual=0; else actual=$?; fi
	[ "$actual" -eq "$expected" ] || fail "$label returned $actual, expected $expected"
}
expect_status() {
	expected=$1
	label=$2
	if ssp_start; then actual=0; else actual=$?; fi
	[ "$actual" -eq "$expected" ] || fail "$label returned $actual, expected $expected"
}
called() { grep -Fqx "$1" "$CALL_LOG" || fail "missing stage: $1"; }
not_called() { if grep -Fqx "$1" "$CALL_LOG"; then fail "unexpected stage: $1"; fi; }

# Keep the first required-stage failure while allowing independent stages to run.
reset
FAIL_TCP=31
FAIL_LOCAL=32
FAIL_WATCHCAT=33
FAIL_AUTO_UPDATE=55
FAIL_DETECT=34
pppoemwan=0
expect_status 31 "TCP proxy failure remains first"
called rules
called cgroups_enabled_probe
called start_redir_tcp
not_called start_redir_udp
not_called start_dns
called start_local
called start_watchcat
called auto_update
called pppoe_detect
called "nvram:set:check_mode=0"
if grep -Fq '已启动科学上网' "$LOG_FILE"; then fail "reported success after required startup failure"; fi

# UDP failure must not stop independent DNS startup.
reset
FAIL_UDP=32
FAIL_DNS=33
SOCKS5_ENABLE=nil
expect_status 32 "UDP relay failure remains first"
called start_redir_udp
called start_dns
not_called start_local
if grep -Fq '已启动科学上网' "$LOG_FILE"; then fail "reported success after DNS failure"; fi

# Configured global-node policy failure is visible; local proxy still runs.
reset
FAIL_RULES=12
expect_status 12 "firewall policy failure propagates"
called start_local
called start_watchcat
not_called cgroups_enabled_probe
not_called start_redir_tcp
if grep -Fq '已启动科学上网' "$LOG_FILE"; then fail "reported success after firewall failure"; fi

# Disabled global and local services retain their legacy no-global status.
reset
GLOBAL_SERVER=nil
SOCKS5_ENABLE=nil
FAIL_RULES=1
expect_status 1 "disabled global service status"
not_called start_local
not_called cgroups_enabled_probe
not_called start_redir_tcp
not_called pppoe_detect

# Resource-limit setup failure must not be silently reported as success.
reset
MOCK_SS_CGROUPS=1
FAIL_CGROUP_MKDIR=30
export MOCK_SS_CGROUPS FAIL_CGROUP_MKDIR
expect_status 30 "resource-limit directory creation failure"
called cgroup_mkdir:/sys/fs/cgroup/cpu/shadowsocksr
not_called cgroup_write:/sys/fs/cgroup/cpu/shadowsocksr/cpu.shares
called start_redir_tcp
called start_dns
if grep -Fq '已启动科学上网' "$LOG_FILE"; then fail "reported success after resource-limit failure"; fi

# A failed cgroup setting write must preserve its exact result.
reset
MOCK_SS_CGROUPS=1
FAIL_CGROUP_CPU_WRITE=38
export MOCK_SS_CGROUPS FAIL_CGROUP_CPU_WRITE
expect_status 38 "CPU shares write failure"
called cgroup_write:/sys/fs/cgroup/cpu/shadowsocksr/cpu.shares
not_called cgroup_write:/sys/fs/cgroup/memory/shadowsocksr/memory.limit_in_bytes
called start_redir_tcp

# Memory limit writes and reads are also required when cgroups are enabled.
reset
MOCK_SS_CGROUPS=1
FAIL_CGROUP_MEM_WRITE=39
export MOCK_SS_CGROUPS FAIL_CGROUP_MEM_WRITE
expect_status 39 "memory limit write failure"
called cgroup_write:/sys/fs/cgroup/cpu/shadowsocksr/cpu.shares
called cgroup_write:/sys/fs/cgroup/memory/shadowsocksr/memory.limit_in_bytes
not_called cgroup_read
called start_redir_tcp

reset
MOCK_SS_CGROUPS=1
FAIL_CGROUP_READ=40
export MOCK_SS_CGROUPS FAIL_CGROUP_READ
expect_status 40 "memory limit readback failure"
called cgroup_read
called start_redir_tcp

# A DNS-only failure is visible even when the UDP relay is healthy.
reset
FAIL_DNS=35
SOCKS5_ENABLE=nil
expect_status 35 "DNS proxy failure"
called start_redir_udp
called start_dns
if grep -Fq '已启动科学上网' "$LOG_FILE"; then fail "reported success after DNS-only failure"; fi

# A local-proxy failure is required only when SOCKS5 is configured.
reset
FAIL_LOCAL=36
expect_status 36 "configured local SOCKS5 failure"
called start_local
if grep -Fq '已启动科学上网' "$LOG_FILE"; then fail "reported success after local proxy failure"; fi

# A requested watchcat failure also reaches the caller.
reset
FAIL_WATCHCAT=37
expect_status 37 "watchcat failure"
called start_watchcat
if grep -Fq '已启动科学上网' "$LOG_FILE"; then fail "reported success after watchcat failure"; fi

# Required state-write and WAN reconciliation errors must reach callers.
reset
FAIL_NVRAM_SET=41
expect_status 41 "check-mode write error"
reset
pppoemwan=0
FAIL_DETECT=42
expect_status 42 "PPPoE-MWAN reconciliation error"

# Optional cron maintenance does not turn a healthy proxy start into failure.
reset
FAIL_AUTO_UPDATE=55
expect_status 0 "optional cron failure remains non-fatal"
called auto_update
called "log:Optional Shadowsocks update scheduling failed (status=55)"
called start_local
called "log:已启动科学上网..."

# Backup-server recovery must use the selected backup node for firewall, proxy and local config.
reset
MOCK_BACKUP_SERVER=backup-node
MOCK_CONFIGURED_GLOBAL_SERVER=primary-node
MOCK_UDP_RELAY_SERVER=same
SOCKS5_ENABLE=same
expect_ressp_status 0 "healthy backup start"
called rules
called rules_for:backup-node:backup-node
called cgroups_enabled_probe
called start_redir_tcp
called tcp_server:backup-node
called start_redir_udp
called udp_server:backup-node:backup-node
called start_dns
called start_local
called local_global_server:backup-node
called start_watchcat
called auto_update
[ "$ENABLE_SERVER" = "primary-node" ] || fail "backup start overwrote configured primary selector"
[ "$GLOBAL_SERVER" = "backup-node" ] || fail "backup selector was not applied to service generation"
grep -Fq '备用服务器启动成功！' "$LOG_FILE" || fail "healthy backup start did not report success"

# Reject missing or unreadable backup selectors before firewall/proxy mutation.
reset
MOCK_BACKUP_SERVER=nil
SOCKS5_ENABLE=nil
expect_ressp_status 1 "missing backup server"
not_called rules
not_called start_redir_tcp
if grep -Fq '备用服务器启动成功！' "$LOG_FILE"; then fail "missing backup was reported as started"; fi

reset
FAIL_BACKUP_READ=74
SOCKS5_ENABLE=nil
expect_ressp_status 74 "backup NVRAM read failure"
not_called rules
not_called start_redir_tcp

# A failed TCP readiness check prevents dependent UDP/DNS startup and false success.
reset
FAIL_TCP=31
SOCKS5_ENABLE=nil
expect_ressp_status 31 "backup TCP start failure"
called rules_for:backup-node:nil
called start_redir_tcp
not_called start_redir_udp
not_called start_dns
if grep -Fq '备用服务器启动成功！' "$LOG_FILE"; then fail "backup TCP failure was reported as success"; fi

# UDP failure is returned while DNS and independent monitor/cron stages still run.
reset
FAIL_UDP=32
SOCKS5_ENABLE=nil
expect_ressp_status 32 "backup UDP start failure"
called start_dns
called start_watchcat
called auto_update
if grep -Fq '备用服务器启动成功！' "$LOG_FILE"; then fail "backup UDP failure was reported as success"; fi

# Optional cron maintenance remains best-effort on healthy backup startup.
reset
FAIL_AUTO_UPDATE=55
SOCKS5_ENABLE=nil
expect_ressp_status 0 "optional backup cron failure"
called start_redir_tcp
called start_dns
called "log:Optional backup Shadowsocks update scheduling failed (status=55)"
grep -Fq '备用服务器启动成功！' "$LOG_FILE" || fail "optional cron failure hid healthy backup startup"
# Exercise the shutdown contract with injected service and cleanup failures.
SS_RULES=mock_ss_rules
mock_ss_rules() { record "ss_rules:$1"; return "${FAIL_SS_RULES:-0}"; }
kill_process() { record kill_process; return "${FAIL_KILL_PROCESS:-0}"; }
cgroups_cleanup() { record cgroups_cleanup; return 0; }
clear_iptable() { record clear_iptable; return 0; }
ss_restart_dhcpd() { record restart_dhcpd; return "${FAIL_RESTART_DHCPD:-0}"; }
ss_run_detect() { record detect; return "${FAIL_DETECT_SCRIPT:-0}"; }
ss_dnsmasq_remove_exact_directive() {
	record "dnsmasq_remove:$2"
	return "${FAIL_DNSMASQ_CLEANUP:-0}"
}
rm() {
	if [ "$1" = "-rf" ] && [ "$2" = "/tmp/cdn" ]; then
		record rm_cdn
		return 0
	fi
	command rm "$@"
}
close_reset() {
	: > "$CALL_LOG"
	: > "$LOG_FILE"
	pppoemwan=0
	unset FAIL_SS_RULES FAIL_KILL_PROCESS FAIL_RESTART_DHCPD FAIL_DETECT_SCRIPT FAIL_DNSMASQ_CLEANUP
}
close_expect_status() {
	expected=$1
	label=$2
	if ssp_close; then actual=0; else actual=$?; fi
	[ "$actual" -eq "$expected" ] || fail "$label returned $actual, expected $expected"
}
close_called() { grep -Fqx "$1" "$CALL_LOG" || fail "shutdown missed stage: $1"; }
close_not_called() { if grep -Fqx "$1" "$CALL_LOG"; then fail "shutdown unexpectedly ran stage: $1"; fi; }

close_reset
close_expect_status 0 "healthy shutdown"
close_called kill_process
close_called 'ss_rules:-f'
close_called cgroups_cleanup
close_called clear_iptable
close_called 'dnsmasq_remove:no-resolv'
close_called 'dnsmasq_remove:conf-dir=/tmp/dnsmasq.dom/'
close_called restart_dhcpd
close_called detect

close_reset
FAIL_RESTART_DHCPD=73
close_expect_status 73 "DNSMasq restart failure propagation"
close_called restart_dhcpd
close_called detect
grep -Fq 'DNSMasq service restart after Shadowsocks shutdown failed (status=73)' "$LOG_FILE" || fail "DNSMasq restart failure was not logged"

close_reset
FAIL_DNSMASQ_CLEANUP=74
FAIL_RESTART_DHCPD=75
close_expect_status 74 "first shutdown error preservation"
close_called restart_dhcpd
close_called detect

close_reset
FAIL_KILL_PROCESS=76
close_expect_status 1 "proxy stop failure aborts shutdown"
close_not_called rm_cdn
close_not_called restart_dhcpd
close_not_called detect

echo "Shadowsocks startup/shutdown status regression: PASS"
 
# Profile 2 (oversea mode) must atomically replace legacy include directives,
# validate the staged file, preserve unrelated options and restart DNSMasq once.
. "$DNSMASQ_HELPER"
DNSMASQ_BIN="$TMP_DIR/dnsmasq-test"
DNSMASQ_CHECK_LOG="$TMP_DIR/dnsmasq-check.log"
export DNSMASQ_CHECK_LOG
cat > "$DNSMASQ_BIN" <<'DNSMASQ_STUB'
#!/bin/sh
[ "$1" = "--test" ] && [ "$2" = "-C" ] || exit 90
printf '%s\n' "$3" >> "$DNSMASQ_CHECK_LOG"
[ "${FAIL_DNSMASQ_TEST:-0}" -eq 0 ] || exit "$FAIL_DNSMASQ_TEST"
[ "$(grep -Fxc 'conf-dir=/etc/storage/dnsmasq.oversea' "$3")" -eq 1 ] || exit 91
exit 0
DNSMASQ_STUB
chmod 755 "$DNSMASQ_BIN"

PROFILE_CONFIG="$TMP_DIR/dnsmasq-oversea.conf"
PROFILE_ORIGINAL="$TMP_DIR/dnsmasq-oversea.original"
write_oversea_config() {
	cat > "$1" <<'DNSMASQ_CONFIG'
interface=br0
conf-dir=/etc/storage/dnsmasq-ss.d
conf-dir=/etc/storage/dnsmasq-ss.d/
conf-dir=/etc/storage/dnsmasq.oversea
conf-dir=/etc/storage/dnsmasq.oversea/
conf-dir=/etc/storage/dnsmasq.oversea-custom
min-cache-ttl=73
dns-forward-max=41
DNSMASQ_CONFIG
}
profile_expect_status() {
	expected=$1
	if ss_publish_and_restart_dnsmasq "$PROFILE_CONFIG" "$DNSMASQ_BIN" 2; then
		actual=0
	else
		actual=$?
	fi
	[ "$actual" -eq "$expected" ] || fail "oversea DNSMasq profile returned $actual, expected $expected"
}

# Successful publication removes all old exact directives but preserves
# similarly named includes and existing cache values.
: > "$CALL_LOG"
: > "$DNSMASQ_CHECK_LOG"
write_oversea_config "$PROFILE_CONFIG"
cp -p "$PROFILE_CONFIG" "$PROFILE_ORIGINAL"
unset FAIL_DNSMASQ_TEST FAIL_RESTART_DHCPD
profile_expect_status 0
[ "$(grep -Fxc 'conf-dir=/etc/storage/dnsmasq.oversea' "$PROFILE_CONFIG")" -eq 1 ] || fail "oversea include was not published exactly once"
[ "$(grep -Fxc 'conf-dir=/etc/storage/dnsmasq-ss.d' "$PROFILE_CONFIG" || true)" -eq 0 ] || fail "legacy Shadowsocks include remained"
[ "$(grep -Fxc 'conf-dir=/etc/storage/dnsmasq-ss.d/' "$PROFILE_CONFIG" || true)" -eq 0 ] || fail "legacy trailing-slash Shadowsocks include remained"
[ "$(grep -Fxc 'conf-dir=/etc/storage/dnsmasq.oversea/' "$PROFILE_CONFIG" || true)" -eq 0 ] || fail "legacy trailing-slash oversea include remained"
grep -Fqx 'conf-dir=/etc/storage/dnsmasq.oversea-custom' "$PROFILE_CONFIG" || fail "near-match include was modified"
grep -Fqx 'min-cache-ttl=73' "$PROFILE_CONFIG" || fail "oversea profile changed min-cache-ttl"
grep -Fqx 'dns-forward-max=41' "$PROFILE_CONFIG" || fail "oversea profile changed dns-forward-max"
[ "$(grep -Fxc restart_dhcpd "$CALL_LOG")" -eq 1 ] || fail "oversea publication restarted DNSMasq more than once"
[ "$(wc -l < "$DNSMASQ_CHECK_LOG" | tr -d ' ')" -eq 1 ] || fail "oversea staged config was not validated exactly once"

# A failed staged validation must leave the live config untouched and skip restart.
: > "$CALL_LOG"
: > "$DNSMASQ_CHECK_LOG"
write_oversea_config "$PROFILE_CONFIG"
cp -p "$PROFILE_CONFIG" "$PROFILE_ORIGINAL"
FAIL_DNSMASQ_TEST=27
export FAIL_DNSMASQ_TEST
profile_expect_status 1
cmp -s "$PROFILE_CONFIG" "$PROFILE_ORIGINAL" || fail "oversea validation failure changed live config"
[ "$(grep -Fc restart_dhcpd "$CALL_LOG" || true)" -eq 0 ] || fail "oversea validation failure restarted DNSMasq"
unset FAIL_DNSMASQ_TEST

# A restart failure restores the complete previous file, attempts rollback
# restart and preserves the original service-restart error.
: > "$CALL_LOG"
: > "$DNSMASQ_CHECK_LOG"
write_oversea_config "$PROFILE_CONFIG"
cp -p "$PROFILE_CONFIG" "$PROFILE_ORIGINAL"
FAIL_RESTART_DHCPD=43
profile_expect_status 43
cmp -s "$PROFILE_CONFIG" "$PROFILE_ORIGINAL" || fail "oversea restart failure did not restore original config"
[ "$(grep -Fxc restart_dhcpd "$CALL_LOG")" -eq 2 ] || fail "oversea rollback did not attempt exactly one recovery restart"
unset FAIL_RESTART_DHCPD

echo "Shadowsocks DNSMasq profile-2 atomic publication regression: PASS"
# Invalid configuration must never stop the currently working DNS proxy.
reset
FAIL_PDNSD_READ=73
expect_dns_proxy_restart_status 73 "DNS proxy selector read failure"
not_called stop_dns_proxy
not_called start_dns_proxy

reset
FAIL_TUNNEL_READ=74
expect_dns_proxy_restart_status 74 "DNS upstream read failure"
not_called stop_dns_proxy
not_called start_dns_proxy

reset
MOCK_TUNNEL_FORWARD=nil
expect_dns_proxy_restart_status 1 "empty DNS upstream"
not_called stop_dns_proxy
not_called start_dns_proxy

reset
MOCK_PDNSD_ENABLE=2
expect_dns_proxy_restart_status 1 "unsupported DNS proxy selector"
not_called stop_dns_proxy
not_called start_dns_proxy

reset
MOCK_PDNSD_ENABLE=1
MOCK_TUNNEL_FORWARD=9.9.9.9#53
expect_dns_proxy_restart_status 0 "valid dns2tcp restart"
called get:pdnsd_enable
called get:tunnel_forward
called stop_dns_proxy
called start_dns_proxy:dns2tcp:9.9.9.9
selector_line=$(grep -n "^get:pdnsd_enable$" "$CALL_LOG" | head -n 1 | cut -d: -f1)
upstream_line=$(grep -n "^get:tunnel_forward$" "$CALL_LOG" | head -n 1 | cut -d: -f1)
stop_line=$(grep -n "^stop_dns_proxy$" "$CALL_LOG" | head -n 1 | cut -d: -f1)
[ "$selector_line" -lt "$stop_line" ] || fail "DNS selector read happened after proxy shutdown"
[ "$upstream_line" -lt "$stop_line" ] || fail "DNS upstream read happened after proxy shutdown"

reset
FAIL_DNS_PROXY_STOP=26
expect_dns_proxy_restart_status 26 "DNS proxy stop failure"
not_called start_dns_proxy

reset
FAIL_DNS_PROXY_START=31
expect_dns_proxy_restart_status 31 "DNS proxy start failure"
called stop_dns_proxy
called start_dns_proxy:dnsproxy:8.8.8.8

echo "Shadowsocks DNS proxy preflight regression: PASS"
