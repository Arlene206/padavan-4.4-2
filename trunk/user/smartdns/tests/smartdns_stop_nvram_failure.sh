#!/bin/sh
set -eu

HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
T=$(mktemp -d "${TMPDIR:-/tmp}/smartdns-stop-nvram.XXXXXX")
trap 'rm -rf "$T"' 0
trap 'exit 1' HUP INT TERM
fail() { echo "SmartDNS lifecycle NVRAM regression: FAIL: $*" >&2; exit 1; }

SOURCE="$HERE/../smartdns.sh"
FUNCTIONS="$T/functions.sh"
READS="$T/reads"
: > "$FUNCTIONS"
sed -n '/^smartdns_nvram_read_checked()/,/^}/p' "$SOURCE" >> "$FUNCTIONS"
sed -n '/^smartdns_validate_enable_selector()/,/^}/p' "$SOURCE" >> "$FUNCTIONS"
sed -n '/^smartdns_load_lifecycle_nvram_state()/,/^}/p' "$SOURCE" >> "$FUNCTIONS"
[ -s "$FUNCTIONS" ] || fail "lifecycle NVRAM functions were not found"
. "$FUNCTIONS"

logger() { :; }
FAIL_KEY=
ENABLE_VALUE=1
nvram()
{
    [ "$1" = "get_checked" ] && [ "$#" -eq 2 ] || return 2
    key=$2
    printf '%s\n' "$key" >> "$READS"
    [ "$key" = "$FAIL_KEY" ] && return 73
    case "$key" in
        sdns_enable) value=$ENABLE_VALUE ;;
        sdns_port) value=5353 ;;
        sdns_redirect) value=1 ;;
        sdnse_enable) value=0 ;;
        sdnse_port) value=5335 ;;
        *) return 2 ;;
    esac
    printf '%s\n' "$value"
}

# Stop must not read any live configuration selector: persisted resolver state
# is loaded later by Read_ini and is sufficient to restore DNSMasq safely.
action=stop
FAIL_KEY=sdns_enable
: > "$READS"
smartdns_load_lifecycle_nvram_state ||
    fail "explicit stop was blocked by an unavailable NVRAM backend"
[ "$sdns_enable" = 0 ] || fail "stop did not select the disabled cleanup path"
[ -z "$sdns_port" ] && [ -z "$sdns_redirect" ] &&
    [ -z "$sdnse_enable" ] && [ -z "$sdnse_port" ] ||
    fail "stop retained transient configuration values"
[ ! -s "$READS" ] ||
    fail "explicit stop unexpectedly read configuration NVRAM"
echo "explicit stop bypasses every configuration NVRAM read: PASS"

# Every non-stop action keeps fail-closed checked reads and the original status.
action=restart
for FAIL_KEY in sdns_enable sdns_port sdns_redirect sdnse_enable sdnse_port; do
    : > "$READS"
    if smartdns_load_lifecycle_nvram_state; then
        fail "restart accepted NVRAM failure for $FAIL_KEY"
    else
        status=$?
    fi
    [ "$status" -eq 73 ] ||
        fail "restart changed $FAIL_KEY failure status to $status"
    case "$FAIL_KEY" in
        sdns_enable) expected=sdns_enable ;;
        sdns_port) expected='sdns_enable
sdns_port' ;;
        sdns_redirect) expected='sdns_enable
sdns_port
sdns_redirect' ;;
        sdnse_enable) expected='sdns_enable
sdns_port
sdns_redirect
sdnse_enable' ;;
        sdnse_port) expected='sdns_enable
sdns_port
sdns_redirect
sdnse_enable
sdnse_port' ;;
    esac
    actual=$(cat "$READS")
    [ "$actual" = "$expected" ] ||
        fail "restart continued reading after $FAIL_KEY failed"
done
echo "each checked NVRAM read preserves its original failure status: PASS"

# Malformed enable selectors must still abort startup before reading settings.
FAIL_KEY=
ENABLE_VALUE='1;reboot'
: > "$READS"
if smartdns_load_lifecycle_nvram_state; then
    fail "restart accepted a malformed enable selector"
else
    status=$?
fi
[ "$status" -eq 22 ] || fail "malformed selector returned $status instead of 22"
[ "$(cat "$READS")" = "sdns_enable" ] ||
    fail "restart read later settings after rejecting its enable selector"
echo "invalid enable selector still fails closed: PASS"

# Valid restart values are copied from the same checked snapshot in order.
ENABLE_VALUE=1
: > "$READS"
smartdns_load_lifecycle_nvram_state ||
    fail "valid restart NVRAM snapshot was rejected"
[ "$sdns_enable:$sdns_port:$sdns_redirect:$sdnse_enable:$sdnse_port" = "1:5353:1:0:5335" ] ||
    fail "valid lifecycle NVRAM snapshot was not copied"
[ "$(cat "$READS")" = 'sdns_enable
sdns_port
sdns_redirect
sdnse_enable
sdnse_port' ] ||
    fail "valid lifecycle NVRAM reads were reordered or omitted"
echo "SmartDNS lifecycle NVRAM stop isolation regression: PASS"
