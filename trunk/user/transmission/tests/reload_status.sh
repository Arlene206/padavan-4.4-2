#!/bin/sh
set -eu
TEST_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
SCRIPT="$TEST_DIR/../transmission.sh"
TMP_DIR=$(mktemp -d "${TMPDIR:-/tmp}/transmission-reload.XXXXXX")
trap 'rm -rf "$TMP_DIR"' 0
trap 'exit 1' HUP INT TERM
fail() { echo "Transmission reload process-identity regression: FAIL: $*" >&2; exit 1; }
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
MOCK_SIGNAL_STATUS=0
logger() { :; }
kill() {
  [ "$#" -eq 2 ] || return 1
  printf '%s %s\n' "$1" "$2" >> "$EVENTS"
  return "$MOCK_SIGNAL_STATUS"
}
make_proc() {
  pid=$1; comm=$2; exe=$3; start=$4
  mkdir -p "$TEST_PROC_ROOT/$pid"
  ln -s "$exe" "$TEST_PROC_ROOT/$pid/exe"
  printf '%s (%s) S 1 1 1 0 -1 0 0 0 0 0 0 0 0 0 0 0 1 0 %s\n' "$pid" "$comm" "$start" > "$TEST_PROC_ROOT/$pid/stat"
}
reset_state() {
  rm -rf "$TEST_PROC_ROOT"; mkdir -p "$TEST_PROC_ROOT"
  : > "$EVENTS"; MOCK_SIGNAL_STATUS=0
  make_proc 1 init "$TEST_SPOOF_BIN" 1001
}
reset_state
func_reload > "$TMP_DIR/absent.out"
[ ! -s "$EVENTS" ] || fail "reload signalled when Transmission was absent"
reset_state
make_proc 456 transmission-daemon "$TEST_SPOOF_BIN" 2001
func_reload > "$TMP_DIR/spoof.out"
[ ! -s "$EVENTS" ] || fail "reload signalled a process that only spoofed the comm name"
echo "Transmission reload rejects process-name spoofing: PASS"
reset_state
make_proc 123 transmission-daemon "$TEST_BIN" 3001
make_proc 456 transmission-daemon "$TEST_BIN" 3002
MOCK_SIGNAL_STATUS=29
if func_reload > "$TMP_DIR/failed.out"; then fail "reload hid failed SIGHUP operations"; else reload_status=$?; fi
[ "$reload_status" -eq 29 ] || fail "reload returned $reload_status instead of first signal status 29"
grep -Fq 'Reload Transmission config:.[FAILED]' "$TMP_DIR/failed.out" || fail "failed reload did not report [FAILED]"
[ "$(cat "$EVENTS")" = "$(printf '%s\n' '-HUP 123' '-HUP 456')" ] || fail "failed reload did not signal exact snapshot: $(cat "$EVENTS")"
MOCK_SIGNAL_STATUS=0
: > "$EVENTS"
func_reload > "$TMP_DIR/success.out"
grep -Fq 'Reload Transmission config:.[  OK  ]' "$TMP_DIR/success.out" || fail "successful reload did not report [  OK  ]"
[ "$(cat "$EVENTS")" = "$(printf '%s\n' '-HUP 123' '-HUP 456')" ] || fail "successful reload did not signal exact snapshot"
reset_state
make_proc 123 transmission-daemon "$TEST_BIN" 4001
transmission_signal_identity HUP 123 3999 || fail "stale identity should be safely skipped"
[ ! -s "$EVENTS" ] || fail "reload signalled a PID whose start time changed"
echo "Transmission reload is fenced against PID reuse: PASS"
reset_state
mkdir -p "$TEST_PROC_ROOT/501"
printf 'malformed process stat\n' > "$TEST_PROC_ROOT/501/stat"
if func_reload > "$TMP_DIR/unknown.out"; then fail "reload accepted incomplete procfs inventory"; else status=$?; fi
[ "$status" -eq 2 ] || fail "unknown inventory should return status 2 ($status)"
[ ! -s "$EVENTS" ] || fail "reload signalled after unknown process inventory"
reset_state
make_proc 601 transmission-daemon "$TEST_BIN" 6001
rm -f "$TEST_PROC_ROOT/601/exe"
ln -s "$TMP_DIR/missing-executable" "$TEST_PROC_ROOT/601/exe"
if func_reload > "$TMP_DIR/missing-exe.out"; then fail "reload accepted unreadable target executable"; else status=$?; fi
[ "$status" -eq 2 ] || fail "missing executable identity should be UNKNOWN ($status)"
[ ! -s "$EVENTS" ] || fail "reload signalled with incomplete executable identity"
echo "Transmission incomplete process identity fails closed: PASS"
