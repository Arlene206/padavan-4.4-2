#!/bin/sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname "$0")/../../../.." && pwd)
SCRIPT="$ROOT/trunk/user/zerotier/zerotier.sh"
TEST_TMP=$(mktemp -d "${TMPDIR:-/tmp}/zerotier-lifecycle.XXXXXX")
TEST_STATE="$TEST_TMP/process.state"
TEST_KILL_LOG="$TEST_TMP/killall.log"
TEST_SLEEP_LOG="$TEST_TMP/sleep.log"
TEST_ACTION_LOG="$TEST_TMP/actions.log"
TEST_IPTABLES_LOG="$TEST_TMP/iptables.log"
TEST_IFCONFIG_COUNT="$TEST_TMP/ifconfig.count"
TEST_ROUTE_COUNT="$TEST_TMP/route.count"
TEST_IFCONFIG_MODE=none
TEST_ROUTE_MODE=none
TEST_NAT_ENABLE=0
TEST_START_INSTANCE_STATUS=0
TEST_STATIC_ROUTE_COUNT=0
TEST_STATIC_ROUTE_ENABLE=0
TEST_STATIC_ROUTE_IP=192.0.2.0/24
TEST_STATIC_ROUTE_GATEWAY=192.0.2.1
TEST_ROUTE_ADD_STATUS=0
TEST_ROUTE_PRESENT=0
TEST_ROUTE_SHOW_STATUS=0
TEST_ROUTE_DEL_STATUS=0
TEST_ROUTE_DELETE_PERSISTS=0
TEST_ROUTE_ADD_PARTIAL=0
: > "$TEST_IFCONFIG_COUNT"
: > "$TEST_ROUTE_COUNT"
export TEST_STATE TEST_KILL_LOG TEST_SLEEP_LOG TEST_ACTION_LOG TEST_TMP
export TEST_IPTABLES_LOG TEST_IFCONFIG_COUNT TEST_ROUTE_COUNT
export TEST_IFCONFIG_MODE TEST_ROUTE_MODE TEST_NAT_ENABLE
export TEST_START_INSTANCE_STATUS
trap 'rm -rf "$TEST_TMP"' EXIT HUP INT TERM

fail() {
	echo "ZeroTier lifecycle regression: FAIL: $*" >&2
	exit 1
}

# Source the production functions without running a service action.
set -- __test__
. "$SCRIPT"

pidof() {
	[ "$1" = zerotier-one ] || return 1
	case "${TEST_PIDOF_MODE:-}" in
		error) return 2 ;;
		malformed) echo invalid; return 0 ;;
		empty-success) return 0 ;;
	esac
	case "$(cat "$TEST_STATE")" in
		running)
			echo 4321
			return 0
			;;
		unknown)
			return 2
			;;
	esac
	return 1
}

killall() {
	printf '%s\n' "$*" >> "$TEST_KILL_LOG"
	case "$TEST_MODE:$*" in
		graceful:zerotier-one|-9-exits:-9\ zerotier-one)
			echo stopped > "$TEST_STATE"
			;;
		unknown-after-term:zerotier-one)
			echo unknown > "$TEST_STATE"
			;;
		*)
			;;
	esac
	return 0
}

sleep() {
	printf '%s\n' "$*" >> "$TEST_SLEEP_LOG"
	return 0
}

logger() {
	return 0
}

# These hooks let the regression prove the stop-failure gate without touching
# host firewall state or the developer's persistent filesystem.
start_instance() {
	echo start >> "$TEST_ACTION_LOG"
	return "$TEST_START_INSTANCE_STATUS"
}

del_rules() {
	if [ -z "${zt0:-}" ]; then
		zt0="$(zerotier_interface_name)"
	fi
	[ "$zt0" = ztunit ] || fail "firewall cleanup lost the interface after daemon exit"
	echo del_rules >> "$TEST_ACTION_LOG"
	return 0
}

rm() {
	printf 'rm:%s\n' "$*" >> "$TEST_ACTION_LOG"
	/bin/rm "$@"
}

