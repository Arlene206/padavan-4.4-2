#!/bin/sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname "$0")/../../../.." && pwd)
SCRIPT="$ROOT/trunk/user/zerotier/zerotier.sh"
TEST_TMP=$(mktemp -d "${TMPDIR:-/tmp}/zerotier-lifecycle.XXXXXX")
ZT_PROC_ROOT="$TEST_TMP/proc"
mkdir -p "$ZT_PROC_ROOT"
export ZT_PROC_ROOT
TEST_STATE="$TEST_TMP/process.state"
TEST_NVRAM_SECRET_STATE="$TEST_TMP/nvram-secret.state"
TEST_NVRAM_PLANET_STATE="$TEST_TMP/nvram-planet.state"
: > "$TEST_NVRAM_SECRET_STATE"
: > "$TEST_NVRAM_PLANET_STATE"
TEST_NVRAM_GET_FAIL=0
TEST_NVRAM_READ_FAIL_KEY=""
TEST_NVRAM_MOONID_STATE=""
TEST_NVRAM_MOON_ENABLE_STATE=""
TEST_NVRAM_MOON_IP_STATE="192.0.2.10"
TEST_NVRAM_MOON_ID_STATE=""
TEST_NVRAM_ID_STATE=""
TEST_NVRAM_SET_FAIL=0
TEST_NVRAM_COMMIT_FAIL=0
TEST_NVRAM_MOON_SET_FAIL_AFTER_WRITE=0
TEST_NVRAM_MOON_SET_FAIL_ONCE=0
TEST_NVRAM_MOON_COMMIT_FAIL_ONCE=0
TEST_NVRAM_MOON_RM_FAIL_ONCE=0
export TEST_NVRAM_SECRET_STATE TEST_NVRAM_PLANET_STATE TEST_NVRAM_GET_FAIL TEST_NVRAM_READ_FAIL_KEY TEST_NVRAM_MOONID_STATE TEST_NVRAM_MOON_ENABLE_STATE TEST_NVRAM_MOON_IP_STATE TEST_NVRAM_MOON_ID_STATE TEST_NVRAM_ID_STATE TEST_NVRAM_SET_FAIL TEST_NVRAM_COMMIT_FAIL
TEST_KILL_LOG="$TEST_TMP/killall.log"
TEST_SLEEP_LOG="$TEST_TMP/sleep.log"
TEST_ACTION_LOG="$TEST_TMP/actions.log"
TEST_IPTABLES_LOG="$TEST_TMP/iptables.log"
TEST_IPTABLES_STATE_FILE="$TEST_TMP/iptables.state"
TEST_IFCONFIG_COUNT="$TEST_TMP/ifconfig.count"
TEST_IFCONFIG_CALL_LOG="$TEST_TMP/ifconfig.calls"
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
TEST_ROUTE_DEL_DETAILS_LOG="$TEST_TMP/route-delete-details.log"
TEST_ROUTE_ADD_PARTIAL=0
TEST_EXPECT_SAVED_SEGMENT=0
TEST_IPTABLES_FAIL_ON=""
TEST_IPTABLES_CHECK_FAIL_ON=""
TEST_IPTABLES_DELETE_FAIL_ON=""
: > "$TEST_IFCONFIG_COUNT"
: > "$TEST_IFCONFIG_CALL_LOG"
: > "$TEST_ROUTE_COUNT"
: > "$TEST_IPTABLES_STATE_FILE"
export TEST_STATE TEST_KILL_LOG TEST_SLEEP_LOG TEST_ACTION_LOG TEST_TMP
export TEST_IPTABLES_LOG TEST_IPTABLES_STATE_FILE TEST_IFCONFIG_COUNT TEST_IFCONFIG_CALL_LOG TEST_ROUTE_COUNT
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

# Keep a separately callable copy of the production launcher for focused
# post-readiness failure tests; start_zero() below still uses its test hook.
sed -n '/^start_instance() {$/,/^}$/p' "$SCRIPT" |
	sed '1s/^start_instance()/start_instance_production()/' > "$TEST_TMP/start_instance_production.sh"
[ -s "$TEST_TMP/start_instance_production.sh" ] || fail "production start_instance extraction failed"
. "$TEST_TMP/start_instance_production.sh"

kill() {
	printf '%s %s\n' "$1" "$2" >> "$TEST_KILL_LOG"
	case "$TEST_MODE:$1" in
		graceful:-TERM|-9-exits:-KILL)
			/bin/rm -rf "$ZT_PROC_ROOT/$2"
			echo stopped > "$TEST_STATE"
			;;
		unknown-after-term:-TERM)
			/bin/rm -f "$ZT_PROC_ROOT/$2/cmdline"
			echo unknown > "$TEST_STATE"
			;;
		*)
			;;
	esac
	return 0
}

test_proc_reset() {
	/bin/rm -rf "$ZT_PROC_ROOT"
	mkdir -p "$ZT_PROC_ROOT"
}

test_proc_add() {
	test_proc_pid="$1"
	test_proc_exe="$2"
	test_proc_argv0="$3"
	test_proc_config="${4:-$config_path}"
	mkdir -p "$ZT_PROC_ROOT/$test_proc_pid"
	printf '%s\n' zerotier-one > "$ZT_PROC_ROOT/$test_proc_pid/comm"
	ln -s "$test_proc_exe" "$ZT_PROC_ROOT/$test_proc_pid/exe"
	printf '%s\000%s\000' "$test_proc_argv0" "$test_proc_config" > "$ZT_PROC_ROOT/$test_proc_pid/cmdline"
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
	if [ "${TEST_ASSERT_CLEAN_BEFORE_START:-0}" = 1 ]; then
		[ ! -e "$config_path/.firewall-state" ] || fail "replacement daemon started before stale firewall checkpoint was cleared"
		[ ! -e "$config_path/.static-routes-state" ] || fail "replacement daemon started before stale static-route checkpoint was cleared"
		[ ! -s "$TEST_IPTABLES_STATE_FILE" ] || fail "replacement daemon started with stale firewall rules"
		[ "$TEST_ROUTE_PRESENT" -eq 0 ] || fail "replacement daemon started with a stale configured route"
	fi
	echo start >> "$TEST_ACTION_LOG"
	return "$TEST_START_INSTANCE_STATUS"
}

rm() {
	if [ "${TEST_NVRAM_MOON_RM_FAIL_ONCE:-0}" = 1 ] && [ "$1" = "-f" ]; then
		TEST_NVRAM_MOON_RM_FAIL_ONCE=0
		shift
		/bin/rm -f "$1"
		return 1
	fi
	if [ "$1" = "-rf" ]; then
		printf 'rm:%s\n' "$*" >> "$TEST_ACTION_LOG"
	fi
	/bin/rm "$@"
}


ifconfig() {
	if [ -n "${TEST_IFCONFIG_CALL_LOG:-}" ]; then
		printf '%s\n' "$*" >> "$TEST_IFCONFIG_CALL_LOG"
	fi
	case "$TEST_IFCONFIG_MODE" in
		moon-ip)
			printf '%s\n' "ppp0 Link encap:Point-to-Point Protocol" "          inet addr:198.51.100.7  P-t-P:198.51.100.1  Mask:255.255.255.255"
			;;
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
		if [ -n "${TEST_ROUTE_SHOW_OUTPUT:-}" ]; then
			printf '%s\n' "$TEST_ROUTE_SHOW_OUTPUT"
			return 0
		fi
		if [ "$TEST_ROUTE_PRESENT" -eq 1 ] && [ "${3:-}" = "$TEST_STATIC_ROUTE_IP" ]; then
			printf '%s\n' "$TEST_STATIC_ROUTE_IP via $TEST_STATIC_ROUTE_GATEWAY dev ztunit"
		fi
		return 0
	fi
	if [ "$1" = route ] && [ "${2:-}" = del ]; then
		[ "$7" = ztunit ] || fail "static route cleanup lost the saved interface after daemon exit"
		echo "zero_route:del" >> "$TEST_ACTION_LOG"
		printf '%s via %s dev %s\n' "$3" "$5" "$7" > "$TEST_ROUTE_DEL_DETAILS_LOG"
		[ "$TEST_ROUTE_DEL_STATUS" -eq 0 ] || return "$TEST_ROUTE_DEL_STATUS"
		if [ "$TEST_ROUTE_DELETE_PERSISTS" -eq 0 ]; then
			TEST_ROUTE_PRESENT=0
			TEST_ROUTE_SHOW_OUTPUT=""
		fi
		return 0
	fi
	[ "$1" = route ] || return 0
	[ "${TEST_ROUTE_DUMP_STATUS:-0}" -eq 0 ] || return "$TEST_ROUTE_DUMP_STATUS"
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
		daemon-only)
			if [ "$(cat "$TEST_STATE")" = running ]; then
				printf '%s\n' "${TEST_ROUTE_SEGMENT:-10.147.17.0/24} dev ztunit proto kernel scope link"
			fi
			;;
		daemon-static)
			if [ "$(cat "$TEST_STATE")" = running ]; then
				printf '%s\n' "${TEST_ROUTE_SEGMENT:-10.147.17.0/24} dev ztunit proto static scope link"
			fi
			;;
		daemon-multipath)
			if [ "$(cat "$TEST_STATE")" = running ]; then
				printf '%s\n' 'default proto static nexthop via 192.0.2.1 dev ztunit weight 1 nexthop via 192.0.2.2 dev br0 weight 1'
			fi
			;;
		*)
			;;
	esac
	return 0
}

nvram() {
	case "$1" in
		get|get_checked)
			if [ "$1" = "get_checked" ] &&
			   [ "${TEST_NVRAM_READ_FAIL_KEY:-}" = "$2" ]; then
				return 1
			fi
			case "$2" in
				zerotier_moonid) printf '%s\n' "$TEST_NVRAM_MOONID_STATE" ;;
				zerotier_secret) cat "$TEST_NVRAM_SECRET_STATE" ;;
				zerotiermoon_enable) printf '%s\n' "$TEST_NVRAM_MOON_ENABLE_STATE" ;;
				zerotiermoon_ip) printf '%s\n' "$TEST_NVRAM_MOON_IP_STATE" ;;
				zerotiermoon_id) printf '%s\n' "$TEST_NVRAM_MOON_ID_STATE" ;;
				zerotier_planet)
					[ "${TEST_NVRAM_GET_FAIL:-0}" != 1 ] || return 1
					cat "$TEST_NVRAM_PLANET_STATE"
					;;
				zerotier_id) printf '%s\n' "$TEST_NVRAM_ID_STATE" ;;
				zerotier_nat) printf '%s\n' "$TEST_NAT_ENABLE" ;;
				zero_staticnum_x) printf '%s\n' "$TEST_STATIC_ROUTE_COUNT" ;;
				zero_enable_x0) printf '%s\n' "$TEST_STATIC_ROUTE_ENABLE" ;;
				zero_ip_x0) printf '%s\n' "$TEST_STATIC_ROUTE_IP" ;;
				zero_route_x0) printf '%s\n' "$TEST_STATIC_ROUTE_GATEWAY" ;;
			esac
			;;
		set)
			case "$2" in
				zerotier_secret=*)
					[ "${TEST_NVRAM_SET_FAIL:-0}" != 1 ] || return 1
					printf '%s' "${2#zerotier_secret=}" > "$TEST_NVRAM_SECRET_STATE"
					;;
				zerotiermoon_id=*)
					if [ "${TEST_NVRAM_MOON_SET_FAIL_ONCE:-0}" = 1 ]; then
						TEST_NVRAM_MOON_SET_FAIL_ONCE=0
						return 1
					fi
					[ "${TEST_NVRAM_SET_FAIL:-0}" != 1 ] || return 1
					TEST_NVRAM_MOON_ID_STATE="${2#zerotiermoon_id=}"
					if [ "${TEST_NVRAM_MOON_SET_FAIL_AFTER_WRITE:-0}" = 1 ]; then
						TEST_NVRAM_MOON_SET_FAIL_AFTER_WRITE=0
						return 1
					fi
					;;
				zerotier_planet=*)
					[ "${TEST_NVRAM_SET_FAIL:-0}" != 1 ] || return 1
					printf '%s' "${2#zerotier_planet=}" > "$TEST_NVRAM_PLANET_STATE"
					;;
			esac
			;;
		commit)
			if [ "${TEST_NVRAM_MOON_COMMIT_FAIL_ONCE:-0}" = 1 ]; then
				TEST_NVRAM_MOON_COMMIT_FAIL_ONCE=0
				return 1
			fi
			[ "${TEST_NVRAM_COMMIT_FAIL:-0}" != 1 ] || return 1
			;;
	esac
	return 0
}

# Startup settings are one fail-closed preflight: absent keys preserve empty defaults.
TEST_NVRAM_MOONID_STATE=""
TEST_NVRAM_MOON_ENABLE_STATE=""
TEST_NVRAM_ID_STATE=""
: > "$TEST_NVRAM_SECRET_STATE"
: > "$TEST_NVRAM_PLANET_STATE"
zerotier_load_start_settings || fail "missing optional startup settings were rejected"
[ -z "$moonid" ] || fail "missing moonid did not retain the legacy empty default"
[ -z "$secret" ] || fail "missing identity secret did not retain the legacy empty default"
[ -z "$enablemoonserv" ] || fail "missing moon-service selector did not retain the legacy empty default"
[ -z "$planet" ] || fail "missing planet did not retain the legacy empty default"
[ -z "$zt_network_id" ] || fail "missing network ID did not retain the legacy empty default"

# A complete snapshot promotes all five values together.
TEST_NVRAM_MOONID_STATE="0123456789abcdef"
printf '%s' "stored-start-secret" > "$TEST_NVRAM_SECRET_STATE"
TEST_NVRAM_MOON_ENABLE_STATE="1"
printf '%s' "encoded-planet" > "$TEST_NVRAM_PLANET_STATE"
TEST_NVRAM_ID_STATE="fedcba9876543210"
zerotier_load_start_settings || fail "valid startup settings snapshot failed"
[ "$moonid" = "0123456789abcdef" ] || fail "startup moonid snapshot mismatch"
[ "$secret" = "stored-start-secret" ] || fail "startup identity-secret snapshot mismatch"
[ "$enablemoonserv" = "1" ] || fail "startup moon-service snapshot mismatch"
[ "$planet" = "encoded-planet" ] || fail "startup planet snapshot mismatch"
[ "$zt_network_id" = "fedcba9876543210" ] || fail "startup network-ID snapshot mismatch"

# Invalid selector values are not defaults: they must fail before publishing
# any of the five settings, especially before a bad Moon flag can mean remove.
for invalid_moon_enable in 2 enabled; do
	moonid="sentinel-moonid"
	secret="sentinel-secret"
	enablemoonserv="sentinel-moon-enable"
	planet="sentinel-planet"
	zt_network_id="sentinel-network-id"
	TEST_NVRAM_MOON_ENABLE_STATE="$invalid_moon_enable"
	TEST_NVRAM_ID_STATE="fedcba9876543210"
	if zerotier_load_start_settings; then
		fail "startup accepted invalid Moon-enable selector $invalid_moon_enable"
	fi
	[ "$moonid" = "sentinel-moonid" ] || fail "invalid Moon-enable selector partially published moonid"
	[ "$secret" = "sentinel-secret" ] || fail "invalid Moon-enable selector partially published secret"
	[ "$enablemoonserv" = "sentinel-moon-enable" ] || fail "invalid Moon-enable selector partially published Moon flag"
	[ "$planet" = "sentinel-planet" ] || fail "invalid Moon-enable selector partially published planet"
	[ "$zt_network_id" = "sentinel-network-id" ] || fail "invalid Moon-enable selector partially published network ID"
done

TEST_NVRAM_MOON_ENABLE_STATE="1"
for invalid_network_id in short 0123456789abcdeg 0123456789abcdef0; do
	moonid="sentinel-moonid"
	secret="sentinel-secret"
	enablemoonserv="sentinel-moon-enable"
	planet="sentinel-planet"
	zt_network_id="sentinel-network-id"
	TEST_NVRAM_ID_STATE="$invalid_network_id"
	if zerotier_load_start_settings; then
		fail "startup accepted invalid network ID $invalid_network_id"
	fi
	[ "$moonid" = "sentinel-moonid" ] || fail "invalid network ID partially published moonid"
	[ "$secret" = "sentinel-secret" ] || fail "invalid network ID partially published secret"
	[ "$enablemoonserv" = "sentinel-moon-enable" ] || fail "invalid network ID partially published Moon flag"
	[ "$planet" = "sentinel-planet" ] || fail "invalid network ID partially published planet"
	[ "$zt_network_id" = "sentinel-network-id" ] || fail "invalid network ID partially published network ID"
done

TEST_NVRAM_MOON_ENABLE_STATE="1"
TEST_NVRAM_ID_STATE="fedcba9876543210"

# Invalid but readable selectors must not stop a previously running daemon.
for setting in zerotiermoon_enable zerotier_id; do
	if [ "$setting" = "zerotiermoon_enable" ]; then
		TEST_NVRAM_MOON_ENABLE_STATE=2
	else
		TEST_NVRAM_MOON_ENABLE_STATE=1
		TEST_NVRAM_ID_STATE=invalid
	fi
	: > "$TEST_KILL_LOG"
	: > "$TEST_ACTION_LOG"
	echo running > "$TEST_STATE"
	if start_zero; then
		fail "start accepted invalid selector $setting"
	fi
	[ "$(cat "$TEST_STATE")" = "running" ] || fail "invalid selector $setting stopped the existing daemon"
	[ ! -s "$TEST_KILL_LOG" ] || fail "invalid selector $setting signalled the existing daemon"
	[ ! -s "$TEST_ACTION_LOG" ] || fail "invalid selector $setting changed network state"
done
TEST_NVRAM_MOON_ENABLE_STATE="1"
TEST_NVRAM_ID_STATE="fedcba9876543210"

# Failure at each selector must leave every consumer-visible value untouched.
for setting in zerotier_moonid zerotier_secret zerotiermoon_enable zerotier_planet zerotier_id; do
	moonid="sentinel-moonid"
	secret="sentinel-secret"
	enablemoonserv="sentinel-moon-enable"
	planet="sentinel-planet"
	zt_network_id="sentinel-network-id"
	TEST_NVRAM_READ_FAIL_KEY="$setting"
	if zerotier_load_start_settings; then
		fail "startup settings read error was accepted for $setting"
	fi
	[ "$moonid" = "sentinel-moonid" ] || fail "startup settings partially published moonid after $setting read failure"
	[ "$secret" = "sentinel-secret" ] || fail "startup settings partially published secret after $setting read failure"
	[ "$enablemoonserv" = "sentinel-moon-enable" ] || fail "startup settings partially published moon-enable after $setting read failure"
	[ "$planet" = "sentinel-planet" ] || fail "startup settings partially published planet after $setting read failure"
	[ "$zt_network_id" = "sentinel-network-id" ] || fail "startup settings partially published network ID after $setting read failure"
