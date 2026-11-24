#!/bin/sh
# Host regression for FRP's managed watchdog crontab transaction.
set -eu
repo_root=$(CDPATH= cd -- "$(dirname "$0")/../../../.." && pwd)
test_root=$(mktemp -d /tmp/frp-cron-test.XXXXXX)
trap 'rm -rf "$test_root"' EXIT HUP INT TERM

TEST_FRPC_ENABLE=1
TEST_FRPS_ENABLE=0
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

# Keep this cron-transaction test independent of the CI runner's procfs visibility.
# Daemon identity and stop behavior have dedicated fixtures in frp_lifecycle.sh.
FRP_PROC_ROOT="$test_root/proc"
FRP_CRON_ROOT="$test_root/cron"
FRP_CRONTAB_DIR="$FRP_CRON_ROOT/crontabs"
mkdir -p "$FRP_CRONTAB_DIR" "$FRP_PROC_ROOT"
http_username=root
# Verify the production runner never follows a substituted FRP script path.
FRP_SCRIPT_PATH="$test_root/frp-script.sh"
FRP_SCRIPT_MARKER="$test_root/script-ran"
printf 'touch "%s"\n' "$FRP_SCRIPT_MARKER" > "$FRP_SCRIPT_PATH"
chmod 755 "$FRP_SCRIPT_PATH"
frp_run_script || { echo "FRP script path regression: FAIL: regular script did not run" >&2; exit 1; }
[ "$(stat -c '%a' "$FRP_SCRIPT_PATH")" = "700" ] || {
	echo "FRP script path regression: FAIL: runner did not repair mode 0700" >&2
	exit 1
}
[ -f "$FRP_SCRIPT_MARKER" ] || {
	echo "FRP script path regression: FAIL: regular script marker missing" >&2
	exit 1
}
rm -f "$FRP_SCRIPT_MARKER" "$FRP_SCRIPT_PATH"
ln -s "$test_root/does-not-exist" "$FRP_SCRIPT_PATH"
if frp_run_script; then
	echo "FRP script path regression: FAIL: symlink was executed" >&2
	exit 1
fi
[ ! -e "$FRP_SCRIPT_MARKER" ] || {
	echo "FRP script path regression: FAIL: symlink target was executed" >&2
	exit 1
}
rm -f "$FRP_SCRIPT_PATH"
mkdir "$FRP_SCRIPT_PATH"
if frp_run_script; then
	echo "FRP script path regression: FAIL: directory was accepted as a script" >&2
	exit 1
fi
rmdir "$FRP_SCRIPT_PATH"
echo "FRP script runner rejects symlink and non-regular targets: PASS"

FRP_RUN_CALLS=0
FRP_RUN_STATUS=0
frp_run_script()
{
	FRP_RUN_CALLS=$((FRP_RUN_CALLS + 1))
	return "$FRP_RUN_STATUS"
}
# This fixture targets cron transactions; exact daemon readiness is covered by frp_lifecycle.sh.
frp_wait_started() { return 0; }
fail() { echo "FRP crontab regression: FAIL: $*" >&2; exit 1; }
watchdog_line='*/1 * * * * /bin/sh /usr/bin/frp.sh C >/dev/null 2>&1'
expect_has_line()
{
	grep -Fqx "$2" "$1" || fail "missing preserved line: $2"
}
expect_not_has_line()
{
	if grep -Fqx "$2" "$1"; then fail "unexpected line remains: $2"; fi
}
expect_no_stage()
{
	for item in "$FRP_CRON_ROOT"/.frp-crontab.*; do
		[ -e "$item" ] && fail "staging file leaked: $item" || :
	done
}

crontab="$FRP_CRONTAB_DIR/root"
cat > "$crontab" <<'EOF'
SHELL=/bin/sh
# operator note: frp maintenance is independent
*/5 * * * * /usr/local/sbin/frp-maintenance
@reboot /usr/local/sbin/frp-backup
*/1 * * * * /bin/sh /usr/bin/frp.sh C >/dev/null 2>&1
*/1 * * * * /bin/sh /usr/bin/frp.sh C >/dev/null 2>&1
EOF
frp_start || fail "start rejected a valid crontab"
[ "$FRP_RUN_CALLS" -eq 1 ] || fail "start script was not called exactly once"
[ "$(grep -Fxc "$watchdog_line" "$crontab")" -eq 1 ] ||
	fail "exact watchdog entry was not de-duplicated"
