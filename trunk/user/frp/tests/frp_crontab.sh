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

FRP_CRON_ROOT="$test_root/cron"
FRP_CRONTAB_DIR="$FRP_CRON_ROOT/crontabs"
mkdir -p "$FRP_CRONTAB_DIR"
http_username=root
FRP_RUN_CALLS=0
FRP_RUN_STATUS=0
frp_run_script()
{
	FRP_RUN_CALLS=$((FRP_RUN_CALLS + 1))
	return "$FRP_RUN_STATUS"
}
pidof() { return 1; }
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
		[ -e "$item" ] && fail "staging file leaked: $item"
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

echo "FRP managed watchdog crontab preserves operator jobs and publishes atomically: PASS"
