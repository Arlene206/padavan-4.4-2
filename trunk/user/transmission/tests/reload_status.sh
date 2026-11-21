#!/bin/sh
set -eu

TEST_DIR=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
SCRIPT="$TEST_DIR/../transmission.sh"
TMP_DIR=$(mktemp -d "${TMPDIR:-/tmp}/transmission-reload.XXXXXX")
trap 'rm -rf "$TMP_DIR"' EXIT HUP INT TERM

sed -n '/^func_reload()/,/^}/p' "$SCRIPT" > "$TMP_DIR/function.sh"
[ -s "$TMP_DIR/function.sh" ] || {
	echo "could not extract Transmission reload function" >&2
	exit 1
}
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
exit "${MOCK_KILLALL_STATUS:-0}"
MOCK_KILLALL
chmod 755 "$TMP_DIR/bin/pidof" "$TMP_DIR/bin/killall"
PATH="$TMP_DIR/bin:$PATH"
export PATH
MOCK_KILLALL_LOG="$TMP_DIR/killall.log"
export MOCK_KILLALL_LOG
logger() { :; }

# A cleanly absent service is a successful no-op and must not invoke killall.
MOCK_PIDOF_STATUS=1
MOCK_PIDOF_OUTPUT=""
export MOCK_PIDOF_STATUS MOCK_PIDOF_OUTPUT
: > "$MOCK_KILLALL_LOG"
func_reload
[ ! -s "$MOCK_KILLALL_LOG" ] || {
	echo "reload signalled when Transmission was absent" >&2
	exit 1
}

# An inventory error or impossible pidof output is UNKNOWN, not ABSENT.
MOCK_PIDOF_STATUS=2
MOCK_PIDOF_OUTPUT=""
export MOCK_PIDOF_STATUS MOCK_PIDOF_OUTPUT
if func_reload >/dev/null; then
	echo "reload accepted an unreadable process inventory" >&2
	exit 1
fi
[ ! -s "$MOCK_KILLALL_LOG" ] || {
	echo "reload signalled after a process inventory error" >&2
	exit 1
}
for bad_output in '' '123 invalid' '0' '1' '123\n456'; do
	MOCK_PIDOF_STATUS=0
	MOCK_PIDOF_OUTPUT="$bad_output"
	export MOCK_PIDOF_STATUS MOCK_PIDOF_OUTPUT
	if func_reload >/dev/null; then
		echo "reload accepted malformed pidof output '$bad_output'" >&2
		exit 1
	fi
	[ ! -s "$MOCK_KILLALL_LOG" ] || {
		echo "reload signalled after malformed pidof output '$bad_output'" >&2
		exit 1
	}
done

# A killall signal failure must be visible to callers and the UI.
MOCK_PIDOF_STATUS=0
MOCK_PIDOF_OUTPUT='123 456'
MOCK_KILLALL_STATUS=29
export MOCK_PIDOF_STATUS MOCK_PIDOF_OUTPUT MOCK_KILLALL_STATUS
: > "$MOCK_KILLALL_LOG"
if func_reload > "$TMP_DIR/failed.out"; then
	echo "reload hid a failed SIGHUP operation" >&2
	exit 1
else
	reload_status=$?
fi
[ "$reload_status" -eq 29 ] || {
	echo "reload returned $reload_status instead of signal status 29" >&2
	exit 1
}
grep -Fq 'Reload Transmission config:.[FAILED]' "$TMP_DIR/failed.out" || {
	echo "failed reload did not report [FAILED]" >&2
	exit 1
}
[ "$(cat "$MOCK_KILLALL_LOG")" = '-SIGHUP transmission-daemon' ] || {
	echo "reload did not invoke the expected SIGHUP command" >&2
	exit 1
}

# A successful signal keeps the original success presentation.
MOCK_KILLALL_STATUS=0
export MOCK_KILLALL_STATUS
: > "$MOCK_KILLALL_LOG"
func_reload > "$TMP_DIR/success.out"
grep -Fq 'Reload Transmission config:.[  OK  ]' "$TMP_DIR/success.out" || {
	echo "successful reload did not report [  OK  ]" >&2
	exit 1
}
[ "$(cat "$MOCK_KILLALL_LOG")" = '-SIGHUP transmission-daemon' ] || {
	echo "successful reload did not invoke the expected SIGHUP command" >&2
	exit 1
}

echo "Transmission reload status regression: PASS"
