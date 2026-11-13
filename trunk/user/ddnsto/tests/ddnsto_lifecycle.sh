#!/bin/sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname "$0")/../../../.." && pwd)
SCRIPT="$ROOT/trunk/user/ddnsto/ddnsto.sh"
TEST_TMP=$(mktemp -d "${TMPDIR:-/tmp}/ddnsto-lifecycle.XXXXXX")
TEST_BIN="$TEST_TMP/bin"
TEST_STATE="$TEST_TMP/state"
TEST_LOG="$TEST_TMP/killall.log"
TEST_SLEEP_LOG="$TEST_TMP/sleep.log"
TEST_NVRAM_LOG="$TEST_TMP/nvram.log"
TEST_OPS="$TEST_TMP/operations.log"
TEST_DAEMON_PID="$TEST_TMP/daemon.pid"
TEST_ENABLE_FILE="$TEST_TMP/enable"
TEST_HELPERS="$TEST_TMP/helpers.sh"
LOCK_FILE="$TEST_TMP/.ddnsto.lifecycle.lock"
export TEST_TMP TEST_STATE TEST_LOG TEST_SLEEP_LOG TEST_NVRAM_LOG TEST_OPS TEST_DAEMON_PID TEST_ENABLE_FILE

mkdir -p "$TEST_BIN"
trap 'if [ -s "$TEST_DAEMON_PID" ]; then kill -9 "$(cat "$TEST_DAEMON_PID")" 2>/dev/null || :; fi; rm -rf "$TEST_TMP"' EXIT HUP INT TERM

fail() {
	echo "DDNSTO lifecycle regression: FAIL: $*" >&2
	exit 1
}

# Import the exact production helpers without executing the installed command dispatcher.
sed -n \
	-e '/^start_zero() {$/,/^}/p' \
	-e '/^ddnsto_wait_stopped() {$/,/^}/p' \
	-e '/^kill_z() {$/,/^}/p' \
	-e '/^stop_zero() {$/,/^}/p' \
	-e '/^ddnsto_lifecycle_lock_path() {$/,/^}/p' \
	-e '/^ddnsto_lifecycle_lock_prepare() {$/,/^}/p' \
	-e '/^ddnsto_lifecycle_lock_execute() {$/,/^}/p' \
	-e '/^ddnsto_lifecycle_locked_action() {$/,/^}/p' \
	-e '/^ddnsto_lifecycle_lock_run() {$/,/^}/p' \
	"$SCRIPT" > "$TEST_HELPERS"
[ -s "$TEST_HELPERS" ] || fail "production helper extraction failed"
. "$TEST_HELPERS"

# All filesystem/command substitutions stay inside this temporary host fixture.
ddnsto_lifecycle_lock_path() { printf '%s\n' "$LOCK_FILE"; }
DDNSTO_BIN="$TEST_BIN/ddnsto"
DDNSTO_LOCK_WAIT_SECONDS=5
echo 1 > "$TEST_ENABLE_FILE"
export DDNSTO_BIN DDNSTO_LOCK_WAIT_SECONDS

cat > "$TEST_BIN/pidof" <<'MOCK'
#!/bin/sh
[ "$1" = ddnsto ] || exit 1
if [ "$(cat "$TEST_STATE")" = running ]; then
	echo "$(cat "$TEST_DAEMON_PID" 2>/dev/null || echo 12345)"
	exit 0
fi
exit 1
MOCK

