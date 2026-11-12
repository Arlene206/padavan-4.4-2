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
