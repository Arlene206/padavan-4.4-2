#!/bin/sh
set -eu

TEST_DIR=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
TMP_DIR=$(mktemp -d "${TMPDIR:-/tmp}/shadowsocks-lifecycle.XXXXXX")
trap 'rm -rf "$TMP_DIR"' EXIT HUP INT TERM
mkdir -p "$TMP_DIR/bin" "$TMP_DIR/state"
export PROCESS_STATE_DIR="$TMP_DIR/state"
export KILL_LOG="$TMP_DIR/kill.log"
export PATH="$TMP_DIR/bin:$PATH"

cat > "$TMP_DIR/bin/pidof" <<'EOF'
#!/bin/sh
[ "$#" -eq 1 ] || exit 2
[ -f "$PROCESS_STATE_DIR/pidof_unknown" ] && exit 2
case "$1" in
	empty_success) exit 0 ;;
	malformed) echo '4242 bad'; exit 0 ;;
	inconsistent) echo '4242'; exit 1 ;;
esac
state_file="$PROCESS_STATE_DIR/$1"
[ -f "$state_file" ] || exit 1
echo 4242
EOF

cat > "$TMP_DIR/bin/killall" <<'EOF'
#!/bin/sh
signal=TERM
process_name=
while [ "$#" -gt 0 ]; do
	case "$1" in
		-q) shift ;;
		-KILL|-9) signal=KILL; shift ;;
		-*) exit 2 ;;
		*) process_name=$1; shift ;;
	esac
done
[ -n "$process_name" ] || exit 2
printf 'NAME-%s %s\n' "$signal" "$process_name" >> "$KILL_LOG"
state_file="$PROCESS_STATE_DIR/$process_name"
[ -f "$state_file" ] || exit 1
mode=$(cat "$state_file")
case "$signal:$mode" in
	TERM:exit_on_term|KILL:exit_on_kill)
		rm -f "$state_file"
		;;
	TERM:unknown_after_term)
		: > "$PROCESS_STATE_DIR/pidof_unknown"
		;;
esac
exit 0
EOF

cat > "$TMP_DIR/bin/pgrep" <<'EOF'
#!/bin/sh
[ "$#" -eq 2 ] && [ "$1" = "-f" ] || exit 2
[ -f "$PROCESS_STATE_DIR/pgrep_unknown" ] && exit 2
[ "$2" = "/usr/bin/ss-monitor" ] || exit 1
[ -f "$PROCESS_STATE_DIR/pgrep_empty_success" ] && exit 0
[ -f "$PROCESS_STATE_DIR/pgrep_malformed" ] && { echo '4243 bad'; exit 0; }
[ -f "$PROCESS_STATE_DIR/pgrep_inconsistent" ] && { echo 4243; exit 1; }
[ -f "$PROCESS_STATE_DIR/monitor" ] || exit 1
echo 4243
EOF

cat > "$TMP_DIR/bin/pkill" <<'EOF'
#!/bin/sh
signal=TERM
[ "$#" -ge 1 ] || exit 2
case "$1" in
	-TERM) signal=TERM; shift ;;
	-KILL|-9) signal=KILL; shift ;;
esac
[ "$#" -eq 2 ] && [ "$1" = "-f" ] && [ "$2" = "/usr/bin/ss-monitor" ] || exit 2
printf 'PATTERN-%s %s\n' "$signal" "$2" >> "$KILL_LOG"
state_file="$PROCESS_STATE_DIR/monitor"
[ -f "$state_file" ] || exit 1
mode=$(cat "$state_file")
case "$signal:$mode" in
	TERM:exit_on_term|KILL:exit_on_kill)
		rm -f "$state_file"
		;;
	TERM:unknown_after_term)
		: > "$PROCESS_STATE_DIR/pgrep_unknown"
		;;
esac
exit 0
EOF

cat > "$TMP_DIR/bin/sleep" <<'EOF'
#!/bin/sh
exit 0
EOF

chmod 755 "$TMP_DIR/bin/pidof" "$TMP_DIR/bin/killall" "$TMP_DIR/bin/pgrep" "$TMP_DIR/bin/pkill" "$TMP_DIR/bin/sleep"
log() { :; }
. "$TEST_DIR/../scripts/shadowsocks_process_lifecycle.sh"

fail() { echo "FAIL: $*" >&2; exit 1; }
expect_status() {
	expected=$1
	label=$2
	shift 2
	if "$@"; then actual=0; else actual=$?; fi
	[ "$actual" -eq "$expected" ] || fail "$label status=$actual expected=$expected"
}

# A successful command must provide a non-empty, canonical PID list.
expect_status 2 "pidof empty success is unknown" ss_process_pid_state empty_success
expect_status 2 "pidof malformed PID output is unknown" ss_process_pid_state malformed
expect_status 2 "pidof failure with output is unknown" ss_process_pid_state inconsistent
: > "$KILL_LOG"
: > "$PROCESS_STATE_DIR/pidof_unknown"
printf 'never_exit\n' > "$PROCESS_STATE_DIR/unknown"
expect_status 2 "pidof error blocks named stop" ss_stop_named_processes unknown
[ ! -s "$KILL_LOG" ] || fail "pidof error caused a signal"
rm -f "$PROCESS_STATE_DIR/pidof_unknown" "$PROCESS_STATE_DIR/unknown"

# If inventory becomes unknown after TERM, do not escalate to KILL.
: > "$KILL_LOG"
printf 'unknown_after_term\n' > "$PROCESS_STATE_DIR/term_unknown"
expect_status 2 "unknown after TERM blocks KILL" ss_stop_named_processes term_unknown
if grep -q '^NAME-KILL term_unknown$' "$KILL_LOG"; then
	fail "named KILL sent after inventory became unknown"
