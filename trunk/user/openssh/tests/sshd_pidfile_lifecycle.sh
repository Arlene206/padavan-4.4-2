#!/bin/sh
set -eu

TEST_DIR=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
SCRIPT="$TEST_DIR/../sshd.sh"
TMP_DIR=$(mktemp -d "${TMPDIR:-/tmp}/openssh-pidfile.XXXXXX")
trap 'rm -rf "$TMP_DIR"' EXIT HUP INT TERM

sed -n \
	-e '/^sshd_process_start_time()/,/^}/p' \
	-e '/^func_reload()/,/^}/p' \
	-e '/^func_stop()/,/^}/p' \
	"$SCRIPT" > "$TMP_DIR/functions.sh"
[ -s "$TMP_DIR/functions.sh" ] || {
	echo "could not extract OpenSSH PID-file lifecycle functions" >&2
	exit 1
}

. "$TMP_DIR/functions.sh"

SSHD_PROC_ROOT="$TMP_DIR/proc"
pid_file="$TMP_DIR/sshd.pid"
MOCK_SIGNAL_LOG="$TMP_DIR/signals.log"
export SSHD_PROC_ROOT MOCK_SIGNAL_LOG

kill()
{
	printf '%s\n' "$*" >> "$MOCK_SIGNAL_LOG"
	return "${MOCK_KILL_STATUS:-0}"
}

logger()
{
	:
}

make_sshd_process()
{
	process_pid="$1"
	process_start_time="$2"
	mkdir -p "$SSHD_PROC_ROOT/$process_pid"
	printf '%s\n' sshd > "$SSHD_PROC_ROOT/$process_pid/comm"
	printf '%s\n' \
		"$process_pid (sshd) S 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 $process_start_time 0 0 0" \
		> "$SSHD_PROC_ROOT/$process_pid/stat"
}

make_sshd_process 4242 123456
printf '%s\n' 4242 > "$pid_file"
: > "$MOCK_SIGNAL_LOG"
func_reload
[ "$(cat "$MOCK_SIGNAL_LOG")" = "-HUP 4242" ] || {
	echo "valid sshd PID file did not signal exactly its verified PID" >&2
	exit 1
}

# A stale PID which now belongs to a different process must never be signalled.
printf '%s\n' httpd > "$SSHD_PROC_ROOT/4242/comm"
: > "$MOCK_SIGNAL_LOG"
if func_reload; then
	echo "reload accepted a PID owned by a different process" >&2
	exit 1
fi
[ ! -s "$MOCK_SIGNAL_LOG" ] || {
	echo "reload signalled a PID owned by a different process" >&2
	exit 1
}
printf '%s\n' sshd > "$SSHD_PROC_ROOT/4242/comm"

# Reject malformed, empty and privileged PID values before any signal attempt.
for invalid_pid in '' '4242 extra' '0' '1' '42x' '999999999999999999999999'; do
	printf '%s\n' "$invalid_pid" > "$pid_file"
	: > "$MOCK_SIGNAL_LOG"
	if func_reload; then
		echo "reload accepted invalid PID value '$invalid_pid'" >&2
		exit 1
	fi
	[ ! -s "$MOCK_SIGNAL_LOG" ] || {
		echo "reload signalled invalid PID value '$invalid_pid'" >&2
		exit 1
	}
done

# A symlinked PID file must not be followed.
printf '%s\n' 4242 > "$TMP_DIR/pid-target"
rm -f "$pid_file"
ln -s "$TMP_DIR/pid-target" "$pid_file"
: > "$MOCK_SIGNAL_LOG"
if func_reload; then
	echo "reload accepted a symlinked PID file" >&2
	exit 1
fi
[ ! -s "$MOCK_SIGNAL_LOG" ] || {
	echo "reload signalled through a symlinked PID file" >&2
	exit 1
}
rm -f "$pid_file"

# A missing /proc identity (including a stale PID) must fail closed.
printf '%s\n' 4242 > "$pid_file"
rm -rf "$SSHD_PROC_ROOT/4242"
: > "$MOCK_SIGNAL_LOG"
if func_reload; then
	echo "reload accepted a stale PID with no procfs identity" >&2
	exit 1
fi
[ ! -s "$MOCK_SIGNAL_LOG" ] || {
	echo "reload signalled a stale PID with no procfs identity" >&2
	exit 1
}

# A process identity change between the two /proc reads must be rejected.
sshd_process_start_time()
{
	identity_read_count=$(cat "$TMP_DIR/identity-count" 2>/dev/null || printf '0')
	identity_read_count=$((identity_read_count + 1))
	printf '%s\n' "$identity_read_count" > "$TMP_DIR/identity-count"
	if [ "$identity_read_count" -eq 1 ]; then
		printf '%s\n' 123456
	else
		printf '%s\n' 654321
	fi
}
: > "$TMP_DIR/identity-count"
: > "$MOCK_SIGNAL_LOG"
if func_reload; then
	echo "reload accepted a changed PID identity" >&2
	exit 1
fi
[ ! -s "$MOCK_SIGNAL_LOG" ] || {
	echo "reload signalled after the PID identity changed" >&2
	exit 1
}
. "$TMP_DIR/functions.sh"

# Signal errors must be returned to the caller rather than reported as success.
make_sshd_process 4242 123456
MOCK_KILL_STATUS=37
export MOCK_KILL_STATUS
if func_reload; then
	echo "reload hid a failed SIGHUP operation" >&2
	exit 1
else
	reload_status=$?
fi
[ "$reload_status" -eq 37 ] || {
	echo "reload returned $reload_status instead of signal status 37" >&2
	exit 1
}


# Direct stop must delegate to the shared tri-state stop transaction.
PROCESS_LIFECYCLE_LIB="$TMP_DIR/process_lifecycle.sh"
MOCK_STOP_LOG="$TMP_DIR/stop-helper.log"
export PROCESS_LIFECYCLE_LIB MOCK_STOP_LOG
cat > "$PROCESS_LIFECYCLE_LIB" <<'STUB'
pl_stop_named()
{
	printf '%s\n' "$*" >> "$MOCK_STOP_LOG"
	return "${MOCK_STOP_STATUS:-0}"
}
STUB
: > "$MOCK_STOP_LOG"
func_stop
[ "$(cat "$MOCK_STOP_LOG")" = "sshd 3 2" ] || {
	echo "direct stop did not invoke the bounded shared SSH lifecycle helper" >&2
	exit 1
}

MOCK_STOP_STATUS=37
export MOCK_STOP_STATUS
: > "$MOCK_STOP_LOG"
if func_stop; then
	echo "direct stop hid a failed shared stop transaction" >&2
	exit 1
else
	stop_status=$?
fi
[ "$stop_status" -eq 37 ] || {
	echo "direct stop returned $stop_status instead of helper status 37" >&2
	exit 1
}
[ "$(cat "$MOCK_STOP_LOG")" = "sshd 3 2" ] || {
	echo "failed stop did not use the shared helper" >&2
	exit 1
}

# A missing helper is UNKNOWN: never fall back to killall.
PROCESS_LIFECYCLE_LIB="$TMP_DIR/missing-process-lifecycle.sh"
export PROCESS_LIFECYCLE_LIB
: > "$MOCK_STOP_LOG"
if func_stop; then
	echo "direct stop succeeded without its required lifecycle helper" >&2
	exit 1
else
	stop_status=$?
fi
[ "$stop_status" -eq 2 ] && [ ! -s "$MOCK_STOP_LOG" ] || {
	echo "missing helper did not fail closed" >&2
	exit 1
}

printf '%s\n' "OpenSSH PID-file and direct-stop lifecycle regression: PASS"
