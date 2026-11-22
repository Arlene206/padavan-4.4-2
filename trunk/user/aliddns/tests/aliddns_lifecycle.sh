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
extract_function "aliddns_interval_normalize() {"
extract_function "aliddns_nvram_read_checked() {"
extract_function "aliddns_enable_initialize() {"
extract_function "aliddns_nvram_write_checked() {"
extract_function "aliddns_get_status () {"
extract_function "aliddns_restart_cancel_requested() {"
extract_function "aliddns_lifecycle_lock_path() {"
extract_function "aliddns_lifecycle_lock_prepare() {"
extract_function "aliddns_lifecycle_lock_execute() {"
extract_function "aliddns_lifecycle_control_start() {"
extract_function "aliddns_start_keeper() {"
extract_function "aliddns_check () {"
extract_function "aliddns_lifecycle_locked_action() {"
extract_function "aliddns_lifecycle_lock_run() {"
extract_function "aliddns_keeper_pid_matches() {"
extract_function "aliddns_find_keeper_pids() {"
extract_function "aliddns_stop_keeper() {"
extract_function "aliddns_close () {"
. "$FUNCTIONS"

expect_interval() {
	interval_input="$1"
	interval_expected="$2"
	interval_actual=$(aliddns_interval_normalize "$interval_input") || {
		echo "interval normalizer failed for input: $interval_input" >&2
		exit 1
	}
	[ "$interval_actual" = "$interval_expected" ] || {
		echo "interval normalizer returned '$interval_actual' for '$interval_input', expected '$interval_expected'" >&2
		exit 1
	}
}
expect_interval "" 600
expect_interval 600 600
expect_interval 1 1
expect_interval 0 1
expect_interval 00000 1
expect_interval 00010 10
expect_interval 99999 99999
expect_interval 100000 600
expect_interval "1 2" 600
expect_interval "10seconds" 600
expect_interval "12;touch /tmp/aliddns-interval-injected" 600
expect_interval -1 600
expect_interval 1.5 600
echo "AliDDNS polling interval rejects invalid/zero values and bounds normalization: PASS"

# Checked reads distinguish an absent/default key from a transient NVRAM failure.
NVRAM_ENABLE_VALUE=1
NVRAM_READ_FAIL_KEY=
NVRAM_READ_STATUS=61
NVRAM_SET_STATUS=0
NVRAM_STATUS_VALUE=stored-fingerprint
NVRAM_RENUM_VALUE=1
NVRAM_STATUS_SET_STATUS=0
NVRAM_STATUS_WRITES="$TEST_DIR/nvram-status-writes"
: > "$NVRAM_STATUS_WRITES"
NVRAM_RENUM_WRITES="$TEST_DIR/nvram-renum-writes"
: > "$NVRAM_RENUM_WRITES"
NVRAM_RENUM_WRITE_STATUS=0
NVRAM_STATUS_WRITE_STATUS=0
NVRAM_WRITES="$TEST_DIR/nvram-writes"
NVRAM_LOG="$TEST_DIR/nvram-read.log"
: > "$NVRAM_WRITES"
: > "$NVRAM_LOG"
nvram() {
	case "${1:-} ${2:-}" in
		"get_checked aliddns_enable")
			[ "$NVRAM_READ_FAIL_KEY" != aliddns_enable ] || return "$NVRAM_READ_STATUS"
			printf '%s\n' "$NVRAM_ENABLE_VALUE"
			;;
		"get_checked aliddns_interval")
			[ "$NVRAM_READ_FAIL_KEY" != aliddns_interval ] || return "$NVRAM_READ_STATUS"
			printf '%s\n' "300"
			;;
		"get_checked aliddns_renum")
			[ "$NVRAM_READ_FAIL_KEY" != aliddns_renum ] || return "$NVRAM_READ_STATUS"
			printf '%s\n' "$NVRAM_RENUM_VALUE"
			;;
		"get_checked aliddns_status")
			[ "$NVRAM_READ_FAIL_KEY" != aliddns_status ] || return "$NVRAM_READ_STATUS"
			printf '%s\n' "$NVRAM_STATUS_VALUE"
			;;
		"set aliddns_renum="*)
			printf '%s\\n' "$*" >> "$NVRAM_RENUM_WRITES"
			return "$NVRAM_RENUM_WRITE_STATUS"
			;;
		"set aliddns_status=0")
			printf '%s\\n' "$*" >> "$NVRAM_STATUS_WRITES"
			return "$NVRAM_STATUS_WRITE_STATUS"
			;;
		"set aliddns_status=fixture-fingerprint")
			printf '%s\n' "$*" >> "$NVRAM_STATUS_WRITES"
			return "$NVRAM_STATUS_SET_STATUS"
			;;
		"set aliddns_enable=0")
			printf '%s\n' "$*" >> "$NVRAM_WRITES"
			[ "$NVRAM_SET_STATUS" -eq 0 ] || return "$NVRAM_SET_STATUS"
			NVRAM_ENABLE_VALUE=0
			;;
		*) return 2 ;;
	esac
}
logger() { printf '%s\n' "$*" >> "$NVRAM_LOG"; }