done
TEST_NVRAM_READ_FAIL_KEY=""

# Moon selectors are consumed after startup settings, so they must be
# snapshotted and validated before an existing daemon is signalled.
TEST_NVRAM_MOON_ENABLE_STATE=1
TEST_NVRAM_MOON_IP_STATE="192.0.2.10"
TEST_NVRAM_MOON_ID_STATE="abcdef0123"
for setting in zerotiermoon_ip zerotiermoon_id; do
	TEST_NVRAM_READ_FAIL_KEY="$setting"
	: > "$TEST_KILL_LOG"
	: > "$TEST_ACTION_LOG"
	echo running > "$TEST_STATE"
	if start_zero; then
		fail "start accepted unreadable Moon selector $setting"
	fi
	[ "$(cat "$TEST_STATE")" = "running" ] || fail "Moon selector $setting read error stopped the existing daemon"
	[ ! -s "$TEST_KILL_LOG" ] || fail "Moon selector $setting read error signalled the existing daemon"
	[ ! -s "$TEST_ACTION_LOG" ] || fail "Moon selector $setting read error changed network state"
done
TEST_NVRAM_READ_FAIL_KEY=""
TEST_NVRAM_MOON_IP_STATE="192.0.2.999"
: > "$TEST_KILL_LOG"
: > "$TEST_ACTION_LOG"
echo running > "$TEST_STATE"
if start_zero; then
	fail "start accepted an invalid Moon IPv4 selector"
fi
[ "$(cat "$TEST_STATE")" = "running" ] || fail "invalid Moon IPv4 selector stopped the existing daemon"
[ ! -s "$TEST_KILL_LOG" ] || fail "invalid Moon IPv4 selector signalled the existing daemon"
[ ! -s "$TEST_ACTION_LOG" ] || fail "invalid Moon IPv4 selector changed network state"

TEST_NVRAM_MOON_ENABLE_STATE=0
TEST_NVRAM_MOON_ID_STATE="invalid"
: > "$TEST_KILL_LOG"
: > "$TEST_ACTION_LOG"
echo running > "$TEST_STATE"
if start_zero; then
	fail "start accepted an invalid Moon identity in removal mode"
fi
[ "$(cat "$TEST_STATE")" = "running" ] || fail "invalid Moon identity stopped the existing daemon"
[ ! -s "$TEST_KILL_LOG" ] || fail "invalid Moon identity signalled the existing daemon"
[ ! -s "$TEST_ACTION_LOG" ] || fail "invalid Moon identity changed network state"

TEST_NVRAM_MOON_ENABLE_STATE=1
TEST_NVRAM_MOON_ID_STATE="abcdef0123"
TEST_NVRAM_MOON_IP_STATE="192.0.2.10"
# An empty configured endpoint is resolved from ppp0 before shutdown too.
TEST_NVRAM_MOON_IP_STATE=""
TEST_IFCONFIG_MODE=moon-ip
if ! zerotier_preflight_moon_settings 1; then
	fail "valid PPP WAN fallback address was rejected by Moon preflight"
fi
[ "$zt_moon_create_ip_snapshot" = "198.51.100.7" ] || fail "Moon preflight did not cache the PPP WAN fallback address"
TEST_IFCONFIG_MODE=none
TEST_NVRAM_MOON_IP_STATE="192.0.2.10"

# A checked startup read failure must not stop an already-running daemon.
: > "$TEST_KILL_LOG"
: > "$TEST_ACTION_LOG"
echo running > "$TEST_STATE"
TEST_MODE=graceful
TEST_NVRAM_READ_FAIL_KEY="zerotier_secret"
if start_zero; then
	fail "start proceeded after startup settings read error"
fi
[ ! -s "$TEST_KILL_LOG" ] || fail "start stopped the existing daemon before checking startup settings"
[ ! -s "$TEST_ACTION_LOG" ] || fail "start performed network cleanup or relaunched after a startup settings error"
[ "$(cat "$TEST_STATE")" = "running" ] || fail "startup settings error changed the prior daemon state"
TEST_NVRAM_READ_FAIL_KEY=""

iptables() {
	iptables_raw="$*"
	printf '%s\n' "$iptables_raw" >> "$TEST_IPTABLES_LOG"
	if [ -n "$TEST_IPTABLES_FAIL_ON" ] && [ "$iptables_raw" = "$TEST_IPTABLES_FAIL_ON" ]; then
		return 1
	fi
	iptables_table=filter
	if [ "$1" = "-t" ]; then
		iptables_table="$2"
		shift 2
	fi
	iptables_operation="$1"
	shift
	iptables_key="$iptables_table $*"

	if [ "${TEST_EXPECT_CANONICAL_CHECKPOINT:-0}" -eq 1 ] &&
	   { [ "$iptables_operation" = "-C" ] || [ "$iptables_operation" = "-D" ]; }; then
		[ -f "$config_path/.firewall-state" ] &&
		[ "$(cat "$config_path/.firewall-state")" = "$(printf 'ztunit\n10.147.17.0/24')" ] &&
		[ "$(wc -l < "$config_path/.firewall-state" | tr -d ' ')" = 2 ] ||
			fail "checkpoint recovery did not preserve the validated tuple"
	fi

	if [ "$TEST_EXPECT_SAVED_SEGMENT" -eq 1 ] && { [ "$iptables_operation" = "-C" ] || [ "$iptables_operation" = "-D" ]; }; then
		case "$iptables_key" in
			"filter INPUT -i ztunit -j ACCEPT"|"filter FORWARD -i ztunit -o ztunit -j ACCEPT"|"filter FORWARD -i ztunit -j ACCEPT"|"nat POSTROUTING -o ztunit -j MASQUERADE"|"nat POSTROUTING -s 10.147.17.0/24 -j MASQUERADE") ;;
			*) fail "firewall cleanup lost the interface after daemon exit" ;;
		esac
		if [ "$iptables_key" = "nat POSTROUTING -s 10.147.17.0/24 -j MASQUERADE" ] &&
		   [ "$ip_segment" != "10.147.17.0/24" ]; then
			fail "firewall cleanup lost the pre-stop NAT source segment"
		fi
	fi

	case "$iptables_operation" in
		-A)
			printf '%s\n' "$iptables_key" >> "$TEST_IPTABLES_STATE_FILE"
			return 0
			;;
		-C)
			if [ "$TEST_IPTABLES_CHECK_FAIL_ON" = "$iptables_key" ]; then
				return 2
			fi
			if grep -Fqx -- "$iptables_key" "$TEST_IPTABLES_STATE_FILE"; then
				return 0
			fi
			return 1
			;;
		-D)
			if [ "$TEST_IPTABLES_DELETE_FAIL_ON" = "$iptables_key" ]; then
				return 1
			fi
			if ! grep -Fqx -- "$iptables_key" "$TEST_IPTABLES_STATE_FILE"; then
				return 1
			fi
			awk -v rule="$iptables_key" '
				$0 == rule && !removed { removed = 1; next }
				{ print }
				END { if (!removed) exit 1 }
			' "$TEST_IPTABLES_STATE_FILE" > "$TEST_IPTABLES_STATE_FILE.tmp" || return 1
			mv "$TEST_IPTABLES_STATE_FILE.tmp" "$TEST_IPTABLES_STATE_FILE"
			return 0
			;;
		*)
			return 2
			;;
	esac
}
test_seed_firewall_rules() {
	iptables -A INPUT -i ztunit -j ACCEPT
	iptables -A FORWARD -i ztunit -o ztunit -j ACCEPT
	iptables -A FORWARD -i ztunit -j ACCEPT
	iptables -t nat -A POSTROUTING -o ztunit -j MASQUERADE
	iptables -t nat -A POSTROUTING -s 10.147.17.0/24 -j MASQUERADE
}

: > "$TEST_KILL_LOG"
: > "$TEST_SLEEP_LOG"
: > "$TEST_ACTION_LOG"
test_proc_reset
echo stopped > "$TEST_STATE"
TEST_MODE=none
export TEST_MODE
kill_z || fail "no-process stop returned failure"
[ ! -s "$TEST_KILL_LOG" ] || fail "stop signalled a process when exact procfs inventory found none"
[ ! -s "$TEST_SLEEP_LOG" ] || fail "no-process stop waited unnecessarily"

# pidof/killall used to match the mutable comm name and could kill an impostor.
# A same-name process with another executable must be treated as ABSENT.
: > "$TEST_KILL_LOG"
: > "$TEST_SLEEP_LOG"
test_proc_reset
test_proc_add 4322 "$TEST_TMP/not-zerotier-one" "$PROG"
if ! kill_z; then
	fail "same-name impostor prevented a confirmed-absent result"
fi
[ ! -s "$TEST_KILL_LOG" ] || fail "same-name impostor executable was signalled"
[ ! -s "$TEST_SLEEP_LOG" ] || fail "same-name impostor caused an unnecessary wait"

# An executable match with an unexpected argv[0] is not owned by this launcher.
: > "$TEST_KILL_LOG"
test_proc_reset
test_proc_add 4323 "$PROG" "$TEST_TMP/renamed-argv0"
if ! kill_z; then
	fail "unowned argv[0] process prevented a confirmed-absent result"
fi
[ ! -s "$TEST_KILL_LOG" ] || fail "executable with mismatched argv[0] was signalled"

# A second ZeroTier binary using another state directory is not this service.
: > "$TEST_KILL_LOG"
test_proc_reset
test_proc_add 4324 "$PROG" "$PROG" "$TEST_TMP/other-zerotier-state"
if ! kill_z; then
	fail "an independent ZeroTier instance prevented a confirmed-absent result"
fi
[ ! -s "$TEST_KILL_LOG" ] || fail "an independent ZeroTier instance was signalled"

# A same-name process without readable executable identity is UNKNOWN, not absent.
: > "$TEST_KILL_LOG"
test_proc_reset
test_proc_add 4325 "$TEST_TMP/not-readable" "$PROG"
 /bin/rm -f "$ZT_PROC_ROOT/4325/exe"
if kill_z; then
	fail "stop accepted a same-name process whose executable identity is unavailable"
fi
[ ! -s "$TEST_KILL_LOG" ] || fail "stop signalled a same-name process without executable identity"

# Missing executable metadata for the exact binary is UNKNOWN, not ABSENT.
: > "$TEST_KILL_LOG"
: > "$TEST_SLEEP_LOG"
test_proc_reset
test_proc_add 4321 "$PROG" "$PROG"
 /bin/rm -f "$ZT_PROC_ROOT/4321/cmdline"
if kill_z; then
	fail "stop accepted an exact executable with unreadable command line"
fi
[ ! -s "$TEST_KILL_LOG" ] || fail "stop signalled a process with incomplete executable identity"
[ ! -s "$TEST_SLEEP_LOG" ] || fail "stop waited after initial executable identity became UNKNOWN"

# A non-directory proc root is UNKNOWN and must block every signal.
: > "$TEST_KILL_LOG"
: > "$TEST_SLEEP_LOG"
test_proc_reset
/bin/rm -rf "$ZT_PROC_ROOT"
printf 'not-a-proc-directory\n' > "$ZT_PROC_ROOT"
if kill_z; then
	fail "stop accepted an unavailable procfs root"
fi
[ ! -s "$TEST_KILL_LOG" ] || fail "stop signalled with an unavailable procfs root"
[ ! -s "$TEST_SLEEP_LOG" ] || fail "stop waited with an unavailable procfs root"
test_proc_reset

# A transition to UNKNOWN after TERM must not be mistaken for process exit or
# trigger SIGKILL escalation.
: > "$TEST_KILL_LOG"
: > "$TEST_SLEEP_LOG"
test_proc_add 4321 "$PROG" "$PROG"
TEST_MODE=unknown-after-term
if kill_z; then
	fail "stop accepted an unknown process state after TERM"
fi
[ "$(cat "$TEST_KILL_LOG")" = "-TERM 4321" ] || fail "stop escalated after process identity became UNKNOWN"
[ ! -s "$TEST_SLEEP_LOG" ] || fail "stop waited after process identity became UNKNOWN"

: > "$TEST_KILL_LOG"
: > "$TEST_SLEEP_LOG"
test_proc_reset
test_proc_add 4321 "$PROG" "$PROG"
TEST_MODE=graceful
if ! kill_z; then
	fail "graceful TERM stop returned failure"
fi
[ "$(cat "$TEST_KILL_LOG")" = "-TERM 4321" ] || fail "graceful stop did not signal only the exact daemon PID"
[ ! -s "$TEST_SLEEP_LOG" ] || fail "graceful exit was not observed immediately"

: > "$TEST_KILL_LOG"
: > "$TEST_SLEEP_LOG"
test_proc_reset
test_proc_add 4321 "$PROG" "$PROG"
TEST_MODE=-9-exits
if ! kill_z; then
	fail "TERM-resistant daemon did not stop after exact-PID escalation"
fi
[ "$(cat "$TEST_KILL_LOG")" = "$(printf '%s\n' '-TERM 4321' '-KILL 4321')" ] || fail "force stop did not revalidate and signal the exact PID"
[ "$(wc -l < "$TEST_SLEEP_LOG" | tr -d ' ')" = 5 ] || fail "TERM grace period was not bounded to five one-second polls"

: > "$TEST_KILL_LOG"
: > "$TEST_SLEEP_LOG"
test_proc_reset
test_proc_add 4321 "$PROG" "$PROG"
TEST_MODE=stuck
if kill_z; then
	fail "stop reported success while daemon remained after SIGKILL"
fi
[ "$(cat "$TEST_KILL_LOG")" = "$(printf '%s\n' '-TERM 4321' '-KILL 4321')" ] || fail "stuck daemon did not receive exact-PID TERM then KILL"
[ "$(wc -l < "$TEST_SLEEP_LOG" | tr -d ' ')" = 10 ] || fail "TERM/KILL waits were not bounded to five polls each"

: > "$TEST_KILL_LOG"
: > "$TEST_SLEEP_LOG"
: > "$TEST_ACTION_LOG"
test_proc_reset
test_proc_add 4321 "$PROG" "$PROG"
TEST_MODE=stuck
if start_zero; then
	fail "start proceeded when the prior daemon could not be stopped"
fi
[ ! -s "$TEST_ACTION_LOG" ] || fail "start reached start_instance after stop failure"

# An unavailable proc root must also block restart rather than start a duplicate.
: > "$TEST_KILL_LOG"
: > "$TEST_ACTION_LOG"
test_proc_reset
/bin/rm -rf "$ZT_PROC_ROOT"
printf 'unavailable\n' > "$ZT_PROC_ROOT"
if start_zero; then
	fail "start proceeded while process inventory was UNKNOWN"
fi
[ ! -s "$TEST_KILL_LOG" ] || fail "start signalled after process inventory became UNKNOWN"
[ ! -s "$TEST_ACTION_LOG" ] || fail "start reached start_instance after process inventory became UNKNOWN"
test_proc_reset

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

# Private configuration/identity writer regression: prove that a permissive
# inherited umask and attacker-controlled persistent paths cannot redirect
# privileged writes or expose the identity key.
chmod 755 "$TEST_CONFIG"
zerotier_prepare_config_directory || fail "config-directory preparation failed"
[ "$(stat -c '%a' "$TEST_CONFIG")" = 700 ] || fail "config directory was not restricted to mode 700"
[ "$(stat -c '%a' "$TEST_CONFIG/networks.d")" = 700 ] || fail "networks.d directory was not restricted to mode 700"

mkdir -p "$TEST_TMP/config-target"
chmod 755 "$TEST_TMP/config-target"
ln -s "$TEST_TMP/config-target" "$TEST_TMP/config-link"
saved_config_path="$config_path"
config_path="$TEST_TMP/config-link"
if zerotier_prepare_config_directory; then
	fail "config-directory preparation followed a symlink"
fi
[ "$(stat -c '%a' "$TEST_TMP/config-target")" = 755 ] || fail "symlink target permissions were changed"
config_path="$saved_config_path"

/bin/mv "$config_path/networks.d" "$TEST_TMP/networks.d.saved"
mkdir -p "$TEST_TMP/networks-target"
ln -s "$TEST_TMP/networks-target" "$config_path/networks.d"
if zerotier_prepare_config_directory; then
	fail "config preparation followed a networks.d symlink"
fi
[ ! -e "$TEST_TMP/networks-target/created" ] || fail "networks.d symlink target was modified"
/bin/rm "$config_path/networks.d"
/bin/mv "$TEST_TMP/networks.d.saved" "$config_path/networks.d"

# Moon address read errors must not be mistaken for an empty address/WAN fallback.
: > "$TEST_IFCONFIG_CALL_LOG"
echo "last-good-moon-json" > "$TEST_CONFIG/moon.json"
TEST_NVRAM_READ_FAIL_KEY="zerotiermoon_ip"
if creat_moon; then
	fail "Moon creation accepted an unreadable address selector"
fi
[ ! -s "$TEST_IFCONFIG_CALL_LOG" ] || fail "Moon address read failure triggered the WAN-interface fallback"
[ "$(cat "$TEST_CONFIG/moon.json")" = "last-good-moon-json" ] || fail "Moon address read failure modified the last-good Moon JSON"
TEST_NVRAM_READ_FAIL_KEY=""

# Moon generation is a staged transaction: invalid input or tool failures
# must preserve the last-good JSON, signature and persistent identity.
TEST_MOON_INIT_FAIL=0
TEST_MOON_GEN_FAIL=0
export TEST_MOON_INIT_FAIL TEST_MOON_GEN_FAIL
cat > "$TEST_TMP/moon-idtool" <<'EOF'
#!/bin/sh
case "$1" in
	initmoon)
		[ "${TEST_MOON_INIT_FAIL:-0}" != 1 ] || exit 1
		cat <<'JSON'
{
  "id": "0123456789",
  "stableEndpoints": []
}
JSON
		;;
	genmoon)
		[ "${TEST_MOON_GEN_FAIL:-0}" != 1 ] || exit 1
		printf '%s\n' generated-moon-signature > 0000000123456789.moon
		;;
	*)
		exit 2
		;;
esac
EOF
chmod 700 "$TEST_TMP/moon-idtool"
PROGIDT="$TEST_TMP/moon-idtool"
printf '%s\n' existing-public > "$TEST_CONFIG/identity.public"
mkdir -p "$TEST_CONFIG/moons.d"
TEST_NVRAM_MOON_IP_STATE="192.0.2.10"
TEST_NVRAM_MOON_ID_STATE="abcdef0123"
echo "last-good-moon-file" > "$TEST_CONFIG/moons.d/000000abcdef0123.moon"
echo "last-good-moon-json" > "$TEST_CONFIG/moon.json"

