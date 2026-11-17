#!/bin/sh
set -u
base=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
tmp=$(mktemp -d "${TMPDIR:-/tmp}/iappd-test.XXXXXX") || exit 1
trap 'rm -rf "$tmp"' EXIT HUP INT TERM
mkdir -p "$tmp/bin" "$tmp/no-pidof" "$tmp/state"
: > "$tmp/commands"; : > "$tmp/daemon"
printf "mt76x3_ap\n" > "$tmp/modules"
printf "rai0: 1\n" > "$tmp/interrupts"
cat > "$tmp/mock" <<'EOF'
#!/bin/sh
name=${0##*/}
echo "$name $*" >> "$COMMAND_LOG"
case "$name" in
  pidof)
    [ "$MOCK_PIDOF" -le 1 ] || exit "$MOCK_PIDOF"
    [ -f "$STATE_DIR/daemon" ] && { echo 123; exit 0; }
    exit 1 ;;
  ralinkiappd)
    echo "$*" >> "$DAEMON_LOG"
    if [ "$MOCK_START" -eq 0 ] || [ "$MOCK_PARTIAL_START" -eq 1 ]; then
      : > "$STATE_DIR/daemon"
    fi
    [ "$MOCK_EXIT_AFTER_LAUNCH" -eq 1 ] && rm -f "$STATE_DIR/daemon"
    exit "$MOCK_START" ;;
  sysctl) exit "$MOCK_SYSCTL" ;;
  iptables)
    if [ "$1" = "-A" ]; then
      append_count=$(grep -c '^iptables -A ' "$COMMAND_LOG")
      if [ "$MOCK_IPTABLES_FAIL_AT" -gt 0 ] &&
         [ "$append_count" -eq "$MOCK_IPTABLES_FAIL_AT" ]; then
        exit "$MOCK_IPTABLES"
      fi
      shift
      printf '%s\n' "-A $*" >> "$RULES_FILE"
      exit 0
    fi
    if [ "$1" = "-D" ]; then
      [ "$MOCK_IPTABLES_DELETE" -eq 0 ] || exit "$MOCK_IPTABLES_DELETE"
      shift
      target="-A"
      for arg in "$@"; do target="$target $arg"; done
      awk -v target="$target" 'BEGIN { removed=0 } $0 == target && !removed { removed=1; next } { print } END { if (!removed) exit 1 }' "$RULES_FILE" > "$RULES_FILE.tmp" || exit 93
      mv "$RULES_FILE.tmp" "$RULES_FILE" || exit 94
      exit 0
    fi
    exit 0 ;;
  iptables-save)
    [ "$MOCK_IPTABLES_SAVE" -eq 0 ] || exit "$MOCK_IPTABLES_SAVE"
    [ "$1" = "-t" ] && [ "$2" = "filter" ] || exit 95
    cat "$RULES_FILE"
    exit 0 ;;
  killall)
    [ "$MOCK_KILLALL" -eq 0 ] && [ "$MOCK_REMOVE" -eq 1 ] && rm -f "$STATE_DIR/daemon"
    exit "$MOCK_KILLALL" ;;
  sleep|logger) exit 0 ;;
  *) exit 127 ;;
esac
EOF
chmod +x "$tmp/mock"
for c in pidof ralinkiappd sysctl iptables iptables-save killall sleep logger; do ln -s "$tmp/mock" "$tmp/bin/$c"; done
ln -s /bin/grep "$tmp/bin/grep"
ln -s "$tmp/bin/logger" "$tmp/no-pidof/logger"
export PATH="$tmp/bin:$PATH" STATE_DIR="$tmp/state" COMMAND_LOG="$tmp/commands" DAEMON_LOG="$tmp/daemon" RULES_FILE="$tmp/state/firewall.rules"
: > "$RULES_FILE"
export MOCK_PIDOF=0 MOCK_START=0 MOCK_PARTIAL_START=0 MOCK_EXIT_AFTER_LAUNCH=0 MOCK_SYSCTL=0 MOCK_IPTABLES=0 MOCK_IPTABLES_FAIL_AT=0 MOCK_IPTABLES_DELETE=0 MOCK_IPTABLES_SAVE=0 MOCK_KILLALL=0 MOCK_REMOVE=1
export IAPPD_PROC_MODULES="$tmp/modules" IAPPD_PROC_INTERRUPTS="$tmp/interrupts"
fail() { echo "$*" >&2; exit 1; }
status() { [ "$1" -eq "$2" ] || fail "expected $2, got $1"; }
run() { /bin/sh "$base/iappd.sh" "$1"; }
capture_status() { "$@"; rc=$?; printf "%s" "$rc"; }
reset() { rm -f "$STATE_DIR/daemon"; : > "$COMMAND_LOG"; : > "$DAEMON_LOG"; : > "$RULES_FILE"; MOCK_PIDOF=0 MOCK_START=0 MOCK_PARTIAL_START=0 MOCK_EXIT_AFTER_LAUNCH=0 MOCK_IPTABLES=0 MOCK_IPTABLES_FAIL_AT=0 MOCK_IPTABLES_DELETE=0 MOCK_IPTABLES_SAVE=0 MOCK_KILLALL=0 MOCK_REMOVE=1; }

