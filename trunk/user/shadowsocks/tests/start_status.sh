#!/bin/sh
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
SCRIPT="$HERE/../scripts/shadowsocks.sh"
TMP_DIR=$(mktemp -d)
CALL_LOG="$TMP_DIR/calls.log"
LOG_FILE="$TMP_DIR/ss.log"
trap 'rm -rf "$TMP_DIR"' EXIT HUP INT TERM

sed -n \
	-e '/^ss_cgroup_mkdir() {/,/^}/p' \
	-e '/^ss_cgroup_write_value() {/,/^}/p' \
	-e '/^ss_cgroup_read_value() {/,/^}/p' \
	-e '/^cgroups_init() {/,/^}/p' \
	-e '/^rules() {/,/^}/p' \
	-e '/^ssp_record_start_failure() {/,/^}/p' \
	-e '/^ssp_run_pppoe_detect() {/,/^}/p' \
	-e '/^ssp_start() {/,/^}/p' \
	"$SCRIPT" > "$TMP_DIR/start-functions.sh"
. "$TMP_DIR/start-functions.sh"

fail() { echo "Shadowsocks startup status regression: FAIL: $*" >&2; exit 1; }
record() { printf '%s\n' "$*" >> "$CALL_LOG"; }
log() { record "log:$*"; printf '%s\n' "$*" >> "$LOG_FILE"; }

nvram() {
	case "$1:$2" in
		get:ss_enable) echo 1 ;;
		get:ss_cgroups) echo "${MOCK_SS_CGROUPS:-0}" ;;
		get:ss_cgoups_cpu_s) echo "${MOCK_CPU_LIMIT:-128}" ;;
		get:ss_cgoups_mem_s) echo "${MOCK_MEM_LIMIT:-1048576}" ;;
		get:global_server) echo "$GLOBAL_SERVER" ;;
		get:socks5_enable) echo "$SOCKS5_ENABLE" ;;
		get:udp_relay_server) echo nil ;;
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
start_rules() { record rules; return "${FAIL_RULES:-0}"; }
start_redir_tcp() { record start_redir_tcp; return "${FAIL_TCP:-0}"; }
start_redir_udp() { record start_redir_udp; return "${FAIL_UDP:-0}"; }
start_dns() { record start_dns; return "${FAIL_DNS:-0}"; }
start_local() { record start_local; return "${FAIL_LOCAL:-0}"; }
start_watchcat() { record start_watchcat; return "${FAIL_WATCHCAT:-0}"; }
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
	unset FAIL_RULES FAIL_CGROUPS FAIL_TCP FAIL_UDP FAIL_DNS FAIL_LOCAL
	unset FAIL_WATCHCAT FAIL_AUTO_UPDATE FAIL_NVRAM_SET FAIL_DETECT
	unset FAIL_CGROUP_MKDIR FAIL_CGROUP_CPU_WRITE FAIL_CGROUP_MEM_WRITE FAIL_CGROUP_READ
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
called cgroups_init
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
not_called cgroups_init
not_called start_redir_tcp
if grep -Fq '已启动科学上网' "$LOG_FILE"; then fail "reported success after firewall failure"; fi

# Disabled global and local services retain their legacy no-global status.
reset
GLOBAL_SERVER=nil
SOCKS5_ENABLE=nil
FAIL_RULES=1
expect_status 1 "disabled global service status"
not_called start_local
not_called cgroups_init
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

echo "Shadowsocks required-stage startup status regression: PASS"
