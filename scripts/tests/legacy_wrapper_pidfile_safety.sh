#!/bin/sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
TEST_TMP=$(mktemp -d "${TMPDIR:-/tmp}/legacy-pidfile.XXXXXX")
TEST_BIN="$TEST_TMP/bin"
SCUT_PIDFILE="$TEST_TMP/scutclient.pid"
MENTO_PIDFILE="$TEST_TMP/mentohust.pid"
KILLALL_LOG="$TEST_TMP/killall.log"
SCUT_LOG="$TEST_TMP/scutclient.log"
mkdir -p "$TEST_BIN"
trap 'rm -rf "$TEST_TMP"' EXIT HUP INT TERM

cat > "$TEST_BIN/killall" <<'MOCK'
#!/bin/sh
printf '%s\n' "$*" >> "$KILLALL_LOG"
exit 0
MOCK
cat > "$TEST_BIN/bin_scutclient" <<'MOCK'
#!/bin/sh
printf '%s\n' "$*" >> "$SCUT_LOG"
exit 0
MOCK
cat > "$TEST_BIN/nvram" <<'MOCK'
#!/bin/sh
[ "$1" = get ] || exit 1
case "$2" in
  wan_ifname) echo br0 ;;
  *) echo '' ;;
esac
MOCK
cat > "$TEST_BIN/logger" <<'MOCK'
#!/bin/sh
exit 0
MOCK
chmod 755 "$TEST_BIN/"*
PATH="$TEST_BIN:$PATH"
export PATH KILLALL_LOG SCUT_LOG

# A stale pidfile can point at an unrelated process. These wrappers do not
# own a persistent shell process, so they must never signal or rewrite it.
printf '%s\n' 2147483647 > "$SCUT_PIDFILE"
SCUTCLIENT_PIDFILE="$SCUT_PIDFILE" sh "$ROOT/trunk/user/scutclient/scripts/scutclient.sh" stop nolog
[ -f "$SCUT_PIDFILE" ] && [ "$(cat "$SCUT_PIDFILE")" = 2147483647 ] || {
  echo "FAIL: scutclient wrapper consumed or rewrote a stale pidfile" >&2; exit 1;
}
[ "$(cat "$SCUT_LOG")" = "-o -i br0" ] || {
  echo "FAIL: scutclient logout action changed" >&2; exit 1;
}

printf '%s\n' 2147483647 > "$MENTO_PIDFILE"
MENTOHUST_PIDFILE="$MENTO_PIDFILE" sh "$ROOT/trunk/user/mentohust/scripts/mentohust.sh" stop
[ -f "$MENTO_PIDFILE" ] && [ "$(cat "$MENTO_PIDFILE")" = 2147483647 ] || {
  echo "FAIL: mentohust wrapper consumed or rewrote a stale pidfile" >&2; exit 1;
}

expected=$(printf '%s\n' '-q -9 bin_scutclient' '-q -9 bin_mentohust')
[ "$(cat "$KILLALL_LOG")" = "$expected" ] || {
  echo "FAIL: daemon stop commands changed unexpectedly" >&2; exit 1;
}

echo "Legacy scutclient/mentohust wrappers never signal stale pidfile PIDs: PASS"
