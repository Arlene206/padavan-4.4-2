#!/bin/sh
# Serialize Optware/Entware startup and teardown. This lock is intentionally
# separate from storage.lock: the C caller owns storage topology, while this
# shell lock prevents delayed service startup racing with an eject's stop hook.

_optware_lifecycle_lock_open()
{
	optware_lock_file=${1:-/var/lock/optware-lifecycle.lock}
	optware_lock_old_umask=`umask`
	umask 077
	if ! ( : >> "$optware_lock_file" ); then
		umask "$optware_lock_old_umask"
		return 1
	fi

	exec 9>>"$optware_lock_file"
	umask "$optware_lock_old_umask"
	return 0
}

optware_lifecycle_lock_acquire()
{
	if ! _optware_lifecycle_lock_open "$1"; then
		return 1
	fi
	if ! flock 9; then
		exec 9>&-
		return 1
	fi
	return 0
}

# Teardown runs below storage.lock. A non-blocking acquire avoids reversing
# the lock order when an Optware startup hook itself requests storage eject.
# In that case eject fails closed and can be retried after startup completes.
optware_lifecycle_lock_try_acquire()
{
	if ! _optware_lifecycle_lock_open "$1"; then
		return 1
	fi
	if ! flock -n 9; then
		exec 9>&-
		return 1
	fi
	return 0
}

optware_lifecycle_lock_release()
{
	# Callers must close fd 9 in child commands to avoid inherited locks.
	exec 9>&-
}