TEST_MOON_INIT_FAIL=1
if creat_moon; then
	fail "Moon creation ignored initmoon failure"
fi
[ "$(cat "$TEST_CONFIG/moons.d/000000abcdef0123.moon")" = "last-good-moon-file" ] || fail "initmoon failure changed the last-good signature"
[ "$(cat "$TEST_CONFIG/moon.json")" = "last-good-moon-json" ] || fail "initmoon failure changed the last-good JSON"
[ "$TEST_NVRAM_MOON_ID_STATE" = "abcdef0123" ] || fail "initmoon failure changed the stored ID"
TEST_MOON_INIT_FAIL=0

TEST_NVRAM_MOON_IP_STATE="192.0.2.10;touch $TEST_TMP/moon-injected"
if creat_moon; then
	fail "Moon creation accepted an unsafe endpoint replacement string"
fi
[ ! -e "$TEST_TMP/moon-injected" ] || fail "Moon endpoint validation allowed command side effects"
[ "$(cat "$TEST_CONFIG/moon.json")" = "last-good-moon-json" ] || fail "invalid endpoint changed the last-good JSON"
TEST_NVRAM_MOON_IP_STATE="192.0.2.10"

TEST_MOON_SED_FAIL=1
sed() {
	[ "${TEST_MOON_SED_FAIL:-0}" != 1 ] || return 1
	command sed "$@"
}
if creat_moon; then
	fail "Moon creation ignored endpoint-edit failure"
fi
TEST_MOON_SED_FAIL=0
[ "$(cat "$TEST_CONFIG/moons.d/000000abcdef0123.moon")" = "last-good-moon-file" ] || fail "endpoint-edit failure changed the last-good signature"
[ "$(cat "$TEST_CONFIG/moon.json")" = "last-good-moon-json" ] || fail "endpoint-edit failure changed the last-good JSON"

TEST_MOON_GEN_FAIL=1
if creat_moon; then
	fail "Moon creation ignored genmoon failure"
fi
TEST_MOON_GEN_FAIL=0
[ "$(cat "$TEST_CONFIG/moon.json")" = "last-good-moon-json" ] || fail "genmoon failure changed the last-good JSON"
[ "$(cat "$TEST_CONFIG/moons.d/000000abcdef0123.moon")" = "last-good-moon-file" ] || fail "genmoon failure changed the last-good signature"

# Simulate a setter that writes the new ID but reports failure afterwards.
TEST_NVRAM_MOON_SET_FAIL_AFTER_WRITE=1
if creat_moon; then
	fail "Moon creation ignored NVRAM set failure after write"
fi
[ "$TEST_NVRAM_MOON_ID_STATE" = "abcdef0123" ] || fail "Moon creation set-failure rollback lost the prior ID"
[ "$(cat "$TEST_CONFIG/moon.json")" = "last-good-moon-json" ] || fail "Moon creation set-failure rollback lost the JSON"
[ "$(cat "$TEST_CONFIG/moons.d/000000abcdef0123.moon")" = "last-good-moon-file" ] || fail "Moon creation set-failure rollback lost the signature"
set -- "$TEST_CONFIG"/.moon-create.*
[ ! -e "$1" ] || fail "successful Moon creation rollback retained a staging directory"

# Fail the second publication once; the first rename and both file snapshots
# must be rolled back before the function reports failure.
TEST_MOON_JSON_MV_FAIL=1
mv() {
	if [ "$#" -eq 3 ] && [ "$3" = "$TEST_CONFIG/moon.json" ] &&
	   [ "${TEST_MOON_JSON_MV_FAIL:-0}" = 1 ]; then
		TEST_MOON_JSON_MV_FAIL=0
		return 1
	fi
	command mv "$@"
}
if creat_moon; then
	fail "Moon creation ignored JSON publication failure"
fi
unset -f mv
[ ! -e "$TEST_CONFIG/moons.d/0000000123456789.moon" ] || fail "JSON publication rollback left the new signature"
[ "$(cat "$TEST_CONFIG/moon.json")" = "last-good-moon-json" ] || fail "JSON publication rollback lost the prior JSON"
[ "$TEST_NVRAM_MOON_ID_STATE" = "abcdef0123" ] || fail "JSON publication failure changed the stored ID"
set -- "$TEST_CONFIG"/.moon-create.*
[ ! -e "$1" ] || fail "JSON publication rollback retained a staging directory"

if ! creat_moon; then
	fail "valid Moon generation failed"
fi
[ "$TEST_NVRAM_MOON_ID_STATE" = "0123456789" ] || fail "successful Moon generation did not persist its identity"
grep -F '"192.0.2.10/9993"' "$TEST_CONFIG/moon.json" >/dev/null || fail "successful Moon JSON did not include the configured stable endpoint"
[ "$(cat "$TEST_CONFIG/moons.d/0000000123456789.moon")" = "generated-moon-signature" ] || fail "successful Moon generation did not publish the signed file"
[ "$(stat -c '%a' "$TEST_CONFIG/moons.d/0000000123456789.moon")" = 600 ] || fail "published Moon signature permissions are not private"
[ "$(stat -c '%a' "$TEST_CONFIG/moon.json")" = 600 ] || fail "published Moon JSON permissions are not private"
set -- "$TEST_CONFIG"/.moon-create.*
[ ! -e "$1" ] || fail "successful Moon generation retained a staging directory"

/bin/rm -f "$TEST_CONFIG/moons.d/000000abcdef0123.moon"
echo "last-good-moon-file" > "$TEST_CONFIG/moons.d/0000000123456789.moon"
echo "last-good-moon-json" > "$TEST_CONFIG/moon.json"
TEST_NVRAM_MOON_ID_STATE="0123456789"

# Moon identity read failures and malformed IDs must preserve signed files/JSON.
mkdir -p "$TEST_CONFIG/moons.d"
TEST_NVRAM_MOON_ID_STATE="0123456789"
echo "last-good-moon-file" > "$TEST_CONFIG/moons.d/0000000123456789.moon"
echo "last-good-moon-json" > "$TEST_CONFIG/moon.json"
TEST_NVRAM_READ_FAIL_KEY="zerotiermoon_id"
if remove_moon; then
	fail "Moon removal accepted an unreadable identity selector"
fi
[ -f "$TEST_CONFIG/moons.d/0000000123456789.moon" ] || fail "Moon identity read error deleted the signed file"
[ -f "$TEST_CONFIG/moon.json" ] || fail "Moon identity read error deleted the JSON"
[ "$TEST_NVRAM_MOON_ID_STATE" = "0123456789" ] || fail "Moon identity read error cleared the NVRAM ID"
TEST_NVRAM_READ_FAIL_KEY=""

TEST_NVRAM_MOON_ID_STATE="../escape!!"
if remove_moon; then
	fail "Moon removal accepted an unsafe non-hex identity path"
fi
[ -f "$TEST_CONFIG/moons.d/0000000123456789.moon" ] || fail "malformed Moon ID removed the last-good signed file"
[ -f "$TEST_CONFIG/moon.json" ] || fail "malformed Moon ID deleted the last-good JSON"
[ "$TEST_NVRAM_MOON_ID_STATE" = "../escape!!" ] || fail "malformed Moon ID was cleared from NVRAM"
TEST_NVRAM_MOON_ID_STATE="0123456789"

# NVRAM set failure must retain signed material and the stored identity.
echo "last-good-moon-file" > "$TEST_CONFIG/moons.d/0000000123456789.moon"
echo "last-good-moon-json" > "$TEST_CONFIG/moon.json"
TEST_NVRAM_MOON_ID_STATE="0123456789"
TEST_NVRAM_MOON_SET_FAIL_ONCE=1
if remove_moon; then
	fail "Moon removal ignored NVRAM set failure"
fi
[ "$(cat "$TEST_CONFIG/moons.d/0000000123456789.moon")" = "last-good-moon-file" ] || fail "NVRAM set failure lost the Moon signature"
[ "$(cat "$TEST_CONFIG/moon.json")" = "last-good-moon-json" ] || fail "NVRAM set failure lost the Moon JSON"
[ "$TEST_NVRAM_MOON_ID_STATE" = "0123456789" ] || fail "NVRAM set failure lost the Moon ID"
set -- "$TEST_CONFIG"/.moon-remove.*
[ ! -e "$1" ] || fail "NVRAM set failure rollback retained a staging directory"

# Commit failure after the volatile ID changes must restore the old value.
TEST_NVRAM_MOON_COMMIT_FAIL_ONCE=1
if remove_moon; then
	fail "Moon removal ignored NVRAM commit failure"
fi
[ "$(cat "$TEST_CONFIG/moons.d/0000000123456789.moon")" = "last-good-moon-file" ] || fail "NVRAM commit failure lost the Moon signature"
[ "$(cat "$TEST_CONFIG/moon.json")" = "last-good-moon-json" ] || fail "NVRAM commit failure lost the Moon JSON"
[ "$TEST_NVRAM_MOON_ID_STATE" = "0123456789" ] || fail "NVRAM commit failure did not restore the Moon ID"
set -- "$TEST_CONFIG"/.moon-remove.*
[ ! -e "$1" ] || fail "NVRAM commit failure rollback retained a staging directory"

# The mock removes the signature, then reports failure to exercise partial-delete rollback.
TEST_NVRAM_MOON_RM_FAIL_ONCE=1
if remove_moon; then
	fail "Moon removal ignored partial file deletion failure"
fi
[ "$(cat "$TEST_CONFIG/moons.d/0000000123456789.moon")" = "last-good-moon-file" ] || fail "partial deletion failure did not restore the Moon signature"
[ "$(cat "$TEST_CONFIG/moon.json")" = "last-good-moon-json" ] || fail "partial deletion failure did not restore the Moon JSON"
[ "$TEST_NVRAM_MOON_ID_STATE" = "0123456789" ] || fail "partial deletion failure did not restore the Moon ID"
set -- "$TEST_CONFIG"/.moon-remove.*
[ ! -e "$1" ] || fail "partial deletion rollback retained a staging directory"

if ! remove_moon; then
	fail "valid Moon identity was not removed"
fi
[ ! -e "$TEST_CONFIG/moons.d/0000000123456789.moon" ] || fail "valid Moon removal retained its signed file"
[ ! -e "$TEST_CONFIG/moon.json" ] || fail "valid Moon removal retained the JSON"
[ -z "$TEST_NVRAM_MOON_ID_STATE" ] || fail "valid Moon removal did not clear the persistent ID"

TEST_NVRAM_MOON_ID_STATE=""
echo "unrelated-moon-json" > "$TEST_CONFIG/moon.json"
if ! remove_moon; then
	fail "removing an already-absent Moon identity was not idempotent"
fi
[ "$(cat "$TEST_CONFIG/moon.json")" = "unrelated-moon-json" ] || fail "empty Moon ID deleted unrelated Moon JSON"

add_join "" || fail "an empty optional network ID should be a no-op"
[ ! -e "$config_path/networks.d/.conf" ] || fail "empty network ID created a .conf file"
valid_network_id=0123456789abcdef
add_join "$valid_network_id" || fail "valid 16-digit network ID was rejected"
valid_network_file="$config_path/networks.d/$valid_network_id.conf"
[ -f "$valid_network_file" ] || fail "valid network ID file was not created"
[ "$(stat -c '%a' "$valid_network_file")" = 600 ] || fail "network join file was not private"
for invalid_network_id in ../../escaped 0123456789ABCDEG 1234; do
	if add_join "$invalid_network_id"; then
		fail "invalid network ID was accepted: $invalid_network_id"
	fi
done
[ ! -e "$TEST_TMP/escaped.conf" ] || fail "path-traversal network ID created a file outside networks.d"

/bin/rm -f "$valid_network_file"
printf '%s\n' join-sentinel > "$TEST_TMP/join-sentinel"
ln -s "$TEST_TMP/join-sentinel" "$valid_network_file"
if add_join "$valid_network_id"; then
	fail "network join writer followed a symlink leaf"
fi
[ "$(cat "$TEST_TMP/join-sentinel")" = join-sentinel ] || fail "network join symlink target was modified"
/bin/rm -f "$valid_network_file"

cat > "$TEST_TMP/idtool" <<'EOF'
#!/bin/sh
set -eu
case "$1" in
	generate)
		printf '%s\n' generated-secret > "$2"
		printf '%s\n' generated-public > "$3"
		;;
	getpublic)
		[ "${TEST_IDTOOL_FAIL_PUBLIC:-0}" -eq 0 ] || exit 1
		[ -s "$2" ] || exit 1
		case "$(cat "$2")" in
			stored-secret) printf '%s\n' derived-public ;;
			replacement-secret) printf '%s\n' replacement-public ;;
			*) printf '%s\n' unexpected-public ;;
		esac
		;;
	*)
		exit 2
		;;
esac
EOF
chmod 700 "$TEST_TMP/idtool"
PROGIDT="$TEST_TMP/idtool"
saved_umask=$(umask)
umask 000
secret=""
zerotier_prepare_identity || fail "generated identity preparation failed"
[ "$(cat "$TEST_CONFIG/identity.secret")" = generated-secret ] || fail "generated identity secret mismatch"
[ "$(cat "$TEST_CONFIG/identity.public")" = generated-public ] || fail "generated identity public key mismatch"
[ "$(stat -c '%a' "$TEST_CONFIG/identity.secret")" = 600 ] || fail "generated identity secret was not mode 600"
[ "$(stat -c '%a' "$TEST_CONFIG/identity.public")" = 600 ] || fail "generated identity public key was not mode 600"

secret="stored-secret"
zerotier_prepare_identity || fail "stored identity preparation failed"
[ "$(cat "$TEST_CONFIG/identity.secret")" = stored-secret ] || fail "stored identity secret mismatch"
[ "$(cat "$TEST_CONFIG/identity.public")" = derived-public ] || fail "stored identity public key mismatch"
[ "$(stat -c '%a' "$TEST_CONFIG/identity.secret")" = 600 ] || fail "stored identity secret was not mode 600"
[ "$(stat -c '%a' "$TEST_CONFIG/identity.public")" = 600 ] || fail "stored identity public key was not mode 600"

TEST_IDTOOL_FAIL_PUBLIC=1
export TEST_IDTOOL_FAIL_PUBLIC
secret="replacement-secret"
if zerotier_prepare_identity; then
	fail "failed public-key derivation was accepted"
fi
[ "$(cat "$TEST_CONFIG/identity.secret")" = stored-secret ] || fail "failed identity staging replaced the prior secret"
[ "$(cat "$TEST_CONFIG/identity.public")" = derived-public ] || fail "failed identity staging replaced the prior public key"
TEST_IDTOOL_FAIL_PUBLIC=0
export TEST_IDTOOL_FAIL_PUBLIC
secret="stored-secret"

# If the second file rename fails, the first rename must be rolled back.
ZT_IDENTITY_MV_CALLS=0
mv() {
	ZT_IDENTITY_MV_CALLS=$((ZT_IDENTITY_MV_CALLS + 1))
	[ "$ZT_IDENTITY_MV_CALLS" -ne 2 ] || return 1
	command mv "$@"
}
secret="replacement-secret"
if zerotier_prepare_identity; then
	fail "second identity-file publication failure was reported as success"
fi
unset -f mv
[ "$(cat "$TEST_CONFIG/identity.secret")" = stored-secret ] || fail "second rename failure changed the old identity secret"
[ "$(cat "$TEST_CONFIG/identity.public")" = derived-public ] || fail "second rename failure did not restore the old public key"
set -- "$TEST_CONFIG"/.identity.*
[ ! -e "$1" ] || fail "successful identity rollback retained a staging directory"
secret="stored-secret"

# A failed restoration must retain the old public key in the private recovery
# directory instead of deleting the only recoverable copy.
ZT_IDENTITY_MV_CALLS=0
mv() {
	ZT_IDENTITY_MV_CALLS=$((ZT_IDENTITY_MV_CALLS + 1))
	case "$ZT_IDENTITY_MV_CALLS" in
		2|3) return 1 ;;
	esac
	command mv "$@"
}
secret="replacement-secret"
if zerotier_prepare_identity; then
	fail "incomplete identity rollback was reported as success"
fi
unset -f mv
[ "$(cat "$TEST_CONFIG/identity.secret")" = stored-secret ] || fail "failed rollback changed the old identity secret"
[ "$(cat "$TEST_CONFIG/identity.public")" = replacement-public ] || fail "rollback-failure fixture did not leave the new public key"
set -- "$TEST_CONFIG"/.identity.*
ZT_IDENTITY_RECOVERY_DIR="$1"
[ -d "$ZT_IDENTITY_RECOVERY_DIR" ] || fail "failed identity rollback removed its recovery directory"
[ "$(stat -c '%a' "$ZT_IDENTITY_RECOVERY_DIR")" = 700 ] || fail "identity recovery directory is not private"
[ "$(cat "$ZT_IDENTITY_RECOVERY_DIR/identity.public.previous")" = derived-public ] || fail "old public-key recovery link is missing or changed"
command mv "$ZT_IDENTITY_RECOVERY_DIR/identity.public.previous" "$TEST_CONFIG/identity.public" ||
	fail "manual identity recovery could not restore the old public key"
[ "$(cat "$TEST_CONFIG/identity.public")" = derived-public ] || fail "manual identity recovery restored the wrong public key"
 /bin/rm -rf "$ZT_IDENTITY_RECOVERY_DIR"
secret="stored-secret"

# If NVRAM persistence fails after publishing both files, the file pair and
# the volatile NVRAM value must return to their previous empty state.
/bin/rm -f "$TEST_CONFIG/identity.secret" "$TEST_CONFIG/identity.public"
: > "$TEST_NVRAM_SECRET_STATE"
TEST_NVRAM_COMMIT_FAIL=1
export TEST_NVRAM_COMMIT_FAIL
secret=""
if zerotier_prepare_identity; then
	fail "NVRAM commit failure was reported as success"
fi
TEST_NVRAM_COMMIT_FAIL=0
export TEST_NVRAM_COMMIT_FAIL
[ ! -e "$TEST_CONFIG/identity.secret" ] || fail "NVRAM failure rollback left a secret file"
[ ! -e "$TEST_CONFIG/identity.public" ] || fail "NVRAM failure rollback left a public-key file"
[ ! -s "$TEST_NVRAM_SECRET_STATE" ] || fail "NVRAM failure rollback retained the uncommitted secret"
secret="stored-secret"

# A failed NVRAM set must also roll back both newly published files.
/bin/rm -f "$TEST_CONFIG/identity.secret" "$TEST_CONFIG/identity.public"
: > "$TEST_NVRAM_SECRET_STATE"
TEST_NVRAM_SET_FAIL=1
export TEST_NVRAM_SET_FAIL
secret=""
if zerotier_prepare_identity; then
	fail "NVRAM set failure was reported as success"
