#!/bin/sh
set -u
. "$(dirname "$0")/../smartdns_process_lifecycle.sh"
fail() { echo "FAIL: $*" >&2; exit 1; }
pass() { echo "PASS: $*"; }
tmpdir=$(mktemp -d "${TMPDIR:-/tmp}/smartdns-process-lifecycle.XXXXXX") || fail "temporary directory creation failed"
trap 'rm -rf "$tmpdir"' 0
trap 'exit 1' HUP INT TERM
PIDOF_STATUS=1
PIDOF_OUTPUT=
KILLALL_CALLS=0
KILLALL_GRACEFUL_BEHAVIOR=clear
KILLALL_FORCE_BEHAVIOR=clear
PIDOF_EXPECTED_NAME=smartdns
pidof() {
    [ "$1" = "$PIDOF_EXPECTED_NAME" ] || return 2
    printf '%s' "$PIDOF_OUTPUT"
    return "$PIDOF_STATUS"
}
killall() {
    KILLALL_CALLS=$((KILLALL_CALLS + 1))
    case " $* " in
        *" -9 "*)
            [ "$KILLALL_FORCE_BEHAVIOR" = clear ] && { PIDOF_STATUS=1; PIDOF_OUTPUT=; }
            [ "$KILLALL_FORCE_BEHAVIOR" = unknown ] && { PIDOF_STATUS=2; PIDOF_OUTPUT=; }
            ;;
        *)
            [ "$KILLALL_GRACEFUL_BEHAVIOR" = clear ] && { PIDOF_STATUS=1; PIDOF_OUTPUT=; }
            [ "$KILLALL_GRACEFUL_BEHAVIOR" = unknown ] && { PIDOF_STATUS=2; PIDOF_OUTPUT=; }
            ;;
    esac
    return 1
}
sleep() { :; }
expect_state() {
    expected=$1; label=$2
    smartdns_process_state; actual=$?
    [ "$actual" -eq "$expected" ] || fail "$label state=$actual expected=$expected"
    pass "$label"
}
expect_stop() {
    expected=$1; label=$2
    smartdns_stop_process; actual=$?
    [ "$actual" -eq "$expected" ] || fail "$label status=$actual expected=$expected"
    pass "$label"
}
PIDOF_STATUS=1; PIDOF_OUTPUT=
expect_state 1 "status 1 plus empty output means absent"
PIDOF_STATUS=0; PIDOF_OUTPUT="123 456"
expect_state 0 "valid multiple PIDs mean running"
PIDOF_STATUS=0; PIDOF_OUTPUT=
expect_state 2 "status 0 plus empty output is unknown"
PIDOF_STATUS=1; PIDOF_OUTPUT=123
expect_state 2 "status 1 plus output is inconsistent"
PIDOF_STATUS=2; PIDOF_OUTPUT=
expect_state 2 "pidof error is unknown"
PIDOF_STATUS=0; PIDOF_OUTPUT="123 bad"
expect_state 2 "malformed PID list is unknown"
PIDOF_STATUS=0; PIDOF_OUTPUT=0
expect_state 2 "PID zero is rejected"
PIDOF_STATUS=0; PIDOF_OUTPUT=0123
expect_state 2 "noncanonical leading-zero PID is rejected"
PIDOF_STATUS=0; PIDOF_OUTPUT="123\t456"
expect_state 2 "unexpected PID delimiter is rejected"
KILLALL_CALLS=0; PIDOF_STATUS=1; PIDOF_OUTPUT=
expect_stop 0 "absent daemon is a successful no-op"
[ "$KILLALL_CALLS" -eq 0 ] || fail "absent daemon was signalled"
KILLALL_CALLS=0; PIDOF_STATUS=2; PIDOF_OUTPUT=
expect_stop 2 "unknown daemon state fails closed"
[ "$KILLALL_CALLS" -eq 0 ] || fail "unknown daemon state was signalled"
KILLALL_CALLS=0; PIDOF_STATUS=0; PIDOF_OUTPUT=123
KILLALL_GRACEFUL_BEHAVIOR=clear
expect_stop 0 "graceful signal is verified by fresh absent snapshot"
[ "$KILLALL_CALLS" -eq 1 ] || fail "graceful stop call count wrong"
KILLALL_CALLS=0; PIDOF_STATUS=0; PIDOF_OUTPUT=123
KILLALL_GRACEFUL_BEHAVIOR=unknown; KILLALL_FORCE_BEHAVIOR=clear
expect_stop 2 "unknown post-signal state prevents force escalation"
[ "$KILLALL_CALLS" -eq 1 ] || fail "unknown state should prevent second signal"
KILLALL_CALLS=0; PIDOF_STATUS=0; PIDOF_OUTPUT=123
KILLALL_GRACEFUL_BEHAVIOR=keep; KILLALL_FORCE_BEHAVIOR=clear
expect_stop 0 "force escalation occurs only after graceful timeout"
[ "$KILLALL_CALLS" -eq 2 ] || fail "expected graceful plus forced stop"
KILLALL_CALLS=0; PIDOF_STATUS=0; PIDOF_OUTPUT=123
KILLALL_GRACEFUL_BEHAVIOR=keep; KILLALL_FORCE_BEHAVIOR=keep
expect_stop 1 "daemon still running after force escalation is reported"
[ "$KILLALL_CALLS" -eq 2 ] || fail "expected exactly two signal attempts"

# Verify the downloaded Adblock Plus list is converted to SmartDNS syntax.
SMARTDNS_TEST_DIR=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
sed -n '/^smartdns_prepare_adblock_filter()/,/^}/p' "$SMARTDNS_TEST_DIR/../smartdns.sh" > "$tmpdir/adblock-function.sh"
[ -s "$tmpdir/adblock-function.sh" ] || fail "adblock filter helper missing"
. "$tmpdir/adblock-function.sh"
mkdir -p "$tmpdir/adblock"