expect_has_line "$crontab" 'SHELL=/bin/sh'
expect_has_line "$crontab" '# operator note: frp maintenance is independent'
expect_has_line "$crontab" '*/5 * * * * /usr/local/sbin/frp-maintenance'
expect_has_line "$crontab" '@reboot /usr/local/sbin/frp-backup'
[ "$(stat -c '%a' "$crontab")" = 600 ] || fail "published crontab is not mode 0600"
expect_no_stage

# Stop when both FRP services are disabled removes only the exact managed line.
frpc_enable=0
frps_enable=0
frp_close || fail "close could not remove the managed watchdog"
expect_not_has_line "$crontab" "$watchdog_line"
expect_has_line "$crontab" '*/5 * * * * /usr/local/sbin/frp-maintenance'
expect_has_line "$crontab" '@reboot /usr/local/sbin/frp-backup'
expect_has_line "$crontab" '# operator note: frp maintenance is independent'
expect_no_stage

# Unsafe directory ownership/modes and existing files must not be imported.
before=$(cat "$crontab")
chmod 0777 "$FRP_CRONTAB_DIR"
if frp_manage_watchdog_cron add; then fail "world-writable crontab directory was accepted"; fi
[ "$(cat "$crontab")" = "$before" ] || fail "writable crontab directory changed the published file"
chmod 0755 "$FRP_CRONTAB_DIR"
chmod 0777 "$FRP_CRON_ROOT"
if frp_manage_watchdog_cron add; then fail "world-writable cron root was accepted"; fi
[ "$(cat "$crontab")" = "$before" ] || fail "writable cron root changed the published file"
chmod 0755 "$FRP_CRON_ROOT"
chmod 0777 "$test_root"
if frp_manage_watchdog_cron add; then fail "world-writable cron parent was accepted"; fi
[ "$(cat "$crontab")" = "$before" ] || fail "writable cron parent changed the published file"
chmod 0700 "$test_root"
chmod 0666 "$crontab"
if frp_manage_watchdog_cron add; then fail "world-writable existing crontab was accepted"; fi
[ "$(cat "$crontab")" = "$before" ] || fail "unsafe existing crontab was modified"
chmod 0600 "$crontab"
ln "$crontab" "$test_root/crontab-hardlink"
if frp_manage_watchdog_cron add; then fail "hard-linked existing crontab was accepted"; fi
[ "$(cat "$crontab")" = "$before" ] || fail "hard-linked crontab was modified"
rm -f "$test_root/crontab-hardlink"

# Simulate owner mismatches without requiring root or chown in CI.
TEST_STAT_PATH="$FRP_CRONTAB_DIR"
TEST_STAT_FORMAT="%u %a"
TEST_STAT_FAKE_UID=$(( $(id -u) + 1 ))
stat()
{
	if [ "${1:-}" = "-c" ] && [ "${2:-}" = "$TEST_STAT_FORMAT" ] &&
	   [ "${3:-}" = "$TEST_STAT_PATH" ]; then
		printf '%s 755\n' "$TEST_STAT_FAKE_UID"
		return 0
	fi
	command stat "$@"
}
if frp_manage_watchdog_cron add; then fail "wrong-owner crontab directory was accepted"; fi
[ "$(cat "$crontab")" = "$before" ] || fail "wrong-owner crontab directory changed the published file"
unset -f stat
TEST_STAT_PATH="$crontab"
TEST_STAT_FORMAT="%u %a %h"
stat()
{
	if [ "${1:-}" = "-c" ] && [ "${2:-}" = "$TEST_STAT_FORMAT" ] &&
	   [ "${3:-}" = "$TEST_STAT_PATH" ]; then
		printf '%s 600 1\n' "$TEST_STAT_FAKE_UID"
		return 0
	fi
	command stat "$@"
}
if frp_manage_watchdog_cron add; then fail "wrong-owner existing crontab was accepted"; fi
[ "$(cat "$crontab")" = "$before" ] || fail "wrong-owner existing crontab was modified"
unset -f stat
echo "FRP crontab ownership, directory and hard-link checks fail closed: PASS"

