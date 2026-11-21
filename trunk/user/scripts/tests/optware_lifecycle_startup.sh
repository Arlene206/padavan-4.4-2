#!/bin/sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
helper="$script_dir/../files/sbin/optware_lifecycle_lock.sh"
tmpdir=$(mktemp -d "${TMPDIR:-/tmp}/padavan-optware-startup.XXXXXX")
cleanup()
{
	optware_lifecycle_lock_release 2>/dev/null || :
	rm -rf "$tmpdir"
}
trap cleanup 0 HUP INT TERM
# Generate child fixtures with the same shell interpreter as this regression.
mock_shell=${OPTWARE_TEST_SHELL:-/bin/sh}
case "$mock_shell" in
	/*) ;;
	*) echo "OPTWARE_TEST_SHELL must be an absolute path" >&2; exit 1 ;;
esac
[ -x "$mock_shell" ] || {
	echo "OPTWARE_TEST_SHELL is not executable: $mock_shell" >&2
	exit 1
}
write_mock()
{
	mock_path=$1
	{
		printf '#!%s\n' "$mock_shell"
		cat
	} > "$mock_path"
	chmod +x "$mock_path"
}

mkdir -p "$tmpdir/bin" "$tmpdir/services-ok" "$tmpdir/services-fail" "$tmpdir/services-empty"
write_mock "$tmpdir/bin/logger" <<'MOCK'
exit 0
MOCK
chmod +x "$tmpdir/bin/logger"
PATH="$tmpdir/bin:$PATH"
export PATH

. "$helper"
lock_file="$tmpdir/optware-lifecycle.lock"
optware_lifecycle_lock_acquire "$lock_file"

EVENTS_FILE="$tmpdir/events"
export EVENTS_FILE

write_mock "$tmpdir/services-ok/S01-first" <<'HOOK'
case "$1" in
start)
	printf 'S01-first start\n' >> "$EVENTS_FILE"
	if flock -n 9 2>/dev/null; then
		echo "startup hook inherited lifecycle lock fd 9" >&2
		exit 90
	fi
	;;
stop) printf 'S01-first stop\n' >> "$EVENTS_FILE" ;;
*) exit 2 ;;
esac
HOOK
write_mock "$tmpdir/services-ok/S02 with space" <<'HOOK'
case "$1" in
start)
	printf 'S02 with space start\n' >> "$EVENTS_FILE"
	if flock -n 9 2>/dev/null; then
		echo "startup hook with spaces inherited lifecycle lock fd 9" >&2
		exit 90
	fi
	;;
stop) printf 'S02 with space stop\n' >> "$EVENTS_FILE" ;;
*) exit 2 ;;
esac
HOOK
write_mock "$tmpdir/services-ok/S03-not-executable" <<'HOOK'
printf 'non-executable hook ran\n' >> "$EVENTS_FILE"
HOOK
chmod +x "$tmpdir/services-ok/S01-first" "$tmpdir/services-ok/S02 with space"
chmod 644 "$tmpdir/services-ok/S03-not-executable"

if ! optware_lifecycle_start_services "$tmpdir/services-ok"; then
	echo "valid startup hooks unexpectedly failed" >&2
	exit 1
fi
expected=$(printf 'S01-first start\nS02 with space start')
[ "$(cat "$EVENTS_FILE")" = "$expected" ] || {
	echo "startup order, pathname quoting, or non-executable filtering failed" >&2
	exit 1
}

: > "$EVENTS_FILE"
write_mock "$tmpdir/services-fail/S01-first" <<'HOOK'
case "$1" in
start) printf 'S01-first start\n' >> "$EVENTS_FILE" ;;
stop) printf 'S01-first stop\n' >> "$EVENTS_FILE" ;;
*) exit 2 ;;
esac
HOOK
write_mock "$tmpdir/services-fail/S02 failing hook" <<'HOOK'
case "$1" in
start) printf 'S02 failing hook start\n' >> "$EVENTS_FILE"; exit 1 ;;
stop) printf 'S02 failing hook stop\n' >> "$EVENTS_FILE"; exit 1 ;;
*) exit 2 ;;
esac
HOOK
write_mock "$tmpdir/services-fail/S03-not-started" <<'HOOK'
printf 'S03 should not start\n' >> "$EVENTS_FILE"
exit 0
HOOK
chmod +x "$tmpdir/services-fail"/S??*
if optware_lifecycle_start_services "$tmpdir/services-fail"; then
	echo "a failed service start was reported as success" >&2
	exit 1
fi
expected=$(printf 'S01-first start\nS02 failing hook start\nS02 failing hook stop\nS01-first stop')
[ "$(cat "$EVENTS_FILE")" = "$expected" ] || {
	echo "partial-start rollback did not stop failed/started hooks in reverse order" >&2
	exit 1
}

if optware_lifecycle_start_services "$tmpdir/missing"; then
	echo "missing service directory was treated as successful startup" >&2
	exit 1
fi
if ! optware_lifecycle_start_services "$tmpdir/services-empty"; then
	echo "empty service directory was treated as failed startup" >&2
	exit 1
fi

optware_lifecycle_lock_release
echo "Optware startup preserves hook pathnames, closes inherited lock fd, and rolls back partial startup: PASS"
