#!/bin/sh
# Host regression for FRP exact process state and asynchronous start readiness.
set -eu
repo_root=$(CDPATH= cd -- "$(dirname "$0")/../../../.." && pwd)
test_root=$(mktemp -d /tmp/frp-lifecycle-test.XXXXXX)
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

FRP_PROC_ROOT="$test_root/proc"
mkdir -p "$FRP_PROC_ROOT"
NETWORK_RESULT=1
check_net() { return "$NETWORK_RESULT"; }
LAUNCH_CALLS=0
WATCHDOG_ADDS=0
WATCHDOG_REMOVES=0
SLEEP_CALLS=0
TEST_LAUNCH_MODE=immediate
CLIENT_TERM=1
SERVER_TERM=1

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
clear_processes()
{
	rm -rf "$FRP_PROC_ROOT"
	mkdir -p "$FRP_PROC_ROOT"
}
kill()
{
	signal="$1"
	pid="$2"
	case "$signal:$pid" in
		-TERM:301) [ "$CLIENT_TERM" = "1" ] && remove_process 301 ;;
		-TERM:302) [ "$SERVER_TERM" = "1" ] && remove_process 302 ;;
		-KILL:301) remove_process 301 ;;
		-KILL:302) remove_process 302 ;;
		*) return 2 ;;
	esac
	return 0
}
sleep()
{
	SLEEP_CALLS=$((SLEEP_CALLS + 1))
	if [ "$TEST_LAUNCH_MODE" = delayed ] && [ "$SLEEP_CALLS" -eq 2 ]; then
		add_process 301 frpc /tmp/frp/myfrpc.ini
		add_process 302 frps /tmp/frp/myfrps.ini
	fi
	return 0
}
frp_run_script()
{
	LAUNCH_CALLS=$((LAUNCH_CALLS + 1))
	case "$TEST_LAUNCH_MODE" in
		immediate)
			[ "$frpc_enable" != "1" ] || add_process 301 frpc /tmp/frp/myfrpc.ini
			[ "$frps_enable" != "1" ] || add_process 302 frps /tmp/frp/myfrps.ini
			;;
		delayed) : ;;
		partial)
			[ "$frpc_enable" != "1" ] || add_process 301 frpc /tmp/frp/myfrpc.ini
			;;
		*) return 2 ;;
	esac
	return 0
}
frp_manage_watchdog_cron()
{
	case "$1" in
		add) WATCHDOG_ADDS=$((WATCHDOG_ADDS + 1)) ;;
		remove) WATCHDOG_REMOVES=$((WATCHDOG_REMOVES + 1)) ;;
		*) return 2 ;;
	esac
	return 0
}
fail() { echo "FRP lifecycle regression: FAIL: $*" >&2; exit 1; }
expect_check_start()
{
	label="$1"
	expected="$2"
	check_frp || fail "$label: check_frp propagated a launch failure"
	[ "$LAUNCH_CALLS" -eq "$expected" ] ||
		fail "$label: expected $expected launch(es), got $LAUNCH_CALLS"
}

clear_processes
TEST_LAUNCH_MODE=immediate
LAUNCH_CALLS=0
WATCHDOG_ADDS=0
SLEEP_CALLS=0
frpc_enable=1
frps_enable=1
expect_check_start "both enabled services missing" 1
frp_service_running frpc || fail "FRPC did not become visible"
frp_service_running frps || fail "FRPS did not become visible"
[ "$WATCHDOG_ADDS" -eq 1 ] || fail "watchdog was not published after readiness"

# Either missing role triggers one replacement, stopping the prior exact peer first.
clear_processes
add_process 302 frps /tmp/frp/myfrps.ini
TEST_LAUNCH_MODE=immediate
LAUNCH_CALLS=0
expect_check_start "client missing" 1
frp_service_running frpc && frp_service_running frps ||
	fail "combined replacement did not restore both enabled roles"

clear_processes
add_process 301 frpc /tmp/frp/myfrpc.ini
TEST_LAUNCH_MODE=immediate
LAUNCH_CALLS=0
expect_check_start "server missing" 1
frp_service_running frpc && frp_service_running frps ||
	fail "combined replacement did not restore both enabled roles"

