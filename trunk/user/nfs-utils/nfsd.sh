#!/bin/sh

proc_root="${NFSD_PROC_ROOT:-/proc}"
nfsd_kill_command="kill"

# Return 0 when exactly the expected source/filesystem is mounted at the
# path, 1 when the path is unused, and 2 for conflicts or inventory errors.
func_mount_status()
{
	expected_mountpoint="$1"
	expected_filesystem="$2"
	expected_source="$3"

	# /proc/mounts is complete only when every record is a well-formed
	# six-field mount row. An unrelated truncated row must not make a target
	# mount look absent and authorize mount, unmount or export-state cleanup.
	mount_result=$(awk -v mountpoint="$expected_mountpoint" \
		-v filesystem="$expected_filesystem" \
		-v expected_source="$expected_source" '
		{
			record_count++
			if (NF != 6 || $1 == "" || $2 == "" || $3 == "" ||
			    $4 == "" || $5 !~ /^[0-9]+$/ || $6 !~ /^[0-9]+$/) {
				inventory_invalid = 1
				next
			}
			if ($2 == mountpoint) {
				entry_count++
				if ($1 == expected_source && $3 == filesystem)
					expected_count++
			}
		}
		END {
			if (inventory_invalid || record_count == 0)
				print "invalid"
			else if (entry_count == 0)
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

# Return 0 when a matching process is running, 1 when absent and 2 when
# procfs cannot provide a complete answer. UNKNOWN must never authorize
# destructive NFS state reset, export withdrawal, or pseudo-filesystem unmount.
func_process_presence()
{
	local wanted_name="$1"
	local process_dir process_comm

	[ -d "$proc_root" ] && [ -r "$proc_root" ] || return 2
	# An accessible ordinary directory at the /proc mountpoint is not a
	# complete process inventory. Require the procfs self descriptor.
	[ -r "$proc_root/self/comm" ] || return 2
	if ! IFS= read -r process_comm < "$proc_root/self/comm"; then
		return 2
	fi
	[ -n "$process_comm" ] || return 2
	for process_dir in "$proc_root"/[0-9]*; do
		[ -d "$process_dir" ] || continue
		if ! IFS= read -r process_comm < "$process_dir/comm"; then
			[ ! -d "$process_dir" ] && continue
			return 2
		fi
		[ -n "$process_comm" ] || return 2
		[ "$process_comm" = "$wanted_name" ] && return 0
	done
	return 1
}


# Capture a process start-time from field 22 of /proc/PID/stat. The comm
# field is parenthesized and may itself contain spaces or closing parens;
# discard through its final ") " delimiter before counting numeric fields.
func_process_start_time()
{
	local process_pid="$1"
	local process_dir="$proc_root/$process_pid"
	local stat_line stat_tail process_start_time
	local IFS=' '

	[ -d "$process_dir" ] || return 1
	if ! IFS= read -r stat_line < "$process_dir/stat"; then
		[ ! -d "$process_dir" ] && return 1
		return 2
	fi
	case "$stat_line" in
		"$process_pid ("*) ;;
		*) return 2 ;;
	esac
	stat_tail=${stat_line##*) }
	[ "$stat_tail" != "$stat_line" ] || return 2
	set -- $stat_tail
	[ "$#" -ge 20 ] || return 2
	process_start_time=${20}
	case "$process_start_time" in
		""|*[!0-9]*) return 2 ;;
	esac
	printf '%s\n' "$process_start_time"
	return 0
}

# Snapshot only exact process names, recording PID and kernel start-time.
# An unreadable/malformed matching process identity makes the whole snapshot
# UNKNOWN; callers must not signal any process from a partial snapshot.
func_snapshot_processes()
{
	local wanted_name="$1"
	local process_dir process_pid process_comm process_start_time start_status

	for process_dir in "$proc_root"/[0-9]*; do
		[ -d "$process_dir" ] || continue
		process_pid=${process_dir##*/}
		case "$process_pid" in
			""|*[!0-9]*) continue ;;
		esac
		if ! IFS= read -r process_comm < "$process_dir/comm"; then
			[ ! -d "$process_dir" ] && continue
			return 2
		fi
		[ "$process_comm" = "$wanted_name" ] || continue

		process_start_time=$(func_process_start_time "$process_pid")
		start_status=$?
		if [ "$start_status" -ne 0 ]; then
			[ "$start_status" -eq 1 ] && [ ! -d "$process_dir" ] && continue
			return 2
		fi
		printf '%s %s\n' "$process_pid" "$process_start_time" || return 2
	done
	return 0
}

# Revalidate both comm and start-time immediately before signalling. Linux
# 4.4 has no pidfd interface, so this narrows (but cannot eliminate) the
# final check-to-kill PID-reuse window.
func_process_identity_matches()
{
	local process_pid="$1"
	local wanted_name="$2"
	local expected_start_time="$3"
	local process_dir="$proc_root/$process_pid"
	local process_comm actual_start_time start_status

	case "$process_pid" in
		""|*[!0-9]*) return 2 ;;
	esac
	[ -d "$process_dir" ] || return 1
	if ! IFS= read -r process_comm < "$process_dir/comm"; then
		[ ! -d "$process_dir" ] && return 1
		return 2
	fi
	[ "$process_comm" = "$wanted_name" ] || return 1

	actual_start_time=$(func_process_start_time "$process_pid")
	start_status=$?
	case "$start_status" in
		0) ;;
		1) return 1 ;;
		*) return 2 ;;
	esac
	[ "$actual_start_time" = "$expected_start_time" ] && return 0
	return 1
}