# Failed script startup never changes the user's scheduled jobs.
frpc_enable=1
FRP_RUN_STATUS=7
before=$(cat "$crontab")
if frp_start; then fail "failed start script was reported as successful"; fi
[ "$(cat "$crontab")" = "$before" ] || fail "failed startup changed the crontab"
FRP_RUN_STATUS=0

# An unsafe NVRAM username cannot escape the crontab directory.
http_username='../../frp-cron-escape'
before=$(cat "$crontab")
if frp_manage_watchdog_cron add; then fail "path-traversal username was accepted"; fi
[ "$(cat "$crontab")" = "$before" ] || fail "unsafe username changed the real crontab"
[ ! -e "$FRP_CRON_ROOT/frp-cron-escape" ] || fail "username escaped into the cron parent"
http_username=root

# Overlong login names are not accepted as filesystem components.
http_username='123456789012345678901234567890123'
if frp_manage_watchdog_cron add; then fail "overlong username was accepted"; fi
http_username=root

# Symlink crontab entries are rejected without truncating their referent.
target="$test_root/symlink-target"
printf 'keep-this-data\n' > "$target"
rm -f "$crontab"
ln -s "$target" "$crontab"
if frp_manage_watchdog_cron add; then fail "symlink crontab was accepted"; fi
[ "$(cat "$target")" = 'keep-this-data' ] || fail "symlink referent was modified"
rm -f "$crontab"
printf '*/5 * * * * /usr/local/sbin/frp-maintenance\n' > "$crontab"

# Inject a rename failure. The published file must remain byte-for-byte intact.
before=$(cat "$crontab")
mv() { return 1; }
if frp_manage_watchdog_cron add; then fail "injected atomic rename failure was ignored"; fi
unset -f mv 2>/dev/null || :
[ "$(cat "$crontab")" = "$before" ] || fail "rename failure changed the published crontab"
expect_no_stage

# Removing a missing watchdog file is a successful no-op and creates no file.
rm -f "$crontab"
frp_manage_watchdog_cron remove || fail "removing an absent crontab failed"
[ ! -e "$crontab" ] || fail "remove created an otherwise absent crontab"
expect_no_stage

# RC/UI restart still reaches frp_start when both roles are disabled. It must
# not execute the custom script or leave a per-minute watchdog behind.
frpc_enable=0
frps_enable=0
printf '%s\n' 'SHELL=/bin/sh' '*/5 * * * * /usr/local/sbin/operator-job' "$watchdog_line" > "$crontab"
FRP_RUN_CALLS=0
frp_start || fail "disabled start failed to clean the watchdog"
[ "$FRP_RUN_CALLS" -eq 0 ] || fail "disabled start executed the custom FRP script"
expect_not_has_line "$crontab" "$watchdog_line"
expect_has_line "$crontab" '*/5 * * * * /usr/local/sbin/operator-job'
expect_no_stage

# Server-only mode remains a valid start path after the disabled guard.
frps_enable=1
FRP_RUN_CALLS=0
frp_start || fail "server-only start was rejected"
[ "$FRP_RUN_CALLS" -eq 1 ] || fail "server-only start did not invoke the custom script once"
[ "$(grep -Fxc "$watchdog_line" "$crontab")" -eq 1 ] ||
	fail "server-only start did not publish exactly one watchdog"

# Disabling the final role removes only the managed watchdog again.
frps_enable=0
FRP_RUN_CALLS=0
frp_start || fail "disabling the final role failed to clean the watchdog"
[ "$FRP_RUN_CALLS" -eq 0 ] || fail "disabled restart executed the custom FRP script"
expect_not_has_line "$crontab" "$watchdog_line"
expect_has_line "$crontab" '*/5 * * * * /usr/local/sbin/operator-job'
expect_no_stage

echo "FRP managed watchdog crontab preserves operator jobs and publishes atomically: PASS"
