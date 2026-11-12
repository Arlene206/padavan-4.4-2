#!/bin/sh
set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
SOURCE="$SCRIPT_DIR/../aliddns.sh"
TEST_DIR=$(mktemp -d "/tmp/aliddns-lifecycle.XXXXXX")
FUNCTIONS="$TEST_DIR/functions.sh"
ACTION_OUTPUT="$TEST_DIR/action.out"
LOCK_DIR="$TEST_DIR/lockdir"
LOCK_FILE="$LOCK_DIR/lifecycle.lock"
READY="$TEST_DIR/holder.ready"
HOLDER_PID=""

cleanup() {
	if [ -n "$HOLDER_PID" ]; then
		kill "$HOLDER_PID" 2>/dev/null || :
		wait "$HOLDER_PID" 2>/dev/null || :
	fi
	rm -rf "$TEST_DIR"
}
trap cleanup EXIT
trap 'exit 1' HUP INT TERM

: > "$FUNCTIONS"
extract_function() {
	awk -v signature="$1" '
		$0 == signature { capture = 1 }
		capture { print }
		capture && $0 == "}" { exit }
	' "$SOURCE" >> "$FUNCTIONS"
}
extract_function "aliddns_lifecycle_lock_path() {"
extract_function "aliddns_lifecycle_lock_prepare() {"
extract_function "aliddns_lifecycle_lock_execute() {"
extract_function "aliddns_lifecycle_control_start() {"
extract_function "aliddns_lifecycle_locked_action() {"
extract_function "aliddns_lifecycle_lock_run() {"
extract_function "aliddns_keeper_pid_matches() {"
extract_function "aliddns_find_keeper_pids() {"
extract_function "aliddns_stop_keeper() {"
extract_function "aliddns_close () {"
. "$FUNCTIONS"

logger() {
	:
}
mkdir -m 700 "$LOCK_DIR"
aliddns_lifecycle_lock_path() {
	printf '%s\n' "$LOCK_FILE"
}

# First creation must produce one private, root-owned, stable regular inode.
aliddns_lifecycle_lock_prepare "$LOCK_FILE"
[ -f "$LOCK_FILE" ] && [ ! -L "$LOCK_FILE" ] || {
	echo "lock file was not created as a regular file" >&2
	exit 1
}
[ "$(stat -c '%a' "$LOCK_FILE")" = 600 ] || {
	echo "lock file mode is not 0600" >&2
	exit 1
}
[ "$(stat -c '%h' "$LOCK_FILE")" = 1 ] || {
	echo "lock file link count is not one" >&2
	exit 1
}

# Symlinks, multiply-linked files and permissive modes fail closed.
SYMLINK_FILE="$LOCK_DIR/symlink.lock"
: > "$LOCK_DIR/target"
ln -s "$LOCK_DIR/target" "$SYMLINK_FILE"
LOCK_FILE="$SYMLINK_FILE"
if aliddns_lifecycle_lock_prepare "$LOCK_FILE"; then
	echo "symlink lock file was accepted" >&2
	exit 1
fi

HARDLINK_FILE="$LOCK_DIR/hardlink.lock"
: > "$HARDLINK_FILE"
chmod 600 "$HARDLINK_FILE"
ln "$HARDLINK_FILE" "$HARDLINK_FILE.link"
LOCK_FILE="$HARDLINK_FILE"
if aliddns_lifecycle_lock_prepare "$LOCK_FILE"; then
	echo "multiply-linked lock file was accepted" >&2
	exit 1
fi

MODE_FILE="$LOCK_DIR/mode.lock"
: > "$MODE_FILE"
chmod 644 "$MODE_FILE"
LOCK_FILE="$MODE_FILE"
if aliddns_lifecycle_lock_prepare "$LOCK_FILE"; then
	echo "permissive lock file was accepted" >&2
	exit 1
fi
echo "AliDDNS lifecycle lock private inode and unsafe-file rejection: PASS"

