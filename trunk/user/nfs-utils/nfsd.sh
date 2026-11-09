#!/bin/sh

# Return 0 when exactly the expected source/filesystem is mounted at the
# path, 1 when the path is unused, and 2 for conflicts or inventory errors.
func_mount_status()
{
	expected_mountpoint="$1"
	expected_filesystem="$2"
	expected_source="$3"

	mount_result=$(awk -v mountpoint="$expected_mountpoint" \
		-v filesystem="$expected_filesystem" \
		-v expected_source="$expected_source" '
		$2 == mountpoint {
			entry_count++
			if ($1 == expected_source && $3 == filesystem)
				expected_count++
		}
		END {
			if (entry_count == 0)
				print "absent"
			else if (entry_count == 1 && expected_count == 1)
				print "present"
			else
				print "conflict"
		}' /proc/mounts)
	awk_status=$?
	[ "$awk_status" -eq 0 ] || return 2

	case "$mount_result" in
	present)
		return 0
		;;
	absent)
		return 1
		;;
	*)
		return 2
		;;
	esac
}

func_mount_if_needed()
{
	mountpoint="$1"
	filesystem="$2"
	source="$3"

	func_mount_status "$mountpoint" "$filesystem" "$source"
	mount_status=$?
	case "$mount_status" in
	0)
		return 0
		;;
	1)
		mount -t "$filesystem" "$source" "$mountpoint" || return 1
		func_mount_status "$mountpoint" "$filesystem" "$source"
		mount_status=$?
		[ "$mount_status" -eq 0 ] && return 0
		logger -t "NFS server" "mount command did not publish the expected $filesystem mount at $mountpoint"
		return 1
		;;
	*)
		logger -t "NFS server" "refusing conflicting or unreadable mount state at $mountpoint"
		return 1
		;;
	esac
}

