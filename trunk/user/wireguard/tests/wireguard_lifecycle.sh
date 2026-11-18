#!/bin/sh
set -eu

repo_root=$(CDPATH= cd -- "$(dirname "$0")/../../../.." && pwd)
script="$repo_root/trunk/user/wireguard/wireguard.sh"
tmp=$(mktemp -d "${TMPDIR:-/tmp}/wireguard-lifecycle.XXXXXX")
trap 'rm -rf "$tmp"' EXIT HUP INT TERM
bin="$tmp/bin"
mkdir -p "$bin"

fail()
{
	echo "WireGuard lifecycle regression: FAIL: $*" >&2
	exit 1
}

assert_status()
{
	expected=$1
	actual=$2
	[ "$actual" -eq "$expected" ] || fail "expected status $expected, got $actual"
}

assert_key_removed()
{
	[ -f "$WG_TEST_KEY_PATH" ] || fail "private-key staging path was not recorded"
	key_file=$(cat "$WG_TEST_KEY_PATH")
	[ -n "$key_file" ] || fail "private-key staging path is empty"
	[ ! -e "$key_file" ] || fail "private-key staging file was not removed: $key_file"
}

cat > "$bin/nvram" <<'STUB'
#!/bin/sh
[ "$1" = get ] || exit 2
case "$2" in
	wireguard_localip) printf '%s\n' "${WG_TEST_LOCALIP-10.7.0.2/24}" ;;
	wireguard_localkey) printf '%s\n' "${WG_TEST_LOCALKEY-test-private-key}" ;;
	wireguard_peerkey) printf '%s\n' "${WG_TEST_PEERKEY-test-peer-key}" ;;
	wireguard_peerip) printf '%s\n' "${WG_TEST_PEERIP-203.0.113.10:51820}" ;;
	*) exit 1 ;;
esac
STUB

cat > "$bin/ip" <<'STUB'
#!/bin/sh
printf 'ip %s\n' "$*" >> "$WG_TEST_EVENTS"
case "$*" in
	"link show dev wg0")
		[ -f "$WG_TEST_INTERFACE" ] && exit 0
		exit 1
		;;
	"link add dev wg0 type wireguard")
		status=${WG_TEST_IP_ADD_STATUS:-0}
		[ "$status" -eq 0 ] || exit "$status"
		: > "$WG_TEST_INTERFACE"
		exit 0
		;;
	"link set dev wg0 mtu 1420")
		exit "${WG_TEST_IP_MTU_STATUS:-0}"
		;;
	"address replace "*" dev wg0")
		exit "${WG_TEST_IP_ADDRESS_STATUS:-0}"
		;;
	"link set dev wg0 up")
		exit "${WG_TEST_IP_UP_STATUS:-0}"
		;;
	"link del dev wg0")
		status=${WG_TEST_IP_DEL_STATUS:-0}
		[ "$status" -eq 0 ] || exit "$status"
		rm -f "$WG_TEST_INTERFACE"
		exit 0
		;;
	*)
		echo "unexpected ip invocation: $*" >&2
		exit 90
		;;
esac
STUB

cat > "$bin/wg" <<'STUB'
#!/bin/sh
printf 'wg %s\n' "$*" >> "$WG_TEST_EVENTS"
[ "$1" = set ] && [ "$2" = wg0 ] || exit 2
if [ "$3" = private-key ]; then
	key_file=$4
	[ -f "$key_file" ] || exit 91
	[ "$(stat -c %a "$key_file")" = 600 ] || exit 92
	[ "$(cat "$key_file")" = "$WG_TEST_LOCALKEY" ] || exit 93
	printf '%s\n' "$key_file" > "$WG_TEST_KEY_PATH"
	exit "${WG_TEST_WG_KEY_STATUS:-0}"
fi
[ "$3" = peer ] || exit 2
exit "${WG_TEST_WG_PEER_STATUS:-0}"
STUB

cat > "$bin/iptables" <<'STUB'
#!/bin/sh
printf 'iptables %s\n' "$*" >> "$WG_TEST_EVENTS"
if [ -f "$WG_TEST_FIREWALL" ]; then
	count=$(cat "$WG_TEST_FIREWALL")
else
	count=0
