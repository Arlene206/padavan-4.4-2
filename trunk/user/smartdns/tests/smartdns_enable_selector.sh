#!/bin/sh
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
T=$(mktemp -d "${TMPDIR:-/tmp}/smartdns-enable-selector.XXXXXX")
trap 'rm -rf "$T"' 0
trap 'exit 1' HUP INT TERM
fail() { echo "SmartDNS enable selector regression: FAIL: $*" >&2; exit 1; }
SOURCE="$HERE/../smartdns.sh"
FUNC="$T/enable-selector.sh"
sed -n '/^smartdns_validate_enable_selector()/,/^}/p' "$SOURCE" > "$FUNC"
[ -s "$FUNC" ] || fail "enable selector validator was not found"
. "$FUNC"
LOG="$T/log"
: > "$LOG"
logger() { printf '%s\n' "$*" >> "$LOG"; }

for value in '' 0 1; do
    smartdns_validate_enable_selector "$value" ||
        fail "valid enable selector '$value' was rejected"
done
for value in unexpected -1 2 '1;reboot' '1 0'; do
    if smartdns_validate_enable_selector "$value"; then
        fail "invalid enable selector '$value' was accepted"
    else
        status=$?
    fi
    [ "$status" -eq 22 ] ||
        fail "invalid enable selector '$value' returned $status instead of EINVAL"
done
grep -Fq 'Invalid sdns_enable selector; refusing to change service state' "$LOG" ||
    fail "invalid selector rejection was not logged"
echo "SmartDNS binary enable selector validation: PASS"
