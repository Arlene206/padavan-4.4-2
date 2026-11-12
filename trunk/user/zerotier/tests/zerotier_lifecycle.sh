#!/bin/sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname "$0")/../../../.." && pwd)
SCRIPT="$ROOT/trunk/user/zerotier/zerotier.sh"
TEST_TMP=$(mktemp -d "${TMPDIR:-/tmp}/zerotier-lifecycle.XXXXXX")
TEST_STATE="$TEST_TMP/process.state"
TEST_KILL_LOG="$TEST_TMP/killall.log"
TEST_SLEEP_LOG="$TEST_TMP/sleep.log"
TEST_ACTION_LOG="$TEST_TMP/actions.log"
export TEST_STATE TEST_KILL_LOG TEST_SLEEP_LOG TEST_ACTION_LOG TEST_TMP
trap 'rm -rf "$TEST_TMP"' EXIT HUP INT TERM

fail() {
	echo "ZeroTier lifecycle regression: FAIL: $*" >&2
	exit 1
}

# Source the production functions without running a service action.
set -- __test__
. "$SCRIPT"

pidof() {
	[ "$1" = zerotier-one ] || return 1
	if [ "$(cat "$TEST_STATE")" = running ]; then
		echo 4321
		return 0
	fi
	return 1
}

killall() {
	printf '%s\n' "$*" >> "$TEST_KILL_LOG"
	case "$TEST_MODE:$*" in
		graceful:zerotier-one|-9-exits:-9\ zerotier-one)
			echo stopped > "$TEST_STATE"
			;;
		*)
			;;
	esac
	return 0
}

sleep() {
	printf '%s\n' "$*" >> "$TEST_SLEEP_LOG"
	return 0
}

logger() {
	return 0
}

# These hooks let the regression prove the stop-failure gate without touching
# host firewall state or the developer's persistent filesystem.
start_instance() {
	echo start >> "$TEST_ACTION_LOG"
	return 0
}

del_rules() {
	echo del_rules >> "$TEST_ACTION_LOG"
	return 0
}

zero_route() {
	echo "zero_route:$1" >> "$TEST_ACTION_LOG"
	return 0
}

rm() {
	printf 'rm:%s\n' "$*" >> "$TEST_ACTION_LOG"
	/bin/rm "$@"
}

kill() {
	echo "$*" >> "$TEST_TMP/numeric-kill.log"
	return 1
}

: > "$TEST_KILL_LOG"
: > "$TEST_SLEEP_LOG"
: > "$TEST_ACTION_LOG"
echo stopped > "$TEST_STATE"
TEST_MODE=none
export TEST_MODE
kill_z || fail "no-process stop returned failure"
[ ! -s "$TEST_KILL_LOG" ] || fail "stop signalled a process when pidof found none"
[ ! -s "$TEST_SLEEP_LOG" ] || fail "no-process stop waited unnecessarily"

: > "$TEST_KILL_LOG"
: > "$TEST_SLEEP_LOG"
echo running > "$TEST_STATE"
TEST_MODE=graceful
if ! kill_z; then
	fail "graceful TERM stop returned failure"
fi
[ "$(cat "$TEST_KILL_LOG")" = zerotier-one ] || fail "graceful stop did not send TERM only"
[ ! -s "$TEST_SLEEP_LOG" ] || fail "graceful exit was not observed immediately"
[ "$(cat "$TEST_STATE")" = stopped ] || fail "graceful stop postcondition missing"

: > "$TEST_KILL_LOG"
: > "$TEST_SLEEP_LOG"
echo running > "$TEST_STATE"
TEST_MODE=-9-exits
if ! kill_z; then
	fail "TERM-resistant daemon did not stop after name-based escalation"
fi
[ "$(cat "$TEST_KILL_LOG")" = "$(printf 'zerotier-one\n-9 zerotier-one')" ] || fail "force stop did not re-enumerate by process name"
[ "$(wc -l < "$TEST_SLEEP_LOG" | tr -d ' ')" = 5 ] || fail "TERM grace period was not bounded to five one-second polls"
[ "$(cat "$TEST_STATE")" = stopped ] || fail "force-stop postcondition missing"

: > "$TEST_KILL_LOG"
: > "$TEST_SLEEP_LOG"
echo running > "$TEST_STATE"
TEST_MODE=stuck
if kill_z; then
	fail "stop reported success while daemon remained after SIGKILL"
fi
[ "$(cat "$TEST_KILL_LOG")" = "$(printf 'zerotier-one\n-9 zerotier-one')" ] || fail "stuck daemon did not receive TERM then name-based KILL"
[ "$(wc -l < "$TEST_SLEEP_LOG" | tr -d ' ')" = 10 ] || fail "TERM/KILL waits were not bounded to five polls each"

: > "$TEST_KILL_LOG"
: > "$TEST_SLEEP_LOG"
: > "$TEST_ACTION_LOG"
echo running > "$TEST_STATE"
TEST_MODE=stuck
if start_zero; then
	fail "start proceeded when the prior daemon could not be stopped"
fi
[ ! -s "$TEST_ACTION_LOG" ] || fail "start reached start_instance after stop failure"

: > "$TEST_KILL_LOG"
: > "$TEST_SLEEP_LOG"
: > "$TEST_ACTION_LOG"
TEST_CONFIG="$TEST_TMP/config"
config_path="$TEST_CONFIG"
mkdir -p "$TEST_CONFIG"
echo identity > "$TEST_CONFIG/identity.secret"
echo running > "$TEST_STATE"
TEST_MODE=stuck
if stop_zero; then
	fail "stop reported success while the daemon remained alive"
fi
[ -f "$TEST_CONFIG/identity.secret" ] || fail "failed stop deleted the persistent ZeroTier identity"
[ ! -s "$TEST_ACTION_LOG" ] || fail "failed stop changed routes, firewall rules or persistent config"

: > "$TEST_KILL_LOG"
: > "$TEST_SLEEP_LOG"
: > "$TEST_ACTION_LOG"
echo running > "$TEST_STATE"
TEST_MODE=graceful
if ! stop_zero; then
	fail "stop failed after a confirmed graceful daemon exit"
fi
[ ! -e "$TEST_CONFIG" ] || fail "successful stop did not complete the existing config cleanup"
[ "$(cat "$TEST_ACTION_LOG")" = "$(printf 'del_rules\nzero_route:del\nrm:-rf %s' "$TEST_CONFIG")" ] || fail "successful cleanup did not run after daemon absence"

[ ! -s "$TEST_TMP/numeric-kill.log" ] || fail "a stale numeric PID was signalled"
echo "ZeroTier bounded stop/start and persistent-config preservation: PASS"
