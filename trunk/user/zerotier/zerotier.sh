#!/bin/sh
#20200426 chongshengB
#20210410 xumng123
PROG=/usr/bin/zerotier-one
PROGCLI=/usr/bin/zerotier-cli
PROGIDT=/usr/bin/zerotier-idtool
config_path="/etc/storage/zerotier-one"
PLANET="/etc/storage/planet"
ZT_READY_TIMEOUT=30
start_instance() {
	cfg="$1"
	echo $cfg
	port=""
	args=""
	moonid="$(nvram get zerotier_moonid)"
	secret="$(nvram get zerotier_secret)"
	enablemoonserv="$(nvram get zerotiermoon_enable)"
	planet="$(nvram get zerotier_planet)"
	
	if [ ! -d "$config_path" ]; then
		mkdir -p $config_path
	fi
	mkdir -p $config_path/networks.d
	if [ -n "$port" ]; then
		args="$args -p$port"
	fi
	if [ -z "$secret" ]; then
		logger -t "zerotier" "设备密匙为空,正在生成密匙,请稍后..."
		sf="$config_path/identity.secret"
		pf="$config_path/identity.public"
		$PROGIDT generate "$sf" "$pf"  >/dev/null
		[ $? -ne 0 ] && return 1
		secret="$(cat $sf)"
		#rm "$sf"
		nvram set zerotier_secret="$secret"
		nvram commit
	fi
	if [ -n "$secret" ]; then
		logger -t "zerotier" "找到密匙,正在写入文件,请稍后..."
		echo "$secret" >$config_path/identity.secret
		$PROGIDT getpublic $config_path/identity.secret >$config_path/identity.public
		#rm -f $config_path/identity.public
	fi
	
	if [ -n "$planet" ]; then
		logger -t "zerotier" "找到planet,正在写入文件,请稍后..."
		echo "$planet" >$config_path/planet.tmp
		base64 -d $config_path/planet.tmp >$config_path/planet
	fi
	
	if [ -f "$PLANET" ]; then
		if [ ! -s "$PLANET" ]; then
			logger -t "zerotier" "自定义planet文件为空,删除..."
			rm -f $config_path/planet
			rm -f $PLANET
			nvram set zerotier_planet=""
			nvram commit
		else
			logger -t "zerotier" "自定义planet文件不为空,创建..."
			planet="$(base64 $PLANET)"
			cp -f $PLANET $config_path/planet
			rm -f $PLANET
			nvram set zerotier_planet="$planet"
			nvram commit
		fi
	fi

	add_join $(nvram get zerotier_id)

	$PROG $args $config_path >/dev/null 2>&1 &

	if ! rules; then
		logger -t "zerotier" "ZeroTier接口/路由未在限定时间内就绪，停止本次启动"
		if ! kill_z; then
			logger -t "zerotier" "启动失败后仍无法确认ZeroTier进程退出"
		fi
		return 1
	fi

	if [ -n "$moonid" ]; then
		$PROGCLI -D$config_path orbit $moonid $moonid
		logger -t "zerotier" "orbit moonid $moonid ok!"
	fi


	if [ -n "$enablemoonserv" ]; then
		if [ "$enablemoonserv" -eq "1" ]; then
			logger -t "zerotier" "creat moon start"
			creat_moon
		else
			logger -t "zerotier" "remove moon start"
			remove_moon
		fi
	fi
}

add_join() {
		touch $config_path/networks.d/$1.conf
}


zerotier_interface_name() {
	ifconfig | awk '$1 ~ /^zt/ { sub(/:$/, "", $1); print $1; exit }'
}

zerotier_kernel_route_segment() {
	[ -n "$1" ] || return 1
	ip route | awk -v zt_if="$1" '
	{
		dev = ""
		proto = ""
		for (i = 1; i < NF; i++) {
			if ($i == "dev")
				dev = $(i + 1)
			else if ($i == "proto")
				proto = $(i + 1)
		}
		if (dev == zt_if && proto == "kernel") {
			print $1
			exit
		}
	}'
}