kill() {
	echo "$*" >> "$TEST_TMP/numeric-kill.log"
	return 1
}

ifconfig() {
	case "$TEST_IFCONFIG_MODE" in
		running-only)
			case "$(cat "$TEST_STATE")" in
				running) printf '%s\n' "ztunit: Link encap:Ethernet" ;;
				*) printf '%s\n' "eth0 Link encap:Ethernet" ;;
			esac
			;;
		ready)
			printf '%s\n' "eth0 Link encap:Ethernet" "ztunit: Link encap:Ethernet"
			;;
		delayed)
			count=$(cat "$TEST_IFCONFIG_COUNT")
			count=$((count + 1))
			printf '%s\n' "$count" > "$TEST_IFCONFIG_COUNT"
			printf '%s\n' "eth0 Link encap:Ethernet"
			if [ "$count" -ge 3 ]; then
				printf '%s\n' "ztunit: Link encap:Ethernet"
			fi
			;;
		*)
			;;
	esac
	return 0
}

ip() {
	if [ "$1" = route ] && [ "${2:-}" = add ]; then
		echo "zero_route:add" >> "$TEST_ACTION_LOG"
		if [ "$TEST_ROUTE_ADD_STATUS" -eq 0 ] || [ "$TEST_ROUTE_ADD_PARTIAL" -eq 1 ]; then
			TEST_ROUTE_PRESENT=1
		fi
		return "$TEST_ROUTE_ADD_STATUS"
	fi
	if [ "$1" = route ] && [ "${2:-}" = show ]; then
		[ "$TEST_ROUTE_SHOW_STATUS" -eq 0 ] || return "$TEST_ROUTE_SHOW_STATUS"
		if [ "$TEST_ROUTE_PRESENT" -eq 1 ] && [ "${3:-}" = "$TEST_STATIC_ROUTE_IP" ]; then
			printf '%s\n' "$TEST_STATIC_ROUTE_IP via $TEST_STATIC_ROUTE_GATEWAY dev ztunit"
		fi
		return 0
	fi
	if [ "$1" = route ] && [ "${2:-}" = del ]; then
		[ "$7" = ztunit ] || fail "static route cleanup lost the saved interface after daemon exit"
		echo "zero_route:del" >> "$TEST_ACTION_LOG"
		[ "$TEST_ROUTE_DEL_STATUS" -eq 0 ] || return "$TEST_ROUTE_DEL_STATUS"
		if [ "$TEST_ROUTE_DELETE_PERSISTS" -eq 0 ]; then
			TEST_ROUTE_PRESENT=0
		fi
		return 0
	fi
	[ "$1" = route ] || return 0
	count=$(cat "$TEST_ROUTE_COUNT")
	count=$((count + 1))
	printf '%s\n' "$count" > "$TEST_ROUTE_COUNT"
	case "$TEST_ROUTE_MODE" in
		ready)
			printf '%s\n' \
				"10.9.0.0/24 dev ztother proto kernel scope link" \
				"10.147.17.0/24 dev ztunit proto kernel scope link"
			;;
		delayed)
			if [ "$count" -ge 3 ]; then
				printf '%s\n' \
					"10.9.0.0/24 dev ztother proto kernel scope link" \
					"10.147.17.0/24 dev ztunit proto kernel scope link"
			fi
			;;
		*)
			;;
	esac
	return 0
}

nvram() {
	if [ "$1" = get ]; then
		case "$2" in
			zerotier_nat) printf '%s\n' "$TEST_NAT_ENABLE" ;;
			zero_staticnum_x) printf '%s\n' "$TEST_STATIC_ROUTE_COUNT" ;;
			zero_enable_x0) printf '%s\n' "$TEST_STATIC_ROUTE_ENABLE" ;;
			zero_ip_x0) printf '%s\n' "$TEST_STATIC_ROUTE_IP" ;;
			zero_route_x0) printf '%s\n' "$TEST_STATIC_ROUTE_GATEWAY" ;;
		esac
	fi
	return 0
}

