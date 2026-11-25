#!/bin/sh

dir_storage="${DROPBEAR_STORAGE_DIR:-/etc/storage/dropbear}"
rsa_key="$dir_storage/rsa_host_key"
dss_key="$dir_storage/dss_host_key"
ecdsa_key="$dir_storage/ecdsa_host_key"
ed25519_key="$dir_storage/ed25519_host_key"
extra_args="$dir_storage/extra_args"
old_pattern="${DROPBEAR_OLD_KEY_PREFIX:-/etc/storage/dropbear_}"
dropbear_bin="${DROPBEAR_BIN:-/usr/sbin/dropbear}"
dropbearkey_bin="${DROPBEAR_KEYGEN_BIN:-/usr/bin/dropbearkey}"

func_log()
{
	logger -t "dropbear" "$1"
}

# Return 0=RUNNING, 1=confirmed ABSENT, 2=UNKNOWN.
# BusyBox pidof can return 1 after a failed procfs scan, so validate both
# its status and output before authorizing any daemon lifecycle action.
dropbear_process_state()
{
	if dropbear_pid_output=$(pidof dropbear 2>/dev/null); then
		dropbear_pidof_status=0
	else
		dropbear_pidof_status=$?
	fi
	case "$dropbear_pidof_status" in
	0)
		[ -n "$dropbear_pid_output" ] || return 2
		case "$dropbear_pid_output" in
			*[!0123456789\ ]*) return 2 ;;
		esac
		case $- in
			*f*) dropbear_restore_noglob=0 ;;
			*) set -f; dropbear_restore_noglob=1 ;;
		esac
		set -- $dropbear_pid_output
		[ "$dropbear_restore_noglob" -eq 0 ] || set +f
		[ "$#" -gt 0 ] || return 2
		for dropbear_pid in "$@"; do
			case "$dropbear_pid" in
				''|0*|*[!0-9]*) return 2 ;;
			esac
		done
		return 0
		;;
	1)
		[ -z "$dropbear_pid_output" ] && return 1
		return 2
		;;
	*)
		return 2
		;;
	esac
}

func_keygen()
{
	key_type="$1"
	key_file="$2"
	if "$dropbearkey_bin" -t "$key_type" -f "$key_file"; then
		:
	else
		keygen_status=$?
		func_log "host-key generation failed for $key_type (status=$keygen_status)"
		return "$keygen_status"
	fi
	if chmod 600 "$key_file"; then
		return 0
	else
		chmod_status=$?
		func_log "host-key permission update failed for $key_type (status=$chmod_status)"
		return "$chmod_status"
	fi
}

func_createkeys()
{
	if [ ! -d "$dir_storage" ]; then
		if mkdir -p -m 755 "$dir_storage"; then :; else
			storage_status=$?
			func_log "cannot create key storage (status=$storage_status)"
			return "$storage_status"
		fi
	fi
	if rm -f "$rsa_key" "$dss_key" "$ecdsa_key" "$ed25519_key"; then :; else
		remove_status=$?
		func_log "cannot remove previous host keys (status=$remove_status)"
		return "$remove_status"
	fi
	func_keygen rsa "$rsa_key"
	key_status=$?
	[ "$key_status" -eq 0 ] || return "$key_status"
	func_keygen dss "$dss_key"
	key_status=$?
	[ "$key_status" -eq 0 ] || return "$key_status"
	func_keygen ecdsa "$ecdsa_key"
	key_status=$?
	[ "$key_status" -eq 0 ] || return "$key_status"
	func_keygen ed25519 "$ed25519_key"
	return $?
}

func_get_extra_args()
{
	if [ ! -f "$extra_args" ]; then
		printf '%s\n' "-p 22" > "$extra_args" || return $?
	fi
	cat "$extra_args"
}

