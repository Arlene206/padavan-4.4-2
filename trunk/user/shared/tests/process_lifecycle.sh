#!/bin/sh
set -eu
repo_root=$(CDPATH= cd -- "$(dirname "$0")/../../../.." && pwd)
lib="${PROCESS_LIFECYCLE_LIB:-$repo_root/trunk/user/shared/scripts/process_lifecycle.sh}"
tmp=$(mktemp -d "${TMPDIR:-/tmp}/process-lifecycle.XXXXXX")
trap 'rm -rf "$tmp"' EXIT HUP INT TERM
mkdir -p "$tmp/bin" "$tmp/proc"
state_file="$tmp/state"; events_file="$tmp/events"
PL_PROC_ROOT="$tmp/proc"
export PL_PROC_ROOT
cat > "$tmp/bin/pidof" <<'STUB'
#!/bin/sh
case "$(cat "$PL_TEST_STATE")" in
 running|stubborn) echo 123; exit 0 ;;
 absent) exit 1 ;;
 unknown) exit 2 ;;
 empty-success) exit 0 ;;
 malformed) echo "123 nope"; exit 0 ;;
 inconsistent) echo 123; exit 1 ;;
 *) exit 2 ;;
esac
STUB
cat > "$tmp/bin/killall" <<'STUB'
#!/bin/sh
mode=TERM
for arg in "$@"; do [ "$arg" = "-9" ] && mode=KILL; done
printf '%s:%s\n' "$mode" "$*" >> "$PL_TEST_EVENTS"
if [ "$mode" = TERM ]; then echo "$PL_TERM_STATE" > "$PL_TEST_STATE"; exit "$PL_TERM_STATUS"; fi
echo "$PL_KILL_STATE" > "$PL_TEST_STATE"; exit "$PL_KILL_STATUS"
STUB
cat > "$tmp/bin/pgrep" <<'STUB'
#!/bin/sh
[ "$1" = "-f" ] && [ "$2" = "$PL_TEST_PATTERN" ] || exit 2
case "$(cat "$PL_PATTERN_STATE_FILE")" in
    running) printf '%s\n' 456; exit 0 ;;
    multiple) printf '%s\n' 456 457; exit 0 ;;
    absent) exit 1 ;;
    unknown) exit 2 ;;
    empty-success) exit 0 ;;
    malformed) printf '%s\n' '456 nope'; exit 0 ;;
    pid1) printf '%s\n' 1; exit 0 ;;
    *) exit 2 ;;
esac
STUB
cat > "$tmp/bin/sleep" <<'STUB'
#!/bin/sh
echo sleep >> "$PL_TEST_EVENTS"
if [ -n "$PL_TEST_SLEEP_COUNT" ]; then
    pl_sleep_count=0
    [ -r "$PL_TEST_SLEEP_COUNT" ] && pl_sleep_count=$(cat "$PL_TEST_SLEEP_COUNT")
    pl_sleep_count=$((pl_sleep_count + 1))
    echo "$pl_sleep_count" > "$PL_TEST_SLEEP_COUNT"
    if [ "$PL_TEST_SLEEP_AFTER" -gt 0 ] && [ "$pl_sleep_count" -ge "$PL_TEST_SLEEP_AFTER" ]; then
        echo "$PL_TEST_SLEEP_STATE" > "$PL_TEST_STATE"
    fi