iptables() {
	printf '%s\n' "$*" >> "$TEST_IPTABLES_LOG"
	if [ -n "${TEST_IPTABLES_FAIL_ON:-}" ] && [ "$*" = "$TEST_IPTABLES_FAIL_ON" ]; then
		return 1
	fi
	return 0
}

: > "$TEST_KILL_LOG"
: > "$TEST_SLEEP_LOG"
: > "$TEST_ACTION_LOG"
echo stopped > "$TEST_STATE"
TEST_MODE=none
export TEST_MODE
kill_z || fail "no-process stop returned failure"
[ ! -s "$TEST_KILL_LOG" ] || fail "stop signalled a process when pidof found none"
[ ! -s "$TEST_SLEEP_LOG" ] || fail "no-process stop waited unnecessarily"

# pidof errors, empty-success output and malformed PID lists are UNKNOWN, not ABSENT.
for TEST_PIDOF_MODE in error malformed empty-success; do
	: > "$TEST_KILL_LOG"
	: > "$TEST_SLEEP_LOG"
	echo running > "$TEST_STATE"
	if kill_z; then
		fail "stop accepted unknown process inventory mode $TEST_PIDOF_MODE"
	fi
	[ ! -s "$TEST_KILL_LOG" ] || fail "stop signalled a process after unknown inventory mode $TEST_PIDOF_MODE"
	[ ! -s "$TEST_SLEEP_LOG" ] || fail "stop waited after unknown inventory mode $TEST_PIDOF_MODE"
done
TEST_PIDOF_MODE=""

# A transition to UNKNOWN after TERM must not be mistaken for process exit or
# trigger a name-wide SIGKILL escalation.
: > "$TEST_KILL_LOG"
: > "$TEST_SLEEP_LOG"
echo running > "$TEST_STATE"
TEST_MODE=unknown-after-term
if kill_z; then
	fail "stop accepted an unknown process state after TERM"
fi
[ "$(cat "$TEST_KILL_LOG")" = zerotier-one ] || fail "stop escalated after process inventory became unknown"
[ ! -s "$TEST_SLEEP_LOG" ] || fail "stop waited after process inventory became unknown"


: > "$TEST_KILL_LOG"
: > "$TEST_SLEEP_LOG"
echo running > "$TEST_STATE"
TEST_MODE=graceful
if ! kill_z; then
	fail "graceful TERM stop returned failure"
fi
[ "$(cat "$TEST_KILL_LOG")" = zerotier-one ] || fail "graceful stop did not send TERM only"
[ ! -s "$TEST_SLEEP_LOG" ] || fail "graceful exit was not observed immediately"
[ "$(cat "$TEST_STATE")" = stopped ] || fail "graceful stop postcondition missing"

: > "$TEST_KILL_LOG"
: > "$TEST_SLEEP_LOG"
echo running > "$TEST_STATE"
TEST_MODE=-9-exits
if ! kill_z; then
	fail "TERM-resistant daemon did not stop after name-based escalation"
fi
[ "$(cat "$TEST_KILL_LOG")" = "$(printf 'zerotier-one\n-9 zerotier-one')" ] || fail "force stop did not re-enumerate by process name"
[ "$(wc -l < "$TEST_SLEEP_LOG" | tr -d ' ')" = 5 ] || fail "TERM grace period was not bounded to five one-second polls"
[ "$(cat "$TEST_STATE")" = stopped ] || fail "force-stop postcondition missing"

: > "$TEST_KILL_LOG"
: > "$TEST_SLEEP_LOG"
echo running > "$TEST_STATE"
TEST_MODE=stuck
if kill_z; then
	fail "stop reported success while daemon remained after SIGKILL"
fi
[ "$(cat "$TEST_KILL_LOG")" = "$(printf 'zerotier-one\n-9 zerotier-one')" ] || fail "stuck daemon did not receive TERM then name-based KILL"
[ "$(wc -l < "$TEST_SLEEP_LOG" | tr -d ' ')" = 10 ] || fail "TERM/KILL waits were not bounded to five polls each"

