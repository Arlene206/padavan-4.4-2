#!/bin/sh
set -eu
TEST_DIR="$(mktemp -d)"
trap 'rm -rf "$TEST_DIR"' EXIT HUP INT TERM
SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
HELPER="$SCRIPT_DIR/../aliddns-ipv4.sh"
SOURCE="$SCRIPT_DIR/../aliddns.sh"
FUNCTIONS="$TEST_DIR/address-functions.sh"
DDNS_FUNCTIONS="$TEST_DIR/ddns-functions.sh"
FAKE_DDNS_HELPER="$TEST_DIR/aliddns-ipv4.sh"
DDNS_HELPER_LOG="$TEST_DIR/ddns-helper.log"
MOCK_BIN="$TEST_DIR/bin"
mkdir -p "$MOCK_BIN"
: > "$FUNCTIONS"
: > "$DDNS_HELPER_LOG"
sed -n '/^aliddns_ipv4_valid() {/,/^}/p' "$SOURCE" >> "$FUNCTIONS"
sed -n '/^aliddns_ipv6_valid() {/,/^}/p' "$SOURCE" >> "$FUNCTIONS"
sed -n '/^arIpAddress () {/,/^}/p' "$SCRIPT_DIR/../ddns_script.sh" |
	sed "s|/usr/bin/aliddns-ipv4.sh|$FAKE_DDNS_HELPER|g" > "$DDNS_FUNCTIONS"
. "$FUNCTIONS"
. "$DDNS_FUNCTIONS"
export DDNS_HELPER_LOG

CA_BUNDLE="$TEST_DIR/ca-certificates.crt"
CURL_ARGS_LOG="$TEST_DIR/curl-args.log"
CURL_COUNT_FILE="$TEST_DIR/curl-count"
: > "$CA_BUNDLE"
: > "$CURL_COUNT_FILE"
export ALIDDNS_CA_BUNDLE="$CA_BUNDLE" CURL_ARGS_LOG CURL_COUNT_FILE

cat > "$MOCK_BIN/logger" <<'MOCK_LOGGER'
#!/bin/sh
exit 0
MOCK_LOGGER

cat > "$MOCK_BIN/nvram" <<'MOCK_NVRAM'
#!/bin/sh
[ "$#" -eq 2 ] && [ "$1" = get_checked ] && [ "$2" = pppoemwan_enable ] || exit 91
status=${FAKE_NVRAM_STATUS:-0}
[ "$status" -eq 0 ] || exit "$status"
printf '%s\n' "${FAKE_PPPOE_MWAN-}"
MOCK_NVRAM

cat > "$FAKE_DDNS_HELPER" <<'MOCK_DDNS_HELPER'
#!/bin/sh
printf 'argc=%s arg=%s\n' "$#" "${1-}" >> "$DDNS_HELPER_LOG"
printf '%s\n' '8.8.8.8'
MOCK_DDNS_HELPER

cat > "$MOCK_BIN/curl" <<'MOCK_CURL'
#!/bin/sh
printf '%s\n' "$@" > "$CURL_ARGS_LOG"
count=0
IFS= read -r count < "$CURL_COUNT_FILE" || :
count=$((count + 1))
printf '%s\n' "$count" > "$CURL_COUNT_FILE"
case "$FAKE_SCENARIO" in
	valid) printf '%s\n' '8.8.8.8' '200' ;;
	special-address) printf '%s\n' "$FAKE_IP" '200' ;;
	tls-failure) exit 60 ;;
	timeout) exit 28 ;;
	http-failure) printf '%s\n' 'upstream error' '503'; exit 22 ;;
	redirect) printf '%s\n' 'redirect body' '302' ;;
	non-200) printf '%s\n' '8.8.8.8' '201' ;;
	*) exit 90 ;;
esac
MOCK_CURL
chmod 700 "$MOCK_BIN/logger" "$MOCK_BIN/nvram" "$MOCK_BIN/curl" "$FAKE_DDNS_HELPER"
PATH="$MOCK_BIN:$PATH"
export PATH

reset_case() {
	FAKE_SCENARIO="$1"
	export FAKE_SCENARIO
	: > "$CURL_ARGS_LOG"
}
fail() {
	echo "AliDDNS public IPv4 lookup regression: FAIL: $1" >&2
	exit 1
}

