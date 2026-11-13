#!/bin/sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
source_script="$script_dir/../dns-forwarder.sh"
tmp=$(mktemp -d "${TMPDIR:-/tmp}/dns-forwarder-lifecycle.XXXXXX")
trap 'rm -rf "$tmp"' 0 HUP INT TERM

mkdir -m 700 "$tmp/lock" "$tmp/bin"
sed "s@/tmp/.dns-forwarder.lifecycle.lock@$tmp/lock/.lock@" \
  "$source_script" > "$tmp/dns-forwarder.sh"
chmod 700 "$tmp/dns-forwarder.sh"

cat > "$tmp/bin/nvram" <<'STUB'
#!/bin/sh
[ "$1" = get ] || exit 2
case "$2" in
  dns_forwarder_enable) printf '%s\n' "${FAKE_ENABLED:-1}" ;;
  dns_forwarder_bind) printf '%s\n' "127.0.0.1" ;;
  dns_forwarder_port) printf '%s\n' "5353" ;;
  dns_forwarder_server) printf '%s\n' "8.8.8.8" ;;
  *) exit 2 ;;
esac
STUB
cat > "$tmp/bin/pidof" <<'STUB'
#!/bin/sh
[ "$1" = dns-forwarder ] || exit 2
[ -s "$FAKE_RUNNING_FILE" ] || exit 1
printf '%s\n' 123
STUB
cat > "$tmp/bin/killall" <<'STUB'
#!/bin/sh
[ "$1" = -q ] && [ "$2" = dns-forwarder ] || exit 2
[ "${FAKE_KILL_FAIL:-0}" = 0 ] || exit 1
[ "${FAKE_KILL_KEEP:-0}" = 1 ] || rm -f "$FAKE_RUNNING_FILE"
exit 0
STUB
cat > "$tmp/bin/start-stop-daemon" <<'STUB'
#!/bin/sh
printf '%s\n' "$*" >> "$FAKE_LAUNCH_LOG"
( : >&9 ) 2>/dev/null && exit 97
[ "${FAKE_START_FAIL:-0}" = 0 ] || exit 23
[ "${FAKE_START_NO_PID:-0}" = 1 ] || printf '%s\n' 123 > "$FAKE_RUNNING_FILE"
exit 0
STUB
cat > "$tmp/bin/sleep" <<'STUB'
#!/bin/sh
exit 0
STUB
cat > "$tmp/bin/logger" <<'STUB'
#!/bin/sh
exit 0
STUB
chmod 700 "$tmp/bin/"*

export FAKE_RUNNING_FILE="$tmp/running"
export FAKE_LAUNCH_LOG="$tmp/launch.log"
export FAKE_ENABLED=1 FAKE_KILL_FAIL=0 FAKE_KILL_KEEP=0
export FAKE_START_FAIL=0 FAKE_START_NO_PID=0
PATH="$tmp/bin:/usr/bin:/bin"
export PATH
: > "$FAKE_LAUNCH_LOG"

run_status() {
  expected="$1"
  shift
  if "$tmp/dns-forwarder.sh" "$@" >/dev/null 2>&1; then
    actual=0
  else
    actual=$?
  fi
  [ "$actual" -eq "$expected" ] || {
    echo "expected status $expected, got $actual for $*" >&2
    exit 1
  }
}
launch_count() {
  wc -l < "$FAKE_LAUNCH_LOG" | tr -d ' '
}

# Initial start creates one process; repeated start must be idempotent.
run_status 0 start
[ -s "$FAKE_RUNNING_FILE" ]
[ "$(launch_count)" = 1 ]
grep -F '/usr/bin/dns-forwarder' "$FAKE_LAUNCH_LOG" >/dev/null
run_status 0 start
[ "$(launch_count)" = 1 ]

# Stop is idempotent and waits until the process is actually gone.
run_status 0 stop
[ ! -e "$FAKE_RUNNING_FILE" ]
run_status 0 stop

# A failed signal must abort restart rather than launching a second daemon.
printf '%s\n' 123 > "$FAKE_RUNNING_FILE"
FAKE_KILL_FAIL=1
export FAKE_KILL_FAIL
run_status 1 restart
[ -s "$FAKE_RUNNING_FILE" ]
[ "$(launch_count)" = 1 ]

# A successful signal is not enough if the daemon survives the stop wait.
FAKE_KILL_FAIL=0 FAKE_KILL_KEEP=1
export FAKE_KILL_FAIL FAKE_KILL_KEEP
run_status 1 restart
[ -s "$FAKE_RUNNING_FILE" ]
[ "$(launch_count)" = 1 ]

# A completed stop allows exactly one replacement under the same lock.
FAKE_KILL_KEEP=0
export FAKE_KILL_KEEP
run_status 0 restart
[ -s "$FAKE_RUNNING_FILE" ]
[ "$(launch_count)" = 2 ]

# Disabled restart stops the old process but does not start a new one.
printf '%s\n' 123 > "$FAKE_RUNNING_FILE"
FAKE_ENABLED=0
export FAKE_ENABLED
run_status 0 restart
[ ! -e "$FAKE_RUNNING_FILE" ]
[ "$(launch_count)" = 2 ]

# Report both launch errors and a launcher which exits zero without a daemon.
FAKE_ENABLED=1 FAKE_START_FAIL=1
export FAKE_ENABLED FAKE_START_FAIL
run_status 23 start
[ ! -e "$FAKE_RUNNING_FILE" ]
FAKE_START_FAIL=0 FAKE_START_NO_PID=1
export FAKE_START_FAIL FAKE_START_NO_PID
run_status 1 start
[ ! -e "$FAKE_RUNNING_FILE" ]
FAKE_START_NO_PID=0
export FAKE_START_NO_PID

# A competing holder of the stable lock must prevent service changes.
lock_file="$tmp/lock/.lock"
exec 8>>"$lock_file"
flock -n 8
run_status 1 start
exec 8>&-
[ ! -e "$FAKE_RUNNING_FILE" ]

# Reject symlink, multiply-linked and unsafe-directory lock paths.
rm -f "$lock_file"
: > "$tmp/outside"
ln -s "$tmp/outside" "$lock_file"
run_status 1 stop
rm -f "$lock_file"
: > "$lock_file"
chmod 600 "$lock_file"
ln "$lock_file" "$tmp/lock/extra-link"
run_status 1 stop
rm -f "$tmp/lock/extra-link"
chmod 777 "$tmp/lock"
run_status 1 stop
chmod 700 "$tmp/lock"

echo "DNS Forwarder lifecycle locking, stop gating and start postconditions: PASS"