cat > "$TEST_BIN/killall" <<'MOCK'
#!/bin/sh
printf '%s\n' "$*" >> "$TEST_LOG"
daemon_pid=$(cat "$TEST_DAEMON_PID" 2>/dev/null || :)
stop_pid()
{
	[ -n "$daemon_pid" ] || return 0
	kill "-$1" "$daemon_pid" 2>/dev/null || :
	i=0
	while kill -0 "$daemon_pid" 2>/dev/null && [ "$i" -lt 20 ]; do
		/bin/sleep 0.05
		i=$((i + 1))
	done
}
case "$TEST_MODE:$*" in
	graceful:ddnsto)
		stop_pid TERM
		echo stopped > "$TEST_STATE"
		;;
	force:ddnsto)
		: # Simulate a daemon that ignores TERM.
		;;
	force:-9\ ddnsto)
		stop_pid KILL
		echo stopped > "$TEST_STATE"
		;;
	stuck:ddnsto|stuck:-9\ ddnsto)
		: # Simulate an unkillable process; the test trap removes the fixture.
		;;
	slow:ddnsto)
		echo TERM-BEGIN >> "$TEST_OPS"
		/bin/sleep 0.1
		stop_pid TERM
		echo stopped > "$TEST_STATE"
		echo TERM-END >> "$TEST_OPS"
		;;
	*) exit 2 ;;
esac
exit 0
MOCK

cat > "$TEST_BIN/sleep" <<'MOCK'
#!/bin/sh
printf '%s\n' "$*" >> "$TEST_SLEEP_LOG"
exec /bin/sleep 0.05
MOCK

cat > "$TEST_BIN/logger" <<'MOCK'
#!/bin/sh
exit 0
MOCK

cat > "$TEST_BIN/nvram" <<'MOCK'
#!/bin/sh
printf '%s\n' "$*" >> "$TEST_NVRAM_LOG"
case "$1:$2" in
	get:ddnsto_id) echo regression-id ;;
	get:ddnsto_enable) cat "$TEST_ENABLE_FILE" ;;
	*) exit 1 ;;
esac
MOCK

