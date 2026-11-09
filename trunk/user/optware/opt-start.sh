#!/bin/sh

self_name="opt-start.sh"

optw_enable=`nvram get optw_enable`
[ "$optw_enable" != "1" -a "$optw_enable" != "2" ] && exit 0

. /sbin/optware_lifecycle_lock.sh || exit 1
if ! optware_lifecycle_lock_acquire; then
	logger -t "${self_name}" "cannot acquire Optware lifecycle lock; refusing startup"
	exit 1
fi

# Recheck configuration and mount state after entering the lifecycle critical section.
optw_enable=`nvram get optw_enable`
if [ "$optw_enable" != "1" -a "$optw_enable" != "2" ]; then
	optware_lifecycle_lock_release
	exit 0
fi
if ! mountpoint -q /opt ; then
	optware_lifecycle_lock_release
	exit 0
fi

logger -t "${self_name}" "call /opt/etc/init.d"

# extend path to /opt
export PATH=/opt/sbin:/opt/bin:/usr/sbin:/usr/bin:/sbin:/bin

# start all services S* in /opt/etc/init.d
for i in `ls /opt/etc/init.d/S??* 2>/dev/null` ; do
	[ ! -x "${i}" ] && continue
	${i} start 9>&-
done

# install and update ipkg/opkg in background
if [ "$optw_enable" == "1" ] ; then
	if [ ! -f /opt/bin/ipkg ] ; then
		if [ -f /opt/bin/opkg ] ; then
			logger -t "Optware:" "WARNING! Entware detected (/opt). Please remove Entware first!"
			optware_lifecycle_lock_release
			exit 0
		fi
		/usr/bin/opt-ipkg-upd.sh 9>&-
	fi
elif [ "$optw_enable" == "2" ] ; then
	if [ ! -f /opt/bin/opkg ] ; then
		if [ -f /opt/bin/ipkg ] ; then
			logger -t "Entware:" "WARNING! Optware detected (/opt). Please remove Optware first!"
			optware_lifecycle_lock_release
			exit 0
		fi
		/usr/bin/opt-opkg-upd.sh 9>&-
	fi
fi

optware_lifecycle_lock_release
