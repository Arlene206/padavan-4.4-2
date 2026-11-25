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
# Comments and similarly named commands must not suppress the exact job.
printf '%s\n' '  # docs: for user script /usr/bin/duplicate_guard.sh' \
  '* * * * * /usr/bin/duplicate_guard.sh.extra > /dev/null 2>&1' > "$ADMIN"
run_check 1 a/2 a a a a duplicate_guard.sh
[ "$(wc -l < "$ADMIN")" -eq 3 ]
/bin/grep -Fx '*/2 * * * * /usr/bin/duplicate_guard.sh > /dev/null 2>&1' "$ADMIN" >/dev/null
run_check 0 a/2 a a a a duplicate_guard.sh
[ "$(wc -l < "$ADMIN")" -eq 3 ]

# Concurrent startups must append at most one identical job.
rm -f "$ADMIN"
for worker in 1 2 3 4 5 6 7 8; do
  (
    if sh "$WORK/check_crontab.sh" a/3 a a a a concurrent_watchcat.sh >/dev/null 2>&1; then
      status=0
    else
      status=$?
    fi
    printf '%s\n' "$status" > "$WORK/concurrent.$worker"
  ) &
done
wait
[ "$(/bin/grep -Fc '/usr/bin/concurrent_watchcat.sh > /dev/null 2>&1' "$ADMIN")" -eq 1 ]
[ "$(stat -c %a "$TEST_STORAGE_ROOT/cron/.admin.lock")" = 600 ]
for status_file in "$WORK"/concurrent.*; do
  status=$(cat "$status_file")
  [ "$status" = 0 ] || [ "$status" = 1 ] || exit 1
done


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

# A lock symlink must be rejected without modifying its target.
rm -rf "$TEST_STORAGE_ROOT/cron"
mkdir -p "$TEST_STORAGE_ROOT/cron/crontabs"
printf '%s\n' 'preserve lock target' > "$WORK/lock-target"
ln -s "$WORK/lock-target" "$TEST_STORAGE_ROOT/cron/.admin.lock"
run_check 2 a/4 a a a a symlink_lock_target.sh
[ "$(cat "$WORK/lock-target")" = "preserve lock target" ]
rm "$TEST_STORAGE_ROOT/cron/.admin.lock"

mkdir -p "$TEST_STORAGE_ROOT/cron/crontabs"
printf '%s\n' '# outside' > "$WORK/outside/admin"
ln -s "$WORK/outside/admin" "$ADMIN"
run_check 2 a/1 a a a a scutclient_watchcat.sh
[ "$(wc -l < "$WORK/outside/admin")" -eq 1 ]

echo "check_crontab exact identity, concurrency and symlink regression: PASS"
