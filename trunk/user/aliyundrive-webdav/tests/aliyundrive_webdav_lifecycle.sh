#!/bin/sh
set -eu

TEST_DIR="$(mktemp -d)"
trap 'rm -rf "$TEST_DIR"' EXIT HUP INT TERM
SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
LAUNCHER="$SCRIPT_DIR/../aliyundrive-webdav.sh"
ALD_LOCK_FILE="$TEST_DIR/aliyundrive-webdav.lock"
TEST_RUN_MARKER="$TEST_DIR/daemon.running"
TEST_START_FAIL_FILE="$TEST_DIR/start-fail"
export TEST_RUN_MARKER TEST_START_FAIL_FILE

TEST_RUNNING=0
TEST_ENABLE=1
TEST_TERM_BEHAVIOR=stuck
TEST_KILL_BEHAVIOR=exit
TEST_SLEEP_COUNT=0
TEST_TERM_COUNT=0
TEST_KILL_COUNT=0
TEST_NUMERIC_KILL=0
TEST_ARG_LOG="$TEST_DIR/argv.log"
export TEST_ARG_LOG

logger() { :; }
nvram() {
  [ "$1" = get ] || return 1
  case "$2" in
    aliyundrive_enable) printf '%s\n' "$TEST_ENABLE" ;;
    ald_refresh_token) printf '%s\n' 'refresh token with spaces *' ;;
    ald_auth_user) printf '%s\n' 'user name' ;;
    ald_auth_password) printf '%s\n' 'secret with spaces ; data' ;;
    ald_read_buffer_size) printf '%s\n' '64' ;;
    ald_cache_size) printf '%s\n' '1024' ;;
    ald_cache_ttl) printf '%s\n' '300' ;;
    ald_host) printf '%s\n' 'host name *; literal' ;;
    ald_port) printf '%s\n' '8080' ;;
    ald_root) printf '%s\n' '/folder with spaces' ;;
    ald_domain_id) printf '%s\n' '123' ;;
    ald_no_trash|ald_read_only) printf '%s\n' '1' ;;
    *) return 1 ;;
  esac
}
pidof() {
  [ "$1" = "aliyundrive-webdav" ] || return 2
  if [ "$TEST_RUNNING" = 1 ] || [ -e "$TEST_RUN_MARKER" ]; then
    printf '%s\n' '12345'
    return 0
  fi
  return 1
}
test_mark_stopped() {
  TEST_RUNNING=0
  rm -f "$TEST_RUN_MARKER"
}
killall() {
  if [ "$1" = "-9" ]; then
    [ "$2" = "aliyundrive-webdav" ] || return 2
    TEST_KILL_COUNT=$((TEST_KILL_COUNT + 1))
    [ "$TEST_KILL_BEHAVIOR" = "exit" ] && test_mark_stopped
  else
    [ "$1" = "aliyundrive-webdav" ] || return 2
    TEST_TERM_COUNT=$((TEST_TERM_COUNT + 1))
    [ "$TEST_TERM_BEHAVIOR" = "immediate" ] && test_mark_stopped
  fi
  return 0
}
sleep() {
  TEST_SLEEP_COUNT=$((TEST_SLEEP_COUNT + 1))
  if [ "$TEST_TERM_BEHAVIOR" = "after-one-poll" ] && [ "$TEST_SLEEP_COUNT" -eq 1 ]; then
    test_mark_stopped
  fi
  # Yield to background daemon mocks so the readiness marker can become visible.
  command sleep 0.05
  return 0
}
kill() {
  TEST_NUMERIC_KILL=$((TEST_NUMERIC_KILL + 1))
  return 1
}

set -- check
. "$LAUNCHER"
set --
ALD_BIN="$TEST_DIR/mock-aliyundrive-webdav"
ALD_START_WAIT_SECONDS=3

# Lifecycle transitions are serialized by one stable lock inode.
exec 8>"$ALD_LOCK_FILE"
flock -n 8
TEST_RUNNING=0
TEST_TERM_COUNT=0
TEST_KILL_COUNT=0
rm -f "$TEST_ARG_LOG" "$TEST_RUN_MARKER"
if start_ald; then
  echo "FAIL: lock contention allowed a lifecycle transition" >&2
  exit 1
fi
[ ! -e "$TEST_ARG_LOG" ] && [ "$TEST_TERM_COUNT" -eq 0 ] || {
  echo "FAIL: contended start read credentials or launched the daemon" >&2; exit 1;
}
TEST_RUNNING=1
if kill_ald; then
  echo "FAIL: lock contention allowed a stop transition" >&2
  exit 1
