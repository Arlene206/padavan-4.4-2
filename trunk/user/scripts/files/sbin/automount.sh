#!/bin/sh
logger -t "automount" "/sbin/automount.sh $1    $2"
func_load_module()
{
	module_name=$1
	modules_file=/proc/modules
	lock_file=/var/lock/fs_module.lock
	if [ "$#" -ge 2 ]; then
		modules_file=$2
	fi
	if [ "$#" -ge 3 ]; then
		lock_file=$3
	fi
	case "$module_name" in
		''|*[!A-Za-z0-9_-]*) return 1 ;;
	esac

	# Snapshot the inventory so read errors cannot be mistaken for "module absent".
	if module_inventory=$(cat "$modules_file" 2>/dev/null); then
		:
	else
		module_status=$?
		logger -t "automount" "Unable to read loaded-module inventory $modules_file (status=$module_status)"
		return "$module_status"
	fi
	if printf '%s\n' "$module_inventory" | awk -v module="$module_name" '$1 == module { found=1 } END { exit !found }'; then
		return 0
	else
		module_status=$?
		if [ "$module_status" -ne 1 ]; then
			logger -t "automount" "Unable to inspect loaded-module inventory $modules_file (status=$module_status)"
			return "$module_status"
		fi
	fi

	(
		if ! flock 9; then
			logger -t "automount" "Unable to acquire filesystem-module lock $lock_file"
			exit 1
		fi

		# Another hotplug worker may have loaded it while this process waited.
		if module_inventory=$(cat "$modules_file" 2>/dev/null); then
			:
		else
			module_status=$?
			logger -t "automount" "Unable to re-read loaded-module inventory $modules_file (status=$module_status)"
			exit "$module_status"
		fi
		if printf '%s\n' "$module_inventory" | awk -v module="$module_name" '$1 == module { found=1 } END { exit !found }'; then
			exit 0
		else
			module_status=$?
			if [ "$module_status" -ne 1 ]; then
				logger -t "automount" "Unable to inspect loaded-module inventory $modules_file (status=$module_status)"
				exit "$module_status"
			fi
		fi

		modprobe -q "$module_name"
		module_status=$?
		if [ "$module_status" -ne 0 ]; then
			logger -t "automount" "Unable to load filesystem module $module_name (status=$module_status)"
			exit "$module_status"
		fi
		usleep 300000
	) 9>>"$lock_file"
}

func_read_module_catalog()
{
	# Do not hide modprobe -l failures behind a grep pipeline's status.
	if module_catalog=$(modprobe -l 2>/dev/null); then
		return 0
	else
		module_catalog_status=$?
		logger -t "automount" "Unable to read available filesystem-module catalog (status=$module_catalog_status)"
		return "$module_catalog_status"
	fi
}

func_module_catalog_contains()
{
	case "$1" in
		''|*[!A-Za-z0-9_-]*) return 1 ;;
	esac

	# Match the module basename exactly; substring matching would confuse
	# "fat" with exfat or "vfat" with a similarly named module.
	module_catalog_remaining=$module_catalog
	while [ -n "$module_catalog_remaining" ]; do
		case "$module_catalog_remaining" in
			*'
'*)
				module_catalog_entry=${module_catalog_remaining%%'
'*}
				module_catalog_remaining=${module_catalog_remaining#*'
'}
				;;
			*)
				module_catalog_entry=$module_catalog_remaining
				module_catalog_remaining=
				;;
		esac
		module_catalog_file=${module_catalog_entry##*/}
		case "$module_catalog_file" in
			"$1".ko|"$1".ko.gz|"$1".ko.xz) return 0 ;;
		esac
	done
	return 1
}

# Device names come from hotplug. Keep them as a single block-device component.
load_module_or_abort()
{
	if ! func_load_module "$1"; then
		logger -t "automount" "Refusing to mount $dev_full: filesystem module $1 could not be loaded safely"
		rmdir "$dev_mount"
		exit 1
	fi
}

case "$1" in
	''|[!A-Za-z0-9]*|*[!A-Za-z0-9_-]*) exit 1 ;;
esac
case "$2" in
	''|.|..|*[!A-Za-z0-9_.-]*) exit 1 ;;
esac

existed=`grep -wF -- "$1" /proc/partitions | wc -l`
[ "$existed" = "0" ] && exit 1

[ ! -x /sbin/blkid ] && exit 1

