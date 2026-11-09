#!/bin/sh
set -eu

if ! command -v flock >/dev/null 2>&1; then
	echo "flock is required for this regression" >&2
	exit 1
fi

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
	: > "$tmpdir/child-started"
	optware_lifecycle_lock_acquire "$lock_file"
	: > "$tmpdir/child-acquired"
	optware_lifecycle_lock_release
) &
child_pid=$!

attempt=0
while [ ! -f "$tmpdir/child-started" ]; do
	attempt=$((attempt + 1))
	if [ "$attempt" -ge 50 ]; then
		echo "child did not reach the lock contention point" >&2
		exit 1
	fi
	sleep 0.1
done

sleep 1
if [ -f "$tmpdir/child-acquired" ]; then
	echo "Optware lifecycle lock did not block a competing process" >&2
	exit 1
fi

optware_lifecycle_lock_release
wait "$child_pid"
child_pid=
[ -f "$tmpdir/child-acquired" ]
echo "Optware startup and teardown lifecycle lock serializes competing processes: PASS"
