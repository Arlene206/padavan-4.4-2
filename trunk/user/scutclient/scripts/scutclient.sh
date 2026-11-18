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

get_arg_debug(){
	[ "$(nvram get scutclient_debug)" = "1" ] && echo "-D"
}

get_arg_skip_udp_hb(){
	[ "$(nvram get scutclient_skip_udp_hb)" = "1" ] && echo "-b"
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
	auth_hook=$(nvram get scutclient_auth_exec)
	fail_hook=$(nvram get scutclient_fail_exec)
	sysdns=$(nvram get wan_dns1_x | cut -d ' ' -f 1)
	echo -n "Starting scutclient:..."
	if start-stop-daemon -S -b -x "$scutclient_exec" -- -u "$(nvram get scutclient_username)" -p "$(nvram get scutclient_password)" \
	-i "$(nvram get wan_ifname)" \
	-n "${sysdns:-"114.114.114.114"}" \
	-H "$(nvram get scutclient_hostname)" \
	-s "$(nvram get scutclient_server_auth_ip)" \
	-c "$(nvram get scutclient_version)" \
	-h "$(nvram get scutclient_hash)" \
	"$(get_arg_debug)" "$(get_arg_skip_udp_hb)" \
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
			return "$readiness_status"
		fi
		echo "[  OK  ]"
		func_log "Daemon is started and running"
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
	if pl_stop_named "$scutclient_exec" 3 2; then
		:
	else
		stop_status=$?
		echo "[FAILED]"
		func_log "Daemon stop failed (status=$stop_status)"
		return "$stop_status"
	fi
	"$scutclient_exec" -o -i "$(nvram get wan_ifname)" >/dev/null 2>&1
	echo "[  OK  ]"
	func_log "Stopped"
	return 0
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
