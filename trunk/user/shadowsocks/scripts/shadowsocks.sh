#!/bin/sh
#
# Copyright (C) 2017 openwrt-ssr
# Copyright (C) 2017 yushi studio <ywb94@qq.com>
# Copyright (C) 2018 lean <coolsnowwolf@gmail.com>
# Copyright (C) 2019 chongshengB <bkye@vip.qq.com>
# Copyright (C) 2022 TurBoTse <860018505@qq.com>
# Copyright (C) 2023 simonchen
#
# This is free software, licensed under the GNU General Public License v3.
# See /LICENSE for more information.
#

NAME=shadowsocksr
pppoemwan=`nvram get pppoemwan_enable`
http_username=`nvram get http_username`
CONFIG_FILE=/tmp/${NAME}.json
CONFIG_UDP_FILE=/tmp/${NAME}_u.json
CONFIG_SOCK5_FILE=/tmp/${NAME}_s.json
v2_json_file="/tmp/v2-redir.json"
trojan_json_file="/tmp/tj-redir.json"
server_count=0
redir_tcp=0
v2ray_enable=0
redir_udp=0
tunnel_enable=0
local_enable=0
pdnsd_enable_flag=0
chinadnsng_enable_flag=0
wan_bp_ips="/tmp/whiteip.txt"
wan_fw_ips="/tmp/blackip.txt"
lan_fp_ips="/tmp/lan_ip.txt"
lan_gm_ips="/tmp/lan_gmip.txt"
run_mode=`nvram get ss_run_mode`
lan_con=`nvram get lan_con`
GLOBAL_SERVER=`nvram get global_server`
socks=""
SS_RULES=/usr/bin/ss-rules
[ -x /etc/storage/ss-rules ] && SS_RULES=/etc/storage/ss-rules

log() {
	logger -t "$NAME" "$@"
	echo "$(date "+%Y-%m-%d %H:%M:%S") $@" >> "/tmp/ssrplus.log"
}

. /usr/bin/shadowsocks_process_lifecycle.sh || exit 1
. /usr/bin/dnsmasq_config_lifecycle.sh || exit 1

find_bin() {
	case "$1" in
	ss) ret="/usr/bin/ss-redir" ;;
	ss-local) ret="/usr/bin/ss-local" ;;
	ssr) ret="/usr/bin/ssr-redir" ;;
	ssr-local) ret="/usr/bin/ssr-local" ;;
	ssr-server) ret="/usr/bin/ssr-server" ;;
	v2ray|xray)
		if [ -f "/usr/bin/$1" ]; then
			ret="/usr/bin/$1"
		else
			bin=$(echo -e "v2ray\nxray" | grep -v $1)
			ret="/usr/bin/$bin"
		fi
		;;
	trojan) ret="/usr/bin/trojan" ;;
	socks5) ret="/usr/bin/ipt2socks" ;;
	esac
	echo $ret
}

run_bin() {
	[ "$#" -gt 0 ] || return 2
	local process_name=""
	local process_count
	local expected_count
	local wait_status

	if [ "$1" = "--wait-for" ]; then
		[ "$#" -ge 3 ] || return 2
		process_name="$2"
		shift 2
	else
		process_name="${1##*/}"
	fi
	[ -n "$process_name" ] && [ "$#" -gt 0 ] || return 2
	if process_count=$(ss_process_pid_count "$process_name"); then
		:
	else
		wait_status=$?
		log "Cannot inspect $process_name before launch (status=$wait_status)"
		return "$wait_status"
	fi
	expected_count=$((process_count + 1))

	(
		if [ "$(nvram get ss_cgroups)" = "1" ]; then
			echo 0 > /sys/fs/cgroup/cpu/$NAME/tasks
			echo 0 > /sys/fs/cgroup/memory/$NAME/tasks
		fi
		exec "$@" > /dev/null 2>&1
	) &

	if ss_wait_named_process_count "$process_name" "$expected_count" 5; then
		return 0
	else
		wait_status=$?
	fi
	log "Proxy process did not become ready: $process_name (expected=$expected_count, status=$wait_status)"
	return "$wait_status"
}

# Keep filesystem operations behind small helpers so their failures can
# be tested without writing to the host or router cgroup filesystem.
ss_cgroup_mkdir() {
	mkdir -p "$1"
}

ss_cgroup_write_value() {
	[ "$#" -eq 2 ] || return 2
	printf '%s\n' "$1" > "$2"
}

ss_cgroup_read_value() {
	cat "$1"
}

cgroups_init() {
	local status
	local cpu_dir="/sys/fs/cgroup/cpu/$NAME"
	local memory_dir="/sys/fs/cgroup/memory/$NAME"
	local cpu_limit
	local mem_limit
	local limit_bytes

	if [ "$(nvram get ss_cgroups)" != "1" ]; then
		return 0
	fi
	if cpu_limit=$(nvram get ss_cgoups_cpu_s); then
		:
	else
		status=$?
		return "$status"
	fi
	if mem_limit=$(nvram get ss_cgoups_mem_s); then
		:
	else
		status=$?
		return "$status"
	fi

	log "启用进程资源限制, CPU: $cpu_limit, 内存: $mem_limit"
	if ss_cgroup_mkdir "$cpu_dir"; then
		:
	else
		status=$?
		return "$status"
	fi
	if ss_cgroup_mkdir "$memory_dir"; then
		:
	else
		status=$?
		return "$status"
	fi
	if ss_cgroup_write_value "$cpu_limit" "$cpu_dir/cpu.shares"; then
		:
	else
		status=$?
		return "$status"
	fi
	if ss_cgroup_write_value "$mem_limit" "$memory_dir/memory.limit_in_bytes"; then
		:
	else
		status=$?
		return "$status"
	fi
	if limit_bytes=$(ss_cgroup_read_value "$memory_dir/memory.limit_in_bytes"); then
		:
	else
		status=$?
		return "$status"
	fi
	[ -n "$limit_bytes" ] || return 1
	if export GOMEMLIMIT="$limit_bytes"; then
		return 0
	else
		status=$?
		return "$status"
	fi
}

