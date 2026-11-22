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
(umask 077; : > "$ALD_LOCK_FILE")
[ "$(stat -c %a "$ALD_LOCK_FILE")" = 600 ] || {
  echo "FAIL: test lifecycle lock was not provisioned as mode 0600" >&2; exit 1;
}

TEST_RUNNING=0
TEST_ENABLE=1
TEST_NVRAM_FAIL_KEY=
TEST_PIDOF_MODE=normal
TEST_HTTP_MODE=ready
TEST_HTTP_LOG="$TEST_DIR/http.log"
TEST_HTTP_READY_AFTER=1
TEST_EXPECT_HOST="host name *; literal"
TEST_EXPECT_PORT=8080
: > "$TEST_HTTP_LOG"
TEST_SLEEP_FAIL=0
TEST_TERM_BEHAVIOR=stuck
TEST_KILL_BEHAVIOR=exit
TEST_SLEEP_COUNT=0
TEST_TERM_COUNT=0
TEST_KILL_COUNT=0
TEST_NUMERIC_KILL=0
TEST_ARG_LOG="$TEST_DIR/argv.log"
TEST_ENV_LOG="$TEST_DIR/env.log"
export TEST_ARG_LOG TEST_ENV_LOG

logger() { :; }
nvram() {
  [ "$1" = get_checked ] || return 1
  [ "${TEST_NVRAM_FAIL_KEY-}" != "$2" ] || return 1
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
  case "$TEST_PIDOF_MODE" in
    error|unknown-after-term) return 2 ;;
    empty-success) return 0 ;;
    malformed) printf '%s\n' '12345 nope'; return 0 ;;
    inconsistent) printf '%s\n' '12345'; return 1 ;;
  esac
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
    [ "$TEST_TERM_BEHAVIOR" = "unknown-after-term" ] && TEST_PIDOF_MODE=error
  fi
  return 0
}
sleep() {
  TEST_SLEEP_COUNT=$((TEST_SLEEP_COUNT + 1))
  [ "$TEST_SLEEP_FAIL" != 1 ] || return 1
  if [ "$TEST_TERM_BEHAVIOR" = "after-one-poll" ] && [ "$TEST_SLEEP_COUNT" -eq 1 ]; then
    test_mark_stopped
  fi
  # Yield to background daemon mocks so the readiness marker can become visible.
  # Keep asynchronous mock startup observable under a loaded CI runner.
  command sleep 0.1
  return 0
}
kill() {
  TEST_NUMERIC_KILL=$((TEST_NUMERIC_KILL + 1))
  return 1
}

nc() {
  [ "$1" = "-w" ] && [ "$2" = "1" ] || return 2
  [ "$3" = "$TEST_EXPECT_HOST" ] && [ "$4" = "$TEST_EXPECT_PORT" ] || return 2
  printf '%s\n' "$3 $4" >> "$TEST_HTTP_LOG"
  test_probe_count=$(wc -l < "$TEST_HTTP_LOG")
  cat >/dev/null
  case "$TEST_HTTP_MODE" in
    unavailable) return 1 ;;
    delayed)
      [ "$test_probe_count" -ge "$TEST_HTTP_READY_AFTER" ] || return 1
      printf '%s\r\n' 'HTTP/1.1 401 Unauthorized'
      ;;
    ready)
      printf '%s\r\n' 'HTTP/1.1 200 OK'
      ;;
    server-error)
      printf '%s\r\n' 'HTTP/1.1 503 Service Unavailable'
      ;;
    *) return 2 ;;
  esac
  return 0
}

set -- check
. "$LAUNCHER"
set --
ALD_BIN="$TEST_DIR/mock-aliyundrive-webdav"
ALD_WORK_DIR="$TEST_DIR/aliyundrive-webdav-workdir"
TEST_UMASK_LOG="$TEST_DIR/umask.log"
export ALD_WORK_DIR TEST_UMASK_LOG
ALD_START_WAIT_SECONDS=10

