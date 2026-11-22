#!/bin/sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
TEST_TMP=$(mktemp -d "${TMPDIR:-/tmp}/legacy-pidfile.XXXXXX")
TEST_BIN="$TEST_TMP/bin"
SCUT_PIDFILE="$TEST_TMP/scutclient.pid"
MENTO_PIDFILE="$TEST_TMP/mentohust.pid"
KILLALL_LOG="$TEST_TMP/killall.log"
SCUT_LOG="$TEST_TMP/scutclient.log"
START_LOG="$TEST_TMP/start-stop-daemon.log"
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
[ "${1:-}" = "-o" ] && exit "${SCUT_LOGOUT_STATUS:-0}"
credentials_file=
expect_file=0
for arg in "$@"; do
  if [ "$expect_file" -eq 1 ]; then
    credentials_file=$arg
    expect_file=0
    continue
  fi
  case "$arg" in
    --credentials-file) expect_file=1 ;;
    -u|-p) exit 91 ;;
  esac
done
[ -n "$credentials_file" ] || exit 92
[ -f "$credentials_file" ] || exit 93
[ "$(stat -c '%a' "$credentials_file")" = 600 ] || exit 94
[ "$(stat -c '%a' "$(dirname "$credentials_file")")" = 700 ] || exit 95
grep -Fqx "username=$SCUTCLIENT_USERNAME" "$credentials_file" || exit 96
grep -Fqx "password=$SCUTCLIENT_PASSWORD" "$credentials_file" || exit 97
printf 'path=%s\n' "$credentials_file" >> "$SCUT_START_LOG"
if [ "${SCUTCLIENT_MOCK_NO_CONSUME:-0}" -ne 1 ]; then
  rm -f "$credentials_file"
fi
exit 0
MOCK
cat > "$TEST_BIN/nvram" <<'MOCK'
#!/bin/sh
case "$1" in
  get)
    case "$2" in
      scutclient_debug) printf '%s\n' 0 ;;
      scutclient_skip_udp_hb) printf '%s\n' 0 ;;
      scutclient_auth_exec|scutclient_fail_exec) printf '\n' ;;
      wan_dns1_x) printf '%s\n' '1.1.1.1 1.0.0.1' ;;
      scutclient_username) printf '%s\n' "$SCUTCLIENT_USERNAME" ;;
      scutclient_password) printf '%s\n' "$SCUTCLIENT_PASSWORD" ;;
      wan_ifname)
        [ "${SCUT_NVRAM_FAIL_STATUS:-0}" -eq 0 ] || exit "$SCUT_NVRAM_FAIL_STATUS"
        printf '%s\n' br0
        ;;
      scutclient_hostname) printf '%s\n' test-router ;;
      scutclient_server_auth_ip) printf '%s\n' 192.0.2.1 ;;
      scutclient_version) printf '%s\n' 01020304 ;;
      scutclient_hash) printf '%s\n' test-hash ;;
      mentohust_nic) printf '%s\n' "${MENTOHUST_NIC:-br0}" ;;
      mentohust_username) printf '%s\n' "${MENTOHUST_USERNAME:-}" ;;
      mentohust_password) printf '%s\n' "${MENTOHUST_PASSWORD:-}" ;;
      *) printf '\n' ;;
    esac
    ;;
  get_checked)
    case "$2" in
      scutclient_debug) printf '%s\n' 0 ;;
      scutclient_skip_udp_hb) printf '%s\n' 0 ;;
      scutclient_auth_exec|scutclient_fail_exec) printf '\n' ;;
      wan_dns1_x) printf '%s\n' '1.1.1.1 1.0.0.1' ;;
      scutclient_username) printf '%s\n' "$SCUTCLIENT_USERNAME" ;;
      scutclient_password) printf '%s\n' "$SCUTCLIENT_PASSWORD" ;;
      wan_ifname)
        [ "${SCUT_NVRAM_FAIL_STATUS:-0}" -eq 0 ] || exit "$SCUT_NVRAM_FAIL_STATUS"
        printf '%s\n' br0
        ;;
      scutclient_hostname) printf '%s\n' test-router ;;
      scutclient_server_auth_ip) printf '%s\n' 192.0.2.1 ;;
      scutclient_version) printf '%s\n' 01020304 ;;
      scutclient_hash) printf '%s\n' test-hash ;;
      mentohust_nic) printf '%s\n' "${MENTOHUST_NIC:-br0}" ;;
      mentohust_username) printf '%s\n' "${MENTOHUST_USERNAME:-}" ;;
      mentohust_password) printf '%s\n' "${MENTOHUST_PASSWORD:-}" ;;
      *) printf '\n' ;;
    esac
    ;;
  set|commit) exit 0 ;;
  *) exit 1 ;;
