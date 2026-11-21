#!/bin/sh
set -eu

automount="$(dirname "$0")/../files/sbin/automount.sh"
tmpdir=$(mktemp -d /tmp/padavan-mount-owner.XXXXXX)
trap 'rm -rf "$tmpdir"' 0 HUP INT TERM

sed -n '/^mount_from_device()/,/^}/p' "$automount" > "$tmpdir/mount-function.sh"
[ -s "$tmpdir/mount-function.sh" ]
. "$tmpdir/mount-function.sh"

dev_full=/dev/sda1
mounts="$tmpdir/mounts"

expect_status()
{
	expected=$1
	shift
	if mount_from_device "$@"; then
		actual=0
	else
		actual=$?
	fi
	if [ "$actual" -ne "$expected" ]; then
		echo "mount_from_device returned $actual, expected $expected" >&2
		exit 1
	fi
}

# Only an exact device/source match is safe to unmount and replace.
printf '/dev/sda1 /media/usb ext4 rw 0 0\n' > "$mounts"
expect_status 0 /media/usb "$mounts"

# A confirmed different source is the sole condition that permits legacy fallback.
printf '/dev/sdb1 /media/usb ext4 rw 0 0\n' > "$mounts"
expect_status 1 /media/usb "$mounts"

# Multiple records with conflicting sources must never be treated as owned.
printf '/dev/sda1 /media/usb ext4 rw 0 0\n/dev/sdb1 /media/usb ext4 rw 0 0\n' > "$mounts"
expect_status 1 /media/usb "$mounts"

# A mountpoint absent from the inventory is UNKNOWN, not a foreign-owner result.
: > "$mounts"
expect_status 2 /media/usb "$mounts"

# An unreadable inventory is UNKNOWN, not an absent mountpoint.
mkdir "$tmpdir/not-a-mount-table"
expect_status 3 /media/usb "$tmpdir/not-a-mount-table"

# A truncated/malformed procfs record must also fail closed.
printf '/dev/sda1 /media/usb ext4 rw\n' > "$mounts"
expect_status 3 /media/usb "$mounts"

echo "Automount mount ownership tri-state inventory policy: PASS"