func_start()
{
	# Do not launch another listener, or mutate key/config state, unless an
	# incomplete process inventory has been ruled out.
	dropbear_process_state
	process_status=$?
	case "$process_status" in
	0) return 0 ;;
	1) ;;
	*)
		func_log "cannot verify existing daemon state; refusing start"
		return 2
		;;
	esac

	if [ ! -d "$dir_storage" ]; then
		if mkdir -p -m 755 "$dir_storage"; then :; else
			storage_status=$?
			func_log "cannot create key storage (status=$storage_status)"
			return "$storage_status"
		fi
	fi
	args=$(func_get_extra_args)
	args_status=$?
	if [ "$args_status" -ne 0 ]; then
		func_log "cannot read daemon arguments (status=$args_status)"
		return "$args_status"
	fi
	for i in rsa_host_key dss_host_key ecdsa_host_key; do
		old_key="${old_pattern}_${i}"
		if [ -f "$old_key" ]; then
			if mv -n "$old_key" "$dir_storage/$i"; then :; else
				move_status=$?
				func_log "cannot migrate host key $old_key (status=$move_status)"
				return "$move_status"
			fi
		fi
	done
	if [ ! -f "$rsa_key" ] || [ ! -f "$dss_key" ]; then
		func_createkeys
		key_status=$?
		[ "$key_status" -eq 0 ] || return "$key_status"
	fi
	if [ ! -f "$ecdsa_key" ]; then
		func_keygen ecdsa "$ecdsa_key"
		key_status=$?
		[ "$key_status" -eq 0 ] || return "$key_status"
	fi
	if [ ! -f "$ed25519_key" ]; then
		func_keygen ed25519 "$ed25519_key"
		key_status=$?
		[ "$key_status" -eq 0 ] || return "$key_status"
	fi

	# Keep the persisted whitespace-separated extra_args convention without xargs
	# exit-status translation or pathname expansion.
	if [ -n "$1" ]; then
		args="$1 $args"
	fi
	set -f
	set -- $args
	if "$dropbear_bin" "$@"; then
		:
	else
		launch_status=$?
		func_log "daemon launch failed (status=$launch_status)"
		return "$launch_status"
	fi

	attempt=0
	while :; do
		dropbear_process_state
		process_status=$?
		case "$process_status" in
		0) return 0 ;;
		1) ;;
		*)
			func_log "cannot verify daemon startup postcondition"
			return 2
			;;
		esac
		if [ "$attempt" -ge 3 ]; then
			func_log "launcher returned success but daemon did not appear"
			return 1
		fi
		attempt=$((attempt + 1))
		sleep 1 || return 2
	done
}

func_stop()
{
	dropbear_process_state
	process_status=$?
	case "$process_status" in
	1) return 0 ;;
	0) ;;
	*)
		func_log "cannot verify daemon state; refusing stop"
		return 2
		;;
	esac

	if killall -q dropbear; then
		:
	else
		stop_status=$?
		dropbear_process_state
		process_status=$?
		case "$process_status" in
		1) return 0 ;;
		0)
			func_log "daemon stop failed (status=$stop_status)"
			return "$stop_status"
			;;
		*)
			func_log "cannot verify daemon after failed stop signal"
			return 2
			;;
		esac
	fi

	attempt=0
	while :; do
		dropbear_process_state
		process_status=$?
		case "$process_status" in
		1) return 0 ;;
		0) ;;
		*)
			func_log "cannot verify daemon stop postcondition"
			return 2
			;;
		esac
		if [ "$attempt" -ge 5 ]; then
			func_log "daemon is still running after the bounded stop wait"
			return 1
		fi
		attempt=$((attempt + 1))
		sleep 1 || return 2
	done
}

action_status=0
case "$1" in
start)
	func_start "$2"
	action_status=$?
	;;
stop)
	func_stop
	action_status=$?
	;;
newkeys)
	func_createkeys
	action_status=$?
	;;
*)
	echo "Usage: $0 {start|stop|newkeys}"
	action_status=1
	;;
esac
exit "$action_status"