cgroups_cleanup() {
	cat /sys/fs/cgroup/cpu/$NAME/tasks > /sys/fs/cgroup/cpu/tasks
	cat /sys/fs/cgroup/memory/$NAME/tasks > /sys/fs/cgroup/memory/tasks
	rmdir /sys/fs/cgroup/cpu/$NAME
	rmdir /sys/fs/cgroup/memory/$NAME
}

gen_config_file() {
	#fastopen="false"
	case "$2" in
	0) config_file=$CONFIG_FILE && local stype=$(nvram get d_type) ;;
	1) config_file=$CONFIG_UDP_FILE && local stype=$(nvram get ud_type) ;;
	*) config_file=$CONFIG_SOCK5_FILE && local stype=$(nvram get s5_type) ;;
	esac
	local type=$stype
	case "$type" in
	ss)
		lua /etc_ro/ss/genssconfig.lua $1 $3 >$config_file
		sed -i 's/\\//g' $config_file
		;;
	ssr)
		lua /etc_ro/ss/genssrconfig.lua $1 $3 >$config_file
		sed -i 's/\\//g' $config_file
		;;
	trojan)
		v2ray_enable=1
		if [ "$2" = "0" ]; then
			lua /etc_ro/ss/gentrojanconfig.lua $1 nat 1080 >$trojan_json_file
			sed -i 's/\\//g' $trojan_json_file
		else
			lua /etc_ro/ss/gentrojanconfig.lua $1 client 10801 >/tmp/trojan-ssr-reudp.json
			sed -i 's/\\//g' /tmp/trojan-ssr-reudp.json
		fi
		;;
	v2ray)
		v2ray_enable=1
		if [ "$2" = "1" ]; then
			lua /etc_ro/ss/genv2config.lua $1 udp 1080 >/tmp/v2-ssr-reudp.json
			sed -i 's/\\//g' /tmp/v2-ssr-reudp.json
		else
			lua /etc_ro/ss/genv2config.lua $1 tcp 1080 >$v2_json_file
			sed -i 's/\\//g' $v2_json_file
		fi
		;;
	xray)
		v2ray_enable=1
		if [ "$2" = "1" ]; then
			lua /etc_ro/ss/genxrayconfig.lua $1 udp 1080 >/tmp/v2-ssr-reudp.json
			sed -i 's/\\//g' /tmp/v2-ssr-reudp.json
		else
			lua /etc_ro/ss/genxrayconfig.lua $1 tcp 1080 >$v2_json_file
			sed -i 's/\\//g' $v2_json_file
		fi
		;;	
	esac
}

get_arg_out() {
	router_proxy="1"
	case "$router_proxy" in
	1) echo "-o" ;;
	2) echo "-O" ;;
	esac
}

start_rules() {
    log "正在添加防火墙规则..."
	lua /etc_ro/ss/getconfig.lua $GLOBAL_SERVER > /tmp/server.txt
	server=`cat /tmp/server.txt`
	rm -f /tmp/server.txt
	cat /etc/storage/ss_ip.sh | grep -v '^!' | grep -v "^$" >$wan_fw_ips
	cat /etc/storage/ss_wan_ip.sh | grep -v '^!' | grep -v "^$" >$wan_bp_ips
	#resolve name
	if echo $server | grep -E "^[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}$" >/dev/null; then
		server=${server}
	elif [ "$server" != "${server#*:[0-9a-fA-F]}" ]; then
		server=${server}
	else
		server=$(resolveip -4 -t 3 $server | awk 'NR==1{print}')
		if echo $server | grep -E "^[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}$" >/dev/null; then
			echo $server >/etc/storage/ssr_ip
		else
			server=$(cat /etc/storage/ssr_ip)
		fi
	fi
	local_port="1080"
	lan_ac_ips=$lan_ac_ips
	lan_ac_mode="b"
	#if [ "$GLOBAL_SERVER" == "$UDP_RELAY_SERVER" ]; then
	#	ARG_UDP="-u"
	if [ "$UDP_RELAY_SERVER" != "nil" ]; then
		ARG_UDP="-U"
		lua /etc_ro/ss/getconfig.lua $UDP_RELAY_SERVER > /tmp/userver.txt
		udp_server=`cat /tmp/userver.txt`
		rm -f /tmp/userver.txt
		udp_local_port="1080"
	fi
	if [ -n "$lan_ac_ips" ]; then
		case "$lan_ac_mode" in
		w | W | b | B) ac_ips="$lan_ac_mode$lan_ac_ips" ;;
		esac
	fi
	#ac_ips="b"
	gfwmode=""
	if [ "$run_mode" = "gfw" ]; then
		gfwmode="-g"
	elif [ "$run_mode" = "router" ]; then
		gfwmode="-r"
	elif [ "$run_mode" = "oversea" ]; then
		gfwmode="-c"
	elif [ "$run_mode" = "all" ]; then
		gfwmode="-z"
	fi
	if [ "$lan_con" = "0" ]; then
		rm -f $lan_fp_ips
		lancon="all"
		lancons="全部IP走代理..."
		cat /etc/storage/ss_lan_ip.sh | grep -v '^!' | grep -v "^$" >$lan_fp_ips
	elif [ "$lan_con" = "1" ]; then
		rm -f $lan_fp_ips
		lancon="bip"
		lancons="指定 IP 走代理: 请到规则管理页面添加需要走代理的 IP..."
		cat /etc/storage/ss_lan_bip.sh | grep -v '^!' | grep -v "^$" >$lan_fp_ips
	fi
	rm -f $lan_gm_ips
	cat /etc/storage/ss_lan_gmip.sh | grep -v '^!' | grep -v "^$" >$lan_gm_ips
	dports=$(nvram get s_dports)
	if [ $dports = "0" ]; then
		proxyport="--syn"
	else
		proxyport="-m multiport --dports 22,53,587,465,995,993,143,80,443,3389 --syn"
	fi
	$SS_RULES \
		-s "$server" \
		-l "$local_port" \
		-S "$udp_server" \
		-L "$udp_local_port" \
		-a "$ac_ips" \
		-i "" \
		-b "$wan_bp_ips" \
		-w "$wan_fw_ips" \
		-p "$lan_fp_ips" \
		-G "$lan_gm_ips" \
		-D "$proxyport" \
		-k "$lancon" \
		$(get_arg_out) $gfwmode $ARG_UDP
	return $?
}

