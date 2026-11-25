#!/bin/sh
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
T=$(mktemp -d "${TMPDIR:-/tmp}/smartdns-firewall-status.XXXXXX")
trap 'rm -rf "$T"' 0
trap 'exit 1' HUP INT TERM
fail() { echo "SmartDNS firewall regression: FAIL: $*" >&2; exit 1; }
pass() { echo "SmartDNS firewall regression: PASS: $*"; }
SOURCE="$HERE/../smartdns.sh"
FUNCTIONS="$T/functions.sh"
LOG="$T/firewall.log"
: > "$FUNCTIONS"
sed -n '/^smartdns_firewall_add () {/,/^}/p' "$SOURCE" >> "$FUNCTIONS"
sed -n '/^smartdns_firewall_delete () {/,/^}/p' "$SOURCE" >> "$FUNCTIONS"
sed -n '/^Change_iptable () {/,/^}/p' "$SOURCE" >> "$FUNCTIONS"
[ -s "$FUNCTIONS" ] || fail "firewall policy functions were not found"
. "$FUNCTIONS"
logger() { :; }
CHECK_STATUS=0
ADD_STATUS=0
DELETE_STATUS=0
mock_firewall()
{
    mock_tool=$1
    shift
    printf '%s\t' "$mock_tool" >> "$LOG"
    printf '%s ' "$@" >> "$LOG"
    printf '\n' >> "$LOG"
    case " $* " in
        *" -C "*) return "$CHECK_STATUS" ;;
        *" -A "*) return "$ADD_STATUS" ;;
        *" -D "*) return "$DELETE_STATUS" ;;
        *) return 90 ;;
    esac
}
iptables() { mock_firewall iptables "$@"; }
ip6tables() { mock_firewall ip6tables "$@"; }
reset_mock()
{
    : > "$LOG"
    CHECK_STATUS=0; ADD_STATUS=0; DELETE_STATUS=0
    sdns_enable=0; IPS4=192.0.2.1; IPS6=2001:db8::1
    sdns_port=5353; sdns_ported=5353
    sdns_redirected=2; sdns_redirect=2
    sdns_tcp_server=1; sdns_ipv6_server=1; sdnse_enable=0
    smartdns_previous_redirected=0; action=stop
}
run_change()
{
    if Change_iptable "$1"; then RUN_STATUS=0; else RUN_STATUS=$?; fi
}

reset_mock
CHECK_STATUS=1
run_change stop
[ "$RUN_STATUS" -eq 0 ] || fail "absent rules should make stop idempotent (status=$RUN_STATUS)"
[ "$(wc -l < "$LOG" | tr -d ' ')" -eq 4 ] || fail "IPv4/IPv6 absent rules were not all checked"
if grep -q ' -D ' "$LOG"; then fail "absent redirect was needlessly deleted"; fi
pass "absent IPv4/IPv6 redirects are harmless during repeated stop"

reset_mock
DELETE_STATUS=19
run_change stop
[ "$RUN_STATUS" -eq 19 ] || fail "delete failure status was lost ($RUN_STATUS)"
[ "$(grep -c ' -C ' "$LOG")" -eq 4 ] || fail "cleanup stopped checking after first error"
[ "$(grep -c ' -D ' "$LOG")" -eq 4 ] || fail "cleanup did not continue best-effort deletes"
pass "delete failures propagate while remaining redirect rules are cleaned"

reset_mock
CHECK_STATUS=4
run_change stop
[ "$RUN_STATUS" -eq 4 ] || fail "check failure status was lost ($RUN_STATUS)"
[ "$(grep -c ' -C ' "$LOG")" -eq 4 ] || fail "check errors stopped remaining cleanup attempts"
if grep -q ' -D ' "$LOG"; then fail "delete ran after check could not verify rule ownership"; fi
pass "iptables check errors fail closed and preserve first status"

reset_mock
sdns_redirected=0; sdns_redirect=2; ADD_STATUS=23
run_change start
[ "$RUN_STATUS" -eq 23 ] || fail "add failure status was lost ($RUN_STATUS)"
[ "$(wc -l < "$LOG" | tr -d ' ')" -eq 1 ] || fail "new redirect setup continued after an add failure"
grep -q '^iptables[[:space:]].*-A ' "$LOG" || fail "IPv4 TCP redirect was not the first add"
pass "partial start redirect installation reports first add failure"

reset_mock
sdns_redirected=0; sdns_redirect=2
run_change start
[ "$RUN_STATUS" -eq 0 ] || fail "successful redirect setup returned $RUN_STATUS"
[ "$(grep -c ' -A ' "$LOG")" -eq 4 ] || fail "IPv4/IPv6 TCP/UDP redirect rules were not all installed"
pass "IPv4/IPv6 TCP/UDP redirects install through checked command wrapper"

reset_mock
sdns_redirect=1; sdns_redirected=0; ADD_STATUS=23
run_change reset
[ "$RUN_STATUS" -eq 23 ] || fail "reset redirect failure status was lost ($RUN_STATUS)"
[ "$(wc -l < "$LOG" | tr -d ' ')" -eq 1 ] || fail "reset continued after redirect add failure"
pass "reset-to-DNS port 53 reports firewall add failures"

reset_mock
if Change_iptable unknown; then fail "unknown firewall operation was accepted"; else RUN_STATUS=$?; fi
[ "$RUN_STATUS" -eq 2 ] || fail "unknown operation returned $RUN_STATUS instead of 2"
pass "unknown firewall operations fail closed"
