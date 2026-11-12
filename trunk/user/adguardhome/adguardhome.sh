#!/bin/sh

change_dns() {
  if [ "$(nvram get adg_redirect)" = 1 ]; then
    sed -i '/no-resolv/d' /etc/storage/dnsmasq/dnsmasq.conf
    sed -i '/server=127.0.0.1/d' /etc/storage/dnsmasq/dnsmasq.conf
    cat >> /etc/storage/dnsmasq/dnsmasq.conf << EOF
no-resolv
server=127.0.0.1#5335
EOF
    /sbin/restart_dhcpd
    logger -t "AdGuardHome" "添加DNS转发到5335端口"
  fi
}

del_dns() {
  sed -i '/no-resolv/d' /etc/storage/dnsmasq/dnsmasq.conf
  sed -i '/server=127.0.0.1#5335/d' /etc/storage/dnsmasq/dnsmasq.conf
  /sbin/restart_dhcpd
}

adg_remove_redirect_rule()
{
	adg_iptables="$1"
	shift

	# Remove every exact copy; repeated start/stop must be idempotent.
	while "$adg_iptables" -t nat -C PREROUTING "$@" >/dev/null 2>&1; do
		if ! "$adg_iptables" -t nat -D PREROUTING "$@" >/dev/null 2>&1; then
			return 1
		fi
	done
	return 0
}

set_iptable()
{
	# Route-derived LOCAL matching follows DHCP/PPPoE and IPv6 address changes;
	# it does not pin rules to a snapshot of interface addresses.
	if ! clear_iptable; then
		logger -t "AdGuardHome" "清理旧DNS重定向规则失败"
		return 1
	fi

	if [ "$(nvram get adg_redirect)" = 2 ]; then
		if ! iptables -t nat -A PREROUTING -p tcp -m addrtype --dst-type LOCAL ! -d 127.0.0.0/8 --dport 53 -j REDIRECT --to-ports 5335 >/dev/null 2>&1; then
			logger -t "AdGuardHome" "添加IPv4 TCP DNS重定向失败"
			return 1
		fi
		if ! iptables -t nat -A PREROUTING -p udp -m addrtype --dst-type LOCAL ! -d 127.0.0.0/8 --dport 53 -j REDIRECT --to-ports 5335 >/dev/null 2>&1; then
			logger -t "AdGuardHome" "添加IPv4 UDP DNS重定向失败"
			clear_iptable
			return 1
		fi

		# ip6tables is not installed on IPv6-disabled firmware profiles.
		if ip6tables -t nat -L PREROUTING -n >/dev/null 2>&1; then
			if ! ip6tables -t nat -A PREROUTING -p tcp -m addrtype --dst-type LOCAL ! -d ::1/128 ! -d fe80::/10 --dport 53 -j REDIRECT --to-ports 5335 >/dev/null 2>&1; then
				logger -t "AdGuardHome" "添加IPv6 TCP DNS重定向失败"
				clear_iptable
				return 1
			fi
			if ! ip6tables -t nat -A PREROUTING -p udp -m addrtype --dst-type LOCAL ! -d ::1/128 ! -d fe80::/10 --dport 53 -j REDIRECT --to-ports 5335 >/dev/null 2>&1; then
				logger -t "AdGuardHome" "添加IPv6 UDP DNS重定向失败"
				clear_iptable
				return 1
			fi
		fi
		logger -t "AdGuardHome" "重定向53端口"
	fi
	return 0
}

clear_iptable()
{
	adg_cleanup_status=0

	adg_remove_redirect_rule iptables -p tcp -m addrtype --dst-type LOCAL ! -d 127.0.0.0/8 --dport 53 -j REDIRECT --to-ports 5335 || adg_cleanup_status=1
	adg_remove_redirect_rule iptables -p udp -m addrtype --dst-type LOCAL ! -d 127.0.0.0/8 --dport 53 -j REDIRECT --to-ports 5335 || adg_cleanup_status=1

	# Skip the IPv6 family when its nat table is not present in this firmware.
	if ip6tables -t nat -L PREROUTING -n >/dev/null 2>&1; then
		adg_remove_redirect_rule ip6tables -p tcp -m addrtype --dst-type LOCAL ! -d ::1/128 ! -d fe80::/10 --dport 53 -j REDIRECT --to-ports 5335 || adg_cleanup_status=1
		adg_remove_redirect_rule ip6tables -p udp -m addrtype --dst-type LOCAL ! -d ::1/128 ! -d fe80::/10 --dport 53 -j REDIRECT --to-ports 5335 || adg_cleanup_status=1
	fi

	return "$adg_cleanup_status"
}