: > "$TEST_KILL_LOG"
: > "$TEST_SLEEP_LOG"
: > "$TEST_ACTION_LOG"
echo running > "$TEST_STATE"
TEST_MODE=stuck
if start_zero; then
	fail "start proceeded when the prior daemon could not be stopped"
fi
[ ! -s "$TEST_ACTION_LOG" ] || fail "start reached start_instance after stop failure"

# A failed pidof scan must also block restart rather than start a duplicate.
: > "$TEST_KILL_LOG"
: > "$TEST_ACTION_LOG"
echo running > "$TEST_STATE"
TEST_MODE=none
TEST_PIDOF_MODE=error
if start_zero; then
	fail "start proceeded while the process inventory was unknown"
fi
[ ! -s "$TEST_KILL_LOG" ] || fail "start signalled after an initial process inventory error"
[ ! -s "$TEST_ACTION_LOG" ] || fail "start reached start_instance after an initial process inventory error"
TEST_PIDOF_MODE=""

# A configured launcher must preserve the start_instance failure status.
: > "$TEST_ACTION_LOG"
echo stopped > "$TEST_STATE"
TEST_MODE=none
TEST_START_INSTANCE_STATUS=23
if start_zero; then
	fail "start reported success after start_instance failed"
fi
[ "$(cat "$TEST_ACTION_LOG")" = "start" ] || fail "start did not reach the launcher failure hook"
TEST_START_INSTANCE_STATUS=0


: > "$TEST_KILL_LOG"
: > "$TEST_SLEEP_LOG"
: > "$TEST_ACTION_LOG"
TEST_CONFIG="$TEST_TMP/config"
config_path="$TEST_CONFIG"
mkdir -p "$TEST_CONFIG"
echo identity > "$TEST_CONFIG/identity.secret"
TEST_STATIC_ROUTE_COUNT=1
TEST_STATIC_ROUTE_ENABLE=1
echo running > "$TEST_STATE"
TEST_MODE=stuck
if stop_zero; then
	fail "stop reported success while the daemon remained alive"
fi
[ -f "$TEST_CONFIG/identity.secret" ] || fail "failed stop deleted the persistent ZeroTier identity"
[ ! -s "$TEST_ACTION_LOG" ] || fail "failed stop changed routes, firewall rules or persistent config"

# An unreadable process inventory must preserve identity and all live state.
: > "$TEST_KILL_LOG"
: > "$TEST_SLEEP_LOG"
: > "$TEST_ACTION_LOG"
echo running > "$TEST_STATE"
TEST_MODE=none
TEST_PIDOF_MODE=error
if stop_zero; then
	fail "stop accepted an unknown process inventory"
fi
[ -f "$TEST_CONFIG/identity.secret" ] || fail "unknown process state deleted the persistent ZeroTier identity"
[ ! -s "$TEST_KILL_LOG" ] || fail "unknown process state triggered killall"
[ ! -s "$TEST_ACTION_LOG" ] || fail "unknown process state changed routes, firewall rules or persistent config"
TEST_PIDOF_MODE=""


: > "$TEST_KILL_LOG"
: > "$TEST_SLEEP_LOG"
: > "$TEST_ACTION_LOG"
echo running > "$TEST_STATE"
TEST_IFCONFIG_MODE=running-only
TEST_ROUTE_PRESENT=1
TEST_ROUTE_SHOW_STATUS=0
TEST_ROUTE_DEL_STATUS=0
TEST_ROUTE_DELETE_PERSISTS=0
TEST_MODE=graceful
if ! stop_zero; then
	fail "stop failed after a confirmed graceful daemon exit"
