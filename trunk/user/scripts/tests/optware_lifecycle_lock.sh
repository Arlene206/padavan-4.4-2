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
(umask 077; : > "$lock_file")
[ "$(stat -c %a "$lock_file")" = 600 ] || {
	echo "Optware lock fixture was not created with mode 0600" >&2
	exit 1
}
. "$helper"

# Lock paths are pre-provisioned. Reject redirection, aliases, weak modes,
# missing files and writable non-sticky parent directories before opening.
target="$tmpdir/lock-target"
printf '%s\n' sentinel > "$target"
symlink="$tmpdir/lock-symlink"
ln -s "$target" "$symlink"
if _optware_lifecycle_lock_open "$symlink"; then
	echo "Optware lifecycle lock followed a symbolic link" >&2
	exit 1
fi
grep -Fqx sentinel "$target"

hardlink="$tmpdir/lock-hardlink"
ln "$lock_file" "$hardlink"
if _optware_lifecycle_lock_open "$hardlink"; then
	echo "Optware lifecycle lock accepted a multiply-linked inode" >&2
	exit 1
fi
rm -f "$hardlink"

chmod 644 "$lock_file"
if _optware_lifecycle_lock_open "$lock_file"; then
	echo "Optware lifecycle lock accepted a non-private file mode" >&2
	exit 1
fi
chmod 600 "$lock_file"

missing="$tmpdir/missing.lock"
if _optware_lifecycle_lock_open "$missing"; then
	echo "Optware lifecycle lock implicitly created a missing file" >&2
	exit 1
fi
[ ! -e "$missing" ]

if _optware_lifecycle_lock_open relative.lock; then
	echo "Optware lifecycle lock accepted a relative pathname" >&2
	exit 1
fi
[ ! -e "$tmpdir/relative.lock" ]

untrusted_dir="$tmpdir/untrusted"
mkdir "$untrusted_dir"
chmod 777 "$untrusted_dir"
(umask 077; : > "$untrusted_dir/lock")
if _optware_lifecycle_lock_open "$untrusted_dir/lock"; then
	echo "Optware lifecycle lock accepted a writable non-sticky directory" >&2
	exit 1
fi

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