fi
case " $* " in
	*" -C "*) [ "$count" -gt 0 ] && exit 0; exit 1 ;;
	*" -A "*)
		status=${WG_TEST_IPTABLES_ADD_STATUS:-0}
		[ "$status" -eq 0 ] || exit "$status"
		count=$((count + 1))
		printf '%s\n' "$count" > "$WG_TEST_FIREWALL"
		exit 0
		;;
	*" -D "*)
		status=${WG_TEST_IPTABLES_DEL_STATUS:-0}
		[ "$status" -eq 0 ] || exit "$status"
		[ "$count" -gt 0 ] || exit 1
		count=$((count - 1))
		if [ "$count" -eq 0 ]; then
			rm -f "$WG_TEST_FIREWALL"
		else
			printf '%s\n' "$count" > "$WG_TEST_FIREWALL"
		fi
		exit 0
		;;
esac
exit 2
STUB

cat > "$bin/logger" <<'STUB'
#!/bin/sh
printf 'logger %s\n' "$*" >> "$WG_TEST_EVENTS"
exit 0
STUB

cat > "$bin/mktemp" <<'STUB'
#!/bin/sh
path=$(/usr/bin/mktemp "$@") || exit $?
printf '%s\n' "$path" > "$WG_TEST_KEY_PATH"
printf '%s\n' "$path"
STUB

chmod 755 "$bin/"*
export PATH="$bin:$PATH"
export WG_TEST_INTERFACE="$tmp/interface"
export WG_TEST_FIREWALL="$tmp/firewall"
export WG_TEST_EVENTS="$tmp/events"
export WG_TEST_KEY_PATH="$tmp/key-path"
export WG_TEST_LOCALKEY="test-private-key"
export WG_TEST_IP_ADD_STATUS=0 WG_TEST_IP_MTU_STATUS=0
export WG_TEST_IP_ADDRESS_STATUS=0 WG_TEST_IP_UP_STATUS=0 WG_TEST_IP_DEL_STATUS=0
export WG_TEST_IPTABLES_ADD_STATUS=0 WG_TEST_IPTABLES_DEL_STATUS=0
export WG_TEST_WG_KEY_STATUS=0 WG_TEST_WG_PEER_STATUS=0

set_firewall_count()
{
	printf '%s\n' "$1" > "$WG_TEST_FIREWALL"
}

sh -n "$script"
: > "$WG_TEST_EVENTS"
rm -f "$WG_TEST_INTERFACE" "$WG_TEST_FIREWALL" "$WG_TEST_KEY_PATH"

# Startup proves the daemon commands succeeded, writes a 0600 private key,
# creates one NAT rule, and deletes the staged key immediately afterwards.
if /bin/sh "$script" start; then status=0; else status=$?; fi
assert_status 0 "$status"
[ -f "$WG_TEST_INTERFACE" ] || fail "successful start did not create wg0"
[ -f "$WG_TEST_FIREWALL" ] || fail "successful start did not add NAT rule"
[ -f "$WG_TEST_KEY_PATH" ] || fail "private-key consumer was never called"
key_file=$(cat "$WG_TEST_KEY_PATH")
[ ! -e "$key_file" ] || fail "private-key staging file was not removed"
[ "$(grep -c '^ip link add dev wg0 type wireguard$' "$WG_TEST_EVENTS")" -eq 1 ] || fail "interface creation count mismatch"
[ "$(grep -c '^iptables -t nat -A POSTROUTING -o wg0 -j MASQUERADE$' "$WG_TEST_EVENTS")" -eq 1 ] || fail "NAT rule append count mismatch"

# Repeated start updates the existing device without duplicating resources.
: > "$WG_TEST_EVENTS"
if /bin/sh "$script" start; then status=0; else status=$?; fi
assert_status 0 "$status"
[ "$(grep -c '^ip link add dev wg0 type wireguard$' "$WG_TEST_EVENTS" || true)" -eq 0 ] || fail "duplicate interface creation"
[ "$(grep -c '^iptables -t nat -A POSTROUTING -o wg0 -j MASQUERADE$' "$WG_TEST_EVENTS" || true)" -eq 0 ] || fail "duplicate NAT rule"

# Missing configuration fails before interface or private-key side effects.
rm -f "$WG_TEST_INTERFACE" "$WG_TEST_FIREWALL" "$WG_TEST_KEY_PATH"
if WG_TEST_PEERIP= /bin/sh "$script" start; then status=0; else status=$?; fi
assert_status 2 "$status"
[ ! -e "$WG_TEST_INTERFACE" ] || fail "missing endpoint created wg0"
[ ! -e "$WG_TEST_KEY_PATH" ] || fail "missing endpoint staged private key"

