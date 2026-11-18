#!/bin/bash
set -euo pipefail

ROOT=$(cd -- "$(dirname -- "$0")/../.." && pwd)
TEST_TMP=$(mktemp -d "${TMPDIR:-/tmp}/redsocks-lifecycle.XXXXXX")
trap 'rm -rf "$TEST_TMP"' EXIT HUP INT TERM
TEST_STATE="$TEST_TMP/state"
TEST_SIGNALS="$TEST_TMP/signals"
TEST_SLEEP_LOG="$TEST_TMP/sleep.log"
TEST_START_LOG="$TEST_TMP/start.log"
TEST_LOG="$TEST_TMP/launcher.log"
export TEST_STATE TEST_SIGNALS TEST_SLEEP_LOG TEST_START_LOG TEST_LOG

BINARY_NAME=redsocks
REDSOCKS_DIR="$TEST_TMP"
REDSOCKS_CONF="$TEST_TMP/redsocks.conf"
BINARY_PATH="$TEST_TMP/redsocks"
touch "$REDSOCKS_CONF"

logger() { printf '%s\n' "$*" >> "$TEST_LOG"; }
pidof() {
  [[ "$1" == "$BINARY_NAME" ]] || return 2
  case "$(cat "$TEST_STATE")" in
    running|stuck) echo 12345; return 0 ;;
    error) return 2 ;;
    malformed) echo "12345 nope"; return 0 ;;
    empty-success) return 0 ;;
    inconsistent) echo 12345; return 1 ;;
    *) return 1 ;;
  esac
}
killall() {
  printf '%s\n' "$*" >> "$TEST_SIGNALS"
  if [[ "$1" == -q && "$2" == -9 ]]; then
    [[ "${TEST_KILL_MODE:-stop}" == stop ]] && echo absent > "$TEST_STATE"
  else
    [[ "${TEST_TERM_MODE:-stop}" == stop ]] && echo absent > "$TEST_STATE"
    [[ "${TEST_TERM_MODE:-stop}" == unknown ]] && echo error > "$TEST_STATE"
  fi
  return 0
}
sleep() {
  printf '%s\n' "$*" >> "$TEST_SLEEP_LOG"
  [[ "${TEST_SLEEP_FAIL:-0}" == 0 ]] || return 1
  sleep_count=$(wc -l < "$TEST_SLEEP_LOG")
  if [[ "${TEST_READY_AFTER_SLEEP:-0}" -gt 0 && "$sleep_count" -ge "${TEST_READY_AFTER_SLEEP}" ]]; then
    echo running > "$TEST_STATE"
  fi
  return 0
}
func_redsocks() {
  echo start >> "$TEST_START_LOG"
  launch_status="${TEST_LAUNCH_STATUS:-0}"
  [[ "$launch_status" == 0 ]] || return "$launch_status"
  case "${TEST_LAUNCH_STATE:-running}" in
    running) echo running > "$TEST_STATE" ;;
    unknown) echo error > "$TEST_STATE" ;;
    *) echo absent > "$TEST_STATE" ;;
  esac
  return 0
}
func_save() { :; }

# Source the complete production script with its explicit test-only
# dispatcher guard; this keeps heredoc-containing functions intact.
set -- check nolog test
RED_SOCKS_TEST_MODE=1
export RED_SOCKS_TEST_MODE
. "$ROOT/trunk/user/redsocks/redsocks.sh"
BINARY_NAME=redsocks
REDSOCKS_DIR="$TEST_TMP"
REDSOCKS_CONF="$TEST_TMP/redsocks.conf"
BINARY_PATH="$TEST_TMP/redsocks"
touch "$REDSOCKS_CONF"
# Reinstall launch/save stubs after sourcing, which defines production versions.
func_redsocks() {
  echo start >> "$TEST_START_LOG"
  launch_status="${TEST_LAUNCH_STATUS:-0}"
  [[ "$launch_status" == 0 ]] || return "$launch_status"
  case "${TEST_LAUNCH_STATE:-running}" in
    running) echo running > "$TEST_STATE" ;;
    unknown) echo error > "$TEST_STATE" ;;
    *) echo absent > "$TEST_STATE" ;;
  esac
  return 0
}
func_save() { :; }

fail() { echo "Redsocks lifecycle regression: FAIL: $*" >&2; exit 1; }
reset() {
  echo "$1" > "$TEST_STATE"
  : > "$TEST_SIGNALS"
  : > "$TEST_SLEEP_LOG"
  : > "$TEST_START_LOG"
  : > "$TEST_LOG"
  unset TEST_SLEEP_FAIL TEST_TERM_MODE TEST_KILL_MODE
  TEST_LAUNCH_STATUS=0
  TEST_LAUNCH_STATE=running
  TEST_READY_AFTER_SLEEP=0
}

# UNKNOWN initial process inventory must never be interpreted as ABSENT.
for mode in error malformed empty-success inconsistent; do
  reset "$mode"
  if func_stop; then fail "stop accepted UNKNOWN inventory: $mode"; fi
  [[ ! -s "$TEST_SIGNALS" ]] || fail "stop signalled UNKNOWN inventory: $mode"
done
reset error
if func_start; then fail "start accepted UNKNOWN process inventory"; fi
[[ ! -s "$TEST_START_LOG" && ! -s "$TEST_SIGNALS" ]] || fail "UNKNOWN start launched or signalled a daemon"

# A daemon that exits after TERM must not receive KILL.
reset running
TEST_TERM_MODE=stop
export TEST_TERM_MODE
func_stop || fail "graceful stop failed"
[[ "$(cat "$TEST_SIGNALS")" == "-q redsocks" ]] || fail "graceful stop sent unexpected signals"
[[ ! -s "$TEST_SLEEP_LOG" ]] || fail "graceful stop did not observe absence immediately"