fi
TEST_NVRAM_SET_FAIL=0
export TEST_NVRAM_SET_FAIL
[ ! -e "$TEST_CONFIG/identity.secret" ] || fail "NVRAM set failure rollback left a secret file"
[ ! -e "$TEST_CONFIG/identity.public" ] || fail "NVRAM set failure rollback left a public-key file"
[ ! -s "$TEST_NVRAM_SECRET_STATE" ] || fail "NVRAM set failure changed the original NVRAM value"
secret="stored-secret"

/bin/rm -f "$TEST_CONFIG/identity.secret"
printf '%s\n' identity-sentinel > "$TEST_TMP/identity-sentinel"
ln -s "$TEST_TMP/identity-sentinel" "$TEST_CONFIG/identity.secret"
if zerotier_prepare_identity; then
	fail "identity publisher followed a symlink destination"
fi
[ "$(cat "$TEST_TMP/identity-sentinel")" = identity-sentinel ] || fail "identity symlink target was modified"
/bin/rm -f "$TEST_CONFIG/identity.secret"
zerotier_prepare_identity || fail "identity recovery after symlink removal failed"

# NVRAM planet writes must decode privately and replace the live file
# only after successful validation; failed decoding and unsafe targets preserve
# the previous file/referent.
printf 'old-planet\n' > "$TEST_CONFIG/planet"
zerotier_publish_planet_base64 'cGxhbmV0LWRhdGE=' ||
	fail "valid NVRAM planet payload was rejected"
[ "$(cat "$TEST_CONFIG/planet")" = "planet-data" ] ||
	fail "valid NVRAM planet payload was not published"
[ "$(stat -c '%a' "$TEST_CONFIG/planet")" = 600 ] ||
	fail "published planet file was not private"

printf 'last-good-planet\n' > "$TEST_CONFIG/planet"
if zerotier_publish_planet_base64 'not base64 !!!'; then
	fail "invalid NVRAM planet payload was accepted"
fi
[ "$(cat "$TEST_CONFIG/planet")" = "last-good-planet" ] ||
	fail "invalid planet payload damaged the last-good file"
set -- "$TEST_CONFIG"/.planet.*
[ ! -e "$1" ] || fail "failed planet decode leaked a staging file"

mkdir -p "$TEST_TMP/planet-target"
printf 'untouched-planet-target\n' > "$TEST_TMP/planet-target/value"
rm -f "$TEST_CONFIG/planet"
ln -s "$TEST_TMP/planet-target/value" "$TEST_CONFIG/planet"
if zerotier_publish_planet_base64 'cGxhbmV0LWRhdGE='; then
	fail "symlink planet destination was accepted"
fi
[ "$(cat "$TEST_TMP/planet-target/value")" = "untouched-planet-target" ] ||
	fail "planet symlink referent was modified"
rm -f "$TEST_CONFIG/planet"
mkdir "$TEST_CONFIG/planet"
if zerotier_publish_planet_base64 'cGxhbmV0LWRhdGE='; then
	fail "directory planet destination was accepted"
fi
rmdir "$TEST_CONFIG/planet"
printf 'planet-reset\n' > "$TEST_CONFIG/planet"

/bin/rm -f "$TEST_CONFIG/planet"

# A failure reading the previous NVRAM value must still clean the stage.
printf 'old-planet-state' > "$TEST_NVRAM_PLANET_STATE"
printf 'prior-planet-file\n' > "$TEST_CONFIG/planet"
printf 'upload-before-read-failure\n' > "$TEST_TMP/planet-upload"
TEST_NVRAM_GET_FAIL=1
export TEST_NVRAM_GET_FAIL
if zerotier_import_planet_file "$TEST_TMP/planet-upload"; then
	fail "planet import accepted a failed old-value read"
fi
TEST_NVRAM_GET_FAIL=0
export TEST_NVRAM_GET_FAIL
[ "$(cat "$TEST_CONFIG/planet")" = "prior-planet-file" ] ||
	fail "failed NVRAM read changed the persistent planet"
[ "$(cat "$TEST_NVRAM_PLANET_STATE")" = "old-planet-state" ] ||
	fail "failed NVRAM read changed the existing NVRAM value"
[ -f "$TEST_TMP/planet-upload" ] ||
	fail "failed NVRAM read deleted the upload"
set -- "$TEST_CONFIG"/.planet-import.*
[ ! -e "$1" ] || fail "failed NVRAM read leaked the private import directory"

# Uploaded planet imports validate both source and destination, stage the file
# privately, and keep the NVRAM copy synchronized with rollback on failure.
printf 'old-planet-state' > "$TEST_NVRAM_PLANET_STATE"
printf 'prior-planet-file\n' > "$TEST_CONFIG/planet"
printf 'uploaded-planet-data\n' > "$TEST_TMP/planet-upload"
expected_planet_nvram="$(base64 "$TEST_TMP/planet-upload")"
zerotier_import_planet_file "$TEST_TMP/planet-upload" ||
	fail "valid uploaded planet import failed"
[ "$(cat "$TEST_CONFIG/planet")" = "uploaded-planet-data" ] ||
	fail "uploaded planet contents were not published"
[ "$(stat -c '%a' "$TEST_CONFIG/planet")" = 600 ] ||
	fail "uploaded planet target was not mode 600"
[ "$(cat "$TEST_NVRAM_PLANET_STATE")" = "$expected_planet_nvram" ] ||
	fail "uploaded planet NVRAM encoding mismatch"
[ ! -e "$TEST_TMP/planet-upload" ] ||
	fail "successfully imported planet upload was not removed"

# A concurrent upload replacement after the private snapshot is created must
# not split the disk file from the Base64 value persisted in NVRAM.
printf 'snapshot-before-mutation\n' > "$TEST_TMP/planet-upload-race"
expected_race_planet_nvram="$(command base64 "$TEST_TMP/planet-upload-race")"
TEST_PLANET_RACE_SOURCE="$TEST_TMP/planet-upload-race"
TEST_PLANET_MUTATE_AFTER_BASE64=1
base64() {
	command base64 "$@" || return $?
	if [ "${TEST_PLANET_MUTATE_AFTER_BASE64:-0}" = 1 ]; then
		TEST_PLANET_MUTATE_AFTER_BASE64=0
		printf 'changed-after-snapshot\n' > "$TEST_PLANET_RACE_SOURCE"
	fi
}
zerotier_import_planet_file "$TEST_PLANET_RACE_SOURCE" ||
	fail "planet import failed while the upload changed after snapshot"
unset -f base64
TEST_PLANET_MUTATE_AFTER_BASE64=0
[ "$(cat "$TEST_CONFIG/planet")" = "snapshot-before-mutation" ] ||
	fail "planet import published a file that differs from the pre-encoding upload snapshot"
[ "$(cat "$TEST_NVRAM_PLANET_STATE")" = "$expected_race_planet_nvram" ] ||
	fail "planet import NVRAM does not encode the published snapshot"
[ ! -e "$TEST_PLANET_RACE_SOURCE" ] ||
	fail "successful snapshot import did not remove the upload source"

# Empty uploads clear both the disk file and the NVRAM value.
: > "$TEST_TMP/planet-upload-empty"
zerotier_import_planet_file "$TEST_TMP/planet-upload-empty" ||
	fail "empty uploaded planet did not clear configuration"
[ ! -e "$TEST_CONFIG/planet" ] || fail "empty planet import left the old file"
[ ! -s "$TEST_NVRAM_PLANET_STATE" ] || fail "empty planet import left stale NVRAM data"
[ ! -e "$TEST_TMP/planet-upload-empty" ] || fail "empty planet upload was not removed"

# Symlink upload inputs are rejected without reading or deleting their targets.
printf 'untouched-upload-target\n' > "$TEST_TMP/planet-upload-sentinel"
ln -s "$TEST_TMP/planet-upload-sentinel" "$TEST_TMP/planet-upload-link"
printf 'old-planet-state' > "$TEST_NVRAM_PLANET_STATE"
printf 'prior-planet-file\n' > "$TEST_CONFIG/planet"
if zerotier_import_planet_file "$TEST_TMP/planet-upload-link"; then
	fail "symlink uploaded planet source was accepted"
fi
[ "$(cat "$TEST_TMP/planet-upload-sentinel")" = "untouched-upload-target" ] ||
	fail "symlink upload referent was modified"
[ "$(cat "$TEST_CONFIG/planet")" = "prior-planet-file" ] ||
	fail "rejected symlink upload changed the target planet"
[ "$(cat "$TEST_NVRAM_PLANET_STATE")" = "old-planet-state" ] ||
	fail "rejected symlink upload changed NVRAM"

# A failed NVRAM commit after the disk rename restores the old file and value.
printf 'new-planet-after-failure\n' > "$TEST_TMP/planet-upload"
TEST_NVRAM_COMMIT_FAIL=1
export TEST_NVRAM_COMMIT_FAIL
if zerotier_import_planet_file "$TEST_TMP/planet-upload"; then
	fail "uploaded planet NVRAM commit failure was accepted"
fi
TEST_NVRAM_COMMIT_FAIL=0
export TEST_NVRAM_COMMIT_FAIL
[ "$(cat "$TEST_CONFIG/planet")" = "prior-planet-file" ] ||
	fail "failed planet NVRAM commit did not restore the previous file"
[ "$(cat "$TEST_NVRAM_PLANET_STATE")" = "old-planet-state" ] ||
	fail "failed planet NVRAM commit did not restore the old value"
set -- "$TEST_CONFIG"/.planet-import.*
[ -d "$1" ] || fail "failed planet NVRAM rollback lost its recovery directory"
[ "$(cat "$1/planet.nvram.previous")" = "old-planet-state" ] ||
	fail "failed planet NVRAM rollback did not retain its old-value recovery snapshot"
[ "$(stat -c '%a' "$1/planet.nvram.previous")" = 600 ] ||
	fail "planet NVRAM recovery snapshot was not private"
recovery_dir="$1"
[ -f "$TEST_TMP/planet-upload" ] ||
	fail "failed planet import deleted the upload before commit"
rm -rf "$recovery_dir"

# Fail the first publish rename and prove no destination or NVRAM mutation.
mv() {
	case "$*" in
		*".planet-import."*"planet.new"*) return 1 ;;
	esac
	command mv "$@"
}
if zerotier_import_planet_file "$TEST_TMP/planet-upload"; then
	fail "planet publish rename failure was accepted"
fi
unset -f mv
[ "$(cat "$TEST_CONFIG/planet")" = "prior-planet-file" ] ||
	fail "failed planet publish changed the prior file"
[ "$(cat "$TEST_NVRAM_PLANET_STATE")" = "old-planet-state" ] ||
	fail "failed planet publish changed NVRAM"
[ -f "$TEST_TMP/planet-upload" ] ||
	fail "failed planet publish deleted its upload"
set -- "$TEST_CONFIG"/.planet-import.*
[ ! -e "$1" ] || fail "failed planet publish rollback leaked its recovery directory"

umask "$saved_umask"

# The rm test double logs staging-directory cleanup; isolate later lifecycle assertions.
: > "$TEST_ACTION_LOG"
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

# Legacy installs may have firewall rules but no cleanup checkpoint. When the
# virtual interface has disappeared, never assume an unknown ruleset is clean.
/bin/rm -rf "$TEST_CONFIG"
mkdir -p "$TEST_CONFIG"
echo identity > "$TEST_CONFIG/identity.secret"
test_proc_reset
test_proc_add 4340 "$PROG" "$PROG"
echo running > "$TEST_STATE"
TEST_MODE=graceful
TEST_IFCONFIG_MODE=none
TEST_ROUTE_MODE=none
TEST_ROUTE_PRESENT=0
TEST_ROUTE_SHOW_OUTPUT=""
TEST_NAT_ENABLE=0
TEST_SAVED_STATIC_ROUTE_COUNT="$TEST_STATIC_ROUTE_COUNT"
TEST_SAVED_STATIC_ROUTE_ENABLE="$TEST_STATIC_ROUTE_ENABLE"
TEST_STATIC_ROUTE_COUNT=0
TEST_STATIC_ROUTE_ENABLE=0
TEST_NVRAM_READ_FAIL_KEY=""
zt0=""
ip_segment=""
zt_firewall_state_checkpoint_loaded=0
: > "$TEST_KILL_LOG"
: > "$TEST_ACTION_LOG"
: > "$TEST_IPTABLES_LOG"
: > "$TEST_IPTABLES_STATE_FILE"
test_seed_firewall_rules
: > "$TEST_IPTABLES_LOG"
if stop_zero; then
	fail "stop accepted an unknown firewall cleanup target"
fi
[ "$(cat "$TEST_STATE")" = "running" ] || fail "unknown firewall target stopped the daemon"
[ ! -s "$TEST_KILL_LOG" ] || fail "unknown firewall target still signalled the daemon"
[ ! -s "$TEST_IPTABLES_LOG" ] || fail "unknown firewall target changed firewall rules"
[ -f "$TEST_CONFIG/identity.secret" ] || fail "unknown firewall target removed persistent identity"
[ -s "$TEST_IPTABLES_STATE_FILE" ] || fail "unknown firewall target discarded still-published rules"
[ ! -e "$TEST_CONFIG/.firewall-state" ] || fail "unknown firewall target fabricated a cleanup checkpoint"
if del_rules; then
	fail "del_rules accepted an unknown firewall cleanup target"
fi
[ ! -s "$TEST_IPTABLES_LOG" ] || fail "del_rules issued firewall commands without a cleanup target"
[ ! -s "$TEST_ACTION_LOG" ] || fail "unknown firewall target changed route state"

# Restore the shared route fixture for the following successful-stop case.
TEST_STATIC_ROUTE_COUNT="$TEST_SAVED_STATIC_ROUTE_COUNT"
TEST_STATIC_ROUTE_ENABLE="$TEST_SAVED_STATIC_ROUTE_ENABLE"

: > "$TEST_KILL_LOG"
: > "$TEST_SLEEP_LOG"
: > "$TEST_ACTION_LOG"
echo running > "$TEST_STATE"
TEST_IFCONFIG_MODE=running-only
TEST_ROUTE_MODE=daemon-only
TEST_ROUTE_PRESENT=1
TEST_ROUTE_SHOW_STATUS=0
TEST_ROUTE_DEL_STATUS=0
TEST_ROUTE_DELETE_PERSISTS=0
TEST_NAT_ENABLE=1
TEST_EXPECT_SAVED_SEGMENT=1
ip_segment=""
: > "$TEST_IPTABLES_STATE_FILE"
test_seed_firewall_rules
: > "$TEST_IPTABLES_LOG"
TEST_MODE=graceful
if ! stop_zero; then
	fail "stop failed after a confirmed graceful daemon exit"
fi
[ ! -e "$TEST_CONFIG" ] || fail "successful stop did not complete the existing config cleanup"
[ "$(cat "$TEST_ACTION_LOG")" = "$(printf 'zero_route:del\nrm:-rf %s' "$TEST_CONFIG")" ] || fail "successful cleanup did not run after daemon absence"
[ ! -s "$TEST_IPTABLES_STATE_FILE" ] || fail "successful stop left stale firewall rules"
for deleted_rule in \
	"-D INPUT -i ztunit -j ACCEPT" \
	"-D FORWARD -i ztunit -o ztunit -j ACCEPT" \
	"-D FORWARD -i ztunit -j ACCEPT" \
	"-t nat -D POSTROUTING -o ztunit -j MASQUERADE" \
	"-t nat -D POSTROUTING -s 10.147.17.0/24 -j MASQUERADE"
do
	if ! grep -Fqx -- "$deleted_rule" "$TEST_IPTABLES_LOG"; then
		echo "ZeroTier iptables cleanup trace (missing delete: $deleted_rule):" >&2
		nl -ba "$TEST_IPTABLES_LOG" >&2
		echo "Remaining fake firewall state:" >&2
		cat "$TEST_IPTABLES_STATE_FILE" >&2
		fail "successful stop omitted a required filter/NAT deletion"
	fi
done
deleted_firewall_rule_count=$(grep -cE '(^-D |^-t nat -D )' "$TEST_IPTABLES_LOG" || true)
if [ "$deleted_firewall_rule_count" -ne 5 ]; then
	echo "ZeroTier iptables cleanup trace (expected five delete commands, found $deleted_firewall_rule_count):" >&2
	nl -ba "$TEST_IPTABLES_LOG" >&2
	echo "Remaining fake firewall state:" >&2
	cat "$TEST_IPTABLES_STATE_FILE" >&2
	fail "successful stop did not delete all five installed firewall rules"
fi
TEST_EXPECT_SAVED_SEGMENT=0
TEST_ROUTE_MODE=none

# The checkpoint records the source segment used by the installed NAT rule.
# A live route can change while that older rule remains; stop must remove the
# saved tuple rather than replace it with the current route before deletion.
/bin/rm -rf "$TEST_CONFIG"
mkdir -p "$TEST_CONFIG"
: > "$TEST_IPTABLES_STATE_FILE"
: > "$TEST_IPTABLES_LOG"
: > "$TEST_ACTION_LOG"
TEST_STATIC_ROUTE_COUNT=0
TEST_STATIC_ROUTE_ENABLE=0
TEST_ROUTE_PRESENT=1
TEST_ROUTE_SHOW_STATUS=0
TEST_ROUTE_DEL_STATUS=0
TEST_ROUTE_DELETE_PERSISTS=0
TEST_NAT_ENABLE=0
TEST_IFCONFIG_MODE=running-only
TEST_ROUTE_MODE=daemon-only
TEST_ROUTE_SEGMENT=10.148.18.0/24
TEST_EXPECT_SAVED_SEGMENT=1
TEST_IPTABLES_FAIL_ON=""
TEST_IPTABLES_CHECK_FAIL_ON=""
TEST_IPTABLES_DELETE_FAIL_ON=""
test_seed_firewall_rules
printf '%s\n' 'ztunit' '10.147.17.0/24' > "$TEST_CONFIG/.firewall-state"
chmod 600 "$TEST_CONFIG/.firewall-state"
echo identity > "$TEST_CONFIG/identity.secret"
echo running > "$TEST_STATE"
TEST_MODE=graceful
test_proc_reset
test_proc_add 4330 "$PROG" "$PROG"
if ! stop_zero; then
	fail "stop failed while replaying the saved firewall tuple after the live route changed"
