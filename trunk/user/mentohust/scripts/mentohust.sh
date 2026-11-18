#!/bin/sh

# PID 1 uses umask 0000; keep generated credentials private.
umask 077

mentohust_exec="bin_mentohust"
PROCESS_LIFECYCLE_LIB="${PROCESS_LIFECYCLE_LIB:-/usr/lib/process_lifecycle.sh}"
if [ ! -r "$PROCESS_LIFECYCLE_LIB" ]; then
	logger -st "mentohust" "process lifecycle helper is unavailable"
	exit 127
fi
. "$PROCESS_LIFECYCLE_LIB" || exit 127
conf_file="${MENTOHUST_CONF_FILE:-/var/mentohust.conf}"
bin_conf_file="${MENTOHUST_BIN_CONF_FILE:-/etc/storage/mentohust.conf}"

func_log(){
	logger -st "mentohust" "$1"
}

func_restrict_private_config(){
	local path owner effective_uid
	path=$1
	if [ -L "$path" ] || [ ! -f "$path" ]; then
		func_log "Refusing non-regular MentoHUST config: $path"
		return 1
	fi
	if ! owner=$(stat -c '%u' "$path"); then
		func_log "Cannot verify MentoHUST config owner: $path"
		return 1
	fi
	if ! effective_uid=$(id -u); then
		func_log "Cannot verify MentoHUST execution identity"
		return 1
	fi
	if [ "$owner" != "$effective_uid" ]; then
		func_log "Refusing MentoHUST config not owned by the executing user: $path"
		return 1
	fi
	if ! chmod 600 "$path"; then
		func_log "Cannot restrict MentoHUST config permissions: $path"
		return 1
	fi
	return 0
}

func_gen_conf(){
	local mapping nvram_name config_name value nic_value wan_ifname
	local conf_tmp link_target newline carriage_return

	# This path stores plaintext credentials; never follow a pre-existing symlink.
	if [ -L "$conf_file" ] || [ -d "$conf_file" ]; then
		func_log "Refusing unsafe MentoHUST config destination: $conf_file"
		return 1
	fi
	if [ -d "$bin_conf_file" ]; then
		func_log "Refusing MentoHUST storage config directory: $bin_conf_file"
		return 1
	fi

	# Older releases linked the storage path to this generated config. Restrict
	# only that exact target; never chmod an arbitrary symlink target.
	if [ -L "$bin_conf_file" ]; then
		if ! link_target=$(readlink "$bin_conf_file"); then
			func_log "Cannot inspect existing MentoHUST config link"
			return 1
		fi
		if [ "$link_target" = "$conf_file" ] && [ -e "$conf_file" ]; then
			if ! func_restrict_private_config "$conf_file"; then
				return 1
			fi
		fi
	fi

	if ! nic_value=$(nvram get mentohust_nic); then
		func_log "Cannot read MentoHUST network interface setting"
		return 1
	fi
	if [ -z "$nic_value" ]; then
		if ! wan_ifname=$(nvram get wan_ifname); then
			func_log "Cannot read WAN interface for MentoHUST config"
			return 1
		fi
		if ! nvram set "mentohust_nic=$wan_ifname"; then
			func_log "Cannot save default MentoHUST network interface"
			return 1
		fi
		if ! nvram commit; then
			func_log "Cannot commit default MentoHUST network interface"
			return 1
		fi
	fi

	# Keep the temporary file in the destination directory so rename is atomic.
	conf_tmp=$(mktemp "$conf_file.XXXXXX") || {
		func_log "Cannot create temporary MentoHUST config"
		return 1
	}
	if ! chmod 600 "$conf_tmp"; then
		func_log "Cannot restrict temporary MentoHUST config permissions"
		rm -f "$conf_tmp"
		return 1
	fi
	if ! printf '[MentoHUST]\n' > "$conf_tmp"; then
		func_log "Cannot write MentoHUST config header"
		rm -f "$conf_tmp"
		return 1
	fi

	# Keep a trailing sentinel so command substitution retains the newline.
	newline=$(printf '\nx')
	newline=${newline%x}
	carriage_return=$(printf '\r')
	for mapping in \
		'username:Username' \
		'password:Password' \
		'nic:Nic' \
		'ip:IP' \
		'mask:Mask' \
		'gw:Gateway' \
		'dns:DNS' \
		'pinghost:PingHost' \
		'timeout:Timeout' \
		'interval:EchoInterval' \
		'restart_wait:RestartWait' \
		'maxfail:MaxFail' \
		'startmode:StartMode' \
		'dhcp:DhcpMode' \
		'daemon:DaemonMode' \
		'ver:Version' \
		'datafile:DataFile' \
		'dhcpscript:DhcpScript' \
		'service:Service'
	do
		nvram_name=${mapping%%:*}
		config_name=${mapping#*:}
		if ! value=$(nvram get "mentohust_${nvram_name}"); then
			func_log "Cannot read MentoHUST setting: $nvram_name"
			rm -f "$conf_tmp"
			return 1
		fi
		case "$value" in
			*"$newline"*|*"$carriage_return"*)
				func_log "Rejecting line break in MentoHUST setting: $nvram_name"
				rm -f "$conf_tmp"
				return 1
				;;
		esac
		if ! printf '%s=%s\n' "$config_name" "$value" >> "$conf_tmp"; then
			func_log "Cannot write MentoHUST setting: $nvram_name"
			rm -f "$conf_tmp"
			return 1
		fi
	done

	if ! chmod 600 "$conf_tmp"; then
		func_log "Cannot finalize private MentoHUST config permissions"
		rm -f "$conf_tmp"
		return 1
	fi
	if [ -L "$conf_file" ] || [ -d "$conf_file" ]; then
		func_log "MentoHUST config destination changed during generation"
		rm -f "$conf_tmp"
		return 1
	fi
	if ! mv -f "$conf_tmp" "$conf_file"; then
		func_log "Cannot atomically publish MentoHUST config"
		rm -f "$conf_tmp"
		return 1
	fi
	if ! func_restrict_private_config "$conf_file"; then
		return 1
	fi
	if ! ln -sf "$conf_file" "$bin_conf_file"; then
		func_log "Cannot link private MentoHUST config into storage"
		return 1
	fi
	return 0
}
func_start(){
	if pl_process_state "$mentohust_exec"; then
		func_log "Daemon is already running"
		return 0
	else
		process_status=$?
	fi
	if [ "$process_status" -ne 1 ]; then
		func_log "Daemon state is unknown; refusing duplicate launch"
		return 2
	fi
	if [ -d "$bin_conf_file" ]; then
		func_log "Refusing MentoHUST config path directory: $bin_conf_file"
		return 1
	fi
	if [ -f "$bin_conf_file" ] && [ ! -L "$bin_conf_file" ]; then
		if ! func_restrict_private_config "$bin_conf_file"; then
			return 1
		fi
		func_log "Using existing private config: $bin_conf_file"
	else
		if ! func_gen_conf; then
			func_log "MentoHUST config generation failed; refusing daemon start"
			return 1
		fi
	fi

	echo -n "Starting mentohust:..."
	if start-stop-daemon -S -b -x "$mentohust_exec"; then
		if pl_wait_started "$mentohust_exec" 1 4 1; then
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
	echo -n "Stopping mentohust:..."
	if pl_stop_named "$mentohust_exec" 3 2; then
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

