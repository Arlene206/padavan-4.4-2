#!/bin/sh
#20200426 chongshengB
#20210410 xumng123
PROG=/usr/bin/zerotier-one
PROGCLI=/usr/bin/zerotier-cli
PROGIDT=/usr/bin/zerotier-idtool
config_path="/etc/storage/zerotier-one"
PLANET="/etc/storage/planet"
ZT_READY_TIMEOUT=30
zt0=""
ip_segment=""
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

# iptables 1.8.7 supports -C. Treat only exit status 1 as "absent";
# command/xtables failures must not be mistaken for completed cleanup.
ZEROTIER_RULE_DELETE_LIMIT=32
zerotier_delete_firewall_rule() {
	zerotier_rule_delete_count=0
	while [ "$zerotier_rule_delete_count" -lt "$ZEROTIER_RULE_DELETE_LIMIT" ]; do
		if iptables -C "$@" >/dev/null 2>&1; then
			if ! iptables -D "$@" >/dev/null 2>&1; then
				logger -t "zerotier" "防火墙规则删除失败: iptables -D $*"
				return 1
			fi
			zerotier_rule_delete_count=$((zerotier_rule_delete_count + 1))
		else
			zerotier_rule_check_status=$?
			case "$zerotier_rule_check_status" in
				1) return 0 ;;
				*)
					logger -t "zerotier" "防火墙规则状态检查失败: iptables -C $* (status=$zerotier_rule_check_status)"
					return 1
					;;
			esac
		fi
	done

	# Bound duplicate cleanup. Refuse to claim success if more copies remain.
	if iptables -C "$@" >/dev/null 2>&1; then
		logger -t "zerotier" "防火墙规则重复数量超过清理上限: $*"
		return 1
	else
		zerotier_rule_check_status=$?
		[ "$zerotier_rule_check_status" -eq 1 ] && return 0
		logger -t "zerotier" "防火墙规则最终状态检查失败: $* (status=$zerotier_rule_check_status)"
		return 1
	fi
}

zerotier_firewall_state_save() {
	[ -n "$zt0" ] && [ -d "$config_path" ] && [ ! -L "$config_path" ] || return 1
	case "$zt0" in
		zt*) ;;
		*) return 1 ;;
	esac
	case "$zt0" in
		*[!A-Za-z0-9_.:-]*) return 1 ;;
	esac
	case "$ip_segment" in
		*[!0-9./]*) return 1 ;;
	esac

	zt_state_file="$config_path/.firewall-state"
	if [ -e "$zt_state_file" ] || [ -L "$zt_state_file" ]; then
		[ -f "$zt_state_file" ] && [ ! -L "$zt_state_file" ] || return 1
	fi
	zt_state_tmp="$(mktemp "$config_path/.firewall-state.XXXXXX")" || return 1
	if ! printf '%s\n%s\n' "$zt0" "$ip_segment" > "$zt_state_tmp"; then
		rm -f "$zt_state_tmp"
		return 1
	fi
	if ! chmod 600 "$zt_state_tmp" || ! mv -f "$zt_state_tmp" "$zt_state_file"; then
		rm -f "$zt_state_tmp"
		return 1
	fi
	return 0
}

zerotier_firewall_state_load() {
	zt_state_file="$config_path/.firewall-state"
	[ -f "$zt_state_file" ] && [ ! -L "$zt_state_file" ] || return 1
	{
		IFS= read -r zt_state_saved_interface || return 1
		IFS= read -r zt_state_saved_segment || return 1
		if IFS= read -r zt_state_extra; then
			return 1
		fi
	} < "$zt_state_file"
	case "$zt_state_saved_interface" in
		zt*) ;;
		*) return 1 ;;
	esac
	case "$zt_state_saved_interface" in
		*[!A-Za-z0-9_.:-]*) return 1 ;;
	esac
	case "$zt_state_saved_segment" in
		*[!0-9./]*) return 1 ;;
	esac
	return 0
}