fi
[ ! -e "$TEST_CONFIG" ] || fail "successful stop did not complete the existing config cleanup"
[ "$(cat "$TEST_ACTION_LOG")" = "$(printf 'del_rules\nzero_route:del\nrm:-rf %s' "$TEST_CONFIG")" ] || fail "successful cleanup did not run after daemon absence"
# An already-absent configured route is an idempotent success.
TEST_STATIC_ROUTE_COUNT=1
TEST_STATIC_ROUTE_ENABLE=1
TEST_IFCONFIG_MODE=running-only
TEST_ROUTE_PRESENT=0
TEST_ROUTE_SHOW_STATUS=0
TEST_ROUTE_DEL_STATUS=0
TEST_ROUTE_DELETE_PERSISTS=0
: > "$TEST_ACTION_LOG"
mkdir -p "$TEST_CONFIG"
echo identity > "$TEST_CONFIG/identity.secret"
echo running > "$TEST_STATE"
TEST_MODE=graceful
if ! stop_zero; then
	fail "stop rejected a configured route that was already absent"
fi
[ ! -e "$TEST_CONFIG" ] || fail "stop retained config after confirming the route was already absent"
if grep -Fqx -- "zero_route:del" "$TEST_ACTION_LOG"; then
	fail "stop issued a redundant delete for an absent route"
fi

# Route cleanup failure, an unreadable route inventory and a false-success
# delete must all preserve the ZeroTier identity for a later recovery attempt.
TEST_ROUTE_PRESENT=1
TEST_ROUTE_DEL_STATUS=1
TEST_ROUTE_SHOW_STATUS=0
TEST_ROUTE_DELETE_PERSISTS=0
: > "$TEST_ACTION_LOG"
mkdir -p "$TEST_CONFIG"
echo identity > "$TEST_CONFIG/identity.secret"
echo running > "$TEST_STATE"
TEST_MODE=graceful
if stop_zero; then
	fail "stop accepted a failed static route deletion"
fi
[ -f "$TEST_CONFIG/identity.secret" ] || fail "failed route deletion removed the persistent identity"
grep -Fqx -- "zero_route:del" "$TEST_ACTION_LOG" || fail "route deletion failure fixture never attempted deletion"
if grep -Fqx -- "rm:-rf $TEST_CONFIG" "$TEST_ACTION_LOG"; then
	fail "stop removed persistent configuration after route deletion failed"
fi
[ "$TEST_ROUTE_PRESENT" -eq 1 ] || fail "failed route deletion unexpectedly cleared the route fixture"

: > "$TEST_ACTION_LOG"
TEST_ROUTE_DEL_STATUS=0
TEST_ROUTE_SHOW_STATUS=1
echo running > "$TEST_STATE"
if stop_zero; then
	fail "stop accepted an unreadable static route inventory"
fi
[ -f "$TEST_CONFIG/identity.secret" ] || fail "unreadable route inventory removed the persistent identity"
if grep -Fqx -- "zero_route:del" "$TEST_ACTION_LOG"; then
	fail "stop attempted a route deletion without a readable pre-delete inventory"
fi
if grep -Fqx -- "rm:-rf $TEST_CONFIG" "$TEST_ACTION_LOG"; then
	fail "stop removed persistent configuration with route state unknown"
fi

: > "$TEST_ACTION_LOG"
TEST_ROUTE_SHOW_STATUS=0
TEST_ROUTE_DELETE_PERSISTS=1
echo running > "$TEST_STATE"
if stop_zero; then
	fail "stop accepted a route that remained after a successful delete command"
fi
[ -f "$TEST_CONFIG/identity.secret" ] || fail "post-delete route verification failure removed the persistent identity"
grep -Fqx -- "zero_route:del" "$TEST_ACTION_LOG" || fail "post-delete verification fixture never attempted deletion"
if grep -Fqx -- "rm:-rf $TEST_CONFIG" "$TEST_ACTION_LOG"; then
	fail "stop removed persistent configuration while the route remained"
fi

TEST_STATIC_ROUTE_COUNT=0
TEST_STATIC_ROUTE_ENABLE=0
TEST_IFCONFIG_MODE=none
TEST_ROUTE_PRESENT=0
TEST_ROUTE_SHOW_STATUS=0
TEST_ROUTE_DEL_STATUS=0
TEST_ROUTE_DELETE_PERSISTS=0

