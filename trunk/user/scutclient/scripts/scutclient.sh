#!/bin/sh
scutclient_exec="bin_scutclient"
PROCESS_LIFECYCLE_LIB="${PROCESS_LIFECYCLE_LIB:-/usr/lib/process_lifecycle.sh}"
if [ ! -r "$PROCESS_LIFECYCLE_LIB" ]; then
	logger -st "scutclient" "process lifecycle helper is unavailable"
	exit 127
fi
. "$PROCESS_LIFECYCLE_LIB" || exit 127
LOG="$2"


func_log(){
	[ "$LOG" != "nolog" ] && logger -st "scutclient" "$1"
}

# Read NVRAM through the status-bearing CLI so backend failures cannot
# silently turn credentials, hooks or the effective WAN interface into blanks.
func_read_nvram_checked(){
	scut_nvram_value=$(nvram get_checked "$1")
	scut_nvram_status=$?
	if [ "$scut_nvram_status" -ne 0 ]; then
		func_log "Cannot read NVRAM setting $1 (status=$scut_nvram_status)"
		return "$scut_nvram_status"
	fi
	return 0
}

func_load_nvram_config(){
	func_read_nvram_checked scutclient_debug || return $?
	scutclient_debug_value=$scut_nvram_value
	func_read_nvram_checked scutclient_skip_udp_hb || return $?
	scutclient_skip_udp_hb_value=$scut_nvram_value
	func_read_nvram_checked scutclient_auth_exec || return $?
	auth_hook=$scut_nvram_value
	func_read_nvram_checked scutclient_fail_exec || return $?
	fail_hook=$scut_nvram_value
	func_read_nvram_checked wan_dns1_x || return $?
	sysdns=${scut_nvram_value%% *}
	func_read_nvram_checked scutclient_username || return $?
	scutclient_username_value=$scut_nvram_value
	func_read_nvram_checked scutclient_password || return $?
	scutclient_password_value=$scut_nvram_value
	func_read_nvram_checked wan_ifname || return $?
	scutclient_wan_ifname=$scut_nvram_value
	func_read_nvram_checked scutclient_hostname || return $?
	scutclient_hostname_value=$scut_nvram_value
	func_read_nvram_checked scutclient_server_auth_ip || return $?
	scutclient_server_auth_ip_value=$scut_nvram_value
	func_read_nvram_checked scutclient_version || return $?
	scutclient_version_value=$scut_nvram_value
	func_read_nvram_checked scutclient_hash || return $?
	scutclient_hash_value=$scut_nvram_value
	return 0
}

func_credentials_cleanup(){
	[ -n "$scutclient_credentials_file" ] && rm -f "$scutclient_credentials_file" 2>/dev/null
	[ -n "$scutclient_credentials_dir" ] && rmdir "$scutclient_credentials_dir" 2>/dev/null
	scutclient_credentials_file=
	scutclient_credentials_dir=
}

func_credentials_create(){
	scutclient_credentials_dir=
	scutclient_credentials_file=
	# Empty values are valid legacy NVRAM results; keep the prior CLI behavior
	# while transporting both fields through the private one-shot file.
	scutclient_lf='
'
	scutclient_cr=$(printf '\r')
	case "$scutclient_username_value$scutclient_password_value" in
		*"$scutclient_lf"*|*"$scutclient_cr"*)
			func_log "SCUTClient credentials contain a line break"
			return 1
			;;
	esac

	scutclient_old_umask=$(umask)
	umask 077 || return 1
	trap 'func_credentials_cleanup' 0
	trap 'func_credentials_cleanup; exit 1' HUP INT TERM
	scutclient_credentials_dir=$(mktemp -d "${TMPDIR:-/tmp}/scutclient-auth.XXXXXX") || {
		umask "$scutclient_old_umask"
		func_log "Cannot create private SCUTClient credential directory"
		return 1
	}
	if ! chmod 700 "$scutclient_credentials_dir"; then
		umask "$scutclient_old_umask"
		func_log "Cannot secure private SCUTClient credential directory"
		return 1
	fi
	scutclient_credentials_file=$(mktemp "$scutclient_credentials_dir/credentials.XXXXXX") || {
		umask "$scutclient_old_umask"
		func_log "Cannot create private SCUTClient credential file"
		return 1
	}
	if ! chmod 600 "$scutclient_credentials_file" ||
	   ! printf 'username=%s\npassword=%s\n' "$scutclient_username_value" "$scutclient_password_value" > "$scutclient_credentials_file"; then
		umask "$scutclient_old_umask"
		func_log "Cannot publish private SCUTClient credentials"
		return 1
	fi
	umask "$scutclient_old_umask"
	return 0
}

