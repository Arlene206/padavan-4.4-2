#!/bin/sh
set -eu

repo_root=$(CDPATH= cd -- "$(dirname "$0")/../../../.." && pwd)
tmp=$(mktemp -d "${TMPDIR:-/tmp}/adbyby-stop.XXXXXX")
trap 'rm -rf "$tmp"' EXIT HUP INT TERM

mkdir -p "$tmp/bin"
state_file="$tmp/state"
events_file="$tmp/events"
: > "$events_file"

cat > "$tmp/bin/pidof" <<'STUB'
#!/bin/sh
case "$(cat "$ADBYBY_TEST_STATE")" in
	absent) exit 1 ;;
	error|unknown) exit 2 ;;
	malformed) echo "123 nope"; exit 0 ;;
	running|stubborn) echo "${ADBYBY_TEST_PID_OUTPUT:-123}"; exit 0 ;;
	*) exit 2 ;;
esac
STUB
cat > "$tmp/bin/killall" <<'STUB'
#!/bin/sh
printf 'killall:%s\n' "$*" >> "$ADBYBY_TEST_EVENTS"
exit 0
STUB

cat > "$tmp/bin/sleep" <<'STUB'
#!/bin/sh
printf 'sleep:%s\n' "$*" >> "$ADBYBY_TEST_EVENTS"
[ "${ADBYBY_TEST_SLEEP_MODE:-ok}" = ok ]
STUB
cat > "$tmp/bin/ps" <<'STUB'
#!/bin/sh
exit 0
STUB
chmod 755 "$tmp/bin/"*
PATH="$tmp/bin:$PATH"
ADBYBY_TEST_STATE="$state_file"
ADBYBY_TEST_EVENTS="$events_file"
ADBYBY_TEST_KILL_MODE=keep
ADBYBY_TEST_SLEEP_MODE=ok
export PATH ADBYBY_TEST_STATE ADBYBY_TEST_EVENTS
export ADBYBY_TEST_KILL_MODE ADBYBY_TEST_SLEEP_MODE

# Use an external signal stub: the test also overrides the shell's kill
# builtin later for the unrelated admem helper cleanup.
sed 's/kill -TERM "$adbyby_stop_pid"/fake_kill -TERM "$adbyby_stop_pid"/' \
	"$repo_root/trunk/user/adbyby/adbyby.sh" > "$tmp/adbyby.sh"
cat > "$tmp/bin/fake_kill" <<'STUB'
#!/bin/sh
printf 'kill:%s\n' "$*" >> "$ADBYBY_TEST_EVENTS"
case "$ADBYBY_TEST_KILL_MODE" in
	stop|stop-fail) echo absent > "$ADBYBY_TEST_STATE" ;;
	unknown) echo unknown > "$ADBYBY_TEST_STATE" ;;
esac
[ "${ADBYBY_TEST_KILL_STATUS:-0}" -eq 0 ] && exit 0
exit "$ADBYBY_TEST_KILL_STATUS"
STUB
chmod 755 "$tmp/bin/fake_kill"
. "$tmp/adbyby.sh"

ADBYBY_PROC_ROOT="$tmp/proc"
mkdir -p "$ADBYBY_PROC_ROOT/1"
: > "$ADBYBY_PROC_ROOT/1/cmdline"
printf '1 (init) S 0 1 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 100\n' > "$ADBYBY_PROC_ROOT/1/stat"
export ADBYBY_PROC_ROOT

