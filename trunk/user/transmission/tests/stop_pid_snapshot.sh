#!/bin/sh
set -eu
TEST_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
SCRIPT="$TEST_DIR/../transmission.sh"
TMP_DIR=$(mktemp -d "${TMPDIR:-/tmp}/transmission-stop-pids.XXXXXX")
trap 'rm -rf "$TMP_DIR"' 0
trap 'exit 1' HUP INT TERM
fail() { echo "Transmission stop process-identity regression: FAIL: $*" >&2; exit 1; }
sed '/^case "$1" in/,$d' "$SCRIPT" > "$TMP_DIR/functions.sh"
[ -s "$TMP_DIR/functions.sh" ] || fail "could not extract lifecycle functions"
. "$TMP_DIR/functions.sh"
TEST_PROC_ROOT="$TMP_DIR/proc"
TEST_BIN="$TMP_DIR/bin/transmission-daemon"
TEST_SPOOF_BIN="/bin/sleep"
SVC_PATH="$TEST_BIN"
TRANSMISSION_PROC_ROOT="$TEST_PROC_ROOT"
export SVC_PATH TRANSMISSION_PROC_ROOT
mkdir -p "$TEST_PROC_ROOT" "$TMP_DIR/bin"
: > "$TEST_BIN"; chmod 755 "$TEST_BIN"
EVENTS="$TMP_DIR/events"; : > "$EVENTS"
TEST_TERM_STUCK=0; TEST_SIGNAL_FAILURE=0
logger() { :; }
sleep() { :; }
kill() {
  [ "$#" -eq 2 ] || return 1
  printf '%s %s\n' "$1" "$2" >> "$EVENTS"
  case "$1" in
    -TERM) [ "$TEST_SIGNAL_FAILURE" -eq 0 ] || return 77; if [ "$TEST_TERM_STUCK" -eq 0 ]; then rm -rf "$TEST_PROC_ROOT/$2"; fi ;;
    -9) rm -rf "$TEST_PROC_ROOT/$2" ;;
    *) return 1 ;;
  esac
  return 0
}
make_proc() {
  pid=$1; comm=$2; exe=$3; start=$4
  mkdir -p "$TEST_PROC_ROOT/$pid"
  ln -s "$exe" "$TEST_PROC_ROOT/$pid/exe"
  printf '%s (%s) S 1 1 1 0 -1 0 0 0 0 0 0 0 0 0 0 0 1 0 %s\n' "$pid" "$comm" "$start" > "$TEST_PROC_ROOT/$pid/stat"
}
reset_state() {
  rm -rf "$TEST_PROC_ROOT"; mkdir -p "$TEST_PROC_ROOT"
  : > "$EVENTS"; TEST_TERM_STUCK=0; TEST_SIGNAL_FAILURE=0
  make_proc 1 init "$TEST_SPOOF_BIN" 1001
}
reset_state
make_proc 123 transmission-daemon "$TEST_BIN" 2001
make_proc 456 transmission-daemon "$TEST_SPOOF_BIN" 2002
if transmission_process_state; then state=0; else state=$?; fi
[ "$state" -eq 0 ] || fail "exact executable was not inventoried (state=$state)"
[ "$transmission_process_pids" = "123:2001" ] || fail "spoofed executable entered snapshot: $transmission_process_pids"
func_start > "$TMP_DIR/already-running.out" || fail "start did not recognize the exact running daemon"
[ ! -s "$EVENTS" ] || fail "start signalled an existing daemon"
func_stop > "$TMP_DIR/graceful.out" || fail "graceful stop failed"
[ "$(cat "$EVENTS")" = "-TERM 123" ] || fail "unexpected signals: $(cat "$EVENTS")"
[ -d "$TEST_PROC_ROOT/456" ] || fail "comm-spoofed process was removed"
echo "Transmission stop signals exact executable/start-time snapshot: PASS"
reset_state
make_proc 456 transmission-daemon "$TEST_SPOOF_BIN" 3001
func_stop > "$TMP_DIR/spoof.out" || fail "spoof-only state should be a clean no-op"
[ ! -s "$EVENTS" ] || fail "signal sent to comm-spoofed process"
echo "Transmission process-name spoof rejection: PASS"
reset_state
make_proc 123 transmission-daemon "$TEST_BIN" 4001
if transmission_process_identity_matches 123 3999; then fail "stale start time was accepted"; else status=$?; fi
[ "$status" -eq 1 ] || fail "PID reuse should be an identity mismatch"
transmission_signal_identity HUP 123 3999 || fail "stale snapshot should be safely skipped"
[ ! -s "$EVENTS" ] || fail "stale PID snapshot signalled a replacement process"
echo "Transmission PID reuse/start-time fencing: PASS"
reset_state
make_proc 123 transmission-daemon "$TEST_BIN" 5001
TEST_TERM_STUCK=1
func_stop > "$TMP_DIR/timeout.out" || fail "SIGKILL fallback did not confirm process absence"
[ "$(cat "$EVENTS")" = "$(printf '%s\n' '-TERM 123' '-9 123')" ] || fail "unexpected TERM/KILL sequence: $(cat "$EVENTS")"
echo "Transmission bounded TERM/KILL identity revalidation: PASS"
reset_state
make_proc 123 transmission-daemon "$TEST_BIN" 6001
TEST_SIGNAL_FAILURE=1
if func_stop > "$TMP_DIR/signal-failure.out"; then fail "failed TERM was reported as success"; else status=$?; fi
[ "$status" -eq 77 ] || fail "TERM failure status was not propagated ($status)"
[ -d "$TEST_PROC_ROOT/123" ] || fail "fixture unexpectedly disappeared after failed TERM"
[ "$(cat "$EVENTS")" = "-TERM 123" ] || fail "unexpected signal after failed TERM"
reset_state
if transmission_process_state; then state=0; else state=$?; fi
[ "$state" -eq 1 ] || fail "clean process inventory should be ABSENT ($state)"
TRANSMISSION_PROC_ROOT="$TMP_DIR/missing-proc"; export TRANSMISSION_PROC_ROOT
if func_stop > "$TMP_DIR/missing-proc.out"; then fail "stop succeeded with missing procfs"; else status=$?; fi
[ "$status" -eq 2 ] || fail "missing procfs should fail closed ($status)"
[ ! -s "$EVENTS" ] || fail "signal sent with missing procfs"
TRANSMISSION_PROC_ROOT="$TEST_PROC_ROOT"; export TRANSMISSION_PROC_ROOT
reset_state
mkdir -p "$TEST_PROC_ROOT/701"
printf 'malformed process stat\n' > "$TEST_PROC_ROOT/701/stat"
if transmission_process_state; then state=0; else state=$?; fi
[ "$state" -eq 2 ] || fail "malformed procfs entry should be UNKNOWN ($state)"
if func_start > "$TMP_DIR/malformed-start.out"; then fail "start accepted an UNKNOWN process inventory"; else status=$?; fi
[ "$status" -eq 2 ] || fail "start should fail closed when procfs is malformed ($status)"
if func_stop > "$TMP_DIR/malformed.out"; then fail "stop succeeded with malformed procfs"; else status=$?; fi
[ "$status" -eq 2 ] || fail "malformed procfs should fail closed ($status)"
[ ! -s "$EVENTS" ] || fail "signal sent with malformed procfs"
echo "Transmission incomplete procfs inventory fails closed: PASS"
if grep -F 'pidof transmission-daemon' "$SCRIPT" >/dev/null 2>&1; then fail "pidof process-name lookup remains"; fi
if grep -F 'killall' "$SCRIPT" >/dev/null 2>&1; then fail "process-name-wide signaling remains"; fi
echo "Transmission PID-only lifecycle control removed: PASS"