dev_full="/dev/$1"

# Read blkid values separately. Never eval metadata from an inserted disk.
ID_FS_TYPE=$(/sbin/blkid -s TYPE -o value "$dev_full" 2>/dev/null)
ID_FS_UUID=$(/sbin/blkid -s UUID -o value "$dev_full" 2>/dev/null)
ID_FS_LABEL=$(/sbin/blkid -s LABEL -o value "$dev_full" 2>/dev/null)


# Use one exclusive lock for filesystem mounts and swap activation. Disk
# spindown holds the same lock while checking both inventories and issuing hdparm.
automount_acquire_mountpoint_lock()
{
	automount_lock_file="/var/lock/automount_mountpoint.lock"
	automount_old_umask=$(umask)
	umask 077
	if ! ( : >> "$automount_lock_file" ); then
		umask "$automount_old_umask"
		logger -t "automount" "Unable to create mountpoint lock $automount_lock_file"
		return 1
	fi
	if ! exec 8>>"$automount_lock_file"; then
		umask "$automount_old_umask"
		logger -t "automount" "Unable to open mountpoint lock $automount_lock_file"
		return 1
	fi
	if ! flock 8; then
		exec 8>&-
		umask "$automount_old_umask"
		logger -t "automount" "Unable to acquire mountpoint lock $automount_lock_file"
		return 1
	fi
	umask "$automount_old_umask"
	return 0
}

if [ "$ID_FS_TYPE" == "swap" ] ; then
	[ ! -x /sbin/swapon ] && exit 1
	if ! automount_acquire_mountpoint_lock; then
		exit 1
	fi
	if ! awk -v dev="$dev_full" '$1 == dev { found=1 } END { exit !found }' /proc/swaps; then
		if swapon "$dev_full" ; then
			logger -t "automount" "Activate swap partition $dev_full SUCCESS!"
		fi
	fi
	exec 8>&-

	# always return !=0 for swap
	exit 1
fi
	[ ! -x /sbin/swapon ] && exit 1
	if ! awk -v dev="$dev_full" '$1 == dev { found=1 } END { exit !found }' /proc/swaps; then
		if swapon "$dev_full" ; then
			logger -t "automount" "Activate swap partition $dev_full SUCCESS!"
		fi
	fi

	# always return !=0 for swap
	exit 1
fi

dev_label=`printf '%s' "$ID_FS_LABEL" | LC_ALL=C tr -cd 'A-Za-z0-9._-'`
case "$dev_label" in
	''|.|..|NO_NAME) dev_label="" ;;
esac

mnt_legacy="/media/$2"
if [ -z "$dev_label" ] ; then
	dev_mount="$mnt_legacy"
else
	dev_mount="/media/$dev_label"
fi

# Serialize mountpoint ownership checks and mounting, then release before app hooks.
if ! automount_acquire_mountpoint_lock; then
	exit 1
fi

# Never unmount a different disk that happens to use the same filesystem label.
mount_from_device()
{
	mounts_file=/proc/mounts
	if [ "$#" -ge 2 ]; then
		mounts_file=$2
	fi

	# Read errors and malformed records are UNKNOWN, never "not mounted".
	if mount_inventory=$(cat "$mounts_file" 2>/dev/null); then
		:
	else
		return 3
	fi

	if mount_state=$(printf '%s\n' "$mount_inventory" | awk -v mount="$1" -v device="$dev_full" '
		NF == 0 { next }
		NF < 6 { invalid = 1; next }
		$2 == mount {
			if ($1 == device)
				found = 1
			else
				conflict = 1
		}
		END {
			if (invalid)
				print "unknown"
			else if (conflict)
				print "foreign"
			else if (found)
				print "owned"
			else
				print "absent"
		}
	'); then
		case "$mount_state" in
			owned) return 0 ;;
			foreign) return 1 ;; # Confirmed mountpoint belongs to another device.
			absent) return 2 ;; # No mount entry for this mountpoint.
			*) return 3 ;; # Malformed/inconsistent inventory or parser output.
		esac
	else
		return 3
	fi
}

# Confirm ownership directly from the checked mount table; do not interpret
# mountpoint(1)'s generic nonzero status as proof that a path is unmounted.
mount_from_device "$dev_mount"
mount_status=$?
case "$mount_status" in
	0) ;; # This device owns the selected path; revalidate before unmount.
	1) dev_mount="$mnt_legacy" ;; # A confirmed foreign owner uses its private path.
	2) ;; # The selected path is not mounted.
	*)
		logger -t "automount" "Unable to verify mount ownership for $dev_mount; refusing to mount $dev_full"
		exit 1
		;;