start_redir_tcp() {
	ARG_OTA=""
	gen_config_file $GLOBAL_SERVER 0 1080
	stype=$(nvram get d_type)
	local bin=$(find_bin $stype)
	[ ! -f "$bin" ] && log "Main node:Can't find $bin program, can't start!" && return 1
	if [ "$(nvram get ss_threads)" = "0" ]; then
		threads=$(cat /proc/cpuinfo | grep 'processor' | wc -l)
	else
		threads=$(nvram get ss_threads)
	fi
	log "正在启动 $stype 服务器..."
	case "$stype" in
	ss | ssr)
		last_config_file=$CONFIG_FILE
		pid_file="/tmp/ssr-retcp.pid"
		for i in $(seq 1 $threads); do
			run_bin "$bin" -c "$CONFIG_FILE" $ARG_OTA -f "/tmp/ssr-retcp_$i.pid" || return $?
			usleep 500000
		done
		redir_tcp=1
		log "Shadowsocks/ShadowsocksR $threads 线程启动成功..."
		;;
	trojan)
		for i in $(seq 1 $threads); do
			run_bin "$bin" --config "$trojan_json_file" || return $?
			usleep 500000
		done
		log "已运行 $($bin --version 2>&1 | head -1)"
		;;
	v2ray)
		run_bin "$bin" -config "$v2_json_file" || return $?
		log "已运行 $($bin -version | head -1)"
		;;
	xray)
		run_bin "$bin" -config "$v2_json_file" || return $?
		log "已运行 $($bin -version | head -1)"
		;;	
	socks5)
		for i in $(seq 1 $threads); do
			run_bin --wait-for ipt2socks lua /etc_ro/ss/gensocks.lua "$GLOBAL_SERVER" 1080 || return $?
			usleep 500000
		done
		;;
	esac
	return 0
}

start_redir_udp() {
	if [ "$UDP_RELAY_SERVER" != "nil" ]; then
		redir_udp=1
		utype=$(nvram get ud_type)
		log "正在启动 $utype 游戏 UDP 中继服务器..."
		local bin=$(find_bin $utype)
		[ ! -f "$bin" ] && log "UDP TPROXY Relay:Can't find $bin program, can't start!" && return 1
		case "$utype" in
		ss | ssr)
			ARG_OTA=""
			gen_config_file $UDP_RELAY_SERVER 1 1080
			last_config_file=$CONFIG_UDP_FILE
			pid_file="/var/run/ssr-reudp.pid"
			run_bin "$bin" -c "$last_config_file" $ARG_OTA -U -f /var/run/ssr-reudp.pid || return $?
			;;
		v2ray)
			gen_config_file $UDP_RELAY_SERVER 1
			run_bin "$bin" -config /tmp/v2-ssr-reudp.json || return $?
			;;
		xray)
			gen_config_file $UDP_RELAY_SERVER 1
			run_bin "$bin" -config /tmp/v2-ssr-reudp.json || return $?
			;;	
		trojan)
			gen_config_file $UDP_RELAY_SERVER 1
			run_bin "$bin" --config /tmp/trojan-ssr-reudp.json || return $?
			run_bin ipt2socks -U -b 0.0.0.0 -4 -s 127.0.0.1 -p 10801 -l 1080 || return $?
			;;
		socks5)
			echo "1"
			;;
		esac
	fi
	return 0
}

stop_dns_proxy() {
	ss_stop_named_processes dns2tcp dnsproxy
}

ss_dns_proxy_config_read() {
	local pdnsd_enable dnsstr dns_process dns_status

	SS_DNS_PROXY_PROCESS=""
	SS_DNS_PROXY_SERVER=""
	dnsserver=""
	pdnsd_enable_flag=""

	if pdnsd_enable=$(nvram get pdnsd_enable); then
		:
	else
		dns_status=$?
		log "Cannot read DNS proxy selector (status=$dns_status)"
		return "$dns_status"
	fi
	if dnsstr=$(nvram get tunnel_forward); then
		:
	else
		dns_status=$?
		log "Cannot read DNS upstream (status=$dns_status)"
		return "$dns_status"
	fi

	dnsserver=${dnsstr%%#*}
	[ -n "$dnsserver" ] && [ "$dnsserver" != "nil" ] || {
		log "DNS upstream is empty; refusing proxy restart"
		return 1
	}
	case "$dnsserver" in
		*[[:space:]]*)
			log "DNS upstream contains whitespace; refusing proxy restart"
			return 1
			;;
	esac

	case "$pdnsd_enable" in
		1) dns_process=dns2tcp ;;
		0) dns_process=dnsproxy ;;
		*)
			log "DNS解析方式不支持该选项: $pdnsd_enable , 建议选择dnsproxy"
			return 1
			;;
	esac
	if ! command -v "$dns_process" >/dev/null 2>&1; then
		log "DNS proxy executable is unavailable: $dns_process"
		return 127
	fi

	SS_DNS_PROXY_PROCESS=$dns_process
	SS_DNS_PROXY_SERVER=$dnsserver
	pdnsd_enable_flag=$pdnsd_enable
	return 0
}

