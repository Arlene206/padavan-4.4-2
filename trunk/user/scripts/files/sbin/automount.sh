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

	# Distinguish an absent module from an unreadable /proc/modules inventory.
	if awk -v module="$module_name" '$1 == module { found=1 } END { exit !found }' "$modules_file" 2>/dev/null; then
		return 0
	else
		module_status=$?
		if [ "$module_status" -ne 1 ]; then
			logger -t "automount" "Unable to read loaded-module inventory $modules_file (status=$module_status)"
			return "$module_status"
		fi
	fi

	(
		# Never run modprobe without the cross-process serialization lock.
		if ! flock 9; then
			logger -t "automount" "Unable to acquire filesystem-module lock $lock_file"
			exit 1
		fi

		# Another hotplug worker may have loaded it while this process waited.
		if awk -v module="$module_name" '$1 == module { found=1 } END { exit !found }' "$modules_file" 2>/dev/null; then
			exit 0
		else
			module_status=$?
			if [ "$module_status" -ne 1 ]; then
				logger -t "automount" "Unable to recheck loaded-module inventory $modules_file (status=$module_status)"
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


if [ "$ID_FS_TYPE" == "swap" ] ; then
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

# Distinct hotplug workers can race when different disks share a filesystem label.
# Serialize mountpoint ownership checks and mounting, then release before app hooks.
automount_lock_file="/var/lock/automount_mountpoint.lock"
automount_old_umask=`umask`
umask 077
if ! ( : >> "$automount_lock_file" ); then
	umask "$automount_old_umask"
	logger -t "automount" "Unable to create mountpoint lock $automount_lock_file"
	exit 1
fi
exec 8>>"$automount_lock_file"
if ! flock 8; then
	exec 8>&-
	umask "$automount_old_umask"
	logger -t "automount" "Unable to acquire mountpoint lock $automount_lock_file"
	exit 1
fi
umask "$automount_old_umask"

# Never unmount a different disk that happens to use the same filesystem label.
mount_from_device()
{
	awk -v mount="$1" -v device="$dev_full" '
		$2 == mount {
			if ($1 == device)
				found = 1
			else
				conflict = 1
		}
		END { exit !(found && !conflict) }
	' /proc/mounts
}

# If the label mountpoint belongs to another disk, use this device's unique
# legacy mountpoint instead. Only unmount mounts created from this device.
if mountpoint -q "$dev_mount" ; then
	if ! mount_from_device "$dev_mount" ; then
		dev_mount="$mnt_legacy"
	fi
	if mountpoint -q "$dev_mount" ; then
		if ! mount_from_device "$dev_mount" ; then
			logger -t "automount" "Mountpoint $dev_mount is already used by another device!"
			exit 1
		fi
		if ! umount -f "$dev_mount" ; then
			logger -t "automount" "Unable to prepare mountpoint $dev_mount!"
			exit 1
		fi
	fi
fi

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
	kernel_vfat=`modprobe -l | grep vfat`
	if [ -n "$kernel_vfat" ] ; then
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
	kernel_ufsd=`modprobe -l | grep ufsd`
	kernel_antfs=`modprobe -l | grep antfs`
	if [ -n "$kernel_antfs" ]; then
		load_module_or_abort antfs
		mount -t antfs "$dev_full" "$dev_mount" -o noatime,utf8 8>&-
	elif [ -n "$kernel_ufsd" ] ; then
		load_module_or_abort ufsd
		mount -t ufsd "$dev_full" "$dev_mount" -o noatime,sparse,nls=utf8,force 8>&-
	elif [ -x /sbin/ntfs-3g ] ; then
		/sbin/ntfs-3g "$dev_full" "$dev_mount" -o noatime,umask=0,big_writes 8>&-
		if [ $? -ne 0 ] ; then
			/sbin/ntfs-3g "$dev_full" "$dev_mount" -o noatime,umask=0,ro 8>&-
		fi
	fi
elif [ "$ID_FS_TYPE" == "hfsplus" -o "$ID_FS_TYPE" == "hfs" ] ; then
	kernel_hfsplus=`modprobe -l | grep hfsplus`
	if [ -n "$kernel_hfsplus" ] ; then
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
	if [ "$ID_FS_TYPE" == "ext4" ] ; then
		load_module_or_abort ext4
	elif [ "$ID_FS_TYPE" == "ext3" ] ; then
		kernel_ext3=`modprobe -l | grep ext3`
		if [ -n "$kernel_ext3" ] ; then
			load_module_or_abort ext3
		else
			load_module_or_abort ext4
		fi
	elif [ "$ID_FS_TYPE" == "ext2" ] ; then
		kernel_ext2=`modprobe -l | grep ext2`
		if [ -n "$kernel_ext2" ] ; then
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

# check failed to mount, clean up mountpoint
if ! mountpoint -q "$dev_mount" ; then
	if [ -n "$ID_FS_TYPE" ] ; then
		logger -t "automount" "Unable to mount device $dev_full ($ID_FS_TYPE) to $dev_mount!"
	fi
	rmdir "$dev_mount"
	exit 1
fi

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
