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
TEST_PROC_ROOT="$TEST_TMP/proc"
RED_SOCKS_PROC_ROOT="$TEST_PROC_ROOT"
mkdir -p "$TEST_PROC_ROOT"
export TEST_STATE TEST_SIGNALS TEST_SLEEP_LOG TEST_START_LOG TEST_LOG TEST_PROC_ROOT RED_SOCKS_PROC_ROOT

BINARY_NAME=redsocks
REDSOCKS_DIR="$TEST_TMP"
REDSOCKS_CONF="$TEST_TMP/redsocks.conf"
BINARY_PATH="$TEST_TMP/redsocks"
touch "$BINARY_PATH"
chmod 755 "$BINARY_PATH"
touch "$TEST_TMP/not-redsocks"
touch "$REDSOCKS_CONF"

logger() { printf '%s\n' "$*" >> "$TEST_LOG"; }
sync_test_proc() {
  local state start_time exe_target
  state=$(cat "$TEST_STATE")
  case "$state" in
    running|stuck|replaced)
      start_time=4000
      [[ "$state" == replaced ]] && start_time=5000
      mkdir -p "$TEST_PROC_ROOT/12345"
      printf '%s\n' "12345 (redsocks) S 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 $start_time" > "$TEST_PROC_ROOT/12345/stat"
      rm -f "$TEST_PROC_ROOT/12345/exe"
      exe_target="$BINARY_PATH"
      [[ "${TEST_EXE_MODE:-valid}" == valid ]] || exe_target="$TEST_TMP/not-redsocks"
      ln -s "$exe_target" "$TEST_PROC_ROOT/12345/exe"
      ;;
    *) rm -rf "$TEST_PROC_ROOT/12345" ;;
  esac
}
pidof() {
  [[ "$1" == "$BINARY_NAME" ]] || return 2
  sync_test_proc
  case "$(cat "$TEST_STATE")" in
    running|stuck|replaced) echo 12345; return 0 ;;
    error) return 2 ;;
    malformed) echo "12345 nope"; return 0 ;;
    empty-success) return 0 ;;
    inconsistent) echo 12345; return 1 ;;
    *) return 1 ;;
  esac
}
kill() {
  local signal="$1" pid="$2"
  [[ "$pid" == 12345 ]] || return 97
  printf '%s %s\n' "$signal" "$pid" >> "$TEST_SIGNALS"
  if [[ "$signal" == -TERM ]]; then
    [[ "${TEST_TERM_MODE:-stop}" == stop ]] && echo absent > "$TEST_STATE"
    [[ "${TEST_TERM_MODE:-stop}" == unknown ]] && echo error > "$TEST_STATE"
  elif [[ "$signal" == -KILL ]]; then
    [[ "${TEST_KILL_MODE:-stop}" == stop ]] && echo absent > "$TEST_STATE"
  else
    return 98
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
  if [[ "${TEST_REPLACE_AFTER_SLEEP:-0}" -gt 0 && "$sleep_count" -ge "${TEST_REPLACE_AFTER_SLEEP}" ]]; then
    echo replaced > "$TEST_STATE"
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
touch "$BINARY_PATH"
chmod 755 "$BINARY_PATH"
touch "$TEST_TMP/not-redsocks"
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
  unset TEST_SLEEP_FAIL TEST_TERM_MODE TEST_KILL_MODE TEST_EXE_MODE TEST_REPLACE_AFTER_SLEEP
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

# A PID recycled under the same comm is not the original daemon and must not
# receive KILL after the bounded TERM wait.
reset running
TEST_TERM_MODE=keep
TEST_REPLACE_AFTER_SLEEP=5
export TEST_TERM_MODE TEST_REPLACE_AFTER_SLEEP
if func_stop; then fail "recycled PID was reported stopped"; fi
[[ "$(cat "$TEST_SIGNALS")" == "-TERM 12345" ]] || fail "KILL was sent to a recycled PID"
echo "Redsocks stop refuses PID reuse between TERM and KILL: PASS"

# pidof's same-name result is insufficient unless /proc/PID/exe resolves to
# the configured binary.
reset running
TEST_EXE_MODE=wrong
export TEST_EXE_MODE
if func_stop; then fail "same-comm foreign executable was accepted"; fi
[[ ! -s "$TEST_SIGNALS" ]] || fail "foreign executable received a signal"
echo "Redsocks refuses same-comm foreign executable: PASS"

# A daemon that exits after TERM must not receive KILL.
reset running
TEST_TERM_MODE=stop
export TEST_TERM_MODE
func_stop || fail "graceful stop failed"
[[ "$(cat "$TEST_SIGNALS")" == "-TERM 12345" ]] || fail "graceful stop did not signal the captured PID"
[[ ! -s "$TEST_SLEEP_LOG" ]] || fail "graceful stop did not observe absence immediately"

# UNKNOWN after TERM and failed sleep both block KILL escalation.
reset running
TEST_TERM_MODE=unknown
export TEST_TERM_MODE
if func_stop; then fail "UNKNOWN after TERM was accepted"; fi
[[ "$(cat "$TEST_SIGNALS")" == "-TERM 12345" ]] || fail "UNKNOWN after TERM escalated to KILL"
reset running
TEST_TERM_MODE=keep
TEST_SLEEP_FAIL=1
export TEST_TERM_MODE TEST_SLEEP_FAIL
if func_stop; then fail "sleep failure was accepted as a successful stop"; fi
[[ "$(cat "$TEST_SIGNALS")" == "-TERM 12345" ]] || fail "sleep failure escalated to KILL"
[[ "$(wc -l < "$TEST_SLEEP_LOG" | tr -d ' ')" == 1 ]] || fail "sleep failure was not observed once"

# TERM-resistant process receives KILL only after five bounded polls.
reset stuck
TEST_TERM_MODE=keep
TEST_KILL_MODE=stop
export TEST_TERM_MODE TEST_KILL_MODE
func_stop || fail "force stop failed"
[[ "$(cat "$TEST_SIGNALS")" == "$(printf '%s\n' '-TERM 12345' '-KILL 12345')" ]] || fail "force stop signal sequence incorrect"
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

# A missing config and no endpoint is a safe no-op; do not stop a healthy daemon.
reset running
rm -f "$REDSOCKS_CONF"
ARG2=
ARG3=
func_start || fail "missing config without endpoint was not a safe no-op"
[[ "$(cat "$TEST_STATE")" == running ]] || fail "missing config stopped the existing daemon"
[[ ! -s "$TEST_SIGNALS" && ! -s "$TEST_START_LOG" ]] ||
  fail "missing config without endpoint signalled or launched a daemon"

# Invalid endpoints and line injection are rejected before any process change.
bad_ip_injected=$(printf '192.0.2.10\nbase {')
for bad_ip in 300.0.0.1 010.0.0.7 "$bad_ip_injected"; do
  reset running
  rm -f "$REDSOCKS_CONF"
  ARG2="$bad_ip"
  ARG3=1080
  if func_start; then fail "invalid SOCKS5 address was accepted"; else status=$?; fi
  [[ "$status" == 2 ]] || fail "invalid SOCKS5 address status was $status"
  [[ "$(cat "$TEST_STATE")" == running ]] || fail "invalid address stopped the existing daemon"
  [[ ! -s "$TEST_SIGNALS" && ! -s "$TEST_START_LOG" ]] ||
    fail "invalid address signalled or launched a daemon"
  [[ ! -e "$REDSOCKS_CONF" ]] || fail "invalid address created a config file"
done
bad_port_injected=$(printf '1080\nbase {')
for bad_port in 0 65536 abc 000000 "$bad_port_injected"; do
  reset running
  rm -f "$REDSOCKS_CONF"
  ARG2=192.0.2.10
  ARG3="$bad_port"
  if func_start; then fail "invalid SOCKS5 port was accepted"; else status=$?; fi
  [[ "$status" == 2 ]] || fail "invalid SOCKS5 port status was $status"
  [[ "$(cat "$TEST_STATE")" == running ]] || fail "invalid port stopped the existing daemon"
  [[ ! -s "$TEST_SIGNALS" && ! -s "$TEST_START_LOG" ]] ||
    fail "invalid port signalled or launched a daemon"
  [[ ! -e "$REDSOCKS_CONF" ]] || fail "invalid port created a config file"
done
echo "Redsocks endpoint validation rejects directive injection before process changes: PASS"

# The generated-config path must use the same detached-child postcondition.
reset absent
rm -f "$REDSOCKS_CONF"
ARG2=192.0.2.10
ARG3=01080
func_start || fail "generated-config detached startup failed"
[[ -s "$REDSOCKS_CONF" ]] || fail "generated configuration was not written"
[[ "$(cat "$TEST_STATE")" == running ]] || fail "generated-config launch skipped readiness"
grep -Fqx 'ip = 192.0.2.10;' "$REDSOCKS_CONF" || fail "validated endpoint address was not emitted"
grep -Fqx 'port = 1080;' "$REDSOCKS_CONF" || fail "leading-zero port was not normalized"
# Symlinked config directories and config files must be rejected before a
# currently running process is stopped or a privileged write follows the link.
REAL_CONFIG_DIR="$TEST_TMP/redsocks-real"
LINK_CONFIG_DIR="$TEST_TMP/redsocks-link"
mkdir -m 700 "$REAL_CONFIG_DIR"
printf 'keep-this-config\n' > "$REAL_CONFIG_DIR/marker"
ln -s "$REAL_CONFIG_DIR" "$LINK_CONFIG_DIR"
REDSOCKS_DIR="$LINK_CONFIG_DIR"
REDSOCKS_CONF="$REDSOCKS_DIR/redsocks.conf"
reset running
ARG2=192.0.2.10
ARG3=1080
if func_start; then fail "symlinked Redsocks config directory was accepted"; else status=$?; fi
[[ "$status" == 1 ]] || fail "symlinked config directory returned status $status"
[[ "$(cat "$TEST_STATE")" == running ]] || fail "symlinked config directory stopped the daemon"
[[ ! -s "$TEST_SIGNALS" && ! -s "$TEST_START_LOG" ]] ||
  fail "symlinked config directory signalled or launched a daemon"
[[ "$(cat "$REAL_CONFIG_DIR/marker")" == keep-this-config ]] ||
  fail "symlinked config directory changed its target"

REDSOCKS_DIR="$REAL_CONFIG_DIR"
REDSOCKS_CONF="$REDSOCKS_DIR/redsocks.conf"
ln -s "$TEST_TMP/dangling-config-target" "$REDSOCKS_CONF"
reset running
if func_start; then fail "dangling Redsocks config symlink was accepted"; else status=$?; fi
[[ "$status" == 1 ]] || fail "dangling config symlink returned status $status"
[[ "$(cat "$TEST_STATE")" == running ]] || fail "dangling config symlink stopped the daemon"
[[ ! -e "$TEST_TMP/dangling-config-target" ]] ||
  fail "dangling config symlink target was created"
[[ ! -s "$TEST_SIGNALS" && ! -s "$TEST_START_LOG" ]] ||
  fail "dangling config symlink signalled or launched a daemon"

rm -f "$REDSOCKS_CONF"
redsocks_config_paths_validate || fail "real config directory with absent target was rejected"
echo "Redsocks config directory and target symlinks fail closed before process changes: PASS"

# A predictable staging-name collision must fail before stopping a healthy daemon.
# Existing config directories may retain broad permissions from older releases.
chmod 755 "$REDSOCKS_DIR"
rm -f "$REDSOCKS_CONF"
reset absent
ARG2=192.0.2.10
ARG3=1080
func_start || fail "config generation failed after securing an existing directory"
[[ "$(stat -c %a "$REDSOCKS_DIR")" == 700 ]] ||
  fail "existing Redsocks config directory was not secured to mode 0700"
[[ -f "$REDSOCKS_CONF" ]] || fail "generated Redsocks config was not published"
rm -f "$REDSOCKS_CONF"
echo "Redsocks existing config directory is secured before generated config publication: PASS"

STAGING_DIR_TARGET="$TEST_TMP/staging-directory-target"
STAGING_DIR="$REDSOCKS_DIR/.${BINARY_NAME}.conf.$$"
mkdir -m 700 "$STAGING_DIR_TARGET"
printf 'staging target must survive\n' > "$STAGING_DIR_TARGET/marker"
ln -s "$STAGING_DIR_TARGET" "$STAGING_DIR"
reset running
ARG2=192.0.2.10
ARG3=1080
if func_start; then fail "pre-existing staging directory symlink was accepted"; else status=$?; fi
[[ "$status" == 1 ]] || fail "staging directory symlink returned status $status"
[[ "$(cat "$TEST_STATE")" == running ]] || fail "staging directory symlink stopped the daemon"
[[ ! -s "$TEST_SIGNALS" && ! -s "$TEST_START_LOG" ]] || fail "staging directory symlink signalled or launched a daemon"
[[ "$(cat "$STAGING_DIR_TARGET/marker")" == "staging target must survive" ]] || fail "staging directory symlink target was modified"
rm -f "$STAGING_DIR"
echo "Redsocks exclusive staging reservation rejects symlinks before process changes: PASS"

# Inject a destination symlink at publication time. Exclusive hard-link creation
# must fail without following the link or modifying its referent.
ln() {
  if [[ "${TEST_CONFIG_PUBLISH_RACE:-0}" == 1 && "$#" == 2 && "$2" == "$REDSOCKS_CONF" ]]; then
    TEST_CONFIG_PUBLISH_RACE=0
    command ln -s "$TEST_CONFIG_RACE_TARGET" "$REDSOCKS_CONF" || return $?
  fi
  command ln "$@"
}
TEST_CONFIG_RACE_TARGET="$TEST_TMP/publish-race-target"
printf 'publication target must survive\n' > "$TEST_CONFIG_RACE_TARGET"
rm -f "$REDSOCKS_CONF"
TEST_CONFIG_PUBLISH_RACE=1
reset absent
ARG2=192.0.2.10
ARG3=1080
if func_start; then fail "destination symlink publication race was accepted"; else status=$?; fi
[[ "$status" == 1 ]] || fail "publication race returned status $status"
[[ -L "$REDSOCKS_CONF" ]] || fail "publication race did not retain the injected symlink"
[[ "$(cat "$TEST_CONFIG_RACE_TARGET")" == "publication target must survive" ]] || fail "publication race wrote through the destination symlink"
[[ ! -s "$TEST_START_LOG" ]] || fail "daemon launched after config publication race"
rm -f "$REDSOCKS_CONF"
unset TEST_CONFIG_PUBLISH_RACE
echo "Redsocks atomic config publication refuses destination symlink races: PASS"

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
  printf '%s %s %s %s\n' "$table" "$operation" "$chain" "$rule" >> "$TEST_IPTABLES_LOG"
  case "$operation" in
    -C)
      [[ "${TEST_IPTABLES_FAIL_CHECK:-0}" == 0 ]] || return 4
      key="$table|$rule"
      grep -Fqx -- "$key" "$TEST_IPTABLES_RULES" && return 0
      return 1
      ;;
    -D)
      [[ "${TEST_IPTABLES_FAIL_DELETE:-0}" == 0 ]] || return 7
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
      if [[ "${TEST_IPTABLES_FAIL_APPEND:-}" == "$rule" ]]; then return 23; fi
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

# Isolate the trace so old successful -N calls cannot mask this failure assertion.
: > "$TEST_IPTABLES_LOG"
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
