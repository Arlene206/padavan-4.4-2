#!/bin/sh
set -eu
TEST_DIR=$(mktemp -d)
trap 'rm -rf "$TEST_DIR"' EXIT HUP INT TERM
SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
SOURCE="$SCRIPT_DIR/../adbyby.sh"
FUNCTIONS="$TEST_DIR/functions.sh"
sed -n \
 -e '/^adbyby_valid_https_url()/,/^}/p' \
 -e '/^adbyby_download_https_200()/,/^}/p' \
 -e '/^adbyby_remove_generated_hosts_config()/,/^}/p' \
 -e '/^adbyby_validate_hosts_file()/,/^}/p' \
 -e '/^adbyby_prepare_hosts_transaction()/,/^)/p' \
 -e '/^adbyby_hosts_cache_is_usable()/,/^}/p' \
 -e '/^hosts_ads()/,/^}/p' "$SOURCE" > "$FUNCTIONS"
. "$FUNCTIONS"
PROG_PATH="$TEST_DIR/adbyby"
ADBYBY_CA_BUNDLE="$TEST_DIR/ca.pem"
ADBYBY_HOST_LIST_FILE="$TEST_DIR/adbyby_host.sh"
ADBYBY_HOSTS_CONFIG_FILE="$TEST_DIR/dnsmasq.conf"
ADBYBY_HOST_SOURCE_MAX_BYTES=4194304
ADBYBY_HOST_TOTAL_MAX_BYTES=16777216
ADBYBY_HOST_MAX_CONFIG_BYTES=314571
ADBYBY_HOST_MAX_SOURCES=16
CURL_LOG="$TEST_DIR/curl.log"
CURL_ERRORS="$TEST_DIR/curl-errors"
LOGGER_LOG="$TEST_DIR/logger.log"
FIXTURE_ONE="$TEST_DIR/source-one"
FIXTURE_TWO="$TEST_DIR/source-two"
HOSTS_ENABLED=1
HOSTS_COUNT=0
mock_mode=valid
fail() { echo "Adbyby host-list regression: FAIL: $1" >&2; exit 1; }
mkdir -p "$PROG_PATH"
printf '%s\n' 'test CA bundle' > "$ADBYBY_CA_BUNDLE"
cat > "$FIXTURE_ONE" <<'EOF'
# upstream comment
0.0.0.0 ads.example tracker.example
127.0.0.1 loopback.example
::1 localhost ip6-localhost
EOF
cat > "$FIXTURE_TWO" <<'EOF'
0.0.0.0 duplicate.example
0.0.0.0 ads.example tracker.example
EOF
URL_ONE='https://rules.example/source1?tag=a%26b&mode=stable'
URL_TWO='https://rules.example/source2'
URL_THREE='https://rules.example/source3'
nvram() {
	case "$1" in
		get) [ "$2" = hosts_ad ] || return 1; printf '%s\n' "$HOSTS_ENABLED" ;;
		set)
			case "$2" in
				adbyby_hostsad=*) HOSTS_COUNT=$(printf '%s\n' "$2" | sed 's/^[^=]*=//') ;;
				*) return 1 ;;
			esac
			;;
		*) return 1 ;;
	esac
}
logger() { printf '%s\n' "$*" >> "$LOGGER_LOG"; }
curl() {
	curl_output= curl_url= curl_ca= curl_proto= curl_max_file=
	while [ "$#" -gt 0 ]; do
		case "$1" in
			--cacert) curl_ca="$2"; shift 2 ;;
			--proto) curl_proto="$2"; shift 2 ;;
			--connect-timeout|--max-time|--retry|--limit-rate) shift 2 ;;
			--max-filesize) curl_max_file="$2"; shift 2 ;;
			--output) curl_output="$2"; shift 2 ;;
			--write-out) shift 2 ;;
			--fail|--silent|--show-error) shift ;;
			*)
				case "$1" in
					https://*) curl_url="$1"; shift ;;
					*) printf 'unexpected curl argument: %s\n' "$1" >> "$CURL_ERRORS"; return 1 ;;
				esac
				;;
		esac
	done
	[ -n "$curl_output" ] && [ -n "$curl_url" ] || { echo "missing output or URL" >> "$CURL_ERRORS"; return 1; }
	[ "$curl_ca" = "$ADBYBY_CA_BUNDLE" ] || { echo "wrong CA bundle" >> "$CURL_ERRORS"; return 1; }
	[ "$curl_proto" = "=https" ] || { echo "HTTPS-only option missing" >> "$CURL_ERRORS"; return 1; }
	[ "$curl_max_file" = "$ADBYBY_HOST_SOURCE_MAX_BYTES" ] || { echo "source byte cap missing" >> "$CURL_ERRORS"; return 1; }
	[ "$(stat -c '%a' "$(dirname "$curl_output")")" = 700 ] || { echo "staging dir not private" >> "$CURL_ERRORS"; return 1; }
	printf '%s\n' "$curl_url" >> "$CURL_LOG"
	case "$mock_mode" in
		tls_failure) printf 'partial body' > "$curl_output"; printf '000'; return 60 ;;
		http_redirect) printf 'redirect body' > "$curl_output"; printf '302'; return 0 ;;
		http_failure) printf 'error body' > "$curl_output"; printf '404'; return 22 ;;
		malformed) printf 'conf-file=/etc/passwd\n' > "$curl_output"; printf '200'; return 0 ;;
		oversized) dd if=/dev/zero of="$curl_output" bs=65 count=1 2>/dev/null; printf '200'; return 0 ;;
		aggregate)
			cat > "$curl_output" <<'EOF'