# A transient read failure must never be mistaken for an uninitialized key.
aliddns_enable=1
NVRAM_READ_FAIL_KEY=aliddns_enable
if aliddns_enable_initialize; then
	echo "AliDDNS enable initialization accepted a failed NVRAM read" >&2
	exit 1
else
	nvram_status=$?
fi
[ "$nvram_status" -eq 61 ] || { echo "enable read status changed to $nvram_status" >&2; exit 1; }
[ "$aliddns_enable" = "1" ] || { echo "failed enable read overwrote the active setting" >&2; exit 1; }
[ ! -s "$NVRAM_WRITES" ] || { echo "failed enable read persisted a false disabled value" >&2; exit 1; }
grep -Fq 'NVRAM read failed for aliddns_enable' "$NVRAM_LOG" || { echo "enable read error was not logged" >&2; exit 1; }

# A missing key (successful empty read) still receives the legacy default.
NVRAM_READ_FAIL_KEY=
NVRAM_ENABLE_VALUE=
aliddns_enable=1
aliddns_enable_initialize || { echo "missing enable key did not initialize safely" >&2; exit 1; }
[ "$aliddns_enable" = "0" ] || { echo "missing enable key did not default to disabled" >&2; exit 1; }
[ "$(cat "$NVRAM_WRITES")" = "set aliddns_enable=0" ] || { echo "missing enable key was not persisted exactly once" >&2; exit 1; }

# Failure to persist that default is itself fatal; do not publish a default in memory.
: > "$NVRAM_WRITES"
NVRAM_ENABLE_VALUE=
NVRAM_SET_STATUS=62
aliddns_enable=1
if aliddns_enable_initialize; then
	echo "AliDDNS accepted a failed default write" >&2
	exit 1
else
	nvram_status=$?
fi
[ "$nvram_status" -eq 62 ] || { echo "default-write status changed to $nvram_status" >&2; exit 1; }
[ "$aliddns_enable" = "1" ] || { echo "failed default write changed active enable state" >&2; exit 1; }

# An interval read error must fail before the caller normalizes it to 600
# and writes that fallback over a previously configured interval.
NVRAM_READ_FAIL_KEY=aliddns_interval
NVRAM_READ_STATUS=63
if aliddns_nvram_read_checked aliddns_interval; then
	echo "AliDDNS accepted a failed interval read" >&2
	exit 1
else
	nvram_status=$?
fi
[ "$nvram_status" -eq 63 ] || { echo "interval read status changed to $nvram_status" >&2; exit 1; }
[ "$(wc -l < "$NVRAM_WRITES" | tr -d ' ')" -eq 1 ] || { echo "interval read failure caused another NVRAM write" >&2; exit 1; }
echo "AliDDNS enable and interval NVRAM read failures do not overwrite configured values: PASS"

aliddns_config_fingerprint() { printf '%s\n' "fixture-fingerprint"; }

# Unknown persisted fingerprint state must not be mistaken for a first boot.
NVRAM_READ_FAIL_KEY=aliddns_status
NVRAM_READ_STATUS=64
NVRAM_STATUS_VALUE=stored-fingerprint
NVRAM_STATUS_SET_STATUS=0
: > "$NVRAM_STATUS_WRITES"
needed_restart=7
if aliddns_get_status; then
	echo "AliDDNS accepted a failed fingerprint NVRAM read" >&2
	exit 1
else
	nvram_status=$?
fi
[ "$nvram_status" -eq 64 ] || { echo "fingerprint read error status changed to $nvram_status" >&2; exit 1; }
[ "$needed_restart" = "7" ] || { echo "fingerprint read error changed the restart decision" >&2; exit 1; }
[ ! -s "$NVRAM_STATUS_WRITES" ] || { echo "fingerprint read error overwrote persisted state" >&2; exit 1; }

# A failed fingerprint write must not publish a restart decision as persisted.
NVRAM_READ_FAIL_KEY=
NVRAM_STATUS_SET_STATUS=65
needed_restart=7
if aliddns_get_status; then
	echo "AliDDNS accepted a failed fingerprint NVRAM write" >&2
	exit 1
