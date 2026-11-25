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


# Firewall programming must be idempotent and every command status is authoritative.
TEST_IPTABLES_RULES="$TEST_TMP/iptables.rules"
TEST_IPTABLES_CHAINS="$TEST_TMP/iptables.chains"
TEST_IPTABLES_LOG="$TEST_TMP/iptables.log"
: > "$TEST_IPTABLES_RULES"
: > "$TEST_IPTABLES_CHAINS"
: > "$TEST_IPTABLES_LOG"
REDSOCKS_DIR="$TEST_TMP/firewall"
mkdir -m 700 "$REDSOCKS_DIR"
REDSOCKS_CONF="$REDSOCKS_DIR/redsocks.conf"
CNZONE_FILE="$REDSOCKS_DIR/cn.zone"
LOCK="$TEST_TMP/set_lock"
CHAIN_NAME=REDSOCKS
SET_NAME=china
printf '203.0.113.0/24\n' > "$CNZONE_FILE"
: > "$LOCK"
ARG2=192.0.2.77
REMOTE_IP=127.0.0.1
func_save() { :; }
ipset() { printf 'unexpected ipset call: %s\n' "$*" >&2; return 97; }
wget() { printf 'unexpected wget call: %s\n' "$*" >&2; return 98; }

iptables() {
  local table=filter operation chain rule key tmp_file
  [[ "$1" == -t && "$2" == nat ]] || return 2
  table=$2
  shift 2
  operation=$1
  shift
  chain=$1
  rule="$*"
  printf '%s %s %s\n' "$table" "$operation" "$rule" >> "$TEST_IPTABLES_LOG"
  case "$operation" in
    -C)
      [[ "\${TEST_IPTABLES_FAIL_CHECK:-0}" == 0 ]] || return 4
      key="$table|$rule"
      grep -Fqx -- "$key" "$TEST_IPTABLES_RULES" && return 0
      return 1
      ;;
    -D)
      [[ "\${TEST_IPTABLES_FAIL_DELETE:-0}" == 0 ]] || return 7
      key="$table|$rule"
      grep -Fqx -- "$key" "$TEST_IPTABLES_RULES" || return 1
      tmp_file="$TEST_IPTABLES_RULES.tmp"
      awk -v key="$key" 'BEGIN { removed=0 } !removed && $0==key { removed=1; next } { print }' "$TEST_IPTABLES_RULES" > "$tmp_file" || return 90
      mv "$tmp_file" "$TEST_IPTABLES_RULES"
      ;;
    -S)
      grep -Fqx -- "$chain" "$TEST_IPTABLES_CHAINS" && return 0
      return 1
      ;;
    -N)
      grep -Fqx -- "$chain" "$TEST_IPTABLES_CHAINS" && return 1
      printf '%s\n' "$chain" >> "$TEST_IPTABLES_CHAINS"
      ;;
    -F)
      tmp_file="$TEST_IPTABLES_RULES.tmp"
      awk -F'|' -v chain="$chain" '$2 !~ ("^" chain "($| )") { print }' "$TEST_IPTABLES_RULES" > "$tmp_file" || return 90
      mv "$tmp_file" "$TEST_IPTABLES_RULES"
      ;;
    -X)
      grep -Fq -- "$table|$chain " "$TEST_IPTABLES_RULES" && return 1
      grep -Fqx -- "$chain" "$TEST_IPTABLES_CHAINS" || return 1
      grep -Fvx -- "$chain" "$TEST_IPTABLES_CHAINS" > "$TEST_IPTABLES_CHAINS.tmp" || :
      mv "$TEST_IPTABLES_CHAINS.tmp" "$TEST_IPTABLES_CHAINS"
      ;;
    -A)
      if [[ "\${TEST_IPTABLES_FAIL_APPEND:-}" == "$rule" ]]; then return 23; fi
      printf '%s|%s\n' "$table" "$rule" >> "$TEST_IPTABLES_RULES"
      ;;
    *)
      return 2
      ;;
  esac
}

firewall_reset() {
  : > "$TEST_IPTABLES_RULES"
  : > "$TEST_IPTABLES_CHAINS"
  : > "$TEST_IPTABLES_LOG"
  unset TEST_IPTABLES_FAIL_CHECK TEST_IPTABLES_FAIL_DELETE TEST_IPTABLES_FAIL_APPEND
}
firewall_rule_count() {
  grep -Fxc -- "nat|$1" "$TEST_IPTABLES_RULES" || :
}
firewall_expect_clean() {
  [[ ! -s "$TEST_IPTABLES_RULES" ]] || fail "firewall rollback left active NAT rules"
  [[ ! -s "$TEST_IPTABLES_CHAINS" ]] || fail "firewall rollback left the REDSOCKS chain"
}