# Signal only identities captured in a complete snapshot and revalidated
# just before kill(2). A process that vanished or changed identity is skipped.
func_signal_snapshot()
{
	local signal_number="$1"
	local wanted_name="$2"
	local process_snapshot="$3"
	local signal_status=0
	local process_pid process_start_time identity_status

	while IFS=' ' read -r process_pid process_start_time; do
		[ -n "$process_pid" ] || continue
		func_process_identity_matches "$process_pid" "$wanted_name" "$process_start_time"
		identity_status=$?
		case "$identity_status" in
			0) ;;
			1) continue ;;
			*)
				logger -t "NFS server" "process identity became unknown before signal (name=$wanted_name pid=$process_pid)"
				signal_status=1
				continue
				;;
		esac

		if ! "$nfsd_kill_command" "-$signal_number" "$process_pid"; then
			func_process_identity_matches "$process_pid" "$wanted_name" "$process_start_time"
			identity_status=$?
			if [ "$identity_status" -eq 0 ] || [ "$identity_status" -eq 2 ]; then
				logger -t "NFS server" "cannot signal verified process (name=$wanted_name pid=$process_pid signal=$signal_number)"
				signal_status=1
			fi
		fi
	done <<EOF
$process_snapshot
EOF
	return "$signal_status"
}


# rpcbind and rpc.mountd can daemonize before their children become visible
# in procfs. Use a bounded readiness window rather than one immediate
# snapshot. UNKNOWN aborts immediately; ABSENT is retried.
func_wait_process_running()
{
	wanted_name="$1"
	attempt=0

	while [ "$attempt" -lt 4 ]; do
		func_process_presence "$wanted_name"
		presence=$?
		case "$presence" in
		0)
			return 0
			;;
		1)
			;;
		*)
			return 2
			;;
		esac

		attempt=$((attempt + 1))
		[ "$attempt" -lt 4 ] || break
		sleep 1 || return 2
	done

	return 1
}

func_start()
{
	func_process_presence rpcbind
	rpcbind_state=$?
	case "$rpcbind_state" in
	0)
		;;
	1)
		/sbin/rpcbind || {
			logger -t "NFS server" "cannot start rpcbind"
			return 1
		}
		func_wait_process_running rpcbind
		rpcbind_state=$?
		if [ "$rpcbind_state" -ne 0 ]; then
			logger -t "NFS server" "rpcbind command returned success but the daemon did not become ready (state=$rpcbind_state)"
			return 1
		fi
		;;
	*)
		logger -t "NFS server" "cannot verify rpcbind process inventory; refusing NFS startup"
		return 1
		;;
	esac

	# A partial process scan must not be mistaken for an empty NFS service.
	func_process_presence nfsd
	nfsd_state=$?
	func_process_presence rpc.mountd
	mountd_state=$?
	if [ "$nfsd_state" -eq 2 ] || [ "$mountd_state" -eq 2 ]; then
		logger -t "NFS server" "cannot verify NFS daemon inventory; refusing to reset NFS state"
		return 1
	fi

	# A complete running service only needs its exports reloaded.
	if [ "$nfsd_state" -eq 0 ] && [ "$mountd_state" -eq 0 ]; then
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
	if [ "$nfsd_state" -eq 0 ] || [ "$mountd_state" -eq 0 ]; then
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
	func_process_presence nfsd
	nfsd_state=$?
	if [ "$nfsd_state" -ne 0 ]; then
		logger -t "NFS server" "rpc.nfsd returned success but nfsd is not running"
		func_stop || logger -t "NFS server" "NFS startup cleanup after nfsd failure was incomplete"
		return 1
	fi

	/sbin/rpc.mountd || {
		logger -t "NFS server" "cannot start rpc.mountd"
		func_stop || logger -t "NFS server" "NFS startup cleanup after rpc.mountd failure was incomplete"
		return 1
	}
	func_wait_process_running rpc.mountd
	mountd_state=$?
	if [ "$mountd_state" -ne 0 ]; then
		logger -t "NFS server" "rpc.mountd returned success but the daemon did not become ready (state=$mountd_state)"
		func_stop || logger -t "NFS server" "NFS startup cleanup after rpc.mountd failure was incomplete"
		return 1
	fi

	# Logging is best-effort after both NFS daemons have passed readiness checks.
	logger -t "NFS server" "daemon is started" >/dev/null 2>&1 || true
	return 0
}