fi
[ ! -s "$TEST_IPTABLES_STATE_FILE" ] || fail "stop left the firewall rule tied to the saved NAT source segment"
grep -Fqx -- "-t nat -D POSTROUTING -s 10.147.17.0/24 -j MASQUERADE" "$TEST_IPTABLES_LOG" || fail "stop did not delete the saved NAT source segment"
if grep -Fqx -- "-t nat -D POSTROUTING -s 10.148.18.0/24 -j MASQUERADE" "$TEST_IPTABLES_LOG"; then
	fail "stop deleted the newly observed route instead of the saved firewall source segment"
fi
[ ! -e "$TEST_CONFIG" ] || fail "stop retained config after saved firewall tuple cleanup"
TEST_EXPECT_SAVED_SEGMENT=0
TEST_ROUTE_SEGMENT=""
TEST_ROUTE_MODE=none

# An empty saved source segment is authoritative: this ruleset installed no
# per-source NAT. Preserve a matching rule belonging to the current live route.
/bin/rm -rf "$TEST_CONFIG"
mkdir -p "$TEST_CONFIG"
: > "$TEST_IPTABLES_STATE_FILE"
: > "$TEST_IPTABLES_LOG"
: > "$TEST_ACTION_LOG"
TEST_STATIC_ROUTE_COUNT=0
TEST_STATIC_ROUTE_ENABLE=0
TEST_ROUTE_PRESENT=1
TEST_ROUTE_SHOW_STATUS=0
TEST_ROUTE_DEL_STATUS=0
TEST_ROUTE_DELETE_PERSISTS=0
TEST_NAT_ENABLE=0
TEST_IFCONFIG_MODE=running-only
TEST_ROUTE_MODE=daemon-only
TEST_ROUTE_SEGMENT=10.148.18.0/24
TEST_EXPECT_SAVED_SEGMENT=0
TEST_IPTABLES_FAIL_ON=""
TEST_IPTABLES_CHECK_FAIL_ON=""
TEST_IPTABLES_DELETE_FAIL_ON=""
iptables -A INPUT -i ztunit -j ACCEPT
iptables -A FORWARD -i ztunit -o ztunit -j ACCEPT
iptables -A FORWARD -i ztunit -j ACCEPT
iptables -t nat -A POSTROUTING -o ztunit -j MASQUERADE
iptables -t nat -A POSTROUTING -s 10.148.18.0/24 -j MASQUERADE
printf '%s\n%s\n' 'ztunit' '' > "$TEST_CONFIG/.firewall-state"
chmod 600 "$TEST_CONFIG/.firewall-state"
echo identity > "$TEST_CONFIG/identity.secret"
echo running > "$TEST_STATE"
TEST_MODE=graceful
test_proc_reset
test_proc_add 4331 "$PROG" "$PROG"
if ! stop_zero; then
	fail "stop failed while replaying a valid checkpoint with an empty source segment"
fi
[ "$(cat "$TEST_IPTABLES_STATE_FILE")" = "nat POSTROUTING -s 10.148.18.0/24 -j MASQUERADE" ] || fail "empty checkpoint cleanup removed an unrelated source-NAT rule"
if grep -E -- '^-t nat -(C|D) POSTROUTING -s 10\.148\.18\.0/24 -j MASQUERADE' "$TEST_IPTABLES_LOG" >/dev/null; then
	fail "empty checkpoint cleanup consulted or deleted the source segment from the current route"
fi
[ ! -e "$TEST_CONFIG" ] || fail "empty checkpoint cleanup retained config after deleting only owned rules"
TEST_ROUTE_SEGMENT=""
TEST_ROUTE_MODE=none

# A firewall deletion failure must retain identity plus the exact interface/
# route checkpoint, and a later stop must finish cleanup after the daemon and
# virtual interface have disappeared.
TEST_STATIC_ROUTE_COUNT=1
TEST_STATIC_ROUTE_ENABLE=1
TEST_ROUTE_PRESENT=1
TEST_ROUTE_SHOW_STATUS=0
TEST_ROUTE_DEL_STATUS=0
TEST_ROUTE_DELETE_PERSISTS=0
TEST_NAT_ENABLE=1
TEST_IFCONFIG_MODE=running-only
TEST_ROUTE_MODE=daemon-only
TEST_EXPECT_SAVED_SEGMENT=1
TEST_IPTABLES_DELETE_FAIL_ON="filter INPUT -i ztunit -j ACCEPT"
: > "$TEST_IPTABLES_STATE_FILE"
test_seed_firewall_rules
: > "$TEST_IPTABLES_LOG"
: > "$TEST_ACTION_LOG"
mkdir -p "$TEST_CONFIG"
echo identity > "$TEST_CONFIG/identity.secret"
echo running > "$TEST_STATE"
TEST_MODE=graceful
if stop_zero; then
	fail "stop accepted iptables deletion failure"
fi
[ -f "$TEST_CONFIG/identity.secret" ] || fail "firewall deletion failure removed the persistent identity"
[ -f "$TEST_CONFIG/.firewall-state" ] || fail "firewall deletion failure did not persist retry state"
[ "$(cat "$TEST_CONFIG/.firewall-state")" = "$(printf 'ztunit\n10.147.17.0/24')" ] || fail "firewall retry state did not preserve exact interface and NAT segment"
[ "$(cat "$TEST_IPTABLES_STATE_FILE")" = "filter INPUT -i ztunit -j ACCEPT" ] || fail "firewall cleanup failure did not continue deleting independent rules"
if grep -Fqx -- "zero_route:del" "$TEST_ACTION_LOG"; then
	fail "stop continued to delete static routes after firewall cleanup failed"
fi

TEST_IPTABLES_DELETE_FAIL_ON=""
echo stopped > "$TEST_STATE"
TEST_IFCONFIG_MODE=none
TEST_ROUTE_MODE=none
zt0=""
ip_segment=""
: > "$TEST_ACTION_LOG"
: > "$TEST_IPTABLES_LOG"
if ! stop_zero; then
	fail "stop could not retry firewall cleanup from saved state after daemon exit"
fi
[ ! -e "$TEST_CONFIG" ] || fail "successful retry retained persistent config"
[ ! -s "$TEST_IPTABLES_STATE_FILE" ] || fail "successful retry left a firewall rule"
grep -Fqx -- "-D INPUT -i ztunit -j ACCEPT" "$TEST_IPTABLES_LOG" || fail "retry did not use saved ZeroTier interface identity"
grep -Fqx -- "-t nat -C POSTROUTING -s 10.147.17.0/24 -j MASQUERADE" "$TEST_IPTABLES_LOG" || fail "retry did not check the saved NAT source segment"

# A replacement daemon must replay the previous daemon's saved firewall/route
# state first. A partial cleanup failure blocks startup and preserves the checkpoint.
TEST_STATIC_ROUTE_COUNT=1
TEST_STATIC_ROUTE_ENABLE=1
TEST_ROUTE_PRESENT=1
TEST_ROUTE_SHOW_STATUS=0
TEST_ROUTE_DEL_STATUS=0
TEST_ROUTE_DELETE_PERSISTS=0
TEST_NAT_ENABLE=1
TEST_IFCONFIG_MODE=none
TEST_ROUTE_MODE=none
TEST_EXPECT_SAVED_SEGMENT=1
TEST_IPTABLES_DELETE_FAIL_ON="filter INPUT -i ztunit -j ACCEPT"
: > "$TEST_IPTABLES_STATE_FILE"
test_seed_firewall_rules
: > "$TEST_ACTION_LOG"
mkdir -p "$TEST_CONFIG"
printf 'ztunit\n10.147.17.0/24\n' > "$TEST_CONFIG/.firewall-state"
chmod 600 "$TEST_CONFIG/.firewall-state"
echo stopped > "$TEST_STATE"
TEST_MODE=none
TEST_ASSERT_CLEAN_BEFORE_START=1
if start_zero; then
	fail "replacement daemon started despite incomplete stale firewall cleanup"
fi
[ -f "$TEST_CONFIG/.firewall-state" ] || fail "failed replay discarded its retry checkpoint"
[ "$(cat "$TEST_IPTABLES_STATE_FILE")" = "filter INPUT -i ztunit -j ACCEPT" ] || fail "failed replay did not retain the undeleted firewall rule"
[ "$TEST_ROUTE_PRESENT" -eq 1 ] || fail "failed firewall cleanup proceeded to delete the saved route"
[ ! -s "$TEST_ACTION_LOG" ] || fail "replacement daemon started after stale-state cleanup failed"

TEST_IPTABLES_DELETE_FAIL_ON=""
: > "$TEST_ACTION_LOG"
if ! start_zero; then
	fail "replacement daemon could not start after stale network cleanup recovered"
fi
[ ! -e "$TEST_CONFIG/.firewall-state" ] || fail "successful stale-state replay retained its checkpoint"
[ ! -s "$TEST_IPTABLES_STATE_FILE" ] || fail "successful stale-state replay left old firewall rules"
[ "$TEST_ROUTE_PRESENT" -eq 0 ] || fail "successful stale-state replay left the old configured route"
[ "$(grep -c '^start$' "$TEST_ACTION_LOG")" = 1 ] || fail "replacement daemon did not start exactly once after cleanup"

# NAT-enabled shutdown must fail before signalling the daemon if its connected
# route cannot be captured for source-NAT firewall cleanup.
TEST_STATIC_ROUTE_COUNT=0
TEST_STATIC_ROUTE_ENABLE=0
TEST_NAT_ENABLE=1
TEST_IFCONFIG_MODE=running-only
TEST_ROUTE_MODE=none
: > "$TEST_KILL_LOG"
: > "$TEST_ACTION_LOG"
mkdir -p "$TEST_CONFIG"
echo identity > "$TEST_CONFIG/identity.secret"
echo running > "$TEST_STATE"
TEST_MODE=graceful
if stop_zero; then
	fail "stop accepted NAT-enabled shutdown without a connected route segment"
fi
[ "$(cat "$TEST_STATE")" = running ] || fail "missing NAT route stopped the daemon before cleanup could be guaranteed"
[ ! -s "$TEST_KILL_LOG" ] || fail "missing NAT route still signalled the daemon"
[ ! -s "$TEST_ACTION_LOG" ] || fail "missing NAT route changed firewall or persistent configuration"
[ -f "$TEST_CONFIG/identity.secret" ] || fail "missing NAT route removed the persistent identity"
TEST_NAT_ENABLE=0

# iproute2 may print a host route as a bare IPv4 address; all other
# saved source segments must be canonical four-octet IPv4 with an optional
# decimal CIDR prefix in the range 0..32.
for valid_segment in "0.0.0.0/0" "10.147.17.0/24" "10.147.17.1"; do
	if ! zerotier_ipv4_cidr_is_valid "$valid_segment"; then
		fail "valid kernel route segment was rejected: $valid_segment"
	fi
done
for invalid_segment in \
	"10.147.017.0/24" \
	"999.147.17.0/24" \
	"10.147.17/24" \
	"10.147.17.0/33" \
	"10.147.17.0/024" \
	"10.147.17.0/24/1"; do
	if zerotier_ipv4_cidr_is_valid "$invalid_segment"; then
		fail "malformed kernel route segment was accepted: $invalid_segment"
	fi
done

# A trailing corrupt record must not strand a daemon when the first two
# newline-terminated checkpoint fields still identify the exact installed rules.
# Stop sanitizes this validated prefix before signalling, then keeps that
# canonical checkpoint if firewall deletion needs a later retry.
/bin/rm -rf "$TEST_CONFIG"
mkdir -p "$TEST_CONFIG"
: > "$TEST_IPTABLES_LOG"
: > "$TEST_IPTABLES_STATE_FILE"
TEST_IPTABLES_FAIL_ON=""
TEST_IPTABLES_CHECK_FAIL_ON=""
TEST_IPTABLES_DELETE_FAIL_ON="filter INPUT -i ztunit -j ACCEPT"
test_seed_firewall_rules
: > "$TEST_IPTABLES_LOG"
printf '%s\n' 'ztunit' '10.147.17.0/24' 'corrupt-extra-record' > "$TEST_CONFIG/.firewall-state"
echo identity > "$TEST_CONFIG/identity.secret"
TEST_STATIC_ROUTE_COUNT=0
TEST_NAT_ENABLE=0
TEST_IFCONFIG_MODE=running-only
TEST_ROUTE_MODE=none
TEST_EXPECT_SAVED_SEGMENT=1
TEST_EXPECT_CANONICAL_CHECKPOINT=1
: > "$TEST_KILL_LOG"
: > "$TEST_ACTION_LOG"
test_proc_reset
test_proc_add 4321 "$PROG" "$PROG"
echo running > "$TEST_STATE"
TEST_MODE=graceful
if stop_zero; then
	fail "stop ignored a firewall deletion failure after recovering a valid checkpoint prefix"
fi
[ "$(cat "$TEST_STATE")" = stopped ] || fail "checkpoint prefix recovery did not stop the identified daemon"
[ "$(cat "$TEST_KILL_LOG")" = "-TERM 4321" ] || fail "checkpoint prefix recovery did not signal only the owned daemon"
[ -f "$TEST_CONFIG/identity.secret" ] || fail "partial recovered-checkpoint cleanup removed persistent identity"
[ "$(cat "$TEST_CONFIG/.firewall-state")" = "$(printf 'ztunit\n10.147.17.0/24')" ] || fail "recovered checkpoint did not retain its canonical retry tuple"
[ "$(wc -l < "$TEST_CONFIG/.firewall-state" | tr -d ' ')" = 2 ] || fail "recovered checkpoint retained trailing records"
[ "$(cat "$TEST_IPTABLES_STATE_FILE")" = "filter INPUT -i ztunit -j ACCEPT" ] || fail "recovered checkpoint cleanup did not continue independent firewall deletions"
grep -Fqx -- "-t nat -D POSTROUTING -s 10.147.17.0/24 -j MASQUERADE" "$TEST_IPTABLES_LOG" || fail "recovery did not delete the saved source-NAT rule after the live route disappeared"
if grep -Fqx -- "zero_route:del" "$TEST_ACTION_LOG"; then
	fail "stop continued to remove static routes after recovered firewall deletion failed"
fi

TEST_IPTABLES_DELETE_FAIL_ON=""
TEST_IFCONFIG_MODE=none
TEST_ROUTE_MODE=none
zt0=""
ip_segment=""
: > "$TEST_KILL_LOG"
: > "$TEST_ACTION_LOG"
: > "$TEST_IPTABLES_LOG"
if ! stop_zero; then
	fail "stop could not retry cleanup from the canonicalized recovered checkpoint"
fi
[ ! -e "$TEST_CONFIG" ] || fail "successful recovered-checkpoint retry retained persistent config"
[ ! -s "$TEST_IPTABLES_STATE_FILE" ] || fail "successful recovered-checkpoint retry left a firewall rule"
grep -Fqx -- "-t nat -C POSTROUTING -s 10.147.17.0/24 -j MASQUERADE" "$TEST_IPTABLES_LOG" || fail "recovered-checkpoint retry did not check the saved NAT segment"
TEST_EXPECT_CANONICAL_CHECKPOINT=0
TEST_EXPECT_SAVED_SEGMENT=0
: > "$TEST_IPTABLES_STATE_FILE"
: > "$TEST_IPTABLES_LOG"
test_proc_reset
TEST_IFCONFIG_MODE=none
TEST_ROUTE_MODE=none
TEST_NAT_ENABLE=0

# Numeric-looking but semantically invalid tuples must fail before stop can
# signal the daemon, rewrite the checkpoint, or mutate firewall state.
for malformed_segment in \
	"10.147.017.0/24" \
	"999.147.17.0/24" \
	"10.147.17/24" \
	"10.147.17.0/33"; do
	/bin/rm -rf "$TEST_CONFIG"
	mkdir -p "$TEST_CONFIG"
	: > "$TEST_IPTABLES_STATE_FILE"
	: > "$TEST_IPTABLES_LOG"
	: > "$TEST_ACTION_LOG"
	: > "$TEST_KILL_LOG"
	TEST_EXPECT_SAVED_SEGMENT=1
	test_seed_firewall_rules
	: > "$TEST_IPTABLES_LOG"
	printf '%s\n' 'ztunit' "$malformed_segment" > "$TEST_CONFIG/.firewall-state"
	echo identity > "$TEST_CONFIG/identity.secret"
	echo running > "$TEST_STATE"
	TEST_STATIC_ROUTE_COUNT=0
	TEST_STATIC_ROUTE_ENABLE=0
	TEST_NAT_ENABLE=0
	TEST_IFCONFIG_MODE=running-only
	TEST_ROUTE_MODE=none
	zt0=""
	ip_segment=""
	zt_firewall_state_checkpoint_loaded=0
	test_proc_reset
	test_proc_add 4333 "$PROG" "$PROG"
	TEST_MODE=graceful
	if stop_zero; then
		fail "stop accepted malformed source-NAT checkpoint tuple: $malformed_segment"
	fi
	[ "$(cat "$TEST_STATE")" = running ] || fail "malformed checkpoint stopped the daemon: $malformed_segment"
	[ ! -s "$TEST_KILL_LOG" ] || fail "malformed checkpoint signalled the daemon: $malformed_segment"
	[ ! -s "$TEST_IPTABLES_LOG" ] || fail "malformed checkpoint changed firewall state: $malformed_segment"
	[ -f "$TEST_CONFIG/identity.secret" ] || fail "malformed checkpoint removed the persistent identity: $malformed_segment"
	[ "$(sed -n '2p' "$TEST_CONFIG/.firewall-state")" = "$malformed_segment" ] || fail "malformed checkpoint was rewritten: $malformed_segment"
	[ ! -s "$TEST_ACTION_LOG" ] || fail "malformed checkpoint changed persistent configuration: $malformed_segment"
done

# A truncated second field lacks its terminating LF and is never recoverable,
# even when its visible bytes look like a valid source-NAT tuple.
/bin/rm -rf "$TEST_CONFIG"
mkdir -p "$TEST_CONFIG"
: > "$TEST_IPTABLES_LOG"
: > "$TEST_IPTABLES_STATE_FILE"
test_seed_firewall_rules
: > "$TEST_IPTABLES_LOG"
printf '%s\n%s' 'ztunit' '10.147.17.0/24' > "$TEST_CONFIG/.firewall-state"
echo identity > "$TEST_CONFIG/identity.secret"
echo running > "$TEST_STATE"
TEST_STATIC_ROUTE_COUNT=0
TEST_STATIC_ROUTE_ENABLE=0
TEST_NAT_ENABLE=0
TEST_IFCONFIG_MODE=running-only
TEST_ROUTE_MODE=none
TEST_EXPECT_SAVED_SEGMENT=1
: > "$TEST_KILL_LOG"
: > "$TEST_ACTION_LOG"
test_proc_reset
test_proc_add 4334 "$PROG" "$PROG"
TEST_MODE=graceful
if stop_zero; then
	fail "stop recovered a truncated checkpoint prefix"