firewall_reset
printf '%s\n' 'nat|PREROUTING -p tcp -j REDSOCKS' >> "$TEST_IPTABLES_RULES"
ARG2=010.0.0.7
before_log=$(wc -l < "$TEST_IPTABLES_LOG" | tr -d ' ')
if func_iptables; then fail "octal-ambiguous Redsocks REMOTE_IP was accepted"; else firewall_status=$?; fi
[[ "$firewall_status" == 2 ]] || fail "invalid Redsocks REMOTE_IP status was $firewall_status"
[[ "$(wc -l < "$TEST_IPTABLES_LOG" | tr -d ' ')" == "$before_log" ]] || fail "invalid REMOTE_IP changed firewall rules"
[[ "$(firewall_rule_count 'PREROUTING -p tcp -j REDSOCKS')" == 1 ]] || fail "invalid REMOTE_IP removed existing jump"
echo "Redsocks rejects ambiguous REMOTE_IP before cleanup: PASS"

firewall_reset
ARG2=192.0.2.77
if func_iptables; then
  :
else
  firewall_status=$?
  echo "Redsocks firewall command trace after initial setup failure:" >&2
  cat "$TEST_IPTABLES_LOG" >&2
  fail "initial Redsocks firewall setup failed (status=$firewall_status)"
fi
[[ "$(firewall_rule_count 'PREROUTING -p tcp -j REDSOCKS')" == 1 ]] || fail "PREROUTING jump was not installed exactly once"
[[ "$(firewall_rule_count 'OUTPUT -p tcp -j REDSOCKS')" == 1 ]] || fail "OUTPUT jump was not installed exactly once"
[[ "$(firewall_rule_count 'REDSOCKS -p tcp -j REDIRECT --to-ports 12345')" == 1 ]] || fail "redirect rule was not installed exactly once"
printf '%s\n' 'nat|PREROUTING -p tcp -j REDSOCKS' 'nat|PREROUTING -p tcp -j REDSOCKS' 'nat|OUTPUT -p tcp -j REDSOCKS' >> "$TEST_IPTABLES_RULES"
func_iptables || fail "reapplying Redsocks firewall policy failed"
[[ "$(firewall_rule_count 'PREROUTING -p tcp -j REDSOCKS')" == 1 ]] || fail "duplicate PREROUTING jumps survived cleanup"
[[ "$(firewall_rule_count 'OUTPUT -p tcp -j REDSOCKS')" == 1 ]] || fail "duplicate OUTPUT jumps survived cleanup"
[[ "$(firewall_rule_count 'REDSOCKS -p tcp -j REDIRECT --to-ports 12345')" == 1 ]] || fail "duplicate chain rules survived rebuild"
echo "Redsocks firewall reapply drains legacy duplicates and remains idempotent: PASS"

TEST_IPTABLES_FAIL_CHECK=1
if func_iptables; then fail "iptables -C error was treated as an absent rule"; else firewall_status=$?; fi
[[ "$firewall_status" == 4 ]] || fail "iptables -C error status was $firewall_status"
[[ -s "$TEST_IPTABLES_RULES" ]] || fail "check error unexpectedly removed existing firewall state"
unset TEST_IPTABLES_FAIL_CHECK
echo "Redsocks firewall check errors fail closed: PASS"

TEST_IPTABLES_FAIL_DELETE=1
if func_iptables; then fail "iptables -D error was ignored"; else firewall_status=$?; fi
[[ "$firewall_status" == 7 ]] || fail "iptables -D error status was $firewall_status"
[[ -s "$TEST_IPTABLES_RULES" ]] || fail "delete failure removed prior rules"
if grep -Fq 'nat -N REDSOCKS' "$TEST_IPTABLES_LOG"; then fail "chain was recreated after cleanup failure"; fi
unset TEST_IPTABLES_FAIL_DELETE
echo "Redsocks firewall delete failures block replacement policy: PASS"

firewall_reset
TEST_IPTABLES_FAIL_APPEND='OUTPUT -p tcp -j REDSOCKS'
if func_iptables; then fail "partial jump installation was accepted"; else firewall_status=$?; fi
[[ "$firewall_status" == 23 ]] || fail "iptables append failure status was $firewall_status"
firewall_expect_clean
echo "Redsocks partial firewall setup rolls back before returning failure: PASS"

echo "Redsocks tri-state inventory, bounded stop and fail-closed restart regression: PASS"
