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
    smartdns_named_process_state "$PIDOF_EXPECTED_NAME"; actual=$?
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
PIDOF_OUTPUT="$(printf '123\t456')"
expect_state 2 "unexpected PID delimiter is rejected"

# A confirmed-absent lookup must not depend on a prior PID identity probe.
# The test runs with set -u, so the production default must be nounset-safe.
unset SMARTDNS_PROC_ROOT
PIDOF_STATUS=1; PIDOF_OUTPUT=
if smartdns_process_state; then proc_root_state=0; else proc_root_state=$?; fi
[ "$proc_root_state" -eq 1 ] || fail "unset procfs override returned state $proc_root_state"
pass "confirmed-absent process lookup uses default /proc with an unset override"

SMARTDNS_PROC_ROOT="$tmpdir/proc"
smartdns_Bin="$tmpdir/smartdns"
smartdns_Conf="$tmpdir/smartdns.conf"
SMARTDNS_IMPOSTOR="$tmpdir/not-smartdns"
mkdir -p "$SMARTDNS_PROC_ROOT/1"
: > "$SMARTDNS_PROC_ROOT/1/stat"
: > "$smartdns_Bin"
: > "$smartdns_Conf"
: > "$SMARTDNS_IMPOSTOR"
PIDOF_EXPECTED_NAME=smartdns
create_smartdns_proc() {
    proc_pid=$1
    proc_exe=$2
    proc_config=$3
    proc_optional=$4
    proc_start=$5
    proc_dir="$SMARTDNS_PROC_ROOT/$proc_pid"
    rm -rf "$proc_dir"
    mkdir -p "$proc_dir"
    ln -s "$proc_exe" "$proc_dir/exe"
    printf '%s (smartdns) S 1' "$proc_pid" > "$proc_dir/stat"
    stat_field=5
    while [ "$stat_field" -le 21 ]; do
        printf ' 0' >> "$proc_dir/stat"
        stat_field=$((stat_field + 1))
    done
    printf ' %s\n' "$proc_start" >> "$proc_dir/stat"
    printf '%s\000%s\000%s\000%s\000%s\000' \
        "$proc_exe" "-f" "-c" "$proc_config" "$proc_optional" > "$proc_dir/cmdline"
}
expect_smartdns_state() {
    expected=$1
    label=$2
    if smartdns_process_state; then actual=0; else actual=$?; fi
    [ "$actual" -eq "$expected" ] || fail "$label state=$actual expected=$expected"
    pass "$label"
}

create_smartdns_proc 2345 "$smartdns_Bin" "$smartdns_Conf" "" 12345
create_smartdns_proc 2346 "$SMARTDNS_IMPOSTOR" "$smartdns_Conf" "" 12346
PIDOF_STATUS=0; PIDOF_OUTPUT="2345 2346"
expect_smartdns_state 0 "exact executable and argv exclude same-name impostors"
[ "$smartdns_process_ids" = "2345:12345" ] || fail "inventory included a same-name impostor"
PIDOF_OUTPUT=2346
expect_smartdns_state 1 "wrong executable is confirmed non-SmartDNS"
create_smartdns_proc 2347 "$smartdns_Bin" "$tmpdir/other.conf" "" 12347
PIDOF_OUTPUT=2347
expect_smartdns_state 1 "wrong config argv is confirmed non-SmartDNS"
create_smartdns_proc 2348 "$smartdns_Bin" "$smartdns_Conf" "" 12348
rm -f "$SMARTDNS_PROC_ROOT/2348/stat"
PIDOF_OUTPUT=2348
expect_smartdns_state 2 "incomplete proc metadata is unknown"
PIDOF_STATUS=2; PIDOF_OUTPUT=
expect_smartdns_state 2 "pidof failure cannot prove SmartDNS absence"
pass "unknown daemon state fails closed"

KILL_LOG="$tmpdir/signals.log"
: > "$KILL_LOG"
KILL_MODE=term-exit
kill() {
    printf '%s %s\n' "$1" "$2" >> "$KILL_LOG"
    case "$KILL_MODE:$1" in
        term-exit:-TERM)
            rm -rf "$SMARTDNS_PROC_ROOT/$2"
            PIDOF_STATUS=1; PIDOF_OUTPUT=
            ;;
        force:-KILL)
            rm -rf "$SMARTDNS_PROC_ROOT/$2"
            PIDOF_STATUS=1; PIDOF_OUTPUT=
            ;;
        unknown:-TERM)
            PIDOF_STATUS=2
            ;;
        still-running:-TERM|still-running:-KILL)
            : # Simulate a daemon which ignores both bounded stop signals.
            ;;
    esac
    return 0
}
create_smartdns_proc 2345 "$smartdns_Bin" "$smartdns_Conf" "" 22345
PIDOF_STATUS=0; PIDOF_OUTPUT=2345
smartdns_stop_process || fail "verified graceful stop failed"
grep -Fqx -- '-TERM 2345' "$KILL_LOG" || fail "stop did not target the validated PID"
[ "$(wc -l < "$KILL_LOG")" -eq 1 ] || fail "graceful stop unexpectedly escalated"
[ "$KILLALL_CALLS" -eq 0 ] || fail "graceful stop used name-wide killall"
pass "graceful signal is verified by fresh absent snapshot"