fi
[ "$(cat "$TEST_STATE")" = running ] || fail "truncated checkpoint prefix stopped the daemon"
[ ! -s "$TEST_KILL_LOG" ] || fail "truncated checkpoint prefix signalled the daemon"
[ ! -s "$TEST_IPTABLES_LOG" ] || fail "truncated checkpoint prefix changed firewall state"
[ -f "$TEST_CONFIG/identity.secret" ] || fail "truncated checkpoint prefix removed persistent identity"
[ "$(cat "$TEST_CONFIG/.firewall-state")" = "$(printf 'ztunit\n10.147.17.0/24')" ] || fail "truncated checkpoint prefix was rewritten"
[ ! -s "$TEST_ACTION_LOG" ] || fail "truncated checkpoint prefix changed persistent configuration"
# An unterminated trailing fragment is not a clean EOF for the strict
# loader, and stop-only prefix recovery also requires a complete extra line.
/bin/rm -rf "$TEST_CONFIG"
mkdir -p "$TEST_CONFIG"
: > "$TEST_IPTABLES_STATE_FILE"
test_seed_firewall_rules
: > "$TEST_IPTABLES_LOG"
printf '%s\n%s\n%s' 'ztunit' '10.147.17.0/24' 'partial-tail' > "$TEST_CONFIG/.firewall-state"
echo identity > "$TEST_CONFIG/identity.secret"
echo running > "$TEST_STATE"
TEST_STATIC_ROUTE_COUNT=0
TEST_STATIC_ROUTE_ENABLE=0
TEST_NAT_ENABLE=0
TEST_IFCONFIG_MODE=running-only
TEST_ROUTE_MODE=none
TEST_EXPECT_SAVED_SEGMENT=1
: > "$TEST_KILL_LOG"
: > "$TEST_ACTION_LOG"
test_proc_reset
test_proc_add 4335 "$PROG" "$PROG"
TEST_MODE=graceful
if stop_zero; then
	fail "stop accepted an unterminated trailing checkpoint fragment"
fi
[ "$(cat "$TEST_STATE")" = running ] || fail "unterminated trailing fragment stopped the daemon"
[ ! -s "$TEST_KILL_LOG" ] || fail "unterminated trailing fragment signalled the daemon"
[ ! -s "$TEST_IPTABLES_LOG" ] || fail "unterminated trailing fragment changed firewall state"
[ -f "$TEST_CONFIG/identity.secret" ] || fail "unterminated trailing fragment removed persistent identity"
[ "$(cat "$TEST_CONFIG/.firewall-state")" = "$(printf 'ztunit\n10.147.17.0/24\npartial-tail')" ] || fail "unterminated trailing fragment was rewritten"
[ ! -s "$TEST_ACTION_LOG" ] || fail "unterminated trailing fragment changed persistent configuration"
TEST_EXPECT_SAVED_SEGMENT=0

# A damaged checkpoint must remain fail-closed when the live route inventory
# is UNKNOWN; absence cannot be inferred from a failed ip route command.
/bin/rm -rf "$TEST_CONFIG"
mkdir -p "$TEST_CONFIG"
: > "$TEST_IPTABLES_STATE_FILE"
: > "$TEST_IPTABLES_LOG"
printf '%s\n' 'ztunit' '10.147.17.0/24' 'corrupt-extra-record' > "$TEST_CONFIG/.firewall-state"
echo identity > "$TEST_CONFIG/identity.secret"
echo running > "$TEST_STATE"
TEST_STATIC_ROUTE_COUNT=0
TEST_STATIC_ROUTE_ENABLE=0
TEST_NAT_ENABLE=0
TEST_IFCONFIG_MODE=running-only
TEST_ROUTE_MODE=none
TEST_ROUTE_DUMP_STATUS=1
TEST_EXPECT_SAVED_SEGMENT=1
: > "$TEST_KILL_LOG"
: > "$TEST_ACTION_LOG"
test_proc_reset
test_proc_add 4336 "$PROG" "$PROG"
TEST_MODE=graceful
if stop_zero; then
	fail "stop recovered a damaged checkpoint with an unreadable live route inventory"
fi
[ "$(cat "$TEST_STATE")" = running ] || fail "unknown route inventory stopped the ZeroTier daemon"
[ ! -s "$TEST_KILL_LOG" ] || fail "unknown route inventory signalled the daemon"
[ ! -s "$TEST_IPTABLES_LOG" ] || fail "unknown route inventory edited firewall state"
[ -f "$TEST_CONFIG/identity.secret" ] || fail "unknown route inventory removed persistent identity"
grep -Fqx -- "corrupt-extra-record" "$TEST_CONFIG/.firewall-state" || fail "unknown route inventory normalized the checkpoint"
TEST_ROUTE_DUMP_STATUS=0
TEST_EXPECT_SAVED_SEGMENT=0

/bin/rm -rf "$TEST_CONFIG"
: > "$TEST_IPTABLES_STATE_FILE"
: > "$TEST_IPTABLES_LOG"
: > "$TEST_ACTION_LOG"
test_proc_reset
TEST_IFCONFIG_MODE=none
TEST_ROUTE_MODE=none
TEST_NAT_ENABLE=0

# A live route cannot reconstruct an older source-NAT segment either. A corrupt
# checkpoint must block shutdown before any rules are overwritten or signalled.
/bin/rm -rf "$TEST_CONFIG"
mkdir -p "$TEST_CONFIG"
: > "$TEST_IPTABLES_LOG"
: > "$TEST_IPTABLES_STATE_FILE"
TEST_IPTABLES_FAIL_ON=""
TEST_IPTABLES_CHECK_FAIL_ON=""
TEST_IPTABLES_DELETE_FAIL_ON=""
test_seed_firewall_rules
printf '%s\n' 'nat POSTROUTING -s 10.148.18.0/24 -j MASQUERADE' >> "$TEST_IPTABLES_STATE_FILE"
: > "$TEST_IPTABLES_LOG"
printf '%s\n' 'ztunit' '10.147.17.0/24' 'corrupt-extra-record' > "$TEST_CONFIG/.firewall-state"
TEST_STATIC_ROUTE_COUNT=0
TEST_STATIC_ROUTE_ENABLE=0
TEST_NAT_ENABLE=0
TEST_IFCONFIG_MODE=running-only
TEST_ROUTE_MODE=daemon-only
echo 0 > "$TEST_ROUTE_COUNT"
zt0=""
ip_segment=""
: > "$TEST_KILL_LOG"
: > "$TEST_ACTION_LOG"
test_proc_reset
test_proc_add 4322 "$PROG" "$PROG"
echo running > "$TEST_STATE"
TEST_MODE=graceful
if stop_zero; then
	fail "stop accepted a corrupt firewall checkpoint with a live route and unknown old source-NAT segment"
fi
[ "$(cat "$TEST_STATE")" = running ] || fail "live-route corrupt checkpoint stopped the daemon"
[ ! -s "$TEST_KILL_LOG" ] || fail "live-route corrupt checkpoint still signalled the daemon"
[ ! -s "$TEST_IPTABLES_LOG" ] || fail "live-route corrupt checkpoint attempted firewall edits"
grep -Fqx -- "nat POSTROUTING -s 10.148.18.0/24 -j MASQUERADE" "$TEST_IPTABLES_STATE_FILE" || fail "live-route corrupt checkpoint lost the older source-NAT rule"
[ "$(wc -l < "$TEST_CONFIG/.firewall-state" | tr -d ' ')" = 3 ] || fail "live-route corrupt checkpoint was overwritten or removed"
grep -Fqx -- "corrupt-extra-record" "$TEST_CONFIG/.firewall-state" || fail "live-route corrupt checkpoint marker was removed"
/bin/rm -rf "$TEST_CONFIG"
: > "$TEST_IPTABLES_LOG"
: > "$TEST_IPTABLES_STATE_FILE"

# A non-kernel route (for example a configured static route) on the saved
# interface is also enough to keep damaged-prefix recovery fail-closed.
mkdir -p "$TEST_CONFIG"
echo identity > "$TEST_CONFIG/identity.secret"
test_seed_firewall_rules
printf '%s\n' 'nat POSTROUTING -s 10.148.18.0/24 -j MASQUERADE' >> "$TEST_IPTABLES_STATE_FILE"
: > "$TEST_IPTABLES_LOG"
printf '%s\n' 'ztunit' '10.147.17.0/24' 'corrupt-extra-record' > "$TEST_CONFIG/.firewall-state"
TEST_STATIC_ROUTE_COUNT=0
TEST_STATIC_ROUTE_ENABLE=0
TEST_NAT_ENABLE=0
TEST_IFCONFIG_MODE=running-only
TEST_ROUTE_MODE=daemon-static
TEST_ROUTE_DUMP_STATUS=0
echo 0 > "$TEST_ROUTE_COUNT"
: > "$TEST_KILL_LOG"
: > "$TEST_ACTION_LOG"
test_proc_reset
test_proc_add 4337 "$PROG" "$PROG"
echo running > "$TEST_STATE"
TEST_MODE=graceful
if stop_zero; then
	fail "stop recovered a damaged checkpoint despite a live static route on the saved interface"
fi
[ "$(cat "$TEST_STATE")" = running ] || fail "live static route on saved interface stopped the daemon"
[ ! -s "$TEST_KILL_LOG" ] || fail "live static route on saved interface signalled the daemon"
[ ! -s "$TEST_IPTABLES_LOG" ] || fail "live static route on saved interface edited firewall state"
grep -Fqx -- "nat POSTROUTING -s 10.148.18.0/24 -j MASQUERADE" "$TEST_IPTABLES_STATE_FILE" || fail "live static route fixture lost the older source-NAT rule"
grep -Fqx -- "corrupt-extra-record" "$TEST_CONFIG/.firewall-state" || fail "live static route normalized the corrupt checkpoint"
[ -f "$TEST_CONFIG/identity.secret" ] || fail "live static route removed persistent identity"
# A multipath route may contain multiple dev tokens on one line. Any
# nexthop using the saved interface must block damaged-prefix recovery.
/bin/rm -rf "$TEST_CONFIG"
mkdir -p "$TEST_CONFIG"
echo identity > "$TEST_CONFIG/identity.secret"
: > "$TEST_IPTABLES_STATE_FILE"
: > "$TEST_IPTABLES_LOG"
printf '%s\n' 'ztunit' '10.147.17.0/24' 'corrupt-extra-record' > "$TEST_CONFIG/.firewall-state"
TEST_STATIC_ROUTE_COUNT=0
TEST_STATIC_ROUTE_ENABLE=0
TEST_NAT_ENABLE=0
TEST_IFCONFIG_MODE=running-only
TEST_ROUTE_MODE=daemon-multipath
TEST_ROUTE_DUMP_STATUS=0
echo 0 > "$TEST_ROUTE_COUNT"
: > "$TEST_KILL_LOG"
: > "$TEST_ACTION_LOG"
test_proc_reset
test_proc_add 4338 "$PROG" "$PROG"
echo running > "$TEST_STATE"
TEST_MODE=graceful
if stop_zero; then
	fail "stop recovered a damaged checkpoint with a multipath nexthop on the saved interface"
fi
[ "$(cat "$TEST_STATE")" = running ] || fail "multipath route on saved interface stopped the daemon"
[ ! -s "$TEST_KILL_LOG" ] || fail "multipath route on saved interface signalled the daemon"
[ ! -s "$TEST_IPTABLES_LOG" ] || fail "multipath route on saved interface edited firewall state"
grep -Fqx -- "corrupt-extra-record" "$TEST_CONFIG/.firewall-state" || fail "multipath route normalized the damaged checkpoint"
[ -f "$TEST_CONFIG/identity.secret" ] || fail "multipath route removed persistent identity"
TEST_ROUTE_MODE=none
/bin/rm -rf "$TEST_CONFIG"
: > "$TEST_IPTABLES_LOG"
: > "$TEST_IPTABLES_STATE_FILE"
test_proc_reset
TEST_IFCONFIG_MODE=none

TEST_ROUTE_MODE=none
/bin/rm -rf "$TEST_CONFIG"
: > "$TEST_IPTABLES_LOG"
: > "$TEST_IPTABLES_STATE_FILE"
test_proc_reset
TEST_IFCONFIG_MODE=none

: > "$TEST_IPTABLES_STATE_FILE"
: > "$TEST_IPTABLES_LOG"
test_proc_reset
TEST_IFCONFIG_MODE=none
TEST_ROUTE_MODE=none
TEST_NAT_ENABLE=0

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

# A destination that still exists through a different nexthop is not absent.
# Stop must preserve identity rather than silently orphaning the conflicting route.
TEST_ROUTE_PRESENT=1
TEST_ROUTE_SHOW_OUTPUT="$TEST_STATIC_ROUTE_IP via 192.0.2.99 dev ztunit"
: > "$TEST_ACTION_LOG"
mkdir -p "$TEST_CONFIG"
echo identity > "$TEST_CONFIG/identity.secret"
echo running > "$TEST_STATE"
TEST_MODE=graceful
if stop_zero; then
	fail "stop accepted a conflicting route to the configured destination"
fi
[ -f "$TEST_CONFIG/identity.secret" ] || fail "conflicting static route removed the persistent identity"
if grep -Fqx -- "zero_route:del" "$TEST_ACTION_LOG"; then
	fail "stop deleted a destination route whose nexthop did not match configuration"
fi
if grep -Fqx -- "rm:-rf $TEST_CONFIG" "$TEST_ACTION_LOG"; then
	fail "stop removed persistent configuration while a conflicting destination route remained"
fi

# Linux 4.4 fib_nh_match() treats a gateway/device delete against a
# multipath FIB alias as a match on the first nexthop and removes the whole
# alias. Even when one nexthop is configured for ZeroTier, never delete the
# unrelated ECMP nexthops as collateral.
TEST_ROUTE_SHOW_OUTPUT="$TEST_STATIC_ROUTE_IP proto static nexthop via $TEST_STATIC_ROUTE_GATEWAY dev ztunit weight 1 nexthop via 192.0.2.2 dev br0 weight 1"
: > "$TEST_ACTION_LOG"
mkdir -p "$TEST_CONFIG"
echo identity > "$TEST_CONFIG/identity.secret"
echo running > "$TEST_STATE"
TEST_MODE=graceful
if stop_zero; then
	fail "stop accepted and deleted a multipath route containing a configured first nexthop"
fi
[ -f "$TEST_CONFIG/identity.secret" ] || fail "ambiguous multipath route removed the persistent identity"
if grep -Fqx -- "zero_route:del" "$TEST_ACTION_LOG"; then
	fail "cleanup issued a gateway/device delete that removes the full multipath FIB alias"
fi

# On-link ECMP legs have no "via" token, but are still separate
# nexthops and make the gateway/device delete ambiguous.
TEST_ROUTE_SHOW_OUTPUT="$TEST_STATIC_ROUTE_IP proto static nexthop via $TEST_STATIC_ROUTE_GATEWAY dev ztunit weight 1 nexthop dev br0 weight 1"
: > "$TEST_ACTION_LOG"
/bin/rm -rf "$TEST_CONFIG"
mkdir -p "$TEST_CONFIG"
echo identity > "$TEST_CONFIG/identity.secret"
echo running > "$TEST_STATE"
TEST_MODE=graceful
if stop_zero; then
	fail "stop accepted a multipath route with a configured gateway and an on-link second nexthop"
fi
[ -f "$TEST_CONFIG/identity.secret" ] || fail "on-link multipath ambiguity removed the persistent identity"
if grep -Fqx -- "zero_route:del" "$TEST_ACTION_LOG"; then
	fail "cleanup deleted a multipath route with an on-link ECMP leg"
fi

TEST_ROUTE_SHOW_OUTPUT="$TEST_STATIC_ROUTE_IP proto static nexthop dev br0 weight 1 nexthop via $TEST_STATIC_ROUTE_GATEWAY dev ztunit weight 1"
: > "$TEST_ACTION_LOG"
/bin/rm -rf "$TEST_CONFIG"
mkdir -p "$TEST_CONFIG"
echo identity > "$TEST_CONFIG/identity.secret"
echo running > "$TEST_STATE"
TEST_MODE=graceful
if stop_zero; then
	fail "stop accepted a multipath route with an on-link first nexthop and configured later gateway"
fi
[ -f "$TEST_CONFIG/identity.secret" ] || fail "on-link-first multipath ambiguity removed the persistent identity"
if grep -Fqx -- "zero_route:del" "$TEST_ACTION_LOG"; then
	fail "cleanup deleted a multipath route with an on-link first ECMP leg"
fi

# The same fail-closed rule applies when the configured nexthop is not first.
TEST_ROUTE_SHOW_OUTPUT="$TEST_STATIC_ROUTE_IP proto static nexthop via 192.0.2.2 dev br0 weight 1 nexthop via $TEST_STATIC_ROUTE_GATEWAY dev ztunit weight 1"
: > "$TEST_ACTION_LOG"
/bin/rm -rf "$TEST_CONFIG"
mkdir -p "$TEST_CONFIG"
echo identity > "$TEST_CONFIG/identity.secret"
echo running > "$TEST_STATE"
TEST_MODE=graceful
if stop_zero; then
	fail "stop accepted a multipath route containing a configured later nexthop"
fi
[ -f "$TEST_CONFIG/identity.secret" ] || fail "later-nexthop ambiguity removed the persistent identity"
if grep -Fqx -- "zero_route:del" "$TEST_ACTION_LOG"; then
	fail "cleanup issued a delete for an ambiguous multipath route"
fi
TEST_ROUTE_SHOW_OUTPUT=""


# Static-route cleanup must use the exact tuples installed earlier, not the
# user's subsequently edited NVRAM settings.
/bin/rm -f "$TEST_CONFIG/.static-routes-state"
mkdir -p "$TEST_CONFIG"
TEST_STATIC_ROUTE_COUNT=1
TEST_STATIC_ROUTE_ENABLE=1
TEST_STATIC_ROUTE_IP="192.0.2.0/24"
TEST_STATIC_ROUTE_GATEWAY="192.0.2.1"
TEST_ROUTE_PRESENT=0
TEST_ROUTE_SHOW_OUTPUT=""
TEST_ROUTE_ADD_STATUS=0
TEST_ROUTE_DEL_STATUS=0
TEST_ROUTE_DELETE_PERSISTS=0
zt0=ztunit
: > "$TEST_ACTION_LOG"
if ! zero_route add; then
	fail "static-route add did not persist its applied-route manifest"
fi
[ "$(cat "$TEST_CONFIG/.static-routes-state")" = "$(printf 'ztunit\nA 192.0.2.0/24 192.0.2.1')" ] ||
	fail "static-route manifest did not record the exact interface/destination/gateway"