else
	nvram_status=$?
fi
[ "$nvram_status" -eq 65 ] || { echo "fingerprint write error status changed to $nvram_status" >&2; exit 1; }
[ "$needed_restart" = "7" ] || { echo "fingerprint write error changed the restart decision" >&2; exit 1; }
[ "$(cat "$NVRAM_STATUS_WRITES")" = "set aliddns_status=fixture-fingerprint" ] || { echo "failed fingerprint write was not recorded exactly once" >&2; exit 1; }

# Matching and successfully updated fingerprints keep the legacy decisions.
: > "$NVRAM_STATUS_WRITES"
NVRAM_STATUS_SET_STATUS=0
NVRAM_STATUS_VALUE=fixture-fingerprint
aliddns_get_status || { echo "matching fingerprint read failed" >&2; exit 1; }
[ "$needed_restart" = "0" ] || { echo "matching fingerprint requested a restart" >&2; exit 1; }
NVRAM_STATUS_VALUE=stored-fingerprint
aliddns_get_status || { echo "changed fingerprint was not handled" >&2; exit 1; }
[ "$needed_restart" = "1" ] || { echo "changed fingerprint did not request a restart" >&2; exit 1; }
[ "$(cat "$NVRAM_STATUS_WRITES")" = "set aliddns_status=fixture-fingerprint" ] || { echo "fingerprint update was not persisted" >&2; exit 1; }
echo "AliDDNS fingerprint NVRAM read/write failures preserve the last-known restart state: PASS"

# A failed retry-counter read is not interpreted as a request to cancel the cooldown.
NVRAM_READ_FAIL_KEY=aliddns_renum
NVRAM_READ_STATUS=66
if aliddns_restart_cancel_requested; then
	echo "AliDDNS accepted a failed retry-cancellation read" >&2
	exit 1
else
	nvram_status=$?
fi
[ "$nvram_status" -eq 66 ] || { echo "retry-cancellation read status changed to $nvram_status" >&2; exit 1; }
NVRAM_READ_FAIL_KEY=
NVRAM_RENUM_VALUE=0
aliddns_restart_cancel_requested || { echo "zero retry counter did not request cancellation" >&2; exit 1; }
NVRAM_RENUM_VALUE=1
if aliddns_restart_cancel_requested; then
	echo "nonzero retry counter incorrectly requested cancellation" >&2
	exit 1
else
	nvram_status=$?
fi
[ "$nvram_status" -eq 1 ] || { echo "nonzero retry counter returned status $nvram_status" >&2; exit 1; }
echo "AliDDNS retry cancellation checks preserve state on NVRAM read errors: PASS"

# Retry counters and the pre-relaunch fingerprint reset must propagate NVRAM write failures.
NVRAM_RENUM_WRITE_STATUS=67
: > "$NVRAM_RENUM_WRITES"
if aliddns_nvram_write_checked aliddns_renum 3; then
	echo "AliDDNS accepted a failed retry-counter write" >&2
	exit 1
else
	nvram_status=$?
fi
[ "$nvram_status" -eq 67 ] || { echo "retry-counter write status changed to $nvram_status" >&2; exit 1; }
[ "$(cat "$NVRAM_RENUM_WRITES")" = "set aliddns_renum=3" ] || { echo "retry-counter write was not attempted exactly once" >&2; exit 1; }
grep -Fq 'NVRAM write failed for aliddns_renum' "$NVRAM_LOG" || { echo "retry-counter write failure was not logged" >&2; exit 1; }
NVRAM_RENUM_WRITE_STATUS=0
aliddns_nvram_write_checked aliddns_renum 0 || { echo "successful retry-counter write failed" >&2; exit 1; }
NVRAM_STATUS_WRITE_STATUS=68
if aliddns_nvram_write_checked aliddns_status 0; then
	echo "AliDDNS accepted a failed status reset write" >&2
	exit 1
else
	nvram_status=$?
