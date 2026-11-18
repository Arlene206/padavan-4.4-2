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
	(if [ "$(nvram get ss_cgroups)" = "1" ]; then
	 	echo 0 > /sys/fs/cgroup/cpu/$NAME/tasks
	 	echo 0 > /sys/fs/cgroup/memory/$NAME/tasks
	 fi
	 "$@" > /dev/null 2>&1
	) &
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
			run_bin $bin -c $CONFIG_FILE $ARG_OTA -f /tmp/ssr-retcp_$i.pid
			usleep 500000
		done
		redir_tcp=1
		log "Shadowsocks/ShadowsocksR $threads 线程启动成功..."
		;;
	trojan)
		for i in $(seq 1 $threads); do
			run_bin $bin --config $trojan_json_file
			usleep 500000
		done
		log "已运行 $($bin --version 2>&1 | head -1)"
		;;
	v2ray)
		run_bin $bin -config $v2_json_file
		log "已运行 $($bin -version | head -1)"
		;;
	xray)
		run_bin $bin -config $v2_json_file
		log "已运行 $($bin -version | head -1)"
		;;	
	socks5)
		for i in $(seq 1 $threads); do
			run_bin lua /etc_ro/ss/gensocks.lua $GLOBAL_SERVER 1080
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
			run_bin $bin -c $last_config_file $ARG_OTA -U -f /var/run/ssr-reudp.pid
			;;
		v2ray)
			gen_config_file $UDP_RELAY_SERVER 1
			run_bin $bin -config /tmp/v2-ssr-reudp.json
			;;
		xray)
			gen_config_file $UDP_RELAY_SERVER 1
			run_bin $bin -config /tmp/v2-ssr-reudp.json
			;;	
		trojan)
			gen_config_file $UDP_RELAY_SERVER 1
			$bin --config /tmp/trojan-ssr-reudp.json >/dev/null 2>&1 &
			run_bin ipt2socks -U -b 0.0.0.0 -4 -s 127.0.0.1 -p 10801 -l 1080
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