cat > "$TEST_BIN/ddnsto" <<'MOCK'
#!/bin/sh
[ "$1" = -u ] && [ "$2" = regression-id ] || exit 2
echo START >> "$TEST_OPS"
echo running > "$TEST_STATE"
echo $$ > "$TEST_DAEMON_PID"
exec /bin/sleep 30
MOCK
chmod +x "$TEST_BIN"/*

PATH="$TEST_BIN:$PATH"
export PATH

run_action() {
	(
		TEST_MODE="$1"
		export TEST_MODE
		ddnsto_lifecycle_lock_run "$2"
	)
}

: > "$TEST_LOG"
: > "$TEST_SLEEP_LOG"
: > "$TEST_NVRAM_LOG"
: > "$TEST_OPS"
echo stopped > "$TEST_STATE"
run_action unused stop || fail "no-process stop returned failure"
[ ! -s "$TEST_LOG" ] || fail "stop signalled a process when pidof found none"

# The lock is a persistent root/user-private inode, never recreated between operations.
ddnsto_lifecycle_lock_prepare "$LOCK_FILE" || fail "private lifecycle lock was rejected"
[ -f "$LOCK_FILE" ] && [ ! -L "$LOCK_FILE" ] || fail "lock file is not a regular file"
[ "$(stat -c '%a' "$LOCK_FILE")" = 600 ] || fail "lock file mode is not 0600"
[ "$(stat -c '%h' "$LOCK_FILE")" = 1 ] || fail "lock file link count is not one"
first_inode=$(stat -c '%i' "$LOCK_FILE")
ddnsto_lifecycle_lock_prepare "$LOCK_FILE" || fail "existing lifecycle lock was rejected"
[ "$(stat -c '%i' "$LOCK_FILE")" = "$first_inode" ] || fail "lock preparation replaced its inode"

# Reject symbolic-link, hard-link and permissive lock-file substitutions.
printf untouched > "$TEST_TMP/referent"
ln -s "$TEST_TMP/referent" "$TEST_TMP/symlink.lock"
if ddnsto_lifecycle_lock_prepare "$TEST_TMP/symlink.lock"; then fail "symlink lock file was accepted"; fi
[ "$(cat "$TEST_TMP/referent")" = untouched ] || fail "symlink referent was modified"
printf lock > "$TEST_TMP/hardlink.lock"
chmod 600 "$TEST_TMP/hardlink.lock"
ln "$TEST_TMP/hardlink.lock" "$TEST_TMP/hardlink-peer"
if ddnsto_lifecycle_lock_prepare "$TEST_TMP/hardlink.lock"; then fail "hard-linked lock file was accepted"; fi
printf lock > "$TEST_TMP/mode.lock"
chmod 644 "$TEST_TMP/mode.lock"
if ddnsto_lifecycle_lock_prepare "$TEST_TMP/mode.lock"; then fail "permissive lock file was accepted"; fi
echo "DDNSTO lifecycle lock keeps a stable private inode and rejects unsafe paths: PASS"

# Lock action failures and an inherited reserved descriptor must fail closed.
action_fail() { return 7; }
if ddnsto_lifecycle_lock_execute "$LOCK_FILE" 0 action_fail; then
	fail "locked action failure was swallowed"
else
	action_status=$?
fi
[ "$action_status" -eq 7 ] || fail "locked action status was not propagated"
if ( exec 9>&1; ddnsto_lifecycle_lock_execute "$LOCK_FILE" 0 action_fail ); then
	fail "inherited FD 9 was accepted"
fi
if ( : <&9 ) 2>/dev/null || ( : >&9 ) 2>/dev/null; then
	fail "lock descriptor leaked after action completion"
fi

# A separate holder forces a zero-wait caller to fail without running its action.
READY="$TEST_TMP/holder.ready"
(
	exec 8>>"$LOCK_FILE"
	flock -n 8 || exit 1
	: > "$READY"
	/bin/sleep 2
) &
holder_pid=$!
ready_attempt=0
while [ ! -f "$READY" ] && [ "$ready_attempt" -lt 100 ]; do
	/bin/sleep 0.02
	ready_attempt=$((ready_attempt + 1))
done
[ -f "$READY" ] || fail "competing lock holder did not acquire the lock"
DDNSTO_LOCK_WAIT_SECONDS=0
if run_action unused start; then fail "contended zero-wait start unexpectedly succeeded"; fi
[ ! -s "$TEST_NVRAM_LOG" ] || fail "contended start reached daemon launch"
wait "$holder_pid" || fail "competing lock holder failed"
DDNSTO_LOCK_WAIT_SECONDS=5
echo "DDNSTO lifecycle lock propagates failures and bounds lock contention: PASS"

# Existing stop postconditions remain intact under the lifecycle dispatcher.
: > "$TEST_LOG"
: > "$TEST_SLEEP_LOG"
echo running > "$TEST_STATE"
run_action graceful stop || fail "graceful stop returned failure"
[ "$(cat "$TEST_LOG")" = "ddnsto" ] || fail "graceful stop did not send TERM exactly once"
[ ! -s "$TEST_SLEEP_LOG" ] || fail "graceful exit was not observed immediately"
[ "$(cat "$TEST_STATE")" = stopped ] || fail "graceful stop postcondition missing"

: > "$TEST_LOG"
: > "$TEST_SLEEP_LOG"
echo running > "$TEST_STATE"
run_action force stop || fail "TERM-resistant daemon did not stop after escalation"
[ "$(cat "$TEST_LOG")" = "$(printf 'ddnsto\n-9 ddnsto')" ] || fail "force stop did not re-enumerate after TERM wait"
[ "$(wc -l < "$TEST_SLEEP_LOG" | tr -d ' ')" = 5 ] || fail "TERM grace period was not bounded to five polls"
[ "$(cat "$TEST_STATE")" = stopped ] || fail "force-stop postcondition missing"

: > "$TEST_LOG"
: > "$TEST_SLEEP_LOG"
echo running > "$TEST_STATE"
if run_action stuck stop; then fail "stop reported success while daemon remained alive after SIGKILL"; fi
[ "$(cat "$TEST_LOG")" = "$(printf 'ddnsto\n-9 ddnsto')" ] || fail "stuck daemon did not receive TERM then KILL"
[ "$(wc -l < "$TEST_SLEEP_LOG" | tr -d ' ')" = 10 ] || fail "TERM/KILL waits were not bounded to five polls each"
[ ! -s "$TEST_TMP/numeric-kill.log" ] || fail "numeric PID signalling was unexpectedly used"
echo "DDNSTO stop retains bounded TERM/KILL and fail-closed postconditions: PASS"

# A stop and a concurrent start serialize: start cannot launch until stop has completed.
: > "$TEST_LOG"
: > "$TEST_SLEEP_LOG"
: > "$TEST_OPS"
echo stopped > "$TEST_STATE"
run_action normal start || fail "initial asynchronous start failed"
# Startup is intentionally asynchronous; observe the mock child with a bounded wait.
start_attempt=0
while { [ "$(cat "$TEST_STATE")" != running ] || [ ! -s "$TEST_DAEMON_PID" ]; } &&
      [ "$start_attempt" -lt 100 ]; do
	/bin/sleep 0.02
	start_attempt=$((start_attempt + 1))
done
[ "$(cat "$TEST_STATE")" = running ] && [ -s "$TEST_DAEMON_PID" ] ||
	fail "mock daemon did not start within the bounded observation window"
# The detached child must not retain FD 9 and keep the launcher lock held.
(
	exec 8>>"$LOCK_FILE"
	flock -n 8
) || fail "background DDNSTO child retained the lifecycle lock descriptor"

DDNSTO_LOCK_WAIT_SECONDS=30
TEST_MODE=slow run_action slow stop &
stop_pid=$!
ready_attempt=0
while ! grep -Fqx TERM-BEGIN "$TEST_OPS" 2>/dev/null && [ "$ready_attempt" -lt 100 ]; do
	/bin/sleep 0.02
	ready_attempt=$((ready_attempt + 1))
done
grep -Fqx TERM-BEGIN "$TEST_OPS" || fail "slow stop did not enter its critical section"
run_action normal start || fail "start did not proceed after the serialized stop released the lock"
wait "$stop_pid" || fail "serialized stop failed"
DDNSTO_LOCK_WAIT_SECONDS=5
awk '/TERM-END/ { stop_end = NR } /^START$/ && stop_end && NR > stop_end { started_after_stop = 1 } END { exit !started_after_stop }' "$TEST_OPS" ||
	fail "concurrent start overlapped the in-progress stop"
echo "DDNSTO start/stop operations serialize and detached daemons do not retain FD 9: PASS"

# RC restart is one stop/start transaction; disabling DDNSTO still performs stop only.
: > "$TEST_LOG"
: > "$TEST_OPS"
echo running > "$TEST_STATE"
run_action graceful restart || fail "serialized restart failed"
# The replacement daemon forks asynchronously; wait for its marker with a hard bound.
restart_attempt=0
while [ "$(grep -Fxc START "$TEST_OPS" || :)" -ne 1 ] &&
      [ "$restart_attempt" -lt 100 ]; do
	/bin/sleep 0.02
	restart_attempt=$((restart_attempt + 1))
done
restart_starts=$(grep -Fxc START "$TEST_OPS" || :)
if [ "$restart_starts" -ne 1 ]; then
	echo "restart diagnostics: enable_file=$(cat "$TEST_ENABLE_FILE") state=$(cat "$TEST_STATE") nvram=$(cat "$TEST_NVRAM_LOG") ops=$(cat "$TEST_OPS")" >&2
	fail "restart did not launch exactly one replacement"
fi
[ "$(cat "$TEST_STATE")" = running ] || fail "restart replacement was not launched"
echo 0 > "$TEST_ENABLE_FILE"
: > "$TEST_OPS"
run_action graceful restart || fail "disabled restart failed to stop existing daemon"
[ ! -s "$TEST_OPS" ] || fail "disabled restart launched a replacement"
[ "$(cat "$TEST_STATE")" = stopped ] || fail "disabled restart left the daemon running"

: > "$TEST_OPS"
echo stopped > "$TEST_STATE"
run_action normal start || fail "disabled direct start failed"
[ ! -s "$TEST_OPS" ] || fail "disabled direct start launched DDNSTO"
[ "$(cat "$TEST_STATE")" = stopped ] || fail "disabled direct start changed process state"

echo "DDNSTO restart is one serialized stop/start action and honors the disabled state: PASS"