LOCK_FILE="$LOCK_DIR/lifecycle.lock"
aliddns_lifecycle_lock_prepare "$LOCK_FILE"
action_capture() {
	printf '%s\n' "$*" > "$ACTION_OUTPUT"
}
aliddns_lifecycle_lock_execute "$LOCK_FILE" 0 action_capture acquired
[ "$(cat "$ACTION_OUTPUT")" = acquired ] || {
	echo "locked action did not execute" >&2
	exit 1
}

# Contention from a separate process must fail immediately at zero wait.
(
	exec 8>>"$LOCK_FILE"
	flock -n 8
	: > "$READY"
	sleep 2
) &
HOLDER_PID=$!
tries=0
while [ ! -f "$READY" ] && [ "$tries" -lt 50 ]; do
	sleep 0.02
	tries=$((tries + 1))
done
[ -f "$READY" ] || {
	echo "lock holder did not become ready" >&2
	exit 1
}
if aliddns_lifecycle_lock_execute "$LOCK_FILE" 0 action_capture blocked; then
	echo "contended lifecycle lock unexpectedly succeeded" >&2
	exit 1
fi
[ "$(cat "$ACTION_OUTPUT")" = acquired ] || {
	echo "contended action ran despite lock ownership" >&2
	exit 1
}
wait "$HOLDER_PID"
HOLDER_PID=""
rm -f "$READY"

action_fail() {
	return 7
}
action_status=0
aliddns_lifecycle_lock_execute "$LOCK_FILE" 0 action_fail || action_status=$?
[ "$action_status" -eq 7 ] || {
	echo "locked action failure status was not propagated" >&2
	exit 1
}
if ( : <&9 ) 2>/dev/null; then
	echo "lifecycle lock descriptor leaked after action failure" >&2
	exit 1
fi
echo "AliDDNS lifecycle lock contention, bounded failure and descriptor release: PASS"

ACTION_LOG="$TEST_DIR/actions"
: > "$ACTION_LOG"
aliddns_close() {
	echo close >> "$ACTION_LOG"
}
nvram() {
	echo "nvram $*" >> "$ACTION_LOG"
}
aliddns_check() {
	echo check >> "$ACTION_LOG"
}

aliddns_lifecycle_lock_run restart
EXPECTED="$TEST_DIR/expected"
printf 'close\nnvram set aliddns_status=0\ncheck\n' > "$EXPECTED"
cmp "$EXPECTED" "$ACTION_LOG"
: > "$ACTION_LOG"
aliddns_lifecycle_lock_run stop
printf 'close\n' > "$EXPECTED"
cmp "$EXPECTED" "$ACTION_LOG"
: > "$ACTION_LOG"
aliddns_lifecycle_lock_run check
printf 'check\n' > "$EXPECTED"
cmp "$EXPECTED" "$ACTION_LOG"
echo "AliDDNS restart close/start transaction and locked action dispatch: PASS"
# Restore the source implementation of aliddns_close after the dispatcher
# ordering stubs above, then exercise exact /proc argv matching and stop gates.
CLOSE_FUNCTION="$TEST_DIR/aliddns-close.sh"
awk -v signature='aliddns_close () {' '
	$0 == signature { capture = 1 }
	capture { print }
	capture && $0 == "}" { exit }
' "$SOURCE" > "$CLOSE_FUNCTION"
. "$CLOSE_FUNCTION"