reset
rc=$(capture_status env PATH="$tmp/no-pidof" /bin/sh "$base/iappd.sh" start)
status "$rc" 127
! grep -q "^ralinkiappd " "$COMMAND_LOG" || fail "started without pidof"

reset; : > "$STATE_DIR/daemon"
rc=$(capture_status env PATH="$tmp/no-pidof" /bin/sh "$base/iappd.sh" stop)
status "$rc" 127
[ -f "$STATE_DIR/daemon" ] || fail "missing pidof changed daemon state"

reset; MOCK_PIDOF=2
rc=$(capture_status run start); status "$rc" 2
! grep -q "^ralinkiappd " "$COMMAND_LOG" || fail "started with UNKNOWN inventory"

reset; MOCK_START=53; MOCK_PARTIAL_START=1
rc=$(capture_status run start); status "$rc" 53
[ ! -f "$STATE_DIR/daemon" ] || fail "launcher failure left a partially started daemon"
grep -q "^killall " "$COMMAND_LOG" || fail "launcher failure skipped daemon rollback"
! grep -q "^iptables -A " "$COMMAND_LOG" || fail "configured firewall after launcher failure"

# A successful launcher that exits before the bounded readiness check
# must not be reported as a successful service start or publish firewall rules.
reset; MOCK_EXIT_AFTER_LAUNCH=1
rc=$(capture_status run start); status "$rc" 1
[ ! -f "$STATE_DIR/daemon" ] || fail "readiness failure left daemon marker"
! grep -q "^iptables -A " "$COMMAND_LOG" || fail "published firewall after readiness failure"
grep -q "did not remain running after launch" "$COMMAND_LOG" || fail "readiness timeout was not logged"

# Cleanup failure must not replace the startup error being reported.
reset; MOCK_START=53; MOCK_PARTIAL_START=1; MOCK_KILLALL=54
rc=$(capture_status run start); status "$rc" 53
[ -f "$STATE_DIR/daemon" ] || fail "failed cleanup unexpectedly changed daemon state"
grep -q "daemon/firewall startup rollback failed" "$COMMAND_LOG" || fail "failed daemon/firewall rollback was not logged"

# A failed first firewall rule stops the daemon without deleting a rule that
# this invocation did not successfully create.
reset; MOCK_IPTABLES=42; MOCK_IPTABLES_FAIL_AT=1
rc=$(capture_status run start); status "$rc" 42
[ ! -f "$STATE_DIR/daemon" ] || fail "TCP rule failure left daemon running"
grep -q "^killall " "$COMMAND_LOG" || fail "TCP rule failure skipped daemon rollback"
[ "$(grep -c "^iptables -D " "$COMMAND_LOG")" -eq 0 ] || fail "removed an unowned firewall rule"

# If UDP publication fails after TCP succeeds, remove the TCP rule created by
# this attempt and stop the daemon while preserving the original error.
reset; MOCK_IPTABLES=42; MOCK_IPTABLES_FAIL_AT=2
rc=$(capture_status run start); status "$rc" 42
[ ! -f "$STATE_DIR/daemon" ] || fail "UDP rule failure left daemon running"
grep -Fq "iptables -D INPUT -i br0 -p tcp --dport 3517 -m comment --comment ralinkiappd-tcp -j ACCEPT" "$COMMAND_LOG" || fail "UDP failure did not roll back owned TCP rule"
[ "$(grep -c "^iptables -D " "$COMMAND_LOG")" -eq 1 ] || fail "unexpected firewall rollback rule count"
! grep -q "iptables -D INPUT -i br0 -p udp " "$COMMAND_LOG" || fail "removed UDP rule not owned by this start"

