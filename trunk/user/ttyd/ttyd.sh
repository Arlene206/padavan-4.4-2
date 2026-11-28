#!/bin/sh
PROCESS_LIFECYCLE_LIB="${PROCESS_LIFECYCLE_LIB:-/usr/lib/process_lifecycle.sh}"
if [ ! -r "$PROCESS_LIFECYCLE_LIB" ]; then
	logger -st "ttyd" "process lifecycle helper is unavailable"
	exit 127
fi
. "$PROCESS_LIFECYCLE_LIB" || exit 127
ttyd_exec="ttyd"
func_log(){ logger -st "ttyd" "$1" >/dev/null 2>&1 || :; }
ttyd_normalize_port() {
	ttyd_port_value="$1"
	case "$ttyd_port_value" in ''|*[!0-9]*) return 1 ;; esac
	while [ "${ttyd_port_value#0}" != "$ttyd_port_value" ]; do
		ttyd_port_value=${ttyd_port_value#0}
	done
	[ -n "$ttyd_port_value" ] || return 1
	[ "${#ttyd_port_value}" -le 5 ] || return 1
	[ "$ttyd_port_value" -le 65535 ] 2>/dev/null || return 1
	printf '%s\n' "$ttyd_port_value"
}
func_start(){
	if pl_process_state "$ttyd_exec"; then
		func_log "Daemon is already running"
		return 0
	else
		process_status=$?
	fi
	if [ "$process_status" -ne 1 ]; then
		func_log "Daemon state is unknown; refusing duplicate launch"
		return 2
	fi
	if ttyd_nvram_port=$(nvram get_checked ttyd_port); then :; else
		launch_status=$?
		func_log "Unable to read ttyd port (status=$launch_status)"
		return "$launch_status"
	fi
	if [ -z "$ttyd_nvram_port" ]; then
		if nvram set ttyd_port=7681; then :; else
			launch_status=$?
			func_log "Unable to set default ttyd port (status=$launch_status)"
			return "$launch_status"
		fi
		if nvram commit; then :; else
			launch_status=$?
			func_log "Unable to commit default ttyd port (status=$launch_status)"
			return "$launch_status"
		fi
		ttyd_nvram_port=7681
	fi
	if ttyd_nvram_port=$(ttyd_normalize_port "$ttyd_nvram_port"); then
		:
	else
		func_log "Invalid ttyd port; expected a decimal value from 1 through 65535"
		return 2
	fi
	if start-stop-daemon -S -b -x "$ttyd_exec" -- -i br0 -p "$ttyd_nvram_port" -W -t disableReconnect=true -t disableLeaveAlert=true login; then
		if pl_wait_started "$ttyd_exec" 1 4 1; then
			func_log "Daemon is running"
			return 0
		else
			launch_status=$?
			func_log "Daemon did not reach running state (status=$launch_status)"
			return "$launch_status"
		fi
	else
		launch_status=$?
		func_log "Daemon launch failed (status=$launch_status)"
		return "$launch_status"
	fi
}
func_stop(){
	if pl_stop_named "$ttyd_exec" 3 2; then
		func_log "Daemon stopped"
		return 0
	else
		stop_status=$?
		func_log "Daemon stop failed (status=$stop_status)"
		return "$stop_status"
	fi
}
action_status=0
case "$1" in
start) func_start; action_status=$? ;;
stop) func_stop; action_status=$? ;;
restart)
	if func_stop; then func_start; action_status=$?; else action_status=$?; fi
	;;
*) echo "Usage: $0 { start | stop | restart }"; action_status=2 ;;
esac
exit "$action_status"