TEST_ROUTE_SHOW_OUTPUT="192.0.2.0/24 via 192.0.2.1 dev ztunit"
TEST_STATIC_ROUTE_COUNT=0
TEST_STATIC_ROUTE_ENABLE=0
TEST_STATIC_ROUTE_IP="198.51.100.0/24"
TEST_STATIC_ROUTE_GATEWAY="198.51.100.1"
: > "$TEST_ROUTE_DEL_DETAILS_LOG"
if ! zero_route del; then
	fail "static-route cleanup failed after its NVRAM entry was removed"
fi
[ "$(cat "$TEST_ROUTE_DEL_DETAILS_LOG")" = "192.0.2.0/24 via 192.0.2.1 dev ztunit" ] ||
	fail "static-route cleanup used changed NVRAM instead of the applied tuple snapshot"
[ ! -e "$TEST_CONFIG/.static-routes-state" ] ||
	fail "successful static-route cleanup retained its snapshot"

# Exact routes which predate ZeroTier are not claimed or deleted on stop.
TEST_STATIC_ROUTE_COUNT=1
TEST_STATIC_ROUTE_ENABLE=1
TEST_STATIC_ROUTE_IP="192.0.2.0/24"
TEST_STATIC_ROUTE_GATEWAY="192.0.2.1"
TEST_ROUTE_PRESENT=1
TEST_ROUTE_SHOW_OUTPUT="192.0.2.0/24 via 192.0.2.1 dev ztunit"
: > "$TEST_ACTION_LOG"
if ! zero_route add; then
	fail "static-route add rejected an exact route which was already present"
fi
[ "$(cat "$TEST_CONFIG/.static-routes-state")" = "ztunit" ] ||
	fail "pre-existing route was incorrectly claimed in the ZeroTier manifest"
if grep -Fqx -- "zero_route:add" "$TEST_ACTION_LOG"; then
	fail "ZeroTier attempted to add a pre-existing exact route"
fi
if ! zero_route del; then
	fail "cleanup failed for a header-only static-route manifest"
fi
if grep -Fqx -- "zero_route:del" "$TEST_ACTION_LOG"; then
	fail "ZeroTier deleted a route which predated service startup"
fi
[ "$TEST_ROUTE_PRESENT" -eq 1 ] || fail "cleanup removed the pre-existing route"

# A conflicting route for the same destination is ambiguous and must fail
# before publishing a snapshot; rollback with no manifest is a safe no-op.
TEST_ROUTE_SHOW_OUTPUT="192.0.2.0/24 via 192.0.2.99 dev ztunit"
: > "$TEST_ACTION_LOG"
if zero_route add; then
	fail "static-route add accepted an existing conflicting nexthop"
fi
[ ! -e "$TEST_CONFIG/.static-routes-state" ] ||
	fail "conflicting route preflight published an ownership manifest"
[ ! -s "$TEST_ACTION_LOG" ] || fail "conflicting route preflight issued a route mutation"
if ! zero_route rollback; then
	fail "rollback without a published route manifest was not idempotent"
fi
[ "$TEST_ROUTE_PRESENT" -eq 1 ] || fail "failed route preflight altered the conflicting route"
TEST_ROUTE_PRESENT=0
TEST_ROUTE_SHOW_OUTPUT=""
TEST_STATIC_ROUTE_COUNT=0
TEST_STATIC_ROUTE_ENABLE=0

# A truncated route manifest must fail closed without issuing route commands.
printf 'ztunit\n192.0.2.0/24 192.0.2.1' > "$TEST_CONFIG/.static-routes-state"
: > "$TEST_ACTION_LOG"
if zero_route del; then
	fail "static-route cleanup accepted an unterminated manifest record"
fi
[ ! -s "$TEST_ACTION_LOG" ] || fail "corrupt static-route manifest issued route commands"
[ -f "$TEST_CONFIG/.static-routes-state" ] || fail "corrupt static-route manifest was discarded"
/bin/rm -f "$TEST_CONFIG/.static-routes-state"
TEST_STATIC_ROUTE_COUNT=0
TEST_STATIC_ROUTE_ENABLE=0
zt0=""

# iproute2 may display an explicitly configured /32 host route without /32.
# Cleanup must still identify and delete the exact destination/nexthop.
# Re-enable this route in the fixture: the preceding manifest-specific cases
# deliberately clear the NVRAM selectors before exercising legacy /32 parsing.
TEST_STATIC_ROUTE_COUNT=1
TEST_STATIC_ROUTE_ENABLE=1
TEST_STATIC_ROUTE_IP="192.0.2.7/32"
TEST_ROUTE_SHOW_OUTPUT="192.0.2.7 via $TEST_STATIC_ROUTE_GATEWAY dev ztunit"
TEST_ROUTE_PRESENT=1
TEST_ROUTE_SHOW_STATUS=0
TEST_ROUTE_DEL_STATUS=0
TEST_ROUTE_DELETE_PERSISTS=0
TEST_IFCONFIG_MODE=running-only
TEST_ROUTE_MODE=none
TEST_NAT_ENABLE=0
: > "$TEST_ACTION_LOG"
mkdir -p "$TEST_CONFIG"
echo identity > "$TEST_CONFIG/identity.secret"
echo running > "$TEST_STATE"
TEST_MODE=graceful
if ! stop_zero; then
	fail "stop failed to match a /32 route displayed without its prefix"
fi
grep -Fqx -- "zero_route:del" "$TEST_ACTION_LOG" || fail "host-route normalization skipped the /32 delete"
[ "$TEST_ROUTE_PRESENT" -eq 0 ] || fail "host-route normalization left the /32 route present"
[ ! -e "$TEST_CONFIG" ] || fail "successful /32 route cleanup retained persistent configuration"
TEST_STATIC_ROUTE_IP="192.0.2.0/24"
TEST_ROUTE_SHOW_OUTPUT=""

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
TEST_ROUTE_MODE=none
TEST_EXPECT_SAVED_SEGMENT=0
TEST_ROUTE_PRESENT=0
TEST_ROUTE_SHOW_STATUS=0
TEST_ROUTE_DEL_STATUS=0
TEST_ROUTE_DELETE_PERSISTS=0


# Direct rules() coverage models start_instance() after its config directory exists.
# Earlier stop-failure cases intentionally preserve recovery checkpoints; isolate
# this independent publication fixture from those expected leftovers.
/bin/rm -rf "$TEST_CONFIG"
mkdir -p "$TEST_CONFIG"

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
[ -f "$TEST_CONFIG/.firewall-state" ] || fail "successful rules publication did not save a recovery checkpoint"
[ "$(sed -n '1p' "$TEST_CONFIG/.firewall-state")" = "ztunit" ] || fail "rules checkpoint lost the published interface identity"

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
[ ! -s "$TEST_IPTABLES_STATE_FILE" ] || fail "filter append failure left published firewall rules behind"
[ ! -e "$TEST_CONFIG/.firewall-state" ] || fail "complete filter rollback retained a stale recovery checkpoint"
grep -Fqx -- "-D INPUT -i ztunit -j ACCEPT" "$TEST_IPTABLES_LOG" || fail "filter append failure did not remove the previously published input rule"

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
[ ! -s "$TEST_IPTABLES_STATE_FILE" ] || fail "NAT append failure left partially published firewall rules behind"
[ ! -e "$TEST_CONFIG/.firewall-state" ] || fail "complete NAT rollback retained a stale recovery checkpoint"
grep -Fqx -- "-t nat -D POSTROUTING -o ztunit -j MASQUERADE" "$TEST_IPTABLES_LOG" || fail "NAT append failure did not remove the previously published NAT rule"
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
# remove the firewall rules which were already installed. The preceding NAT-
# readiness case intentionally published an empty route manifest; clear that
# known-empty checkpoint before modelling a fresh configuration/start attempt.
: > "$TEST_SLEEP_LOG"
: > "$TEST_IPTABLES_LOG"
: > "$TEST_ACTION_LOG"
echo 0 > "$TEST_ROUTE_COUNT"
TEST_ROUTE_MODE=ready
TEST_IFCONFIG_MODE=ready
TEST_NAT_ENABLE=1
/bin/rm -f "$TEST_CONFIG/.static-routes-state"
TEST_STATIC_ROUTE_COUNT=1
TEST_STATIC_ROUTE_ENABLE=1
TEST_ROUTE_ADD_STATUS=1
TEST_ROUTE_ADD_PARTIAL=1
TEST_ROUTE_PRESENT=0
TEST_STATIC_ROUTE_IP="192.0.2.0/24"
TEST_STATIC_ROUTE_GATEWAY="192.0.2.1"
TEST_ROUTE_SHOW_OUTPUT=""
if rules; then
	fail "rules reported success after a configured static route add failed"
fi
grep -Fqx -- "zero_route:add" "$TEST_ACTION_LOG" || fail "static route failure fixture did not attempt route installation"
[ "$(cat "$TEST_ACTION_LOG")" = "zero_route:add" ] || fail "failed add attempted to delete an unconfirmed route"
[ "$TEST_ROUTE_PRESENT" -eq 1 ] || fail "race fixture did not leave the competing exact route present"
[ "$(cat "$TEST_CONFIG/.static-routes-state")" = "$(printf 'ztunit\nP 192.0.2.0/24 192.0.2.1')" ] || fail "unconfirmed route did not retain a pending ownership marker"
[ ! -s "$TEST_IPTABLES_STATE_FILE" ] || fail "static route failure left firewall rules behind"
[ -f "$TEST_CONFIG/.firewall-state" ] || fail "incomplete route rollback discarded its firewall cleanup checkpoint"
[ "$(grep -cE '(^| )-A ' "$TEST_IPTABLES_LOG" || true)" -eq 5 ] || fail "static route failure fixture did not publish all firewall rules before route failure"

# Exercise the real stop path: an ambiguous pending route must not stop
# or signal the already-running daemon whose network state must be preserved.
: > "$TEST_KILL_LOG"
test_proc_reset
test_proc_add 4321 "$PROG" "$PROG"
echo running > "$TEST_STATE"
TEST_MODE=graceful
TEST_IFCONFIG_MODE=ready
TEST_ROUTE_MODE=ready
zt0=""
ip_segment=""
if stop_zero; then
	fail "stop accepted an exact route whose add ownership was unconfirmed"
fi
[ "$(cat "$TEST_STATE")" = running ] || fail "pending route cleanup stopped the managed daemon"
[ -d "$ZT_PROC_ROOT/4321" ] || fail "pending route cleanup lost the managed process"
[ ! -s "$TEST_KILL_LOG" ] || fail "pending route cleanup signalled the managed daemon"

# Exercise the real start path against the same unconfirmed exact route. A
# replacement must not signal the healthy daemon or launch over ambiguous state.
TEST_NVRAM_READ_FAIL_KEY=""
TEST_NVRAM_MOONID_STATE=""
TEST_NVRAM_MOON_ENABLE_STATE=1
TEST_NVRAM_ID_STATE=""
if start_zero; then
	fail "start accepted an exact route whose add ownership was unconfirmed"
fi
[ "$(cat "$TEST_STATE")" = running ] || fail "pending route start restarted the daemon"
[ -d "$ZT_PROC_ROOT/4321" ] || fail "pending route start lost the managed process"
[ ! -s "$TEST_KILL_LOG" ] || fail "pending route start signalled the managed daemon"

# A pending exact match is unowned: replay must preserve it and retain recovery state.
: > "$TEST_ACTION_LOG"
if zerotier_replay_firewall_state; then
	fail "replay deleted or accepted a pending route still present in the kernel"
fi
[ "$TEST_ROUTE_PRESENT" -eq 1 ] || fail "replay removed a route whose add ownership was unconfirmed"
[ -f "$TEST_CONFIG/.static-routes-state" ] || fail "pending route replay discarded its ownership marker"
[ -f "$TEST_CONFIG/.firewall-state" ] || fail "pending route replay discarded the firewall checkpoint"
if grep -Fqx -- "zero_route:del" "$TEST_ACTION_LOG"; then
	fail "replay issued a delete for an unconfirmed route"
fi

# Once an administrator removes the ambiguous route, a later replay safely
# removes the pending record and retires the already-clean firewall checkpoint.
TEST_ROUTE_PRESENT=0
if ! zerotier_replay_firewall_state; then
	fail "replay could not finish cleanup after the pending route became absent"
fi
[ ! -e "$TEST_CONFIG/.static-routes-state" ] || fail "confirmed-absent pending route kept its manifest"
[ ! -e "$TEST_CONFIG/.firewall-state" ] || fail "confirmed-absent pending route kept its firewall checkpoint"
[ "$TEST_ROUTE_PRESENT" -eq 0 ] || fail "pending route retry changed the absent route fixture"
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
TEST_ASSERT_CLEAN_BEFORE_START=0
TEST_EXPECT_SAVED_SEGMENT=0
TEST_NAT_ENABLE=0
TEST_STATIC_ROUTE_COUNT=0
TEST_STATIC_ROUTE_ENABLE=0
TEST_ROUTE_PRESENT=0
TEST_ROUTE_SHOW_STATUS=0
TEST_ROUTE_DEL_STATUS=0
TEST_ROUTE_DELETE_PERSISTS=0
TEST_IFCONFIG_MODE=none
TEST_ROUTE_MODE=none

# NAT-enabled shutdown must fail before signalling the daemon if its connected
# route cannot be captured for source-NAT firewall cleanup.
TEST_STATIC_ROUTE_COUNT=0
TEST_STATIC_ROUTE_ENABLE=0
TEST_NAT_ENABLE=1
TEST_IFCONFIG_MODE=running-only
TEST_ROUTE_MODE=none
: > "$TEST_KILL_LOG"
: > "$TEST_ACTION_LOG"
mkdir -p "$TEST_CONFIG"
echo identity > "$TEST_CONFIG/identity.secret"
echo running > "$TEST_STATE"
TEST_MODE=graceful
if stop_zero; then
	fail "stop accepted NAT-enabled shutdown without a connected route segment"
fi
[ "$(cat "$TEST_STATE")" = running ] || fail "missing NAT route stopped the daemon before cleanup could be guaranteed"
[ ! -s "$TEST_KILL_LOG" ] || fail "missing NAT route still signalled the daemon"
[ ! -s "$TEST_ACTION_LOG" ] || fail "missing NAT route changed firewall or persistent configuration"
[ -f "$TEST_CONFIG/identity.secret" ] || fail "missing NAT route removed the persistent identity"
TEST_NAT_ENABLE=0

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
TEST_ROUTE_MODE=none
TEST_EXPECT_SAVED_SEGMENT=0
TEST_ROUTE_PRESENT=0
TEST_ROUTE_SHOW_STATUS=0
TEST_ROUTE_DEL_STATUS=0
TEST_ROUTE_DELETE_PERSISTS=0


# Direct rules() coverage models start_instance() after its config directory exists.
# Earlier stop-failure cases intentionally preserve recovery checkpoints; isolate
# this independent publication fixture from those expected leftovers.
/bin/rm -rf "$TEST_CONFIG"
mkdir -p "$TEST_CONFIG"

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
[ -f "$TEST_CONFIG/.firewall-state" ] || fail "successful rules publication did not save a recovery checkpoint"
[ "$(sed -n '1p' "$TEST_CONFIG/.firewall-state")" = "ztunit" ] || fail "rules checkpoint lost the published interface identity"

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
[ ! -s "$TEST_IPTABLES_STATE_FILE" ] || fail "filter append failure left published firewall rules behind"
[ ! -e "$TEST_CONFIG/.firewall-state" ] || fail "complete filter rollback retained a stale recovery checkpoint"
grep -Fqx -- "-D INPUT -i ztunit -j ACCEPT" "$TEST_IPTABLES_LOG" || fail "filter append failure did not remove the previously published input rule"

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
[ ! -s "$TEST_IPTABLES_STATE_FILE" ] || fail "NAT append failure left partially published firewall rules behind"
[ ! -e "$TEST_CONFIG/.firewall-state" ] || fail "complete NAT rollback retained a stale recovery checkpoint"
grep -Fqx -- "-t nat -D POSTROUTING -o ztunit -j MASQUERADE" "$TEST_IPTABLES_LOG" || fail "NAT append failure did not remove the previously published NAT rule"
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
# remove the firewall rules which were already installed. The preceding NAT-
# readiness case intentionally published an empty route manifest; clear that
# known-empty checkpoint before modelling a fresh configuration/start attempt.
: > "$TEST_SLEEP_LOG"
: > "$TEST_IPTABLES_LOG"
: > "$TEST_ACTION_LOG"
echo 0 > "$TEST_ROUTE_COUNT"
TEST_ROUTE_MODE=ready
TEST_IFCONFIG_MODE=ready
TEST_NAT_ENABLE=1
/bin/rm -f "$TEST_CONFIG/.static-routes-state"
TEST_STATIC_ROUTE_COUNT=1
TEST_STATIC_ROUTE_ENABLE=1
TEST_ROUTE_ADD_STATUS=1
TEST_ROUTE_ADD_PARTIAL=1
TEST_ROUTE_PRESENT=0
TEST_STATIC_ROUTE_IP="192.0.2.0/24"
TEST_STATIC_ROUTE_GATEWAY="192.0.2.1"
TEST_ROUTE_SHOW_OUTPUT=""
if rules; then
	fail "rules reported success after a configured static route add failed"
fi
grep -Fqx -- "zero_route:add" "$TEST_ACTION_LOG" || fail "static route failure fixture did not attempt route installation"
[ "$(cat "$TEST_ACTION_LOG")" = "zero_route:add" ] || fail "failed add attempted to delete an unconfirmed route"
[ "$TEST_ROUTE_PRESENT" -eq 1 ] || fail "race fixture did not leave the competing exact route present"
[ "$(cat "$TEST_CONFIG/.static-routes-state")" = "$(printf 'ztunit\nP 192.0.2.0/24 192.0.2.1')" ] || fail "unconfirmed route did not retain a pending ownership marker"
[ ! -s "$TEST_IPTABLES_STATE_FILE" ] || fail "static route failure left firewall rules behind"
[ -f "$TEST_CONFIG/.firewall-state" ] || fail "incomplete route rollback discarded its firewall cleanup checkpoint"
[ "$(grep -cE '(^| )-A ' "$TEST_IPTABLES_LOG" || true)" -eq 5 ] || fail "static route failure fixture did not publish all firewall rules before route failure"

# Exercise the real stop path: an ambiguous pending route must not stop
# or signal the already-running daemon whose network state must be preserved.
: > "$TEST_KILL_LOG"
test_proc_reset
test_proc_add 4321 "$PROG" "$PROG"
echo running > "$TEST_STATE"
TEST_MODE=graceful
TEST_IFCONFIG_MODE=ready
TEST_ROUTE_MODE=ready
zt0=""
ip_segment=""
if stop_zero; then
	fail "stop accepted an exact route whose add ownership was unconfirmed"