fi
[ -e "$PROCESS_STATE_DIR/pidof_unknown" ] || fail "TERM fixture did not inject the unknown state"
rm -f "$PROCESS_STATE_DIR/pidof_unknown" "$PROCESS_STATE_DIR/term_unknown"

# Preserve the existing graceful / forced / timeout behavior.
: > "$KILL_LOG"
printf 'exit_on_term\n' > "$PROCESS_STATE_DIR/term_exit"
ss_stop_named_processes term_exit
[ ! -e "$PROCESS_STATE_DIR/term_exit" ] || fail "TERM process was not stopped"
grep -q '^NAME-TERM term_exit$' "$KILL_LOG"
if grep -q '^NAME-KILL term_exit$' "$KILL_LOG"; then fail "named stop escalated after TERM exit"; fi

: > "$KILL_LOG"
printf 'exit_on_kill\n' > "$PROCESS_STATE_DIR/force_exit"
ss_stop_named_processes force_exit
[ ! -e "$PROCESS_STATE_DIR/force_exit" ] || fail "KILL escalation did not stop process"
grep -q '^NAME-TERM force_exit$' "$KILL_LOG"
grep -q '^NAME-KILL force_exit$' "$KILL_LOG"

: > "$KILL_LOG"
printf 'never_exit\n' > "$PROCESS_STATE_DIR/stuck"
expect_status 1 "stuck named process remains a stop failure" ss_stop_named_processes stuck
[ -e "$PROCESS_STATE_DIR/stuck" ] || fail "stuck named process fixture disappeared"
grep -q '^NAME-TERM stuck$' "$KILL_LOG"
grep -q '^NAME-KILL stuck$' "$KILL_LOG"

# pgrep has the same strict status/output contract as pidof.
: > "$PROCESS_STATE_DIR/pgrep_empty_success"
expect_status 2 "pgrep empty success is unknown" ss_process_pattern_state '/usr/bin/ss-monitor'
rm -f "$PROCESS_STATE_DIR/pgrep_empty_success"
: > "$PROCESS_STATE_DIR/pgrep_malformed"
expect_status 2 "pgrep malformed PID output is unknown" ss_process_pattern_state '/usr/bin/ss-monitor'
rm -f "$PROCESS_STATE_DIR/pgrep_malformed"
: > "$PROCESS_STATE_DIR/pgrep_inconsistent"
expect_status 2 "pgrep failure with output is unknown" ss_process_pattern_state '/usr/bin/ss-monitor'
rm -f "$PROCESS_STATE_DIR/pgrep_inconsistent"

: > "$KILL_LOG"
: > "$PROCESS_STATE_DIR/pgrep_unknown"
printf 'never_exit\n' > "$PROCESS_STATE_DIR/monitor"
expect_status 2 "pgrep error blocks monitor stop" ss_stop_process_pattern '/usr/bin/ss-monitor'
[ ! -s "$KILL_LOG" ] || fail "pgrep error caused a monitor signal"
rm -f "$PROCESS_STATE_DIR/pgrep_unknown" "$PROCESS_STATE_DIR/monitor"

: > "$KILL_LOG"
printf 'unknown_after_term\n' > "$PROCESS_STATE_DIR/monitor"
expect_status 2 "unknown after monitor TERM blocks KILL" ss_stop_process_pattern '/usr/bin/ss-monitor'
if grep -q '^PATTERN-KILL /usr/bin/ss-monitor$' "$KILL_LOG"; then
	fail "monitor KILL sent after inventory became unknown"
fi
[ -e "$PROCESS_STATE_DIR/pgrep_unknown" ] || fail "monitor TERM fixture did not inject unknown state"
rm -f "$PROCESS_STATE_DIR/pgrep_unknown" "$PROCESS_STATE_DIR/monitor"

: > "$KILL_LOG"
printf 'exit_on_term\n' > "$PROCESS_STATE_DIR/monitor"
ss_stop_process_pattern '/usr/bin/ss-monitor'
[ ! -e "$PROCESS_STATE_DIR/monitor" ] || fail "monitor was not stopped"
grep -q '^PATTERN-TERM /usr/bin/ss-monitor$' "$KILL_LOG"
if grep -q '^PATTERN-KILL /usr/bin/ss-monitor$' "$KILL_LOG"; then fail "monitor escalated after TERM exit"; fi

: > "$KILL_LOG"
printf 'exit_on_kill\n' > "$PROCESS_STATE_DIR/monitor"
ss_stop_process_pattern '/usr/bin/ss-monitor'
[ ! -e "$PROCESS_STATE_DIR/monitor" ] || fail "monitor KILL escalation failed"
grep -q '^PATTERN-TERM /usr/bin/ss-monitor$' "$KILL_LOG"
grep -q '^PATTERN-KILL /usr/bin/ss-monitor$' "$KILL_LOG"

: > "$KILL_LOG"
printf 'never_exit\n' > "$PROCESS_STATE_DIR/monitor"
expect_status 1 "stuck monitor remains a stop failure" ss_stop_process_pattern '/usr/bin/ss-monitor'
[ -e "$PROCESS_STATE_DIR/monitor" ] || fail "stuck monitor fixture disappeared"
grep -q '^PATTERN-TERM /usr/bin/ss-monitor$' "$KILL_LOG"
grep -q '^PATTERN-KILL /usr/bin/ss-monitor$' "$KILL_LOG"

echo "Shadowsocks process lifecycle regression: PASS"
