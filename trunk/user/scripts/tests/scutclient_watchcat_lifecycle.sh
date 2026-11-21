#!/bin/sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname "$0")/../../../.." && pwd)
TEST_TMP=$(mktemp -d "${TMPDIR:-/tmp}/scut-watchcat.XXXXXX")
mkdir -p "$TEST_TMP"
TEST_BIN="$TEST_TMP/bin"
PROC_ROOT="$TEST_TMP/proc"
PIDFILE="$TEST_TMP/watchcat.pid"
LOCKFILE="${PIDFILE}.lock"
NVRAM_LOG="$TEST_TMP/nvram.log"
KILL_LOG="$TEST_TMP/kill.log"
RM_BLOCKED="$TEST_TMP/rm-blocked"
OUTSIDE="$TEST_TMP/outside.pid"
mkdir -p "$TEST_BIN" "$PROC_ROOT"
trap 'rm -rf "$TEST_TMP"' EXIT HUP INT TERM

cat > "$TEST_BIN/nvram" <<'MOCK'
#!/bin/sh
printf '%s\n' "$*" >> "$NVRAM_LOG"
if [ "${REPLACE_PIDFILE_ON_NVRAM:-0}" = "1" ]; then
	printf '%s\n' 7777 > "$SCUTCLIENT_WATCHCAT_PIDFILE"
fi
printf '0\n'
MOCK
cat > "$TEST_BIN/kill" <<'MOCK'
#!/bin/sh
printf '%s\n' "$*" >> "$KILL_LOG"
exit 0
MOCK
cat > "$TEST_BIN/rm" <<'MOCK'
#!/bin/sh
if [ "${BLOCK_FIRST_RM:-0}" = "1" ] && [ "$#" -eq 2 ] && \
   [ "$1" = "-f" ] && [ "$2" = "$SCUTCLIENT_WATCHCAT_PIDFILE" ] && \
   [ ! -e "$RM_BLOCKED" ]; then
	: > "$RM_BLOCKED"
	sleep 1
fi
exec /bin/rm "$@"
MOCK
chmod 755 "$TEST_BIN/"*

export PATH="$TEST_BIN:$PATH"
export SCUTCLIENT_WATCHCAT_PIDFILE="$PIDFILE"
export SCUTCLIENT_WATCHCAT_PROC_ROOT="$PROC_ROOT"
export NVRAM_LOG KILL_LOG RM_BLOCKED

command -v flock >/dev/null 2>&1 || {
	echo "host flock utility is required for the kernel-lock regression" >&2; exit 1;
}

run_watchcat()
{
	expected_status=$1
	if /bin/sh "$ROOT/trunk/user/scutclient/scripts/scutclient_watchcat.sh"; then
		actual_status=0
	else
		actual_status=$?
	fi
	[ "$actual_status" -eq "$expected_status" ] || {
		echo "watchcat returned $actual_status; expected $expected_status" >&2
		exit 1
	}
}

# A second watchdog must leave a verified instance alone and must not run
# another health check or send it a signal.
mkdir -p "$PROC_ROOT/42424242"
printf '/bin/sh\000/usr/bin/scutclient_watchcat.sh\000' > "$PROC_ROOT/42424242/cmdline"
printf '42424242\n' > "$PIDFILE"
: > "$NVRAM_LOG"
: > "$KILL_LOG"
run_watchcat 0
[ "$(cat "$PIDFILE")" = "42424242" ] || {
	echo "active watchdog PID file was changed" >&2; exit 1;
}
[ ! -s "$NVRAM_LOG" ] && [ ! -s "$KILL_LOG" ] || {
	echo "active watchdog was signalled or a duplicate health check ran" >&2; exit 1;
}

# A recycled PID now belonging to another process is stale; never kill it.
mkdir -p "$PROC_ROOT/51515151"
printf '/usr/sbin/other-service\000' > "$PROC_ROOT/51515151/cmdline"
printf '51515151\n' > "$PIDFILE"
: > "$NVRAM_LOG"
: > "$KILL_LOG"
run_watchcat 0
[ ! -e "$PIDFILE" ] || {
	echo "watchdog did not clean up its own replacement PID file" >&2; exit 1;
}
[ ! -s "$KILL_LOG" ] || {
	echo "watchdog signalled a recycled PID owned by another process" >&2; exit 1;
}


