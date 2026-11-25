#!/bin/sh

sbin_dir=/sbin
proc_mtd=/proc/mtd
dev_dir=/dev

fail()
{
	printf '%s\n' "$*" >&2
	exit 1
}

for tool in ubiattach ubiformat ubimkvol ubidetach; do
	if [ ! -x "$sbin_dir/$tool" ]; then
		fail "Unable to find executable: $sbin_dir/$tool"
	fi
done
for tool in mount mtd_write nvram; do
	if ! command -v "$tool" >/dev/null 2>&1; then
		fail "Unable to find executable: $tool"
	fi
done
if [ ! -r "$proc_mtd" ]; then
	fail "Unable to read MTD partition inventory: $proc_mtd"
fi

# Serialize destructive conversion so concurrent invocations cannot race.
operation_lock_dir=/tmp/.rwfs2ubi
operation_lock_file="$operation_lock_dir/.rwfs2ubi.lock"
process_uid=$(id -u 2>/dev/null) || fail "Unable to determine RWFS conversion owner"

# Keep the lock under a private, owner-checked directory. A predictable lock
# path directly in /tmp could be pre-created as a symlink and truncated by
# opening it with > before flock has a chance to protect anything.
if [ -L "$operation_lock_dir" ] ||
   { [ -e "$operation_lock_dir" ] && [ ! -d "$operation_lock_dir" ]; }; then
	fail "Unsafe RWFS conversion lock directory: $operation_lock_dir"
fi
if [ ! -d "$operation_lock_dir" ] &&
   ! mkdir -m 700 "$operation_lock_dir"; then
	fail "Unable to create RWFS conversion lock directory: $operation_lock_dir"
fi
lock_dir_owner=$(stat -c %u "$operation_lock_dir" 2>/dev/null) ||
	fail "Unable to inspect RWFS conversion lock directory: $operation_lock_dir"
if [ "$lock_dir_owner" != "$process_uid" ]; then
	fail "RWFS conversion lock directory has an unexpected owner: $operation_lock_dir"
fi
if ! chmod 700 "$operation_lock_dir"; then
	fail "Unable to secure RWFS conversion lock directory: $operation_lock_dir"
fi
if [ -L "$operation_lock_file" ] ||
   { [ -e "$operation_lock_file" ] && [ ! -f "$operation_lock_file" ]; }; then
	fail "Unsafe RWFS conversion lock file: $operation_lock_file"
fi
if [ -e "$operation_lock_file" ]; then
	lock_file_owner=$(stat -c %u "$operation_lock_file" 2>/dev/null) ||
		fail "Unable to inspect RWFS conversion lock file: $operation_lock_file"
	lock_file_links=$(stat -c %h "$operation_lock_file" 2>/dev/null) ||
		fail "Unable to inspect RWFS conversion lock links: $operation_lock_file"
	if [ "$lock_file_owner" != "$process_uid" ] || [ "$lock_file_links" != 1 ]; then
		fail "RWFS conversion lock file has unsafe ownership or links: $operation_lock_file"
	fi
	if ! chmod 600 "$operation_lock_file"; then
		fail "Unable to secure RWFS conversion lock file: $operation_lock_file"
	fi
fi
umask 077
if ! command -v flock >/dev/null 2>&1; then
	fail "Unable to find executable: flock"
fi
if ! exec 9>>"$operation_lock_file"; then
	fail "Unable to open RWFS conversion lock: $operation_lock_file"
fi
if ! flock -n 9; then
	fail "Another RWFS conversion is already running"
fi


# Match one exact RWFS partition using shell built-ins only. The firmware
# BusyBox configuration does not guarantee the awk applet is available.
rwfs_matches=0
rwfs_invalid=0
rwfs_idx=
while IFS= read -r mtd_line || [ -n "$mtd_line" ]; do
	set -f
	set -- $mtd_line
	set +f
	if [ "$#" -lt 4 ] || [ "$4" != '"RWFS"' ]; then
		continue
	fi
	rwfs_matches=$((rwfs_matches + 1))
	if [ "$#" -ne 4 ]; then
		rwfs_invalid=1
		continue
	fi
	case "$1" in
		mtd[0-9]*:) ;;
		*) rwfs_invalid=1; continue ;;
	esac
	candidate_idx=${1#mtd}
	candidate_idx=${candidate_idx%:}
	case "$candidate_idx" in
		''|0*|*[!0-9]*) rwfs_invalid=1; continue ;;
	esac
	if [ "$candidate_idx" -lt 4 ]; then
		rwfs_invalid=1
		continue
	fi
	case "$2" in
		''|*[!0123456789abcdefABCDEF]*) rwfs_invalid=1; continue ;;
	esac
	case "$3" in
		''|*[!0123456789abcdefABCDEF]*) rwfs_invalid=1; continue ;;
	esac
	rwfs_idx=$candidate_idx