logger() { :; }
mem_mode=0
del_rule() { echo del_rule >> "$events_file"; }
del_cron() { echo del_cron >> "$events_file"; }
del_dns() { echo del_dns >> "$events_file"; }
adbyby_restart_dhcpd() { echo restart_dhcpd >> "$events_file"; }
addscripts() { echo addscripts >> "$events_file"; }
add_rules() { echo add_rules >> "$events_file"; return 0; }
add_dns() { echo add_dns >> "$events_file"; }
add_rule() { echo add_rule >> "$events_file"; }
hosts_ads() { echo hosts_ads >> "$events_file"; }
PROG_PATH="$tmp/adbyby"
mkdir -p "$PROG_PATH"
printf '%s\n' '#!/bin/sh' 'echo launch >> "$ADBYBY_TEST_EVENTS"' > "$PROG_PATH/adbyby"
chmod 755 "$PROG_PATH/adbyby"
adbyby_test_add_process()
{
	adbyby_test_pid="$1"
	adbyby_test_exe="$2"
	adbyby_test_start="$3"
	mkdir -p "$ADBYBY_PROC_ROOT/$adbyby_test_pid"
	ln -sfn "$adbyby_test_exe" "$ADBYBY_PROC_ROOT/$adbyby_test_pid/exe"
	: > "$ADBYBY_PROC_ROOT/$adbyby_test_pid/cmdline"
	printf '%s (adbyby) S 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 %s\n' \
		"$adbyby_test_pid" "$adbyby_test_start" > "$ADBYBY_PROC_ROOT/$adbyby_test_pid/stat"
}
adbyby_test_add_process 123 "$PROG_PATH/adbyby" 1000
adbyby_test_add_process 456 "$PROG_PATH/adbyby" 1001
cat > "$tmp/bin/iptables-save" <<'STUB'
#!/bin/sh
echo ADBYBY
STUB
chmod 755 "$tmp/bin/iptables-save"
# Model SIGKILL against a procfs target and record its exact PID.
kill()
{
	[ "$#" -eq 2 ] && [ "$1" = "-KILL" ] || return 1
	printf 'mem_helper_kill:%s\n' "$2" >> "$events_file"
	rm -rf "$ADBYBY_PROC_ROOT/$2"
	return 0
}

event_count()
{
	awk -v wanted="$1" '$0 == wanted { count++ } END { print count + 0 }' "$events_file"
}
fail()
{
	echo "Adbyby stop postcondition regression: FAIL: $*" >&2
	exit 1
}
assert_no_cleanup()
{
	if grep -E '^(del_rule|del_cron|del_dns|restart_dhcpd)$' "$events_file" >/dev/null; then
		fail "$1: cleanup/restart ran before confirmed daemon absence"
	fi
}

# The helper is a shell script. Require its exact argv token, a shell
# executable and a stable procfs start_time before signalling a PID.
printf '%s\n' '#!/bin/sh' 'exit 0' > "$PROG_PATH/admem.sh"
chmod 755 "$PROG_PATH/admem.sh"
adbyby_test_add_mem_process()
{
	adbyby_test_mem_pid="$1"
	adbyby_test_mem_exe="$2"
	adbyby_test_mem_arg="$3"
	adbyby_test_mem_start="$4"
	mkdir -p "$ADBYBY_PROC_ROOT/$adbyby_test_mem_pid"
	ln -sfn "$adbyby_test_mem_exe" "$ADBYBY_PROC_ROOT/$adbyby_test_mem_pid/exe"
	printf '/bin/sh\000%s\000' "$adbyby_test_mem_arg" > "$ADBYBY_PROC_ROOT/$adbyby_test_mem_pid/cmdline"
	printf '%s (sh) S 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 %s\n' "$adbyby_test_mem_pid" "$adbyby_test_mem_start" > "$ADBYBY_PROC_ROOT/$adbyby_test_mem_pid/stat"
	if [ "$#" -ge 5 ]; then
		ln -sfn "$5" "$ADBYBY_PROC_ROOT/$adbyby_test_mem_pid/cwd"
	fi
}

: > "$events_file"
adbyby_stop_mem_helper || fail "empty non-helper cmdline blocked memory-helper stop"
if grep -E '^mem_helper_kill:' "$events_file" >/dev/null; then
	fail "empty non-helper cmdline was signalled"
fi
echo "Adbyby memory-helper ignores empty non-candidate cmdline without exe: PASS"