start_dns_proxy() {
	local dns_process="$SS_DNS_PROXY_PROCESS"
	local dns_upstream="$SS_DNS_PROXY_SERVER"
	local launch_status=0

	[ -n "$dns_process" ] && [ -n "$dns_upstream" ] || {
		log "DNS proxy launch refused without a validated configuration"
		return 1
	}
	if [ "$dns_process" = "dns2tcp" ]; then
		log "启动 dns2tcp：5353 端口..."
		# 将dnsserver (上游国外DNS: 比如 8.8.8.8) 放入ipset:gfwlist，强制走SS_SPEC_WAN_FW代理
		ipset add gfwlist "$dns_upstream" 2>/dev/null
		dns2tcp -L"127.0.0.1#5353" -R"$dns_upstream" >/dev/null 2>&1 &
	else
		log "启动 dnsproxy：5353 端口..."
		# 将dnsserver (上游国外DNS: 比如 8.8.8.8) 放入ipset:gfwlist，强制走SS_SPEC_WAN_FW代理
		ipset add gfwlist "$dns_upstream" 2>/dev/null
		dnsproxy -d -p 5353 -R "$dns_upstream" >/dev/null 2>&1 &
	fi

	if ss_wait_named_process_started "$dns_process" 5; then
		return 0
	else
		launch_status=$?
	fi
	log "DNS proxy did not become available: $dns_process (status=$launch_status)"
	if ! ss_stop_named_processes "$dns_process"; then
		log "DNS proxy launch cleanup could not verify $dns_process shutdown"
	fi
	return "$launch_status"
}

ss_restart_dns_proxy() {
	local dns_status

	if ss_dns_proxy_config_read; then
		:
	else
		dns_status=$?
		return "$dns_status"
	fi
	if stop_dns_proxy; then
		:
	else
		dns_status=$?
		log "DNS proxy cleanup failed (status=$dns_status)"
		return "$dns_status"
	fi
	if start_dns_proxy; then
		return 0
	else
		dns_status=$?
		return "$dns_status"
	fi
}

ss_start_chinadns_daemon() {
	[ "$#" -eq 2 ] || return 2
	local china_dns="$1"
	local local_chnlist_file="$2"
	local chinadns_bin=""
	local chinadns_count=""
	local launch_status=""

	[ -n "$china_dns" ] || {
		log "ChinaDNS-ng upstream DNS is empty"
		return 1
	}
	if chinadns_count=$(ss_process_pid_count chinadns-ng); then
		:
	else
		launch_status=$?
		log "Cannot inspect ChinaDNS-ng before startup (status=$launch_status)"
		return "$launch_status"
	fi
	[ "$chinadns_count" -gt 0 ] && return 0

	if chinadns_bin=$(command -v chinadns-ng); then
		:
	else
		log "ChinaDNS-ng executable is unavailable"
		return 127
	fi
	[ -n "$chinadns_bin" ] || return 127

	if [ -n "$local_chnlist_file" ]; then
		run_bin "$chinadns_bin" -b 0.0.0.0 -l 65353 -c "$china_dns" \
			-t 127.0.0.1#5353 -4 china -M -m "$local_chnlist_file"
	else
		run_bin "$chinadns_bin" -b 0.0.0.0 -l 65353 -c "$china_dns" \
			-t 127.0.0.1#5353 -4 china
	fi
}

# Stage and validate the DNSMasq include before the RC-owned service lifecycle restart.
ss_publish_and_restart_dnsmasq() {
	[ "$#" -eq 3 ] || return 2
	local config_file="$1"
	local dnsmasq_bin="$2"
	local dnsmasq_profile="$3"
	local tmp_file=""
	local backup_file=""
	local restart_status=0
	local rollback_status=0

	case "$dnsmasq_profile" in 0|1|2) ;; *) return 2 ;; esac
	[ -f "$config_file" ] && [ -r "$config_file" ] || return 1
	[ -x "$dnsmasq_bin" ] || return 127
	tmp_file=$(mktemp "${config_file}.tmp.XXXXXX") || return 1
	backup_file=$(mktemp "${config_file}.rollback.XXXXXX") || {
		rm -f "$tmp_file"
		return 1
	}
	if ! cp -p "$config_file" "$tmp_file" ||
	   ! cp -p "$config_file" "$backup_file"; then
		rm -f "$tmp_file" "$backup_file"
		return 1
	fi
	if [ "$dnsmasq_profile" = "1" ]; then
		if ! ss_dnsmasq_remove_exact_directive "$tmp_file" 'no-resolv' ||
		   ! ss_dnsmasq_remove_exact_directive "$tmp_file" 'server=127.0.0.1' ||
		   ! ss_dnsmasq_remove_exact_directive "$tmp_file" 'server=127.0.0.1#65353' ||
		   ! printf 'no-resolv\nserver=127.0.0.1#65353\n' >> "$tmp_file"; then
			rm -f "$tmp_file" "$backup_file"
			return 1
		fi
	elif [ "$dnsmasq_profile" = "2" ]; then
		if ! ss_dnsmasq_remove_exact_directive "$tmp_file" 'conf-dir=/etc/storage/dnsmasq-ss.d' ||
		   ! ss_dnsmasq_remove_exact_directive "$tmp_file" 'conf-dir=/etc/storage/dnsmasq-ss.d/' ||
		   ! ss_dnsmasq_remove_exact_directive "$tmp_file" 'conf-dir=/etc/storage/dnsmasq.oversea' ||
		   ! ss_dnsmasq_remove_exact_directive "$tmp_file" 'conf-dir=/etc/storage/dnsmasq.oversea/' ||
		   ! printf 'conf-dir=/etc/storage/dnsmasq.oversea\n' >> "$tmp_file"; then
			rm -f "$tmp_file" "$backup_file"
			return 1
		fi
	fi
	if [ "$dnsmasq_profile" != "2" ]; then
		if ! sed -i '/^[[:space:]]*min-cache-ttl=.*$/d' "$tmp_file" ||
		   ! sed -i '/^[[:space:]]*dns-forward-max=.*$/d' "$tmp_file" ||
		   ! printf 'min-cache-ttl=1800\ndns-forward-max=1000\n' >> "$tmp_file"; then
			rm -f "$tmp_file" "$backup_file"
			return 1
		fi
	fi
	if ! "$dnsmasq_bin" --test -C "$tmp_file" >/dev/null 2>&1; then
		log "Staged dnsmasq config failed validation; previous config retained"
		rm -f "$tmp_file" "$backup_file"
		return 1
	fi

	# The RC service owns DNSMasq stop/start and serializes lifecycle changes
	# with /tmp/.dnsmasq.lifecycle.lock. Publish the validated include first,
	# then use its authoritative NVRAM configuration and fail-closed checks.
	if mv -f "$tmp_file" "$config_file"; then
		:
	else
		restart_status=$?
		log "dnsmasq config publication failed (status=$restart_status); previous config retained"
		rm -f "$tmp_file" "$backup_file"
		return "$restart_status"
	fi
	if ss_restart_dhcpd; then
		rm -f "$backup_file"
		return 0
	else
		restart_status=$?
	fi

	log "dnsmasq service restart failed (status=$restart_status); restoring previous config"
	if mv -f "$backup_file" "$config_file"; then
		backup_file=""
		if ss_restart_dhcpd; then
			log "Previous dnsmasq config restored and RC service restart succeeded"
		else
			rollback_status=$?
			log "Previous dnsmasq config restored, but RC service rollback failed (status=$rollback_status)"
		fi
	else
		rollback_status=$?
		log "Unable to restore previous dnsmasq config; backup retained at $backup_file (status=$rollback_status)"
	fi
	rm -f "$tmp_file"
	return "$restart_status"
}