func_start()
{
	# rpcbind is required by the NFS user-space services.
	if [ -z "$(pidof rpcbind)" ] ; then
		/sbin/rpcbind || {
			logger -t "NFS server" "cannot start rpcbind"
			return 1
		}
		sleep 1
		if [ -z "$(pidof rpcbind)" ] ; then
			logger -t "NFS server" "rpcbind command succeeded but the daemon is not running"
			return 1
		fi
	fi

	# A complete running service only needs its exports reloaded.
	if [ -n "$(pidof nfsd)" ] && [ -n "$(pidof rpc.mountd)" ] ; then
		if [ ! -f /etc/exports ] ; then
			logger -t "NFS server" "cannot reload exports because /etc/exports is missing"
			return 1
		fi
		/sbin/exportfs -r || {
			logger -t "NFS server" "cannot reload NFS exports"
			return 1
		}
		return 0
	fi

	# Do not erase export/recovery state while only half of the service is alive.
	if [ -n "$(pidof nfsd)" ] || [ -n "$(pidof rpc.mountd)" ] ; then
		logger -t "NFS server" "refusing to reset NFS state while only one server daemon is running"
		return 1
	fi

	if ! touch /etc/exports ; then
		logger -t "NFS server" "cannot create /etc/exports"
		return 1
	fi

	if ! mkdir -p /var/lib/nfs/sm /var/lib/nfs/sm.bak /var/lib/nfs/rpc_pipefs ; then
		logger -t "NFS server" "cannot create NFS state directories"
		return 1
	fi

	: > /var/lib/nfs/etab || {
		logger -t "NFS server" "cannot initialize NFS export state"
		return 1
	}
	: > /var/lib/nfs/rmtab || {
		logger -t "NFS server" "cannot initialize NFS client state"
		return 1
	}
	: > /var/lib/nfs/xtab || {
		logger -t "NFS server" "cannot initialize NFS export compatibility state"
		return 1
	}

	grep -q nfsd /proc/filesystems
	filesystem_status=$?
	if [ "$filesystem_status" -eq 1 ] ; then
		modprobe -q nfsd || {
			logger -t "NFS server" "cannot load the nfsd kernel module"
			return 1
		}
	elif [ "$filesystem_status" -ne 0 ] ; then
		logger -t "NFS server" "cannot inspect /proc/filesystems"
		return 1
	fi

	# The module can remain loaded after its control filesystem was unmounted.
	func_mount_if_needed /proc/fs/nfsd nfsd nfsd || {
		logger -t "NFS server" "cannot mount the nfsd control filesystem"
		func_stop || logger -t "NFS server" "NFS startup cleanup after nfsd control mount failure was incomplete"
		return 1
	}
	func_mount_if_needed /var/lib/nfs/rpc_pipefs rpc_pipefs rpc_pipefs || {
		logger -t "NFS server" "cannot mount rpc_pipefs"
		func_stop || logger -t "NFS server" "NFS startup cleanup after rpc_pipefs mount failure was incomplete"
		return 1
	}

	# This kernel tuning value is optional; a missing knob must not hide RPC failures.
	echo 65536 > /proc/fs/nfsd/max_block_size ||
		logger -t "NFS server" "cannot set nfsd max_block_size"

	/sbin/exportfs -r || {
		logger -t "NFS server" "cannot publish NFS exports"
		func_stop || logger -t "NFS server" "NFS startup cleanup after export publication failure was incomplete"
		return 1
	}

	/sbin/rpc.nfsd -N4 -N4.1 1 || {
		logger -t "NFS server" "cannot start nfsd"
		func_stop || logger -t "NFS server" "NFS startup cleanup after nfsd failure was incomplete"
		return 1
	}
	if [ -z "$(pidof nfsd)" ] ; then
		logger -t "NFS server" "rpc.nfsd returned success but nfsd is not running"
		func_stop || logger -t "NFS server" "NFS startup cleanup after nfsd failure was incomplete"
		return 1
	fi

	/sbin/rpc.mountd || {
		logger -t "NFS server" "cannot start rpc.mountd"
		func_stop || logger -t "NFS server" "NFS startup cleanup after rpc.mountd failure was incomplete"
		return 1
	}
	if [ -z "$(pidof rpc.mountd)" ] ; then
		logger -t "NFS server" "rpc.mountd returned success but the daemon is not running"
		func_stop || logger -t "NFS server" "NFS startup cleanup after rpc.mountd failure was incomplete"
		return 1
	fi

	logger -t "NFS server" "daemon is started"
	return 0
}

func_reload()
{
	if [ -z "$(pidof rpcbind)" ] ; then
		logger -t "NFS server" "cannot reload NFS exports because rpcbind is not running"
		return 1
	fi
	if [ -z "$(pidof nfsd)" ] || [ -z "$(pidof rpc.mountd)" ] ; then
		logger -t "NFS server" "cannot reload NFS exports because the NFS daemons are not both running"
		return 1
	fi
	if [ ! -f /etc/exports ] ; then
		logger -t "NFS server" "cannot reload NFS exports because /etc/exports is missing"
		return 1
	fi
	/sbin/exportfs -r || {
		logger -t "NFS server" "cannot reload NFS exports"
		return 1
	}
	return 0
}

func_unmount_if_mounted()
{
	mountpoint="$1"
	filesystem="$2"
	source="$3"

	func_mount_status "$mountpoint" "$filesystem" "$source"
	mount_status=$?
	case "$mount_status" in
	0)
		;;
	1)
		return 0
		;;
	*)
		logger -t "NFS server" "refusing to unmount conflicting or unreadable mount state at $mountpoint"
		return 1
		;;
	esac

	umount "$mountpoint" 2>/dev/null || return 1
	func_mount_status "$mountpoint" "$filesystem" "$source"
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
		func_unmount_if_mounted /var/lib/nfs/rpc_pipefs rpc_pipefs rpc_pipefs || status=1
	fi
	if [ "$status" -eq 0 ] ; then
		func_unmount_if_mounted /proc/fs/nfsd nfsd nfsd || status=1
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