# Address validators reject empty, malformed, ambiguous and multi-line data.
aliddns_ipv4_valid "203.0.113.9" || fail "valid IPv4 was rejected"
for invalid in "" "256.1.2.3" "1.2.3" "1.2.3.4.5" "01.2.3.4" "1.2.3.4
198.51.100.2" "1.2.3.x"
do
	if aliddns_ipv4_valid "$invalid"; then fail "invalid IPv4 accepted: $invalid"; fi
done
aliddns_ipv6_valid "2001:db8::1" || fail "compressed IPv6 was rejected"
aliddns_ipv6_valid "2001:db8:0:0:0:0:0:1" || fail "full IPv6 was rejected"
aliddns_ipv6_valid "::" || fail "compressed zero IPv6 was rejected"
for invalid in "" "2001:db8::zz" "2001::1::2" ":::1" "1:2:3:4:5:6:7" \
	"1:2:3:4:5:6:7:8::" "1:2:3:4:5:6:7:8:9" "2001:db8::1
2001:db8::2"
do
	if aliddns_ipv6_valid "$invalid"; then fail "invalid IPv6 accepted: $invalid"; fi
done

# The helper pins the endpoint, certificate bundle, protocol and time bounds.
reset_case valid
result=$("$HELPER") || fail "valid HTTPS IPv4 response was rejected"
[ "$result" = "8.8.8.8" ] || fail "helper did not emit exactly one public IPv4 address"
grep -Fqx -- '--fail' "$CURL_ARGS_LOG" || fail "curl is missing --fail"
grep -Fqx -- '--ipv4' "$CURL_ARGS_LOG" || fail "curl is not pinned to IPv4 transport"
grep -Fqx -- '--connect-timeout' "$CURL_ARGS_LOG" || fail "curl is missing a connect timeout"
grep -Fqx -- '5' "$CURL_ARGS_LOG" || fail "connect timeout is not five seconds"
grep -Fqx -- '--max-time' "$CURL_ARGS_LOG" || fail "curl is missing a total timeout"
grep -Fqx -- '15' "$CURL_ARGS_LOG" || fail "total timeout is not fifteen seconds"
grep -Fqx -- '--proto' "$CURL_ARGS_LOG" || fail "curl is missing a protocol allowlist"
grep -Fqx -- '=https' "$CURL_ARGS_LOG" || fail "protocol allowlist is not HTTPS-only"
grep -Fqx -- '--cacert' "$CURL_ARGS_LOG" || fail "curl is missing the CA bundle"
grep -Fqx -- "$CA_BUNDLE" "$CURL_ARGS_LOG" || fail "curl did not use the CA bundle"
[ "$(tail -n 1 "$CURL_ARGS_LOG")" = "https://api.ipify.org" ] || fail "unexpected public-IP endpoint"
! grep -Fqx -- '-k' "$CURL_ARGS_LOG" || fail "TLS verification bypass was enabled"
! grep -Fqx -- '--insecure' "$CURL_ARGS_LOG" || fail "TLS verification bypass was enabled"
! grep -Fqx -- '-L' "$CURL_ARGS_LOG" || fail "redirect following was enabled"
! grep -Fqx -- '--location' "$CURL_ARGS_LOG" || fail "redirect following was enabled"

# Missing CA must fail before curl is invoked.
count_before="$(cat "$CURL_COUNT_FILE")"
if ALIDDNS_CA_BUNDLE="$TEST_DIR/missing-ca.pem" "$HELPER"; then
	fail "lookup succeeded without a readable CA bundle"
fi
[ "$(cat "$CURL_COUNT_FILE")" = "$count_before" ] || fail "curl ran without a CA bundle"

for scenario in tls-failure timeout http-failure redirect non-200; do
	reset_case "$scenario"
	if "$HELPER"; then fail "transport failure was accepted: $scenario"; fi
	[ "$(cat "$CURL_COUNT_FILE")" -eq $((count_before + 1)) ] && count_before="$(cat "$CURL_COUNT_FILE")" ||
		fail "failure case retried without a bounded retry policy: $scenario"