zerotier_add_firewall_rule() {
	if iptables "$@"; then
		return 0
	fi

	logger -t "zerotier" "防火墙规则安装失败: iptables $*"
	# Remove every rule from this transaction; the rejected append may follow
	# one or more successful appends, so cleanup must cover the whole set.
	if ! del_rules; then
		logger -t "zerotier" "防火墙规则安装失败后回滚不完整"
	fi
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
	if ! del_rules; then
		logger -t "zerotier" "旧防火墙规则清理未完成，拒绝发布新规则"
		return 1
	fi
	if ! zerotier_add_firewall_rule -A INPUT -i "$zt0" -j ACCEPT; then
		return 1
	fi
	if ! zerotier_add_firewall_rule -A FORWARD -i "$zt0" -o "$zt0" -j ACCEPT; then
		return 1
	fi
	if ! zerotier_add_firewall_rule -A FORWARD -i "$zt0" -j ACCEPT; then
		return 1
	fi
	if [ "$nat_enable" = "1" ]; then
		if ! zerotier_add_firewall_rule -t nat -A POSTROUTING -o "$zt0" -j MASQUERADE; then
			return 1
		fi
		if ! zerotier_add_firewall_rule -t nat -A POSTROUTING -s "$ip_segment" -j MASQUERADE; then
			return 1
		fi
		if ! zero_route "add"; then
			logger -t "zerotier" "ZeroTier静态路由安装失败，回滚网络规则"
			if ! del_rules; then
				logger -t "zerotier" "静态路由安装失败后防火墙回滚不完整"
			fi
			if ! zero_route "del"; then
				logger -t "zerotier" "静态路由回滚未完全确认"
			fi
			return 1
		fi
	fi
	return 0
}


del_rules() {
	del_rules_status=0
	if [ -z "$zt0" ] || [ -z "$ip_segment" ]; then
		if zerotier_firewall_state_load; then
			[ -n "$zt0" ] || zt0="$zt_state_saved_interface"
			[ -n "$ip_segment" ] || ip_segment="$zt_state_saved_segment"
		fi
	fi
	if [ -z "$zt0" ]; then
		logger -t "zerotier" "无法确定ZeroTier接口，跳过无目标的防火墙删除"
		return 0
	fi
	if [ -z "$ip_segment" ]; then
		ip_segment="$(zerotier_kernel_route_segment "$zt0")"
	fi

	if ! zerotier_delete_firewall_rule INPUT -i "$zt0" -j ACCEPT; then
		del_rules_status=1
	fi
	if ! zerotier_delete_firewall_rule FORWARD -i "$zt0" -o "$zt0" -j ACCEPT; then
		del_rules_status=1
	fi
	if ! zerotier_delete_firewall_rule FORWARD -i "$zt0" -j ACCEPT; then
		del_rules_status=1
	fi
	if ! zerotier_delete_firewall_rule -t nat POSTROUTING -o "$zt0" -j MASQUERADE; then
		del_rules_status=1
	fi
	if [ -n "$ip_segment" ]; then
		if ! zerotier_delete_firewall_rule -t nat POSTROUTING -s "$ip_segment" -j MASQUERADE; then
			del_rules_status=1
		fi
	fi
	return "$del_rules_status"
}


zero_route_entry_matches() {
	awk -v destination="$1" -v gateway="$2" -v interface="$3" '
		$1 == destination {
			via = ""
			dev = ""
			for (i = 2; i < NF; i++) {
				if ($i == "via")
					via = $(i + 1)
				else if ($i == "dev")
					dev = $(i + 1)
			}
			if (via == gateway && dev == interface)
				found = 1
		}
		END { exit found ? 0 : 1 }
	'
}