fi
exit "$PL_SLEEP_STATUS"
STUB
chmod 755 "$tmp/bin/"*
PATH="$tmp/bin:$PATH"
PL_TEST_STATE="$state_file"; PL_TEST_EVENTS="$events_file"
PL_PATTERN_STATE_FILE="$tmp/pattern-state"
PL_TEST_PATTERN='(^|[[:space:]])/usr/bin/ipkg[.]sh([[:space:]]|$)'
echo absent > "$PL_PATTERN_STATE_FILE"
export PL_PATTERN_STATE_FILE PL_TEST_PATTERN
PL_TEST_SLEEP_COUNT="$tmp/sleep-count"; PL_TEST_SLEEP_AFTER=0; PL_TEST_SLEEP_STATE=running
PL_TERM_STATE=absent; PL_KILL_STATE=absent
PL_TERM_STATUS=0; PL_KILL_STATUS=0; PL_SLEEP_STATUS=0
export PATH PL_TEST_STATE PL_TEST_EVENTS PL_TEST_SLEEP_COUNT PL_TEST_SLEEP_AFTER PL_TEST_SLEEP_STATE
export PL_TERM_STATE PL_KILL_STATE PL_TERM_STATUS PL_KILL_STATUS PL_SLEEP_STATUS
. "$lib"
count_event() { awk -v wanted="$1" '$0 == wanted { n++ } END { print n + 0 }' "$events_file"; }
fail() { echo "shared process lifecycle regression: FAIL: $*" >&2; exit 1; }

for item in "running:0" "absent:1" "unknown:2" "empty-success:2" "malformed:2" "inconsistent:2"; do
 state="${item%%:*}"; expected="${item#*:}"; echo "$state" > "$state_file"
 if pl_process_state test-daemon; then actual=0; else actual=$?; fi
 [ "$actual" -eq "$expected" ] || fail "$state classified as $actual, expected $expected"
done
# pidof status 1 with empty output is not sufficient to prove absence:
# discover a matching task directly from the complete procfs inventory.
echo absent > "$state_file"
mkdir -p "$PL_PROC_ROOT/321"
printf '%s\n' '321 (test-daemon) S 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19' > "$PL_PROC_ROOT/321/stat"
if pl_process_state test-daemon; then actual=0; else actual=$?; fi
[ "$actual" -eq 0 ] || fail "procfs fallback missed a live daemon"
rm -rf "$PL_PROC_ROOT/321"

# An unreadable stat entry means the snapshot is incomplete, not ABSENT.
mkdir -p "$PL_PROC_ROOT/322/stat"
if pl_process_state test-daemon; then actual=0; else actual=$?; fi
[ "$actual" -eq 2 ] || fail "unreadable proc stat was classified as $actual"
rm -rf "$PL_PROC_ROOT/322"

# Malformed records are UNKNOWN; zombies/dead tasks are not live instances.
mkdir -p "$PL_PROC_ROOT/323"
printf '%s\n' 'not a proc stat record' > "$PL_PROC_ROOT/323/stat"
if pl_process_state test-daemon; then actual=0; else actual=$?; fi
[ "$actual" -eq 2 ] || fail "malformed proc stat was classified as $actual"
printf '%s\n' '323 (test-daemon) BROKEN 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19' > "$PL_PROC_ROOT/323/stat"
if pl_process_state test-daemon; then actual=0; else actual=$?; fi
[ "$actual" -eq 2 ] || fail "invalid proc task state was classified as $actual"
printf '%s\n' '323 (test-daemon) S 1 2 3' > "$PL_PROC_ROOT/323/stat"
if pl_process_state test-daemon; then actual=0; else actual=$?; fi
[ "$actual" -eq 2 ] || fail "truncated proc stat was classified as $actual"
printf '%s\n' '323 (test-daemon) S 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 bad' > "$PL_PROC_ROOT/323/stat"
if pl_process_state test-daemon; then actual=0; else actual=$?; fi
[ "$actual" -eq 2 ] || fail "invalid proc start time was classified as $actual"
printf '%s\n' '999 (test-daemon) S 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19' > "$PL_PROC_ROOT/323/stat"
if pl_process_state test-daemon; then actual=0; else actual=$?; fi
[ "$actual" -eq 2 ] || fail "mismatched proc PID was classified as $actual"
printf '%s\n' '323 (test-daemon) Z 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19' > "$PL_PROC_ROOT/323/stat"
if pl_process_state test-daemon; then actual=0; else actual=$?; fi
[ "$actual" -eq 1 ] || fail "zombie daemon was classified as $actual"