fi
[ "$TEST_TERM_COUNT" -eq 0 ] && [ "$TEST_KILL_COUNT" -eq 0 ] || {
  echo "FAIL: contended stop signaled the daemon" >&2; exit 1;
}
flock -u 8
exec 8>&-

# No-process stop is a no-op and must not signal anything.
TEST_RUNNING=0
TEST_TERM_COUNT=0
TEST_KILL_COUNT=0
TEST_SLEEP_COUNT=0
TEST_NUMERIC_KILL=0
kill_ald || { echo "FAIL: no-process stop returned failure" >&2; exit 1; }
[ "$TEST_TERM_COUNT" -eq 0 ] && [ "$TEST_KILL_COUNT" -eq 0 ] || {
  echo "FAIL: no-process stop sent a signal" >&2; exit 1;
}

# A daemon that exits during the TERM grace period must never receive KILL.
TEST_RUNNING=1
TEST_TERM_BEHAVIOR=after-one-poll
TEST_KILL_BEHAVIOR=exit
TEST_TERM_COUNT=0
TEST_KILL_COUNT=0
TEST_SLEEP_COUNT=0
TEST_NUMERIC_KILL=0
kill_ald || { echo "FAIL: graceful stop returned failure" >&2; exit 1; }
[ "$TEST_TERM_COUNT" -eq 1 ] && [ "$TEST_KILL_COUNT" -eq 0 ] &&
[ "$TEST_SLEEP_COUNT" -eq 1 ] && [ "$TEST_NUMERIC_KILL" -eq 0 ] || {
  echo "FAIL: graceful stop escalated or used a cached numeric PID" >&2; exit 1;
}

# A TERM-resistant daemon is re-enumerated by name for KILL, then verified absent.
TEST_RUNNING=1
TEST_TERM_BEHAVIOR=stuck
TEST_KILL_BEHAVIOR=exit
TEST_TERM_COUNT=0
TEST_KILL_COUNT=0
TEST_SLEEP_COUNT=0
TEST_NUMERIC_KILL=0
kill_ald || { echo "FAIL: KILL escalation did not complete" >&2; exit 1; }
[ "$TEST_TERM_COUNT" -eq 1 ] && [ "$TEST_KILL_COUNT" -eq 1 ] &&
[ "$TEST_SLEEP_COUNT" -eq 5 ] && [ "$TEST_RUNNING" -eq 0 ] &&
[ "$TEST_NUMERIC_KILL" -eq 0 ] || {
  echo "FAIL: bounded TERM/KILL/absence sequence is incorrect" >&2; exit 1;
}

# If the daemon survives KILL, stop fails and reports no false success.
TEST_RUNNING=1
TEST_TERM_BEHAVIOR=stuck
TEST_KILL_BEHAVIOR=stuck
TEST_TERM_COUNT=0
TEST_KILL_COUNT=0
TEST_SLEEP_COUNT=0
TEST_NUMERIC_KILL=0
if kill_ald; then
  echo "FAIL: permanently running daemon was reported stopped" >&2
  exit 1
fi
[ "$TEST_TERM_COUNT" -eq 1 ] && [ "$TEST_KILL_COUNT" -eq 1 ] &&
[ "$TEST_SLEEP_COUNT" -eq 10 ] && [ "$TEST_NUMERIC_KILL" -eq 0 ] || {
  echo "FAIL: stuck-daemon timeout was not bounded or used a numeric PID" >&2; exit 1;
}

# A duplicate start must be a no-op before it reads credentials or launches.
TEST_RUNNING=1
TEST_ENABLE=1
rm -f "$TEST_ARG_LOG"
TEST_TERM_COUNT=0
TEST_KILL_COUNT=0
start_ald || { echo "FAIL: duplicate start returned failure" >&2; exit 1; }
[ ! -e "$TEST_ARG_LOG" ] && [ "$TEST_TERM_COUNT" -eq 0 ] &&
[ "$TEST_KILL_COUNT" -eq 0 ] || {
  echo "FAIL: duplicate start launched or stopped an existing daemon" >&2; exit 1;
}

# A disabled start request must use the same checked stop path.
TEST_RUNNING=1
TEST_ENABLE=0
TEST_TERM_BEHAVIOR=after-one-poll
TEST_KILL_BEHAVIOR=exit
TEST_TERM_COUNT=0
TEST_KILL_COUNT=0
TEST_SLEEP_COUNT=0
start_ald || { echo "FAIL: disabled start did not stop the daemon" >&2; exit 1; }
[ "$TEST_RUNNING" -eq 0 ] && [ "$TEST_TERM_COUNT" -eq 1 ] &&
[ "$TEST_KILL_COUNT" -eq 0 ] || {
  echo "FAIL: disabled start did not complete graceful stop" >&2; exit 1;
}