# Exact-argv live processes suppress restart; unrelated same-name argv does not.
clear_processes
add_process 301 frpc /tmp/frp/myfrpc.ini
add_process 302 frps /tmp/frp/myfrps.ini
TEST_LAUNCH_MODE=immediate
LAUNCH_CALLS=0
expect_check_start "both services alive" 0
clear_processes
mkdir -p "$FRP_PROC_ROOT/401"
printf '/usr/bin/frpc\000-c\000/tmp/frp/other.ini\000' > "$FRP_PROC_ROOT/401/cmdline"
ln -sfn /usr/bin/frpc "$FRP_PROC_ROOT/401/exe"
LAUNCH_CALLS=0
expect_check_start "wrong-config process is not alive FRPC" 1
 
# argv[0] can be forged by an unrelated executable; it must not count as FRPC.
clear_processes
mkdir -p "$FRP_PROC_ROOT/402"
printf '/usr/bin/frpc\000-c\000/tmp/frp/myfrpc.ini\000' > "$FRP_PROC_ROOT/402/cmdline"
ln -sfn /bin/sleep "$FRP_PROC_ROOT/402/exe"
LAUNCH_CALLS=0
expect_check_start "argv-matching impostor executable is not FRPC" 1
[ -s "$FRP_PROC_ROOT/402/cmdline" ] || fail "impostor process fixture was unexpectedly removed"

clear_processes
frpc_enable=0
frps_enable=0
LAUNCH_CALLS=0
expect_check_start "all services disabled" 0
frpc_enable=0
frps_enable=1
add_process 302 frps /tmp/frp/myfrps.ini
LAUNCH_CALLS=0
expect_check_start "disabled client is not required" 0
frpc_enable=1
frps_enable=1
clear_processes
NETWORK_RESULT=2
LAUNCH_CALLS=0
expect_check_start "WAN unavailable" 0
NETWORK_RESULT=1

# Startup accepts a delayed background daemon, but only after bounded polling.
clear_processes
TEST_LAUNCH_MODE=delayed
frpc_enable=1
frps_enable=1
LAUNCH_CALLS=0
WATCHDOG_ADDS=0
SLEEP_CALLS=0
frp_start || fail "delayed daemons did not satisfy readiness"
[ "$SLEEP_CALLS" -eq 2 ] || fail "readiness did not tolerate delayed PID publication"
[ "$WATCHDOG_ADDS" -eq 1 ] || fail "watchdog was not added after delayed readiness"

# Partial startup times out, stops the surviving daemon and never publishes watchdog.
clear_processes
TEST_LAUNCH_MODE=partial
frpc_enable=1
frps_enable=1
LAUNCH_CALLS=0
WATCHDOG_ADDS=0
SLEEP_CALLS=0
if frp_start; then fail "partial startup was incorrectly reported as successful"; fi
[ "$SLEEP_CALLS" -eq 4 ] || fail "readiness timeout was not bounded to four retry sleeps"
frp_service_running frpc && fail "partial FRPC daemon was not cleaned up"
[ "$WATCHDOG_ADDS" -eq 0 ] || fail "watchdog was added after failed readiness"

clear_processes
TEST_LAUNCH_MODE=immediate
frpc_enable=1
frps_enable=0
WATCHDOG_ADDS=0
frp_start || fail "client-only readiness failed"
frp_service_running frpc || fail "client-only daemon is absent"
frp_service_running frps && fail "disabled server was unexpectedly started"
clear_processes
frpc_enable=0
frps_enable=1
WATCHDOG_ADDS=0
frp_start || fail "server-only readiness failed"
frp_service_running frps || fail "server-only daemon is absent"
frp_service_running frpc && fail "disabled client was unexpectedly started"

echo "FRP watchdog detects exact-config daemon state and dispatches one replacement: PASS"
echo "FRP asynchronous daemon startup uses bounded readiness and fail-closed cleanup: PASS"