[ ! -s "$TEST_TMP/numeric-kill.log" ] || fail "a stale numeric PID was signalled"

# interface readiness ignores unrelated interfaces and succeeds after bounded polling.
: > "$TEST_SLEEP_LOG"
: > "$TEST_IPTABLES_LOG"
echo 0 > "$TEST_IFCONFIG_COUNT"
TEST_IFCONFIG_MODE=delayed
TEST_ROUTE_MODE=none
TEST_NAT_ENABLE=0
if ! rules; then
	fail "rules rejected an interface that appeared before the deadline"
fi
[ "$(wc -l < "$TEST_SLEEP_LOG" | tr -d ' ')" = 2 ] || fail "interface readiness did not poll the expected two seconds"
grep -Fqx -- "-A INPUT -i ztunit -j ACCEPT" "$TEST_IPTABLES_LOG" || fail "rules selected the wrong interface"

# A rejected filter-rule append must fail and roll back earlier rules.
: > "$TEST_SLEEP_LOG"
: > "$TEST_IPTABLES_LOG"
: > "$TEST_ACTION_LOG"
TEST_IFCONFIG_MODE=ready
TEST_NAT_ENABLE=0
TEST_IPTABLES_FAIL_ON="-A FORWARD -i ztunit -o ztunit -j ACCEPT"
if rules; then
	fail "rules reported success after a filter rule append failed"
fi
grep -Fqx -- "-A INPUT -i ztunit -j ACCEPT" "$TEST_IPTABLES_LOG" || fail "filter failure fixture did not install the first rule"
grep -Fqx -- "$TEST_IPTABLES_FAIL_ON" "$TEST_IPTABLES_LOG" || fail "filter failure fixture did not reach the rejected rule"
if grep -Fqx -- "-A FORWARD -i ztunit -j ACCEPT" "$TEST_IPTABLES_LOG"; then
	fail "rules continued publishing after a filter append failed"
fi
[ "$(cat "$TEST_ACTION_LOG")" = "$(printf 'del_rules\ndel_rules')" ] || fail "filter append failure did not roll back previously published rules"

# NAT publication has the same failure gate after the filter rules and first
# NAT rule have succeeded.
: > "$TEST_SLEEP_LOG"
: > "$TEST_IPTABLES_LOG"
: > "$TEST_ACTION_LOG"
TEST_ROUTE_MODE=ready
TEST_NAT_ENABLE=1
TEST_IPTABLES_FAIL_ON="-t nat -A POSTROUTING -s 10.147.17.0/24 -j MASQUERADE"
if rules; then
	fail "rules reported success after a NAT rule append failed"
fi
grep -Fqx -- "-A FORWARD -i ztunit -j ACCEPT" "$TEST_IPTABLES_LOG" || fail "NAT failure fixture did not install filter rules"
grep -Fqx -- "-t nat -A POSTROUTING -o ztunit -j MASQUERADE" "$TEST_IPTABLES_LOG" || fail "NAT failure fixture did not install the first NAT rule"
grep -Fqx -- "$TEST_IPTABLES_FAIL_ON" "$TEST_IPTABLES_LOG" || fail "NAT failure fixture did not reach the rejected rule"
[ "$(cat "$TEST_ACTION_LOG")" = "$(printf 'del_rules\ndel_rules')" ] || fail "NAT append failure did not roll back partially published rules"
TEST_IPTABLES_FAIL_ON=""
TEST_NAT_ENABLE=0
TEST_ROUTE_MODE=none

# Missing interface is bounded and must not publish partial firewall rules.
: > "$TEST_SLEEP_LOG"
: > "$TEST_IPTABLES_LOG"
TEST_IFCONFIG_MODE=none
if rules; then
	fail "rules succeeded without a ZeroTier interface"