PROC_ROOT="$TEST_DIR/proc"
KILL_LOG="$TEST_DIR/kill.log"
SLEEP_LOG="$TEST_DIR/sleep.log"
mkdir -p "$PROC_ROOT/self" "$PROC_ROOT/101" "$PROC_ROOT/102" "$PROC_ROOT/103" "$PROC_ROOT/$$"
: > "$PROC_ROOT/self/cmdline"
scriptfilepath="$TEST_DIR/aliddns.sh"
ALIDDNS_PROC_ROOT="$PROC_ROOT"
printf 'sh\000%s\000keep\000' "$scriptfilepath" > "$PROC_ROOT/101/cmdline"
printf 'sh\000%s\000stop\000' "$scriptfilepath" > "$PROC_ROOT/102/cmdline"
printf 'sh\000%s-extra\000keep\000' "$scriptfilepath" > "$PROC_ROOT/103/cmdline"
printf 'sh\000%s\000keep\000' "$scriptfilepath" > "$PROC_ROOT/$$/cmdline"

aliddns_find_keeper_pids
[ "$ALIDDNS_KEEPER_PIDS" = "101" ] || {
	echo "keeper discovery did not require exact script path + keep argv or excluded self" >&2
	exit 1
}
echo "AliDDNS keeper process identity matches exact script/keep argv: PASS"

kill() {
	printf '%s %s\n' "$1" "$2" >> "$KILL_LOG"
	case "$MOCK_KILL_BEHAVIOR:$1" in
		graceful:-TERM|force:-KILL) rm -f "$PROC_ROOT/$2/cmdline" ;;
	esac
	return 0
}
sleep() {
	printf '%s\n' "$1" >> "$SLEEP_LOG"
}

# A cooperative keeper exits after TERM and is never sent KILL.
: > "$KILL_LOG"
: > "$SLEEP_LOG"
MOCK_KILL_BEHAVIOR=graceful
aliddns_close
[ ! -e "$PROC_ROOT/101/cmdline" ] || {
	echo "TERM did not remove the cooperative keeper fixture" >&2
	exit 1
}
[ "$(cat "$KILL_LOG")" = "-TERM 101" ] || {
	echo "cooperative stop sent an unexpected signal or targeted a decoy" >&2
	exit 1
}

# A keeper ignoring TERM is escalated only after the five-second grace period.
printf 'sh\000%s\000keep\000' "$scriptfilepath" > "$PROC_ROOT/101/cmdline"
: > "$KILL_LOG"
: > "$SLEEP_LOG"
MOCK_KILL_BEHAVIOR=force
aliddns_stop_keeper
[ "$(cat "$KILL_LOG")" = "$(printf '%s\n' '-TERM 101' '-KILL 101')" ] || {
	echo "TERM/KILL escalation did not remain scoped to the exact keeper" >&2
	exit 1
}
[ "$(wc -l < "$SLEEP_LOG" | tr -d ' ')" -eq 5 ] || {
	echo "TERM grace wait was not bounded to five one-second intervals" >&2
	exit 1
}
echo "AliDDNS keeper stop uses TERM grace then bounded KILL postcondition: PASS"

# An unkillable matching process must fail the close gate; restart must not
# mutate service state or dispatch a replacement keeper.
printf 'sh\000%s\000keep\000' "$scriptfilepath" > "$PROC_ROOT/101/cmdline"
: > "$KILL_LOG"
: > "$SLEEP_LOG"
: > "$ACTION_LOG"
MOCK_KILL_BEHAVIOR=stubborn
action_status=0
aliddns_lifecycle_control_start || action_status=$?
[ "$action_status" -ne 0 ] || {
	echo "restart accepted a keeper that survived TERM and KILL" >&2
	exit 1
}
[ ! -s "$ACTION_LOG" ] || {
	echo "restart mutated NVRAM or dispatched start after failed stop" >&2
	exit 1
}
[ "$(cat "$KILL_LOG")" = "$(printf '%s\n' '-TERM 101' '-KILL 101')" ] || {
	echo "failed stop signalled a process outside the exact keeper identity" >&2
	exit 1
}
[ "$(wc -l < "$SLEEP_LOG" | tr -d ' ')" -eq 7 ] || {
	echo "failed stop did not enforce its bounded TERM and KILL windows" >&2
	exit 1
}
echo "AliDDNS restart refuses to launch when keeper remains alive: PASS"
