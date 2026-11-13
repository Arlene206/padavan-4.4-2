#!/bin/sh
# Host regression for FRP stop postconditions and RC restart safety.
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
mkdir -p "$FRP_CRONTAB_DIR"
http_username=root
crontab="$FRP_CRONTAB_DIR/root"
watchdog_line='*/1 * * * * /bin/sh /usr/bin/frp.sh C >/dev/null 2>&1'
SIGNAL_LOG="$test_root/signals.log"

CLIENT_RUNNING=0
SERVER_RUNNING=0
TERM_CLIENT_STOP=1
TERM_SERVER_STOP=1
KILL_CLIENT_STOP=1
KILL_SERVER_STOP=1
SLEEP_CALLS=0

pidof()
{
	case "$1" in
		frpc) [ "$CLIENT_RUNNING" = "1" ] && printf '101\n' ;;
		frps) [ "$SERVER_RUNNING" = "1" ] && printf '102\n' ;;
	esac
	return 0
}
killall()
{
	case "$1" in
		-TERM) signal=TERM ;;
		-KILL) signal=KILL ;;
		*) echo "unexpected killall syntax: $*" >&2; return 2 ;;
	esac
	shift
	for target in "$@"; do
		printf '%s:%s\n' "$signal" "$target" >> "$SIGNAL_LOG"
		case "$signal:$target" in
			TERM:frpc) if [ "$TERM_CLIENT_STOP" = "1" ]; then CLIENT_RUNNING=0; fi ;;
			TERM:frps) if [ "$TERM_SERVER_STOP" = "1" ]; then SERVER_RUNNING=0; fi ;;
			KILL:frpc) if [ "$KILL_CLIENT_STOP" = "1" ]; then CLIENT_RUNNING=0; fi ;;
			KILL:frps) if [ "$KILL_SERVER_STOP" = "1" ]; then SERVER_RUNNING=0; fi ;;
		esac
	done
	return 0
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

# Stop must terminate both daemons even while both boot flags remain enabled.
setup_crontab
frpc_enable=1
frps_enable=1
CLIENT_RUNNING=1
SERVER_RUNNING=1
TERM_CLIENT_STOP=1
TERM_SERVER_STOP=1
frp_close || fail "graceful stop failed"
[ "$CLIENT_RUNNING" = "0" ] || fail "enabled FRPC survived stop"
[ "$SERVER_RUNNING" = "0" ] || fail "enabled FRPS survived stop"
grep -Fqx 'TERM:frpc' "$SIGNAL_LOG" || fail "FRPC did not receive TERM"
grep -Fqx 'TERM:frps' "$SIGNAL_LOG" || fail "FRPS did not receive TERM"
if grep -Fq 'KILL:' "$SIGNAL_LOG"; then fail "graceful stop escalated unnecessarily"; fi
[ "$SLEEP_CALLS" -eq 0 ] || fail "graceful stop waited unnecessarily"
expect_watchdog_removed
expect_operator_cron_preserved

# A daemon ignoring TERM is escalated to KILL after five bounded polls.
setup_crontab
CLIENT_RUNNING=1
SERVER_RUNNING=1
TERM_CLIENT_STOP=1
TERM_SERVER_STOP=0
KILL_SERVER_STOP=1
frp_close || fail "stop did not escalate after TERM timeout"
[ "$CLIENT_RUNNING" = "0" ] || fail "FRPC survived graceful TERM"
[ "$SERVER_RUNNING" = "0" ] || fail "FRPS survived KILL escalation"
grep -Fqx 'KILL:frps' "$SIGNAL_LOG" || fail "FRPS was not escalated to KILL"
[ "$SLEEP_CALLS" -eq 5 ] || fail "TERM timeout was not bounded to five polls"
expect_watchdog_removed
expect_operator_cron_preserved

# A process surviving both TERM and KILL must make stop return failure.
setup_crontab
CLIENT_RUNNING=0
SERVER_RUNNING=1
TERM_SERVER_STOP=0
KILL_SERVER_STOP=0
if frp_close; then fail "stubborn FRPS stop was reported successful"; fi
[ "$SERVER_RUNNING" = "1" ] || fail "stubborn process fixture unexpectedly exited"
[ "$SLEEP_CALLS" -eq 7 ] || fail "stop exceeded its five-plus-two poll bound"
expect_watchdog_removed
expect_operator_cron_preserved

# Even if cron publication fails, attempt to stop daemons and report failure.
setup_crontab
CLIENT_RUNNING=1
SERVER_RUNNING=0
TERM_CLIENT_STOP=1
mv() { return 1; }
if frp_close; then fail "failed watchdog removal was reported successful"; fi
unset -f mv
[ "$CLIENT_RUNNING" = "0" ] || fail "cron error prevented daemon termination"
grep -Fqx "$watchdog_line" "$crontab" ||
	fail "failed cron removal unexpectedly replaced the published crontab"
expect_operator_cron_preserved

echo "FRP stop terminates enabled daemons and removes only its watchdog: PASS"
echo "FRP stop escalates TERM to KILL within a bounded interval: PASS"
echo "FRP stop reports stubborn processes and cron cleanup failures: PASS"