start_dns() {
	local dnsmasq_restart_done=0

	echo "create china hash:net family inet hashsize 1024 maxelem 65536" >/tmp/china.ipset
	awk '!/^$/&&!/^#/{printf("add china %s'" "'\n",$0)}' /etc/storage/chinadns/chnroute.txt >>/tmp/china.ipset
	ipset -! flush china
	ipset -! restore </tmp/china.ipset 2>/dev/null
	rm -f /tmp/china.ipset
	start_chinadns() {
		local ss_chdns=""
		local china_dns=""
		local local_chnlist_file=""
		local use_chinadns=0
		local dns_status=""
		chinadnsng_enable_flag=0

		if ss_chdns=$(nvram get ss_chdns); then
			:
		else
			dns_status=$?
			return "$dns_status"
		fi
		if [ "$ss_chdns" = "1" ]; then
			use_chinadns=1
			local_chnlist_file='/etc/storage/chinadns/chnlist_mini.txt'
			if china_dns=$(nvram get china_dns); then
				:
			else
				dns_status=$?
				return "$dns_status"
			fi
			if [ -f "$local_chnlist_file" ]; then
				log "启动chinadns分流，仅国外域名走DNS代理..."
			else
				log "启动chinadns分流，全部域名走DNS代理...本次不使用本地cdn域名文件$local_chnlist_file, 下次你自已可以创建它，文件中每行表示一个域名（不用要子域名）"
				local_chnlist_file=""
			fi
			if ss_start_chinadns_daemon "$china_dns" "$local_chnlist_file"; then
				:
			else
				dns_status=$?
				log "ChinaDNS-ng failed to become ready (status=$dns_status)"
				return "$dns_status"
			fi
		fi
		if ss_publish_and_restart_dnsmasq "/etc/storage/dnsmasq/dnsmasq.conf" "/user/sbin/dnsmasq" "$use_chinadns"; then
			dnsmasq_restart_done=1
			if [ "$use_chinadns" = "1" ]; then
				chinadnsng_enable_flag=1
			fi
		else
			dns_status=$?
			log "dnsmasq configuration/restart failed (status=$dns_status)"
			return "$dns_status"
		fi
	}
	case "$run_mode" in
	router)
		# Validate the selector, upstream and executable before stopping the live proxy.
		# The checked launcher adds the validated upstream to gfwlist.
		if ss_restart_dns_proxy; then
			:
		else
			dns_status=$?
			return "$dns_status"
		fi
		if start_chinadns; then
			:
		else
			dns_status=$?
			return "$dns_status"
		fi
	;;
	gfw)
		# Validate the selector, upstream and executable before stopping the live proxy.
		# The checked launcher adds the validated upstream to gfwlist.
		if ss_restart_dns_proxy; then
			:
		else
			dns_status=$?
			return "$dns_status"
		fi
		if start_chinadns; then
			:
		else
			dns_status=$?
			return "$dns_status"
		fi
		log "开始处理 GFWList..."
		;;
	oversea)
		ipset add gfwlist $dnsserver 2>/dev/null
		mkdir -p /etc/storage/dnsmasq.oversea || return $?
		if ss_publish_and_restart_dnsmasq "/etc/storage/dnsmasq/dnsmasq.conf" "/user/sbin/dnsmasq" 2; then
			dnsmasq_restart_done=1
		else
			dns_status=$?
			log "oversea DNSMasq configuration/restart failed (status=$dns_status)"
			return "$dns_status"
		fi
		;;
	*)
		ipset -N ss_spec_wan_ac hash:net 2>/dev/null
		ipset add ss_spec_wan_ac $dnsserver 2>/dev/null
	;;
	esac
	# start_chinadns() publishes the validated config and already performs the
	# RC-serialized DNSMasq restart. Avoid immediately restarting it a second time.
	if [ "$dnsmasq_restart_done" -eq 1 ]; then
		return 0
	fi
	/sbin/restart_dhcpd
}

