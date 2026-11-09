#!/bin/sh

self_name="opt-umount.sh"

# check params
[ -z "$1" ] || [ -z "$2" ] && exit 1

. /sbin/optware_lifecycle_lock.sh || exit 1
if ! optware_lifecycle_lock_acquire; then
	logger -t "${self_name}" "cannot acquire Optware lifecycle lock; refusing unmount"
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

# stop all services S* in /opt/etc/init.d
if [ -d /opt/etc/init.d ] ; then
	for i in `ls -r /opt/etc/init.d/S??* 2>/dev/null` ; do
		[ ! -x "${i}" ] && continue
		${i} stop 9>&-
	done
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

# unbinding
umount /opt 9>&-
if [ $? -ne 0 ] ; then
	logger -t "${self_name}" "umount /opt FAILED! Device is busy?"
	sleep 1
	umount /opt 2>/dev/null 9>&-
fi

optware_lifecycle_lock_release
