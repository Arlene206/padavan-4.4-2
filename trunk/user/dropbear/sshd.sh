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


# Match a PID only when its procfs identity is complete and its executable is
# the installed Dropbear binary. BusyBox pidof/killall use the short comm name,
# which any process can spoof with PR_SET_NAME.
dropbear_process_identity_matches()
{
	dropbear_identity_pid="$1"
	dropbear_identity_required_start="$2"
	dropbear_identity_root="${DROPBEAR_PROC_ROOT:-/proc}"
	case "$dropbear_identity_pid" in ''|0*|*[!0-9]*) return 1 ;; esac
	dropbear_identity_dir="$dropbear_identity_root/$dropbear_identity_pid"
	[ -d "$dropbear_identity_dir" ] || return 1
	if IFS= read -r dropbear_identity_stat_line < "$dropbear_identity_dir/stat"; then
		:
	else
		[ -d "$dropbear_identity_dir" ] && return 2
		return 1
	fi
	case "$dropbear_identity_stat_line" in
		"$dropbear_identity_pid ("*") "*) ;;
		*) return 2 ;;
	esac
	dropbear_identity_fields=${dropbear_identity_stat_line##*) }
	set -- $dropbear_identity_fields
	[ "$#" -ge 20 ] || return 2
	dropbear_identity_state="$1"
	dropbear_identity_start_time="${20}"
	case "$dropbear_identity_state" in
		R|S|D|Z|T|t|X|x|K|W|P|I) ;;
		*) return 2 ;;
	esac
	case "$dropbear_identity_start_time" in ''|*[!0-9]*) return 2 ;; esac
	case "$dropbear_identity_state" in Z|X|x) return 1 ;; esac
	dropbear_identity_comm=${dropbear_identity_stat_line#*(}
	dropbear_identity_comm=${dropbear_identity_comm%)*}
	[ "$dropbear_identity_comm" = dropbear ] || return 1
	if [ -n "$dropbear_identity_required_start" ] &&
	   [ "$dropbear_identity_start_time" != "$dropbear_identity_required_start" ]; then
		return 1
	fi
	[ -n "$dropbear_expected_exe" ] || return 2
	dropbear_identity_actual_exe=$(stat -L -c '%d:%i' "$dropbear_identity_dir/exe" 2>/dev/null) || {
		[ -d "$dropbear_identity_dir" ] && return 2
		return 1
	}
	[ "$dropbear_identity_actual_exe" = "$dropbear_expected_exe" ] || return 1
	return 0
}

# Return 0=verified RUNNING, 1=verified ABSENT, 2=UNKNOWN/incomplete procfs.
# Publish only exact executable identities; a comm-only match is ignored.
dropbear_process_state()
{
	dropbear_process_pids=
	dropbear_expected_exe=
	dropbear_process_root="${DROPBEAR_PROC_ROOT:-/proc}"
	if [ ! -d "$dropbear_process_root" ] ||
	   [ ! -r "$dropbear_process_root" ] ||
	   [ ! -x "$dropbear_process_root" ] ||
	   ! command -v stat >/dev/null 2>&1; then
		return 2
	fi
	dropbear_expected_exe=$(stat -L -c '%d:%i' "$dropbear_bin" 2>/dev/null) || return 2
	case "$dropbear_expected_exe" in
		''|*[!0-9:]*|:*|*:|*:*:*) return 2 ;;
	esac
	for dropbear_proc_dir in "$dropbear_process_root"/[0-9]*; do
		[ -d "$dropbear_proc_dir" ] || continue
		dropbear_proc_pid=${dropbear_proc_dir##*/}
		case "$dropbear_proc_pid" in ''|0*|*[!0-9]*) return 2 ;; esac
		if dropbear_process_identity_matches "$dropbear_proc_pid" ""; then
			dropbear_process_pids="${dropbear_process_pids}${dropbear_process_pids:+ }$dropbear_proc_pid:$dropbear_identity_start_time"
		else
			dropbear_identity_status=$?
			case "$dropbear_identity_status" in
				1) ;;
				*) return 2 ;;
			esac
		fi
	done
	[ -n "$dropbear_process_pids" ] && return 0
	return 1
}

dropbear_wait_stopped()
{
	dropbear_wait_limit="$1"
	dropbear_wait_attempt=0
	while :; do
		if dropbear_process_state; then dropbear_wait_state=0; else dropbear_wait_state=$?; fi
		case "$dropbear_wait_state" in
			1) return 0 ;;
			0) ;;
			*) return 2 ;;
		esac
		[ "$dropbear_wait_attempt" -lt "$dropbear_wait_limit" ] || return 1
		sleep 1 || return 2
		dropbear_wait_attempt=$((dropbear_wait_attempt + 1))
	done
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