esac
MOCK
cat > "$TEST_BIN/logger" <<'MOCK'
#!/bin/sh
exit 0
MOCK
cat > "$TEST_BIN/pidof" <<'MOCK'
#!/bin/sh
exit 1
MOCK
cat > "$TEST_BIN/start-stop-daemon" <<'MOCK'
#!/bin/sh
printf '%s\n' "$*" >> "$START_LOG"
[ "${SCUT_START_CONSUME:-0}" -eq 1 ] || exit 0
exec_path=
while [ "$#" -gt 0 ]; do
  case "$1" in
    -x) exec_path=$2; shift 2 ;;
    --) shift; "$exec_path" "$@"; exit $? ;;
    *) shift ;;
  esac
done
exit 98
MOCK
chmod 755 "$TEST_BIN/"*

PATH="$TEST_BIN:$PATH"
PROCESS_LIFECYCLE_LIB="$ROOT/trunk/user/shared/scripts/process_lifecycle.sh"
export PATH KILLALL_LOG SCUT_LOG START_LOG PROCESS_LIFECYCLE_LIB
export SCUTCLIENT_PIDFILE="$SCUT_PIDFILE" MENTOHUST_PIDFILE="$MENTO_PIDFILE"

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
[ ! -s "$KILLALL_LOG" ] || {
  echo "FAIL: confirmed-absent wrappers sent an unnecessary signal" >&2; exit 1;
}

cat > "$TEST_TMP/process_lifecycle_mock.sh" <<'MOCK'
pl_process_state() { return 1; }
pl_wait_started() { return 0; }
pl_stop_named() { printf '%s\n' "$1" >> "$SCUT_STOP_LOG"; return 0; }
MOCK
PROCESS_LIFECYCLE_LIB="$TEST_TMP/process_lifecycle_mock.sh"
SCUT_STOP_LOG="$TEST_TMP/scut-stop.log"
: > "$SCUT_STOP_LOG"
export PROCESS_LIFECYCLE_LIB SCUT_STOP_LOG

# A transient WAN-interface read failure must leave the local client running;
# otherwise its server-side session cannot be logged out.
SCUT_LOG_LINES_BEFORE=$(wc -l < "$SCUT_LOG" | tr -d ' ')
if SCUT_NVRAM_FAIL_STATUS=37 SCUTCLIENT_PIDFILE="$SCUT_PIDFILE" \
   sh "$ROOT/trunk/user/scutclient/scripts/scutclient.sh" stop nolog >/dev/null; then
  echo "FAIL: scutclient stop accepted an unreadable WAN interface" >&2; exit 1
else
  STOP_STATUS=$?
fi
[ "$STOP_STATUS" -eq 37 ] || { echo "FAIL: scutclient NVRAM read status changed to $STOP_STATUS" >&2; exit 1; }
[ ! -s "$SCUT_STOP_LOG" ] || { echo "FAIL: scutclient stopped before resolving logout interface" >&2; exit 1; }
[ "$(wc -l < "$SCUT_LOG" | tr -d ' ')" = "$SCUT_LOG_LINES_BEFORE" ] || {
  echo "FAIL: scutclient attempted logout with an unknown interface" >&2; exit 1;
}

