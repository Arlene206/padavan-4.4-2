#!/bin/sh
# Host regression for FRP exact process identity and bounded stop postconditions.
set -eu
repo_root=$(CDPATH= cd -- "$(dirname "$0")/../../../.." && pwd)
test_root=$(mktemp -d /tmp/frp-stop-test.XXXXXX)
trap 'rm -rf "$test_root"' EXIT HUP INT TERM

TEST_FRPC_ENABLE=1
TEST_FRPS_ENABLE=1
TEST_HTTP_USERNAME=root
nvram()
{
	case "$2" in
		frpc_enable) printf '%s\n' "$TEST_FRPC_ENABLE" ;;
		frps_enable) printf '%s\n' "$TEST_FRPS_ENABLE" ;;
		http_username) printf '%s\n' "$TEST_HTTP_USERNAME" ;;
		*) printf '\n' ;;
	esac
}
logger() { :; }
set -- __test_source__
. "$repo_root/trunk/user/frp/frp.sh" __test_source__

FRP_CRON_ROOT="$test_root/cron"
FRP_CRONTAB_DIR="$FRP_CRON_ROOT/crontabs"
FRP_PROC_ROOT="$test_root/proc"
mkdir -p "$FRP_CRONTAB_DIR" "$FRP_PROC_ROOT"
http_username=root
crontab="$FRP_CRONTAB_DIR/root"
watchdog_line='*/1 * * * * /bin/sh /usr/bin/frp.sh C >/dev/null 2>&1'
SIGNAL_LOG="$test_root/signals.log"

TERM_CLIENT_STOP=1
TERM_SERVER_STOP=1
KILL_CLIENT_STOP=1
KILL_SERVER_STOP=1
SLEEP_CALLS=0
KILLALL_CALLS=0

add_process()
{
	pid="$1"
	service="$2"
	config="$3"
	mkdir -p "$FRP_PROC_ROOT/$pid"
	printf '/usr/bin/%s\000-c\000%s\000' "$service" "$config" > "$FRP_PROC_ROOT/$pid/cmdline"
	ln -sfn "/usr/bin/$service" "$FRP_PROC_ROOT/$pid/exe"
}
remove_process()
{
	rm -f "$FRP_PROC_ROOT/$1/cmdline"
}
process_exists()
{
	[ -r "$FRP_PROC_ROOT/$1/cmdline" ] &&
		[ -s "$FRP_PROC_ROOT/$1/cmdline" ]
}
kill()
{
	signal="$1"
	pid="$2"
	printf '%s:%s\n' "$signal" "$pid" >> "$SIGNAL_LOG"
	case "$signal:$pid" in
		-TERM:101) [ "$TERM_CLIENT_STOP" = "1" ] && remove_process 101 ;;
		-TERM:102) [ "$TERM_SERVER_STOP" = "1" ] && remove_process 102 ;;
		-KILL:101) [ "$KILL_CLIENT_STOP" = "1" ] && remove_process 101 ;;
		-KILL:102) [ "$KILL_SERVER_STOP" = "1" ] && remove_process 102 ;;
		*) return 2 ;;
	esac
	return 0
}
killall()
{
	KILLALL_CALLS=$((KILLALL_CALLS + 1))
	return 97
}
sleep()
{
	SLEEP_CALLS=$((SLEEP_CALLS + 1))
	return 0
}
fail() { echo "FRP stop regression: FAIL: $*" >&2; exit 1; }
setup_crontab()
{
	printf '%s\n' 'SHELL=/bin/sh' "$watchdog_line" \
		'*/5 * * * * /usr/local/sbin/operator-job' > "$crontab"
	: > "$SIGNAL_LOG"
	SLEEP_CALLS=0
}
clear_processes()
{
	rm -rf "$FRP_PROC_ROOT"
	mkdir -p "$FRP_PROC_ROOT"
}
expect_operator_cron_preserved()
{
	grep -Fqx 'SHELL=/bin/sh' "$crontab" || fail "crontab SHELL setting was lost"
	grep -Fqx '*/5 * * * * /usr/local/sbin/operator-job' "$crontab" ||
		fail "operator-owned cron entry was lost"
}
expect_watchdog_removed()
{
	if grep -Fqx "$watchdog_line" "$crontab"; then
		fail "stopped FRP watchdog entry remains scheduled"
	fi
}

