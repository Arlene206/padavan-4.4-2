#!/bin/sh
set -u
. "$(dirname "$0")/../smartdns_process_lifecycle.sh"
fail() { echo "FAIL: $*" >&2; exit 1; }
pass() { echo "PASS: $*"; }
PIDOF_STATUS=1
PIDOF_OUTPUT=
KILLALL_CALLS=0
KILLALL_GRACEFUL_BEHAVIOR=clear
KILLALL_FORCE_BEHAVIOR=clear
pidof() {
    [ "$1" = smartdns ] || return 2
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
echo "SmartDNS process lifecycle regression: PASS"
