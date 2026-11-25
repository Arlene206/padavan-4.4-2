#!/bin/sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname "$0")/../../../.." && pwd)
SMARTDNS_SCRIPT="$ROOT/trunk/user/smartdns/smartdns.sh"
TMP_DIR=$(mktemp -d /tmp/smartdns-adblock-preservation.XXXXXX) || exit 1
trap 'rm -rf "$TMP_DIR"' 0 HUP INT TERM

fail() { echo "FAIL: $*" >&2; exit 1; }
pass() { echo "PASS: $*"; }

{
    awk '
        $0 == "smartdns_prepare_adblock_filter()" { copying = 1 }
        $0 == "Start_AD () {" { exit }
        copying { print }
    ' "$SMARTDNS_SCRIPT"
    awk '
        $0 == "Start_AD () {" { copying = 1 }
        $0 == "Change_adbyby () {" { exit }
        copying { print }
    ' "$SMARTDNS_SCRIPT"
} | sed "s|_smartdns_adblock_output=/tmp/anti-ad-for-smartdns.conf|_smartdns_adblock_output=$TMP_DIR/anti-ad-for-smartdns.conf|" > "$TMP_DIR/functions.sh"

. "$TMP_DIR/functions.sh"
smartdns_adblock_output="$TMP_DIR/anti-ad-for-smartdns.conf"
sdns_adblock_url="https://filters.example.test/list"

logger() { :; }
curl() {
    output=
    while [ "$#" -gt 0 ]; do
        case "$1" in
            -o) output=$2; shift 2 ;;
            *) shift ;;
        esac
    done
    [ -n "$output" ] || return 2
    [ "$CURL_STATUS" -eq 0 ] || return "$CURL_STATUS"
    printf '%s' "$CURL_BODY" > "$output"
}

printf '%s\n' 'address=/previous.example/0.0.0.0' > "$smartdns_adblock_output"
CURL_STATUS=7
CURL_BODY=
if Start_AD; then fail "curl failure unexpectedly succeeded"; fi
grep -Fqx 'address=/previous.example/0.0.0.0' "$smartdns_adblock_output" || fail "curl failure erased the last valid filter"
pass "download failure preserves the previous valid filter"

CURL_STATUS=0
CURL_BODY=$(printf '%s\n' 'address=/previous.example/0.0.0.0' 'server 9.9.9.9')
if Start_AD; then fail "mixed native config unexpectedly succeeded"; fi
grep -Fqx 'address=/previous.example/0.0.0.0' "$smartdns_adblock_output" || fail "invalid directive replaced the previous filter"
[ "$(wc -l < "$smartdns_adblock_output")" -eq 1 ] || fail "invalid native config changed previous output"
pass "invalid native directive preserves the previous valid filter"

CURL_BODY=$(printf '%s\n' 'address=/new.example/0.0.0.0' '# refreshed')
Start_AD || fail "valid native filter failed"
[ "$(cat "$smartdns_adblock_output")" = "$CURL_BODY" ] || fail "valid replacement content mismatch"
pass "valid native filter replaces the previous output"

rm -f "$smartdns_adblock_output"
CURL_STATUS=7
CURL_BODY=
if Start_AD; then fail "first download failure unexpectedly succeeded"; fi
[ -f "$smartdns_adblock_output" ] && [ ! -s "$smartdns_adblock_output" ] || fail "fresh install did not keep an empty default filter"
pass "first failed download initializes an empty filter"

rm -f "$smartdns_adblock_output"
printf '%s\n' "protected" > "$TMP_DIR/protected"
ln -s "$TMP_DIR/protected" "$smartdns_adblock_output"
CURL_STATUS=0
CURL_BODY='address=/new.example/0.0.0.0'
if Start_AD; then fail "symlinked filter target unexpectedly succeeded"; fi
[ -L "$smartdns_adblock_output" ] || fail "symlinked target was replaced"
[ "$(cat "$TMP_DIR/protected")" = "protected" ] || fail "symlink target was modified"
pass "symlinked filter target is rejected without following it"

echo "SmartDNS adblock previous-output preservation: PASS"