# HTTP 5xx is not readiness; healthy and authentication-required status codes are.
TEST_HTTP_MODE=server-error
: > "$TEST_HTTP_LOG"
if ald_http_ready "$TEST_EXPECT_HOST" "$TEST_EXPECT_PORT"; then
  echo "FAIL: HTTP 503 was accepted as readiness" >&2
  exit 1
fi
TEST_HTTP_MODE=ready
: > "$TEST_HTTP_LOG"
ald_http_ready "$TEST_EXPECT_HOST" "$TEST_EXPECT_PORT" || {
  echo "FAIL: HTTP 200 was not accepted as readiness" >&2; exit 1;
}
TEST_HTTP_MODE=delayed
TEST_HTTP_READY_AFTER=1
: > "$TEST_HTTP_LOG"
ald_http_ready "$TEST_EXPECT_HOST" "$TEST_EXPECT_PORT" || {
  echo "FAIL: authentication-required HTTP 401 was not accepted" >&2; exit 1;
}

# The upstream daemon persists its rotated refresh token using the process
# umask. Harden existing paths before the daemon can read or replace them.
ald_secure_workdir || { echo "FAIL: secure workdir creation failed" >&2; exit 1; }
[ "$(stat -c %a "$ALD_WORK_DIR")" = 700 ] || {
  echo "FAIL: newly created workdir is not mode 0700" >&2; exit 1;
}
chmod 755 "$ALD_WORK_DIR"
printf '%s' "old-token" > "$ALD_WORK_DIR/refresh_token"
chmod 644 "$ALD_WORK_DIR/refresh_token"
ald_secure_workdir || { echo "FAIL: existing workdir hardening failed" >&2; exit 1; }
[ "$(stat -c %a "$ALD_WORK_DIR")" = 700 ] &&
[ "$(stat -c %a "$ALD_WORK_DIR/refresh_token")" = 600 ] || {
  echo "FAIL: existing token path was not reduced to private modes" >&2; exit 1;
}
ln "$ALD_WORK_DIR/refresh_token" "$TEST_DIR/token-hardlink"
if ald_secure_workdir; then
  echo "FAIL: multiply-linked refresh-token file was accepted" >&2; exit 1;
fi
[ "$(stat -c %a "$TEST_DIR/token-hardlink")" = 600 ] || {
  echo "FAIL: hardlink rejection changed the external file mode" >&2; exit 1;
}
rm -f "$TEST_DIR/token-hardlink"

# Reject directory and token symlinks without changing their targets.
rm -rf "$ALD_WORK_DIR" "$TEST_DIR/workdir-target" "$TEST_DIR/token-target"
mkdir "$TEST_DIR/workdir-target"
chmod 755 "$TEST_DIR/workdir-target"
ln -s "$TEST_DIR/workdir-target" "$ALD_WORK_DIR"
if ald_secure_workdir; then
  echo "FAIL: symlink workdir was accepted" >&2; exit 1;
fi
[ "$(stat -c %a "$TEST_DIR/workdir-target")" = 755 ] || {
  echo "FAIL: rejected symlink target was modified" >&2; exit 1;
}
rm -f "$ALD_WORK_DIR"
ald_secure_workdir || { echo "FAIL: secure workdir recreation failed" >&2; exit 1; }
printf '%s' "external-target" > "$TEST_DIR/token-target"
chmod 644 "$TEST_DIR/token-target"
ln -s "$TEST_DIR/token-target" "$ALD_WORK_DIR/refresh_token"
if ald_secure_workdir; then
  echo "FAIL: symlink refresh-token file was accepted" >&2; exit 1;
fi
[ "$(stat -c %a "$TEST_DIR/token-target")" = 644 ] || {
  echo "FAIL: rejected token symlink target was modified" >&2; exit 1;
}
rm -f "$ALD_WORK_DIR/refresh_token"
ald_secure_workdir || { echo "FAIL: workdir recovery after rejected symlink failed" >&2; exit 1; }

# The lifecycle lock is provisioned, private and not replaceable by a local user.
ald_run_locked true || { echo "FAIL: pre-provisioned lifecycle lock was rejected" >&2; exit 1; }
lock_target="$TEST_DIR/lock-target"
printf '%s\n' sentinel > "$lock_target"
ALD_LOCK_FILE="$TEST_DIR/lock-symlink"
ln -s "$lock_target" "$ALD_LOCK_FILE"
if ald_run_locked true; then
  echo "FAIL: a symbolic-link lifecycle lock path was accepted" >&2; exit 1