fi
[ "$nvram_status" -eq 68 ] || { echo "status reset write status changed to $nvram_status" >&2; exit 1; }
grep -Fq 'NVRAM write failed for aliddns_status' "$NVRAM_LOG" || { echo "status reset write failure was not logged" >&2; exit 1; }
echo "AliDDNS retry and relaunch NVRAM writes propagate backend errors: PASS"

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
mkdir -p "$PROC_ROOT/self" "$PROC_ROOT/101" "$PROC_ROOT/102" "$PROC_ROOT/103" "$PROC_ROOT/104" "$PROC_ROOT/$"
: > "$PROC_ROOT/self/cmdline"
scriptfilepath="$TEST_DIR/aliddns.sh"
ALIDDNS_PROC_ROOT="$PROC_ROOT"
printf 'sh\000%s\000keep\000' "$scriptfilepath" > "$PROC_ROOT/101/cmdline"
printf 'sh\000%s\000stop\000' "$scriptfilepath" > "$PROC_ROOT/102/cmdline"
printf 'sh\000%s-extra\000keep\000' "$scriptfilepath" > "$PROC_ROOT/103/cmdline"
printf 'sh\000%s\000keep\000unexpected\000' "$scriptfilepath" > "$PROC_ROOT/104/cmdline"
printf 'sh\000%s\000keep\000' "$scriptfilepath" > "$PROC_ROOT/$/cmdline"

aliddns_find_keeper_pids
[ "$ALIDDNS_KEEPER_PIDS" = "101" ] || {
	echo "keeper discovery accepted a similar path, non-final keep argument or the current shell" >&2
	exit 1
}
echo "AliDDNS keeper process identity matches exact script/keep argv: PASS"

# An unreadable cmdline for a still-present PID makes the scan UNKNOWN, not ABSENT.
mkdir -p "$PROC_ROOT/105"
printf 'sh\\000%s\\000keep\\000' "$scriptfilepath" > "$PROC_ROOT/105/cmdline"
chmod 000 "$PROC_ROOT/105/cmdline"
if aliddns_find_keeper_pids; then
	echo "keeper discovery accepted an incomplete /proc inventory" >&2
	exit 1
fi
[ -z "$ALIDDNS_KEEPER_PIDS" ] || {
	echo "failed keeper discovery retained a partial PID snapshot" >&2
	exit 1
}
chmod 600 "$PROC_ROOT/105/cmdline"
rm -rf "$PROC_ROOT/105"
echo "AliDDNS keeper discovery refuses incomplete /proc visibility: PASS"

# A new keeper is not accepted until its exact path/final-keep argv is visible.
ALIDDNS_PROC_ROOT="/proc"
export ALIDDNS_PROC_ROOT
KEEPER_LAUNCHER="$TEST_DIR/keeper-launcher.sh"
cat > "$KEEPER_LAUNCHER" <<'KEEPER_LAUNCHER'
#!/bin/sh
trap 'exit 0' TERM INT HUP
while :; do sleep 1; done
KEEPER_LAUNCHER
chmod 700 "$KEEPER_LAUNCHER"
scriptfilepath="$KEEPER_LAUNCHER"
aliddns_start_keeper || { echo "keeper launch did not confirm exact /proc identity" >&2; exit 1; }
[ -n "$ALIDDNS_STARTED_KEEPER_PID" ] || { echo "confirmed keeper PID was not published" >&2; exit 1; }
aliddns_keeper_pid_matches "$ALIDDNS_STARTED_KEEPER_PID" "/proc" || { echo "confirmed PID lost exact argv identity" >&2; exit 1; }
kill -TERM "$ALIDDNS_STARTED_KEEPER_PID" 2>/dev/null || :
wait "$ALIDDNS_STARTED_KEEPER_PID" 2>/dev/null || :

# A missing executable must never be accepted as a successful background start.
scriptfilepath="$TEST_DIR/missing-keeper"
failed_launch_status=0
if aliddns_start_keeper; then
	echo "missing keeper executable was accepted" >&2
	exit 1
else
	failed_launch_status=$?
fi
[ "$failed_launch_status" -ne 0 ] || { echo "missing keeper launch returned success" >&2; exit 1; }
[ -z "$ALIDDNS_STARTED_KEEPER_PID" ] || { echo "failed launch published a keeper PID" >&2; exit 1; }

ALIDDNS_PROC_ROOT="$PROC_ROOT"
export ALIDDNS_PROC_ROOT
scriptfilepath="$TEST_DIR/aliddns.sh"

# Restore the production dispatcher after the earlier ordering test stubbed aliddns_check.
. "$FUNCTIONS"

# A failed launch must be returned to the locked caller, not hidden by exit 0.
aliddns_get_status() { needed_restart=1; }
aliddns_close() { return 0; }
aliddns_start_keeper() { return 23; }
aliddns_enable=1
start_status=0
if aliddns_check; then
	echo "aliddns_check accepted a failed keeper launch" >&2
	exit 1
else
	start_status=$?
fi
[ "$start_status" -eq 23 ] || { echo "keeper launch failure status was not propagated" >&2; exit 1; }
echo "AliDDNS keeper startup requires RUNNING identity and propagates launch failure: PASS"

. "$FUNCTIONS"

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
