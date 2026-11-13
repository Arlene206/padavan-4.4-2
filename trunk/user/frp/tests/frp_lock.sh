#!/bin/sh
# Host regression for FRP's stable lifecycle lock and bounded serialization.
set -eu
repo_root=$(CDPATH= cd -- "$(dirname "$0")/../../../.." && pwd)
test_root=$(mktemp -d /tmp/frp-lock-test.XXXXXX)
trap 'rm -rf "$test_root"' EXIT HUP INT TERM

TEST_FRPC_ENABLE=1
TEST_FRPS_ENABLE=1
TEST_HTTP_USERNAME=root
nvram()
{
	case "$2" in
		frpc_enable) printf '%s\\n' "$TEST_FRPC_ENABLE" ;;
		frps_enable) printf '%s\\n' "$TEST_FRPS_ENABLE" ;;
		http_username) printf '%s\\n' "$TEST_HTTP_USERNAME" ;;
		*) printf '\\n' ;;
	esac
}
logger() { :; }
set -- __test_source__
. "$repo_root/trunk/user/frp/frp.sh" __test_source__

LOCK_PATH="$test_root/.frp.lifecycle.lock"
frp_lifecycle_lock_path() { printf '%s\\n' "$LOCK_PATH"; }
fail() { echo "FRP lock regression: FAIL: $*" >&2; exit 1; }

command -v flock >/dev/null 2>&1 || fail "host flock command is unavailable"
frp_lifecycle_lock_prepare "$LOCK_PATH" || fail "safe private lock file was rejected"
[ "$(stat -c '%a' "$LOCK_PATH")" = 600 ] || fail "lock file mode is not 0600"
[ "$(stat -c '%h' "$LOCK_PATH")" = 1 ] || fail "lock file has more than one hard link"
first_inode=$(stat -c '%i' "$LOCK_PATH")
frp_lifecycle_lock_prepare "$LOCK_PATH" || fail "existing lock file was rejected"
[ "$(stat -c '%i' "$LOCK_PATH")" = "$first_inode" ] ||
	fail "lock preparation replaced the stable inode"

# Never follow a symlink, even when its referent is a regular file.
printf 'untouched\\n' > "$test_root/referent"
ln -s "$test_root/referent" "$test_root/symlink.lock"
LOCK_PATH="$test_root/symlink.lock"
if frp_lifecycle_lock_prepare "$LOCK_PATH"; then fail "symlink lock path was accepted"; fi
[ "$(cat "$test_root/referent")" = untouched ] || fail "symlink referent was modified"

# A hard-linked or world-readable lock could split ownership or disclose lock state.
printf 'lock\\n' > "$test_root/hardlink.lock"
chmod 600 "$test_root/hardlink.lock"
ln "$test_root/hardlink.lock" "$test_root/second-link"
LOCK_PATH="$test_root/hardlink.lock"
if frp_lifecycle_lock_prepare "$LOCK_PATH"; then fail "hard-linked lock path was accepted"; fi
printf 'lock\\n' > "$test_root/world-readable.lock"
chmod 644 "$test_root/world-readable.lock"
LOCK_PATH="$test_root/world-readable.lock"
if frp_lifecycle_lock_prepare "$LOCK_PATH"; then fail "world-readable lock path was accepted"; fi
mkfifo "$test_root/fifo.lock"
LOCK_PATH="$test_root/fifo.lock"
if frp_lifecycle_lock_prepare "$LOCK_PATH"; then fail "FIFO lock path was accepted"; fi
mkdir "$test_root/real-dir"
ln -s "$test_root/real-dir" "$test_root/link-dir"
LOCK_PATH="$test_root/link-dir/nested.lock"
if frp_lifecycle_lock_prepare "$LOCK_PATH"; then fail "symlink parent directory was accepted"; fi

# Lock actions propagate success and failure, and always close reserved FD 9.
LOCK_PATH="$test_root/.frp.lifecycle.lock"
ACTION_CALLS=0
lock_action() { ACTION_CALLS=$((ACTION_CALLS + 1)); return 0; }
frp_lifecycle_lock_execute "$LOCK_PATH" 0 lock_action ||
	fail "uncontended lock action failed"
[ "$ACTION_CALLS" -eq 1 ] || fail "successful action was not invoked exactly once"
if ( : <&9 ) 2>/dev/null || ( : >&9 ) 2>/dev/null; then
	fail "launcher left reserved FD 9 open"
fi
lock_fail_action() { return 7; }
if frp_lifecycle_lock_execute "$LOCK_PATH" 0 lock_fail_action; then
	fail "action failure was swallowed"
else
	action_status=$?
fi
[ "$action_status" -eq 7 ] || fail "action failure status was not propagated"

# A competing launcher owns the same inode. The waiting caller must time out
# without executing its action or leaving the reserved descriptor open.
READY="$test_root/holder-ready"
(
	exec 8>>"$LOCK_PATH"
	flock -n 8 || exit 1
	: > "$READY"
	command sleep 3
) &
holder_pid=$!
ready_attempt=0
while [ ! -f "$READY" ] && [ "$ready_attempt" -lt 100 ]; do
	command sleep 0.02
	ready_attempt=$((ready_attempt + 1))
done
[ -f "$READY" ] || fail "competing lock holder did not acquire the fixture lock"
ACTION_CALLS=0
LOCK_SLEEP_CALLS=0
sleep() { LOCK_SLEEP_CALLS=$((LOCK_SLEEP_CALLS + 1)); command sleep "$1"; }
if frp_lifecycle_lock_execute "$LOCK_PATH" 1 lock_action; then
	fail "contended lock action did not time out"
fi
unset -f sleep
[ "$ACTION_CALLS" -eq 0 ] || fail "action ran without acquiring the lock"
[ "$LOCK_SLEEP_CALLS" -eq 1 ] || fail "lock wait was not bounded to one retry interval"
if ( : <&9 ) 2>/dev/null || ( : >&9 ) 2>/dev/null; then
	fail "timeout left reserved FD 9 open"
fi
wait "$holder_pid" || fail "competing lock holder failed"

# The lock remains usable after the previous holder releases it.
ACTION_CALLS=0
frp_lifecycle_lock_execute "$LOCK_PATH" 0 lock_action ||
	fail "lock did not recover after contention"
[ "$ACTION_CALLS" -eq 1 ] || fail "recovered lock action did not run"

echo "FRP lifecycle lock protects launcher actions with a stable private inode: PASS"
echo "FRP lifecycle lock rejects unsafe paths and does not leak FD 9: PASS"
echo "FRP lifecycle lock bounds contention and propagates action status: PASS"