0.0.0.0 aggregate-one.example
0.0.0.0 aggregate-two.example
0.0.0.0 aggregate-three.example
0.0.0.0 aggregate-four.example
0.0.0.0 aggregate-five.example
EOF
			printf '200'; return 0
			;;
		valid)
			case "$curl_url" in
				*source1*) cp "$FIXTURE_ONE" "$curl_output" ;;
				*source2*) cp "$FIXTURE_TWO" "$curl_output" ;;
				*) echo "unexpected fixture URL" >> "$CURL_ERRORS"; return 1 ;;
			esac
			printf '200'; return 0
			;;
		*) echo "unknown mock mode" >> "$CURL_ERRORS"; return 1 ;;
	esac
}
write_two_source_config() {
	cat > "$ADBYBY_HOST_LIST_FILE" <<EOF
# comments skipped
$URL_ONE
$URL_TWO
EOF
}
write_dnsmasq_config() {
	{
		printf '%s\n' 'conf-file=/etc/storage/operator.conf'
		printf '%s\n' 'addn-hosts=/etc/storage/operator-hosts'
		printf 'addn-hosts=%s\n' "$PROG_PATH/hosts"
	} > "$ADBYBY_HOSTS_CONFIG_FILE"
}
assert_no_staging() {
	for stage in "$PROG_PATH"/.adbyby-hosts.*; do
		[ ! -e "$stage" ] || fail "private stage not cleaned"
	done
}
: > "$CURL_LOG"; : > "$CURL_ERRORS"; : > "$LOGGER_LOG"
write_two_source_config; write_dnsmasq_config

# URL splitting and all-source aggregation preserve query delimiters and deduplicate exact records.
hosts_ads || fail "valid aggregation failed"
grep -Fx "$URL_ONE" "$CURL_LOG" >/dev/null || fail "URL query was split or rewritten"
grep -Fx "$URL_TWO" "$CURL_LOG" >/dev/null || fail "second source was skipped"
[ "$(wc -l < "$CURL_LOG")" -eq 2 ] || fail "wrong URL count"
[ "$(wc -l < "$PROG_PATH/hosts")" -eq 4 ] || fail "aggregation/deduplication mismatch"
grep -F '0.0.0.0 ads.example tracker.example' "$PROG_PATH/hosts" >/dev/null || fail "IPv4 hosts mapping missing"
grep -F '::1 localhost ip6-localhost' "$PROG_PATH/hosts" >/dev/null || fail "IPv6 hosts mapping missing"
grep -F 'addn-hosts=/etc/storage/operator-hosts' "$ADBYBY_HOSTS_CONFIG_FILE" >/dev/null || fail "operator config line changed"
[ "$(grep -Fc "addn-hosts=$PROG_PATH/hosts" "$ADBYBY_HOSTS_CONFIG_FILE")" -eq 1 ] || fail "managed entry not idempotent"
[ "$HOSTS_COUNT" = 4 ] || fail "wrong NVRAM record count"
[ "$(stat -c '%a' "$PROG_PATH/hosts")" = 644 ] || fail "wrong published file mode"
GOOD_HOSTS=$(cat "$PROG_PATH/hosts")
assert_no_staging

# TLS, redirect and HTTP failures preserve the last-good file.
for mock_mode in tls_failure http_redirect http_failure; do
	: > "$CURL_LOG"; printf '%s\n' "$URL_ONE" > "$ADBYBY_HOST_LIST_FILE"
	hosts_ads || fail "$mock_mode did not fall back"
	[ "$(cat "$PROG_PATH/hosts")" = "$GOOD_HOSTS" ] || fail "$mock_mode replaced last-good"