fi
grep -Fqx sentinel "$lock_target" || {
  echo "FAIL: rejected lifecycle lock symlink changed its target" >&2; exit 1;
}
rm -f "$ALD_LOCK_FILE"

ALD_LOCK_FILE="$TEST_DIR/lock-hardlink"
ln "$TEST_DIR/aliyundrive-webdav.lock" "$ALD_LOCK_FILE"
if ald_run_locked true; then
  echo "FAIL: a multiply-linked lifecycle lock was accepted" >&2; exit 1
fi
rm -f "$ALD_LOCK_FILE"

ALD_LOCK_FILE="$TEST_DIR/lock-insecure-mode"
cp "$TEST_DIR/aliyundrive-webdav.lock" "$ALD_LOCK_FILE"
chmod 644 "$ALD_LOCK_FILE"
if ald_run_locked true; then
  echo "FAIL: a non-private lifecycle lock file was accepted" >&2; exit 1
fi

ALD_LOCK_FILE="$TEST_DIR/missing.lock"
if ald_run_locked true; then
  echo "FAIL: a missing lifecycle lock path was recreated" >&2; exit 1
fi
[ ! -e "$ALD_LOCK_FILE" ] || {
  echo "FAIL: missing lifecycle lock was created implicitly" >&2; exit 1;
}

mkdir "$TEST_DIR/untrusted-lock-dir"
chmod 777 "$TEST_DIR/untrusted-lock-dir"
(umask 077; : > "$TEST_DIR/untrusted-lock-dir/lock")
ALD_LOCK_FILE="$TEST_DIR/untrusted-lock-dir/lock"
if ald_run_locked true; then
  echo "FAIL: an insecure lifecycle lock directory was accepted" >&2; exit 1
fi

ALD_LOCK_FILE="$TEST_DIR/aliyundrive-webdav.lock"
echo "AliyunDrive WebDAV pre-provisioned private lifecycle lock regression: PASS"

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

# pidof must distinguish confirmed absence from command errors and malformed output.
for TEST_PIDOF_MODE in error empty-success malformed inconsistent; do
  if ald_process_state; then
    echo "FAIL: pidof mode $TEST_PIDOF_MODE was accepted as a valid process state" >&2
    exit 1
  else
    TEST_STATE_STATUS=$?
  fi
  [ "$TEST_STATE_STATUS" -eq 2 ] || {
    echo "FAIL: pidof mode $TEST_PIDOF_MODE did not return UNKNOWN" >&2; exit 1;
  }
done
TEST_PIDOF_MODE=normal
TEST_RUNNING=0
TEST_ENABLE=1
rm -f "$TEST_ARG_LOG"
if start_ald; then
  echo "FAIL: pidof error was accepted as process absence during start" >&2; exit 1
fi
[ ! -e "$TEST_ARG_LOG" ] || {
  echo "FAIL: unknown process inventory allowed a duplicate launch" >&2; exit 1;
}
TEST_RUNNING=1
TEST_TERM_COUNT=0
TEST_KILL_COUNT=0
TEST_PIDOF_MODE=error
if kill_ald; then
  echo "FAIL: pidof error was accepted as process absence during stop" >&2; exit 1
fi
[ "$TEST_TERM_COUNT" -eq 0 ] && [ "$TEST_KILL_COUNT" -eq 0 ] || {
  echo "FAIL: unknown process inventory caused a signal" >&2; exit 1;
}
if ald_wait_started; then
  echo "FAIL: unknown startup inventory was accepted as readiness" >&2; exit 1
else
  TEST_STATE_STATUS=$?
fi
[ "$TEST_STATE_STATUS" -eq 2 ] || {
  echo "FAIL: unknown startup inventory did not propagate UNKNOWN" >&2; exit 1;
}
TEST_PIDOF_MODE=normal
TEST_RUNNING=0

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