esac

# Recheck the final target. Only an owned mount can be replaced; absence is safe.
mount_from_device "$dev_mount"
mount_status=$?
case "$mount_status" in
	0)
		if ! umount -f "$dev_mount" ; then
			logger -t "automount" "Unable to prepare mountpoint $dev_mount!"
			exit 1
		fi
		;;
	2) ;; # Not mounted; proceed.
	1)
		logger -t "automount" "Mountpoint $dev_mount is already used by another device!"
		exit 1
		;;
	*)
		logger -t "automount" "Unable to verify mount ownership for $dev_mount; refusing to unmount"
		exit 1
		;;
esac

logger -t "automount" "mount  device $dev_full ($ID_FS_TYPE) to $dev_mount @@@@@ $mnt_legacy"

if ! mkdir -p "$dev_mount" ; then
	logger -t "automount" "Unable to create mountpoint $dev_mount!"
	exit 1
fi

achk_enable=`nvram get achk_enable`

if [ "$ID_FS_TYPE" == "msdos" -o "$ID_FS_TYPE" == "vfat" ] ; then
	if [ "$achk_enable" != "0" ] && [ -x /sbin/fsck.fat ] ; then
		if [ ! -r /sbin/storage_fsck_status.sh ]; then
			logger -t "automount" "Missing FAT checker status policy; refusing to mount $dev_full"
			rmdir "$dev_mount"
			exit 1
		fi
		. /sbin/storage_fsck_status.sh
		/sbin/fsck.fat -a -v "$dev_full" > "/tmp/fsck.fat_result_$1" 2>&1
		fsck_fat_status=$?
		if storage_fsck_fat_status_needs_verification "$fsck_fat_status"; then
			/sbin/fsck.fat -a -v "$dev_full" >> "/tmp/fsck.fat_result_$1" 2>&1
			fsck_fat_status=$?
		fi
		if ! storage_fsck_fat_status_allows_mount "$fsck_fat_status"; then
			logger -t "automount" "Refusing to mount $dev_full after fsck.fat status $fsck_fat_status"
			rmdir "$dev_mount"
			exit 1
		fi
	fi
	if ! func_read_module_catalog; then
		logger -t "automount" "Refusing to mount $dev_full: filesystem-module catalog is unavailable"
		rmdir "$dev_mount"
		exit 1
	fi
	if func_module_catalog_contains vfat ; then
		load_module_or_abort vfat
		mount -t vfat "$dev_full" "$dev_mount" -o noatime,umask=0,iocharset=utf8,codepage=936,shortname=winnt 8>&-
	else
		load_module_or_abort exfat
		mount -t exfat "$dev_full" "$dev_mount" -o noatime,umask=0,iocharset=utf8 8>&-
	fi
elif [ "$ID_FS_TYPE" == "exfat" ] ; then
	load_module_or_abort exfat
	mount -t exfat "$dev_full" "$dev_mount" -o noatime,umask=0,iocharset=utf8 8>&-
elif [ "$ID_FS_TYPE" == "ntfs" ] ; then
	if [ "$achk_enable" != "0" ] && [ -x /sbin/chkntfs ] ; then
		if [ ! -r /sbin/storage_fsck_status.sh ]; then
			logger -t "automount" "Missing NTFS checker status policy; refusing to mount $dev_full"
			rmdir "$dev_mount"
			exit 1
		fi
		. /sbin/storage_fsck_status.sh
		/sbin/chkntfs -a -f --verbose "$dev_full" > "/tmp/chkntfs_result_$1" 2>&1
		chkntfs_status=$?
		if ! storage_chkntfs_status_allows_mount "$chkntfs_status"; then
			logger -t "automount" "Refusing to mount $dev_full after chkntfs status $chkntfs_status"
			rmdir "$dev_mount"
			exit 1
		fi
	fi
	if ! func_read_module_catalog; then
		logger -t "automount" "Refusing to mount $dev_full: filesystem-module catalog is unavailable"
		rmdir "$dev_mount"
		exit 1
	fi
	if func_module_catalog_contains antfs; then
		load_module_or_abort antfs
		mount -t antfs "$dev_full" "$dev_mount" -o noatime,utf8 8>&-
	elif func_module_catalog_contains ufsd ; then
		load_module_or_abort ufsd
		mount -t ufsd "$dev_full" "$dev_mount" -o noatime,sparse,nls=utf8,force 8>&-
	elif [ -x /sbin/ntfs-3g ] ; then
		/sbin/ntfs-3g "$dev_full" "$dev_mount" -o noatime,umask=0,big_writes 8>&-
		if [ $? -ne 0 ] ; then
			/sbin/ntfs-3g "$dev_full" "$dev_mount" -o noatime,umask=0,ro 8>&-
		fi
	fi