start_dns_proxy() {
	local dns_process=""
	local launch_status=0

	pdnsd_enable=$(nvram get pdnsd_enable) # 0: dnsproxy , 1: dns2tcp
	pdnsd_enable_flag=$pdnsd_enable
	dnsstr="$(nvram get tunnel_forward)"
	dnsserver=$(echo "$dnsstr" | awk -F '#' '{print $1}')
	[ -n "$dnsserver" ] && [ "$dnsserver" != "nil" ] || {
		log "DNS upstream is empty; refusing proxy launch"
		return 1
	}

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

	if [ "$dns_process" = "dns2tcp" ]; then
		log "启动 dns2tcp：5353 端口..."
		# 将dnsserver (上游国外DNS: 比如 8.8.8.8) 放入ipset:gfwlist，强制走SS_SPEC_WAN_FW代理
		ipset add gfwlist "$dnsserver" 2>/dev/null
		dns2tcp -L"127.0.0.1#5353" -R"$dnsserver" >/dev/null 2>&1 &
	else
		log "启动 dnsproxy：5353 端口..."
		# 将dnsserver (上游国外DNS: 比如 8.8.8.8) 放入ipset:gfwlist，强制走SS_SPEC_WAN_FW代理
		ipset add gfwlist "$dnsserver" 2>/dev/null
		dnsproxy -d -p 5353 -R "$dnsserver" >/dev/null 2>&1 &
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

start_dns() {
	echo "create china hash:net family inet hashsize 1024 maxelem 65536" >/tmp/china.ipset
	awk '!/^$/&&!/^#/{printf("add china %s'" "'\n",$0)}' /etc/storage/chinadns/chnroute.txt >>/tmp/china.ipset
	ipset -! flush china
	ipset -! restore </tmp/china.ipset 2>/dev/null
	rm -f /tmp/china.ipset
	start_chinadns() {
		ss_chdns=$(nvram get ss_chdns)
		if [ $ss_chdns = 1 ]; then
			chinadnsng_enable_flag=1
			local_chnlist_file='/etc/storage/chinadns/chnlist_mini.txt'
			if [ -f "$local_chnlist_file" ]; then
			  log "启动chinadns分流，仅国外域名走DNS代理..."
			  chinadns-ng -b 0.0.0.0 -l 65353 -c $(nvram get china_dns) -t 127.0.0.1#5353 -4 china -M -m $local_chnlist_file >/dev/null 2>&1 &
			else
			  log "启动chinadns分流，全部域名走DNS代理...本次不使用本地cdn域名文件$local_chnlist_file, 下次你自已可以创建它，文件中每行表示一个域名（不用要子域名）"
			  chinadns-ng -b 0.0.0.0 -l 65353 -c $(nvram get china_dns) -t 127.0.0.1#5353 -4 china >/dev/null 2>&1 &
			fi
			# adding upstream chinadns-ng 
			sed -i '/no-resolv/d' /etc/storage/dnsmasq/dnsmasq.conf
			sed -i '/server=127.0.0.1/d' /etc/storage/dnsmasq/dnsmasq.conf
			cat >> /etc/storage/dnsmasq/dnsmasq.conf << EOF
no-resolv
server=127.0.0.1#65353
EOF
		fi
		# dnsmasq optimization
		sed -i '/min-cache-ttl/d' /etc/storage/dnsmasq/dnsmasq.conf
		sed -i '/dns-forward-max/d' /etc/storage/dnsmasq/dnsmasq.conf
		cat >> /etc/storage/dnsmasq/dnsmasq.conf << EOF
min-cache-ttl=1800
dns-forward-max=1000
EOF
		# restart dnsmasq
		killall dnsmasq
		/user/sbin/dnsmasq >/dev/null 2>&1 &
	}
	case "$run_mode" in
	router)
		ipset add gfwlist $dnsserver 2>/dev/null
		# 不论chinadns-ng打开与否，都重启dns_proxy 
		# 原因是针对gfwlist ipset有一个专有的dnsmasq配置表（由ss-rule创建放在/tmp/dnsmasq.dom/gfwlist_list.conf)
		# 需要查询上游dns_proxy在本地5353端口
		if stop_dns_proxy; then
			:
		else
			dns_status=$?
			log "DNS proxy cleanup failed (status=$dns_status)"
			return "$dns_status"
		fi
		if start_dns_proxy; then
			:
		else
			dns_status=$?
			return "$dns_status"
		fi
		start_chinadns
	;;
	gfw)
		dnsstr="$(nvram get tunnel_forward)"
		dnsserver=$(echo "$dnsstr" | awk -F '#' '{print $1}')
		#dnsport=$(echo "$dnsstr" | awk -F '#' '{print $2}')
		ipset add gfwlist $dnsserver 2>/dev/null
		if stop_dns_proxy; then
			:
		else
			dns_status=$?
			log "DNS proxy cleanup failed (status=$dns_status)"
			return "$dns_status"
		fi
		if start_dns_proxy; then
			:
		else
			dns_status=$?
			return "$dns_status"
		fi
		start_chinadns
		log "开始处理 GFWList..."
		;;
	oversea)
		ipset add gfwlist $dnsserver 2>/dev/null
		mkdir -p /etc/storage/dnsmasq.oversea
		sed -i '/dnsmasq-ss/d' /etc/storage/dnsmasq/dnsmasq.conf
		sed -i '/dnsmasq.oversea/d' /etc/storage/dnsmasq/dnsmasq.conf
		cat >>/etc/storage/dnsmasq/dnsmasq.conf <<EOF