# Stop must terminate exact-config daemons even while both boot flags remain enabled.
setup_crontab
add_process 101 frpc /tmp/frp/myfrpc.ini
add_process 102 frps /tmp/frp/myfrps.ini
frp_close || fail "graceful stop failed"
process_exists 101 && fail "FRPC survived stop"
process_exists 102 && fail "FRPS survived stop"
grep -Fqx -- '-TERM:101' "$SIGNAL_LOG" || fail "FRPC did not receive TERM"
grep -Fqx -- '-TERM:102' "$SIGNAL_LOG" || fail "FRPS did not receive TERM"
if grep -Fq -- '-KILL:' "$SIGNAL_LOG"; then fail "graceful stop escalated unnecessarily"; fi
[ "$SLEEP_CALLS" -eq 0 ] || fail "graceful stop waited unnecessarily"
[ "$KILLALL_CALLS" -eq 0 ] || fail "name-based killall was invoked"
expect_watchdog_removed
expect_operator_cron_preserved

# A daemon ignoring TERM is escalated to KILL after five bounded polls.
setup_crontab
clear_processes
add_process 101 frpc /tmp/frp/myfrpc.ini
add_process 102 frps /tmp/frp/myfrps.ini
TERM_CLIENT_STOP=1
TERM_SERVER_STOP=0
KILL_SERVER_STOP=1
frp_close || fail "stop did not escalate after TERM timeout"
process_exists 101 && fail "FRPC survived graceful TERM"
process_exists 102 && fail "FRPS survived KILL escalation"
grep -Fqx -- '-KILL:102' "$SIGNAL_LOG" || fail "FRPS was not escalated to KILL"
[ "$SLEEP_CALLS" -eq 5 ] || fail "TERM timeout was not bounded to five polls"
expect_watchdog_removed
expect_operator_cron_preserved

# A process surviving both TERM and KILL must make stop return failure.
setup_crontab
clear_processes
add_process 102 frps /tmp/frp/myfrps.ini
TERM_SERVER_STOP=0
KILL_SERVER_STOP=0
if frp_close; then fail "stubborn FRPS stop was reported successful"; fi
process_exists 102 || fail "stubborn process fixture unexpectedly exited"
[ "$SLEEP_CALLS" -eq 7 ] || fail "stop exceeded its five-plus-two poll bound"
expect_watchdog_removed
expect_operator_cron_preserved

# Same-name processes with another config or extra argv are not owned by FRP.
setup_crontab
clear_processes
mkdir -p "$FRP_PROC_ROOT/201" "$FRP_PROC_ROOT/202" "$FRP_PROC_ROOT/203"
printf '/usr/bin/frpc\000-c\000/tmp/frp/operator.ini\000' > "$FRP_PROC_ROOT/201/cmdline"
ln -sfn /usr/bin/frpc "$FRP_PROC_ROOT/201/exe"
printf '/usr/bin/frps\000-c\000/tmp/frp/myfrps.ini\000--unexpected\000' > "$FRP_PROC_ROOT/202/cmdline"
ln -sfn /usr/bin/frps "$FRP_PROC_ROOT/202/exe"
printf '/usr/bin/frpc-wrapper\000-c\000/tmp/frp/myfrpc.ini\000' > "$FRP_PROC_ROOT/203/cmdline"
ln -sfn /usr/bin/frpc-wrapper "$FRP_PROC_ROOT/203/exe"
frp_close || fail "unowned same-name decoys made FRP stop fail"
[ ! -s "$SIGNAL_LOG" ] || fail "unowned process received a signal"
process_exists 201 || fail "wrong-config FRPC decoy was altered"
process_exists 202 || fail "extra-argv FRPS decoy was altered"
process_exists 203 || fail "wrong-executable FRPC decoy was altered"
 
# An unrelated executable can copy the exact argv tuple; it still is not FRPC.
mkdir -p "$FRP_PROC_ROOT/204"
printf '/usr/bin/frpc\000-c\000/tmp/frp/myfrpc.ini\000' > "$FRP_PROC_ROOT/204/cmdline"
ln -sfn /bin/sleep "$FRP_PROC_ROOT/204/exe"
process_exists 204 || fail "argv-matching impostor executable was altered"
[ "$KILLALL_CALLS" -eq 0 ] || fail "name-based killall was invoked"

# Even if cron publication fails, attempt to stop owned daemons and report failure.
setup_crontab
clear_processes
add_process 101 frpc /tmp/frp/myfrpc.ini
TERM_CLIENT_STOP=1
mv() { return 1; }
if frp_close; then fail "failed watchdog removal was reported successful"; fi
unset -f mv
process_exists 101 && fail "cron error prevented daemon termination"
grep -Fqx "$watchdog_line" "$crontab" ||
	fail "failed cron removal unexpectedly replaced the published crontab"
expect_operator_cron_preserved

echo "FRP stop matches exact daemon argv and preserves same-name decoys: PASS"
echo "FRP stop escalates TERM to KILL within a bounded interval: PASS"
echo "FRP stop reports stubborn processes and cron cleanup failures: PASS"