done

# Invalid schemes and whitespace are rejected before curl.
for invalid_url in 'http://rules.example/hosts' 'https://rules.example/bad path'; do
	: > "$CURL_LOG"; printf '%s\n' "$invalid_url" > "$ADBYBY_HOST_LIST_FILE"
	hosts_ads || fail "invalid URL did not use fallback"
	[ ! -s "$CURL_LOG" ] || fail "invalid URL reached curl"
	[ "$(cat "$PROG_PATH/hosts")" = "$GOOD_HOSTS" ] || fail "invalid URL replaced last-good"
done

# Missing firmware CA prevents network access and keeps last-good.
: > "$CURL_LOG"; printf '%s\n' "$URL_ONE" > "$ADBYBY_HOST_LIST_FILE"
ADBYBY_CA_BUNDLE="$TEST_DIR/missing-ca"
hosts_ads || fail "missing CA did not use fallback"
[ ! -s "$CURL_LOG" ] || fail "curl ran without CA"
[ "$(cat "$PROG_PATH/hosts")" = "$GOOD_HOSTS" ] || fail "missing CA replaced last-good"
ADBYBY_CA_BUNDLE="$TEST_DIR/ca.pem"

# Malformed and oversized bodies are rejected without replacing last-good.
mock_mode=malformed
hosts_ads || fail "malformed response did not use fallback"
[ "$(cat "$PROG_PATH/hosts")" = "$GOOD_HOSTS" ] || fail "malformed response replaced last-good"
mock_mode=oversized
ADBYBY_HOST_SOURCE_MAX_BYTES=64
hosts_ads || fail "oversized response did not use fallback"
[ "$(cat "$PROG_PATH/hosts")" = "$GOOD_HOSTS" ] || fail "oversized response replaced last-good"
ADBYBY_HOST_SOURCE_MAX_BYTES=4194304

# Aggregate size is bounded across all sources, not only per transfer.
mock_mode=aggregate
ADBYBY_HOST_SOURCE_MAX_BYTES=256
ADBYBY_HOST_TOTAL_MAX_BYTES=180
cat > "$ADBYBY_HOST_LIST_FILE" <<EOF
$URL_ONE
$URL_THREE
EOF
hosts_ads || fail "aggregate overflow did not use fallback"
[ "$(cat "$PROG_PATH/hosts")" = "$GOOD_HOSTS" ] || fail "aggregate overflow replaced last-good"
ADBYBY_HOST_SOURCE_MAX_BYTES=4194304
ADBYBY_HOST_TOTAL_MAX_BYTES=16777216

# An unsuccessful final rename is a publication failure and leaves last-good intact.
mock_mode=valid; write_two_source_config
mv() { return 1; }
hosts_ads || fail "publication failure did not use last-good"
unset -f mv
[ "$(cat "$PROG_PATH/hosts")" = "$GOOD_HOSTS" ] || fail "failed rename changed live file"
assert_no_staging

# Without a validated cache, a malformed required source publishes nothing.
rm -f "$PROG_PATH/hosts"; mock_mode=malformed
printf '%s\n' "$URL_ONE" > "$ADBYBY_HOST_LIST_FILE"
if hosts_ads; then fail "malformed source without last-good was accepted"; fi
[ ! -e "$PROG_PATH/hosts" ] || fail "bad source created a hosts file"
! grep -F "addn-hosts=$PROG_PATH/hosts" "$ADBYBY_HOSTS_CONFIG_FILE" >/dev/null || fail "invalid source left addn-hosts enabled"
[ "$HOSTS_COUNT" = 0 ] || fail "stale NVRAM count remained"

HOSTS_ENABLED=0; : > "$CURL_LOG"
hosts_ads || fail "disabled mode failed"
[ "$HOSTS_COUNT" = 0 ] && [ ! -s "$CURL_LOG" ] || fail "disabled mode count/download changed"

# A stale /tmp/host.txt scratch file must never be consumed by the production path.
grep -F '/tmp/host.txt' "$SOURCE" >/dev/null && fail "legacy shared output path remains"
[ ! -s "$CURL_ERRORS" ] || fail "$(cat "$CURL_ERRORS")"
assert_no_staging
echo "Adbyby configured hosts URL splitting, bounded HTTPS, malformed/oversized rejection, aggregate validation, last-good fallback and atomic publication: PASS"
