#!/bin/sh
# Serialize Optware/Entware startup and teardown. This lock is intentionally
# separate from storage.lock: the C caller owns storage topology, while this
# shell lock prevents delayed service startup racing with an eject's stop hook.

_optware_lifecycle_lock_open()
{
	optware_lock_file=${1:-/tmp/optware-lifecycle.lock}

	case "$optware_lock_file" in
	/*) ;;
	*) return 1 ;;
	esac
	optware_lock_dir=${optware_lock_file%/*}
	[ -n "$optware_lock_dir" ] || optware_lock_dir=/

	# The production directory is root-owned sticky /tmp. Test callers may
	# use a private owner-only directory. Never follow a replaceable path.
	if [ -L "$optware_lock_dir" ] || [ ! -d "$optware_lock_dir" ] ||
	   [ -L "$optware_lock_file" ] || [ ! -f "$optware_lock_file" ]; then
		return 1
	fi

	optware_lock_uid=$(id -u 2>/dev/null) || return 1
	optware_lock_dir_owner=$(stat -c %u "$optware_lock_dir" 2>/dev/null) || return 1
	optware_lock_dir_mode=$(stat -c %a "$optware_lock_dir" 2>/dev/null) || return 1
	case "$optware_lock_dir_mode" in
	''|*[!01234567]*) return 1 ;;
	esac
	optware_lock_dir_mode_value=$((0$optware_lock_dir_mode))
	optware_lock_dir_write_bits=$((optware_lock_dir_mode_value & 0022))
	optware_lock_dir_sticky=$((optware_lock_dir_mode_value & 01000))
	if [ "$optware_lock_dir_owner" != "$optware_lock_uid" ] ||
	   { [ "$optware_lock_dir_write_bits" -ne 0 ] &&
	     [ "$optware_lock_dir_sticky" -eq 0 ]; }; then
		return 1
	fi

	optware_lock_owner=$(stat -c %u "$optware_lock_file" 2>/dev/null) || return 1
	optware_lock_links=$(stat -c %h "$optware_lock_file" 2>/dev/null) || return 1
	optware_lock_mode=$(stat -c %a "$optware_lock_file" 2>/dev/null) || return 1
	if [ "$optware_lock_owner" != "$optware_lock_uid" ] ||
	   [ "$optware_lock_links" != 1 ] || [ "$optware_lock_mode" != 600 ]; then
		return 1
	fi

	# dev_init provisions this inode before mdev. Open only after all
	# ownership, mode, link-count and directory checks have succeeded.
	exec 9>>"$optware_lock_file" || return 1
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

# Return 0 only for one complete /proc/mounts row matching the exact
# source device and /opt mountpoint. Return 1 for a different mount entry;
# return 2 when arguments or the mount inventory cannot be trusted.
optware_lifecycle_mount_matches()
{
	[ "$#" -eq 3 ] || return 2
	[ -n "$1" ] && [ -n "$2" ] && [ -n "$3" ] || return 2
	[ "$2" = "/opt" ] || return 1

	# BusyBox awk 1.24 may collapse nonzero exit values to status 1.
	# Emit an explicit result token so UNKNOWN remains distinct from no match.
	_mount_match_result=$(awk -v wanted_device="$1" -v wanted_mountpoint="$2" '
	{
		records++
		if (NF != 6 || $1 == "" || $2 == "" || $3 == "" ||
		    $4 == "" || $5 !~ /^[0-9]+$/ || $6 !~ /^[0-9]+$/) {
			invalid = 1
			next
		}
		if ($1 == wanted_device && $2 == wanted_mountpoint)
			found++
	}
	END {
		if (records == 0 || invalid || found > 1)
			print "unknown"
		else if (found == 1)
			print "match"
		else
			print "different"
	}' "$3" 2>/dev/null) || return 2

	case $_mount_match_result in
	match) return 0 ;;
	different) return 1 ;;
	unknown) return 2 ;;
	*) return 2 ;;
	esac
}

# Start services in S??* order. If a hook fails, stop that possibly partially
# started hook and every earlier successful hook in reverse order. Positional
# parameters retain complete pathnames (including spaces and newlines), unlike
# parsing `ls` output through command substitution.
# The caller owns the lifecycle lock; children must never inherit fd 9.
optware_lifecycle_start_services()
{
	_optware_service_dir=
	[ "$#" -eq 1 ] || return 1
	_optware_service_dir=$1
	if [ -z "$_optware_service_dir" ] || [ ! -d "$_optware_service_dir" ] ||
	   [ ! -r "$_optware_service_dir" ] || [ ! -x "$_optware_service_dir" ]; then
		logger -t "opt-start.sh" "cannot access Optware service directory: $_optware_service_dir" || :
		return 1
	fi
	shift

	for _optware_service in "$_optware_service_dir"/S??*; do
		[ -e "$_optware_service" ] || continue
		[ -f "$_optware_service" ] && [ -x "$_optware_service" ] || continue
		if "$_optware_service" start 9>&-; then
			# Prepend so the list is already in reverse start order.
			set -- "$_optware_service" "$@"
			continue
		fi

		logger -t "opt-start.sh" "service start failed: $_optware_service" || :
		# A failing start hook may have changed state before returning nonzero.
		set -- "$_optware_service" "$@"
		_optware_rollback_failed=0
		for _optware_rollback_service in "$@"; do
			if ! "$_optware_rollback_service" stop 9>&-; then
				logger -t "opt-start.sh" "service rollback failed: $_optware_rollback_service" || :
				_optware_rollback_failed=1
			fi
		done
		if [ "$_optware_rollback_failed" -ne 0 ]; then
			logger -t "opt-start.sh" "Optware startup rollback was incomplete" || :
		fi
		return 1
	done

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

	# Keep reverse S??* startup order without parsing ls output. Prepending
	# complete pathnames to positional parameters preserves spaces/newlines.
	set --
	for _optware_service in "$_optware_service_dir"/S??*; do
		[ -e "$_optware_service" ] || continue
		[ -f "$_optware_service" ] && [ -x "$_optware_service" ] || continue
		set -- "$_optware_service" "$@"
	done

	for _optware_service in "$@"; do
		if ! "$_optware_service" stop 9>&-; then
			logger -t "opt-umount.sh" "service stop failed: $_optware_service" || :
			_optware_stop_failed=1
		fi
	done

	return "$_optware_stop_failed"
}

# Return 0 when the exact path is active, 1 when inactive, and 2 when the
# procfs inventory is unreadable or malformed. Never interpret an unknown
# inventory as proof that a swap file is inactive.
_optware_lifecycle_swap_is_active()
{
	[ "$#" -eq 2 ] || return 2

	awk -v wanted="$1" -v procfile="$2" '
	BEGIN {
		while ((read_status = (getline line < procfile)) > 0) {
			records++
			$0 = line
			if (records == 1) {
				if (NF != 5 || $1 != "Filename" || $2 != "Type" ||
				    $3 != "Size" || $4 != "Used" || $5 != "Priority")
					invalid = 1
				continue
			}

			if (NF != 5 || ($2 != "file" && $2 != "partition") ||
			    $3 !~ /^[0-9]+$/ || $4 !~ /^[0-9]+$/ ||
			    $5 !~ /^-?[0-9]+$/) {
				invalid = 1
				continue
			}
			if ($1 == wanted) {
				if (found)
					invalid = 1
				found = 1
			}
		}
		if (read_status < 0)
			invalid = 1
		close_status = close(procfile)
		if (close_status != 0)
			invalid = 1
		if (records == 0 || invalid)
			exit 2
		if (found)
			exit 0
		exit 1
	}' 9>&- 2>/dev/null
}

# Release one exact swap path and verify that it disappeared from a complete
# /proc/swaps snapshot. A successful swapoff command alone is not sufficient.
# The optional inventory path exists only for the host regression harness.
optware_lifecycle_swapoff_file()
{
	[ "$#" -ge 1 ] && [ "$#" -le 2 ] && [ -n "$1" ] || return 1
	_optware_swap_path=$1
	_optware_swaps_file=${2:-/proc/swaps}

	if _optware_lifecycle_swap_is_active "$_optware_swap_path" "$_optware_swaps_file"; then
		if ! swapoff "$_optware_swap_path" 9>&- 2>/dev/null; then
			logger -t "opt-umount.sh" "swapoff failed for $_optware_swap_path" 9>&- || :
			return 1
		fi

		if _optware_lifecycle_swap_is_active "$_optware_swap_path" "$_optware_swaps_file"; then
			logger -t "opt-umount.sh" "swap remains active after swapoff: $_optware_swap_path" 9>&- || :
			return 1
		else
			_optware_swap_state=$?
			if [ "$_optware_swap_state" -ne 1 ]; then
				logger -t "opt-umount.sh" "cannot verify swap release for $_optware_swap_path" 9>&- || :
				return 1
			fi
		fi

		logger -t "opt-umount.sh" "Deactivate swap file $_optware_swap_path SUCCESS!" 9>&- || :
		return 0
	else
		_optware_swap_state=$?
		if [ "$_optware_swap_state" -eq 1 ]; then
			return 0
		fi
		logger -t "opt-umount.sh" "cannot read a complete swap inventory; refusing to unmount $_optware_swap_path" 9>&- || :
		return 1
	fi
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
