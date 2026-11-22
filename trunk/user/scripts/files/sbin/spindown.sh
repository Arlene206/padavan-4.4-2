#!/bin/sh

[ ! -x /sbin/hdparm ] && exit 1

# check param
[ -z "$1" ] && exit 1
[ "$#" -eq 1 ] || exit 2
case "$1" in
	[!A-Za-z0-9]*|*[!A-Za-z0-9_-]*) exit 1 ;;
esac

# Share automount.sh's mountpoint lock across the inventory check and
# hdparm command. Keep FD 9 open until hdparm returns so a hotplug mount cannot
# start between the snapshot and the power command.
spindown_lock_file=/var/lock/automount_mountpoint.lock
spindown_old_umask=$(umask)
umask 077
if ! ( : >> "$spindown_lock_file" ); then
	umask "$spindown_old_umask"
	exit 1
fi
umask "$spindown_old_umask"
exec 9>>"$spindown_lock_file" || exit 1
if ! flock 9; then
	exec 9>&-
	exit 1
fi

# Return 0 when the disk or one of its partitions is mounted, 1 only
# after a complete mount-table read proves it is absent, and 2 on any error.
spindown_mount_status() {
	spindown_device=$1
	spindown_mounts_file=$2

	if [ -z "$spindown_device" ] || [ -z "$spindown_mounts_file" ] ||
	   [ ! -r "$spindown_mounts_file" ]; then
		return 2
	fi
	spindown_mount_snapshot=$(cat "$spindown_mounts_file") || return 2
	printf '%s\n' "$spindown_mount_snapshot" | awk -v dev="/dev/$spindown_device" '
NF == 0 { next }
NF < 6 {
	invalid = 1
	next
}
{
	records++
	source = $1
	if (source == dev) {
		found = 1
	} else if (index(source, dev) == 1) {
		suffix = substr(source, length(dev) + 1)
		if (suffix ~ /^[0-9]+$/ || suffix ~ /^p[0-9]+$/)
			found = 1
	}
}
END {
	if (invalid || records == 0)
		exit 2
	exit !found
}
	spindown_awk_status=$?
	case "$spindown_awk_status" in
		0|1) return "$spindown_awk_status" ;;
		*) return 2 ;;
	esac
}

# Refuse to spin down a mounted disk or any of its partitions.
spindown_mount_status "$1" /proc/mounts
mount_status=$?
case "$mount_status" in
	0) exit 1 ;; # mounted; do not spin down
	1) ;;        # full read succeeded and no matching mount exists
	*) exit 1 ;; # cannot prove the disk is unused
esac

if [ -r "/sys/block/$1/removable" ] ; then
	dev_removable=`cat "/sys/block/$1/removable"`
	if [ "$dev_removable" = "0" ] ; then
		/sbin/hdparm -y "/dev/$1"
	fi
fi

exit 0
