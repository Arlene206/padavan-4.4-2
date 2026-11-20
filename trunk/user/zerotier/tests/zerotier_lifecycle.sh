#!/bin/sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname "$0")/../../../.." && pwd)
SCRIPT="$ROOT/trunk/user/zerotier/zerotier.sh"
TEST_TMP=$(mktemp -d "${TMPDIR:-/tmp}/zerotier-lifecycle.XXXXXX")
TEST_STATE="$TEST_TMP/process.state"
TEST_NVRAM_SECRET_STATE="$TEST_TMP/nvram-secret.state"
: > "$TEST_NVRAM_SECRET_STATE"
TEST_NVRAM_SET_FAIL=0
TEST_NVRAM_COMMIT_FAIL=0
export TEST_NVRAM_SECRET_STATE TEST_NVRAM_SET_FAIL TEST_NVRAM_COMMIT_FAIL
TEST_KILL_LOG="$TEST_TMP/killall.log"
TEST_SLEEP_LOG="$TEST_TMP/sleep.log"
TEST_ACTION_LOG="$TEST_TMP/actions.log"
TEST_IPTABLES_LOG="$TEST_TMP/iptables.log"
TEST_IPTABLES_STATE_FILE="$TEST_TMP/iptables.state"
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
TEST_EXPECT_SAVED_SEGMENT=0
TEST_IPTABLES_FAIL_ON=""
TEST_IPTABLES_CHECK_FAIL_ON=""
TEST_IPTABLES_DELETE_FAIL_ON=""
: > "$TEST_IFCONFIG_COUNT"
: > "$TEST_ROUTE_COUNT"
: > "$TEST_IPTABLES_STATE_FILE"
export TEST_STATE TEST_KILL_LOG TEST_SLEEP_LOG TEST_ACTION_LOG TEST_TMP
export TEST_IPTABLES_LOG TEST_IPTABLES_STATE_FILE TEST_IFCONFIG_COUNT TEST_ROUTE_COUNT
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
	if [ "${TEST_ASSERT_CLEAN_BEFORE_START:-0}" = 1 ]; then
		[ ! -e "$config_path/.firewall-state" ] || fail "replacement daemon started before stale firewall checkpoint was cleared"
		[ ! -s "$TEST_IPTABLES_STATE_FILE" ] || fail "replacement daemon started with stale firewall rules"
		[ "$TEST_ROUTE_PRESENT" -eq 0 ] || fail "replacement daemon started with a stale configured route"
	fi
	echo start >> "$TEST_ACTION_LOG"
	return "$TEST_START_INSTANCE_STATUS"
}

rm() {
	if [ "$1" = "-rf" ]; then
		printf 'rm:%s\n' "$*" >> "$TEST_ACTION_LOG"
	fi
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
		daemon-only)
			if [ "$(cat "$TEST_STATE")" = running ]; then
				printf '%s\n' "10.147.17.0/24 dev ztunit proto kernel scope link"
			fi
			;;
		*)
			;;
	esac
	return 0
}

nvram() {
	case "$1" in
		get)
			case "$2" in
				zerotier_secret) cat "$TEST_NVRAM_SECRET_STATE" ;;
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
			esac
			;;
		commit)
			[ "${TEST_NVRAM_COMMIT_FAIL:-0}" != 1 ] || return 1
			;;
	esac
	return 0
}

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

[ ! -s "$TEST_TMP/numeric-kill.log" ] || fail "a stale numeric PID was signalled"

# Direct rules() coverage models start_instance() after its config directory exists.
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
[ "$(cat "$TEST_ACTION_LOG")" = "$(printf 'zero_route:add\nzero_route:del')" ] || fail "static route failure did not roll back firewall and partial route state"
[ ! -s "$TEST_IPTABLES_STATE_FILE" ] || fail "static route failure left firewall rules behind"
[ ! -e "$TEST_CONFIG/.firewall-state" ] || fail "complete static-route rollback retained a stale recovery checkpoint"
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

[ ! -s "$TEST_TMP/numeric-kill.log" ] || fail "a stale numeric PID was signalled"

# Direct rules() coverage models start_instance() after its config directory exists.
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
[ "$(cat "$TEST_ACTION_LOG")" = "$(printf 'zero_route:add\nzero_route:del')" ] || fail "static route failure did not roll back firewall and partial route state"
[ ! -s "$TEST_IPTABLES_STATE_FILE" ] || fail "static route failure left firewall rules behind"
[ ! -e "$TEST_CONFIG/.firewall-state" ] || fail "complete static-route rollback retained a stale recovery checkpoint"
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