# Race the stale-PID equality check against unlink. The first process pauses
# inside rm after validation; a second invocation must be excluded by flock.
mkdir -p "$PROC_ROOT/81818181"
printf '/usr/sbin/other-service\000' > "$PROC_ROOT/81818181/cmdline"
printf '81818181\n' > "$PIDFILE"
: > "$NVRAM_LOG"
: > "$KILL_LOG"
rm -f "$RM_BLOCKED"
BLOCK_FIRST_RM=1
export BLOCK_FIRST_RM
/bin/sh "$ROOT/trunk/user/scutclient/scripts/scutclient_watchcat.sh" &
FIRST_WATCHCAT_PID=$!
i=0
while [ ! -e "$RM_BLOCKED" ]; do
	i=$((i + 1))
	if [ "$i" -gt 100 ]; then
		kill "$FIRST_WATCHCAT_PID" 2>/dev/null || :
		wait "$FIRST_WATCHCAT_PID" 2>/dev/null || :
		echo "first watchdog did not reach the locked stale-PID unlink" >&2
		exit 1
	fi
	sleep 0.02
done
run_watchcat 0
wait "$FIRST_WATCHCAT_PID"
unset BLOCK_FIRST_RM
[ ! -e "$PIDFILE" ] || {
	echo "concurrent watchdog left a stale/replaced PID file behind" >&2; exit 1;
}
nvram_calls=$(wc -l < "$NVRAM_LOG")
[ "$nvram_calls" -eq 1 ] || {
	echo "overlapping stale-PID reclaim ran $nvram_calls health checks; expected one" >&2
	exit 1
}
[ -f "$LOCKFILE" ] && [ ! -L "$LOCKFILE" ] || {
	echo "watchdog did not retain its stable regular lock inode" >&2; exit 1;
}

# Malformed and exited PIDs may be replaced without signalling anything.
printf 'not-a-pid\n' > "$PIDFILE"
run_watchcat 0
[ ! -e "$PIDFILE" ] || {
	echo "watchdog did not replace and clean up malformed PID state" >&2; exit 1;
}
[ ! -s "$KILL_LOG" ] || {
	echo "watchdog signalled a malformed PID" >&2; exit 1;
}

# If a numeric PID exists but its cmdline is unreadable, identity is UNKNOWN.
# Preserve the existing lock and skip this run rather than guessing.
mkdir -p "$PROC_ROOT/61616161"
printf '61616161\n' > "$PIDFILE"
: > "$NVRAM_LOG"
run_watchcat 1
[ "$(cat "$PIDFILE")" = "61616161" ] || {
	echo "watchdog overwrote PID state while process identity was UNKNOWN" >&2; exit 1;
}
[ ! -s "$NVRAM_LOG" ] && [ ! -s "$KILL_LOG" ] || {
	echo "watchdog acted while process identity was UNKNOWN" >&2; exit 1;
}

# Never follow a symlink supplied as the lock file.
LOCK_OUTSIDE="$TEST_TMP/lock-outside"
printf 'lock-sentinel\n' > "$LOCK_OUTSIDE"
rm -f "$LOCKFILE"
ln -s "$LOCK_OUTSIDE" "$LOCKFILE"
printf '91919191\n' > "$PIDFILE"
run_watchcat 1
[ -L "$LOCKFILE" ] && [ "$(cat "$LOCK_OUTSIDE")" = "lock-sentinel" ] && \
  [ "$(cat "$PIDFILE")" = "91919191" ] || {
	echo "watchdog followed/replaced a symlink lock file or changed the PID file" >&2; exit 1;
}
rm -f "$LOCKFILE" "$PIDFILE"

# Never follow a symlink supplied as the PID file.
rm -f "$PIDFILE"
printf '71717171\n' > "$OUTSIDE"
ln -s "$OUTSIDE" "$PIDFILE"
run_watchcat 1
[ -L "$PIDFILE" ] && [ "$(cat "$OUTSIDE")" = "71717171" ] || {
	echo "watchdog followed or replaced a symlink PID file" >&2; exit 1;
}
rm -f "$PIDFILE"

# Cleanup must remove only the PID file that this process claimed.
REPLACE_PIDFILE_ON_NVRAM=1
export REPLACE_PIDFILE_ON_NVRAM
run_watchcat 0
[ "$(cat "$PIDFILE")" = "7777" ] || {
	echo "watchdog cleanup removed a concurrently replaced PID file" >&2; exit 1;
}
unset REPLACE_PIDFILE_ON_NVRAM

echo "SCUT watchdog PID identity, serialized stale-PID reclaim and fail-closed lock regression: PASS"
