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
esac
exit 0
EOF

cat > "$TMP_DIR/bin/pgrep" <<'EOF'
#!/bin/sh
[ "$#" -eq 2 ] && [ "$1" = "-f" ] || exit 2
[ "$2" = "/usr/bin/ss-monitor" ] || exit 1
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

: > "$KILL_LOG"
ss_stop_named_processes absent
[ ! -s "$KILL_LOG" ] || { echo "unexpected signal for absent process" >&2; exit 1; }

printf 'exit_on_term\n' > "$PROCESS_STATE_DIR/term_exit"
ss_stop_named_processes term_exit
[ ! -e "$PROCESS_STATE_DIR/term_exit" ] || { echo "TERM process was not stopped" >&2; exit 1; }
grep -q '^NAME-TERM term_exit$' "$KILL_LOG"
if grep -q '^NAME-KILL term_exit$' "$KILL_LOG"; then
	echo "escalated after process had already exited" >&2
	exit 1
fi

: > "$KILL_LOG"
printf 'exit_on_kill\n' > "$PROCESS_STATE_DIR/force_exit"
ss_stop_named_processes force_exit
[ ! -e "$PROCESS_STATE_DIR/force_exit" ] || { echo "KILL escalation did not stop process" >&2; exit 1; }
grep -q '^NAME-TERM force_exit$' "$KILL_LOG"
grep -q '^NAME-KILL force_exit$' "$KILL_LOG"

: > "$KILL_LOG"
printf 'never_exit\n' > "$PROCESS_STATE_DIR/stuck"
if ss_stop_named_processes stuck; then
	echo "stuck process unexpectedly reported stopped" >&2
	exit 1
fi
[ -e "$PROCESS_STATE_DIR/stuck" ] || { echo "stuck fixture unexpectedly disappeared" >&2; exit 1; }
grep -q '^NAME-TERM stuck$' "$KILL_LOG"
grep -q '^NAME-KILL stuck$' "$KILL_LOG"

: > "$KILL_LOG"
printf 'exit_on_term\n' > "$PROCESS_STATE_DIR/monitor"
ss_stop_process_pattern '/usr/bin/ss-monitor'
[ ! -e "$PROCESS_STATE_DIR/monitor" ] || { echo "monitor was not stopped" >&2; exit 1; }
grep -q '^PATTERN-TERM /usr/bin/ss-monitor$' "$KILL_LOG"
if grep -q '^PATTERN-KILL /usr/bin/ss-monitor$' "$KILL_LOG"; then
	echo "monitor escalation occurred after TERM exit" >&2
	exit 1
fi

: > "$KILL_LOG"
printf 'exit_on_kill\n' > "$PROCESS_STATE_DIR/monitor"
ss_stop_process_pattern '/usr/bin/ss-monitor'
[ ! -e "$PROCESS_STATE_DIR/monitor" ] || { echo "monitor KILL escalation failed" >&2; exit 1; }
grep -q '^PATTERN-TERM /usr/bin/ss-monitor$' "$KILL_LOG"
grep -q '^PATTERN-KILL /usr/bin/ss-monitor$' "$KILL_LOG"

: > "$KILL_LOG"
printf 'never_exit\n' > "$PROCESS_STATE_DIR/monitor"
if ss_stop_process_pattern '/usr/bin/ss-monitor'; then
	echo "stuck monitor unexpectedly reported stopped" >&2
	exit 1
fi
[ -e "$PROCESS_STATE_DIR/monitor" ] || { echo "stuck monitor fixture disappeared" >&2; exit 1; }
grep -q '^PATTERN-TERM /usr/bin/ss-monitor$' "$KILL_LOG"
grep -q '^PATTERN-KILL /usr/bin/ss-monitor$' "$KILL_LOG"

echo "Shadowsocks process lifecycle regression: PASS"