done

# A WAN-discovery endpoint must not trick DDNS into publishing special-use
# addresses if a proxy, captive portal, or upstream service returns bad data.
for reserved in \
	0.1.2.3 \
	10.1.2.3 \
	100.64.0.1 \
	100.127.255.254 \
	127.0.0.1 \
	169.254.1.1 \
	172.16.0.1 \
	172.31.255.254 \
	192.0.0.1 \
	192.0.2.1 \
	192.88.99.1 \
	192.168.1.1 \
	198.18.0.1 \
	198.19.255.254 \
	198.51.100.1 \
	203.0.113.1 \
	224.0.0.1 \
	240.0.0.1 \
	255.255.255.255
do
	reset_case special-address
	FAKE_IP="$reserved"
	export FAKE_IP
	if "$HELPER" >/dev/null 2>&1; then
		fail "non-public IPv4 accepted by WAN discovery: $reserved"
	fi
done

# IANA protocol-assignment exceptions that are globally reachable stay valid.
reset_case special-address
FAKE_IP=192.0.0.9
export FAKE_IP
result=$("$HELPER") || fail "globally reachable IETF anycast address was rejected"
[ "$result" = "192.0.0.9" ] || fail "IETF anycast address was changed"

# Checked WAN-mode reads preserve legacy defaults but never fall back on backend errors.
FAKE_NVRAM_STATUS=0
FAKE_PPPOE_MWAN=0
export FAKE_NVRAM_STATUS FAKE_PPPOE_MWAN
result=$(arIpAddress) || fail "checked ordinary-WAN selection failed"
[ "$result" = "8.8.8.8" ] || fail "IPv4 DDNS wrapper did not relay helper output"
grep -Fqx 'argc=0 arg=' "$DDNS_HELPER_LOG" || fail "disabled PPPoE-MWAN did not select ordinary WAN"
FAKE_PPPOE_MWAN=1
export FAKE_PPPOE_MWAN
result=$(arIpAddress) || fail "checked PPPoE-MWAN selection failed"
grep -Fqx 'argc=1 arg=ppp0' "$DDNS_HELPER_LOG" || fail "enabled PPPoE-MWAN did not bind helper to ppp0"
helper_calls_before=$(wc -l < "$DDNS_HELPER_LOG")
FAKE_NVRAM_STATUS=7
export FAKE_NVRAM_STATUS
if arIpAddress >/dev/null 2>&1; then
	fail "IPv4 DDNS wrapper ignored a WAN-mode NVRAM read error"
fi
[ "$(wc -l < "$DDNS_HELPER_LOG")" -eq "$helper_calls_before" ] || fail "IPv4 discovery ran after WAN-mode NVRAM failure"
FAKE_NVRAM_STATUS=0
FAKE_PPPOE_MWAN=
export FAKE_NVRAM_STATUS FAKE_PPPOE_MWAN
result=$(arIpAddress) || fail "missing PPPoE-MWAN key did not preserve legacy behavior"
grep -Fqx 'argc=0 arg=' "$DDNS_HELPER_LOG" || fail "missing PPPoE-MWAN key did not select ordinary WAN"

# Keep the previous PPPoE source-interface policy with a validated one-item argument.
reset_case valid
result=$("$HELPER" ppp0) || fail "ppp0-bound request failed"
[ "$result" = "8.8.8.8" ] || fail "ppp0 request changed the address"
grep -Fqx -- '--interface' "$CURL_ARGS_LOG" || fail "ppp0 interface binding was omitted"
grep -Fqx -- 'ppp0' "$CURL_ARGS_LOG" || fail "ppp0 source interface was not passed literally"
count_before="$(cat "$CURL_COUNT_FILE")"
if "$HELPER" 'ppp0;touch /tmp/aliddns-test-owned'; then
	fail "invalid interface argument was accepted"
fi
[ "$(cat "$CURL_COUNT_FILE")" = "$count_before" ] || fail "curl ran for an invalid interface argument"

echo "AliDDNS public IPv4 HTTPS discovery, global-unicast filtering, CA and interface regression: PASS"
echo "AliDDNS strict IPv4/IPv6 address validation regression: PASS"