# Publish either a dnsmasq address configuration or a simple Adblock domain
# list. Never truncate the live file until a complete replacement is ready.
ss_publish_adblock_config() {
	[ "$#" -eq 2 ] || return 2
	local ss_adblock_input="$1"
	local ss_adblock_output="$2"
	local ss_adblock_tmp=""
	local ss_adblock_rules=""
	local ss_adblock_status=""

	[ -r "$ss_adblock_input" ] && [ -s "$ss_adblock_input" ] || return 1
	ss_adblock_tmp=$(mktemp "${ss_adblock_output}.tmp.XXXXXX") || return 1
	if grep -q '^[[:space:]]*address=' "$ss_adblock_input"; then
		# Downloaded dnsmasq input is untrusted: allow comments, blank lines
		# and address rules only; reject unrelated executable/config directives.
		if grep -Ev '^[[:space:]]*(#|address=|$)' "$ss_adblock_input" >/dev/null; then
			rm -f "$ss_adblock_tmp"
			return 1
		else
			ss_adblock_status=$?
			if [ "$ss_adblock_status" -ne 1 ]; then
				rm -f "$ss_adblock_tmp"
				return 1
			fi
		fi
		if ! grep '^[[:space:]]*address=' "$ss_adblock_input" > "$ss_adblock_tmp"; then
			rm -f "$ss_adblock_tmp"
			return 1
		fi
	else
		ss_adblock_rules=$(mktemp "${ss_adblock_output}.rules.XXXXXX") || {
			rm -f "$ss_adblock_tmp"
			return 1
		}
		if ! grep '^[|][|][A-Za-z0-9][A-Za-z0-9.-]*\^$' \
			"$ss_adblock_input" > "$ss_adblock_rules"; then
			rm -f "$ss_adblock_rules" "$ss_adblock_tmp"
			return 1
		fi
		if ! sed -e 's:^[|][|]:address=/:' \
			-e 's:\^$:/0.0.0.0:' "$ss_adblock_rules" > "$ss_adblock_tmp"; then
			rm -f "$ss_adblock_rules" "$ss_adblock_tmp"
			return 1
		fi
		rm -f "$ss_adblock_rules"
	fi

	if [ ! -s "$ss_adblock_tmp" ] || ! chmod 644 "$ss_adblock_tmp"; then
		rm -f "$ss_adblock_tmp"
		return 1
	fi
	if ! mv -f "$ss_adblock_tmp" "$ss_adblock_output"; then
		rm -f "$ss_adblock_tmp"
		return 1
	fi
	return 0
}

start_AD() {
	local ss_adblock_url=""
	local ss_adblock_input=""
	local ss_adblock_output="/tmp/dnsmasq.dom/anti-ad-for-dnsmasq.conf"

	if ! mkdir -p /tmp/dnsmasq.dom; then
		log "Unable to prepare adblock configuration directory"
		return 1
	fi
	ss_adblock_url=$(nvram get ss_adblock_url)
	if [ -z "$ss_adblock_url" ] || [ "$ss_adblock_url" = "nil" ]; then
		log "广告过滤地址未配置，保留上次可用配置"
		return 1
	fi
	ss_adblock_input=$(mktemp /tmp/adnew.conf.XXXXXX) || {
		log "Unable to create temporary adblock download"
		return 1
	}
	if ! curl -fsS -o "$ss_adblock_input" --connect-timeout 10 --retry 3 "$ss_adblock_url"; then
		rm -f "$ss_adblock_input"
		log "广告过滤文件下载失败，保留上次可用配置"
		return 1
	fi
	if ! ss_publish_adblock_config "$ss_adblock_input" "$ss_adblock_output"; then
		rm -f "$ss_adblock_input"
		log "广告过滤文件格式无效，保留上次可用配置"
		return 1
	fi
	rm -f "$ss_adblock_input"
	log "去广告文件下载并发布成功"
	return 0
}

# ========== 启动 Socks5 代理 ==========
start_local() {
	local s5_port=$(nvram get socks5_port)
	local local_server=$(nvram get socks5_enable)
	[ "$local_server" == "nil" ] && return 1
	[ "$local_server" == "same" ] && local_server=$GLOBAL_SERVER
	local type=$(nvram get s5_type)
	local bin=$(find_bin $type)
	[ ! -f "$bin" ] && log "Global_Socks5:Can't find $bin program, can't start!" && return 1
	case "$type" in
	ss | ssr)
		local name="Shadowsocks"
		local bin=$(find_bin ss-local)
		[ ! -f "$bin" ] && log "Global_Socks5:Can't find $bin program, can't start!" && return 1
		[ "$type" == "ssr" ] && name="ShadowsocksR"
		gen_config_file $local_server 3 $s5_port
		run_bin "$bin" -c "$CONFIG_SOCK5_FILE" -u -f /var/run/ssr-local.pid || return $?
		log "Global_Socks5:$name Started!"
		;;
	v2ray)
		lua /etc_ro/ss/genv2config.lua $local_server tcp 0 $s5_port >/tmp/v2-ssr-local.json
		sed -i 's/\\//g' /tmp/v2-ssr-local.json
		run_bin "$bin" -config /tmp/v2-ssr-local.json || return $?
		log "Global_Socks5:$($bin -version | head -1) Started!"
		;;
	xray)
		lua /etc_ro/ss/genxrayconfig.lua $local_server tcp 0 $s5_port >/tmp/v2-ssr-local.json
		sed -i 's/\\//g' /tmp/v2-ssr-local.json
		run_bin "$bin" -config /tmp/v2-ssr-local.json || return $?
		log "Global_Socks5:$($bin -version | head -1) Started!"
		;;
	trojan)
		lua /etc_ro/ss/gentrojanconfig.lua $local_server client $s5_port >/tmp/trojan-ssr-local.json
		sed -i 's/\\//g' /tmp/trojan-ssr-local.json
		run_bin "$bin" --config /tmp/trojan-ssr-local.json || return $?
		log "Global_Socks5:$($bin --version 2>&1 | head -1) Started!"
		;;
	*)
		[ -e /proc/sys/net/ipv6 ] && local listenip='-i ::'
		run_bin microsocks $listenip -p "$s5_port" ssr-local || return $?
		log "Global_Socks5:$type Started!"
		;;
	esac
	local_enable=1
	return 0
}

rules() {
	[ "$GLOBAL_SERVER" = "nil" ] && return 1
	UDP_RELAY_SERVER=$(nvram get udp_relay_server)
	if [ "$UDP_RELAY_SERVER" = "same" ]; then
		UDP_RELAY_SERVER=$GLOBAL_SERVER
	fi
	if start_rules; then
		return 0
	else
		status=$?
		return "$status"
	fi
}

ss_launch_watchcat() {
	/usr/bin/ss-monitor "$@" >/dev/null 2>&1 &
}

