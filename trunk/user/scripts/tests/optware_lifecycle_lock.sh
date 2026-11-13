#!/bin/sh
set -eu

# Do not probe with `command -v`: firmware ash may have CONFIG_ASH_CMDCMD
# disabled. The first lock acquisition below exercises the required flock
# applet directly and fails if the command is unavailable.

script_dir=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
helper="$script_dir/../files/sbin/optware_lifecycle_lock.sh"
tmpdir=$(mktemp -d "${TMPDIR:-/tmp}/padavan-optware-lock.XXXXXX")
child_pid=
cleanup()
{
	if [ -n "$child_pid" ]; then
		kill "$child_pid" 2>/dev/null || true
	fi
	rm -rf "$tmpdir"
}
trap cleanup 0 HUP INT TERM

lock_file="$tmpdir/optware-lifecycle.lock"
. "$helper"
optware_lifecycle_lock_acquire "$lock_file"

(
	exec 9>&-
	. "$helper"
	if optware_lifecycle_lock_try_acquire "$lock_file"; then
		: > "$tmpdir/nonblocking-acquired"
		optware_lifecycle_lock_release
		exit 1
	fi
	: > "$tmpdir/nonblocking-refused"
	optware_lifecycle_lock_acquire "$lock_file"
	: > "$tmpdir/blocking-acquired"
	optware_lifecycle_lock_release
) &
child_pid=$!

attempt=0
while [ ! -f "$tmpdir/nonblocking-refused" ]; do
	if [ -f "$tmpdir/nonblocking-acquired" ]; then
		echo "nonblocking Optware teardown lock acquired while startup owned it" >&2
		exit 1
	fi
	attempt=$((attempt + 1))
	if [ "$attempt" -ge 5 ]; then
		echo "nonblocking Optware lock attempt did not return promptly" >&2
		exit 1
	fi
	sleep 1
done

sleep 1
if [ -f "$tmpdir/blocking-acquired" ]; then
	echo "blocking Optware lifecycle lock did not wait for the owner" >&2
	exit 1
fi

optware_lifecycle_lock_release
wait "$child_pid"
child_pid=
[ -f "$tmpdir/blocking-acquired" ]
echo "Optware teardown fails fast under a held lifecycle lock; startup waits for release: PASS"