zerotier_wait_interface() {
	zerotier_wait=0
	while :; do
		zt0="$(zerotier_interface_name)"
		if [ -n "$zt0" ]; then
			printf '%s\n' "$zt0"
			return 0
		fi
		if [ "$zerotier_wait" -ge "$ZT_READY_TIMEOUT" ]; then
			break
		fi
		sleep 1
		zerotier_wait=$((zerotier_wait + 1))
	done
	return 1
}

zerotier_wait_route() {
	zerotier_wait=0
	while :; do
		ip_segment="$(zerotier_kernel_route_segment "$1")"
		if [ -n "$ip_segment" ]; then
			printf '%s\n' "$ip_segment"
			return 0
		fi
		if [ "$zerotier_wait" -ge "$ZT_READY_TIMEOUT" ]; then
			break
		fi
		sleep 1
		zerotier_wait=$((zerotier_wait + 1))
	done
	return 1
}

rules() {
	zt0="$(zerotier_wait_interface)" || {
		logger -t "zerotier" "等待ZeroTier接口超时"
		return 1
	}
	nat_enable="$(nvram get zerotier_nat)"
	case "$nat_enable" in
		"") nat_enable=0 ;;
		0|1) ;;
		*)
			logger -t "zerotier" "无效的ZeroTier NAT配置"
			return 1
			;;
	esac
	ip_segment=""
	if [ "$nat_enable" = "1" ]; then
		ip_segment="$(zerotier_wait_route "$zt0")" || {
			logger -t "zerotier" "等待ZeroTier内核路由超时"
			return 1
		}
	fi
	logger -t "zerotier" "zt interface $zt0 is started!"
	del_rules
	iptables -A INPUT -i "$zt0" -j ACCEPT
	iptables -A FORWARD -i "$zt0" -o "$zt0" -j ACCEPT
	iptables -A FORWARD -i "$zt0" -j ACCEPT
	if [ "$nat_enable" = "1" ]; then
		iptables -t nat -A POSTROUTING -o "$zt0" -j MASQUERADE
		iptables -t nat -A POSTROUTING -s "$ip_segment" -j MASQUERADE
		zero_route "add"
	fi
	return 0
}


del_rules() {
	if [ -z "$zt0" ]; then
		zt0="$(zerotier_interface_name)"
	fi
	ip_segment="$(zerotier_kernel_route_segment "$zt0")"
#	iptables -D FORWARD -i $zt0 -j ACCEPT 2>/dev/null
#	iptables -D FORWARD -o $zt0 -j ACCEPT 2>/dev/null
#	iptables -D FORWARD -i $zt0 -o $zt0 -j ACCEPT
	iptables -D INPUT -i $zt0 -j ACCEPT 2>/dev/null
	iptables -D FORWARD -i $zt0 -o $zt0 -j ACCEPT 2>/dev/null
	iptables -D FORWARD -i $zt0 -j ACCEPT 2>/dev/null
	iptables -t nat -D POSTROUTING -o $zt0 -j MASQUERADE 2>/dev/null
	iptables -t nat -D POSTROUTING -s $ip_segment -j MASQUERADE 2>/dev/null
}

zero_route() {
	rulesnum=`nvram get zero_staticnum_x`
	for i in $(seq 1 $rulesnum)
	do
		j=`expr $i - 1`
		route_enable=`nvram get zero_enable_x$j`
		zero_ip=`nvram get zero_ip_x$j`
		zero_route=`nvram get zero_route_x$j`
		if [ "$1" = "add" ]; then
			if [ $route_enable -ne 0 ]; then
				ip route add $zero_ip via $zero_route dev $zt0
				echo "$zt0"
			fi
		else
			ip route del $zero_ip via $zero_route dev $zt0
		fi
	done
}