# The library is sourced by long-lived router shell scripts: procfs field
# parsing must not leak temporary IFS or pathname-expansion changes.
echo absent > "$state_file"
pl_test_saved_ifs=$IFS
IFS=:
if pl_process_state test-daemon; then actual=0; else actual=$?; fi
[ "$actual" -eq 1 ] || fail "custom-IFS procfs scan returned $actual"
[ "$IFS" = ":" ] || fail "procfs scan changed the caller's IFS"
unset IFS
if pl_process_state test-daemon; then actual=0; else actual=$?; fi
[ "$actual" -eq 1 ] || fail "unset-IFS procfs scan returned $actual"
[ "${IFS+x}" != x ] || fail "procfs scan changed unset IFS into a set value"
set -- one two
[ "$#" -eq 2 ] || fail "procfs scan changed default field splitting after unset IFS"
IFS=$pl_test_saved_ifs
case "$-" in *f*) fail "test expected pathname expansion to be enabled" ;; esac
set -f
if pl_process_state test-daemon; then actual=0; else actual=$?; fi
[ "$actual" -eq 1 ] || fail "glob-disabled procfs scan returned $actual"
case "$-" in *f*) ;; *) fail "procfs scan enabled pathname expansion for caller" ;; esac
set +f
rm -rf "$PL_PROC_ROOT/323"

# An unavailable procfs root cannot be treated as a complete empty snapshot.
PL_PROC_ROOT="$tmp/missing-proc"
if pl_process_state test-daemon; then actual=0; else actual=$?; fi
[ "$actual" -eq 2 ] || fail "missing procfs root was classified as $actual"
PL_PROC_ROOT="$tmp/proc"
export PL_PROC_ROOT


# Detached startup requires a confirmed RUNNING inventory.
echo running > "$state_file"; echo 0 > "$PL_TEST_SLEEP_COUNT"; PL_TEST_SLEEP_AFTER=0
pl_wait_started test-daemon 1 4 1 || fail "RUNNING start rejected"
[ "$(cat "$PL_TEST_SLEEP_COUNT")" -eq 1 ] || fail "initial start delay mismatch"
echo absent > "$state_file"; echo 0 > "$PL_TEST_SLEEP_COUNT"; PL_TEST_SLEEP_AFTER=2
pl_wait_started test-daemon 1 4 1 || fail "delayed RUNNING start rejected"
[ "$(cat "$PL_TEST_SLEEP_COUNT")" -eq 2 ] || fail "delayed child count mismatch"
echo absent > "$state_file"; echo 0 > "$PL_TEST_SLEEP_COUNT"; PL_TEST_SLEEP_AFTER=0
if pl_wait_started test-daemon 1 4 1; then fail "ABSENT startup timeout accepted"; else status=$?; fi
[ "$status" -eq 1 ] && [ "$(cat "$PL_TEST_SLEEP_COUNT")" -eq 4 ] || fail "ABSENT timeout bounds/status mismatch"
echo unknown > "$state_file"; echo 0 > "$PL_TEST_SLEEP_COUNT"
if pl_wait_started test-daemon 1 4 1; then fail "UNKNOWN inventory accepted"; else status=$?; fi
[ "$status" -eq 2 ] && [ "$(cat "$PL_TEST_SLEEP_COUNT")" -eq 1 ] || fail "UNKNOWN inventory result mismatch"
echo absent > "$state_file"; PL_SLEEP_STATUS=42; echo 0 > "$PL_TEST_SLEEP_COUNT"
if pl_wait_started test-daemon 1 4 1; then fail "interrupted startup wait accepted"; else status=$?; fi
[ "$status" -eq 2 ] || fail "interrupted wait returned $status"
PL_SLEEP_STATUS=0
if pl_wait_started "" 1 4 1; then fail "empty startup name accepted"; else status=$?; fi
[ "$status" -eq 2 ] || fail "empty startup name returned $status"
if pl_wait_started test-daemon 1 0 1; then fail "zero probe count accepted"; else status=$?; fi
[ "$status" -eq 2 ] || fail "zero probe count returned $status"
echo unknown > "$state_file"; : > "$events_file"
if pl_stop_named test-daemon 2 1; then fail "UNKNOWN stop succeeded"; else status=$?; fi
[ "$status" -eq 2 ] && [ ! -s "$events_file" ] || fail "UNKNOWN authorized a signal"

