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

# Swap activation must distinguish an active device, a fully inventoried
# inactive device, and an unreadable or malformed /proc/swaps snapshot.
sed -n '/^automount_swap_status()$/,/^}/p' "$automount" > "$tmpdir/swap-function.sh"
[ -s "$tmpdir/swap-function.sh" ]
. "$tmpdir/swap-function.sh"

expect_swap_status()
{
	expected=$1
	device=$2
	swap_file=$3
	if automount_swap_status "$device" "$swap_file"; then
		actual=0
	else
		actual=$?
	fi
	if [ "$actual" -ne "$expected" ]; then
		echo "automount_swap_status returned $actual, expected $expected for $device" >&2
		exit 1
	fi
}

swaps="$tmpdir/swaps"
cat > "$swaps" <<'EOF_SWAPS'
Filename Type Size Used Priority
/dev/sdb1 partition 1024 0 -2
EOF_SWAPS
expect_swap_status 1 /dev/sda1 "$swaps"
expect_swap_status 0 /dev/sdb1 "$swaps"

# Exact path matching must not treat /dev/sda10 as /dev/sda1.
cat > "$swaps" <<'EOF_SWAPS'
Filename Type Size Used Priority
/dev/sda10 partition 1024 0 -2
EOF_SWAPS
expect_swap_status 1 /dev/sda1 "$swaps"

# Every row is validated, including unrelated active swap devices.
cat > "$swaps" <<'EOF_SWAPS'
Filename Type Size Used Priority
/dev/sdb1 partition 1024 0
EOF_SWAPS
expect_swap_status 2 /dev/sda1 "$swaps"

cat > "$swaps" <<'EOF_SWAPS'
Filename Type Size Used
/dev/sdb1 partition 1024 0 -2
EOF_SWAPS
expect_swap_status 2 /dev/sda1 "$swaps"

cat > "$swaps" <<'EOF_SWAPS'
Filename Type Size Used Priority
/dev/sdb1 partition invalid 0 -2
EOF_SWAPS
expect_swap_status 2 /dev/sda1 "$swaps"

# Command substitution must not manufacture a missing procfs record terminator.
printf 'Filename Type Size Used Priority' > "$swaps"
expect_swap_status 2 /dev/sda1 "$swaps"
printf 'Filename Type Size Used Priority\n/dev/sdb1 partition 1024 0 -2' > "$swaps"
expect_swap_status 2 /dev/sda1 "$swaps"

: > "$swaps"
expect_swap_status 2 /dev/sda1 "$swaps"
expect_swap_status 2 /dev/sda1 "$tmpdir/missing-swaps"
mkdir "$tmpdir/swaps-is-directory"
expect_swap_status 2 /dev/sda1 "$tmpdir/swaps-is-directory"

# Only a fully parsed inventory proving absence may reach swapon.
python3 - "$automount" <<'PY'
from pathlib import Path
import sys

source = Path(sys.argv[1]).read_text()
start = source.index('if [ "$ID_FS_TYPE" == "swap" ] ; then')
end = source.index('\n\t# always return !=0 for swap', start)
block = source[start:end]
assert block.index("automount_acquire_mountpoint_lock") < block.index(
    'automount_swap_status "$dev_full" /proc/swaps'
)
case_start = block.index('case "$swap_status" in')
case_end = block.index("\n\tesac", case_start)
dispatch = block[case_start:case_end]
active = dispatch.split("0)", 1)[1].split("1)", 1)[0]
inactive = dispatch.split("1)", 1)[1].split("*)", 1)[0]
unknown = dispatch.split("*)", 1)[1]
assert 'swapon "$dev_full"' not in active
assert 'swapon "$dev_full"' in inactive
assert 'swapon "$dev_full"' not in unknown
assert "refusing to activate" in unknown
assert "Unable to activate swap partition" in inactive
print("Automount swap activation fail-closed caller contract: PASS")
PY

echo "Automount mount ownership tri-state inventory policy: PASS"
# The lock helper is shared with swap activation. Verify private creation,
# umask restoration and exclusion of a competing worker.
sed -e "s|/tmp/automount_mountpoint.lock|$tmpdir/automount.lock|g" \
	-n -e '/^automount_acquire_mountpoint_lock()$/,/^}$/p' "$automount" > "$tmpdir/lock-function.sh"
[ -s "$tmpdir/lock-function.sh" ]
(umask 077; : > "$tmpdir/automount.lock")
logger() { :; }
. "$tmpdir/lock-function.sh"
saved_umask=$(umask)
automount_acquire_mountpoint_lock
[ "$(umask)" = "$saved_umask" ]
[ "$(stat -c %a "$tmpdir/automount.lock")" = 600 ]
if (
	exec 9>>"$tmpdir/automount.lock"
	flock -n 9
); then
	echo "automount mountpoint lock allowed a competing holder" >&2
	exec 8>&-
	exit 1
fi
exec 8>&-

# The runtime helper must reject symlinks even if the pre-boot provisioner is
# bypassed or the /tmp entry is unexpectedly replaced.
sed -e "s|/tmp/automount_mountpoint.lock|$tmpdir/unsafe-lock|g" \
	-n -e '/^automount_acquire_mountpoint_lock()$/,/^}$/p' "$automount" > "$tmpdir/unsafe-lock-function.sh"
ln -s "$tmpdir/target" "$tmpdir/unsafe-lock"
if (
	logger() { :; }
	. "$tmpdir/unsafe-lock-function.sh"
	automount_acquire_mountpoint_lock
); then
	echo "automount lock helper followed a symlink" >&2
	exit 1
fi

# Swap activation must take the same lock before swapon runs.
lock_line=$(grep -nF 'if ! automount_acquire_mountpoint_lock; then' "$automount" | head -n 1 | cut -d: -f1)
swapon_line=$(grep -nF 'if swapon "$dev_full"; then' "$automount" | cut -d: -f1)
[ -n "$lock_line" ] && [ -n "$swapon_line" ] && [ "$lock_line" -lt "$swapon_line" ]