fi
[ "$(cat "$TEST_STATE")" = running ] || fail "pending route cleanup stopped the managed daemon"
[ -d "$ZT_PROC_ROOT/4321" ] || fail "pending route cleanup lost the managed process"
[ ! -s "$TEST_KILL_LOG" ] || fail "pending route cleanup signalled the managed daemon"

# Exercise the real start path against the same unconfirmed exact route. A
# replacement must not signal the healthy daemon or launch over ambiguous state.
TEST_NVRAM_READ_FAIL_KEY=""
TEST_NVRAM_MOONID_STATE=""
TEST_NVRAM_MOON_ENABLE_STATE=1
TEST_NVRAM_ID_STATE=""
if start_zero; then
	fail "start accepted an exact route whose add ownership was unconfirmed"
fi
[ "$(cat "$TEST_STATE")" = running ] || fail "pending route start restarted the daemon"
[ -d "$ZT_PROC_ROOT/4321" ] || fail "pending route start lost the managed process"
[ ! -s "$TEST_KILL_LOG" ] || fail "pending route start signalled the managed daemon"

# A pending exact match is unowned: replay must preserve it and retain recovery state.
: > "$TEST_ACTION_LOG"
if zerotier_replay_firewall_state; then
	fail "replay deleted or accepted a pending route still present in the kernel"
fi
[ "$TEST_ROUTE_PRESENT" -eq 1 ] || fail "replay removed a route whose add ownership was unconfirmed"
[ -f "$TEST_CONFIG/.static-routes-state" ] || fail "pending route replay discarded its ownership marker"
[ -f "$TEST_CONFIG/.firewall-state" ] || fail "pending route replay discarded the firewall checkpoint"
if grep -Fqx -- "zero_route:del" "$TEST_ACTION_LOG"; then
	fail "replay issued a delete for an unconfirmed route"
fi

# Once an administrator removes the ambiguous route, a later replay safely
# removes the pending record and retires the already-clean firewall checkpoint.
TEST_ROUTE_PRESENT=0
if ! zerotier_replay_firewall_state; then
	fail "replay could not finish cleanup after the pending route became absent"
fi
[ ! -e "$TEST_CONFIG/.static-routes-state" ] || fail "confirmed-absent pending route kept its manifest"
[ ! -e "$TEST_CONFIG/.firewall-state" ] || fail "confirmed-absent pending route kept its firewall checkpoint"
[ "$TEST_ROUTE_PRESENT" -eq 0 ] || fail "pending route retry changed the absent route fixture"
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

# Runtime NAT policy read errors must not be interpreted as "NAT disabled".
: > "$TEST_IPTABLES_LOG"
TEST_IFCONFIG_MODE=ready
TEST_ROUTE_MODE=ready
TEST_NAT_ENABLE=1
TEST_NVRAM_READ_FAIL_KEY=zerotier_nat
if rules; then
	fail "rules accepted an unreadable NAT selector as disabled"
fi
[ ! -s "$TEST_IPTABLES_LOG" ] || fail "NAT selector read failure published partial firewall rules"
TEST_NVRAM_READ_FAIL_KEY=""

# A failed NAT read during stop must leave the live daemon and rules untouched.
: > "$TEST_KILL_LOG"
: > "$TEST_ACTION_LOG"
TEST_NVRAM_READ_FAIL_KEY=zerotier_nat
TEST_NAT_ENABLE=1
zt0=ztunit
ip_segment=10.147.17.0/24
echo running > "$TEST_STATE"
if stop_zero; then
	fail "stop accepted an unreadable NAT selector"
fi
[ "$(cat "$TEST_STATE")" = "running" ] || fail "NAT selector read failure stopped the daemon"
[ ! -s "$TEST_KILL_LOG" ] || fail "NAT selector read failure still signalled the daemon"
[ ! -s "$TEST_ACTION_LOG" ] || fail "NAT selector read failure changed firewall or route state"
TEST_NVRAM_READ_FAIL_KEY=""

# Every enabled static-route selector is checked; a fault must prevent
# route commands and must not stop a currently working daemon.
TEST_STATIC_ROUTE_COUNT=1
TEST_STATIC_ROUTE_ENABLE=1
TEST_NAT_ENABLE=0
TEST_IFCONFIG_MODE=running-only
TEST_ROUTE_MODE=none
zt0=ztunit
ip_segment=10.147.17.0/24
for setting in zero_staticnum_x zero_enable_x0 zero_ip_x0 zero_route_x0; do
	TEST_NVRAM_READ_FAIL_KEY="$setting"
	: > "$TEST_ACTION_LOG"
	if zero_route add; then
		fail "static-route add accepted a checked-read error for $setting"
	fi
	[ ! -s "$TEST_ACTION_LOG" ] || fail "static-route add changed routes after $setting read failure"
	: > "$TEST_KILL_LOG"
	echo running > "$TEST_STATE"
	if start_zero; then
		fail "start accepted a static-route preflight error for $setting"
	fi
	[ "$(cat "$TEST_STATE")" = "running" ] || fail "start stopped the daemon after $setting read failure"
	[ ! -s "$TEST_KILL_LOG" ] || fail "start signalled the daemon after $setting read failure"
	: > "$TEST_KILL_LOG"
	echo running > "$TEST_STATE"
	if stop_zero; then
		fail "stop accepted a static-route preflight error for $setting"
	fi
	[ "$(cat "$TEST_STATE")" = "running" ] || fail "stop stopped the daemon after $setting read failure"
	[ ! -s "$TEST_KILL_LOG" ] || fail "stop signalled the daemon after $setting read failure"
done
TEST_NVRAM_READ_FAIL_KEY=""
TEST_STATIC_ROUTE_ENABLE=0
TEST_NAT_ENABLE=0
zt0=ztunit
ip_segment=10.147.17.0/24

# The UI allows at most 64 static routes. Reject corrupted or oversized
# NVRAM counts before either preflight or route execution can start a seq loop.
for invalid_count in 65 00065 999999999999999999999999; do
	TEST_STATIC_ROUTE_COUNT="$invalid_count"
	: > "$TEST_ACTION_LOG"
	if zerotier_preflight_static_routes; then
		fail "static-route preflight accepted unsupported count $invalid_count"
	fi
	if zero_route add; then
		fail "static-route executor accepted unsupported count $invalid_count"
	fi
	[ ! -s "$TEST_ACTION_LOG" ] || fail "unsupported route count $invalid_count issued network commands"
done

# Preserve empty/zero compatibility and allow the full UI-supported count.
for valid_count in "" 0 00064; do
	TEST_STATIC_ROUTE_COUNT="$valid_count"
	if ! zerotier_preflight_static_routes; then
		fail "static-route preflight rejected compatible count '$valid_count'"
	fi
	if ! zero_route add; then
		fail "static-route executor rejected compatible count '$valid_count'"
	fi
	# Each iteration models a fresh add/stop cycle. The header-only manifest
	# written for a zero enabled-route count must not poison the next count.
	/bin/rm -f "$TEST_CONFIG/.static-routes-state"
done

# The route UI exposes exactly 0/1. Reject any other readable value in
# both validation paths and preserve the running service on start/stop.
TEST_NVRAM_READ_FAIL_KEY=""
TEST_NVRAM_MOON_ENABLE_STATE=1
TEST_NVRAM_ID_STATE=fedcba9876543210
TEST_STATIC_ROUTE_COUNT=1
TEST_STATIC_ROUTE_ENABLE=2
TEST_NAT_ENABLE=0
TEST_IFCONFIG_MODE=none
TEST_ROUTE_MODE=none
zt0=""
ip_segment=""
: > "$TEST_ACTION_LOG"
if zerotier_preflight_static_routes; then
	fail "static route preflight accepted invalid enable selector 2"
fi
if zero_route add; then
	fail "static route executor accepted invalid enable selector 2"
fi
[ ! -s "$TEST_ACTION_LOG" ] || fail "invalid route enable selector issued network commands"

: > "$TEST_KILL_LOG"
: > "$TEST_ACTION_LOG"
echo running > "$TEST_STATE"
if start_zero; then
	fail "start accepted invalid route enable selector"
fi
[ "$(cat "$TEST_STATE")" = "running" ] || fail "invalid route enable selector stopped the daemon during start"
[ ! -s "$TEST_KILL_LOG" ] || fail "invalid route enable selector signalled the daemon during start"
[ ! -s "$TEST_ACTION_LOG" ] || fail "invalid route enable selector changed network state during start"

: > "$TEST_KILL_LOG"
: > "$TEST_ACTION_LOG"
echo running > "$TEST_STATE"
if stop_zero; then
	fail "stop accepted invalid route enable selector"
fi
[ "$(cat "$TEST_STATE")" = "running" ] || fail "invalid route enable selector stopped the daemon during stop"
[ ! -s "$TEST_KILL_LOG" ] || fail "invalid route enable selector signalled the daemon during stop"
[ ! -s "$TEST_ACTION_LOG" ] || fail "invalid route enable selector changed network state during stop"

# Reject iproute2 keywords, malformed CIDRs and non-literal gateways before
# either the daemon lifecycle or any route command can be changed.
TEST_STATIC_ROUTE_COUNT=1
TEST_STATIC_ROUTE_ENABLE=1
TEST_NAT_ENABLE=0
TEST_NVRAM_READ_FAIL_KEY=""
TEST_NVRAM_MOON_ENABLE_STATE=0
TEST_IFCONFIG_MODE=running-only
TEST_ROUTE_MODE=none
zt0=ztunit
ip_segment=10.147.17.0/24
for invalid_ip in default blackhole 192.0.2.256/24 192.0.2.0/33 192.0.2.1/24 010.0.0.0/24 "192.0.2.0/24 dev br0"; do
	TEST_STATIC_ROUTE_IP="$invalid_ip"
	: > "$TEST_ACTION_LOG"
	if zerotier_preflight_static_routes; then
		fail "static-route preflight accepted invalid destination $invalid_ip"
	fi
	if zero_route add; then
		fail "static-route executor accepted invalid destination $invalid_ip"
	fi
	[ ! -s "$TEST_ACTION_LOG" ] || fail "invalid destination $invalid_ip reached iproute2"
	: > "$TEST_KILL_LOG"
	echo running > "$TEST_STATE"
	if start_zero; then
		fail "start accepted invalid destination $invalid_ip"
	fi
	[ "$(cat "$TEST_STATE")" = "running" ] || fail "invalid destination $invalid_ip stopped the running daemon"
	[ ! -s "$TEST_KILL_LOG" ] || fail "invalid destination $invalid_ip signalled the running daemon"
	[ ! -s "$TEST_ACTION_LOG" ] || fail "invalid destination $invalid_ip changed network state"
	: > "$TEST_KILL_LOG"
	echo running > "$TEST_STATE"
	if stop_zero; then
		fail "stop accepted invalid destination $invalid_ip"
	fi
	[ "$(cat "$TEST_STATE")" = "running" ] || fail "invalid destination $invalid_ip stopped the running daemon"
	[ ! -s "$TEST_KILL_LOG" ] || fail "invalid destination $invalid_ip signalled the running daemon"
	[ ! -s "$TEST_ACTION_LOG" ] || fail "invalid destination $invalid_ip changed network state during stop"
done
TEST_STATIC_ROUTE_IP=192.0.2.0/24
for invalid_gateway in default 192.0.2.256 192.0.2.1/24 010.0.0.1 "192.0.2.1 dev br0"; do
	TEST_STATIC_ROUTE_GATEWAY="$invalid_gateway"
	: > "$TEST_ACTION_LOG"
	if zerotier_preflight_static_routes; then
		fail "static-route preflight accepted invalid gateway $invalid_gateway"
	fi
	if zero_route add; then
		fail "static-route executor accepted invalid gateway $invalid_gateway"
	fi
	[ ! -s "$TEST_ACTION_LOG" ] || fail "invalid gateway $invalid_gateway reached iproute2"
	: > "$TEST_KILL_LOG"
	echo running > "$TEST_STATE"
	if start_zero; then
		fail "start accepted invalid gateway $invalid_gateway"
	fi
	[ "$(cat "$TEST_STATE")" = "running" ] || fail "invalid gateway $invalid_gateway stopped the running daemon"
	[ ! -s "$TEST_KILL_LOG" ] || fail "invalid gateway $invalid_gateway signalled the running daemon"
	[ ! -s "$TEST_ACTION_LOG" ] || fail "invalid gateway $invalid_gateway changed network state"
	: > "$TEST_KILL_LOG"
	echo running > "$TEST_STATE"
	if stop_zero; then
		fail "stop accepted invalid gateway $invalid_gateway"
	fi
	[ "$(cat "$TEST_STATE")" = "running" ] || fail "invalid gateway $invalid_gateway stopped the running daemon"
	[ ! -s "$TEST_KILL_LOG" ] || fail "invalid gateway $invalid_gateway signalled the running daemon during stop"
	[ ! -s "$TEST_ACTION_LOG" ] || fail "invalid gateway $invalid_gateway changed network state during stop"
done
TEST_STATIC_ROUTE_GATEWAY=192.0.2.1
# The explicit all-addresses default route remains syntactically valid.
TEST_STATIC_ROUTE_IP=0.0.0.0/0
if ! zerotier_preflight_static_routes || ! zero_route add; then
	fail "valid 0.0.0.0/0 destination with a literal IPv4 gateway was rejected"
fi
TEST_STATIC_ROUTE_IP=192.0.2.0/24
TEST_STATIC_ROUTE_COUNT=0
TEST_STATIC_ROUTE_ENABLE=0
TEST_NAT_ENABLE=0
zt0=""
ip_segment=""

# Exercise the production launcher after readiness: an orbit failure must not
# be reported as success or leave the daemon/network state half-published.
TEST_ORBIT_STATUS=23
TEST_KILL_STATUS=0
TEST_DEL_RULES_STATUS=0
TEST_ZERO_ROUTE_STATUS=0
TEST_ORBIT_LOG="$TEST_TMP/orbit.log"
export TEST_ORBIT_STATUS TEST_ORBIT_LOG
TEST_FAKE_ZEROTIER_CLI="$TEST_TMP/zerotier-cli"
cat > "$TEST_FAKE_ZEROTIER_CLI" <<'ORBIT_CLI'
#!/bin/sh
printf '%s\n' "$*" >> "$TEST_ORBIT_LOG"
exit "$TEST_ORBIT_STATUS"
ORBIT_CLI
chmod 700 "$TEST_FAKE_ZEROTIER_CLI"
PROG=/bin/true
PROGCLI="$TEST_FAKE_ZEROTIER_CLI"
PLANET="$TEST_TMP/no-uploaded-planet"
config_path="$TEST_TMP/orbit-config"
mkdir -p "$config_path"
moonid=0123456789abcdef
secret=
planet=
enablemoonserv=
zt_network_id=

zerotier_prepare_config_directory() { return 0; }
zerotier_prepare_identity() { return 0; }
add_join() { return 0; }
rules() {
	zt0=ztunit
	ip_segment=10.147.17.0/24
	: > "$config_path/.firewall-state"
	return 0
}
kill_z() {
	printf '%s\n' kill_z >> "$TEST_ACTION_LOG"
	return "$TEST_KILL_STATUS"
}
del_rules() {
	printf '%s\n' del_rules >> "$TEST_ACTION_LOG"
	return "$TEST_DEL_RULES_STATUS"
}
zero_route() {
	printf 'zero_route:%s\n' "$1" >> "$TEST_ACTION_LOG"
	return "$TEST_ZERO_ROUTE_STATUS"
}
zerotier_firewall_state_clear() {
	printf '%s\n' clear_firewall_state >> "$TEST_ACTION_LOG"
	/bin/rm -f "$config_path/.firewall-state"
}

: > "$TEST_ACTION_LOG"
: > "$TEST_ORBIT_LOG"
if start_instance_production orbit-failure; then
	fail "production launcher reported success after the Moon orbit command failed"
fi
grep -Fq -- '-D' "$TEST_ORBIT_LOG" || fail "production launcher did not invoke zerotier-cli orbit"
[ "$(cat "$TEST_ACTION_LOG")" = "$(printf 'kill_z\ndel_rules\nzero_route:del\nclear_firewall_state')" ] ||
	fail "successful orbit-failure rollback did not stop then clean network state in order"
[ ! -e "$config_path/.firewall-state" ] || fail "successful orbit rollback retained its recovery checkpoint"

# If the daemon cannot be confirmed stopped, do not touch firewall/routes or
# clear the checkpoint that is needed for a later cleanup retry.
TEST_KILL_STATUS=1
: > "$TEST_ACTION_LOG"
if start_instance_production orbit-stop-failure; then
	fail "production launcher accepted an orbit failure when daemon stop failed"
fi
[ "$(cat "$TEST_ACTION_LOG")" = "kill_z" ] || fail "network cleanup ran before daemon absence was confirmed"
[ -f "$config_path/.firewall-state" ] || fail "stop failure discarded the network recovery checkpoint"

# Independent cleanup legs are both attempted, but a failed leg preserves
# the checkpoint instead of pretending the rollback was complete.
TEST_KILL_STATUS=0
TEST_DEL_RULES_STATUS=1
TEST_ZERO_ROUTE_STATUS=0
: > "$TEST_ACTION_LOG"
if start_instance_production orbit-firewall-cleanup-failure; then
	fail "production launcher accepted an incomplete firewall rollback"
fi
[ "$(cat "$TEST_ACTION_LOG")" = "$(printf 'kill_z\ndel_rules\nzero_route:del')" ] ||
	fail "route cleanup was skipped after an independent firewall cleanup failure"
[ -f "$config_path/.firewall-state" ] || fail "incomplete firewall rollback discarded its checkpoint"

# A successful orbit still leaves the daemon and its network checkpoint live.
TEST_ORBIT_STATUS=0
TEST_DEL_RULES_STATUS=0
: > "$TEST_ACTION_LOG"
: > "$TEST_ORBIT_LOG"
start_instance_production orbit-success || fail "production launcher rejected a successful Moon orbit"
[ ! -s "$TEST_ACTION_LOG" ] || fail "successful Moon orbit unexpectedly ran rollback"
grep -Fq -- 'orbit 0123456789abcdef 0123456789abcdef' "$TEST_ORBIT_LOG" ||
	fail "Moon orbit did not receive the configured ID as two distinct argv values"
[ -f "$config_path/.firewall-state" ] || fail "successful Moon orbit lost its live network checkpoint"

echo "ZeroTier bounded stop/start, readiness, firewall/static-route rollback and persistent-config preservation: PASS"
echo "ZeroTier Moon orbit propagates CLI failure and rolls back readiness state: PASS"
