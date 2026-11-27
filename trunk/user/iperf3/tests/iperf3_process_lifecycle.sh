#!/bin/sh
set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
HELPER="$SCRIPT_DIR/../../shared/scripts/process_lifecycle.sh"
SCRIPT="$SCRIPT_DIR/../iperf3.sh"
TMP_DIR=$(mktemp -d "${TMPDIR:-/tmp}/iperf3-process.XXXXXX")
trap 'rm -rf "$TMP_DIR"' EXIT HUP INT TERM

BIN_DIR="$TMP_DIR/bin"
PROC_ROOT="$TMP_DIR/proc"
EVENTS="$TMP_DIR/events"
mkdir -p "$BIN_DIR" "$PROC_ROOT/1"
: > "$EVENTS"

write_stat() {
    printf '%s (%s) S 0 1 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 100\n' "$1" "$2" > "$PROC_ROOT/$1/stat"
}

write_stat 1 init

cat > "$BIN_DIR/pidof" <<'STUB'
#!/bin/sh
case "${IPERF3_PIDOF_MODE:-absent}" in
    absent) exit 1 ;;
    running) printf '%s\n' 321; exit 0 ;;
    inconsistent) printf '%s\n' 321; exit 1 ;;
    unknown) exit 2 ;;
    *) exit 2 ;;
esac
STUB

cat > "$BIN_DIR/start-stop-daemon" <<'STUB'
#!/bin/sh
printf 'start:%s\n' "$*" >> "$IPERF3_TEST_EVENTS"
exit "${IPERF3_LAUNCH_STATUS:-0}"
STUB

cat > "$BIN_DIR/killall" <<'STUB'
#!/bin/sh
printf 'stop:%s\n' "$*" >> "$IPERF3_TEST_EVENTS"
rm -rf "$IPERF3_PROC_ROOT/321"
exit "${IPERF3_KILL_STATUS:-0}"
STUB

cat > "$BIN_DIR/sleep" <<'STUB'
#!/bin/sh
exit 0
STUB

cat > "$BIN_DIR/logger" <<'STUB'
#!/bin/sh
exit 0
STUB
chmod 700 "$BIN_DIR/"*

: > "$TMP_DIR/iperf3"
chmod 700 "$TMP_DIR/iperf3"
export IPERF3_TEST_EVENTS="$EVENTS" IPERF3_PROC_ROOT="$PROC_ROOT"
export IPERF3_PROCESS_HELPER="$HELPER" IPERF3_BIN="$TMP_DIR/iperf3"
PATH="$BIN_DIR:$PATH"
export PATH

fail() {
    echo "iperf3 process lifecycle regression: FAIL: $*" >&2
    exit 1
}

run_action() {
    IPERF3_PIDOF_MODE=absent "$SCRIPT" "$1"
}

# BusyBox pidof reports status 1 with empty output when its proc scan omits a
# live process. The shared procfs inventory must find it and prevent duplicate
# startup and premature stop success.
mkdir -p "$PROC_ROOT/321"
write_stat 321 iperf3
run_action start || fail "live daemon was not accepted as RUNNING"
[ ! -s "$EVENTS" ] || fail "duplicate start was launched for a live daemon"

run_action stop || fail "live daemon was not stopped"
grep -Fqx 'stop:-q iperf3' "$EVENTS" || fail "stop did not signal the discovered daemon"
[ ! -d "$PROC_ROOT/321" ] || fail "stop fixture did not remove the daemon"

# An incomplete numeric procfs entry is UNKNOWN, never permission to launch
# or signal a service.
mkdir -p "$PROC_ROOT/404"
if run_action start; then
    fail "startup accepted an incomplete procfs inventory"
else
    action_status=$?
fi
[ "$action_status" -eq 2 ] || fail "incomplete startup inventory returned $action_status, expected 2"
if run_action stop; then
    fail "stop accepted an incomplete procfs inventory"
else
    action_status=$?
fi
[ "$action_status" -eq 2 ] || fail "incomplete stop inventory returned $action_status, expected 2"
[ "$(wc -l < "$EVENTS")" -eq 1 ] || fail "unknown process state authorized an extra launch or signal"

# A complete inventory containing no daemon still returns ABSENT and permits
# the normal launcher path.
rm -rf "$PROC_ROOT/404"
if run_action start; then
    action_status=0
else
    action_status=$?
fi
[ "$action_status" -eq 1 ] || fail "mock launcher without a daemon returned $action_status, expected bounded-start status 1"
grep -Fq 'start:-S -b -x ' "$EVENTS" || fail "confirmed ABSENT state did not invoke the launcher"

echo "iperf3 pidof false-negative, UNKNOWN inventory and bounded lifecycle regression: PASS"