zero_route() {
	rulesnum="$(nvram get zero_staticnum_x)"
	case "$1" in
		add|del) ;;
		*) return 2 ;;
	esac
	case "$rulesnum" in
		""|0) return 0 ;;
		*[!0-9]*)
			logger -t "zerotier" "静态路由数量配置无效"
			return 1
			;;
	esac
	route_status=0
	for i in $(seq 1 "$rulesnum")
	do
		j=$((i - 1))
		route_enable="$(nvram get zero_enable_x$j)"
		zero_ip="$(nvram get zero_ip_x$j)"
		zero_route="$(nvram get zero_route_x$j)"
		case "$route_enable" in
			""|0) continue ;;
			*[!0-9]*)
				logger -t "zerotier" "静态路由启用标志无效: $j"
				if [ "$1" = "add" ]; then
					return 1
				fi
				route_status=1
				continue
				;;
		esac
		if [ "$route_enable" -eq 0 ]; then
			continue
		fi
		if [ -z "$zero_ip" ] || [ -z "$zero_route" ] || [ -z "$zt0" ]; then
			logger -t "zerotier" "静态路由配置不完整: $j"
			if [ "$1" = "add" ]; then
				return 1
			fi
			route_status=1
			continue
		fi
		if [ "$1" = "add" ]; then
			if ! ip route add "$zero_ip" via "$zero_route" dev "$zt0"; then
				logger -t "zerotier" "静态路由安装失败: $zero_ip via $zero_route"
				return 1
			fi
			echo "$zt0"
			continue
		fi

		# Treat an already-absent route as an idempotent success, but do not
		# discard inspection or deletion failures: stop must retain identity
		# until every configured route is confirmed absent.
		if ! route_output="$(ip route show "$zero_ip" 2>/dev/null)"; then
			logger -t "zerotier" "无法检查静态路由: $zero_ip"
			route_status=1
			continue
		fi
		if printf '%s\n' "$route_output" | zero_route_entry_matches "$zero_ip" "$zero_route" "$zt0"; then
			if ! ip route del "$zero_ip" via "$zero_route" dev "$zt0" >/dev/null 2>&1; then
				logger -t "zerotier" "静态路由删除失败: $zero_ip via $zero_route"
				route_status=1
				continue
			fi
			route_output="$(ip route show "$zero_ip" 2>/dev/null)" || {
				logger -t "zerotier" "无法确认静态路由已删除: $zero_ip"
				route_status=1
				continue
			}
			if printf '%s\n' "$route_output" | zero_route_entry_matches "$zero_ip" "$zero_route" "$zt0"; then
				logger -t "zerotier" "静态路由删除后仍然存在: $zero_ip via $zero_route"
				route_status=1
			else
				route_match_status=$?
				if [ "$route_match_status" -ne 1 ]; then
					logger -t "zerotier" "无法解析静态路由删除后的状态: $zero_ip"
					route_status=1
				fi
			fi
		else
			route_match_status=$?
			if [ "$route_match_status" -ne 1 ]; then
				logger -t "zerotier" "无法解析静态路由状态: $zero_ip"
				route_status=1
			fi
		fi
	done
	return "$route_status"
}

start_zero() {
	logger -t "zerotier" "正在启动zerotier"
	if ! kill_z; then
		logger -t "zerotier" "旧进程未能确认退出，取消启动"
		return 1
	fi
	start_instance 'zerotier'
}
# Return 0 when running, 1 only when pidof confirms absence, and 2 when
# process state cannot be trusted. A scan failure must never look like absence.
zerotier_process_state() {
	if zt_process_pids=$(pidof zerotier-one 2>/dev/null); then
		zt_process_pidof_status=0
	else
		zt_process_pidof_status=$?
	fi
	case "$zt_process_pidof_status" in
		0)
			[ -n "$zt_process_pids" ] || return 2
			case "$zt_process_pids" in
				*[!0-9\ ]*) return 2 ;;
			esac
			zt_process_pid_count=0
			for zt_process_pid in $zt_process_pids; do
				case "$zt_process_pid" in
					''|0*|1|*[!0-9]*) return 2 ;;
				esac
				[ "$zt_process_pid" -gt 1 ] 2>/dev/null || return 2
				zt_process_pid_count=$((zt_process_pid_count + 1))
			done
			[ "$zt_process_pid_count" -gt 0 ] || return 2
			return 0
			;;
		1)
			[ -z "$zt_process_pids" ] && return 1
			return 2
			;;
		*)
			return 2
			;;
	esac
}