# Successful logout is part of the stop contract.
SCUT_LOGOUT_STATUS=0 SCUTCLIENT_PIDFILE="$SCUT_PIDFILE" \
  sh "$ROOT/trunk/user/scutclient/scripts/scutclient.sh" stop nolog >/dev/null
[ "$(cat "$SCUT_STOP_LOG")" = "bin_scutclient" ] || {
  echo "FAIL: scutclient stop did not use the lifecycle manager" >&2; exit 1;
}
if SCUT_LOGOUT_STATUS=42 SCUTCLIENT_PIDFILE="$SCUT_PIDFILE" \
   sh "$ROOT/trunk/user/scutclient/scripts/scutclient.sh" stop nolog >/dev/null; then
  echo "FAIL: scutclient reported a failed server logout as success" >&2; exit 1
else
  STOP_STATUS=$?
fi
[ "$STOP_STATUS" -eq 42 ] || { echo "FAIL: scutclient logout failure status changed to $STOP_STATUS" >&2; exit 1; }

# SCUTClient must pass a one-shot credential path, never plaintext argv values.
SCUTCLIENT_USERNAME='alice&literal'
SCUTCLIENT_PASSWORD='test&secret\value%'
SCUT_START_LOG="$TEST_TMP/scutclient-start.log"
export SCUTCLIENT_USERNAME SCUTCLIENT_PASSWORD SCUT_START_LOG
: > "$SCUT_START_LOG"
TMPDIR="$TEST_TMP" SCUT_START_CONSUME=1 SCUTCLIENT_PIDFILE="$SCUT_PIDFILE" \
  sh "$ROOT/trunk/user/scutclient/scripts/scutclient.sh" start nolog >/dev/null
SCUT_CREDENTIAL_PATH=$(sed -n 's/^path=//p' "$SCUT_START_LOG")
[ -n "$SCUT_CREDENTIAL_PATH" ] && [ ! -e "$SCUT_CREDENTIAL_PATH" ] &&
  [ ! -d "$(dirname "$SCUT_CREDENTIAL_PATH")" ] || {
  echo "FAIL: SCUTClient credential file or private directory survived consumption" >&2; exit 1;
}
grep -Fq -- '--credentials-file' "$START_LOG" || {
  echo "FAIL: SCUTClient did not use the private credential-file CLI" >&2; exit 1;
}
! grep -Fq -- "$SCUTCLIENT_USERNAME" "$SCUT_LOG" &&
  ! grep -Fq -- "$SCUTCLIENT_PASSWORD" "$SCUT_LOG" &&
  ! grep -Fq -- "$SCUTCLIENT_USERNAME" "$START_LOG" &&
  ! grep -Fq -- "$SCUTCLIENT_PASSWORD" "$START_LOG" || {
  echo "FAIL: SCUTClient credentials appeared in process argv" >&2; exit 1;
}
START_COUNT=$(wc -l < "$START_LOG" | tr -d ' ')
if TMPDIR="$TEST_TMP" SCUT_START_CONSUME=1 SCUTCLIENT_MOCK_NO_CONSUME=1 \
   SCUTCLIENT_PIDFILE="$SCUT_PIDFILE" \
   sh "$ROOT/trunk/user/scutclient/scripts/scutclient.sh" start nolog >/dev/null 2>&1; then
  echo "FAIL: SCUTClient accepted an unconsumed private credential file" >&2; exit 1;
