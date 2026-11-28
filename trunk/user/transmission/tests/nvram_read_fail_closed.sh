#!/bin/sh
set -eu
TEST_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(CDPATH= cd -- "$TEST_DIR/../../../.." && pwd)
TEST_TMP=$(mktemp -d "${TMPDIR:-/tmp}/transmission-nvram.XXXXXX")
BIN="$TEST_TMP/bin"
STORAGE="$TEST_TMP/storage"
NVRAM_LOG="$TEST_TMP/nvram.log"
START_LOG="$TEST_TMP/start.log"
GENERATOR_ARGS="$TEST_TMP/generator.args"
FUNC_FILE="$TEST_TMP/functions.sh"
trap 'rm -rf "$TEST_TMP"' 0
trap 'exit 1' HUP INT TERM
fail() { echo "Transmission checked-NVRAM regression: FAIL: $*" >&2; exit 1; }
sed '/^case "$1" in/,$d' "$ROOT/trunk/user/transmission/transmission.sh" > "$FUNC_FILE"
[ -s "$FUNC_FILE" ] || fail "could not extract Transmission functions"
. "$FUNC_FILE"
logger() { :; }
transmission_process_state() { return 1; }
transmission_wait_started() { return 0; }
cat > "$BIN/nvram" <<'STUB'
#!/bin/sh
[ "$1" = get_checked ] && [ "$#" -eq 2 ] || exit 90
printf '%s\n' "$2" >> "$MOCK_NVRAM_LOG"
[ "$2" = "$MOCK_FAIL_KEY" ] && exit "$MOCK_FAIL_STATUS"
case "$2" in
 trmd_pport) printf '%s\n' 51413 ;;
 trmd_rport) printf '%s\n' 9091 ;;
 http_username) printf '%s\n' transmission-user ;;
 http_passwd) printf '%s\n' transmission-pass ;;
 *) exit 91 ;;
esac
STUB
cat > "$BIN/transmission-daemon" <<'STUB'
#!/bin/sh
printf '%s\n' "$@" > "$MOCK_GENERATOR_ARGS"
printf '"umask": 18,\n' >&2
STUB
cat > "$BIN/start-stop-daemon" <<'STUB'
#!/bin/sh
printf '%s\n' "$*" >> "$MOCK_START_LOG"
exit 0
STUB
mkdir -p "$BIN" "$STORAGE/config" "$STORAGE/downloads" "$STORAGE/incomplete" "$STORAGE/watch"
chmod 755 "$BIN/nvram" "$BIN/transmission-daemon" "$BIN/start-stop-daemon"
PATH="$BIN:$PATH"
export PATH
export MOCK_NVRAM_LOG="$NVRAM_LOG" MOCK_START_LOG="$START_LOG" MOCK_GENERATOR_ARGS="$GENERATOR_ARGS"
MOCK_FAIL_KEY=; MOCK_FAIL_STATUS=0; export MOCK_FAIL_KEY MOCK_FAIL_STATUS
SVC_ROOT=1; SVC_PRIORITY=5; SVC_NAME=Transmission; SVC_PATH="$BIN/transmission-daemon"; DIR_LINK="$STORAGE"
export SVC_ROOT SVC_PRIORITY SVC_NAME SVC_PATH DIR_LINK
reset_fixture() {
  : > "$NVRAM_LOG"; : > "$START_LOG"; : > "$GENERATOR_ARGS"
  rm -f "$STORAGE/config/settings.json" "$STORAGE/config"/settings.json.tmp.*
  mkdir -p "$STORAGE/config" "$STORAGE/downloads" "$STORAGE/incomplete" "$STORAGE/watch"
  CONFIG_TMP=; CONFIG_TMP_OWNED=0
}
assert_no_config_or_launch() {
  [ ! -e "$STORAGE/config/settings.json" ] || fail "failed read published a settings file"
  for candidate in "$STORAGE/config"/settings.json.tmp.*; do
    [ ! -e "$candidate" ] || fail "failed read left a staging file: $candidate"
  done
  [ ! -s "$START_LOG" ] || fail "failed read launched Transmission"
}
check_read_failure() {
  key=$1; expected_status=$2; expected_log=$3
  reset_fixture
  MOCK_FAIL_KEY=$key; MOCK_FAIL_STATUS=$expected_status; export MOCK_FAIL_KEY MOCK_FAIL_STATUS
  if func_start > "$TEST_TMP/start.out" 2>&1; then fail "NVRAM read failure for $key was accepted"; else actual_status=$?; fi
  [ "$actual_status" -eq "$expected_status" ] || fail "$key returned $actual_status, expected $expected_status"
  [ "$(cat "$NVRAM_LOG")" = "$expected_log" ] || fail "$key read ordering changed: $(cat "$NVRAM_LOG")"
  assert_no_config_or_launch
}
check_read_failure trmd_pport 41 "trmd_pport"
check_read_failure trmd_rport 42 "$(printf 'trmd_pport\ntrmd_rport')"
check_read_failure http_username 43 "$(printf 'trmd_pport\ntrmd_rport\nhttp_username')"
check_read_failure http_passwd 44 "$(printf 'trmd_pport\ntrmd_rport\nhttp_username\nhttp_passwd')"

# Successful checked reads preserve configured ports and credentials.
reset_fixture
MOCK_FAIL_KEY=; MOCK_FAIL_STATUS=0; export MOCK_FAIL_KEY MOCK_FAIL_STATUS
if func_start > "$TEST_TMP/success.out" 2>&1; then :; else status=$?; fail "valid settings failed to start (status=$status)"; fi
[ "$(cat "$NVRAM_LOG")" = "$(printf 'trmd_pport\ntrmd_rport\nhttp_username\nhttp_passwd')" ] || fail "expected checked NVRAM reads were not completed"
[ -s "$START_LOG" ] || fail "successful settings did not launch Transmission"
[ -s "$STORAGE/config/settings.json" ] || fail "default settings were not published"
grep -F '"umask": 0,' "$STORAGE/config/settings.json" >/dev/null || fail "default settings transform was not applied"
grep -Fx '-u' "$GENERATOR_ARGS" >/dev/null || fail "RPC username option was not passed"
grep -Fx 'transmission-user' "$GENERATOR_ARGS" >/dev/null || fail "RPC username was not passed"
grep -Fx '-v' "$GENERATOR_ARGS" >/dev/null || fail "RPC password option was not passed"
grep -Fx 'transmission-pass' "$GENERATOR_ARGS" >/dev/null || fail "RPC password was not passed"
echo "Transmission checked NVRAM ports and RPC credentials fail closed before config publication and daemon launch: PASS"
