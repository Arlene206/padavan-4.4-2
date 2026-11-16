#!/bin/sh


mentohust_exec="bin_mentohust"
conf_file="${MENTOHUST_CONF_FILE:-/var/mentohust.conf}"
bin_conf_file="${MENTOHUST_BIN_CONF_FILE:-/etc/storage/mentohust.conf}"

mentohust_vars="username password nic ip mask gw dns pinghost timeout interval \
	restart_wait maxfail startmode dhcp daemon ver datafile dhcpscript service"

mentohust_conf_vars="Username Password Nic IP Mask Gateway DNS PingHost Timeout EchoInterval \
	RestartWait MaxFail StartMode DhcpMode DaemonMode Version DataFile DhcpScript Service"

func_log(){
	logger -st "mentohust" "$1"
}

func_gen_conf(){
	if [ -z "$(nvram get mentohust_nic)" ]; then
		nvram set mentohust_nic="$(nvram get wan_ifname)"
		nvram commit
	fi

	echo "[MentoHUST]" > $conf_file
	for c in $mentohust_conf_vars; do
		echo "${c}=" >> $conf_file
	done

	local line=2
	for c in $mentohust_vars; do
		i="$(nvram get mentohust_${c})"
		if [ -n "${i}" ]; then
			i="$(echo $i |sed 's/\//\\\//g')"
			sed -i "${line}s/\$/${i}/" $conf_file
		fi
		line=$((line+1))
	done

	ln -sf $conf_file $bin_conf_file
}

func_start(){
	if [ -f $bin_conf_file ] && [ ! -L $bin_conf_file ]; then
		func_log "Using existed conf: $bin_conf_file"
	else
		func_gen_conf
	fi

	echo -n "Starting mentohust:..."
	if start-stop-daemon -S -b -x "$mentohust_exec"; then
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
	echo -n "Stopping mentohust:..."
	if killall -q -9 "$mentohust_exec"; then
		echo "[  OK  ]"
		func_log "Stopped"
		return 0
	else
		stop_status=$?
		echo "[FAILED]"
		func_log "Daemon stop failed (status=$stop_status)"
		return "$stop_status"
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
	func_stop
	stop_status=$?
	if [ "$stop_status" -eq 0 ]; then
		func_start
		action_status=$?
	else
		action_status=$stop_status
	fi
	;;
*)
	echo "Usage: $0 { start | stop | restart }"
	action_status=2
	;;
esac
exit "$action_status"

