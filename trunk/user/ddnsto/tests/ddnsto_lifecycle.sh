#!/bin/sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname "$0")/../../../.." && pwd)
SCRIPT="$ROOT/trunk/user/ddnsto/ddnsto.sh"
TEST_TMP=$(mktemp -d "${TMPDIR:-/tmp}/ddnsto-lifecycle.XXXXXX")
TEST_BIN="$TEST_TMP/bin"
TEST_STATE="$TEST_TMP/state"
TEST_LOG="$TEST_TMP/signals.log"
TEST_SLEEP_LOG="$TEST_TMP/sleep.log"
TEST_NVRAM_LOG="$TEST_TMP/nvram.log"
TEST_OPS="$TEST_TMP/operations.log"
TEST_DAEMON_PID="$TEST_TMP/daemon.pid"
TEST_PIDOF_CALLS="$TEST_TMP/pidof.calls"
TEST_ENABLE_FILE="$TEST_TMP/enable"
TEST_PROC_EXE_ID="8:128"
TEST_PROC_UID="$(id -u)"
TEST_PROC_STAT_FAIL=0
DDNSTO_PROCESS_LIFECYCLE_LIB="$ROOT/trunk/user/shared/scripts/process_lifecycle.sh"
DDNSTO_PROC_ROOT="$TEST_TMP/proc"
TEST_HELPERS="$TEST_TMP/helpers.sh"
LOCK_FILE="$TEST_TMP/.ddnsto.lifecycle.lock"
export TEST_TMP TEST_STATE TEST_LOG TEST_SLEEP_LOG TEST_NVRAM_LOG TEST_OPS TEST_DAEMON_PID TEST_PIDOF_CALLS TEST_ENABLE_FILE TEST_PROC_EXE_ID TEST_PROC_UID TEST_PROC_STAT_FAIL DDNSTO_PROCESS_LIFECYCLE_LIB DDNSTO_PROC_ROOT

mkdir -p "$TEST_BIN" "$DDNSTO_PROC_ROOT/1"
printf '%s\n' '1 (init) S 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1' > "$DDNSTO_PROC_ROOT/1/stat"
trap 'if [ -s "$TEST_DAEMON_PID" ]; then /bin/kill -9 "$(cat "$TEST_DAEMON_PID")" 2>/dev/null || :; fi; rm -rf "$TEST_TMP"' EXIT HUP INT TERM

fail() {
	echo "DDNSTO lifecycle regression: FAIL: $*" >&2
	exit 1
}

# Import the exact production helpers without executing the installed command dispatcher.
sed -n \
	-e '/^start_zero() {$/,/^}/p' \
	-e '/^ddnsto_confirm_absent() {$/,/^}/p' \
	-e '/^ddnsto_process_state() {$/,/^}/p' \
	-e '/^ddnsto_wait_started() {$/,/^}/p' \
	-e '/^ddnsto_wait_stopped() {$/,/^}/p' \
	-e '/^ddnsto_signal_validated_processes() {$/,/^}/p' \
	-e '/^kill_z() {$/,/^}/p' \
	-e '/^stop_zero() {$/,/^}/p' \
	-e '/^ddnsto_read_enable_state() {$/,/^}/p' \
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
DDNSTO_START_WAIT_ATTEMPTS=5
echo 1 > "$TEST_ENABLE_FILE"
export DDNSTO_BIN DDNSTO_LOCK_WAIT_SECONDS

cat > "$TEST_BIN/pidof" <<'MOCK'
#!/bin/sh
[ "$1" = ddnsto ] || exit 1
# The pre-launch process check uses pidof directly, then the shared helper
# performs its initial lookup and post-scan recheck. Fail the next lookup so the
# UNKNOWN state is injected only after the daemon launch, independent of timing.
if [ "${TEST_UNKNOWN_AFTER_LAUNCH:-0}" = 1 ]; then
	pidof_call_count=$(cat "$TEST_PIDOF_CALLS" 2>/dev/null || echo 0)
	pidof_call_count=$((pidof_call_count + 1))
	printf '%s\n' "$pidof_call_count" > "$TEST_PIDOF_CALLS"
	[ "$pidof_call_count" -lt 4 ] || exit 2
