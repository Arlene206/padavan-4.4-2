#!/bin/sh
set -eu

TEST_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
SCRIPT="$TEST_DIR/../transmission.sh"
TMP_DIR=$(mktemp -d "${TMPDIR:-/tmp}/transmission-stop-pids.XXXXXX")
trap 'rm -rf "$TMP_DIR"' 0
trap 'exit 1' HUP INT TERM

fail() {
  echo "Transmission stop PID snapshot regression: FAIL: $*" >&2
  exit 1
}

sed -n '/^func_stop()/,/^}/p' "$SCRIPT" > "$TMP_DIR/function.sh"
[ -s "$TMP_DIR/function.sh" ] || fail "could not extract func_stop"
. "$TMP_DIR/function.sh"
SVC_NAME="Transmission"
EVENTS="$TMP_DIR/events"
: > "$EVENTS"

MOCK_PIDOF_MODE=graceful
MOCK_PIDOF_CALLS=0
pidof() {
  MOCK_PIDOF_CALLS=$((MOCK_PIDOF_CALLS + 1))
  case "$MOCK_PIDOF_MODE" in
    graceful)
      if [ "$MOCK_PIDOF_CALLS" -eq 1 ]; then
        printf '%s\n' '123 456'
        return 0
      fi
      return 1
      ;;
    timeout)
      if [ "$MOCK_PIDOF_CALLS" -le 27 ]; then
        printf '%s\n' '123'
        return 0
      fi
      return 1
      ;;
    malformed)
      printf '%s\n' '123 invalid'
      return 0
      ;;
    unknown)
      return 2
      ;;
  esac
  return 2
}
kill() {
  printf 'kill %s %s\n' "$1" "$2" >> "$EVENTS"
  return 0
}
killall() {
  printf 'killall %s\n' "$*" >> "$EVENTS"
  return 0
}
sleep() { :; }
logger() { :; }

# The graceful path must signal every and only PID in the validated snapshot.
func_stop > "$TMP_DIR/graceful.out" || fail "graceful stop failed"
grep -Fxq 'kill -TERM 123' "$EVENTS" || fail "PID 123 did not receive SIGTERM"
grep -Fxq 'kill -TERM 456' "$EVENTS" || fail "PID 456 did not receive SIGTERM"
[ "$(grep -c '^kill ' "$EVENTS")" -eq 2 ] || fail "unexpected process signal on graceful stop"
if grep -q '^killall ' "$EVENTS"; then
  fail "stop rescanned by process name instead of using the validated PID snapshot"
fi

# An invalid or unknown initial process inventory must not signal anything.
: > "$EVENTS"
MOCK_PIDOF_CALLS=0
MOCK_PIDOF_MODE=malformed
if func_stop >/dev/null 2>&1; then
  fail "malformed PID output was accepted"
fi
[ ! -s "$EVENTS" ] || fail "malformed PID output caused a process signal"

MOCK_PIDOF_CALLS=0
MOCK_PIDOF_MODE=unknown
if func_stop >/dev/null 2>&1; then
  fail "unknown PID inventory was accepted"
fi
[ ! -s "$EVENTS" ] || fail "unknown PID inventory caused a process signal"

# The forced-stop fallback also targets the validated refreshed PID list only.
: > "$EVENTS"
MOCK_PIDOF_CALLS=0
MOCK_PIDOF_MODE=timeout
func_stop > "$TMP_DIR/timeout.out" || fail "SIGKILL fallback did not verify daemon exit"
grep -Fxq 'kill -TERM 123' "$EVENTS" || fail "timeout path omitted graceful SIGTERM"
grep -Fxq 'kill -9 123' "$EVENTS" || fail "timeout path omitted per-PID SIGKILL"
[ "$(grep -c '^kill ' "$EVENTS")" -eq 2 ] || fail "timeout path signalled an unexpected PID"
if grep -q '^killall ' "$EVENTS"; then
  fail "timeout path used a process-name-wide signal"
fi

echo "Transmission stop signals target only validated PID snapshots: PASS"