zerotier_wait_stopped() {
	zerotier_wait=0
	while [ "$zerotier_wait" -lt 5 ]; do
		if zerotier_process_state; then
			zerotier_state=0
		else
			zerotier_state=$?
		fi
		case "$zerotier_state" in
			0)
				sleep 1
				zerotier_wait=$((zerotier_wait + 1))
				;;
			1)
				return 0
				;;
			*)
				return 2
				;;
		esac
	done
	if zerotier_process_state; then
		return 1
	else
		zerotier_state=$?
	fi
	case "$zerotier_state" in
		1) return 0 ;;
		2) return 2 ;;
		*) return 1 ;;
	esac
}

kill_z() {
	if zerotier_process_state; then
		zerotier_state=0
	else
		zerotier_state=$?
	fi
	case "$zerotier_state" in
		1) return 0 ;;
		2)
			logger -t "zerotier" "无法读取ZeroTier进程状态，拒绝发送信号"
			return 1
			;;
	esac

	logger -t "zerotier" "关闭进程..."
	killall zerotier-one >/dev/null 2>&1 || true
	if zerotier_wait_stopped; then
		return 0
	else
		zerotier_stop_state=$?
	fi
	if [ "$zerotier_stop_state" -ne 1 ]; then
		logger -t "zerotier" "停止期间进程状态变为未知，拒绝升级信号"
		return 1
	fi

	logger -t "zerotier" "进程未响应TERM，尝试强制停止..."
	# Re-enumerate by process name; never signal a cached numeric PID.
	killall -9 zerotier-one >/dev/null 2>&1 || true
	if zerotier_wait_stopped; then
		return 0
	else
		zerotier_stop_state=$?
	fi
	case "$zerotier_stop_state" in
		1)
			logger -t "zerotier" "ZeroTier守护进程在SIGKILL后仍运行"
			return 1
			;;
		*)
			logger -t "zerotier" "SIGKILL后无法确认ZeroTier进程状态"
			return 1
			;;
	esac
}
stop_zero() {
	# Capture the virtual interface while zerotier-one still owns it. The
	# interface can disappear as soon as the daemon exits, but both firewall
	# and static-route cleanup must use the original interface name.
	if [ -z "$zt0" ]; then
		zt0="$(zerotier_interface_name)"
	fi
	ip_segment="$(zerotier_kernel_route_segment "$zt0")"
	if [ -z "$zt0" ] || [ -z "$ip_segment" ]; then
		if zerotier_firewall_state_load; then
			[ -n "$zt0" ] || zt0="$zt_state_saved_interface"
			[ -n "$ip_segment" ] || ip_segment="$zt_state_saved_segment"
		fi
	fi
	if [ "$(nvram get zerotier_nat)" = "1" ] && [ -z "$ip_segment" ]; then
		logger -t "zerotier" "无法在停止前读取ZeroTier内核路由，保留守护进程与网络配置"
		return 1
	fi
	if [ -n "$zt0" ] && ! zerotier_firewall_state_save; then
		logger -t "zerotier" "无法安全保存防火墙清理状态，拒绝停止ZeroTier"
		return 1
	fi

	# Keep the persistent identity/configuration and live network state intact
	# unless the daemon is confirmed absent.
	if ! kill_z; then
		logger -t "zerotier" "无法确认ZeroTier已停止，保留配置与网络规则"
		return 1
	fi
	if ! del_rules; then
		logger -t "zerotier" "无法确认ZeroTier防火墙规则已清理，保留清理状态与持久配置"
		return 1
	fi
	if ! zero_route "del"; then
		logger -t "zerotier" "无法确认ZeroTier静态路由已清理，保留持久身份与配置"
		return 1
	fi
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