adbyby_test_add_mem_process 321 /bin/busybox "$PROG_PATH/admem.sh" 4000
adbyby_test_add_mem_process 322 /bin/busybox /tmp/unrelated/admem.sh 4001
adbyby_test_add_mem_process 323 /bin/sleep "$PROG_PATH/admem.sh" 4002
adbyby_test_add_mem_process 324 /bin/ash admem.sh 4003 "$PROG_PATH"
: > "$events_file"
adbyby_stop_mem_helper || fail "exact memory-helper cleanup failed"
[ "$(event_count 'mem_helper_kill:321')" -eq 1 ] || fail "absolute-path memory helper was not signalled"
[ "$(event_count 'mem_helper_kill:324')" -eq 1 ] || fail "working-directory-bound relative memory helper was not signalled"
[ -d "$ADBYBY_PROC_ROOT/322" ] || fail "unrelated admem.sh path was removed"
[ -d "$ADBYBY_PROC_ROOT/323" ] || fail "non-shell argv decoy was removed"
[ "$(event_count 'mem_helper_kill:322')" -eq 0 ] || fail "unrelated script path received SIGKILL"
[ "$(event_count 'mem_helper_kill:323')" -eq 0 ] || fail "non-shell decoy received SIGKILL"
echo "Adbyby memory-helper exact argv/executable identity and decoy rejection: PASS"
adbyby_test_add_mem_process 327 /bin/busybox "$PROG_PATH/admem.sh" 4030
rm -f "$ADBYBY_PROC_ROOT/327/exe"
: > "$events_file"
if adbyby_stop_mem_helper; then fail "matching helper argv without executable identity was accepted"; fi
[ "$(event_count 'mem_helper_kill:327')" -eq 0 ] || fail "helper without executable identity was signalled"
rm -rf "$ADBYBY_PROC_ROOT/327"
echo "Adbyby memory-helper matching argv with missing exe metadata fails closed: PASS"


# A matching argv with malformed proc metadata is UNKNOWN and blocks signal.
adbyby_test_add_mem_process 325 /bin/busybox "$PROG_PATH/admem.sh" 4010
printf 'truncated\n' > "$ADBYBY_PROC_ROOT/325/stat"
: > "$events_file"
if adbyby_stop_mem_helper; then fail "malformed memory-helper identity was accepted"; fi
[ "$(event_count 'mem_helper_kill:325')" -eq 0 ] || fail "malformed memory-helper PID received SIGKILL"
rm -rf "$ADBYBY_PROC_ROOT/325"
echo "Adbyby memory-helper malformed proc metadata fails closed: PASS"

# Replace the PID start_time after inventory but before the pre-signal check.
adbyby_test_add_mem_process 326 /bin/busybox "$PROG_PATH/admem.sh" 4020
reuse_trigger="$tmp/mem-reuse-once"
: > "$reuse_trigger"
awk() {
	/usr/bin/awk "$@"
	adbyby_test_awk_status=$?
	case "$*" in
		*"$ADBYBY_PROC_ROOT/326/stat"*)
			if [ -e "$reuse_trigger" ]; then
				rm -f "$reuse_trigger"
				printf '326 (sh) S 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 4021\n' > "$ADBYBY_PROC_ROOT/326/stat"
			fi
			;;
	esac
	return "$adbyby_test_awk_status"
}
: > "$events_file"
if adbyby_stop_mem_helper; then fail "reused memory-helper PID was accepted as stopped"; fi
unset -f awk
[ "$(event_count 'mem_helper_kill:326')" -eq 0 ] || fail "reused memory-helper PID received SIGKILL"
rm -rf "$ADBYBY_PROC_ROOT/326"
echo "Adbyby memory-helper PID reuse blocks stale signalling: PASS"

# Keep the close-order fixture independent from procfs identity details.
adbyby_stop_mem_helper() { echo mem_helper_cleanup >> "$events_file"; return 0; }

# Initial inventory errors and malformed PID output cannot authorize a signal.
for blocked_state in error malformed; do
	echo "$blocked_state" > "$state_file"
	: > "$events_file"
	ADBYBY_TEST_KILL_MODE=keep
	if adbyby_close; then fail "initial $blocked_state inventory was accepted"; fi
	[ "$(event_count 'killall:-q adbyby')" -eq 0 ] || fail "signal sent for $blocked_state"
	assert_no_cleanup "$blocked_state"
done