start_watchcat() {
	[ "$(nvram get ss_watchcat)" = "1" ] || return 0
	total_count=$((server_count + redir_tcp + redir_udp + tunnel_enable + v2ray_enable + local_enable + pdnsd_enable_flag + chinadnsng_enable_flag))
	[ "$total_count" -gt 0 ] || return 0

	if watchcat_process_count=$(ss_process_pattern_count "/usr/bin/ss-monitor"); then
		:
	else
		watchcat_status=$?
		log "Cannot inspect ss-monitor before startup (status=$watchcat_status)"
		return "$watchcat_status"
	fi
	[ "$watchcat_process_count" -eq 0 ] || return 0
	watchcat_expected_count=$((watchcat_process_count + 1))

	if ss_launch_watchcat "$server_count" "$redir_tcp" "$redir_udp" "$tunnel_enable" "$v2ray_enable" "$local_enable" "$pdnsd_enable_flag" "$chinadnsng_enable_flag"; then
		:
	else
		watchcat_status=$?
		log "Unable to schedule ss-monitor startup (status=$watchcat_status)"
		return "$watchcat_status"
	fi
	if ss_wait_process_pattern_count "/usr/bin/ss-monitor" "$watchcat_expected_count" 5; then
		return 0
	else
		watchcat_status=$?
	fi
	log "ss-monitor did not become ready (status=$watchcat_status)"
	return "$watchcat_status"
}

auto_update() {
	sed -i '/update_chnroute/d' /etc/storage/cron/crontabs/$http_username
	sed -i '/update_gfwlist/d' /etc/storage/cron/crontabs/$http_username
	sed -i '/ss-watchcat/d' /etc/storage/cron/crontabs/$http_username
	if [ $(nvram get ss_update_chnroute) = "1" ]; then
		cat >>/etc/storage/cron/crontabs/$http_username <<EOF
0 7 * * * /usr/bin/update_chnroute.sh > /dev/null 2>&1
EOF
	fi
	if [ $(nvram get ss_update_gfwlist) = "1" ]; then
		cat >>/etc/storage/cron/crontabs/$http_username <<EOF
0 8 * * * /usr/bin/update_gfwlist.sh > /dev/null 2>&1
EOF
	fi
}

# Keep the first required startup failure while allowing independent
# services and WAN reconciliation to finish their own cleanup/recovery work.
ssp_record_start_failure() {
	local stage="$1"
	local status="$2"

	[ "$status" -ne 0 ] || return 0
	if [ "$SS_START_STATUS" -eq 0 ]; then
		SS_START_STATUS="$status"
	fi
	log "Shadowsocks startup stage failed: $stage (status=$status)"
	return 0
}

ssp_run_pppoe_detect() {
	/usr/bin/detect.sh
}

# ========== 启动 SS ==========
ssp_start() {
	local stage_status
	local local_server

	SS_START_STATUS=0
	ss_enable=`nvram get ss_enable`
	if rules; then
		if cgroups_init; then
			:
		else
			stage_status=$?
			ssp_record_start_failure "resource limits" "$stage_status"
		fi
		if start_redir_tcp; then
			if start_redir_udp; then
				:
			else
				stage_status=$?
				ssp_record_start_failure "UDP relay" "$stage_status"
			fi
			if start_dns; then
				:
			else
				stage_status=$?
				ssp_record_start_failure "DNS proxy" "$stage_status"
			fi
		else
			stage_status=$?
			ssp_record_start_failure "TCP proxy" "$stage_status"
		fi
	else
		stage_status=$?
		# No global node is a supported local-only configuration. rules()
		# returns 1 for that state, and the legacy API reports no global start.
		if [ "$GLOBAL_SERVER" != "nil" ]; then
			ssp_record_start_failure "firewall policy" "$stage_status"
		fi
	fi

	if local_server=$(nvram get socks5_enable); then
		if [ -n "$local_server" ] && [ "$local_server" != "nil" ]; then
			if start_local; then
				:
			else
				stage_status=$?
				ssp_record_start_failure "local SOCKS5 proxy" "$stage_status"
			fi
		fi
	else
		stage_status=$?
		ssp_record_start_failure "local SOCKS5 configuration" "$stage_status"
	fi

	if start_watchcat; then
		:
	else
		stage_status=$?
		ssp_record_start_failure "watchcat" "$stage_status"
	fi

	# Cron refresh is maintenance-only; log its failure without marking the
	# proxy service unavailable.
	if auto_update; then
		:
	else
		stage_status=$?
		log "Optional Shadowsocks update scheduling failed (status=$stage_status)"
	fi

	if ENABLE_SERVER=$(nvram get global_server); then
		:
	else
		stage_status=$?
		ssp_record_start_failure "global server configuration" "$stage_status"
	fi
	if [ "$ENABLE_SERVER" = "nil" ]; then
		[ "$SS_START_STATUS" -ne 0 ] && return "$SS_START_STATUS"
		return 1
	fi

	if nvram set check_mode=0; then
		:
	else
		stage_status=$?
		ssp_record_start_failure "check-mode update" "$stage_status"
	fi
	if [ "$pppoemwan" = 0 ]; then
		if ssp_run_pppoe_detect; then
			:
		else
			stage_status=$?
			ssp_record_start_failure "PPPoE-MWAN reconciliation" "$stage_status"
		fi
	fi

	[ "$SS_START_STATUS" -ne 0 ] && return "$SS_START_STATUS"
	log "已启动科学上网..."
	log "内网IP控制为: $lancons"
	return 0
}

ss_restart_dhcpd() {
	/sbin/restart_dhcpd
}

ss_run_detect() {
	/usr/bin/detect.sh
}

