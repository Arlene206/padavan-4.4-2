#!/bin/sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname "$0")/../../../.." && pwd)
SCRIPT="$ROOT/trunk/user/ddnsto/ddnsto.sh"
TEST_TMP=$(mktemp -d "${TMPDIR:-/tmp}/ddnsto-lifecycle.XXXXXX")
TEST_BIN="$TEST_TMP/bin"
TEST_STATE="$TEST_TMP/state"
TEST_LOG="$TEST_TMP/killall.log"
TEST_SLEEP_LOG="$TEST_TMP/sleep.log"
TEST_NVRAM_LOG="$TEST_TMP/nvram.log"
TEST_HOOKS="$TEST_TMP/hooks.sh"
export TEST_STATE TEST_LOG TEST_SLEEP_LOG TEST_NVRAM_LOG TEST_TMP SCRIPT TEST_HOOKS
mkdir -p "$TEST_BIN"
trap 'rm -rf "$TEST_TMP"' EXIT HUP INT TERM

fail() {
	echo "DDNSTO lifecycle regression: FAIL: $*" >&2
	exit 1
}

cat > "$TEST_BIN/pidof" <<'MOCK'
#!/bin/sh
[ "$1" = ddnsto ] || exit 1
if [ "$(cat "$TEST_STATE")" = running ]; then
	echo 12345
	exit 0
fi
exit 1
MOCK

cat > "$TEST_BIN/killall" <<'MOCK'
#!/bin/sh
printf '%s\n' "$*" >> "$TEST_LOG"
case "$TEST_MODE:$*" in
	term-exits:ddnsto)
		echo stopped > "$TEST_STATE"
		;;
	force-exits:-9\ ddnsto)
		echo stopped > "$TEST_STATE"
		;;
	*)
		;;
esac
exit 0
MOCK

cat > "$TEST_BIN/sleep" <<'MOCK'
#!/bin/sh
printf '%s\n' "$*" >> "$TEST_SLEEP_LOG"
exit 0
MOCK

cat > "$TEST_BIN/logger" <<'MOCK'
#!/bin/sh
exit 0
MOCK

cat > "$TEST_BIN/nvram" <<'MOCK'
#!/bin/sh
printf '%s\n' "$*" >> "$TEST_NVRAM_LOG"
if [ "$1" = get ] && [ "$2" = ddnsto_id ]; then
	echo regression-id
fi
MOCK
cat > "$TEST_HOOKS" <<'MOCK'
kill() {
	printf '%s\n' "$*" >> "$TEST_TMP/numeric-kill.log"
	return 0
}
MOCK
chmod +x "$TEST_BIN"/*

run_script() {
	TEST_MODE=$1 PATH="$TEST_BIN:$PATH" sh -c '. "$TEST_HOOKS"; . "$SCRIPT"' ddnsto-test "$2"
}

: > "$TEST_LOG"
: > "$TEST_SLEEP_LOG"
: > "$TEST_NVRAM_LOG"
echo stopped > "$TEST_STATE"
run_script unused stop || fail "no-process stop returned failure"
[ ! -s "$TEST_LOG" ] || fail "stop signalled a process when pidof found none"

: > "$TEST_LOG"
: > "$TEST_SLEEP_LOG"
echo running > "$TEST_STATE"
run_script term-exits stop || fail "graceful stop returned failure"
[ "$(cat "$TEST_LOG")" = "ddnsto" ] || fail "graceful stop did not send TERM once without escalation"
[ ! -s "$TEST_SLEEP_LOG" ] || fail "graceful exit was not observed immediately"
[ "$(cat "$TEST_STATE")" = stopped ] || fail "graceful stop postcondition missing"

: > "$TEST_LOG"
: > "$TEST_SLEEP_LOG"
echo running > "$TEST_STATE"
run_script force-exits stop || fail "TERM-resistant process did not stop after escalation"
[ "$(cat "$TEST_LOG")" = "$(printf 'ddnsto\n-9 ddnsto')" ] || fail "force stop did not re-enumerate by process name after bounded TERM wait"
[ "$(wc -l < "$TEST_SLEEP_LOG" | tr -d ' ')" = 5 ] || fail "TERM grace period was not bounded to five one-second polls"
[ "$(cat "$TEST_STATE")" = stopped ] || fail "force-stop postcondition missing"

: > "$TEST_LOG"
: > "$TEST_SLEEP_LOG"
echo running > "$TEST_STATE"
if run_script stuck stop; then
	fail "stop reported success while ddnsto remained alive after SIGKILL"
fi
[ "$(cat "$TEST_LOG")" = "$(printf 'ddnsto\n-9 ddnsto')" ] || fail "stuck process did not receive exactly TERM then name-based KILL"
[ "$(wc -l < "$TEST_SLEEP_LOG" | tr -d ' ')" = 10 ] || fail "TERM/KILL waits were not bounded to five polls each"

: > "$TEST_LOG"
: > "$TEST_SLEEP_LOG"
: > "$TEST_NVRAM_LOG"
echo running > "$TEST_STATE"
if run_script stuck start; then
	fail "start proceeded when previous ddnsto could not be stopped"
fi
[ ! -s "$TEST_NVRAM_LOG" ] || fail "start reached daemon launch argument expansion after stop failure"
[ "$(cat "$TEST_STATE")" = running ] || fail "failed stop unexpectedly changed simulated process state"

[ ! -s "$TEST_TMP/numeric-kill.log" ] || fail "stale numeric PID was signalled after process-name enumeration"
echo "DDNSTO lifecycle stop/start regression: PASS"
