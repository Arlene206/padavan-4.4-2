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

set_iptable()
{
  if [ "$(nvram get adg_redirect)" = 2 ]; then
    IPS="`ifconfig | grep "inet addr" | grep -v ":127" | grep "Bcast" | awk '{print $2}' | awk -F : '{print $2}'`"
    for IP in $IPS
    do
      iptables -t nat -A PREROUTING -p tcp -d $IP --dport 53 -j REDIRECT --to-ports 5335 >/dev/null 2>&1
      iptables -t nat -A PREROUTING -p udp -d $IP --dport 53 -j REDIRECT --to-ports 5335 >/dev/null 2>&1
    done

    IPS="`ifconfig | grep "inet6 addr" | grep -v " fe80::" | grep -v " ::1" | grep "Global" | awk '{print $3}'`"
    for IP in $IPS
    do
      ip6tables -t nat -A PREROUTING -p tcp -d $IP --dport 53 -j REDIRECT --to-ports 5335 >/dev/null 2>&1
      ip6tables -t nat -A PREROUTING -p udp -d $IP --dport 53 -j REDIRECT --to-ports 5335 >/dev/null 2>&1
    done
      logger -t "AdGuardHome" "重定向53端口"
  fi
}

clear_iptable()
{
	OLD_PORT="5335"
	IPS="`ifconfig | grep "inet addr" | grep -v ":127" | grep "Bcast" | awk '{print $2}' | awk -F : '{print $2}'`"
	for IP in $IPS
	do
		iptables -t nat -D PREROUTING -p udp -d $IP --dport 53 -j REDIRECT --to-ports $OLD_PORT >/dev/null 2>&1
		iptables -t nat -D PREROUTING -p tcp -d $IP --dport 53 -j REDIRECT --to-ports $OLD_PORT >/dev/null 2>&1
	done

	IPS="`ifconfig | grep "inet6 addr" | grep -v " fe80::" | grep -v " ::1" | grep "Global" | awk '{print $3}'`"
	for IP in $IPS
	do
		ip6tables -t nat -D PREROUTING -p udp -d $IP --dport 53 -j REDIRECT --to-ports $OLD_PORT >/dev/null 2>&1
		ip6tables -t nat -D PREROUTING -p tcp -d $IP --dport 53 -j REDIRECT --to-ports $OLD_PORT >/dev/null 2>&1
	done
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

stop_adg(){
  rm -rf /tmp/AdGuardHome
  logger -t "AdGuardHome" "停止 AdGuardHome"
  killall -9 AdGuardHome
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