# ========== 关闭 SS ==========
ssp_close() {
	local close_status=0
	local command_status=0
	local cleanup_directive=""

	# Do not tear down firewall/configuration state while a proxy process can
	# still be running and consuming it.
	if ! kill_process; then
		log "Proxy process stop incomplete; preserving Shadowsocks firewall and cgroup state"
		return 1
	fi
	rm -rf /tmp/cdn
	if "$SS_RULES" -f; then
		:
	else
		command_status=$?
		close_status=$command_status
		log "Shadowsocks firewall cleanup failed (status=$command_status)"
	fi
	cgroups_cleanup
	for cleanup_directive in \
		'no-resolv' \
		'server=127.0.0.1' \
		'server=127.0.0.1#65353' \
		'conf-dir=/tmp/cdn' \
		'conf-dir=/tmp/cdn/' \
		'conf-dir=/tmp/dnsmasq.dom' \
		'conf-dir=/tmp/dnsmasq.dom/' \
		'conf-file=/tmp/dnsmasq.dom/gfwlist_list.conf' \
		'conf-dir=/etc/storage/dnsmasq-ss.d' \
		'conf-dir=/etc/storage/dnsmasq-ss.d/' \
		'conf-dir=/etc/storage/dnsmasq.oversea' \
		'conf-dir=/etc/storage/dnsmasq.oversea/'; do
		if ss_dnsmasq_remove_exact_directive "/etc/storage/dnsmasq/dnsmasq.conf" "$cleanup_directive"; then
			:
		else
			command_status=$?
			if [ "$close_status" -eq 0 ]; then close_status=$command_status; fi
			log "DNSMasq config cleanup failed for $cleanup_directive (status=$command_status)"
		fi
	done
	if [ -f "/etc/storage/dnsmasq-ss.d" ]; then
		if rm -f /etc/storage/dnsmasq-ss.d; then
			:
		else
			command_status=$?
			if [ "$close_status" -eq 0 ]; then
				close_status=$command_status
			fi
			log "DNSMasq Shadowsocks include cleanup failed (status=$command_status)"
		fi
	fi
	clear_iptable
	if ss_restart_dhcpd; then
		:
	else
		command_status=$?
		if [ "$close_status" -eq 0 ]; then
			close_status=$command_status
		fi
		log "DNSMasq service restart after Shadowsocks shutdown failed (status=$command_status)"
	fi
	if [ "$pppoemwan" = 0 ]; then
		if ss_run_detect; then
			:
		else
			command_status=$?
			if [ "$close_status" -eq 0 ]; then
				close_status=$command_status
			fi
			log "PPPoE-MWAN reconciliation after Shadowsocks shutdown failed (status=$command_status)"
		fi
	fi
	return "$close_status"
}


clear_iptable() {
	s5_port=$(nvram get socks5_port)
	iptables -t filter -D INPUT -p tcp --dport $s5_port -j ACCEPT 2>/dev/null
	iptables -t filter -D INPUT -p tcp --dport $s5_port -j ACCEPT 2>/dev/null
	ip6tables -t filter -D INPUT -p tcp --dport $s5_port -j ACCEPT 2>/dev/null
	ip6tables -t filter -D INPUT -p tcp --dport $s5_port -j ACCEPT 2>/dev/null
}

kill_process() {
	local status=0
	ss_stop_process_pattern '/usr/bin/ss-monitor' || status=1
	ss_stop_named_processes \
		v2ray xray \
		ss-redir ssr-redir ss-local \
		trojan ipt2socks srelay ssr-server \
		chinadns-ng dns2tcp dnsproxy microsocks || status=1
	return "$status"
}

# ========== 启用备用服务器 ==========
ressp() {
	local stage_status
	local local_server

	SS_START_STATUS=0
	if BACKUP_SERVER=$(nvram get backup_server); then
		:
	else
		stage_status=$?
		log "Cannot read backup server configuration (status=$stage_status)"
		return "$stage_status"
	fi
	case "$BACKUP_SERVER" in
		''|nil)
			log "Backup server is not configured; refusing backup start"
			return 1
			;;
	esac

	# Firewall and config generators read GLOBAL_SERVER, not an argument.
	GLOBAL_SERVER=$BACKUP_SERVER
	if rules; then
		if cgroups_init; then
			:
		else
			stage_status=$?
			ssp_record_start_failure "backup resource limits" "$stage_status"
		fi
		if start_redir_tcp; then
			if start_redir_udp; then
				:
			else
				stage_status=$?
				ssp_record_start_failure "backup UDP relay" "$stage_status"
			fi
			if start_dns; then
				:
			else
				stage_status=$?
				ssp_record_start_failure "backup DNS proxy" "$stage_status"
			fi
		else
			stage_status=$?
			ssp_record_start_failure "backup TCP proxy" "$stage_status"
		fi
	else
		stage_status=$?
		ssp_record_start_failure "backup firewall policy" "$stage_status"
	fi

	if local_server=$(nvram get socks5_enable); then
		if [ -n "$local_server" ] && [ "$local_server" != "nil" ]; then
			if start_local; then
				:
			else
				stage_status=$?
				ssp_record_start_failure "backup local SOCKS5 proxy" "$stage_status"
			fi
		fi
	else
		stage_status=$?
		ssp_record_start_failure "backup local SOCKS5 configuration" "$stage_status"
	fi

	if start_watchcat; then
		:
	else
		stage_status=$?
		ssp_record_start_failure "backup watchcat" "$stage_status"
	fi

	# Optional cron scheduling is diagnostic-only and cannot mask startup errors.
	if auto_update; then
		:
	else
		stage_status=$?
		log "Optional backup Shadowsocks update scheduling failed (status=$stage_status)"
	fi
	ENABLE_SERVER=$(nvram get global_server)
	[ "$SS_START_STATUS" -ne 0 ] && return "$SS_START_STATUS"
	log "备用服务器启动成功！"
	log "内网IP控制为: $lancons"
	return 0
}

case $1 in
start)
	if [ $(nvram get ss_adblock) = "1" ]; then
		start_AD
	fi
	ssp_start || exit $?
	echo 3 > /proc/sys/vm/drop_caches
	;;
stop)
	if ! ssp_close; then
		exit 1
	fi
	echo 3 > /proc/sys/vm/drop_caches
	;;
restart)
	if ! ssp_close; then
		exit 1
	fi
	ssp_start || exit $?
	echo 3 > /proc/sys/vm/drop_caches
	;;
reserver)
	if ! ssp_close; then
		exit 1
	fi
	ressp || exit $?
	echo 3 > /proc/sys/vm/drop_caches
	;;
*)
	echo "check"
	#exit 0
	;;
esac

