#!/bin/sh
# Serialize Optware/Entware startup and teardown. This lock is intentionally
# separate from storage.lock: the C caller owns storage topology, while this
# shell lock prevents delayed service startup racing with an eject's stop hook.

optware_lifecycle_lock_acquire()
{
	optware_lock_file=${1:-/var/lock/optware-lifecycle.lock}
	optware_lock_old_umask=`umask`
	umask 077
	if ! ( : >> "$optware_lock_file" ); then
		umask "$optware_lock_old_umask"
		return 1
	fi

	exec 9>>"$optware_lock_file"
	if ! flock 9; then
		exec 9>&-
		umask "$optware_lock_old_umask"
		return 1
	fi
	umask "$optware_lock_old_umask"
	return 0
}

optware_lifecycle_lock_release()
{
	# Callers must close fd 9 in child commands to avoid inherited locks.
	exec 9>&-
}
