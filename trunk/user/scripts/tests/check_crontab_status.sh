#!/bin/sh
set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
ROOT=$(CDPATH= cd -- "$SCRIPT_DIR/../../../.." && pwd)
SOURCE="$ROOT/trunk/user/scripts/files/sbin/check_crontab.sh"
WORK=$(mktemp -d "${TMPDIR:-/tmp}/check-crontab-status.XXXXXX")
trap 'rm -rf "$WORK"' EXIT HUP INT TERM

# Exercise the production script with a disposable storage-root fixture.
sed 's@^cd /etc/storage || exit 2$@cd "$TEST_STORAGE_ROOT" || exit 2@' "$SOURCE" > "$WORK/check_crontab.sh"
chmod 755 "$WORK/check_crontab.sh"
TEST_STORAGE_ROOT="$WORK/storage"
export TEST_STORAGE_ROOT
mkdir -p "$TEST_STORAGE_ROOT"

run_check()
{
	expected=$1
	shift
	if sh "$WORK/check_crontab.sh" "$@" >/dev/null 2>&1; then
		actual=0
	else
		actual=$?
	fi
	if [ "$actual" -ne "$expected" ]; then
		echo "expected check_crontab status $expected, got $actual" >&2
		exit 1
	fi
}

run_check 1 a/1 a a a a scutclient_watchcat.sh
ADMIN="$TEST_STORAGE_ROOT/cron/crontabs/admin"
[ -f "$ADMIN" ]
[ "$(wc -l < "$ADMIN")" -eq 1 ]
run_check 0 a/1 a a a a scutclient_watchcat.sh
[ "$(wc -l < "$ADMIN")" -eq 1 ]
/bin/grep -Fx '*/1 * * * * /usr/bin/scutclient_watchcat.sh > /dev/null 2>&1' "$ADMIN" >/dev/null

run_check 2 a/1 a a a
run_check 2 a/1 a a a a ../escape.sh
TEST_STORAGE_ROOT="$WORK/missing-storage"
export TEST_STORAGE_ROOT
run_check 2 a/1 a a a a scutclient_watchcat.sh
TEST_STORAGE_ROOT="$WORK/storage"
export TEST_STORAGE_ROOT

printf '%s\n' '# existing content' > "$ADMIN"
TEST_GREP_STATUS=2
export TEST_GREP_STATUS
run_check 2 a/1 a a a a scutclient_watchcat.sh
unset TEST_GREP_STATUS
[ "$(wc -l < "$ADMIN")" -eq 1 ]

rm -f "$ADMIN"
mkdir "$ADMIN"
run_check 2 a/1 a a a a scutclient_watchcat.sh
rmdir "$ADMIN"

rm -rf "$TEST_STORAGE_ROOT/cron"
mkdir "$WORK/outside"
ln -s "$WORK/outside" "$TEST_STORAGE_ROOT/cron"
run_check 2 a/1 a a a a scutclient_watchcat.sh
[ ! -e "$WORK/outside/crontabs" ]
rm "$TEST_STORAGE_ROOT/cron"

mkdir -p "$TEST_STORAGE_ROOT/cron"
ln -s "$WORK/outside" "$TEST_STORAGE_ROOT/cron/crontabs"
run_check 2 a/1 a a a a scutclient_watchcat.sh
rm "$TEST_STORAGE_ROOT/cron/crontabs"

mkdir -p "$TEST_STORAGE_ROOT/cron/crontabs"
printf '%s\n' '# outside' > "$WORK/outside/admin"
ln -s "$WORK/outside/admin" "$ADMIN"
run_check 2 a/1 a a a a scutclient_watchcat.sh
[ "$(wc -l < "$WORK/outside/admin")" -eq 1 ]

echo "check_crontab.sh 0/1/2+ result and symlink rejection regression: PASS"