# If process inventory becomes UNKNOWN after TERM, never escalate to KILL.
TEST_RUNNING=1
TEST_PIDOF_MODE=normal
TEST_TERM_BEHAVIOR=unknown-after-term
TEST_TERM_COUNT=0
TEST_KILL_COUNT=0
TEST_SLEEP_COUNT=0
if kill_ald; then
  echo "FAIL: UNKNOWN after TERM was reported as stopped" >&2; exit 1
fi
[ "$TEST_TERM_COUNT" -eq 1 ] && [ "$TEST_KILL_COUNT" -eq 0 ] || {
  echo "FAIL: UNKNOWN after TERM triggered KILL" >&2; exit 1;
}
TEST_PIDOF_MODE=normal
TEST_TERM_BEHAVIOR=stuck

# A failed sleep primitive is UNKNOWN, not proof that the daemon exited.
TEST_RUNNING=1
TEST_SLEEP_FAIL=1
TEST_TERM_COUNT=0
TEST_KILL_COUNT=0
if kill_ald; then
  echo "FAIL: sleep failure was reported as a successful stop" >&2; exit 1
fi
[ "$TEST_TERM_COUNT" -eq 1 ] && [ "$TEST_KILL_COUNT" -eq 0 ] || {
  echo "FAIL: sleep failure allowed KILL escalation" >&2; exit 1;
}
TEST_SLEEP_FAIL=0
TEST_RUNNING=0

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

# A failed checked-get is not an absent/disabled key; never stop a live daemon.
TEST_RUNNING=1
TEST_ENABLE=1
TEST_NVRAM_FAIL_KEY=aliyundrive_enable
TEST_TERM_COUNT=0
TEST_KILL_COUNT=0
if start_ald; then
  echo "FAIL: NVRAM enable read failure allowed a lifecycle transition" >&2
  exit 1
fi
[ "$TEST_TERM_COUNT" -eq 0 ] && [ "$TEST_KILL_COUNT" -eq 0 ] || {
  echo "FAIL: NVRAM enable read failure stopped the existing daemon" >&2; exit 1;
}

# Fail every enabled-start setting before workdir permissions or process launch change.
TEST_RUNNING=0
chmod 755 "$ALD_WORK_DIR"
printf '%s' "old-token" > "$ALD_WORK_DIR/refresh_token"
chmod 644 "$ALD_WORK_DIR/refresh_token"
for TEST_NVRAM_FAIL_KEY in ald_host ald_port ald_refresh_token ald_auth_user \
  ald_auth_password ald_read_buffer_size ald_cache_size ald_cache_ttl \
  ald_root ald_domain_id ald_no_trash ald_read_only; do
  rm -f "$TEST_ARG_LOG" "$TEST_RUN_MARKER"
  TEST_SLEEP_COUNT=0
  if start_ald; then
    echo "FAIL: NVRAM read failure for $TEST_NVRAM_FAIL_KEY allowed startup" >&2
    exit 1
  fi
  [ ! -e "$TEST_ARG_LOG" ] && [ ! -e "$TEST_RUN_MARKER" ] &&
  [ "$TEST_SLEEP_COUNT" -eq 0 ] &&
  [ "$(stat -c %a "$ALD_WORK_DIR")" = 755 ] &&
  [ "$(stat -c %a "$ALD_WORK_DIR/refresh_token")" = 644 ] || {
    echo "FAIL: NVRAM read failure allowed launch/workdir mutation" >&2; exit 1;
  }
done
TEST_NVRAM_FAIL_KEY=
chmod 700 "$ALD_WORK_DIR"
chmod 600 "$ALD_WORK_DIR/refresh_token"
cat > "$ALD_BIN" <<'MOCK'
#!/bin/sh
: > "$TEST_ARG_LOG"
for arg do
  printf '%s\n' "$arg" >> "$TEST_ARG_LOG"
