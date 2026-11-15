#!/bin/sh
pidfile="${SCUTCLIENT_PIDFILE:-/var/scutclient.sh.pid}"
scutclient_exec="bin_scutclient"
LOG="$2"

[ -f $pidfile ] && kill -9 "$(cat $pidfile)" || echo "$$" > $pidfile

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
		echo "[  OK  ]"
		func_log "Daemon is started"
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
	killall -q -9 $scutclient_exec
	$scutclient_exec -o -i "$(nvram get wan_ifname)" >/dev/null 2>&1
	echo "[  OK  ]"
	func_log "Stopped"
}

action_status=0
case "$1" in
start)
	func_start
	action_status=$?
	;;
stop)
	func_stop
	action_status=0
	;;
restart)
	func_stop
	func_start
	action_status=$?
	;;
*)
	echo "Usage: $0 { start | stop | restart }"
	;;
esac
rm -f "$pidfile"
exit "$action_status"