# UNKNOWN after TERM and failed sleep both block KILL escalation.
reset running
TEST_TERM_MODE=unknown
export TEST_TERM_MODE
if func_stop; then fail "UNKNOWN after TERM was accepted"; fi
[[ "$(cat "$TEST_SIGNALS")" == "-q redsocks" ]] || fail "UNKNOWN after TERM escalated to KILL"
reset running
TEST_TERM_MODE=keep
TEST_SLEEP_FAIL=1
export TEST_TERM_MODE TEST_SLEEP_FAIL
if func_stop; then fail "sleep failure was accepted as a successful stop"; fi
[[ "$(cat "$TEST_SIGNALS")" == "-q redsocks" ]] || fail "sleep failure escalated to KILL"
[[ "$(wc -l < "$TEST_SLEEP_LOG" | tr -d ' ')" == 1 ]] || fail "sleep failure was not observed once"

# TERM-resistant process receives KILL only after five bounded polls.
reset stuck
TEST_TERM_MODE=keep
TEST_KILL_MODE=stop
export TEST_TERM_MODE TEST_KILL_MODE
func_stop || fail "force stop failed"
[[ "$(cat "$TEST_SIGNALS")" == "$(printf '%s\n' '-q redsocks' '-q -9 redsocks')" ]] || fail "force stop signal sequence incorrect"
[[ "$(wc -l < "$TEST_SLEEP_LOG" | tr -d ' ')" == 5 ]] || fail "TERM wait was not bounded to five polls"
[[ "$(cat "$TEST_STATE")" == absent ]] || fail "force stop did not verify absence"

# A daemon that survives KILL remains a failed stop.
reset stuck
TEST_TERM_MODE=keep
TEST_KILL_MODE=keep
export TEST_TERM_MODE TEST_KILL_MODE
if func_stop; then fail "stuck daemon was reported stopped"; fi
[[ "$(wc -l < "$TEST_SLEEP_LOG" | tr -d ' ')" == 10 ]] || fail "TERM/KILL waits were not bounded"

# Start must not launch a replacement when stop postconditions fail.
reset stuck
TEST_TERM_MODE=keep
TEST_KILL_MODE=keep
export TEST_TERM_MODE TEST_KILL_MODE
if func_start; then fail "start launched over a daemon that could not be stopped"; fi
[[ ! -s "$TEST_START_LOG" ]] || fail "replacement daemon launched after failed stop"

# Running start performs a verified stop before launching; absent start is direct.
reset running
TEST_TERM_MODE=stop
export TEST_TERM_MODE
func_start || fail "start did not replace a stoppable existing daemon"
[[ "$(cat "$TEST_START_LOG")" == start ]] || fail "replacement launch missing"
reset absent
func_start || fail "start from confirmed ABSENT failed"
[[ "$(cat "$TEST_START_LOG")" == start ]] || fail "initial launch missing"

# Detached startup is not successful until the child is visible in pidof.
reset absent
TEST_LAUNCH_STATE=absent
TEST_READY_AFTER_SLEEP=2
func_start || fail "delayed detached child was not accepted"
[[ "$(cat "$TEST_STATE")" == running ]] || fail "delayed child never became RUNNING"
[[ "$(wc -l < "$TEST_SLEEP_LOG" | tr -d ' ')" == 2 ]] || fail "delayed readiness polling count incorrect"
grep -F "STARTED." "$TEST_LOG" >/dev/null || fail "STARTED was not logged after readiness"

# A successful launch with no child must time out, not report STARTED.
reset absent
TEST_LAUNCH_STATE=absent
if func_start; then fail "ABSENT startup timeout was accepted"; fi
[[ "$(wc -l < "$TEST_SLEEP_LOG" | tr -d ' ')" == 4 ]] || fail "startup timeout did not use four bounded waits"
if grep -F "STARTED." "$TEST_LOG" >/dev/null; then fail "timed-out daemon was logged STARTED"; fi

# UNKNOWN inventory and a failed wait primitive fail closed without retrying.
reset absent
TEST_LAUNCH_STATE=unknown
if func_start; then fail "UNKNOWN startup inventory was accepted"; fi
[[ ! -s "$TEST_SLEEP_LOG" ]] || fail "UNKNOWN startup inventory was retried"
reset absent
TEST_LAUNCH_STATE=absent
TEST_SLEEP_FAIL=1
if func_start; then fail "startup sleep failure was accepted"; fi
[[ "$(wc -l < "$TEST_SLEEP_LOG" | tr -d ' ')" == 1 ]] || fail "startup sleep failure did not abort immediately"

# Preserve the launcher's exact error and never enter readiness polling.
reset absent
TEST_LAUNCH_STATUS=23
if func_start; then fail "launcher error status 23 was accepted"; else launch_result=$?; fi
[[ "$launch_result" == 23 ]] || fail "launcher status was changed to $launch_result"
[[ ! -s "$TEST_SLEEP_LOG" ]] || fail "readiness polling ran after launcher failure"

# The generated-config path must use the same detached-child postcondition.
reset absent
rm -f "$REDSOCKS_CONF"
ARG2=192.0.2.10
ARG3=1080
func_start || fail "generated-config detached startup failed"
[[ -s "$REDSOCKS_CONF" ]] || fail "generated configuration was not written"
[[ "$(cat "$TEST_STATE")" == running ]] || fail "generated-config launch skipped readiness"
ARG2=nolog
ARG3=test

echo "Redsocks tri-state inventory, bounded stop and fail-closed restart regression: PASS"