fi
[ "$(wc -l < "$START_LOG" | tr -d ' ')" -eq $((START_COUNT + 1)) ] || {
  echo "FAIL: SCUTClient credential-consumption failure did not reach the launcher" >&2; exit 1;
}
[ -z "$(find "$TEST_TMP" -maxdepth 3 -name 'credentials.*' -print)" ] || {
  echo "FAIL: SCUTClient left credentials behind after failed consumption" >&2; exit 1;
}
START_COUNT=$(wc -l < "$START_LOG" | tr -d ' ')
SCUTCLIENT_PASSWORD='invalid-secret
Injected=1'
export SCUTCLIENT_PASSWORD
if TMPDIR="$TEST_TMP" SCUT_START_CONSUME=1 SCUTCLIENT_PIDFILE="$SCUT_PIDFILE" \
   sh "$ROOT/trunk/user/scutclient/scripts/scutclient.sh" start nolog >/dev/null 2>&1; then
  echo "FAIL: SCUTClient accepted a newline-bearing password" >&2; exit 1;
fi
[ "$(wc -l < "$START_LOG" | tr -d ' ')" = "$START_COUNT" ] || {
  echo "FAIL: SCUTClient launched with a newline-bearing credential" >&2; exit 1;
}
SCUTCLIENT_PASSWORD=$(printf 'invalid-secret\rInjected=1')
export SCUTCLIENT_PASSWORD
if TMPDIR="$TEST_TMP" SCUT_START_CONSUME=1 SCUTCLIENT_PIDFILE="$SCUT_PIDFILE" \
   sh "$ROOT/trunk/user/scutclient/scripts/scutclient.sh" start nolog >/dev/null 2>&1; then
  echo "FAIL: SCUTClient accepted a carriage-return-bearing password" >&2; exit 1;
fi
[ "$(wc -l < "$START_LOG" | tr -d ' ')" = "$START_COUNT" ] || {
  echo "FAIL: SCUTClient launched with a carriage-return-bearing credential" >&2; exit 1;
}
MENTOHUST_NIC=br0
MENTOHUST_USERNAME='test-user'
MENTOHUST_PASSWORD='test&secret\value%'
export MENTOHUST_NIC MENTOHUST_USERNAME MENTOHUST_PASSWORD

mkdir -p "$TEST_TMP/config" "$TEST_TMP/storage"
CONFIG_FILE="$TEST_TMP/config/mentohust.conf"
BIN_CONFIG="$TEST_TMP/storage/mentohust.conf"
OLD_UMASK=$(umask)
umask 0000
MENTOHUST_CONF_FILE="$CONFIG_FILE" MENTOHUST_BIN_CONF_FILE="$BIN_CONFIG" \
  sh "$ROOT/trunk/user/mentohust/scripts/mentohust.sh" start >/dev/null
umask "$OLD_UMASK"

[ -f "$CONFIG_FILE" ] && [ "$(stat -c '%a' "$CONFIG_FILE")" = 600 ] || {
  echo "FAIL: generated MentoHUST credentials are not stored mode 0600" >&2; exit 1;
}
[ -L "$BIN_CONFIG" ] && [ "$(readlink "$BIN_CONFIG")" = "$CONFIG_FILE" ] || {
  echo "FAIL: generated MentoHUST config link changed" >&2; exit 1;
}
grep -Fqx 'Username=test-user' "$CONFIG_FILE" || {
  echo "FAIL: generated MentoHUST username changed" >&2; exit 1;
}
grep -Fqx 'Password=test&secret\value%' "$CONFIG_FILE" || {
  echo "FAIL: generated MentoHUST password was not written literally" >&2; exit 1;
}
[ "$(wc -l < "$START_LOG" | tr -d ' ')" = 1 ] || {
  echo "FAIL: secure generated config did not launch exactly once" >&2; exit 1;
}

EXISTING_CONFIG="$TEST_TMP/storage/existing.conf"
printf '%s\n' '[MentoHUST]' 'Password=existing-secret' > "$EXISTING_CONFIG"
chmod 666 "$EXISTING_CONFIG"
MENTOHUST_BIN_CONF_FILE="$EXISTING_CONFIG" sh "$ROOT/trunk/user/mentohust/scripts/mentohust.sh" start >/dev/null
[ "$(stat -c '%a' "$EXISTING_CONFIG")" = 600 ] || {
  echo "FAIL: existing MentoHUST config permissions were not restricted" >&2; exit 1;
}

