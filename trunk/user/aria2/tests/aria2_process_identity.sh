#!/bin/sh
set -eu
TEST_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
SCRIPT="$TEST_DIR/../aria.sh"
TMP_DIR=$(mktemp -d "${TMPDIR:-/tmp}/aria2-process-identity.XXXXXX")
trap 'rm -rf "$TMP_DIR"' 0
trap 'exit 1' HUP INT TERM
fail() { echo "Aria2 process-identity regression: FAIL: $*" >&2; exit 1; }
sed '/^case "$1" in/,$d' "$SCRIPT" > "$TMP_DIR/functions.sh"
[ -s "$TMP_DIR/functions.sh" ] || fail "could not extract production lifecycle functions"
. "$TMP_DIR/functions.sh"
TEST_PROC_ROOT="$TMP_DIR/proc"
TEST_BIN="$TMP_DIR/bin/aria2c"
TEST_SPOOF_BIN="/bin/sleep"
SVC_PATH="$TEST_BIN"
ARIA_PROC_ROOT="$TEST_PROC_ROOT"
export SVC_PATH ARIA_PROC_ROOT
mkdir -p "$TEST_PROC_ROOT" "$TMP_DIR/bin"
: > "$TEST_BIN"
chmod 755 "$TEST_BIN"
EVENTS="$TMP_DIR/events"
TEST_START_LOG="$TMP_DIR/start.log"
: > "$EVENTS"; : > "$TEST_START_LOG"
cat > "$TMP_DIR/bin/start-stop-daemon" <<'EOF'
#!/bin/sh
printf '%s\n' launch >> "$TEST_START_LOG"
exit 0
EOF
chmod 755 "$TMP_DIR/bin/start-stop-daemon"
PATH="$TMP_DIR/bin:$PATH"
export PATH TEST_START_LOG
TEST_TERM_STUCK=0
TEST_SIGNAL_STATUS=0
logger() { :; }
sleep() { :; }
make_proc() {
  pid=$1; comm=$2; exe=$3; start=$4
  mkdir -p "$TEST_PROC_ROOT/$pid"
  ln -s "$exe" "$TEST_PROC_ROOT/$pid/exe"
  printf '%s (%s) S 1 1 1 0 -1 0 0 0 0 0 0 0 0 0 0 0 1 0 %s\n' "$pid" "$comm" "$start" > "$TEST_PROC_ROOT/$pid/stat"
}
reset_state() {
  rm -rf "$TEST_PROC_ROOT"; mkdir -p "$TEST_PROC_ROOT"
  : > "$EVENTS"; : > "$TEST_START_LOG"
  TEST_TERM_STUCK=0; TEST_SIGNAL_STATUS=0
  make_proc 1 init "$TEST_SPOOF_BIN" 1001
}
kill() {
  [ "$#" -eq 2 ] || return 1
  printf '%s %s\n' "$1" "$2" >> "$EVENTS"
  [ "$TEST_SIGNAL_STATUS" -eq 0 ] || return "$TEST_SIGNAL_STATUS"
  case "$1" in
    -TERM) [ "$TEST_TERM_STUCK" -eq 1 ] || rm -rf "$TEST_PROC_ROOT/$2" ;;
    -9) rm -rf "$TEST_PROC_ROOT/$2" ;;
    -HUP) ;;
    *) return 1 ;;
  esac
  return 0
}
reset_state
make_proc 123 aria2c "$TEST_BIN" 2001
make_proc 456 aria2c "$TEST_SPOOF_BIN" 2002
if aria_process_state; then state=0; else state=$?; fi
[ "$state" -eq 0 ] || fail "installed aria2c executable was not inventoried ($state)"
[ "$aria_process_pids" = "123:2001" ] || fail "spoofed executable entered snapshot: $aria_process_pids"
func_start > "$TMP_DIR/already-running.out" || fail "start did not recognize existing daemon"
[ ! -s "$TEST_START_LOG" ] || fail "start launched a duplicate daemon"
[ ! -s "$EVENTS" ] || fail "start signalled an existing daemon"
echo "Aria2 start accepts exact executable and avoids duplicate launch: PASS"
reset_state
make_proc 456 aria2c "$TEST_SPOOF_BIN" 3001
if aria_process_state; then state=0; else state=$?; fi
[ "$state" -eq 1 ] || fail "comm-name spoof should be ABSENT ($state)"
func_stop > "$TMP_DIR/spoof-stop.out" || fail "spoof-only stop should be a no-op"
[ ! -s "$EVENTS" ] || fail "spoofed executable received a signal"
echo "Aria2 rejects process-name-only spoofing: PASS"
reset_state
make_proc 123 aria2c "$TEST_BIN" 4001
if aria_process_identity_matches 123 3999; then fail "stale start time was accepted"; else status=$?; fi
[ "$status" -eq 1 ] || fail "PID reuse should be an identity mismatch"
aria_signal_identity HUP 123 3999 || fail "stale snapshot should be skipped"
[ ! -s "$EVENTS" ] || fail "stale snapshot signalled replacement process"
echo "Aria2 PID reuse/start-time fencing: PASS"
reset_state
make_proc 123 aria2c "$TEST_BIN" 5001
func_stop > "$TMP_DIR/graceful.out" || fail "graceful stop failed"
[ "$(cat "$EVENTS")" = "-TERM 123" ] || fail "unexpected graceful-stop signals"
[ ! -d "$TEST_PROC_ROOT/123" ] || fail "graceful stop left daemon"
echo "Aria2 graceful stop signals exact process snapshot: PASS"
reset_state
make_proc 123 aria2c "$TEST_BIN" 6001
TEST_TERM_STUCK=1
func_stop > "$TMP_DIR/forced.out" || fail "SIGKILL fallback failed"
[ "$(cat "$EVENTS")" = "$(printf '%s\n' '-TERM 123' '-9 123')" ] || fail "unexpected TERM/KILL sequence"
echo "Aria2 bounded TERM/KILL escalation revalidates identity: PASS"
reset_state
make_proc 123 aria2c "$TEST_BIN" 7001
TEST_SIGNAL_STATUS=77
if func_stop > "$TMP_DIR/signal-failure.out"; then fail "failed TERM was reported as success"; else status=$?; fi
[ "$status" -eq 77 ] || fail "TERM failure status not propagated"
[ -d "$TEST_PROC_ROOT/123" ] || fail "daemon disappeared after failed TERM"
echo "Aria2 stop propagates signal failures: PASS"
reset_state
make_proc 123 aria2c "$TEST_BIN" 8001
make_proc 124 aria2c "$TEST_BIN" 8002
func_reload > "$TMP_DIR/reload.out" || fail "successful reload failed"
[ "$(cat "$EVENTS")" = "$(printf '%s\n' '-HUP 123' '-HUP 124')" ] || fail "reload signal snapshot mismatch"
echo "Aria2 reload signals exact executable/start-time identities: PASS"
reset_state
if func_reload > "$TMP_DIR/reload-absent.out"; then fail "reload reported success when daemon was absent"; else status=$?; fi
[ "$status" -eq 1 ] || fail "absent reload should preserve status 1 ($status)"
[ ! -s "$EVENTS" ] || fail "reload signalled with no daemon"
echo "Aria2 reload reports an absent daemon without signalling: PASS"
reset_state
make_proc 123 aria2c "$TEST_BIN" 9001
TEST_SIGNAL_STATUS=29
if func_reload > "$TMP_DIR/reload-failure.out"; then fail "reload hid SIGHUP failure"; else status=$?; fi
[ "$status" -eq 29 ] || fail "reload did not preserve SIGHUP status"
echo "Aria2 reload propagates signal failures: PASS"
reset_state
if aria_process_state; then state=0; else state=$?; fi
[ "$state" -eq 1 ] || fail "empty process inventory should be ABSENT ($state)"
ARIA_PROC_ROOT="$TMP_DIR/missing-proc"; export ARIA_PROC_ROOT
if func_stop > "$TMP_DIR/missing-proc.out"; then fail "stop succeeded with missing procfs"; else status=$?; fi
[ "$status" -eq 2 ] || fail "missing procfs should fail closed ($status)"
[ ! -s "$EVENTS" ] || fail "signal sent with missing procfs"
ARIA_PROC_ROOT="$TEST_PROC_ROOT"; export ARIA_PROC_ROOT
echo "Aria2 missing procfs fails closed: PASS"
reset_state
mkdir -p "$TEST_PROC_ROOT/702"
ln -s "$TMP_DIR/no-such-aria2-binary" "$TEST_PROC_ROOT/702/exe"
printf '%s (%s) S 1 1 1 0 -1 0 0 0 0 0 0 0 0 0 0 0 1 0 7021\n' 702 aria2c > "$TEST_PROC_ROOT/702/stat"
if aria_process_state; then state=0; else state=$?; fi
[ "$state" -eq 2 ] || fail "unreadable matching executable should be UNKNOWN ($state)"
if func_stop > "$TMP_DIR/missing-exe-stop.out"; then fail "stop accepted an incomplete executable identity"; else status=$?; fi
[ "$status" -eq 2 ] || fail "incomplete executable identity should fail closed ($status)"
[ ! -s "$EVENTS" ] || fail "signal sent with incomplete executable identity"
echo "Aria2 incomplete executable identity fails closed: PASS"
reset_state
mkdir -p "$TEST_PROC_ROOT/701"
printf '%s\n' 'malformed process stat' > "$TEST_PROC_ROOT/701/stat"
if aria_process_state; then state=0; else state=$?; fi
[ "$state" -eq 2 ] || fail "malformed procfs entry should be UNKNOWN ($state)"
if func_start > "$TMP_DIR/malformed-start.out"; then fail "start accepted UNKNOWN process inventory"; else status=$?; fi
[ "$status" -eq 2 ] || fail "start should fail closed ($status)"
if func_stop > "$TMP_DIR/malformed-stop.out"; then fail "stop accepted UNKNOWN process inventory"; else status=$?; fi
[ "$status" -eq 2 ] || fail "stop should fail closed ($status)"
if func_reload > "$TMP_DIR/malformed-reload.out"; then fail "reload accepted UNKNOWN process inventory"; else status=$?; fi
[ "$status" -eq 2 ] || fail "reload should fail closed ($status)"
[ ! -s "$EVENTS" ] || fail "signal occurred with malformed procfs"
[ ! -s "$TEST_START_LOG" ] || fail "start launched with malformed procfs"
echo "Aria2 malformed procfs fails closed for all lifecycle operations: PASS"
if grep -F 'pidof aria2c' "$SCRIPT" >/dev/null 2>&1; then fail "pidof process-name lookup remains"; fi
if grep -F 'killall -q aria2c' "$SCRIPT" >/dev/null 2>&1; then fail "process-name-wide signaling remains"; fi
echo "Aria2 process-name-only lifecycle control removed: PASS"
