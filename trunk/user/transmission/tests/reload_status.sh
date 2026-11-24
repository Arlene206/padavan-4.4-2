#!/bin/sh
set -eu

TEST_DIR=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
SCRIPT="$TEST_DIR/../transmission.sh"
TMP_DIR=$(mktemp -d "${TMPDIR:-/tmp}/transmission-reload.XXXXXX")
trap 'rm -rf "$TMP_DIR"' 0
trap 'exit 1' HUP INT TERM

fail() {
  echo "Transmission reload PID snapshot regression: FAIL: $*" >&2
  exit 1
}

sed -n '/^func_reload()/,/^}/p' "$SCRIPT" > "$TMP_DIR/function.sh"
[ -s "$TMP_DIR/function.sh" ] || fail "could not extract func_reload"
. "$TMP_DIR/function.sh"
SVC_NAME="Transmission"

mkdir -p "$TMP_DIR/bin"
cat > "$TMP_DIR/bin/pidof" <<'MOCK_PIDOF'
#!/bin/sh
printf '%s\n' "${MOCK_PIDOF_OUTPUT:-}"
exit "${MOCK_PIDOF_STATUS:-1}"
MOCK_PIDOF
cat > "$TMP_DIR/bin/killall" <<'MOCK_KILLALL'
#!/bin/sh
printf '%s\n' "$*" >> "$MOCK_KILLALL_LOG"
exit 0
MOCK_KILLALL
chmod 755 "$TMP_DIR/bin/pidof" "$TMP_DIR/bin/killall"
PATH="$TMP_DIR/bin:$PATH"
export PATH
MOCK_SIGNAL_LOG="$TMP_DIR/signals.log"
MOCK_KILLALL_LOG="$TMP_DIR/killall.log"
kill() {
  printf '%s %s\n' "$1" "$2" >> "$MOCK_SIGNAL_LOG"
  return "${MOCK_KILL_STATUS:-0}"
}
export MOCK_SIGNAL_LOG MOCK_KILLALL_LOG
logger() { :; }

# A cleanly absent service is a successful no-op and must not signal anything.
MOCK_PIDOF_STATUS=1
MOCK_PIDOF_OUTPUT=""
export MOCK_PIDOF_STATUS MOCK_PIDOF_OUTPUT
: > "$MOCK_SIGNAL_LOG"
: > "$MOCK_KILLALL_LOG"
func_reload
[ ! -s "$MOCK_SIGNAL_LOG" ] && [ ! -s "$MOCK_KILLALL_LOG" ] || {
  fail "reload signalled when Transmission was absent"
}

# An inventory error or malformed PID output is UNKNOWN, not ABSENT.
MOCK_PIDOF_STATUS=2
MOCK_PIDOF_OUTPUT=""
export MOCK_PIDOF_STATUS MOCK_PIDOF_OUTPUT
if func_reload >/dev/null; then
  fail "reload accepted an unreadable process inventory"
fi
[ ! -s "$MOCK_SIGNAL_LOG" ] && [ ! -s "$MOCK_KILLALL_LOG" ] || {
  fail "reload signalled after a process inventory error"
}
for bad_output in '' '123 invalid' '0' '1' '123\n456'; do
  MOCK_PIDOF_STATUS=0
  MOCK_PIDOF_OUTPUT="$bad_output"
  export MOCK_PIDOF_STATUS MOCK_PIDOF_OUTPUT
  if func_reload >/dev/null; then
    fail "reload accepted malformed pidof output '$bad_output'"
  fi
  [ ! -s "$MOCK_SIGNAL_LOG" ] && [ ! -s "$MOCK_KILLALL_LOG" ] || {
    fail "reload signalled after malformed pidof output '$bad_output'"
  }
done

# Signal every PID from the validated snapshot and preserve the first error.
MOCK_PIDOF_STATUS=0
MOCK_PIDOF_OUTPUT='123 456'
MOCK_KILL_STATUS=29
export MOCK_PIDOF_STATUS MOCK_PIDOF_OUTPUT MOCK_KILL_STATUS
: > "$MOCK_SIGNAL_LOG"
: > "$MOCK_KILLALL_LOG"
if func_reload > "$TMP_DIR/failed.out"; then
  fail "reload hid a failed SIGHUP operation"
else
  reload_status=$?
fi
[ "$reload_status" -eq 29 ] || {
  fail "reload returned $reload_status instead of signal status 29"
}
grep -Fq 'Reload Transmission config:.[FAILED]' "$TMP_DIR/failed.out" || {
  fail "failed reload did not report [FAILED]"
}
[ "$(cat "$MOCK_SIGNAL_LOG")" = "$(printf '%s\n' '-HUP 123' '-HUP 456')" ] || {
  fail "failed reload did not signal exactly the validated PID list"
}
[ ! -s "$MOCK_KILLALL_LOG" ] || {
  fail "failed reload used a process-name-wide killall"
}

# A successful reload keeps the original success presentation.
MOCK_KILL_STATUS=0
export MOCK_KILL_STATUS
: > "$MOCK_SIGNAL_LOG"
: > "$MOCK_KILLALL_LOG"
func_reload > "$TMP_DIR/success.out"
grep -Fq 'Reload Transmission config:.[  OK  ]' "$TMP_DIR/success.out" || {
  fail "successful reload did not report [  OK  ]"
}
[ "$(cat "$MOCK_SIGNAL_LOG")" = "$(printf '%s\n' '-HUP 123' '-HUP 456')" ] || {
  fail "successful reload did not signal the validated PID list"
}
[ ! -s "$MOCK_KILLALL_LOG" ] || {
  fail "successful reload used a process-name-wide killall"
}

echo "Transmission reload status and PID snapshot regression: PASS"
