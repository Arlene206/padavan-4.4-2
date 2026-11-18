#!/bin/sh
set -eu
TEST_DIR="$(mktemp -d)"
trap 'rm -rf "$TEST_DIR"' EXIT HUP INT TERM
SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
FUNCTIONS="$TEST_DIR/functions.sh"
MOCK_BIN="$TEST_DIR/bin"
IFCONFIG_LOG="$TEST_DIR/ifconfig.log"
mkdir -p "$MOCK_BIN"
: > "$IFCONFIG_LOG"
sed -n '/^arIpAddress6 () {/,/^}/p' "$SCRIPT_DIR/../ddns_script.sh" >> "$FUNCTIONS"
sed -n '/^aliddns_ipv6_valid() {/,/^}/p' "$SCRIPT_DIR/../aliddns.sh" >> "$FUNCTIONS"
sed -n '/^aliddns_ipv6_public_valid() {/,/^}/p' "$SCRIPT_DIR/../aliddns.sh" >> "$FUNCTIONS"
. "$FUNCTIONS"

cat > "$MOCK_BIN/nvram" <<'MOCK_NVRAM'
#!/bin/sh
[ "$#" -eq 2 ] && [ "$1" = get ] && [ "$2" = wan0_ifname6 ] || exit 91
printf '%s\n' "$FAKE_WAN_IFNAME"
MOCK_NVRAM
cat > "$MOCK_BIN/ifconfig" <<'MOCK_IFCONFIG'
#!/bin/sh
[ "$#" -eq 1 ] || exit 92
printf '%s\n' "$1" >> "$IFCONFIG_LOG"
[ "$1" = "$EXPECTED_IFNAME" ] || exit 93
case "$FAKE_IFCONFIG_MODE" in
	global)
		cat <<'OUTPUT'
ppp0 Link encap:Point-to-Point Protocol
          inet6 addr: fe80::1234/64 Scope:Link
          inet6 addr: 2001:4860:4860::8888/64 Scope:Global
OUTPUT
		;;
	sit-global)
		cat <<'OUTPUT'
sit0 Link encap:IPv6-in-IPv4
          inet6 addr: 2001:4860::1/64 Scope:Global
OUTPUT
		;;
	documentation-2001)
		cat <<'OUTPUT'
          inet6 addr: 2001:0db8:0000:0000:0000:0000:0000:0001/64 Scope:Global
OUTPUT
		;;
		benchmarking)
		cat <<'OUTPUT'
          inet6 addr: 2001:0002:0000:0000:0000:0000:0000:0001/64 Scope:Global
OUTPUT
		;;
		orchid-v1)
		cat <<'OUTPUT'
          inet6 addr: 2001:0010:0000:0000:0000:0000:0000:0001/64 Scope:Global
OUTPUT
		;;
	documentation-3fff)
		cat <<'OUTPUT'
          inet6 addr: 3fff:0fff:0000:0000:0000:0000:0000:0001/64 Scope:Global
OUTPUT
		;;
	ula)
		cat <<'OUTPUT'
          inet6 addr: fd12:3456::1/64 Scope:Global
OUTPUT
		;;
	link-local-only)
		cat <<'OUTPUT'
          inet6 addr: fe80::1/64 Scope:Link
OUTPUT
		;;
	multiple-global)
		cat <<'OUTPUT'
          inet6 addr: 2001:4860::1/64 Scope:Global
          inet6 addr: 2606:4700::1/64 Scope:Global
OUTPUT
		;;
	partial-failure)
		printf '%s\n' 'inet6 addr: 2001:4860::1/64 Scope:Global'
		exit 7
		;;
	*) exit 94 ;;
esac
MOCK_IFCONFIG
chmod 755 "$MOCK_BIN/nvram" "$MOCK_BIN/ifconfig"
PATH="$MOCK_BIN:$PATH"
export PATH IFCONFIG_LOG

fail() {
	printf 'AliDDNS WAN IPv6 discovery regression: FAIL: %s\n' "$1" >&2
	exit 1
}
expect_success() {
	FAKE_WAN_IFNAME="$1"
	EXPECTED_IFNAME="$1"
	FAKE_IFCONFIG_MODE="$2"
	export FAKE_WAN_IFNAME EXPECTED_IFNAME FAKE_IFCONFIG_MODE
	actual=$(arIpAddress6) || fail "failed to discover WAN IPv6 on $1"
	[ "$actual" = "$3" ] || fail "wrong address for $1: $actual"
	aliddns_ipv6_valid "$actual" || fail "discovered address failed strict IPv6 validation"
	[ "$(tail -n 1 "$IFCONFIG_LOG")" = "$1" ] || fail "lookup did not use WAN IPv6 interface $1"
}
expect_failure() {
	FAKE_WAN_IFNAME="$1"
	EXPECTED_IFNAME="$2"
	FAKE_IFCONFIG_MODE="$3"
	export FAKE_WAN_IFNAME EXPECTED_IFNAME FAKE_IFCONFIG_MODE
	if actual=$(arIpAddress6); then
		fail "unexpected result for $3: $actual"
	fi
}
expect_success ppp0 global 2001:4860:4860::8888
expect_success sit0 sit-global 2001:4860::1
calls_before=$(wc -l < "$IFCONFIG_LOG")
malicious_ifname="ppp0;touch $TEST_DIR/injected"
expect_failure "" "" global
expect_failure "$malicious_ifname" "$malicious_ifname" global
[ "$(wc -l < "$IFCONFIG_LOG")" -eq "$calls_before" ] || fail "invalid WAN interface reached ifconfig"
expect_failure ppp0 ppp0 documentation-2001
expect_failure ppp0 ppp0 benchmarking
expect_failure ppp0 ppp0 orchid-v1
expect_failure ppp0 ppp0 documentation-3fff
expect_failure ppp0 ppp0 ula
expect_failure ppp0 ppp0 link-local-only
expect_failure ppp0 ppp0 multiple-global
expect_failure ppp0 ppp0 partial-failure
[ ! -e "$TEST_DIR/injected" ] || fail "WAN interface was evaluated as shell code"
if aliddns_ipv6_valid '2001::1::2'; then
	fail "malformed IPv6 passed the strict validator"
fi
if aliddns_ipv6_public_valid '2001:db8::1'; then
	fail "documentation IPv6 passed the public-address policy"
fi
if aliddns_ipv6_public_valid '3fff:0fff::1'; then
	fail "new documentation IPv6 range passed the public-address policy"
fi
if ! aliddns_ipv6_public_valid '2001:4860:4860::8888'; then
	fail "a routable global-unicast IPv6 address was rejected"
fi
printf '%s\n' 'AliDDNS WAN IPv6 discovery regression: PASS'