done
{
  printf '%s\n' "${REFRESH_TOKEN-}"
  printf '%s\n' "${WEBDAV_AUTH_USER-}"
  printf '%s\n' "${WEBDAV_AUTH_PASSWORD-}"
  printf '%s\n' "${RUST_LOG-}"
} > "$TEST_ENV_LOG"
umask > "$TEST_UMASK_LOG"
: > "$ALD_WORK_DIR/umask-probe"
if [ ! -e "$TEST_START_FAIL_FILE" ]; then
  : > "$TEST_RUN_MARKER"
fi
MOCK
chmod 700 "$ALD_BIN"

# A live process without an HTTP listener must not satisfy readiness.
# Model a pre-existing PID directly; the no-listener contract is independent
# of the separate background-launch mock and its marker-file scheduling.
# The preceding disabled-stop test set this mock to stop after one poll.
TEST_TERM_BEHAVIOR=stuck
TEST_RUNNING=1
TEST_ENABLE=1
TEST_HTTP_MODE=unavailable
: > "$TEST_HTTP_LOG"
TEST_SLEEP_COUNT=0
TEST_TERM_COUNT=0
TEST_KILL_COUNT=0
rm -f "$TEST_ARG_LOG" "$TEST_RUN_MARKER" "$TEST_START_FAIL_FILE"
if start_ald; then
  echo "FAIL: process presence without an HTTP listener was reported ready" >&2
  exit 1
fi
test_http_probe_lines=$(wc -l < "$TEST_HTTP_LOG")
if [ "$test_http_probe_lines" -ne 11 ] ||
   [ "$TEST_SLEEP_COUNT" -ne "$ALD_START_WAIT_SECONDS" ] ||
   [ -e "$TEST_ARG_LOG" ] ||
   [ "$TEST_TERM_COUNT" -ne 0 ] || [ "$TEST_KILL_COUNT" -ne 0 ]; then
  echo "FAIL: HTTP timeout probes=$test_http_probe_lines sleeps=$TEST_SLEEP_COUNT expected_sleeps=$ALD_START_WAIT_SECONDS argv=$([ -e "$TEST_ARG_LOG" ] && echo present || echo absent) term=$TEST_TERM_COUNT kill=$TEST_KILL_COUNT" >&2
  exit 1
fi

# A running process whose listener appears after three probes is accepted.
TEST_TERM_BEHAVIOR=stuck
TEST_RUNNING=1
TEST_HTTP_MODE=delayed
TEST_HTTP_READY_AFTER=3
: > "$TEST_HTTP_LOG"
TEST_SLEEP_COUNT=0
TEST_TERM_COUNT=0
TEST_KILL_COUNT=0
rm -f "$TEST_ARG_LOG" "$TEST_RUN_MARKER"
start_ald || { echo "FAIL: delayed HTTP readiness did not recover" >&2; exit 1; }
[ "$(wc -l < "$TEST_HTTP_LOG")" -eq 3 ] &&
[ ! -e "$TEST_ARG_LOG" ] &&
[ "$TEST_TERM_COUNT" -eq 0 ] && [ "$TEST_KILL_COUNT" -eq 0 ] || {
  echo "FAIL: delayed HTTP readiness did not recover without a duplicate launch" >&2
  exit 1
}

# Duplicate-start preflight must not treat an unhealthy existing process as ready.
TEST_RUNNING=1
TEST_HTTP_MODE=unavailable
: > "$TEST_HTTP_LOG"
TEST_SLEEP_COUNT=0
rm -f "$TEST_ARG_LOG"
if start_ald; then
  echo "FAIL: unhealthy existing HTTP service was accepted as ready" >&2
  exit 1
fi
[ ! -e "$TEST_ARG_LOG" ] && [ "$(wc -l < "$TEST_HTTP_LOG")" -eq 11 ] || {
  echo "FAIL: unhealthy duplicate start launched a second daemon or did not wait" >&2
  exit 1
}
TEST_RUNNING=0
TEST_HTTP_MODE=ready
TEST_HTTP_READY_AFTER=1
: > "$TEST_HTTP_LOG"

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