func_reload()
{
	func_process_presence rpcbind
	rpcbind_state=$?
	if [ "$rpcbind_state" -eq 2 ]; then
		logger -t "NFS server" "cannot reload NFS exports because rpcbind process inventory is unknown"
		return 1
	fi
	if [ "$rpcbind_state" -ne 0 ]; then
		logger -t "NFS server" "cannot reload NFS exports because rpcbind is not running"
		return 1
	fi
	func_process_presence nfsd
	nfsd_state=$?
	func_process_presence rpc.mountd
	mountd_state=$?
	if [ "$nfsd_state" -eq 2 ] || [ "$mountd_state" -eq 2 ]; then
		logger -t "NFS server" "cannot reload NFS exports because NFS daemon inventory is unknown"
		return 1
	fi
	if [ "$nfsd_state" -ne 0 ] || [ "$mountd_state" -ne 0 ]; then
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
	func_process_presence rpc.mountd
	mountd_state=$?
	func_process_presence nfsd
	nfsd_state=$?
	if [ "$mountd_state" -eq 2 ] || [ "$nfsd_state" -eq 2 ]; then
		logger -t "NFS server" "cannot verify NFS daemon inventory; refusing to withdraw exports or unmount filesystems"
		return 1
	fi

	# Take complete PID/start-time snapshots for both daemons before the
	# first signal. A partial snapshot must not authorize name-wide cleanup.
	mountd_snapshot=$(func_snapshot_processes rpc.mountd) || {
		logger -t "NFS server" "cannot snapshot rpc.mountd identities; refusing stop"
		return 1
	}
	nfsd_snapshot=$(func_snapshot_processes nfsd) || {
		logger -t "NFS server" "cannot snapshot nfsd identities; refusing stop"
		return 1
	}

	# Preserve the legacy signal semantics: TERM for rpc.mountd, SIGINT for
	# kernel nfsd. Only captured and just-revalidated process identities are hit.
	func_signal_snapshot TERM rpc.mountd "$mountd_snapshot" ||
		logger -t "NFS server" "one or more rpc.mountd TERM signals failed"
	func_signal_snapshot 2 nfsd "$nfsd_snapshot" ||
		logger -t "NFS server" "one or more nfsd SIGINT signals failed"

	for i in 1 2 3 ; do
		func_process_presence rpc.mountd
		mountd_state=$?
		func_process_presence nfsd
		nfsd_state=$?
		if [ "$mountd_state" -eq 2 ] || [ "$nfsd_state" -eq 2 ]; then
			logger -t "NFS server" "NFS daemon inventory became unknown during graceful stop"
			return 1
		fi
		[ "$mountd_state" -eq 1 ] && [ "$nfsd_state" -eq 1 ] && break
		sleep 1
	done

	# Force termination only after the graceful stop window has expired.
	if [ "$mountd_state" -eq 0 ] || [ "$nfsd_state" -eq 0 ]; then
		mountd_snapshot=$(func_snapshot_processes rpc.mountd) || {
			logger -t "NFS server" "cannot snapshot rpc.mountd identities for forced stop"
			return 1
		}
		nfsd_snapshot=$(func_snapshot_processes nfsd) || {
			logger -t "NFS server" "cannot snapshot nfsd identities for forced stop"
			return 1
		}
		func_signal_snapshot 9 rpc.mountd "$mountd_snapshot" ||
			logger -t "NFS server" "one or more rpc.mountd SIGKILL signals failed"
		func_signal_snapshot 9 nfsd "$nfsd_snapshot" ||
			logger -t "NFS server" "one or more nfsd SIGKILL signals failed"
		for i in 1 2 3 ; do
			func_process_presence rpc.mountd
			mountd_state=$?
			func_process_presence nfsd
			nfsd_state=$?
			if [ "$mountd_state" -eq 2 ] || [ "$nfsd_state" -eq 2 ]; then
				logger -t "NFS server" "NFS daemon inventory became unknown during forced stop"
				return 1
			fi
			[ "$mountd_state" -eq 1 ] && [ "$nfsd_state" -eq 1 ] && break
			sleep 1
		done
	fi

	# Never unexport or detach pseudo-filesystems while either daemon survives.
	func_process_presence rpc.mountd
	mountd_state=$?
	func_process_presence nfsd
	nfsd_state=$?
	if [ "$mountd_state" -ne 1 ] || [ "$nfsd_state" -ne 1 ]; then
		logger -t "NFS server" "stop incomplete; retaining NFS state"
		return 1
	fi

	# etab records exported state. Keep it as recovery evidence on any failure.
	if [ -f /var/lib/nfs/etab ] ; then
		/sbin/exportfs -ua || return 1
	fi
	func_unmount_if_mounted /var/lib/nfs/rpc_pipefs rpc_pipefs rpc_pipefs || return 1
	func_unmount_if_mounted /proc/fs/nfsd nfsd nfsd || return 1
	rm -f /var/lib/nfs/etab || return 1
	return 0
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