getconfig(){
  adg_file="${ADG_CONFIG_FILE:-/etc/storage/adg.sh}"

  # This YAML contains the web UI password and may contain a TLS private key.
  # Never follow a link or write credentials into a non-regular destination.
  if [ -L "$adg_file" ]; then
    logger -t "AdGuardHome" "配置文件是符号链接，拒绝启动"
    return 1
  fi
  if [ -e "$adg_file" ] && [ ! -f "$adg_file" ]; then
    logger -t "AdGuardHome" "配置文件不是普通文件，拒绝启动"
    return 1
  fi

  if [ ! -f "$adg_file" ] || [ ! -s "$adg_file" ]; then
    # Publish a complete default file from a private same-directory stage.
    adg_tmp="$(mktemp "${adg_file}.tmp.XXXXXX")" || {
      logger -t "AdGuardHome" "无法创建私有配置暂存文件"
      return 1
    }
    if ! (
      umask 077
      cat > "$adg_tmp" <<-\EEE
bind_host: 0.0.0.0
bind_port: 3030
auth_name: adguardhome
auth_pass: adguardhome
language: zh-cn
rlimit_nofile: 0
dns:
  bind_host: 0.0.0.0
  port: 5335
  protection_enabled: true
  filtering_enabled: true
  blocking_mode: nxdomain
  blocked_response_ttl: 10
  querylog_enabled: true
  ratelimit: 20
  ratelimit_whitelist: []
  refuse_any: true
  bootstrap_dns:
  - 223.5.5.5
  all_servers: true
  allowed_clients: []
  disallowed_clients: []
  blocked_hosts: []
  parental_sensitivity: 0
  parental_enabled: false
  safesearch_enabled: false
  safebrowsing_enabled: false
  resolveraddress: ""
  upstream_dns:
  - 223.5.5.5
tls:
  enabled: false
  server_name: ""
  force_https: false
  port_https: 443
  port_dns_over_tls: 853
  certificate_chain: ""
  private_key: ""
filters:
- enabled: true
  url: https://adguardteam.github.io/AdGuardSDNSFilter/Filters/filter.txt
  name: AdGuard Simplified Domain Names filter
  id: 1
- enabled: true
  url: https://adaway.org/hosts.txt
  name: AdAway
  id: 2
user_rules: []
dhcp:
  enabled: false
  interface_name: ""
  gateway_ip: ""
  subnet_mask: ""
  range_start: ""
  range_end: ""
  lease_duration: 86400
  icmp_timeout_msec: 1000
log_file: ""
verbose: false
schema_version: 3

EEE
    ); then
      rm -f "$adg_tmp"
      logger -t "AdGuardHome" "生成默认配置失败"
      return 1
    fi
    if ! chmod 600 "$adg_tmp"; then
      rm -f "$adg_tmp"
      logger -t "AdGuardHome" "无法设置默认配置权限"
      return 1
    fi
    if ! mv -f "$adg_tmp" "$adg_file"; then
      rm -f "$adg_tmp"
      logger -t "AdGuardHome" "无法发布默认配置"
      return 1
    fi
  fi

  # Existing configurations can also contain secrets; repair permissive modes
  # on every start and fail closed if confidentiality cannot be enforced.
  if ! chmod 600 "$adg_file"; then
    logger -t "AdGuardHome" "无法保护配置文件权限，拒绝启动"
    return 1
  fi
  return 0
}

start_adg(){
  # Avoid duplicate daemon instances and repeated DNS/firewall mutations.
  if pidof AdGuardHome >/dev/null 2>&1; then
    logger -t "AdGuardHome" "AdGuardHome 已在运行，跳过重复启动"
    return 0
  fi

  # Do not alter resolver/firewall state until the credential-bearing config
  # has been validated, privately published and made owner-readable only.
  if ! getconfig; then
    logger -t "AdGuardHome" "配置准备失败，取消启动"
    return 1
  fi
  mkdir -p /tmp/AdGuardHome
	mkdir -p /etc/storage/AdGuardHome
	change_dns
	set_iptable
	logger -t "AdGuardHome" "启动 AdGuardHome"
	eval "AdGuardHome -c $adg_file -w /tmp/AdGuardHome -v" &
}

adg_wait_stopped() {
  adg_wait=0
  while [ "$adg_wait" -lt 5 ]; do
    if ! pidof AdGuardHome >/dev/null 2>&1; then
      return 0
    fi
    sleep 1
    adg_wait=$((adg_wait + 1))
  done
  if pidof AdGuardHome >/dev/null 2>&1; then
    return 1
  fi
  return 0
}

adg_stop_daemon() {
  if ! pidof AdGuardHome >/dev/null 2>&1; then
    return 0
  fi

  killall AdGuardHome >/dev/null 2>&1
  if adg_wait_stopped; then
    return 0
  fi

  logger -t "AdGuardHome" "正常停止超时，尝试强制停止"
  # Re-enumerate by process name; do not signal a stale numeric PID snapshot.
  killall -9 AdGuardHome >/dev/null 2>&1
  adg_wait_stopped
}

stop_adg(){
  logger -t "AdGuardHome" "停止 AdGuardHome"
  if ! adg_stop_daemon; then
    logger -t "AdGuardHome" "进程仍在运行，保留工作目录和 DNS/防火墙状态"
    return 1
  fi

  # Runtime files and traffic redirection are removed only after process exit.
  rm -rf /tmp/AdGuardHome
  del_dns
  clear_iptable
}

case $1 in
start)
	start_adg
	;;
stop)
	stop_adg
	;;
*)
	echo "check"
	;;
esac
