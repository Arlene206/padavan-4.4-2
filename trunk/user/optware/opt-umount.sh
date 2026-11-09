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

# check is this partition mounted to /opt
mountres=`grep ' /opt ' /proc/mounts | grep $1`
if [ -z "$mountres" ]; then
	optware_lifecycle_lock_release
	exit 0
fi

logger -t "${self_name}" "started [$@]"

# try to kill ipkg/opkg (may be running)
killall -q ipkg
killall -q opkg

# try to kill mc (may be running)
killall -q mc

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

# check swap file exist
if [ -f /opt/.swap ] ; then
	swapoff /opt/.swap 2>/dev/null
	[ $? -eq 0 ] && logger -t "${self_name}" "Deactivate swap file /opt/.swap SUCCESS!"
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
