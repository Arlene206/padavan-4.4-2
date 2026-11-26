#!/bin/sh
set -eu

HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
T=$(mktemp -d "${TMPDIR:-/tmp}/smartdns-runtime-nvram.XXXXXX")
trap 'rm -rf "$T"' 0
trap 'exit 1' HUP INT TERM
fail() { echo "SmartDNS runtime NVRAM regression: FAIL: $*" >&2; exit 1; }

SOURCE="$HERE/../smartdns.sh"
FUNCTIONS="$T/functions.sh"
READS="$T/reads"
: > "$FUNCTIONS"
sed -n '/^smartdns_nvram_read_checked()/,/^}/p' "$SOURCE" >> "$FUNCTIONS"
sed -n '/^smartdns_validate_enable_selector()/,/^}/p' "$SOURCE" >> "$FUNCTIONS"
sed -n '/^smartdns_load_lifecycle_nvram_state()/,/^}/p' "$SOURCE" >> "$FUNCTIONS"
sed -n '/^smartdns_load_runtime_nvram_state()/,/^}/p' "$SOURCE" >> "$FUNCTIONS"
sed -n '/^smartdns_disable_ss_conflict()/,/^}/p' "$SOURCE" >> "$FUNCTIONS"
sed -n '/^smartdns_disable_after_start_failure()/,/^}/p' "$SOURCE" >> "$FUNCTIONS"
[ -s "$FUNCTIONS" ] || fail "runtime NVRAM functions were not found"
. "$FUNCTIONS"

LOG="$T/log"
NVRAM_WRITES="$T/writes"
STOP_CALLS="$T/stop-calls"
: > "$LOG"
: > "$NVRAM_WRITES"
: > "$STOP_CALLS"
logger() { printf '%s\n' "$*" >> "$LOG"; }
STOP_STATUS=0
Stop_smartdns()
{
    printf 'stop\n' >> "$STOP_CALLS"
    return "$STOP_STATUS"
}
FAIL_KEY=
FAIL_SET=0
FAIL_STATUS=73
nvram()
{
    case "${1:-}" in
        set)
            [ "$#" -eq 2 ] || return 2
            printf '%s\n' "$2" >> "$NVRAM_WRITES"
            [ "$FAIL_SET" -eq 0 ] || return "$FAIL_STATUS"
            return 0
            ;;
        get_checked)
            [ "$#" -eq 2 ] || return 2
            ;;
        *) return 2 ;;
    esac
    key=$2
    printf '%s\n' "$key" >> "$READS"
    [ "$key" != "$FAIL_KEY" ] || return "$FAIL_STATUS"
    case "$key" in
        sdns_enable) value=1 ;;
        sdns_port) value=5353 ;;
        sdns_redirect) value=1 ;;
        sdnse_enable) value=0 ;;
        sdnse_port) value=5335 ;;
        *) value=1 ;;
    esac
    printf '%s\n' "$value"
}

ifconfig()
{
    [ "$1" = br0 ] || return 2
    cat <<'IFCONFIG'
br0 Link encap:Ethernet
          inet addr:192.0.2.1  Bcast:192.0.2.255  Mask:255.255.255.0
          inet6 addr: 2001:db8::1/64 Scope:Global
IFCONFIG
}
md5sum()
{
    printf '%s  %s\n' '0123456789abcdef' "$1"
}

dnsmasq_Conf="$T/dnsmasq.conf"
: > "$dnsmasq_Conf"

KEYS='sdns_enable sdns_port sdns_redirect sdnse_enable sdnse_port
sdns_name sdns_tcp_server sdns_ipv6_server sdns_cache sdns_cache_persist
sdns_tcp_idle_time sdns_rr_ttl sdns_rr_ttl_min sdns_rr_ttl_max
sdns_rr_ttl_reply_max sdns_max_reply_ip_num sdns_speed sdns_speed_mode
sdns_address sdns_ipset sdns_ipset_timeout sdns_as sdns_ip_change
sdns_ip_change_time sdns_dualstack_ip_allow_force_aaaa sdns_force_aaaa_soa
sdns_force_qtype_soa sdns_prefetch_domain sdns_exp sdns_exp_ttl
sdns_exp_ttl_max sdns_exp_prefetch_time sdnse_tcp_server sdnse_speed sdnse_name
sdnse_address sdnse_ns sdns_ns sdnse_ipset sdnse_as sdnse_ipv6_server
sdnse_ipc sdnse_cache sdns_adblock sdns_white sdns_black sdns_coredump'

# Every checked snapshot key must fail closed and preserve its original errno.
for FAIL_KEY in $KEYS; do
    : > "$READS"
    action=start
    if smartdns_load_runtime_nvram_state; then
        fail "snapshot accepted NVRAM failure for $FAIL_KEY"
    else
        status=$?
    fi
    [ "$status" -eq "$FAIL_STATUS" ] ||
        fail "$FAIL_KEY failure status changed to $status"

    expected=
    for key in $KEYS; do
        if [ -n "$expected" ]; then
            expected="$expected
$key"
        else
            expected=$key
        fi
        [ "$key" = "$FAIL_KEY" ] && break
    done
    actual=$(cat "$READS")
    [ "$actual" = "$expected" ] ||
        fail "$FAIL_KEY failure did not stop at the failing key"
done
echo "every runtime NVRAM snapshot failure aborts before later reads: PASS"