echo absent > "$state_file"; : > "$events_file"
pl_stop_named test-daemon 2 1 || fail "ABSENT stop failed"
[ ! -s "$events_file" ] || fail "ABSENT stop sent signal"

echo running > "$state_file"; : > "$events_file"
PL_TERM_STATE=absent; PL_TERM_STATUS=17
pl_stop_named test-daemon 2 1 || fail "verified ABSENT did not override TERM status"
[ "$(count_event 'TERM:-q test-daemon')" -eq 1 ] || fail "TERM count mismatch"
[ "$(count_event 'KILL:-q -9 test-daemon')" -eq 0 ] || fail "KILL after verified TERM"

echo running > "$state_file"; : > "$events_file"
PL_TERM_STATE=running; PL_TERM_STATUS=0; PL_KILL_STATE=absent
pl_stop_named test-daemon 1 1 || fail "verified KILL exit not accepted"
[ "$(count_event 'TERM:-q test-daemon')" -eq 1 ] || fail "TERM count mismatch"
[ "$(count_event 'KILL:-q -9 test-daemon')" -eq 1 ] || fail "KILL count mismatch"

echo running > "$state_file"; : > "$events_file"; PL_TERM_STATE=unknown
if pl_stop_named test-daemon 2 1; then fail "post-TERM UNKNOWN accepted"; else status=$?; fi
[ "$status" -eq 2 ] || fail "post-TERM UNKNOWN returned $status"
[ "$(count_event 'KILL:-q -9 test-daemon')" -eq 0 ] || fail "UNKNOWN authorized KILL"

echo running > "$state_file"; : > "$events_file"; PL_TERM_STATE=running; PL_SLEEP_STATUS=42
if pl_stop_named test-daemon 2 1; then fail "sleep failure accepted"; else status=$?; fi
[ "$status" -eq 2 ] || fail "sleep failure returned $status"
[ "$(count_event 'KILL:-q -9 test-daemon')" -eq 0 ] || fail "sleep failure authorized KILL"
PL_SLEEP_STATUS=0

echo running > "$state_file"; : > "$events_file"; PL_TERM_STATE=running; PL_KILL_STATE=running
if pl_stop_named test-daemon 1 1; then fail "stubborn daemon accepted"; else status=$?; fi
[ "$status" -eq 1 ] || fail "stubborn stop returned $status"
[ "$(count_event 'KILL:-q -9 test-daemon')" -eq 1 ] || fail "stubborn process not escalated"

# Full-command-line lookup must distinguish running scripts from verified absence.
expect_pattern_state()
{
    pattern_expected="$2"
    if pl_process_pattern_state "$1"; then pattern_actual=0; else pattern_actual=$?; fi
    [ "$pattern_actual" -eq "$pattern_expected" ] ||
        fail "pgrep pattern state returned $pattern_actual, expected $pattern_expected"
}

echo running > "$PL_PATTERN_STATE_FILE"
expect_pattern_state "$PL_TEST_PATTERN" 0
echo multiple > "$PL_PATTERN_STATE_FILE"
expect_pattern_state "$PL_TEST_PATTERN" 0
echo absent > "$PL_PATTERN_STATE_FILE"
expect_pattern_state "$PL_TEST_PATTERN" 1
echo unknown > "$PL_PATTERN_STATE_FILE"
expect_pattern_state "$PL_TEST_PATTERN" 2
echo empty-success > "$PL_PATTERN_STATE_FILE"
expect_pattern_state "$PL_TEST_PATTERN" 2
echo malformed > "$PL_PATTERN_STATE_FILE"
expect_pattern_state "$PL_TEST_PATTERN" 2
echo pid1 > "$PL_PATTERN_STATE_FILE"
expect_pattern_state "$PL_TEST_PATTERN" 2
expect_pattern_state "" 2
echo "shared process lifecycle regression: PASS"
