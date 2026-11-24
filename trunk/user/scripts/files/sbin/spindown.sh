#!/bin/sh

[ ! -x /sbin/hdparm ] && exit 1

# check param
[ -z "$1" ] && exit 1
[ "$#" -eq 1 ] || exit 2
case "$1" in
	[!A-Za-z0-9]*|*[!A-Za-z0-9_-]*) exit 1 ;;
esac

# Share automount.sh's pre-created root-owned mountpoint lock across the inventory check and
# hdparm command. Keep FD 9 open until hdparm returns so a hotplug mount cannot
# start between the snapshot and the power command.
spindown_lock_file=/tmp/automount_mountpoint.lock
if [ -L "$spindown_lock_file" ] || [ ! -f "$spindown_lock_file" ]; then
	exit 1
fi
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
}'
	spindown_awk_status=$?
	case "$spindown_awk_status" in
		0|1) return "$spindown_awk_status" ;;
		*) return 2 ;;
	esac
}

# Return 0 for an active swap partition, 1 when a complete /proc/swaps
# inventory proves it is inactive, and 2 when the inventory is unknown.
spindown_swap_status() {
	spindown_device=$1
	spindown_swaps_file=$2

	if [ -z "$spindown_device" ] || [ -z "$spindown_swaps_file" ] ||
	   [ ! -r "$spindown_swaps_file" ]; then
		return 2
	fi
	if spindown_swap_snapshot=$(cat "$spindown_swaps_file" && printf '.'); then
		case "$spindown_swap_snapshot" in
			*.) spindown_swap_snapshot=${spindown_swap_snapshot%.} ;;
			*) return 2 ;;
		esac
	else
		return 2
	fi
	case "$spindown_swap_snapshot" in
		*'
') ;;
		*) return 2 ;;
	esac
	printf '%s' "$spindown_swap_snapshot" | awk -v dev="/dev/$spindown_device" '
NR == 1 {
	if (NF != 5 || $1 != "Filename" || $2 != "Type" || $3 != "Size" ||
	    $4 != "Used" || $5 != "Priority")
		invalid = 1
	next
}
NF == 0 { next }
{
	if (NF != 5 || ($2 != "partition" && $2 != "file") ||
	    $3 !~ /^[0-9]+$/ || $4 !~ /^[0-9]+$/ || $5 !~ /^-?[0-9]+$/) {
		invalid = 1
		next
	}
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
	if (NR == 0 || invalid)
		exit 2
	exit !found
}'
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

# /proc/mounts does not list active swap partitions.
spindown_swap_status "$1" /proc/swaps
swap_status=$?
case "$swap_status" in
	0) exit 1 ;; # active swap; do not spin down
	1) ;;        # complete swap inventory confirms no active partition
	*) exit 1 ;; # cannot prove the disk has no active swap
esac

if [ ! -r "/sys/block/$1/removable" ]; then
	logger -t "Storage" "cannot verify removable state for /dev/$1"
	exit 1
fi
if ! dev_removable=$(cat "/sys/block/$1/removable" 2>/dev/null); then
	logger -t "Storage" "cannot read removable state for /dev/$1"
	exit 1
fi
case "$dev_removable" in
	0) ;;
	1) exit 0 ;; # Removable media is intentionally not spun down here.
	*)
		logger -t "Storage" "invalid removable state for /dev/$1: $dev_removable"
		exit 1
		;;
esac

if /sbin/hdparm -y "/dev/$1"; then
	exit 0
else
	hdparm_status=$?
	logger -t "Storage" "hdparm failed for /dev/$1 (status=$hdparm_status)"
	exit "$hdparm_status"
fi
