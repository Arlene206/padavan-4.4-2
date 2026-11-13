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
sed -n '/^frp_launch()$/,/^}/p; /^frp_autostart()$/,/^}/p' "$autostart" > "$functions"
[ -s "$functions" ] || { echo "FRP autostart helper extraction failed" >&2; exit 1; }
. "$functions"

TEST_FRPC_ENABLE=0
TEST_FRPS_ENABLE=0
START_CALLS=0
LOG_CALLS=0
nvram()
{
	case "$2" in
		frpc_enable) printf '%s\n' "$TEST_FRPC_ENABLE" ;;
		frps_enable) printf '%s\n' "$TEST_FRPS_ENABLE" ;;
		*) return 1 ;;
	esac
}
logger() { LOG_CALLS=$((LOG_CALLS + 1)); }
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

echo "FRP boot starts client-only, server-only and combined configurations exactly once: PASS"