conf-dir=/etc/storage/dnsmasq.oversea
EOF
;;
	*)
		ipset -N ss_spec_wan_ac hash:net 2>/dev/null
		ipset add ss_spec_wan_ac $dnsserver 2>/dev/null
	;;
	esac
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
		run_bin $bin -c $CONFIG_SOCK5_FILE -u -f /var/run/ssr-local.pid
		log "Global_Socks5:$name Started!"
		;;
	v2ray)
		lua /etc_ro/ss/genv2config.lua $local_server tcp 0 $s5_port >/tmp/v2-ssr-local.json
		sed -i 's/\\//g' /tmp/v2-ssr-local.json
		run_bin $bin -config /tmp/v2-ssr-local.json
		log "Global_Socks5:$($bin -version | head -1) Started!"
		;;
	xray)
		lua /etc_ro/ss/genxrayconfig.lua $local_server tcp 0 $s5_port >/tmp/v2-ssr-local.json
		sed -i 's/\\//g' /tmp/v2-ssr-local.json
		run_bin $bin -config /tmp/v2-ssr-local.json
		log "Global_Socks5:$($bin -version | head -1) Started!"
		;;
	trojan)
		lua /etc_ro/ss/gentrojanconfig.lua $local_server client $s5_port >/tmp/trojan-ssr-local.json
		sed -i 's/\\//g' /tmp/trojan-ssr-local.json
		run_bin $bin --config /tmp/trojan-ssr-local.json
		log "Global_Socks5:$($bin --version 2>&1 | head -1) Started!"
		;;
	*)
		[ -e /proc/sys/net/ipv6 ] && local listenip='-i ::'
		run_bin microsocks $listenip -p $s5_port ssr-local
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

start_watchcat() {
	if [ $(nvram get ss_watchcat) = 1 ]; then
		let total_count=server_count+redir_tcp+redir_udp+tunnel_enable+v2ray_enable+local_enable+pdnsd_enable_flag+chinadnsng_enable_flag
		if [ $total_count -gt 0 ]; then
			#param:server(count) redir_tcp(0:no,1:yes)  redir_udp tunnel kcp local gfw
			/usr/bin/ss-monitor $server_count $redir_tcp $redir_udp $tunnel_enable $v2ray_enable $local_enable $pdnsd_enable_flag $chinadnsng_enable_flag >/dev/null 2>&1 &
		fi
	fi
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

# ========== 关闭 SS ==========
ssp_close() {
	# Do not tear down firewall/configuration state while a proxy process can
	# still be running and consuming it.
	if ! kill_process; then
		log "Proxy process stop incomplete; preserving Shadowsocks firewall and cgroup state"
		return 1
	fi
	rm -rf /tmp/cdn
	$SS_RULES -f
	cgroups_cleanup
	sed -i '/no-resolv/d' /etc/storage/dnsmasq/dnsmasq.conf
	sed -i '/server=127.0.0.1/d' /etc/storage/dnsmasq/dnsmasq.conf
	sed -i '/cdn/d' /etc/storage/dnsmasq/dnsmasq.conf
	sed -i '/gfwlist/d' /etc/storage/dnsmasq/dnsmasq.conf
	sed -i '/dnsmasq.oversea/d' /etc/storage/dnsmasq/dnsmasq.conf
	if [ -f "/etc/storage/dnsmasq-ss.d" ]; then
		rm -f /etc/storage/dnsmasq-ss.d
	fi
	clear_iptable
	/sbin/restart_dhcpd
	if [ "$pppoemwan" = 0 ]; then
		/usr/bin/detect.sh
	fi
	return 0
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
	BACKUP_SERVER=$(nvram get backup_server)
	start_redir $BACKUP_SERVER
	start_rules $BACKUP_SERVER
	start_dns
	start_local
	start_watchcat
	auto_update
	ENABLE_SERVER=$(nvram get global_server)
	log "备用服务器启动成功！"
	log "内网IP控制为: $lancons"
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
	ressp
	echo 3 > /proc/sys/vm/drop_caches
	;;
*)
	echo "check"
	#exit 0
	;;
esac