# Peer setup failure preserves its exact status and rolls back only this
# invocation's newly-created interface.
rm -f "$WG_TEST_INTERFACE" "$WG_TEST_FIREWALL" "$WG_TEST_KEY_PATH"
if WG_TEST_WG_PEER_STATUS=23 /bin/sh "$script" start; then status=0; else status=$?; fi
assert_status 23 "$status"
[ ! -e "$WG_TEST_INTERFACE" ] || fail "peer failure left newly-created interface behind"
assert_key_removed

# Link activation failure cleans both resources created during this startup.
rm -f "$WG_TEST_INTERFACE" "$WG_TEST_FIREWALL" "$WG_TEST_KEY_PATH"
if WG_TEST_IP_UP_STATUS=29 /bin/sh "$script" start; then status=0; else status=$?; fi
assert_status 29 "$status"
[ ! -e "$WG_TEST_INTERFACE" ] || fail "link-up failure left interface behind"
[ ! -e "$WG_TEST_FIREWALL" ] || fail "link-up failure left added NAT rule behind"
assert_key_removed

# Failed in-place updates must not delete a pre-existing interface.
: > "$WG_TEST_INTERFACE"
rm -f "$WG_TEST_FIREWALL" "$WG_TEST_KEY_PATH"
if WG_TEST_IP_MTU_STATUS=31 /bin/sh "$script" start; then status=0; else status=$?; fi
assert_status 31 "$status"
[ -f "$WG_TEST_INTERFACE" ] || fail "failed update deleted a pre-existing interface"
assert_key_removed

# Stop removes every exact rule left by legacy starts, not just the first.
# The old startup command appended the same MASQUERADE rule on each restart.
set_firewall_count 4
: > "$WG_TEST_INTERFACE"
: > "$WG_TEST_EVENTS"
if /bin/sh "$script" stop; then status=0; else status=$?; fi
assert_status 0 "$status"
[ ! -e "$WG_TEST_INTERFACE" ] || fail "stop left wg0 behind"
[ ! -e "$WG_TEST_FIREWALL" ] || fail "stop left duplicate masquerade rules behind"
[ "$(grep -c '^iptables -t nat -D POSTROUTING -o wg0 -j MASQUERADE$' "$WG_TEST_EVENTS")" -eq 4 ] || fail "stop did not remove every duplicate NAT rule"

# Repeated stop is safe when both resources are already absent.
if /bin/sh "$script" stop; then status=0; else status=$?; fi
assert_status 0 "$status"

# Interface deletion failure remains visible even after all firewall rules are removed.
: > "$WG_TEST_INTERFACE"
set_firewall_count 1
if WG_TEST_IP_DEL_STATUS=41 /bin/sh "$script" stop; then status=0; else status=$?; fi
assert_status 41 "$status"
[ -f "$WG_TEST_INTERFACE" ] || fail "failed delete unexpectedly removed interface"
[ ! -e "$WG_TEST_FIREWALL" ] || fail "stop skipped firewall cleanup after interface failure"

# Firewall deletion failure is returned while interface teardown still runs.
set_firewall_count 1
: > "$WG_TEST_INTERFACE"
if WG_TEST_IPTABLES_DEL_STATUS=43 /bin/sh "$script" stop; then status=0; else status=$?; fi
assert_status 43 "$status"
[ -f "$WG_TEST_FIREWALL" ] || fail "failed firewall deletion unexpectedly removed rule"
[ ! -e "$WG_TEST_INTERFACE" ] || fail "stop skipped interface deletion after firewall failure"

# Interface creation failure must not continue with key or firewall setup.
rm -f "$WG_TEST_INTERFACE" "$WG_TEST_FIREWALL" "$WG_TEST_KEY_PATH"
if WG_TEST_IP_ADD_STATUS=47 /bin/sh "$script" start; then status=0; else status=$?; fi
assert_status 47 "$status"
[ ! -e "$WG_TEST_INTERFACE" ] || fail "failed interface creation changed interface state"
assert_key_removed

echo "WireGuard startup, key confinement, rollback and stop regressions: PASS"
