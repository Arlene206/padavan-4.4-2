#!/bin/sh
# Host regression for FRP's asynchronous watchdog restart decision.
set -u
repo_root=$(CDPATH= cd -- "$(dirname "$0")/../../../.." && pwd)

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
set -- __test_source__
. "$repo_root/trunk/user/frp/frp.sh" __test_source__

# check_net uses legacy status 1 for online and 2 for offline. Keep this
# fixture deterministic while exercising the real check_frp dispatch logic.
NETWORK_RESULT=1
check_net()
{
	return "$NETWORK_RESULT"
}
CLIENT_PID=
SERVER_PID=
pidof()
{
	case "$1" in
		frpc) [ -n "$CLIENT_PID" ] && printf '%s\n' "$CLIENT_PID" ;;
		frps) [ -n "$SERVER_PID" ] && printf '%s\n' "$SERVER_PID" ;;
	esac
	return 0
}
START_CALLS=0
frp_start()
{
	START_CALLS=$((START_CALLS + 1))
	# Deliberately leave both PIDs absent: frp_script.sh backgrounds both
	# daemons, so pidof may not observe either before the parent returns.
	return 0
}
fail()
{
	printf 'FAIL: %s\n' "$1" >&2
	exit 1
}
expect_start_count()
{
	label=$1
	expected=$2
	check_frp
	[ "$START_CALLS" -eq "$expected" ] ||
		fail "$label: expected $expected frp_start call(s), got $START_CALLS"
}

# Both enabled services missing used to call frp_start twice. Each call
# invokes frp_script.sh, whose first action kills both daemons.
frpc_enable=1
frps_enable=1
CLIENT_PID=
SERVER_PID=
START_CALLS=0
expect_start_count "both enabled services missing" 1

# Either single missing service still requests one complete configured restart.
CLIENT_PID=
SERVER_PID=1201
START_CALLS=0
expect_start_count "client missing" 1
CLIENT_PID=1202
SERVER_PID=
START_CALLS=0
expect_start_count "server missing" 1

# Already-running services and disabled services must not trigger restart.
CLIENT_PID=1203
SERVER_PID=1204
START_CALLS=0
expect_start_count "both services alive" 0
frpc_enable=0
frps_enable=0
CLIENT_PID=
SERVER_PID=
START_CALLS=0
expect_start_count "all services disabled" 0
frpc_enable=0
frps_enable=1
SERVER_PID=1205
START_CALLS=0
expect_start_count "disabled client is not required" 0

# A failed WAN reachability probe must not launch either daemon.
frpc_enable=1
frps_enable=1
CLIENT_PID=
SERVER_PID=
NETWORK_RESULT=2
START_CALLS=0
expect_start_count "WAN unavailable" 0

printf 'FRP watchdog aggregates asynchronous daemon restarts: PASS\n'
