#!/bin/bash
set -euo pipefail

ROOT=$(cd -- "$(dirname -- "$0")/../.." && pwd)
TEST_TMP=$(mktemp -d "${TMPDIR:-/tmp}/redsocks-lifecycle.XXXXXX")
trap 'rm -rf "$TEST_TMP"' EXIT HUP INT TERM
TEST_STATE="$TEST_TMP/state"
TEST_SIGNALS="$TEST_TMP/signals"
TEST_SLEEP_LOG="$TEST_TMP/sleep.log"
TEST_START_LOG="$TEST_TMP/start.log"
export TEST_STATE TEST_SIGNALS TEST_SLEEP_LOG TEST_START_LOG

BINARY_NAME=redsocks
REDSOCKS_DIR="$TEST_TMP"
REDSOCKS_CONF="$TEST_TMP/redsocks.conf"
BINARY_PATH="$TEST_TMP/redsocks"
touch "$REDSOCKS_CONF"

logger() { :; }
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
  return 0
}
func_redsocks() {
  echo start >> "$TEST_START_LOG"
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

fail() { echo "Redsocks lifecycle regression: FAIL: $*" >&2; exit 1; }
reset() {
  echo "$1" > "$TEST_STATE"
  : > "$TEST_SIGNALS"
  : > "$TEST_SLEEP_LOG"
  : > "$TEST_START_LOG"
  unset TEST_SLEEP_FAIL TEST_TERM_MODE TEST_KILL_MODE
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

echo "Redsocks tri-state inventory, bounded stop and fail-closed restart regression: PASS"
