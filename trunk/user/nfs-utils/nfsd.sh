#!/bin/sh

func_start()
{
	# check rpcbind (needed for NFS)
	if [ -z "`pidof rpcbind`" ] ; then
		/sbin/rpcbind
		sleep 1
	fi

	# check NFS server already running
	if [ -n "`pidof nfsd`" ] && [ -n "`pidof rpc.mountd`" ] ; then
		# reload exports only
		[ -f /etc/exports ] && /sbin/exportfs -r
		return 0
	fi

	touch /etc/exports

	mkdir -p /var/lib/nfs/sm
	mkdir -p /var/lib/nfs/sm.bak
	mkdir -p /var/lib/nfs/rpc_pipefs

	echo -n > /var/lib/nfs/etab
	echo -n > /var/lib/nfs/rmtab
	echo -n > /var/lib/nfs/xtab

	if [ -z "`cat /proc/filesystems | grep nfsd`" ] ; then
		modprobe -q nfsd
		mount -t nfsd nfsd /proc/fs/nfsd
	fi

	if ! grep -q /var/lib/nfs/rpc_pipefs /proc/mounts ; then
		mount -t rpc_pipefs rpc_pipefs /var/lib/nfs/rpc_pipefs
	fi

	echo 65536 > /proc/fs/nfsd/max_block_size

	/sbin/exportfs -r

	/sbin/rpc.nfsd -N4 -N4.1 1
	/sbin/rpc.mountd

	if [ $? -eq 0 ] ; then
		logger -t "NFS server" "daemon is started"
	fi
}

func_reload()
{
	if [ -n "`pidof rpcbind`" ] && [ -f /etc/exports ] ; then
		/sbin/exportfs -r
	fi
}

func_unmount_if_mounted()
{
	mountpoint="$1"

	grep -q " $mountpoint " /proc/mounts
	mount_status=$?
	if [ "$mount_status" -eq 1 ]; then
		return 0
	fi
	[ "$mount_status" -eq 0 ] || return 1

	umount "$mountpoint" 2>/dev/null || return 1
	grep -q " $mountpoint " /proc/mounts
	mount_status=$?
	[ "$mount_status" -eq 1 ]
}

func_stop()
{
	status=0

	# Stop rpc.mountd and the kernel NFS server before touching export state.
	[ -n "`pidof rpc.mountd`" ] && killall -q rpc.mountd
	[ -n "`pidof nfsd`" ] && killall -q -2 nfsd

	for i in 1 2 3 ; do
		[ -z "`pidof rpc.mountd`" ] && [ -z "`pidof nfsd`" ] && break
		sleep 1
	done

	# Force termination only after the graceful stop window has expired.
	[ -n "`pidof rpc.mountd`" ] && killall -q -9 rpc.mountd
	[ -n "`pidof nfsd`" ] && killall -q -9 nfsd

	for i in 1 2 3 ; do
		[ -z "`pidof rpc.mountd`" ] && [ -z "`pidof nfsd`" ] && break
		sleep 1
	done

	# Never unexport or detach pseudo-filesystems while either server survives.
	if [ -n "`pidof rpc.mountd`" ] || [ -n "`pidof nfsd`" ] ; then
		status=1
	fi

	# etab records exported state. Keep it as recovery evidence on any failure.
	if [ "$status" -eq 0 ] && [ -f /var/lib/nfs/etab ] ; then
		/sbin/exportfs -ua || status=1
	fi

	if [ "$status" -eq 0 ] ; then
		func_unmount_if_mounted /var/lib/nfs/rpc_pipefs || status=1
	fi
	if [ "$status" -eq 0 ] ; then
		func_unmount_if_mounted /proc/fs/nfsd || status=1
	fi

	if [ "$status" -eq 0 ] ; then
		rm -f /var/lib/nfs/etab || status=1
	fi

	if [ "$status" -ne 0 ] ; then
		logger -t "NFS server" "stop incomplete; retaining NFS state"
	fi
	return "$status"
}

case "$1" in
start)
	func_start
	;;
stop)
	func_stop
	;;
restart)
	func_stop || exit $?
	sleep 1
	func_start
	;;
reload)
	func_reload
	;;
*)
	echo "Usage: $0 {start|stop|restart|reload}"
	exit 1
esac

exit $?

