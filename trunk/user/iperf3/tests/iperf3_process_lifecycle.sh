#!/bin/sh
set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
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
printf 'killall:%s\n' "$*" >> "$IPERF3_TEST_EVENTS"
exit 99
STUB

cat > "$BIN_DIR/kill" <<'STUB'
#!/bin/sh
printf 'signal:%s %s\n' "$1" "$2" >> "$IPERF3_TEST_EVENTS"
case "${IPERF3_KILL_MODE:-graceful}:$1" in
    term-stalls:-TERM) ;;
    *:-TERM|*:-KILL) rm -rf "$IPERF3_PROC_ROOT/$2" ;;
    *) exit 2 ;;
esac
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
export IPERF3_BIN="$TMP_DIR/iperf3"
export IPERF3_KILL_CMD="$BIN_DIR/kill"
PATH="$BIN_DIR:$PATH"
export PATH

fail() {
    echo "iperf3 process lifecycle regression: FAIL: $*" >&2
    exit 1
}

run_action() {
    IPERF3_PIDOF_MODE=absent sh "$SCRIPT" "$1"
}

# Create a server-shaped procfs fixture. Matching comm alone is insufficient:
# executable path, server argv and start_time must all be available.
write_server() {
    pid="$1"
    mode="${2:-server}"
    mkdir -p "$PROC_ROOT/$pid"
    write_stat "$pid" iperf3
    ln -s "$IPERF3_BIN" "$PROC_ROOT/$pid/exe"
    case "$mode" in
        server) printf '%s\\000-s\\000' "$IPERF3_BIN" > "$PROC_ROOT/$pid/cmdline" ;;
        client) printf '%s\\000-c\\000127.0.0.1\\000' "$IPERF3_BIN" > "$PROC_ROOT/$pid/cmdline" ;;
    esac
}

# Exact executable + server argv is recognized and stopped by PID only.
write_server 321
run_action start || fail "verified server was not accepted as RUNNING"
[ ! -s "$EVENTS" ] || fail "duplicate start was launched for a live server"
run_action stop || fail "verified server was not stopped"
grep -Fqx 'signal:-TERM 321' "$EVENTS" || fail "TERM was not sent to the exact server PID"
[ ! -d "$PROC_ROOT/321" ] || fail "stop fixture did not remove the verified server"
[ "$(grep -c '^killall:' "$EVENTS" || :)" -eq 0 ] || fail "stop used process-name-wide killall"

# Same comm and argv are insufficient when the executable is another file.
: > "$TMP_DIR/not-iperf3"
mkdir -p "$PROC_ROOT/322"
write_stat 322 iperf3
ln -s "$TMP_DIR/not-iperf3" "$PROC_ROOT/322/exe"
printf '%s\\000-s\\000' "$IPERF3_BIN" > "$PROC_ROOT/322/cmdline"
run_action stop || fail "same-name executable impostor should be treated as absent"
[ "$(grep -c '^signal:' "$EVENTS")" -eq 1 ] || fail "same-name impostor was signalled"

# The exact binary running in client mode is not the managed server.
rm -rf "$PROC_ROOT/322"
write_server 323 client
run_action stop || fail "iperf3 client must not be treated as the managed server"
[ "$(grep -c '^signal:' "$EVENTS")" -eq 1 ] || fail "iperf3 client was signalled"

# A matching comm and executable with missing argv metadata is UNKNOWN.
rm -rf "$PROC_ROOT/323"
mkdir -p "$PROC_ROOT/324"
write_stat 324 iperf3
ln -s "$IPERF3_BIN" "$PROC_ROOT/324/exe"
: > "$PROC_ROOT/324/cmdline"
if run_action stop; then
    fail "missing server argv metadata was accepted as ABSENT"
else
    action_status=$?
fi
[ "$action_status" -eq 2 ] || fail "missing argv metadata returned $action_status, expected UNKNOWN"
[ "$(grep -c '^signal:' "$EVENTS")" -eq 1 ] || fail "UNKNOWN argv metadata authorized a signal"

# An incomplete unrelated proc entry is also UNKNOWN and blocks lifecycle ops.
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
[ "$(grep -c '^signal:' "$EVENTS")" -eq 1 ] || fail "UNKNOWN procfs state authorized an extra signal"

# A stubborn verified server receives TERM then KILL after the bounded wait.
rm -rf "$PROC_ROOT/404" "$PROC_ROOT/324"
write_server 325
IPERF3_KILL_MODE=term-stalls run_action stop || fail "stubborn server did not complete force-stop"
[ "$(grep -Fxc 'signal:-TERM 325' "$EVENTS")" -eq 1 ] || fail "graceful TERM was not sent exactly once"
[ "$(grep -Fxc 'signal:-KILL 325' "$EVENTS")" -eq 1 ] || fail "validated KILL escalation was not sent exactly once"
[ ! -d "$PROC_ROOT/325" ] || fail "force-stop fixture did not remove the server"
[ "$(grep -c '^killall:' "$EVENTS" || :)" -eq 0 ] || fail "a stop path used process-name-wide killall"

# Confirmed ABSENT permits the normal launcher; the fake launcher does not
# create a daemon, so the bounded startup readiness check must fail.
if run_action start; then
    action_status=0
else
    action_status=$?
fi
[ "$action_status" -eq 1 ] || fail "mock launcher without a daemon returned $action_status, expected 1"
grep -Fq 'start:-S -b -x ' "$EVENTS" || fail "confirmed ABSENT state did not invoke the launcher"

echo "iperf3 exact executable/argv stop, start_time identity, UNKNOWN inventory, bounded TERM/KILL and startup readiness: PASS"