# Complete only missing host-key types. Startup must never rotate the other
# host identities just because one algorithm's key is absent.
func_ensure_key()
{
	ensure_key_type="$1"
	ensure_key_path="$2"
	if [ -L "$ensure_key_path" ] ||
	   { [ -e "$ensure_key_path" ] && [ ! -f "$ensure_key_path" ]; }; then
		func_log "Refusing unsafe host-key path for $ensure_key_type"
		return 1
	fi
	if [ -f "$ensure_key_path" ]; then
		if chmod 600 "$ensure_key_path"; then
			return 0
		else
			ensure_key_chmod_status=$?
			func_log "Cannot restrict existing $ensure_key_type host key (status=$ensure_key_chmod_status)"
			return "$ensure_key_chmod_status"
		fi
	fi
	func_keygen "$ensure_key_type" "$ensure_key_path"
}

func_ensurekeys()
{
	func_ensure_key rsa "$rsa_key" || return $?
	func_ensure_key dss "$dss_key" || return $?
	func_ensure_key ecdsa "$ecdsa_key" || return $?
	func_ensure_key ed25519 "$ed25519_key"
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
	func_ensurekeys
	key_status=$?
	[ "$key_status" -eq 0 ] || return "$key_status"

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
	if dropbear_process_state; then dropbear_stop_state=0; else dropbear_stop_state=$?; fi
	case "$dropbear_stop_state" in
		1) return 0 ;;
		0) ;;
		*) func_log "cannot verify daemon state; refusing stop"; return 2 ;;
	esac
	dropbear_stop_snapshot="$dropbear_process_pids"
	for dropbear_stop_entry in $dropbear_stop_snapshot; do
		dropbear_stop_pid=${dropbear_stop_entry%%:*}
		dropbear_stop_start=${dropbear_stop_entry#*:}
		if dropbear_process_identity_matches "$dropbear_stop_pid" "$dropbear_stop_start"; then
			if kill -TERM "$dropbear_stop_pid" 2>/dev/null; then :
			else
				dropbear_signal_status=$?
				if dropbear_process_identity_matches "$dropbear_stop_pid" "$dropbear_stop_start"; then
					func_log "TERM failed for verified daemon PID $dropbear_stop_pid (status=$dropbear_signal_status)"
					return "$dropbear_signal_status"
				else
					dropbear_identity_status=$?
					[ "$dropbear_identity_status" -eq 1 ] || return 2
				fi
			fi
		else
			dropbear_identity_status=$?
			[ "$dropbear_identity_status" -eq 1 ] || return 2
		fi
	done
	if dropbear_wait_stopped 5; then return 0; else dropbear_wait_status=$?; fi
	[ "$dropbear_wait_status" -eq 1 ] || return 2
	if dropbear_process_state; then dropbear_stop_snapshot="$dropbear_process_pids"
	else
		dropbear_stop_state=$?
		[ "$dropbear_stop_state" -eq 1 ] && return 0
		return 2
	fi
	for dropbear_stop_entry in $dropbear_stop_snapshot; do
		dropbear_stop_pid=${dropbear_stop_entry%%:*}
		dropbear_stop_start=${dropbear_stop_entry#*:}
		if dropbear_process_identity_matches "$dropbear_stop_pid" "$dropbear_stop_start"; then
			if kill -KILL "$dropbear_stop_pid" 2>/dev/null; then :
			else
				dropbear_signal_status=$?
				if dropbear_process_identity_matches "$dropbear_stop_pid" "$dropbear_stop_start"; then
					func_log "KILL failed for verified daemon PID $dropbear_stop_pid (status=$dropbear_signal_status)"
					return "$dropbear_signal_status"
				else
					dropbear_identity_status=$?
					[ "$dropbear_identity_status" -eq 1 ] || return 2
				fi
			fi
		else
			dropbear_identity_status=$?
			[ "$dropbear_identity_status" -eq 1 ] || return 2
		fi
	done
	if dropbear_wait_stopped 2; then return 0; else dropbear_wait_status=$?; fi
	case "$dropbear_wait_status" in
		1) func_log "verified daemon remained after bounded TERM/KILL waits"; return 1 ;;
		*) func_log "daemon state became unknown during stop"; return 2 ;;
	esac
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