done < "$proc_mtd"
if [ "$rwfs_invalid" -ne 0 ] || [ "$rwfs_matches" -ne 1 ] || [ -z "$rwfs_idx" ]; then
	fail "RWFS partition is missing, malformed, or ambiguous in $proc_mtd"
fi

mtd_device="$dev_dir/mtd$rwfs_idx"
ubi_device="$dev_dir/ubi$rwfs_idx"
if [ ! -e "$mtd_device" ]; then
	fail "RWFS MTD device does not exist: $mtd_device"
fi

# Reject only exact devices/volumes belonging to this MTD index. Prefix grep
# would confuse ubi5 with ubi50, while also missing ubiblock-backed mounts.
if ! mount_inventory=$(mount 2>/dev/null); then
	fail "Unable to inspect mounted filesystems before RWFS conversion"
fi
mount_records=0
mount_invalid=0
mount_found=0
while IFS=' ' read -r mount_source mount_on mount_path mount_type mount_fs mount_opts; do
	[ -n "$mount_source" ] || continue
	mount_records=$((mount_records + 1))
	if [ "$mount_on" != "on" ] || [ -z "$mount_path" ] ||
	   [ "$mount_type" != "type" ] || [ -z "$mount_fs" ] || [ -z "$mount_opts" ]; then
		mount_invalid=1
		continue
	fi
	case "$mount_source" in
		"/dev/ubi$rwfs_idx"|"/dev/ubi${rwfs_idx}_"*|"/dev/ubiblock${rwfs_idx}_"*|"/dev/mtdblock$rwfs_idx"|"ubi${rwfs_idx}:"*)
			mount_found=1
			;;
	esac
done <<EOF_MOUNT
$mount_inventory
EOF_MOUNT
if [ "$mount_invalid" -ne 0 ] || [ "$mount_records" -eq 0 ]; then
	mount_check_status=2
elif [ "$mount_found" -eq 1 ]; then
	mount_check_status=0
else
	mount_check_status=1
fi
case "$mount_check_status" in
	0) fail "RWFS UBI/MTD device is already mounted; refusing to erase $mtd_device" ;;
	1) ;;
	*) fail "Unable to parse mounted filesystem inventory" ;;
esac

# The partition may not currently be attached; that expected case is harmless.
# ubiformat below remains the authoritative guard against an attached/busy MTD.
if ! "$sbin_dir/ubidetach" -p "$mtd_device" >/dev/null 2>&1; then
	printf '%s\n' "No existing UBI attachment could be detached; proceeding to checked format" >&2
fi

if ! mtd_write erase "$mtd_device"; then
	fail "Erase RWFS partition failed: $mtd_device"
fi

if ! "$sbin_dir/ubiformat" "$mtd_device" -y -q; then
	fail "Format RWFS partition ($mtd_device) to UBIFS FAILED!"
fi

if ! "$sbin_dir/ubiattach" -p "$mtd_device" -d "$rwfs_idx"; then
	printf '%s\n' "Attach UBIFS ($mtd_device) FAILED!" >&2
	# An attach command can fail after partially publishing an attachment.
	if ! "$sbin_dir/ubidetach" -p "$mtd_device" >/dev/null 2>&1; then
		printf '%s\n' "Cleanup detach also failed after UBI attach error" >&2
	fi
	exit 1
fi

operation_status=0
if ! "$sbin_dir/ubimkvol" "$ubi_device" -m -N rwfs; then
	printf '%s\n' "Create UBI volume FAILED!" >&2
	operation_status=1
else
	printf '%s\n\n' "Create UBI volume done!"
	if ! nvram set mtd_rwfs_mount=1; then
		printf '%s\n' "Unable to set mtd_rwfs_mount=1 in NVRAM" >&2
		operation_status=1
	elif ! nvram commit; then
		printf '%s\n' "Unable to commit RWFS auto-mount setting to NVRAM" >&2
		operation_status=1
	fi
fi

# Always attempt cleanup after a successful attach, but never let a successful
# detach hide an earlier volume-creation or persistent-configuration failure.
if ! "$sbin_dir/ubidetach" -p "$mtd_device"; then
	printf '%s\n' "Detach UBIFS ($mtd_device) after conversion FAILED!" >&2
	operation_status=1
fi

if [ "$operation_status" -ne 0 ]; then
	exit 1
fi

printf '%s\n' "Please reboot your router to automount RWFS partition at '/media/mtd_rwfs'."
printf '%s\n' "To automount RWFS to /opt, create the 'opt' directory in '/media/mtd_rwfs' before rebooting."
printf '%s\n' "To install Entware in /opt, set 'optw_enable=2' in NVRAM and run 'opt-start.sh'."
exit 0