cat > "$ALD_BIN" <<'MOCK'
#!/bin/sh
: > "$TEST_ARG_LOG"
for arg do
  printf '%s\n' "$arg" >> "$TEST_ARG_LOG"
done
if [ ! -e "$TEST_START_FAIL_FILE" ]; then
  : > "$TEST_RUN_MARKER"
fi
MOCK
chmod 700 "$ALD_BIN"

# A launched daemon must become observable before start reports success.
TEST_RUNNING=0
TEST_ENABLE=1
TEST_TERM_BEHAVIOR=stuck
: > "$TEST_START_FAIL_FILE"
rm -f "$TEST_RUN_MARKER" "$TEST_ARG_LOG"
TEST_SLEEP_COUNT=0
if start_ald; then
  echo "FAIL: startup timeout was reported successful" >&2
  exit 1
fi
[ -f "$TEST_ARG_LOG" ] && [ "$TEST_SLEEP_COUNT" -eq "$ALD_START_WAIT_SECONDS" ] && [ ! -e "$TEST_RUN_MARKER" ] || {
  echo "FAIL: startup readiness wait was not bounded or did not time out" >&2; exit 1;
}
rm -f "$TEST_START_FAIL_FILE"

# Argument integrity: each untrusted NVRAM string remains one literal argv item.
rm -f "$TEST_RUN_MARKER"
TEST_RUNNING=0
TEST_ENABLE=1
rm -f "$TEST_START_FAIL_FILE" "$TEST_ARG_LOG" "$TEST_RUN_MARKER"
start_ald || { echo "FAIL: enabled start failed" >&2; exit 1; }
TEST_LAUNCH_PID=$!
wait "$TEST_LAUNCH_PID"
cat > "$TEST_DIR/expected-argv" <<'EXPECTED'
-I
--no-trash
--read-only
--host
host name *; literal
--root
/folder with spaces
-p
8080
-r
refresh token with spaces *
-U
user name
-W
secret with spaces ; data
-S
64
--cache-size
1024
--cache-ttl
300
--workdir
/tmp/aliyundrive-webdav
EXPECTED
cmp -s "$TEST_DIR/expected-argv" "$TEST_ARG_LOG" || {
  echo "FAIL: NVRAM values were split, globbed or reordered in daemon argv" >&2
  diff -u "$TEST_DIR/expected-argv" "$TEST_ARG_LOG" >&2 || :
  exit 1
}

# The special domain profile must not inherit normal-mode optional flags.
nvram() {
  [ "$1" = get ] || return 1
  case "$2" in
    aliyundrive_enable) printf '%s\n' '1' ;;
    ald_refresh_token) printf '%s\n' 'token' ;;
    ald_auth_user) printf '%s\n' 'user' ;;
    ald_auth_password) printf '%s\n' 'pass' ;;
    ald_read_buffer_size) printf '%s\n' '64' ;;
    ald_cache_size) printf '%s\n' '1024' ;;
    ald_cache_ttl) printf '%s\n' '300' ;;
    ald_host) printf '%s\n' '127.0.0.1' ;;
    ald_port) printf '%s\n' '8080' ;;
    ald_root) printf '%s\n' '/' ;;
    ald_domain_id) printf '%s\n' '99999' ;;
    ald_no_trash|ald_read_only) printf '%s\n' '1' ;;
    *) return 1 ;;
  esac
}
TEST_RUNNING=0
rm -f "$TEST_ARG_LOG" "$TEST_RUN_MARKER"
start_ald || { echo "FAIL: special-domain start failed" >&2; exit 1; }
TEST_LAUNCH_PID=$!
wait "$TEST_LAUNCH_PID"
grep -Fqx -- '--domain-id' "$TEST_ARG_LOG" &&
grep -Fqx -- '99999' "$TEST_ARG_LOG" &&
! grep -Fqx -- '--no-trash' "$TEST_ARG_LOG" &&
! grep -Fqx -- '--read-only' "$TEST_ARG_LOG" || {
  echo "FAIL: special-domain options differ from the existing policy" >&2; exit 1;
}

echo "AliyunDrive WebDAV bounded stop and process-name signaling regression: PASS"
echo "AliyunDrive WebDAV duplicate-start guard and argv integrity regression: PASS"
echo "AliyunDrive WebDAV serialized lifecycle lock and startup readiness regression: PASS"