# Credentials must travel only in the environment; other NVRAM values remain exact argv items.
rm -f "$TEST_RUN_MARKER"
TEST_RUNNING=0
TEST_ENABLE=1
TEST_HTTP_MODE=ready
: > "$TEST_HTTP_LOG"
rm -f "$TEST_START_FAIL_FILE" "$TEST_ARG_LOG" "$TEST_RUN_MARKER"
start_ald || { echo "FAIL: enabled start failed" >&2; exit 1; }
TEST_LAUNCH_PID=$!
wait "$TEST_LAUNCH_PID"
cat > "$TEST_DIR/expected-argv" <<EXPECTED
-I
--no-trash
--read-only
--host
host name *; literal
--root
/folder with spaces
-p
8080
-S
64
--cache-size
1024
--cache-ttl
300
--workdir
$ALD_WORK_DIR
EXPECTED
cmp -s "$TEST_DIR/expected-argv" "$TEST_ARG_LOG" || {
  echo "FAIL: non-secret NVRAM values were split, globbed or reordered in daemon argv" >&2
  diff -u "$TEST_DIR/expected-argv" "$TEST_ARG_LOG" >&2 || :
  exit 1
}
cat > "$TEST_DIR/expected-env" <<'EXPECTED_ENV'
refresh token with spaces *
user name
secret with spaces ; data
aliyundrive_webdav=warn
EXPECTED_ENV
cmp -s "$TEST_DIR/expected-env" "$TEST_ENV_LOG" || {
  echo "FAIL: credentials and log filter were not passed as exact environment values" >&2
  diff -u "$TEST_DIR/expected-env" "$TEST_ENV_LOG" >&2 || :
  exit 1
}
[ "$(cat "$TEST_UMASK_LOG")" = "0077" ] &&
[ "$(stat -c %a "$ALD_WORK_DIR/umask-probe")" = 600 ] || {
  echo "FAIL: daemon did not inherit the private workdir umask" >&2; exit 1;
}
[ "$(stat -c %a "$ALD_WORK_DIR")" = 700 ] || {
  echo "FAIL: daemon workdir permissions are not private" >&2; exit 1;
}
! grep -Fq "refresh token with spaces" "$TEST_ARG_LOG" &&
! grep -Fq "user name" "$TEST_ARG_LOG" &&
! grep -Fq "secret with spaces" "$TEST_ARG_LOG" || {
  echo "FAIL: a credential value leaked into daemon argv" >&2
  exit 1
}

# The special domain profile must not inherit normal-mode optional flags.
nvram() {
  [ "$1" = get_checked ] || return 1
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
TEST_EXPECT_HOST=127.0.0.1
TEST_EXPECT_PORT=8080
TEST_HTTP_MODE=ready
: > "$TEST_HTTP_LOG"
rm -f "$TEST_ARG_LOG" "$TEST_RUN_MARKER"
start_ald || { echo "FAIL: special-domain start failed" >&2; exit 1; }
TEST_LAUNCH_PID=$!
wait "$TEST_LAUNCH_PID"
cat > "$TEST_DIR/expected-special-env" <<'SPECIAL_ENV'
token
user
pass
aliyundrive_webdav=warn
SPECIAL_ENV
grep -Fqx -- '--domain-id' "$TEST_ARG_LOG" &&
grep -Fqx -- '99999' "$TEST_ARG_LOG" &&
! grep -Fqx -- '--no-trash' "$TEST_ARG_LOG" &&
! grep -Fqx -- '--read-only' "$TEST_ARG_LOG" &&
cmp -s "$TEST_DIR/expected-special-env" "$TEST_ENV_LOG" &&
! grep -Fqx -- 'token' "$TEST_ARG_LOG" &&
! grep -Fqx -- 'user' "$TEST_ARG_LOG" &&
! grep -Fqx -- 'pass' "$TEST_ARG_LOG" || {
  echo "FAIL: special-domain options differ from the existing policy" >&2; exit 1;
}

echo "AliyunDrive WebDAV private token workdir and umask regression: PASS"
echo "AliyunDrive WebDAV bounded stop and process-name signaling regression: PASS"
echo "AliyunDrive WebDAV duplicate-start guard and argv integrity regression: PASS"
echo "AliyunDrive WebDAV serialized lifecycle lock and startup readiness regression: PASS"