reset
rc=$(capture_status run start); status "$rc" 0
[ -f "$STATE_DIR/daemon" ] || fail "start did not create daemon marker"
grep -Fq -- "-wi rai0 -d 0" "$DAEMON_LOG" || fail "wrong radio arguments"
[ "$(grep -c "^iptables -A " "$COMMAND_LOG")" -eq 2 ] || fail "missing firewall rules"
[ "$(grep -c "ralinkiappd-" "$RULES_FILE")" -eq 2 ] || fail "missing tagged firewall ownership rules"
count=$(grep -c "^ralinkiappd " "$COMMAND_LOG")
rc=$(capture_status run start); status "$rc" 0
[ "$(grep -c "^ralinkiappd " "$COMMAND_LOG")" -eq "$count" ] || fail "duplicate start"
rc=$(capture_status run stop); status "$rc" 0
[ ! -s "$RULES_FILE" ] || fail "normal stop left owned firewall rules"
[ "$(grep -c "^iptables -D " "$COMMAND_LOG")" -eq 2 ] || fail "normal stop did not delete both owned rules"

# A stop with no daemon also reconciles stale IAPP-owned rules but preserves
# administrator rules with the same protocol/port tuple and no ownership tag.
reset
printf '%s\n' '-A INPUT -i br0 -p tcp --dport 3517 -j ACCEPT' '-A INPUT -i br0 -p tcp --dport 3517 -m comment --comment ralinkiappd-tcp -j ACCEPT' '-A INPUT -i br0 -p udp --dport 3517 -m comment --comment ralinkiappd-udp -j ACCEPT' > "$RULES_FILE"
rc=$(capture_status run stop); status "$rc" 0
[ "$(wc -l < "$RULES_FILE" | tr -d ' ')" -eq 1 ] || fail "stop removed an unowned rule or retained owned rules"
grep -Fq -- '-A INPUT -i br0 -p tcp --dport 3517 -j ACCEPT' "$RULES_FILE" || fail "untagged administrator rule was removed"

# Startup cleans a stale owned tag before publishing the current two rules.
reset
printf '%s\n' '-A INPUT -i br0 -p tcp --dport 3517 -m comment --comment ralinkiappd-tcp -j ACCEPT' > "$RULES_FILE"
rc=$(capture_status run start); status "$rc" 0
[ "$(grep -c "ralinkiappd-" "$RULES_FILE")" -eq 2 ] || fail "start failed to replace stale tagged rules"
[ "$(grep -c "^iptables -D " "$COMMAND_LOG")" -eq 1 ] || fail "start did not delete the stale tagged rule"

# Restart must not accumulate duplicate firewall rules.
reset; rc=$(capture_status run start); status "$rc" 0
rc=$(capture_status run restart); status "$rc" 0
[ "$(grep -c "ralinkiappd-" "$RULES_FILE")" -eq 2 ] || fail "restart accumulated duplicate tagged rules"

# A cleanup error is surfaced after the daemon is confirmed stopped. A later
# stop can retry cleanup without leaving a daemon behind.
reset; rc=$(capture_status run start); status "$rc" 0
MOCK_IPTABLES_DELETE=55
rc=$(capture_status run stop); status "$rc" 55
[ ! -f "$STATE_DIR/daemon" ] || fail "firewall cleanup failure left daemon running"
[ "$(grep -c "ralinkiappd-" "$RULES_FILE")" -eq 2 ] || fail "failed deletion did not preserve remaining rules"
MOCK_IPTABLES_DELETE=0
rc=$(capture_status run stop); status "$rc" 0
[ ! -s "$RULES_FILE" ] || fail "subsequent stop did not finish firewall cleanup"

reset; MOCK_IPTABLES=42; MOCK_IPTABLES_FAIL_AT=1
rc=$(capture_status run start); status "$rc" 42

reset; : > "$STATE_DIR/daemon"; MOCK_KILLALL=54
rc=$(capture_status run stop); status "$rc" 54
[ -f "$STATE_DIR/daemon" ] || fail "failed killall lost daemon marker"

reset; : > "$STATE_DIR/daemon"; MOCK_REMOVE=0
rc=$(capture_status run stop); status "$rc" 1
[ -f "$STATE_DIR/daemon" ] || fail "stop postcondition false positive"

reset; : > "$STATE_DIR/daemon"
rc=$(capture_status run stop); status "$rc" 0
[ ! -f "$STATE_DIR/daemon" ] || fail "successful stop left daemon marker"

reset; : > "$STATE_DIR/daemon"; MOCK_KILLALL=54
rc=$(capture_status run restart); status "$rc" 54
! grep -q "^ralinkiappd " "$COMMAND_LOG" || fail "restart ran after failed stop"

reset
rc=$(capture_status run test); status "$rc" 1
: > "$STATE_DIR/daemon"
rc=$(capture_status run test); status "$rc" 0
rc=$(capture_status run invalid); status "$rc" 1
echo "Ralink IAPP daemon lifecycle regressions: PASS"