LINK_TARGET="$TEST_TMP/config/keep-me"
LINK_CONFIG="$TEST_TMP/config/symlink.conf"
printf '%s\n' do-not-overwrite > "$LINK_TARGET"
ln -s "$LINK_TARGET" "$LINK_CONFIG"
START_COUNT=$(wc -l < "$START_LOG" | tr -d ' ')
if MENTOHUST_CONF_FILE="$LINK_CONFIG" MENTOHUST_BIN_CONF_FILE="$TEST_TMP/storage/symlink-target.conf" \
   sh "$ROOT/trunk/user/mentohust/scripts/mentohust.sh" start >/dev/null 2>&1; then
  echo "FAIL: MentoHUST accepted a symlink config destination" >&2; exit 1;
fi
[ -L "$LINK_CONFIG" ] && [ "$(cat "$LINK_TARGET")" = do-not-overwrite ] || {
  echo "FAIL: MentoHUST followed or replaced a symlink config destination" >&2; exit 1;
}
[ "$(wc -l < "$START_LOG" | tr -d ' ')" = "$START_COUNT" ] || {
  echo "FAIL: MentoHUST launched after rejecting a symlink config destination" >&2; exit 1;
}

NEWLINE_CONFIG="$TEST_TMP/config/newline.conf"
NEWLINE_BIN_CONFIG="$TEST_TMP/storage/newline.conf"
MENTOHUST_PASSWORD='invalid-secret
Injected=1'
export MENTOHUST_PASSWORD
START_COUNT=$(wc -l < "$START_LOG" | tr -d ' ')
if MENTOHUST_CONF_FILE="$NEWLINE_CONFIG" MENTOHUST_BIN_CONF_FILE="$NEWLINE_BIN_CONFIG" \
   sh "$ROOT/trunk/user/mentohust/scripts/mentohust.sh" start >/dev/null 2>&1; then
  echo "FAIL: MentoHUST accepted a line-break-bearing setting" >&2; exit 1;
fi
[ ! -e "$NEWLINE_CONFIG" ] && [ ! -L "$NEWLINE_CONFIG" ] || {
  echo "FAIL: rejected MentoHUST config was published" >&2; exit 1;
}
[ -z "$(find "$TEST_TMP/config" -maxdepth 1 -name 'newline.conf.*' -print)" ] || {
  echo "FAIL: rejected MentoHUST config left a temporary file" >&2; exit 1;
}
[ "$(wc -l < "$START_LOG" | tr -d ' ')" = "$START_COUNT" ] || {
  echo "FAIL: MentoHUST launched after rejecting a line-break-bearing setting" >&2; exit 1;
}

CR_CONFIG="$TEST_TMP/config/carriage-return.conf"
CR_BIN_CONFIG="$TEST_TMP/storage/carriage-return.conf"
MENTOHUST_PASSWORD=$(printf 'invalid-secret\rInjected=1')
export MENTOHUST_PASSWORD
START_COUNT=$(wc -l < "$START_LOG" | tr -d ' ')
if MENTOHUST_CONF_FILE="$CR_CONFIG" MENTOHUST_BIN_CONF_FILE="$CR_BIN_CONFIG" \
   sh "$ROOT/trunk/user/mentohust/scripts/mentohust.sh" start >/dev/null 2>&1; then
  echo "FAIL: MentoHUST accepted a carriage-return-bearing setting" >&2; exit 1;
fi
[ ! -e "$CR_CONFIG" ] && [ ! -L "$CR_CONFIG" ] || {
  echo "FAIL: rejected carriage-return config was published" >&2; exit 1;
}
[ "$(wc -l < "$START_LOG" | tr -d ' ')" = "$START_COUNT" ] || {
  echo "FAIL: MentoHUST launched after rejecting a carriage-return-bearing setting" >&2; exit 1;
}

echo "Legacy wrappers preserve stale-pidfile safety and SCUTClient/MentoHUST credential confidentiality: PASS"