fi
[ "$(wc -l < "$TEST_SLEEP_LOG" | tr -d ' ')" = 30 ] || fail "interface wait was not bounded to 30 polls"
[ ! -s "$TEST_IPTABLES_LOG" ] || fail "firewall rules were changed before interface readiness"

# single-space iproute2 output and another interface's route must be handled safely.
: > "$TEST_SLEEP_LOG"
: > "$TEST_IPTABLES_LOG"
echo 0 > "$TEST_IFCONFIG_COUNT"
echo 0 > "$TEST_ROUTE_COUNT"
TEST_IFCONFIG_MODE=ready
TEST_ROUTE_MODE=delayed
TEST_NAT_ENABLE=1
if ! rules; then
	fail "NAT rules rejected a kernel route that appeared before the deadline"
fi
[ "$(wc -l < "$TEST_SLEEP_LOG" | tr -d ' ')" = 2 ] || fail "route readiness did not poll the expected two seconds"
grep -Fqx -- "-t nat -A POSTROUTING -s 10.147.17.0/24 -j MASQUERADE" "$TEST_IPTABLES_LOG" || fail "kernel route parser selected an unrelated or missing network"

# A failed configured static route must fail readiness publication and
# remove the firewall rules which were already installed.
: > "$TEST_SLEEP_LOG"
: > "$TEST_IPTABLES_LOG"
: > "$TEST_ACTION_LOG"
echo 0 > "$TEST_ROUTE_COUNT"
TEST_ROUTE_MODE=ready
TEST_IFCONFIG_MODE=ready
TEST_NAT_ENABLE=1
TEST_STATIC_ROUTE_COUNT=1
TEST_STATIC_ROUTE_ENABLE=1
TEST_ROUTE_ADD_STATUS=1
TEST_ROUTE_ADD_PARTIAL=1
TEST_ROUTE_PRESENT=0
if rules; then
	fail "rules reported success after a configured static route add failed"
fi
grep -Fqx -- "zero_route:add" "$TEST_ACTION_LOG" || fail "static route failure fixture did not attempt route installation"
[ "$(cat "$TEST_ACTION_LOG")" = "$(printf 'del_rules\nzero_route:add\ndel_rules\nzero_route:del')" ] || fail "static route failure did not roll back firewall and partial route state"
[ "$(grep -cE '(^| )-A ' "$TEST_IPTABLES_LOG" || true)" -eq 5 ] || fail "static route failure fixture did not publish all firewall rules before route failure"
TEST_STATIC_ROUTE_COUNT=0
TEST_STATIC_ROUTE_ENABLE=0
TEST_ROUTE_ADD_STATUS=0
TEST_ROUTE_ADD_PARTIAL=0
TEST_ROUTE_PRESENT=0
TEST_NAT_ENABLE=0
TEST_ROUTE_MODE=none

# Missing NAT route is bounded and leaves firewall state untouched.
: > "$TEST_SLEEP_LOG"
: > "$TEST_IPTABLES_LOG"
echo 0 > "$TEST_ROUTE_COUNT"
TEST_NAT_ENABLE=1
TEST_ROUTE_MODE=none
if rules; then
	fail "rules succeeded without a ZeroTier kernel route while NAT was enabled"
fi
[ "$(wc -l < "$TEST_SLEEP_LOG" | tr -d ' ')" = 30 ] || fail "route wait was not bounded to 30 polls"
[ ! -s "$TEST_IPTABLES_LOG" ] || fail "firewall rules were published before NAT route readiness"

# Malformed NVRAM NAT state must fail before firewall publication.
: > "$TEST_IPTABLES_LOG"
TEST_NAT_ENABLE=enabled
TEST_IFCONFIG_MODE=ready
if rules; then
	fail "rules accepted an invalid NAT setting"
fi
[ ! -s "$TEST_IPTABLES_LOG" ] || fail "invalid NAT setting changed firewall rules"

echo "ZeroTier bounded stop/start, readiness, firewall/static-route rollback and persistent-config preservation: PASS"