# A valid but stubborn daemon times out after five bounded polls.
echo stubborn > "$state_file"
: > "$events_file"
ADBYBY_TEST_KILL_MODE=keep
if adbyby_close; then fail "stubborn daemon was accepted as stopped"; fi
[ "$(event_count 'kill:-TERM 123')" -eq 1 ] || fail "TERM count was not one"
[ "$(event_count 'sleep:1')" -eq 5 ] || fail "stop did not make five bounded polls"
assert_no_cleanup stubborn

# UNKNOWN after TERM blocks destructive cleanup.
echo running > "$state_file"
: > "$events_file"
ADBYBY_TEST_KILL_MODE=unknown
if adbyby_close; then fail "post-signal UNKNOWN was accepted"; fi
[ "$(event_count 'kill:-TERM 123')" -eq 1 ] || fail "unexpected signal count after UNKNOWN"
[ "$(event_count 'sleep:1')" -eq 1 ] || fail "UNKNOWN did not stop at the first poll"
assert_no_cleanup post-signal-UNKNOWN

# A failed sleep/wait primitive is not evidence of process exit.
echo running > "$state_file"
: > "$events_file"
ADBYBY_TEST_KILL_MODE=keep
ADBYBY_TEST_SLEEP_MODE=fail
if adbyby_close; then fail "sleep failure was accepted as a stop"; fi
[ "$(event_count 'kill:-TERM 123')" -eq 1 ] || fail "sleep-failure path did not send TERM"
[ "$(event_count 'sleep:1')" -eq 1 ] || fail "sleep failure was not observed once"
assert_no_cleanup sleep-failure
ADBYBY_TEST_SLEEP_MODE=ok

# Multiple validated PIDs are signalled individually; killall stays unused.
echo running > "$state_file"
ADBYBY_TEST_PID_OUTPUT='123 456'
export ADBYBY_TEST_PID_OUTPUT
: > "$events_file"
ADBYBY_TEST_KILL_MODE=stop
adbyby_close || fail "multi-PID stop failed"
[ "$(event_count 'kill:-TERM 123')" -eq 1 ] || fail "first validated PID was not signalled"
[ "$(event_count 'kill:-TERM 456')" -eq 1 ] || fail "second validated PID was not signalled"
[ "$(event_count 'killall:-q adbyby')" -eq 0 ] || fail "stop rescanned process name with killall"
ADBYBY_TEST_PID_OUTPUT=
export ADBYBY_TEST_PID_OUTPUT

# A spoofed comm/pidof match with a different executable is not Adbyby.
echo running > "$state_file"
ADBYBY_TEST_PID_OUTPUT=789
export ADBYBY_TEST_PID_OUTPUT
adbyby_test_add_process 789 /bin/sleep 2000
: > "$events_file"
ADBYBY_TEST_KILL_MODE=keep
adbyby_close || fail "same-name process with another executable blocked confirmed-absent cleanup"
[ "$(event_count 'kill:-TERM 789')" -eq 0 ] || fail "same-name decoy executable was signalled"
echo "same-name decoys with another executable are not signalled: PASS"

# A matching executable with malformed proc metadata is UNKNOWN.
echo running > "$state_file"
ADBYBY_TEST_PID_OUTPUT=790
export ADBYBY_TEST_PID_OUTPUT
adbyby_test_add_process 790 "$PROG_PATH/adbyby" 2010
printf 'truncated\n' > "$ADBYBY_PROC_ROOT/790/stat"
: > "$events_file"
if adbyby_close; then fail "malformed process metadata was accepted as absent"; fi
[ "$(event_count 'kill:-TERM 790')" -eq 0 ] || fail "unknown process identity was signalled"
assert_no_cleanup malformed-process-identity
echo "unverifiable process metadata blocks signal and cleanup: PASS"

