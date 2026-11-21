#!/bin/sh
set -eu
ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../../../.." && pwd)
TEST_TMP=$(mktemp -d "${TMPDIR:-/tmp}/ttyd-lifecycle.XXXXXX")
BIN="$TEST_TMP/bin"; STATE="$TEST_TMP/state"; EVENTS="$TEST_TMP/events"
NVRAM_LOG="$TEST_TMP/nvram"; START_LOG="$TEST_TMP/start"
LIB="${PROCESS_LIFECYCLE_LIB:-$ROOT/trunk/user/shared/scripts/process_lifecycle.sh}"
SCRIPT="${TTYD_SCRIPT:-$ROOT/trunk/user/ttyd/ttyd.sh}"
mkdir -p "$BIN"
trap 'rm -rf "$TEST_TMP"' EXIT HUP INT TERM
cat > "$BIN/pidof" <<'STUB'
#!/bin/sh
case "$(cat "$MOCK_STATE")" in
 running|stubborn) echo 123; exit 0 ;;
 absent) exit 1 ;;
 unknown) exit 0 ;;
 malformed) echo "123 nope"; exit 0 ;;
 inconsistent) echo 123; exit 1 ;;
 error) exit 3 ;;
 *) exit 3 ;;
esac
STUB
cat > "$BIN/killall" <<'STUB'
#!/bin/sh
printf 'killall:%s\n' "$*" >> "$MOCK_EVENTS"
mode=TERM; for arg in "$@"; do [ "$arg" = "-9" ] && mode=KILL; done
if [ "$mode" = TERM ]; then echo "$MOCK_TERM_STATE" > "$MOCK_STATE"; exit "$MOCK_TERM_STATUS"; fi
echo "$MOCK_KILL_STATE" > "$MOCK_STATE"; exit "$MOCK_KILL_STATUS"
STUB
cat > "$BIN/sleep" <<'STUB'
#!/bin/sh
exit "$MOCK_SLEEP_STATUS"
STUB
cat > "$BIN/nvram" <<'STUB'
#!/bin/sh
printf '%s\n' "$*" >> "$MOCK_NVRAM_LOG"
case "$1" in
 get) [ "$2" = ttyd_port ] || exit 9; printf '%s\n' "$MOCK_TTYD_PORT" ;;
 set) exit "$MOCK_NVRAM_SET_STATUS" ;;
 commit) exit "$MOCK_NVRAM_COMMIT_STATUS" ;;
 *) exit 9 ;;
esac
STUB
cat > "$BIN/start-stop-daemon" <<'STUB'
#!/bin/sh
printf '%s\n' "$*" >> "$MOCK_START_LOG"
[ "$MOCK_START_STATUS" -eq 0 ] || exit "$MOCK_START_STATUS"
[ "$MOCK_START_APPEARS" = 1 ] && echo running > "$MOCK_STATE"
exit 0
STUB
cat > "$BIN/logger" <<'STUB'
#!/bin/sh
printf '%s\n' "$*" >> "$MOCK_EVENTS"
STUB
chmod 755 "$BIN/"*
PATH="$BIN:$PATH"; export PATH
export MOCK_STATE="$STATE" MOCK_EVENTS="$EVENTS" MOCK_NVRAM_LOG="$NVRAM_LOG" MOCK_START_LOG="$START_LOG"
export MOCK_TTYD_PORT="" MOCK_START_STATUS=0 MOCK_START_APPEARS=1
export MOCK_TERM_STATE=absent MOCK_KILL_STATE=absent MOCK_TERM_STATUS=0 MOCK_KILL_STATUS=0 MOCK_SLEEP_STATUS=0
export MOCK_NVRAM_SET_STATUS=0 MOCK_NVRAM_COMMIT_STATUS=0
PROCESS_LIFECYCLE_LIB="$LIB"; export PROCESS_LIFECYCLE_LIB
fail(){ echo "ttyd lifecycle regression: FAIL: $*" >&2; exit 1; }
reset(){ : > "$EVENTS"; : > "$NVRAM_LOG"; : > "$START_LOG"; }
run(){ /bin/sh "$SCRIPT" "$1"; }
kills(){ grep -c '^killall:' "$EVENTS" 2>/dev/null || :; }

# ABSENT is a no-op; stop does not read or commit NVRAM.
echo absent > "$STATE"; reset
run stop || fail "ABSENT stop failed"
[ "$(kills)" -eq 0 ] && [ ! -s "$NVRAM_LOG" ] || fail "ABSENT stop had side effects"

# UNKNOWN, malformed, contradictory and failed process scans all fail closed.
for state in unknown malformed inconsistent error; do
 echo "$state" > "$STATE"; reset
 if run stop; then fail "$state stop succeeded"; else rc=$?; fi
 [ "$rc" -eq 2 ] && [ "$(kills)" -eq 0 ] || fail "$state stop signalled or returned $rc"
done

# A fresh ABSENT snapshot overrides killall's status.
echo running > "$STATE"; reset
MOCK_TERM_STATE=absent; MOCK_TERM_STATUS=17; export MOCK_TERM_STATE MOCK_TERM_STATUS
run stop || fail "verified ABSENT after TERM was rejected"
[ "$(kills)" -eq 1 ] && [ "$(grep -c 'killall:-q -9 ttyd' "$EVENTS" 2>/dev/null || :)" -eq 0 ] || fail "unexpected TERM/KILL sequence"