start_zero() {
	logger -t "zerotier" "正在启动zerotier"
	if ! kill_z; then
		logger -t "zerotier" "旧进程未能确认退出，取消启动"
		return 1
	fi
	start_instance 'zerotier'
}
zerotier_wait_stopped() {
	zerotier_wait=0
	while [ "$zerotier_wait" -lt 5 ]; do
		if ! pidof zerotier-one >/dev/null 2>&1; then
			return 0
		fi
		sleep 1
		zerotier_wait=$((zerotier_wait + 1))
	done
	if pidof zerotier-one >/dev/null 2>&1; then
		return 1
	fi
	return 0
}
kill_z() {
	if ! pidof zerotier-one >/dev/null 2>&1; then
		return 0
	fi
	logger -t "zerotier" "关闭进程..."
	killall zerotier-one >/dev/null 2>&1
	if zerotier_wait_stopped; then
		return 0
	fi

	logger -t "zerotier" "进程未响应TERM，尝试强制停止..."
	# Re-enumerate by process name; never signal a cached numeric PID.
	killall -9 zerotier-one >/dev/null 2>&1
	zerotier_wait_stopped
}
stop_zero() {
	# Keep the persistent identity/configuration and live network state intact
	# unless the daemon is confirmed absent.
	if ! kill_z; then
		logger -t "zerotier" "无法确认ZeroTier已停止，保留配置与网络规则"
		return 1
	fi
	del_rules
	zero_route "del"
	rm -rf "$config_path"
}

#创建moon节点
creat_moon() {
	moonip="$(nvram get zerotiermoon_ip)"
	logger -t "zerotier" "moonip $moonip"
	#检查是否合法ip
	regex="\b(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[1-9])\.(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])\.(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])\.(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[1-9])\b"
	ckStep2=`echo $moonip | egrep $regex | wc -l`

	logger -t "zerotier" "搭建ZeroTier的Moon中转服务器，生成moon配置文件"
	if [ -z "$moonip" ]; then
		#自动获取wanip
		ip_addr=`ifconfig -a ppp0 | grep 'inet addr:' | cut -d: -f2 | awk '{ print $1}'`
	#elif [ $ckStep2 -eq 0 ]; then
		#不是ip
	#	ip_addr = `curl $moonip`
	else
		ip_addr=$moonip
	fi
	logger -t "zerotier" "moonip $ip_addr"
	if [ -e $config_path/identity.public ]; then

		$PROGIDT initmoon $config_path/identity.public > $config_path/moon.json
		if `sed -i "s/\[\]/\[ \"$ip_addr\/9993\" \]/" $config_path/moon.json >/dev/null 2>/dev/null`; then
			logger -t "zerotier" "生成moon配置文件成功"
		else
			logger -t "zerotier" "生成moon配置文件失败"
		fi

		logger -t "zerotier" "生成签名文件"
		cd $config_path
		pwd
		$PROGIDT genmoon $config_path/moon.json
		[ $? -ne 0 ] && return 1
		logger -t "zerotier" "创建moons.d文件夹，并把签名文件移动到文件夹内"
		if [ ! -d "$config_path/moons.d" ]; then
			mkdir -p $config_path/moons.d
		fi
		
		#服务器加入moon server
		mv $config_path/*.moon $config_path/moons.d/ >/dev/null 2>&1
		logger -t "zerotier" "moon节点创建完成"

		zmoonid=`cat moon.json | awk -F "[id]" '/"id"/{print$0}'` >/dev/null 2>&1
		zmoonid=`echo $zmoonid | awk -F "[:]" '/"id"/{print$2}'` >/dev/null 2>&1
		zmoonid=`echo $zmoonid | tr -d '"|,'`

		nvram set zerotiermoon_id="$zmoonid"
		nvram commit
	else
		logger -t "zerotier" "identity.public不存在"
	fi
}

remove_moon() {
	zmoonid="$(nvram get zerotiermoon_id)"
	
	if [ ! -n "$zmoonid"]; then
		rm -f $config_path/moons.d/000000$zmoonid.moon
		rm -f $config_path/moon.json
		nvram set zerotiermoon_id=""
		nvram commit
	fi
}

case $1 in
start)
	start_zero
	;;
stop)
	stop_zero
	;;
*)
	echo "check"
	#exit 0
	;;
esac