: > "$KILL_LOG"
create_smartdns_proc 2345 "$smartdns_Bin" "$smartdns_Conf" "" 32345
PIDOF_STATUS=0; PIDOF_OUTPUT=2345
KILL_MODE=force
smartdns_stop_process || fail "SIGKILL did not stop the same verified daemon"
grep -Fqx -- '-TERM 2345' "$KILL_LOG" || fail "force stop skipped TERM"
grep -Fqx -- '-KILL 2345' "$KILL_LOG" || fail "force stop did not target the validated PID"
[ "$(wc -l < "$KILL_LOG")" -eq 2 ] || fail "force stop used an unexpected signal sequence"
expected_force_signals=$(printf '%s\n' '-TERM 2345' '-KILL 2345')
[ "$(cat "$KILL_LOG")" = "$expected_force_signals" ] || fail "force stop signal order was not TERM then KILL"
[ "$KILLALL_CALLS" -eq 0 ] || fail "force stop used name-wide killall"
pass "force escalation occurs only after graceful timeout"

: > "$KILL_LOG"
create_smartdns_proc 2345 "$smartdns_Bin" "$smartdns_Conf" "" 37345
PIDOF_STATUS=0; PIDOF_OUTPUT=2345
KILL_MODE=still-running
if smartdns_stop_process; then
    fail "daemon still running after force escalation was accepted as stopped"
else
    stop_status=$?
fi
[ "$stop_status" -eq 1 ] || fail "daemon still running after force escalation returned $stop_status"
expected_stuck_signals=$(printf '%s\n' '-TERM 2345' '-KILL 2345')
[ "$(cat "$KILL_LOG")" = "$expected_stuck_signals" ] || fail "stuck daemon did not receive exactly TERM then KILL"
[ "$KILLALL_CALLS" -eq 0 ] || fail "stuck daemon stop used name-wide killall"
pass "daemon still running after force escalation is reported"

: > "$KILL_LOG"
create_smartdns_proc 2345 "$smartdns_Bin" "$smartdns_Conf" "" 42345
PIDOF_STATUS=0; PIDOF_OUTPUT=2345
KILL_MODE=unknown
if smartdns_stop_process; then fail "unknown post-TERM process inventory was accepted"; else stop_status=$?; fi
[ "$stop_status" -eq 2 ] || fail "unknown post-TERM state returned $stop_status"
[ "$(wc -l < "$KILL_LOG")" -eq 1 ] || fail "unknown inventory must prevent SIGKILL escalation"
[ "$KILLALL_CALLS" -eq 0 ] || fail "PID-scoped stop never calls name-wide killall"
pass "unknown post-signal state prevents force escalation"

# A PID reused for another executable between inventory and signal must not
# let the replacement process receive a signal.
: > "$KILL_LOG"
create_smartdns_proc 2345 "$smartdns_Bin" "$smartdns_Conf" "" 52345
snapshot_start=$(smartdns_process_identity 2345) || fail "could not snapshot a valid SmartDNS identity"
rm -f "$SMARTDNS_PROC_ROOT/2345/exe"
ln -s "$SMARTDNS_IMPOSTOR" "$SMARTDNS_PROC_ROOT/2345/exe"
smartdns_signal_process_ids TERM "2345:$snapshot_start" || fail "replacement process identity was not safely skipped"
[ ! -s "$KILL_LOG" ] || fail "replacement process received a signal"
pass "PID-scoped stop revalidates executable and start_time before signalling"

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
cat > "$tmpdir/adblock/mixed-native.conf" <<'RULES'
# Valid address rule mixed with unrelated daemon configuration.
address=/ads.example/0.0.0.0
server 1.1.1.1
bind :53
conf-file /etc/passwd
RULES
if smartdns_prepare_adblock_filter "$tmpdir/adblock/mixed-native.conf" "$tmpdir/adblock/preserved.conf"; then
    fail "native SmartDNS filter accepted unrelated daemon directives"
fi
grep -Fxq 'preserve-on-error' "$tmpdir/adblock/preserved.conf" || fail "rejected native filter replaced the previous output"
pass "native SmartDNS filter rejects unrelated directives and preserves the previous output"

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