# A valid snapshot captures config plus the cleanup context used by stop/start.
FAIL_KEY=
: > "$READS"
action=start
smartdns_load_runtime_nvram_state || fail "valid runtime snapshot was rejected"
[ "$sdnse_tcp_server" = 1 ] ||
    fail "the UI-backed sdnse_tcp_server value was not copied from NVRAM"
[ "$sdns_name" = 1 ] && [ "$sdns_port" = 5353 ] &&
    [ "$sdns_redirect" = 1 ] && [ "$sdnse_port" = 5335 ] ||
    fail "valid runtime NVRAM values were not copied"
[ "$IPS4" = 192.0.2.1 ] && [ "$IPS6" = 2001:db8::1/64 ] ||
    fail "LAN addresses were not captured for firewall cleanup"
[ "$dnsmasq_md5" = 0123456789abcdef ] ||
    fail "DNSMasq baseline digest was not captured"
[ "$(wc -l < "$READS")" -eq 47 ] ||
    fail "runtime snapshot did not read all 47 required NVRAM keys"
echo "valid runtime snapshot captures settings and LAN cleanup context: PASS"

# Explicit stop restores its persisted state without consulting live NVRAM.
: > "$READS"
FAIL_KEY=sdns_enable
action=stop
smartdns_load_runtime_nvram_state || fail "explicit stop depended on live NVRAM"
[ ! -s "$READS" ] || fail "explicit stop read transient configuration NVRAM"
[ "$sdns_enable" = 0 ] && [ -z "$sdns_port" ] && [ -z "$sdns_redirect" ] ||
    fail "explicit stop did not select saved-state cleanup"
[ "$IPS4" = 192.0.2.1 ] && [ "$IPS6" = 2001:db8::1/64 ] ||
    fail "explicit stop skipped non-NVRAM cleanup context"
echo "explicit stop remains independent of live configuration NVRAM: PASS"

# SS DNS-conflict shutdown must preserve a failed NVRAM write status. The
# lifecycle caller must not mistake an unsuccessful disable for a clean exit.
FAIL_SET=0
: > "$NVRAM_WRITES"
smartdns_disable_ss_conflict || fail "SS conflict disable rejected a successful NVRAM write"
[ "$(cat "$NVRAM_WRITES")" = "sdns_enable=0" ] ||
    fail "SS conflict disable did not persist sdns_enable=0 exactly once"

FAIL_SET=1
: > "$NVRAM_WRITES"
if smartdns_disable_ss_conflict; then
    fail "SS conflict disable hid a failed NVRAM write"
else
    status=$?
fi
[ "$status" -eq "$FAIL_STATUS" ] ||
    fail "SS conflict disable changed NVRAM failure status to $status"
[ "$(cat "$NVRAM_WRITES")" = "sdns_enable=0" ] ||
    fail "SS conflict disable retried or changed the failing NVRAM write"
grep -Fq "Failed to persist sdns_enable=0 after SS DNS conflict" "$LOG" ||
    fail "SS conflict NVRAM failure was not logged"
echo "SS conflict disable propagates NVRAM write failure: PASS"

# Failed daemon startup uses the same fail-closed persistence rule, but always
# completes resolver/daemon cleanup. Stop errors outrank the NVRAM write error.
FAIL_SET=0
STOP_STATUS=0
: > "$NVRAM_WRITES"
: > "$STOP_CALLS"
sdns_enable=1
smartdns_disable_after_start_failure ||
    fail "startup rollback rejected successful persistence and cleanup"
[ "$sdns_enable" = 0 ] || fail "startup rollback did not switch its local selector off"
[ "$(cat "$NVRAM_WRITES")" = "sdns_enable=0" ] ||
    fail "startup rollback did not persist exactly one disable write"
[ "$(cat "$STOP_CALLS")" = stop ] ||
    fail "startup rollback did not delegate cleanup exactly once"

FAIL_SET=1
STOP_STATUS=0
: > "$NVRAM_WRITES"
: > "$STOP_CALLS"
if smartdns_disable_after_start_failure; then
    fail "startup rollback hid a failed NVRAM disable"
else
    status=$?
fi
[ "$status" -eq "$FAIL_STATUS" ] ||
    fail "startup rollback returned $status instead of NVRAM failure $FAIL_STATUS"
[ "$(cat "$STOP_CALLS")" = stop ] ||
    fail "startup rollback skipped cleanup after NVRAM failure"
grep -Fq "Failed to persist sdns_enable=0 after startup failure" "$LOG" ||
    fail "startup rollback did not log NVRAM persistence failure"

FAIL_SET=0
STOP_STATUS=51
: > "$NVRAM_WRITES"
: > "$STOP_CALLS"
if smartdns_disable_after_start_failure; then
    fail "startup rollback hid a failed Stop_smartdns operation"
else
    status=$?
fi
[ "$status" -eq 51 ] || fail "startup rollback did not preserve cleanup failure status"
[ "$(cat "$STOP_CALLS")" = stop ] ||
    fail "startup rollback did not attempt cleanup once on stop failure"

FAIL_SET=1
STOP_STATUS=51
: > "$NVRAM_WRITES"
: > "$STOP_CALLS"
if smartdns_disable_after_start_failure; then
    fail "startup rollback hid simultaneous persistence and cleanup failures"
else
    status=$?
fi
[ "$status" -eq 51 ] || fail "cleanup error did not take precedence over NVRAM error"
[ "$(cat "$STOP_CALLS")" = stop ] ||
    fail "startup rollback did not attempt cleanup after both failures"
echo "failed-start rollback persists disable, cleans once, and propagates errors: PASS"