# RUNNING after TERM permits bounded KILL escalation; verified absence completes.
echo running > "$STATE"; reset
MOCK_TERM_STATE=running; MOCK_KILL_STATE=absent; MOCK_TERM_STATUS=0; MOCK_KILL_STATUS=0
export MOCK_TERM_STATE MOCK_KILL_STATE MOCK_TERM_STATUS MOCK_KILL_STATUS
run stop || fail "verified KILL postcondition failed"
[ "$(grep -c 'killall:-q -9 ttyd' "$EVENTS")" -eq 1 ] || fail "KILL escalation missing"

# UNKNOWN after TERM and sleep errors never authorize KILL.
for mode in unknown sleepfail; do
 echo running > "$STATE"; reset
 MOCK_TERM_STATE=running; MOCK_SLEEP_STATUS=0
 [ "$mode" = unknown ] && MOCK_TERM_STATE=unknown
 [ "$mode" = sleepfail ] && MOCK_SLEEP_STATUS=42
 export MOCK_TERM_STATE MOCK_SLEEP_STATUS
 if run stop; then fail "$mode stop succeeded"; else rc=$?; fi
 [ "$rc" -eq 2 ] && [ "$(grep -c 'killall:-q -9 ttyd' "$EVENTS" 2>/dev/null || :)" -eq 0 ] || fail "$mode authorized KILL or returned $rc"
done
MOCK_SLEEP_STATUS=0; export MOCK_SLEEP_STATUS

# A process surviving both bounded stages must return timeout.
echo running > "$STATE"; reset
MOCK_TERM_STATE=running; MOCK_KILL_STATE=running; export MOCK_TERM_STATE MOCK_KILL_STATE
if run stop; then fail "stubborn process stop succeeded"; else rc=$?; fi
[ "$rc" -eq 1 ] || fail "stubborn process returned $rc"

# Duplicate/UNKNOWN starts never spawn a second ttyd.
echo running > "$STATE"; reset
run start || fail "already-running start failed"
[ ! -s "$START_LOG" ] || fail "duplicate start launched"
echo unknown > "$STATE"; reset
if run start; then fail "UNKNOWN start succeeded"; else rc=$?; fi
[ "$rc" -eq 2 ] && [ ! -s "$START_LOG" ] || fail "UNKNOWN start launched or returned $rc"

# Default port write+commit occurs only for confirmed-ABSENT startup.
echo absent > "$STATE"; reset
MOCK_TTYD_PORT=""; MOCK_START_STATUS=0; export MOCK_TTYD_PORT MOCK_START_STATUS
run start || fail "default-port start failed"
[ "$(cat "$NVRAM_LOG")" = "get ttyd_port
set ttyd_port=7681
commit" ] || fail "default port setup was not scoped to start"
[ "$(cat "$START_LOG")" = "-S -b -x ttyd -- -i br0 -p 7681 -W -t disableReconnect=true -t disableLeaveAlert=true login" ] || fail "launch arguments changed"

# Launcher errors propagate; the configured port is not rewritten.
echo absent > "$STATE"; reset
MOCK_TTYD_PORT=4444; MOCK_START_STATUS=37; export MOCK_TTYD_PORT MOCK_START_STATUS
if run start; then fail "launcher failure accepted"; else rc=$?; fi
[ "$rc" -eq 37 ] && [ "$(cat "$NVRAM_LOG")" = "get ttyd_port" ] || fail "launcher status/port handling mismatch"
MOCK_START_STATUS=0; export MOCK_START_STATUS

# Restart cannot launch unless stop verifies ABSENT.
echo running > "$STATE"; reset
MOCK_TERM_STATE=unknown; export MOCK_TERM_STATE
if run restart; then fail "restart ignored UNKNOWN stop"; else rc=$?; fi
[ "$rc" -eq 2 ] && [ ! -s "$START_LOG" ] || fail "restart launched after failed stop"
echo running > "$STATE"; reset
MOCK_TERM_STATE=absent; MOCK_TTYD_PORT=9443; export MOCK_TERM_STATE MOCK_TTYD_PORT
run restart || fail "verified restart failed"
[ "$(cat "$START_LOG")" = "-S -b -x ttyd -- -i br0 -p 9443 -W -t disableReconnect=true -t disableLeaveAlert=true login" ] || fail "restart changed configured port"

# NVRAM write/commit failures prevent launch.
echo absent > "$STATE"; reset
MOCK_TTYD_PORT=""; MOCK_NVRAM_SET_STATUS=41; MOCK_START_STATUS=0
export MOCK_TTYD_PORT MOCK_NVRAM_SET_STATUS MOCK_START_STATUS
if run start; then fail "NVRAM set failure accepted"; else rc=$?; fi
[ "$rc" -eq 41 ] && [ ! -s "$START_LOG" ] || fail "NVRAM set failure started ttyd"
MOCK_NVRAM_SET_STATUS=0; MOCK_NVRAM_COMMIT_STATUS=42; export MOCK_NVRAM_SET_STATUS MOCK_NVRAM_COMMIT_STATUS
if run start; then fail "NVRAM commit failure accepted"; else rc=$?; fi
[ "$rc" -eq 42 ] && [ ! -s "$START_LOG" ] || fail "NVRAM commit failure started ttyd"

echo "ttyd tri-state lifecycle, restart gate and NVRAM initialization regression: PASS"
