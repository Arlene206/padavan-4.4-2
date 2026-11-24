#!/bin/sh
# Host regression for FRPC/FRPS-only boot activation.
set -eu
repo_root=$(CDPATH= cd -- "$(dirname "$0")/../../../.." && pwd)
test_root=$(mktemp -d /tmp/frp-autostart-test.XXXXXX)
trap 'rm -rf "$test_root"' EXIT HUP INT TERM
autostart="$repo_root/trunk/user/scripts/files/usr/bin/autostart.sh"
functions="$test_root/frp-autostart-functions.sh"

# Extract only the production FRP helper functions; the full boot script
# includes network waits and unrelated services that are outside this fixture.
sed -n '/^autostart_nvram_read()$/,/^}/p; /^frp_launch()$/,/^}/p; /^frp_autostart()$/,/^}/p' "$autostart" > "$functions"
[ -s "$functions" ] || { echo "FRP autostart helper extraction failed" >&2; exit 1; }
. "$functions"

TEST_FRPC_ENABLE=0
TEST_FRPS_ENABLE=0
START_CALLS=0
LOG_CALLS=0
TEST_NVRAM_FAIL_KEY=
TEST_NVRAM_FAIL_STATUS=9
LOG_MESSAGES=
nvram()
{
	if [ "$TEST_NVRAM_FAIL_KEY" = "$2" ]; then
		return "$TEST_NVRAM_FAIL_STATUS"
	fi
	case "$2" in
		frpc_enable) printf '%s\n' "$TEST_FRPC_ENABLE" ;;
		frps_enable) printf '%s\n' "$TEST_FRPS_ENABLE" ;;
		*) return 1 ;;
	esac
}
logger()
{
	LOG_CALLS=$((LOG_CALLS + 1))
	LOG_MESSAGES="$LOG_MESSAGES$*\n"
}
frp_launch()
{
	START_CALLS=$((START_CALLS + 1))
	return 0
}
fail() { echo "FRP autostart regression: FAIL: $*" >&2; exit 1; }
expect_calls()
{
	label=$1
	expected=$2
	frp_autostart || fail "$label: autostart returned failure"
	[ "$START_CALLS" -eq "$expected" ] ||
		fail "$label: expected $expected launch(es), got $START_CALLS"
}

expect_calls "both services disabled" 0
[ "$LOG_CALLS" -eq 0 ] || fail "disabled FRP unexpectedly logged a launch"

TEST_FRPC_ENABLE=1
expect_calls "client-only configuration" 1
[ "$LOG_CALLS" -eq 1 ] || fail "client-only launch log count mismatch"

TEST_FRPC_ENABLE=0
TEST_FRPS_ENABLE=1
expect_calls "server-only configuration" 2
[ "$LOG_CALLS" -eq 2 ] || fail "server-only launch log count mismatch"

TEST_FRPC_ENABLE=1
expect_calls "both services enabled" 3
[ "$LOG_CALLS" -eq 3 ] || fail "combined launch log count mismatch"

# Unknown feature-switch state must fail closed and preserve the nvram status.
TEST_FRPC_ENABLE=0
TEST_FRPS_ENABLE=0
TEST_NVRAM_FAIL_KEY=frpc_enable
before_calls=$START_CALLS
if frp_autostart; then fail "FRPC read failure was treated as a disabled switch"; else read_status=$?; fi
[ "$read_status" -eq 9 ] || fail "FRPC read failure status was not preserved"
[ "$START_CALLS" -eq "$before_calls" ] || fail "FRP launched with unknown FRPC state"
case "$LOG_MESSAGES" in *"NVRAM read failed for frpc_enable (status=9)"*) ;; *) fail "FRPC read error key/status not logged" ;; esac

TEST_NVRAM_FAIL_KEY=frps_enable
TEST_FRPC_ENABLE=0
TEST_FRPS_ENABLE=1
before_calls=$START_CALLS
if frp_autostart; then fail "FRPS read failure was accepted"; else read_status=$?; fi
[ "$read_status" -eq 9 ] || fail "FRPS read failure status was not preserved"
[ "$START_CALLS" -eq "$before_calls" ] || fail "FRP launched with unknown FRPS state"
case "$LOG_MESSAGES" in *"NVRAM read failed for frps_enable (status=9)"*) ;; *) fail "FRPS read error key/status not logged" ;; esac

echo "FRP boot starts client-only, server-only and combined configurations exactly once: PASS"