elif [ "$ID_FS_TYPE" == "hfsplus" -o "$ID_FS_TYPE" == "hfs" ] ; then
	if ! func_read_module_catalog; then
		logger -t "automount" "Refusing to mount $dev_full: filesystem-module catalog is unavailable"
		rmdir "$dev_mount"
		exit 1
	fi
	if func_module_catalog_contains hfsplus ; then
		load_module_or_abort hfsplus
		mount -t $ID_FS_TYPE -o noatime,umask=0,nls=utf8 "$dev_full" "$dev_mount" 8>&-
	else
		load_module_or_abort ufsd
		mount -t ufsd -o noatime,nls=utf8,force "$dev_full" "$dev_mount" 8>&-
	fi
elif [ "$ID_FS_TYPE" == "ext4" -o "$ID_FS_TYPE" == "ext3" -o "$ID_FS_TYPE" == "ext2" ] ; then
	if [ "$achk_enable" != "0" ] && [ -x /sbin/e2fsck ] ; then
		if [ ! -r /sbin/storage_fsck_status.sh ]; then
			logger -t "automount" "Missing e2fsck status policy; refusing to mount $dev_full"
			exit 1
		fi
		. /sbin/storage_fsck_status.sh
		/sbin/e2fsck -p -v "$dev_full" > "/tmp/e2fsck_result_$1" 2>&1
		e2fsck_status=$?
		if ! storage_e2fsck_status_allows_mount "$e2fsck_status"; then
			logger -t "automount" "Refusing to mount $dev_full after e2fsck exit status $e2fsck_status"
			exit 1
		fi
	fi
	if [ "$ID_FS_TYPE" != "ext4" ]; then
		if ! func_read_module_catalog; then
			logger -t "automount" "Refusing to mount $dev_full: filesystem-module catalog is unavailable"
			rmdir "$dev_mount"
			exit 1
		fi
	fi
	if [ "$ID_FS_TYPE" == "ext4" ] ; then
		load_module_or_abort ext4
	elif [ "$ID_FS_TYPE" == "ext3" ] ; then
		if func_module_catalog_contains ext3 ; then
			load_module_or_abort ext3
		else
			load_module_or_abort ext4
		fi
	elif [ "$ID_FS_TYPE" == "ext2" ] ; then
		if func_module_catalog_contains ext2 ; then
			load_module_or_abort ext2
		else
			load_module_or_abort ext4
		fi
	fi
	mount -t $ID_FS_TYPE -o noatime "$dev_full" "$dev_mount" 8>&-
elif [ "$ID_FS_TYPE" == "xfs" ] ; then
	load_module_or_abort xfs
	mount -t $ID_FS_TYPE -o noatime "$dev_full" "$dev_mount" 8>&-
fi

# Verify the mount table names this exact device as the owner of the target.
mount_from_device "$dev_mount"
mount_status=$?
case "$mount_status" in
	0) ;; # The intended device is mounted at the selected path.
	1)
		logger -t "automount" "Mount result for $dev_mount belongs to another device; refusing to continue"
		exit 1
		;;
	*)
		if [ -n "$ID_FS_TYPE" ] ; then
			logger -t "automount" "Unable to verify mount of $dev_full ($ID_FS_TYPE) at $dev_mount (status=$mount_status)"
		fi
		rmdir "$dev_mount"
		exit 1
		;;
esac

# The ownership decision and mount are complete; do not let hooks inherit the lock.
exec 8>&-

chmod 777 "$dev_mount"

# check ACL files
if [ -x /sbin/test_share ] ; then
	/sbin/test_share "$dev_mount"
fi

# call optware script
/usr/bin/opt-mount.sh "$dev_full" "$dev_mount"

exit 0
