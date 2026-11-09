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

# Stop every executable service in reverse startup order. Continue after an
# individual failure so remaining services still get a chance to release state.
# The caller must keep /opt mounted if any stop operation cannot complete.
optware_lifecycle_stop_services()
{
	_optware_service_dir=
	[ "$#" -gt 0 ] || return 1
	_optware_service_dir=$1
	_optware_stop_failed=0

	if [ -z "$_optware_service_dir" ] || [ ! -d "$_optware_service_dir" ]; then
		return 1
	fi

	# An empty init.d directory is valid. If scripts exist but the directory
	# cannot be enumerated, fail closed rather than treating it as empty.
	set -- "$_optware_service_dir"/S??*
	[ -e "$1" ] || return 0
	_optware_service_list=`ls -r "$_optware_service_dir"/S??* 2>/dev/null` || {
		logger -t "opt-umount.sh" "cannot enumerate Optware service scripts" || :
		return 1
	}

	for _optware_service in $_optware_service_list; do
		[ -x "$_optware_service" ] || continue
		if ! "$_optware_service" stop 9>&-; then
			logger -t "opt-umount.sh" "service stop failed: $_optware_service" || :
			_optware_stop_failed=1
		fi
	done

	return "$_optware_stop_failed"
}

# Retry one failed /opt unmount, but never report success when both attempts
# fail. Keep fd 9 closed in child processes so they cannot inherit the lock.
optware_lifecycle_umount_opt()
{
	if umount /opt 9>&-; then
		return 0
	fi

	logger -t "opt-umount.sh" "umount /opt FAILED! Device is busy? Retrying." || :
	sleep 1 || :
	if umount /opt 2>/dev/null 9>&-; then
		return 0
	fi

	logger -t "opt-umount.sh" "retry umount /opt FAILED!" || :
	return 1
}

optware_lifecycle_lock_release()
{
	# Callers must close fd 9 in child commands to avoid inherited locks.
	exec 9>&-
}