func_wait_credentials_consumed(){
	scutclient_credential_wait=0
	while [ -e "$scutclient_credentials_file" ] || [ -L "$scutclient_credentials_file" ]; do
		[ "$scutclient_credential_wait" -lt 4 ] || return 1
		scutclient_credential_wait=$((scutclient_credential_wait + 1))
		sleep 1
	done
	return 0
}
func_start(){
	if pl_process_state "$scutclient_exec"; then
		func_log "Daemon is already running"
		return 0
	else
		process_status=$?
	fi
	if [ "$process_status" -ne 1 ]; then
		func_log "Daemon state is unknown; refusing duplicate launch"
		return 2
	fi
#	[ "$(mtk_esw 11)" = "WAN ports link state: 0" ] && func_log "WAN has no link!" && exit 1
	if func_load_nvram_config; then
		:
	else
		config_status=$?
		return "$config_status"
	fi
	scutclient_debug_arg=
	scutclient_skip_udp_hb_arg=
	[ "$scutclient_debug_value" = "1" ] && scutclient_debug_arg="-D"
	[ "$scutclient_skip_udp_hb_value" = "1" ] && scutclient_skip_udp_hb_arg="-b"
	echo -n "Starting scutclient:..."
	if ! func_credentials_create; then
		echo "[FAILED]"
		return 1
	fi
	if start-stop-daemon -S -b -x "$scutclient_exec" -- --credentials-file "$scutclient_credentials_file" \
	-i "$scutclient_wan_ifname" \
	-n "${sysdns:-"114.114.114.114"}" \
	-H "$scutclient_hostname_value" \
	-s "$scutclient_server_auth_ip_value" \
	-c "$scutclient_version_value" \
	-h "$scutclient_hash_value" \
	"$scutclient_debug_arg" "$scutclient_skip_udp_hb_arg" \
	-E "${auth_hook:-"echo 0 > /tmp/scutclient_status"}" \
	-Q "${fail_hook:-"echo 1 > /tmp/scutclient_status"}"; then
		if pl_wait_started "$scutclient_exec" 1 4 1; then
			:
		else
			readiness_status=$?
			echo "[FAILED]"
			if [ "$readiness_status" -eq 1 ]; then
				func_log "Daemon startup readiness timed out (process remained absent)"
			else
				func_log "Daemon startup readiness could not be verified (status=$readiness_status)"
			fi
			# A child can appear just after the final bounded probe. Re-inventory
			# before returning failure; pl_stop_named never signals UNKNOWN state.
			if pl_stop_named "$scutclient_exec" 3 2; then
				func_log "Daemon startup rollback confirmed process absent"
			else
				rollback_status=$?
				func_log "Daemon startup rollback incomplete (primary_status=$readiness_status, rollback_status=$rollback_status)"
			fi
			return "$readiness_status"
		fi
		if func_wait_credentials_consumed; then
			:
		else
			echo "[FAILED]"
			func_log "Daemon did not consume its private credential file"
			if pl_stop_named "$scutclient_exec" 3 2; then
				func_log "Credential-consumption rollback confirmed process absent"
			else
				rollback_status=$?
				func_log "Credential-consumption rollback incomplete (status=$rollback_status)"
			fi
			return 1
		fi
		func_credentials_cleanup
		trap - 0 HUP INT TERM
		echo "[  OK  ]"
		func_log "Daemon is started and consumed its private credentials"
		return 0
	else
		launch_status=$?
		echo "[FAILED]"
		func_log "Daemon launch failed (status=$launch_status)"
		return "$launch_status"
	fi
}

func_stop(){
	echo -n "Stopping scutclient:..."
	# Resolve the logout interface before stopping the daemon. A transient
	# NVRAM read error must not leave a stopped client with an unlogged session.
	if func_read_nvram_checked wan_ifname; then
		scutclient_wan_ifname=$scut_nvram_value
	else
		config_status=$?
		echo "[FAILED]"
		func_log "Cannot resolve WAN interface for logout; leaving daemon running (status=$config_status)"
		return "$config_status"
	fi
	if pl_stop_named "$scutclient_exec" 3 2; then
		:
	else
		stop_status=$?
		echo "[FAILED]"
		func_log "Daemon stop failed (status=$stop_status)"
		return "$stop_status"
	fi
	if "$scutclient_exec" -o -i "$scutclient_wan_ifname" >/dev/null 2>&1; then
		echo "[  OK  ]"
		func_log "Stopped and logout request completed"
		return 0
	else
		logout_status=$?
		echo "[FAILED]"
		func_log "Daemon stopped but server logout request failed (status=$logout_status)"
		return "$logout_status"
	fi
}

action_status=0
case "$1" in
start)
	func_start
	action_status=$?
	;;
stop)
	func_stop
	action_status=$?
	;;
restart)
	if func_stop; then
		func_start
		action_status=$?
	else
		action_status=$?
	fi
	;;
*)
	echo "Usage: $0 { start | stop | restart }"
	action_status=2
	;;
esac
exit "$action_status"