fi
case "$(cat "$TEST_STATE")" in
	running)
		if [ -s "$TEST_DAEMON_PID" ]; then
			cat "$TEST_DAEMON_PID"
		else
			echo 12345
		fi
		exit 0
		;;
	error|unknown) exit 2 ;;
	malformed) echo "12345 invalid"; exit 0 ;;
	empty-success) exit 0 ;;
	inconsistent) echo 12345; exit 1 ;;
	*) exit 1 ;;
esac
MOCK

cat > "$TEST_BIN/stat" <<'MOCK'
#!/bin/sh
if [ "$1" = "-L" ] && [ "$2" = "-c" ] && [ "$3" = "%d:%i" ]; then
	case "$4" in
		"$DDNSTO_BIN") echo 8:128; exit 0 ;;
		/proc/*/exe)
			[ "$TEST_PROC_STAT_FAIL" = 0 ] || exit 1
			echo "$TEST_PROC_EXE_ID"
			exit 0
			;;
	esac
fi
if [ "$1" = "-c" ] && [ "$2" = "%u" ]; then
	case "$3" in /proc/*) echo "$TEST_PROC_UID"; exit 0 ;; esac
fi
exec /usr/bin/stat "$@"
MOCK
chmod +x "$TEST_BIN/stat"

ddnsto_test_real_signal() {
	ddnsto_real_signal="$1"
	ddnsto_real_pid="$2"
	if [ -s "$TEST_DAEMON_PID" ] &&
	   [ "$(cat "$TEST_DAEMON_PID")" = "$ddnsto_real_pid" ]; then
		/bin/kill "-$ddnsto_real_signal" "$ddnsto_real_pid" >/dev/null 2>&1 || :
	fi
}

# Intercept process-helper signals in this fixture and log exact PID arguments.
kill() {
	printf '%s\n' "$*" >> "$TEST_LOG"
	ddnsto_test_signal_name="${1#-}"
	ddnsto_test_signal_pid="$2"
	case "$TEST_MODE:$ddnsto_test_signal_name" in
		graceful:TERM|nonzero-graceful:TERM)
			ddnsto_test_real_signal TERM "$ddnsto_test_signal_pid"
			echo stopped > "$TEST_STATE"
			;;
		force:TERM|stuck:TERM|stuck:KILL)
			: # Simulate a daemon which ignores these signals.
			;;
		force:KILL)
			ddnsto_test_real_signal KILL "$ddnsto_test_signal_pid"
			echo stopped > "$TEST_STATE"
			;;
		unknown-after-term:TERM)
			echo error > "$TEST_STATE"
			;;
		slow:TERM)
			echo TERM-BEGIN >> "$TEST_OPS"
			/bin/sleep 0.1
			ddnsto_test_real_signal TERM "$ddnsto_test_signal_pid"
			echo stopped > "$TEST_STATE"
			echo TERM-END >> "$TEST_OPS"
			;;
		*) ddnsto_test_real_signal "$ddnsto_test_signal_name" "$ddnsto_test_signal_pid" ;;
	esac
	[ "$TEST_MODE" != nonzero-graceful ] || return 29
	return 0
}

cat > "$TEST_BIN/sleep" <<'MOCK'
#!/bin/sh
printf '%s\n' "$*" >> "$TEST_SLEEP_LOG"
[ "${TEST_SLEEP_FAIL:-0}" = 0 ] || exit 7
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
	get_checked:ddnsto_id)
		[ "${TEST_NVRAM_ID_FAIL:-0}" = 0 ] || exit 9
		if [ "${TEST_NVRAM_ID_SET:-0}" = 1 ]; then
			printf '%s\n' "$TEST_NVRAM_ID"
		else
			echo regression-id
		fi
		;;
	get_checked:ddnsto_enable)
		[ "${TEST_NVRAM_ENABLE_FAIL:-0}" = 0 ] || exit 9
		cat "$TEST_ENABLE_FILE"
		;;
	*) exit 1 ;;
esac
MOCK

cat > "$TEST_BIN/ddnsto" <<'MOCK'
#!/bin/sh
[ "$1" = -u ] && [ "$2" = regression-id ] || exit 2
case "${TEST_DAEMON_MODE:-normal}" in
	fail)
		echo LAUNCH-FAIL >> "$TEST_OPS"
		exit 42
		;;
	unknown)
		echo LAUNCH-UNKNOWN >> "$TEST_OPS"
		echo error > "$TEST_STATE"
		exit 0
		;;
	delayed)
		echo START >> "$TEST_OPS"
		/bin/sleep 0.05
		echo running > "$TEST_STATE"
		echo $$ > "$TEST_DAEMON_PID"
		exec /bin/sleep 30
		;;
	normal)
		echo START >> "$TEST_OPS"
		echo running > "$TEST_STATE"
		echo $$ > "$TEST_DAEMON_PID"
		exec /bin/sleep 30
		;;
	*) exit 2 ;;
esac
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

# A same-name PID is UNKNOWN unless executable inode and UID match the installed daemon.
: > "$TEST_LOG"
: > "$TEST_SLEEP_LOG"
echo running > "$TEST_STATE"
TEST_PROC_EXE_ID=8:129
export TEST_PROC_EXE_ID
if run_action unused stop; then fail "stop accepted a same-name process with another executable"; fi
[ ! -s "$TEST_LOG" ] && [ ! -s "$TEST_SLEEP_LOG" ] || fail "spoofed executable was signalled"
TEST_PROC_EXE_ID=8:128
TEST_PROC_UID=65534
export TEST_PROC_UID
if run_action unused stop; then fail "stop accepted a DDNSTO PID owned by another UID"; fi
[ ! -s "$TEST_LOG" ] && [ ! -s "$TEST_SLEEP_LOG" ] || fail "foreign-UID process was signalled"
TEST_PROC_UID="$(id -u)"
TEST_PROC_STAT_FAIL=1
export TEST_PROC_UID TEST_PROC_STAT_FAIL
if run_action unused stop; then fail "stop accepted unreadable executable metadata"; fi
[ ! -s "$TEST_LOG" ] && [ ! -s "$TEST_SLEEP_LOG" ] || fail "unknown executable metadata was signalled"
TEST_PROC_STAT_FAIL=0
export TEST_PROC_STAT_FAIL
echo "DDNSTO stop rejects spoofed executable, foreign UID and unreadable process identity: PASS"

# A pidof miss is ABSENT only after the shared procfs inventory confirms absence.
echo stopped > "$TEST_STATE"
if ddnsto_process_state; then fail "empty pidof result with empty procfs was reported RUNNING"; else
	absence_status=$?
fi
[ "$absence_status" -eq 1 ] || fail "empty procfs did not confirm process absence"
mkdir -p "$DDNSTO_PROC_ROOT/23456"
printf '23456 (ddnsto) S 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1\n' > "$DDNSTO_PROC_ROOT/23456/stat"
if ddnsto_process_state; then fail "pidof miss hid a visible same-name process"; else
	absence_status=$?
fi
[ "$absence_status" -eq 2 ] || fail "pidof/procfs disagreement was not UNKNOWN"
rm -rf "$DDNSTO_PROC_ROOT/23456"
saved_proc_root="$DDNSTO_PROC_ROOT"
DDNSTO_PROC_ROOT="$TEST_TMP/missing-proc"
export DDNSTO_PROC_ROOT
if ddnsto_process_state; then fail "unreadable procfs was treated as ABSENT"; else
	absence_status=$?
fi
[ "$absence_status" -eq 2 ] || fail "unreadable procfs did not produce UNKNOWN"
DDNSTO_PROC_ROOT="$saved_proc_root"
export DDNSTO_PROC_ROOT
echo "DDNSTO pidof absence is verified against procfs; ambiguous and unreadable scans fail closed: PASS"

# Process inventory errors and malformed PID lists are UNKNOWN, never ABSENT.
for unknown_state in error malformed empty-success inconsistent; do
	: > "$TEST_LOG"
	: > "$TEST_SLEEP_LOG"
	echo "$unknown_state" > "$TEST_STATE"
	if run_action unused stop; then fail "stop accepted UNKNOWN process inventory: $unknown_state"; fi
	[ ! -s "$TEST_LOG" ] || fail "stop signalled with UNKNOWN process inventory: $unknown_state"
	[ ! -s "$TEST_SLEEP_LOG" ] || fail "stop waited with UNKNOWN initial inventory: $unknown_state"
done

# start_zero must not launch a replacement when the old process cannot be inventoried.
: > "$TEST_LOG"
: > "$TEST_SLEEP_LOG"
: > "$TEST_NVRAM_LOG"
: > "$TEST_OPS"
echo error > "$TEST_STATE"
if run_action normal start; then fail "start accepted UNKNOWN process inventory"; fi
[ ! -s "$TEST_LOG" ] && [ ! -s "$TEST_OPS" ] ||
	fail "UNKNOWN start signalled or launched a duplicate"
grep -Fqx 'get_checked ddnsto_enable' "$TEST_NVRAM_LOG" ||
	fail "start did not read the enable switch under the lifecycle lock"
grep -Fqx 'get_checked ddnsto_id' "$TEST_NVRAM_LOG" ||
	fail "start did not preflight the daemon identity under the lifecycle lock"

# A failed launch prerequisite must not stop a previously running DDNSTO.
: > "$TEST_LOG"
: > "$TEST_OPS"
echo running > "$TEST_STATE"
saved_ddnsto_bin="$DDNSTO_BIN"
DDNSTO_BIN="$TEST_TMP/missing-ddnsto"
export DDNSTO_BIN
if run_action normal start; then fail "start accepted a missing DDNSTO executable"; fi
[ ! -s "$TEST_LOG" ] && [ ! -s "$TEST_OPS" ] ||
	fail "missing executable disrupted the existing daemon"
DDNSTO_BIN="$saved_ddnsto_bin"
export DDNSTO_BIN

: > "$TEST_NVRAM_LOG"
TEST_NVRAM_ID_FAIL=1
export TEST_NVRAM_ID_FAIL
if run_action normal start; then
	fail "start accepted an NVRAM identity read failure"
else
	identity_status=$?
fi
[ "$identity_status" -eq 9 ] ||
	fail "start did not preserve the NVRAM identity read status"
[ "$(grep -Fxc 'get_checked ddnsto_id' "$TEST_NVRAM_LOG" || :)" = 1 ] ||
	fail "start did not perform exactly one checked identity read"
[ ! -s "$TEST_LOG" ] && [ ! -s "$TEST_OPS" ] ||
	fail "NVRAM read failure disrupted the existing daemon"
TEST_NVRAM_ID_FAIL=0
TEST_NVRAM_ID_SET=1
TEST_NVRAM_ID=""
export TEST_NVRAM_ID_FAIL TEST_NVRAM_ID_SET TEST_NVRAM_ID
if run_action normal start; then fail "start accepted an empty DDNSTO identity"; fi
[ ! -s "$TEST_LOG" ] && [ ! -s "$TEST_OPS" ] ||
	fail "empty identity disrupted the existing daemon"
unset TEST_NVRAM_ID_SET TEST_NVRAM_ID
TEST_NVRAM_ID_FAIL=0
export TEST_NVRAM_ID_FAIL

# Enable-state read errors must abort start/restart before disturbing a live daemon.
TEST_NVRAM_ENABLE_FAIL=1
export TEST_NVRAM_ENABLE_FAIL
for enable_action in start restart; do
	: > "$TEST_LOG"
	: > "$TEST_OPS"
	: > "$TEST_NVRAM_LOG"
	echo running > "$TEST_STATE"
	if run_action graceful "$enable_action"; then
		fail "$enable_action accepted an NVRAM enable-state read failure"
	else
		enable_status=$?
	fi
	[ "$enable_status" -eq 9 ] ||
		fail "$enable_action did not preserve the original NVRAM error status"
	[ "$(cat "$TEST_STATE")" = running ] ||
		fail "$enable_action disrupted the existing daemon after an NVRAM read failure"
	[ ! -s "$TEST_LOG" ] && [ ! -s "$TEST_OPS" ] ||
		fail "$enable_action sent signals or launched after an NVRAM read failure"
	[ "$(grep -Fxc 'get_checked ddnsto_enable' "$TEST_NVRAM_LOG" || :)" = 1 ] ||
		fail "$enable_action did not perform exactly one enable-state preflight"
done
TEST_NVRAM_ENABLE_FAIL=0
export TEST_NVRAM_ENABLE_FAIL

# Restart must not stop a healthy daemon for a corrupt enable selector.
# Empty remains the historical disabled default; only 0/1 are non-empty
# selector values accepted from the binary web UI.
for invalid_enable in unexpected -1 2 '1;reboot'; do
	echo "$invalid_enable" > "$TEST_ENABLE_FILE"
	for enable_action in start restart; do
		: > "$TEST_LOG"
		: > "$TEST_OPS"
		: > "$TEST_NVRAM_LOG"
		echo running > "$TEST_STATE"
		if run_action graceful "$enable_action"; then
			fail "$enable_action accepted invalid DDNSTO enable selector '$invalid_enable'"
		else
			invalid_status=$?
		fi
		[ "$invalid_status" -eq 22 ] ||
			fail "$enable_action returned $invalid_status instead of EINVAL for invalid selector '$invalid_enable'"
		[ "$(cat "$TEST_STATE")" = running ] ||
			fail "$enable_action stopped the daemon for invalid selector '$invalid_enable'"
		[ ! -s "$TEST_LOG" ] && [ ! -s "$TEST_OPS" ] ||
			fail "$enable_action sent signals or launched after invalid selector '$invalid_enable'"
		[ "$(grep -Fxc 'get_checked ddnsto_enable' "$TEST_NVRAM_LOG" || :)" = 1 ] ||
			fail "$enable_action did not read the enable selector exactly once"
	done
done
# Restore the enabled fixture expected by the following launch tests.
echo 1 > "$TEST_ENABLE_FILE"
echo "DDNSTO invalid enable selectors cannot stop or launch a daemon: PASS"

# Background fork success is insufficient: wait for validated daemon presence.
: > "$TEST_LOG"
: > "$TEST_SLEEP_LOG"
: > "$TEST_OPS"
echo stopped > "$TEST_STATE"
TEST_DAEMON_MODE=fail
export TEST_DAEMON_MODE
if run_action normal start; then fail "start reported success after the daemon exited immediately"; fi
unset TEST_DAEMON_MODE
[ "$(cat "$TEST_OPS")" = LAUNCH-FAIL ] || fail "immediate-exit daemon fixture did not execute"
[ "$(cat "$TEST_STATE")" = stopped ] || fail "immediate-exit daemon changed process state"
[ "$(wc -l < "$TEST_SLEEP_LOG" | tr -d ' ')" = 4 ] ||
	fail "confirmed-ABSENT startup did not use exactly five bounded polls"

: > "$TEST_SLEEP_LOG"
: > "$TEST_OPS"
echo stopped > "$TEST_STATE"
: > "$TEST_PIDOF_CALLS"
TEST_UNKNOWN_AFTER_LAUNCH=1
TEST_DAEMON_MODE=unknown
export TEST_UNKNOWN_AFTER_LAUNCH TEST_DAEMON_MODE
if run_action normal start; then fail "start accepted UNKNOWN post-launch process inventory"; fi
unset TEST_UNKNOWN_AFTER_LAUNCH TEST_DAEMON_MODE
unknown_attempt=0
while [ "$(cat "$TEST_STATE")" != error ] && [ "$unknown_attempt" -lt 100 ]; do
	/bin/sleep 0.02
	unknown_attempt=$((unknown_attempt + 1))
done
[ "$(cat "$TEST_STATE")" = error ] || fail "UNKNOWN-start fixture did not publish UNKNOWN"
[ ! -s "$TEST_SLEEP_LOG" ] || fail "UNKNOWN startup inventory was incorrectly retried"

: > "$TEST_SLEEP_LOG"
: > "$TEST_OPS"
echo stopped > "$TEST_STATE"
TEST_DAEMON_MODE=fail
TEST_SLEEP_FAIL=1
export TEST_DAEMON_MODE TEST_SLEEP_FAIL
if run_action normal start; then fail "start accepted a failed startup wait primitive"; fi
unset TEST_DAEMON_MODE
TEST_SLEEP_FAIL=0
export TEST_SLEEP_FAIL
[ "$(wc -l < "$TEST_SLEEP_LOG" | tr -d ' ')" = 1 ] ||
	fail "startup sleep failure was not observed exactly once"
[ "$(cat "$TEST_OPS")" = LAUNCH-FAIL ] || fail "startup wait-failure daemon fixture did not execute"

# ABSENT is retried: a daemon that becomes visible satisfies startup within the bound.
: > "$TEST_SLEEP_LOG"
: > "$TEST_OPS"
: > "$TEST_DAEMON_PID"
echo stopped > "$TEST_STATE"
TEST_DAEMON_MODE=delayed
export TEST_DAEMON_MODE
run_action normal start || fail "start did not accept a delayed but confirmed daemon"
unset TEST_DAEMON_MODE
[ "$(cat "$TEST_STATE")" = running ] && [ -s "$TEST_DAEMON_PID" ] ||
	fail "delayed daemon did not satisfy the RUNNING postcondition"
[ "$(grep -Fxc START "$TEST_OPS" || :)" = 1 ] || fail "delayed startup launched more than once"
[ "$(wc -l < "$TEST_SLEEP_LOG" | tr -d ' ')" -ge 1 ] ||
	fail "delayed startup returned without retrying confirmed ABSENT"
run_action graceful stop || fail "delayed-start cleanup failed"
echo stopped > "$TEST_STATE"
: > "$TEST_DAEMON_PID"

# UNKNOWN after TERM blocks exact-PID SIGKILL escalation.
: > "$TEST_LOG"
: > "$TEST_SLEEP_LOG"
echo running > "$TEST_STATE"
if run_action unknown-after-term stop; then fail "stop accepted UNKNOWN state after TERM"; fi
[ "$(cat "$TEST_LOG")" = "-TERM 12345" ] || fail "UNKNOWN after TERM triggered SIGKILL"
[ ! -s "$TEST_SLEEP_LOG" ] || fail "UNKNOWN after TERM did not abort immediately"

# Sleep failure is UNKNOWN and cannot trigger force-kill escalation.
: > "$TEST_LOG"
: > "$TEST_SLEEP_LOG"
echo running > "$TEST_STATE"
TEST_SLEEP_FAIL=1
export TEST_SLEEP_FAIL
if run_action unused stop; then fail "stop accepted a failed wait primitive"; fi
[ "$(cat "$TEST_LOG")" = "-TERM 12345" ] || fail "sleep failure triggered SIGKILL"
[ "$(wc -l < "$TEST_SLEEP_LOG" | tr -d ' ')" = 1 ] || fail "sleep failure was not observed once"
TEST_SLEEP_FAIL=0
export TEST_SLEEP_FAIL

# A failed TERM command is harmless when a fresh process inventory confirms ABSENT.
: > "$TEST_LOG"
: > "$TEST_SLEEP_LOG"
echo running > "$TEST_STATE"
if run_action nonzero-graceful stop; then :; else fail "nonzero TERM result overrode confirmed ABSENT"; fi
[ "$(cat "$TEST_LOG")" = "-TERM 12345" ] || fail "failed TERM path used unexpected signals"
[ ! -s "$TEST_SLEEP_LOG" ] || fail "failed TERM path waited after confirmed ABSENT"

# Existing stop postconditions remain intact under the lifecycle dispatcher.
: > "$TEST_LOG"
: > "$TEST_SLEEP_LOG"
echo running > "$TEST_STATE"
run_action graceful stop || fail "graceful stop returned failure"
[ "$(cat "$TEST_LOG")" = "-TERM 12345" ] || fail "graceful stop did not send exact-PID TERM exactly once"
[ ! -s "$TEST_SLEEP_LOG" ] || fail "graceful exit was not observed immediately"
[ "$(cat "$TEST_STATE")" = stopped ] || fail "graceful stop postcondition missing"

: > "$TEST_LOG"
: > "$TEST_SLEEP_LOG"
echo running > "$TEST_STATE"
run_action force stop || fail "TERM-resistant daemon did not stop after escalation"
[ "$(cat "$TEST_LOG")" = "$(printf '%s\n' '-TERM 12345' '-KILL 12345')" ] || fail "force stop did not revalidate exact PID after TERM wait"
[ "$(wc -l < "$TEST_SLEEP_LOG" | tr -d ' ')" = 5 ] || fail "TERM grace period was not bounded to five polls"
[ "$(cat "$TEST_STATE")" = stopped ] || fail "force-stop postcondition missing"

: > "$TEST_LOG"
: > "$TEST_SLEEP_LOG"
echo running > "$TEST_STATE"
if run_action stuck stop; then fail "stop reported success while daemon remained alive after SIGKILL"; fi
[ "$(cat "$TEST_LOG")" = "$(printf '%s\n' '-TERM 12345' '-KILL 12345')" ] || fail "stuck daemon did not receive exact-PID TERM then KILL"
[ "$(wc -l < "$TEST_SLEEP_LOG" | tr -d ' ')" = 10 ] || fail "TERM/KILL waits were not bounded to five polls each"
[ "$(grep -Ec '^-(TERM|KILL) [1-9][0-9]*$' "$TEST_LOG" || :)" = 2 ] ||
	fail "force-stop signal records did not carry positive numeric PIDs"
echo "DDNSTO stop retains bounded TERM/KILL and fail-closed postconditions: PASS"

# A stop and a concurrent start serialize: start cannot launch until stop has completed.
: > "$TEST_LOG"
: > "$TEST_SLEEP_LOG"
: > "$TEST_OPS"
echo stopped > "$TEST_STATE"
run_action normal start || fail "initial start did not confirm daemon presence"
# The start call now returns only after pidof confirms the detached daemon.
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
# The launcher has already waited for validated process presence; observe the
# detached child's START marker before asserting the serialized event ordering.
start_after_stop_attempt=0
while { [ "$(grep -Fxc START "$TEST_OPS" || :)" -lt 2 ] ||
        [ "$(cat "$TEST_STATE")" != running ]; } &&
      [ "$start_after_stop_attempt" -lt 100 ]; do
	/bin/sleep 0.02
	start_after_stop_attempt=$((start_after_stop_attempt + 1))
done
[ "$(grep -Fxc START "$TEST_OPS" || :)" -ge 2 ] &&
[ "$(cat "$TEST_STATE")" = running ] || fail "serialized replacement start was not observed"
awk '/TERM-END/ { stop_end = NR } /^START$/ && stop_end && NR > stop_end { started_after_stop = 1 } END { exit !started_after_stop }' "$TEST_OPS" ||
	fail "concurrent start overlapped the in-progress stop"
echo "DDNSTO start/stop operations serialize and detached daemons do not retain FD 9: PASS"

# RC restart is one stop/start transaction; disabling DDNSTO still performs stop only.
: > "$TEST_LOG"
: > "$TEST_OPS"
echo running > "$TEST_STATE"
run_action graceful restart || fail "serialized restart failed"
# The replacement daemon is detached; wait for its marker with a hard bound.
restart_attempt=0
while { [ "$(grep -Fxc START "$TEST_OPS" || :)" -ne 1 ] ||
        [ "$(cat "$TEST_STATE")" != running ]; } &&
      [ "$restart_attempt" -lt 100 ]; do
	/bin/sleep 0.02
	restart_attempt=$((restart_attempt + 1))
done
restart_starts=$(grep -Fxc START "$TEST_OPS" || :)
if [ "$restart_starts" -ne 1 ]; then
	echo "restart diagnostics: enable_file=$(cat "$TEST_ENABLE_FILE") state=$(cat "$TEST_STATE") nvram=$(cat "$TEST_NVRAM_LOG") kill=$(cat "$TEST_LOG") ops=$(cat "$TEST_OPS")" >&2
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

echo "DDNSTO lifecycle regression: PASS"
