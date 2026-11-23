#!/bin/sh

self_name="opt-umount.sh"

# check params
[ -z "$1" ] || [ -z "$2" ] && exit 1

. /sbin/optware_lifecycle_lock.sh || exit 1
if ! optware_lifecycle_lock_try_acquire; then
	logger -t "${self_name}" "Optware lifecycle is busy; refusing unmount so storage remains attached"
	exit 1
fi

# Recheck /opt while holding the same lock as opt-start.sh.
if ! mountpoint -q /opt ; then
	optware_lifecycle_lock_release
	exit 0
fi

# Only the exact device+mountpoint row owns the Optware teardown. A
# substring match could mistake /dev/sda10 for /dev/sda1 and stop services
# while the actual /opt filesystem belongs to a different partition.
if optware_lifecycle_mount_matches "$1" "$2" /proc/mounts; then
	:
else
	mount_match_status=$?
	if [ "$mount_match_status" -eq 1 ]; then
		optware_lifecycle_lock_release
		exit 0
	fi
	logger -t "$self_name" "cannot verify exact /opt mount owner for [$1] [$2]; refusing teardown"
	optware_lifecycle_lock_release
	exit 1
fi

logger -t "${self_name}" "started [$@]"

# Package managers are shell scripts, so their process comm is "sh";
# killall ipkg/opkg does not reliably find these command lines. Refuse to
# detach /opt while a script/update client is active or its inventory unknown.
. /usr/lib/process_lifecycle.sh || {
	logger -t "$self_name" "cannot load shared process lifecycle checks; refusing unmount"
	optware_lifecycle_lock_release
	exit 1
}

optware_lifecycle_check_package_clients()
{
	for package_pattern in \
		'(^|[[:space:]])/usr/bin/ipkg[.]sh([[:space:]]|$)' \
		'(^|[[:space:]])/usr/bin/opkg[.]sh([[:space:]]|$)' \
		'(^|[[:space:]])/usr/bin/opt-ipkg-upd[.]sh([[:space:]]|$)' \
		'(^|[[:space:]])/usr/bin/opt-opkg-upd[.]sh([[:space:]]|$)'; do
		if pl_process_pattern_state "$package_pattern"; then
			logger -t "$self_name" "package manager is still active; refusing unmount: $package_pattern"
			return 1
		else
			package_pattern_status=$?
			if [ "$package_pattern_status" -ne 1 ]; then
				logger -t "$self_name" "cannot verify package manager state; refusing unmount: $package_pattern"
				return 1
			fi
		fi
	done

	# The script wrappers must be absent before stopping any remaining binaries.
	for package_binary in ipkg opkg mc; do
		if ! pl_stop_named "$package_binary" 5 2; then
			logger -t "$self_name" "package client did not stop cleanly: $package_binary"
			return 1
		fi
	done
	return 0
}

if ! optware_lifecycle_check_package_clients; then
	logger -t "$self_name" "package clients are active or process inventory is unknown; refusing /opt teardown"
	optware_lifecycle_lock_release
	exit 1
fi

# Stop every service even if one fails, but do not detach /opt unless all
# service stop hooks succeeded. The storage caller will retain the mount and
# retry later when the external service failure is corrected.
if [ -d /opt/etc/init.d ] ; then
	if ! optware_lifecycle_stop_services /opt/etc/init.d; then
		logger -t "${self_name}" "one or more Optware services could not be stopped; refusing unmount"
		optware_lifecycle_lock_release
		exit 1
	fi
fi

# Do not change home/localtime or unmount /opt until swap release is
# confirmed from a complete /proc/swaps inventory.
if ! optware_lifecycle_swapoff_file /opt/.swap; then
	logger -t "${self_name}" "cannot confirm /opt/.swap is inactive; refusing cleanup and unmount"
	optware_lifecycle_lock_release
	exit 1
fi

# restore home
rm -f /home/admin
ln -sf /home/root /home/admin

rm -f /etc/localtime

# flush buffers
sync

# unbinding; return failure if both attempts fail so the C storage caller
# cannot proceed to detach a still-mounted /opt filesystem.
if ! optware_lifecycle_umount_opt; then
	optware_lifecycle_lock_release
	exit 1
fi

optware_lifecycle_lock_release
exit 0