cat > "$tmpdir/adblock/adblock-plus.txt" <<'RULES'
! Example filter list
||ads.example^
||tracker.example^
@@||allowed.example^
||wildcard.*.example^
||-invalid.example^
||bad..example^
||ads.example^$important
RULES
cat > "$tmpdir/adblock/adblock-plus.expected" <<'RULES'
address=/ads.example/0.0.0.0
address=/tracker.example/0.0.0.0
RULES
smartdns_prepare_adblock_filter "$tmpdir/adblock/adblock-plus.txt" "$tmpdir/adblock/converted.conf" || fail "Adblock Plus conversion failed"
cmp "$tmpdir/adblock/adblock-plus.expected" "$tmpdir/adblock/converted.conf" || fail "Adblock Plus rules were not converted precisely"
pass "Adblock Plus rules convert to SmartDNS address directives"

cat > "$tmpdir/adblock/native-smartdns.conf" <<'RULES'
# Native SmartDNS address rules
address=/ads.example/0.0.0.0
address=/tracker.example/0.0.0.0
RULES
smartdns_prepare_adblock_filter "$tmpdir/adblock/native-smartdns.conf" "$tmpdir/adblock/native-output.conf" || fail "native SmartDNS filter copy failed"
cmp "$tmpdir/adblock/native-smartdns.conf" "$tmpdir/adblock/native-output.conf" || fail "native SmartDNS address directives were altered"
pass "native SmartDNS address rules remain unchanged"

printf 'preserve-on-error\n' > "$tmpdir/adblock/preserved.conf"
printf 'no supported filter rules\n' > "$tmpdir/adblock/invalid.txt"
if smartdns_prepare_adblock_filter "$tmpdir/adblock/invalid.txt" "$tmpdir/adblock/preserved.conf"; then
    fail "unsupported adblock input was accepted"
fi
grep -Fxq 'preserve-on-error' "$tmpdir/adblock/preserved.conf" || fail "failed conversion replaced the previous output"
if smartdns_prepare_adblock_filter "$tmpdir/adblock/missing.txt" "$tmpdir/adblock/preserved.conf"; then
    fail "missing adblock input was accepted"
fi
grep -Fxq 'preserve-on-error' "$tmpdir/adblock/preserved.conf" || fail "missing input replaced the previous output"
pass "failed adblock conversion preserves the previous output"

# Adbyby shares the strict tri-state parser: a failed lookup must not
# masquerade as "not running" and silently skip a DNS ownership update.
SMARTDNS_TEST_DIR=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
sed -n '/^Change_adbyby () {$/,/^}/p' "$SMARTDNS_TEST_DIR/../smartdns.sh" > "$tmpdir/change-adbyby.sh"
[ -s "$tmpdir/change-adbyby.sh" ] || fail "Change_adbyby helper missing"
. "$tmpdir/change-adbyby.sh"
smartdns_nvram_read_checked() {
    ADDBYBY_NVRAM_READS=$((ADDBYBY_NVRAM_READS + 1))
    case "$1" in
        adbyby_enable|adbyby_add) smartdns_nvram_value=1; return 0 ;;
        *) return 2 ;;
    esac
}
nvram() {
    ADDBYBY_NVRAM_WRITES=$((ADDBYBY_NVRAM_WRITES + 1))
    return 0
}
logger() { :; }
PIDOF_EXPECTED_NAME=adbyby
ADDBYBY_NVRAM_READS=0
ADDBYBY_NVRAM_WRITES=0
sdns_enable=0
hosts_type=SmartDNS
action=start
PIDOF_STATUS=1; PIDOF_OUTPUT=
Change_adbyby || fail "confirmed-absent Adbyby was not a safe no-op"
[ "$ADDBYBY_NVRAM_READS" -eq 0 ] || fail "absent Adbyby triggered NVRAM reads"
for mode in unknown empty_success inconsistent malformed; do
    case "$mode" in
        unknown) PIDOF_STATUS=2; PIDOF_OUTPUT= ;;
        empty_success) PIDOF_STATUS=0; PIDOF_OUTPUT= ;;
        inconsistent) PIDOF_STATUS=1; PIDOF_OUTPUT=123 ;;
        malformed) PIDOF_STATUS=0; PIDOF_OUTPUT="123 bad" ;;
    esac
    if Change_adbyby; then
        fail "Adbyby process state $mode was accepted"
    else
        adbyby_status=$?
    fi
    [ "$adbyby_status" -eq 2 ] || fail "Adbyby process state $mode returned $adbyby_status"
    [ "$ADDBYBY_NVRAM_READS" -eq 0 ] || fail "unknown Adbyby state $mode changed DNS policy"
    [ "$ADDBYBY_NVRAM_WRITES" -eq 0 ] || fail "unknown Adbyby state $mode wrote NVRAM"
done
PIDOF_STATUS=0; PIDOF_OUTPUT=123
Change_adbyby || fail "running Adbyby process did not permit the configured transition"
[ "$ADDBYBY_NVRAM_READS" -eq 2 ] || fail "running Adbyby did not read both owner settings"
[ "$ADDBYBY_NVRAM_WRITES" -eq 1 ] || fail "running Adbyby did not apply its single owner update"
[ "$hosts_type" = dnsmasq ] || fail "running Adbyby transition did not update the DNS owner"
pass "Adbyby process inventory errors preserve DNS ownership"

echo "SmartDNS process lifecycle regression: PASS"
