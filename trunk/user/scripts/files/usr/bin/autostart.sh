#!/bin/sh
#
# Copyright (C) 2022 TurBoTse <860018505@qq.com>
#
#nvram set ntp_ready=0

mkdir -p /tmp/dnsmasq.dom
logger -t "防止 DNSmasq 启动失败，创建/tmp/dnsmasq.dom/"

# ap relay monitor [simonchen]
if [ ! -x /etc/storage/sh_ezscript.sh ]; then
	cp /usr/bin/sh_ezscript.sh /etc/storage/sh_ezscript.sh
fi
if [ -x /etc/storage/ap_script.sh ]; then
	# Replace only the byte-identical pre-hardening stock copy; never clobber
	# a user-customized persistent script.
	if [ -x /usr/bin/ap_script_migrate.sh ]; then
		/usr/bin/ap_script_migrate.sh /etc/storage/ap_script.sh /usr/bin/ap_script.sh
	fi
	/etc/storage/ap_script.sh >/dev/null 2>&1 &
else
	cp /usr/bin/ap_script.sh /etc/storage/ap_script.sh
fi
smartdns_conf="/etc/storage/smartdns_custom.conf"

logger -t "自动启动" "正在检查路由是否已连接互联网..."
count=0
while :
do
	ping -c 1 -W 1 -q 223.5.5.5 1>/dev/null 2>&1
	if [ "$?" = "0" ]; then
		break
	fi
	sleep 5
	ping -c 1 -W 1 -q baidu.com 1>/dev/null 2>&1
	if [ "$?" = "0" ]; then
		break
	fi
	sleep 5
	count=$((count+1))
	if [ $count -gt 18 ]; then
		break
	fi
done

if [ $(nvram get pppoemwan_enable) = 1 ] ; then
sleep 20
fi

if [ $(nvram get sqm_enable) = 1 ] ; then
sleep 10
logger -t "自动启动" "正在启动 SQM QOS..."
/usr/lib/sqm/run.sh
fi

run_optional_start()
{
	_autostart_label=$1
	shift
	"$@"
	_autostart_status=$?
	if [ "$_autostart_status" -ne 0 ]; then
		logger -t "自动启动" "$_autostart_label autostart failed (status=$_autostart_status)"
	fi
	# One optional add-on must not abort the independent boot launchers.
	return 0
}

if [ $(nvram get adbyby_enable) = 1 ] ; then
logger -t "自动启动" "正在启动 Adbyby plus..."
run_optional_start "Adbyby" /usr/bin/adbyby.sh start
fi

if [ $(nvram get adg_enable) = 1 ] ; then
logger -t "自动启动" "正在启动 adguardhome..."
run_optional_start "AdGuardHome" /usr/bin/adguardhome.sh start
fi

if [ $(nvram get ss_enable) = 1 ] ; then
logger -t "自动启动" "正在启动科学上网..."
run_optional_start "Shadowsocks" /usr/bin/shadowsocks.sh start
fi

if [ $(nvram get sdns_enable) = 1 ] ; then
   if [ -f "$smartdns_conf" ] ; then
       sed -i '/去广告/d' "$smartdns_conf"
       sed -i '/adbyby/d' "$smartdns_conf"
   fi
logger -t "自动启动" "正在启动 SmartDNS..."
run_optional_start "SmartDNS" /usr/bin/smartdns.sh start
fi

if [ "$(nvram get aliddns_enable)" = 1 ]; then
	if [ -r /etc/ssl/certs/ca-certificates.crt ]; then
		logger -t "自动启动" "正在启动阿里 ddns..."
		run_optional_start "AliDDNS" /usr/bin/aliddns.sh start
	else
		logger -t "自动启动" "AliDDNS autostart skipped: CA trust bundle is not readable"
	fi
fi

if [ $(nvram get ddnsto_enable) = 1 ] ; then
logger -t "自动启动" "正在启动 ddnsto..."
run_optional_start "DDNSTO" /usr/bin/ddnsto.sh start
fi

if [ $(nvram get aliyundrive_enable) = 1 ] ; then
logger -t "自动启动" "正在启动阿里云盘..."
run_optional_start "AliyunDrive WebDAV" /usr/bin/aliyundrive-webdav.sh start
fi

if [ $(nvram get zerotier_enable) = 1 ] ; then
logger -t "自动启动" "正在启动 zerotier..."
run_optional_start "ZeroTier" /usr/bin/zerotier.sh start
fi

if [ $(nvram get wireguard_enable) = 1 ] ; then
logger -t "自动启动" "正在启动 wireguard..."
run_optional_start "WireGuard" /usr/bin/wireguard.sh start
fi

frp_launch()
{
	/usr/bin/frp.sh start
}

frp_autostart()
{
	frp_client_enabled=$(nvram get frpc_enable) || return 1
	frp_server_enabled=$(nvram get frps_enable) || return 1
	if [ "$frp_client_enabled" = "1" ] || [ "$frp_server_enabled" = "1" ]; then
		logger -t "自动启动" "正在启动 FRP..."
		frp_launch
		return $?
	fi
	return 0
}

run_optional_start "FRP" frp_autostart