# A PID reused by the same executable after inventory must not receive TERM.
echo running > "$state_file"
ADBYBY_TEST_PID_OUTPUT=123
export ADBYBY_TEST_PID_OUTPUT
adbyby_test_add_process 123 "$PROG_PATH/adbyby" 3000
reuse_trigger="$tmp/reuse-once"
: > "$reuse_trigger"
awk()
{
	/usr/bin/awk "$@"
	adbyby_test_awk_status=$?
	case "$*" in
		*"$ADBYBY_PROC_ROOT/123/stat"*)
			if [ -e "$reuse_trigger" ]; then
				rm -f "$reuse_trigger"
				printf '123 (adbyby) S 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 3001\n' > "$ADBYBY_PROC_ROOT/123/stat"
			fi
			;;
	esac
	return "$adbyby_test_awk_status"
}
: > "$events_file"
ADBYBY_TEST_KILL_MODE=keep
if adbyby_close; then fail "PID start_time replacement was accepted as stopped"; fi
unset -f awk
[ "$(event_count 'kill:-TERM 123')" -eq 0 ] || fail "reused PID received TERM"
[ "$(event_count 'sleep:1')" -eq 5 ] || fail "PID-reuse path did not retain bounded stop verification"
assert_no_cleanup pid-reuse
echo "PID start-time replacement is not signalled: PASS"

# Confirmed ABSENT is idempotent and permits cleanup without signalling.
echo absent > "$state_file"
: > "$events_file"
ADBYBY_TEST_KILL_MODE=keep
adbyby_close || fail "confirmed-absent daemon did not stop idempotently"
[ "$(event_count 'killall:-q adbyby')" -eq 0 ] || fail "absent daemon was signalled"
[ "$(event_count del_rule)" -eq 1 ] || fail "firewall cleanup count was not one"
[ "$(event_count del_cron)" -eq 1 ] || fail "cron cleanup count was not one"
[ "$(event_count del_dns)" -eq 1 ] || fail "DNS cleanup count was not one"
[ "$(event_count mem_helper_cleanup)" -eq 1 ] || fail "mem helper cleanup was not reached"
[ "$(event_count restart_dhcpd)" -eq 1 ] || fail "dnsmasq restart count was not one"

# RUNNING followed by a confirmed ABSENT postcondition may safely clean up.
echo running > "$state_file"
: > "$events_file"
ADBYBY_TEST_KILL_MODE=stop
adbyby_close || fail "RUNNING-to-ABSENT stop failed"
[ "$(event_count 'kill:-TERM 123')" -eq 1 ] || fail "successful stop did not send one TERM"
[ "$(event_count 'sleep:1')" -eq 1 ] || fail "successful stop did not verify its postcondition"
[ "$(event_count del_rule)" -eq 1 ] || fail "successful firewall cleanup count was not one"
[ "$(event_count del_cron)" -eq 1 ] || fail "successful cron cleanup count was not one"
[ "$(event_count del_dns)" -eq 1 ] || fail "successful DNS cleanup count was not one"
[ "$(event_count restart_dhcpd)" -eq 1 ] || fail "successful dnsmasq restart count was not one"

# A nonzero killall result is not decisive when the fresh inventory proves ABSENT.
echo running > "$state_file"
: > "$events_file"
ADBYBY_TEST_KILL_MODE=stop-fail
ADBYBY_TEST_KILL_STATUS=29
export ADBYBY_TEST_KILL_STATUS
adbyby_close || fail "failed killall status overrode confirmed ABSENT postcondition"
[ "$(event_count 'kill:-TERM 123')" -eq 1 ] || fail "failed-signal path did not send one TERM"
[ "$(event_count del_rule)" -eq 1 ] || fail "failed-signal path did not complete firewall cleanup"
[ "$(event_count restart_dhcpd)" -eq 1 ] || fail "failed-signal path did not restart dnsmasq"

# updateadb must stop before staging rules or launching a replacement when
# the current daemon remains alive after the bounded stop window.
echo stubborn > "$state_file"
: > "$events_file"
ADBYBY_TEST_KILL_MODE=keep
if adbyby_uprules; then fail "update succeeded while the old daemon remained alive"; fi
if grep -E '^(addscripts|add_rules|launch|add_dns|add_rule|hosts_ads|restart_dhcpd)$' "$events_file" >/dev/null; then
	fail "rule update continued after Adbyby stop failure"
fi

echo "Adbyby stop postcondition and fail-closed cleanup regression: PASS"
