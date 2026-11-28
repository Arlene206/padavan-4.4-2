#!/bin/sh
set -eu
TEST_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
ROOT=$(CDPATH= cd -- "$TEST_DIR/../../../.." && pwd)
TMP_DIR=$(mktemp -d "${TMPDIR:-/tmp}/aria2-nvram.XXXXXX")
BIN="$TMP_DIR/bin"
NVRAM_LOG="$TMP_DIR/nvram.log"
FUNC_FILE="$TMP_DIR/functions.sh"
trap 'rm -rf "$TMP_DIR"' 0
trap 'exit 1' HUP INT TERM
fail() { echo "Aria2 checked-NVRAM regression: FAIL: $*" >&2; exit 1; }
mkdir -p "$BIN"
sed '/^case "$1" in/,$d' "$ROOT/trunk/user/aria2/aria.sh" > "$FUNC_FILE"
[ -s "$FUNC_FILE" ] || fail "could not extract Aria2 functions"
. "$FUNC_FILE"
logger() { :; }
aria_process_state() { return 1; }
cat > "$BIN/nvram" <<'STUB'
#!/bin/sh
[ "$1" = get_checked ] && [ "$#" -eq 2 ] || exit 90
printf '%s\n' "$2" >> "$ARIA_TEST_NVRAM_LOG"
if [ "$2" = "${ARIA_TEST_FAIL_KEY-}" ]; then exit "${ARIA_TEST_FAIL_STATUS:-1}"; fi
case "$2" in
 aria_pport) printf '%s\n' "${ARIA_TEST_PPORT-16888}" ;;
 aria_rport) printf '%s\n' "${ARIA_TEST_RPORT-6800}" ;;
 http_proto) printf '%s\n' "${ARIA_TEST_HTTP_PROTO-0}" ;;
 *) exit 91 ;;
esac
STUB
chmod 755 "$BIN/nvram"
PATH="$BIN:$PATH"; export PATH
ARIA_TEST_NVRAM_LOG="$NVRAM_LOG"; export ARIA_TEST_NVRAM_LOG
ARIA_TEST_FAIL_KEY=; ARIA_TEST_FAIL_STATUS=0; export ARIA_TEST_FAIL_KEY ARIA_TEST_FAIL_STATUS
DIR_LINK="$TMP_DIR/storage-must-not-exist"; export DIR_LINK
assert_status() { [ "$1" -eq "$2" ] || fail "expected status $1, got $2"; }
check_failure() {
  failed_key=$1; expected_status=$2; expected_reads=$3
  : > "$NVRAM_LOG"
  ARIA_TEST_FAIL_KEY=$failed_key; ARIA_TEST_FAIL_STATUS=$expected_status; export ARIA_TEST_FAIL_KEY ARIA_TEST_FAIL_STATUS
  if func_start > "$TMP_DIR/start.out" 2>&1; then status=0; else status=$?; fi
  assert_status "$expected_status" "$status"
  [ "$(cat "$NVRAM_LOG")" = "$expected_reads" ] || fail "wrong read sequence for $failed_key: $(cat "$NVRAM_LOG")"
  [ ! -e "$DIR_LINK" ] || fail "$failed_key failure reached storage directory operations"
}
check_failure aria_pport 41 "aria_pport"
check_failure aria_rport 42 "$(printf 'aria_pport\naria_rport')"
check_failure http_proto 43 "$(printf 'aria_pport\naria_rport\nhttp_proto')"

# Empty values are successful reads and keep the legacy defaults / TLS choice.
: > "$NVRAM_LOG"
ARIA_TEST_FAIL_KEY=; ARIA_TEST_FAIL_STATUS=0; ARIA_TEST_PPORT=; ARIA_TEST_RPORT=; ARIA_TEST_HTTP_PROTO=0
export ARIA_TEST_FAIL_KEY ARIA_TEST_FAIL_STATUS ARIA_TEST_PPORT ARIA_TEST_RPORT ARIA_TEST_HTTP_PROTO
aria_read_start_settings || fail "successful empty NVRAM values were rejected"
[ "$aria_pport" = 16888 ] || fail "empty peer port lost its default"
[ "$aria_rport" = 6800 ] || fail "empty RPC port lost its default"
[ "$aria_http_proto" = 0 ] || fail "HTTP protocol value was not preserved"
[ "$(cat "$NVRAM_LOG")" = "$(printf 'aria_pport\naria_rport\nhttp_proto')" ] || fail "successful reads were duplicated or reordered"

# Non-empty values are snapshotted without coercion.
: > "$NVRAM_LOG"
ARIA_TEST_PPORT=27888; ARIA_TEST_RPORT=6999; ARIA_TEST_HTTP_PROTO=1
export ARIA_TEST_PPORT ARIA_TEST_RPORT ARIA_TEST_HTTP_PROTO
aria_read_start_settings || fail "valid non-empty NVRAM values were rejected"
[ "$aria_pport" = 27888 ] || fail "peer port changed"
[ "$aria_rport" = 6999 ] || fail "RPC port changed"
[ "$aria_http_proto" = 1 ] || fail "HTTPS selector changed"
echo "Aria2 checked NVRAM ports and HTTP protocol fail closed before storage mutation: PASS"
