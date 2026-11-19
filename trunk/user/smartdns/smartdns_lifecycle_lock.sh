#!/bin/sh
# Serialize SmartDNS start, stop and restart actions. The directory creation is
# atomic; dead-owner reclamation renames the old directory before removing it.
smartdns_lifecycle_lock_acquire()
{
	[ "$#" -le 2 ] || return 2
	_smartdns_lock_path=${1:-/tmp/.smartdns.lifecycle.lock}
	_smartdns_lock_attempts=${2:-10}
	case "$_smartdns_lock_attempts" in
		''|*[!0-9]*|0) return 2 ;;
	esac
	smartdns_lifecycle_lock_path=$_smartdns_lock_path
	_smartdns_lock_attempt=0
	while [ "$_smartdns_lock_attempt" -lt "$_smartdns_lock_attempts" ]; do
		if [ -L "$_smartdns_lock_path" ]; then
			return 1
		fi
		if mkdir -m 0700 "$_smartdns_lock_path" 2>/dev/null; then
			if (umask 077; printf '%s\n' "$$" > "$_smartdns_lock_path/pid"); then
				return 0
			fi
			rmdir "$_smartdns_lock_path" 2>/dev/null || :
			return 1
		fi
		if [ -L "$_smartdns_lock_path" ] ||
		   { [ -e "$_smartdns_lock_path" ] && [ ! -d "$_smartdns_lock_path" ]; }; then
			return 1
		fi
		if [ ! -e "$_smartdns_lock_path/pid" ]; then
			sleep 1
			rmdir "$_smartdns_lock_path" 2>/dev/null || :
		elif [ -L "$_smartdns_lock_path/pid" ] || [ ! -f "$_smartdns_lock_path/pid" ]; then
			return 1
		else
			IFS= read -r _smartdns_lock_owner < "$_smartdns_lock_path/pid" || return 1
			case "$_smartdns_lock_owner" in
				''|0|0*|*[!0-9]*) return 1 ;;
			esac
			if kill -0 "$_smartdns_lock_owner" 2>/dev/null; then
				sleep 1
			else
				_smartdns_stale_path="$_smartdns_lock_path.stale.$$.$_smartdns_lock_attempt"
				if [ ! -e "$_smartdns_stale_path" ] &&
				   [ ! -L "$_smartdns_stale_path" ] &&
				   mv "$_smartdns_lock_path" "$_smartdns_stale_path" 2>/dev/null; then
					if [ -f "$_smartdns_stale_path/pid" ] &&
					   [ ! -L "$_smartdns_stale_path/pid" ]; then
						IFS= read -r _smartdns_moved_owner < "$_smartdns_stale_path/pid" || return 1
						[ "$_smartdns_moved_owner" = "$_smartdns_lock_owner" ] || return 1
						rm -f "$_smartdns_stale_path/pid" || return 1
						rmdir "$_smartdns_stale_path" || return 1
					else
						return 1
					fi
					continue
				fi
			fi
		fi
		_smartdns_lock_attempt=$((_smartdns_lock_attempt + 1))
	done
	return 1
}

smartdns_lifecycle_lock_release()
{
	_smartdns_lock_path=${smartdns_lifecycle_lock_path:-/tmp/.smartdns.lifecycle.lock}
	if [ -L "$_smartdns_lock_path" ] || [ ! -d "$_smartdns_lock_path" ] ||
	   [ -L "$_smartdns_lock_path/pid" ] || [ ! -f "$_smartdns_lock_path/pid" ]; then
		return 1
	fi
	IFS= read -r _smartdns_lock_owner < "$_smartdns_lock_path/pid" || return 1
	[ "$_smartdns_lock_owner" = "$$" ] || return 1
	rm -f "$_smartdns_lock_path/pid" || return 1
	rmdir "$_smartdns_lock_path"
}
