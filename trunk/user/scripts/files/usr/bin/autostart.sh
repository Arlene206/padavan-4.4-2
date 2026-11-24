#!/bin/sh
#
# Copyright (C) 2022 TurBoTse <860018505@qq.com>
#
#nvram set ntp_ready=0

mkdir -p /tmp/dnsmasq.dom
logger -t "防止 DNSmasq 启动失败，创建/tmp/dnsmasq.dom/"

# Publish packaged defaults without following persistent symlinks. These
# paths are restored from user-controlled storage archives, so a dangling link
# must not redirect a privileged boot-time copy outside /etc/storage.
autostart_seed_storage_script()
{
	_autostart_source=$1
	_autostart_destination=$2
	_autostart_tmp=

	case "$_autostart_destination" in
		/etc/storage/*) ;;
		*)
			logger -t "自动启动" "Refusing to seed script outside /etc/storage: $_autostart_destination"
			return 1
			;;
	esac
	if [ -L /etc/storage ] || [ ! -d /etc/storage ]; then
		logger -t "自动启动" "Refusing to seed script through an invalid storage directory"
		return 1
	fi
	if [ -L "$_autostart_destination" ]; then
		logger -t "自动启动" "Refusing to follow persistent script symlink: $_autostart_destination"
		return 1
	fi
	_autostart_destination_exists=0
	if [ -e "$_autostart_destination" ]; then
		if [ ! -f "$_autostart_destination" ]; then
			logger -t "自动启动" "Refusing to replace non-regular persistent script: $_autostart_destination"
			return 1
		fi
		if [ -x "$_autostart_destination" ]; then
			return 0
		fi
		_autostart_destination_exists=1
	fi
	if [ ! -f "$_autostart_source" ] || [ -L "$_autostart_source" ] || [ ! -r "$_autostart_source" ]; then
		logger -t "自动启动" "Packaged script source is unavailable: $_autostart_source"
		return 1
	fi

	_autostart_tmp=$(mktemp "${_autostart_destination}.tmp.XXXXXX") || {
		logger -t "自动启动" "Cannot allocate temporary script beside $_autostart_destination"
		return 1
	}
	if ! cat "$_autostart_source" > "$_autostart_tmp" ||
	   ! chmod 755 "$_autostart_tmp"; then
		rm -f "$_autostart_tmp"
		logger -t "自动启动" "Cannot prepare packaged script for $_autostart_destination"
		return 1
	fi
	if [ "$_autostart_destination_exists" = "1" ]; then
		# Repair a pre-existing regular but non-executable default atomically.
		if ! mv -f "$_autostart_tmp" "$_autostart_destination"; then
			rm -f "$_autostart_tmp"
			logger -t "自动启动" "Cannot replace non-executable persistent script: $_autostart_destination"
			return 1
		fi
	else
		# A same-directory hard link publishes without replacing a destination
		# created concurrently after the initial no-follow checks.
		if ! ln "$_autostart_tmp" "$_autostart_destination"; then
			if [ -L "$_autostart_destination" ] ||
			   [ ! -f "$_autostart_destination" ]; then
				rm -f "$_autostart_tmp"
				logger -t "自动启动" "Cannot publish packaged script: $_autostart_destination"
				return 1
			fi
			# A concurrently published regular file wins; preserve its contents.
			logger -t "自动启动" "Persistent script appeared during publication; preserving it: $_autostart_destination"
		fi
		if ! rm -f "$_autostart_tmp"; then
			logger -t "自动启动" "Cannot remove temporary packaged script: $_autostart_tmp"
			return 1
		fi
	fi
	return 0
}

# ap relay monitor [simonchen]
if ! autostart_seed_storage_script /usr/bin/sh_ezscript.sh /etc/storage/sh_ezscript.sh; then
	logger -t "自动启动" "Skipping sh_ezscript default publication"
fi
if [ -L /etc/storage/ap_script.sh ]; then
	logger -t "自动启动" "Skipping symlinked persistent ap_script.sh"
elif [ -x /etc/storage/ap_script.sh ]; then
	# Replace only the byte-identical pre-hardening stock copy; never clobber
	# a user-customized persistent script.
	if [ -x /usr/bin/ap_script_migrate.sh ]; then
		/usr/bin/ap_script_migrate.sh /etc/storage/ap_script.sh /usr/bin/ap_script.sh
	fi
	/etc/storage/ap_script.sh >/dev/null 2>&1 &
else
	if ! autostart_seed_storage_script /usr/bin/ap_script.sh /etc/storage/ap_script.sh; then
		logger -t "自动启动" "Skipping ap_script default publication"
	fi
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

autostart_nvram_read()
{
	_autostart_nvram_key=$1
	if _autostart_nvram_value=$(nvram get "$_autostart_nvram_key"); then
		return 0
	else
		_autostart_nvram_status=$?
	fi
	logger -t "自动启动" "NVRAM read failed for $_autostart_nvram_key (status=$_autostart_nvram_status); dependent service remains disabled"
	[ "$_autostart_nvram_status" -ne 0 ] || _autostart_nvram_status=1
	return "$_autostart_nvram_status"
}

autostart_nvram_enabled()
{
	autostart_nvram_read "$1" || return $?
	[ "$_autostart_nvram_value" = "1" ]
}

if autostart_nvram_enabled pppoemwan_enable; then
sleep 20
fi

if autostart_nvram_enabled sqm_enable; then
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

if autostart_nvram_enabled adbyby_enable; then
logger -t "自动启动" "正在启动 Adbyby plus..."
run_optional_start "Adbyby" /usr/bin/adbyby.sh start
fi

if autostart_nvram_enabled adg_enable; then
logger -t "自动启动" "正在启动 adguardhome..."
run_optional_start "AdGuardHome" /usr/bin/adguardhome.sh start
fi

if autostart_nvram_enabled ss_enable; then
logger -t "自动启动" "正在启动科学上网..."
run_optional_start "Shadowsocks" /usr/bin/shadowsocks.sh start
fi

if autostart_nvram_enabled sdns_enable; then
   if [ -f "$smartdns_conf" ] ; then
       sed -i '/去广告/d' "$smartdns_conf"
       sed -i '/adbyby/d' "$smartdns_conf"
   fi
logger -t "自动启动" "正在启动 SmartDNS..."
run_optional_start "SmartDNS" /usr/bin/smartdns.sh start
fi

if autostart_nvram_enabled aliddns_enable; then
	if [ -r /etc/ssl/certs/ca-certificates.crt ]; then
		logger -t "自动启动" "正在启动阿里 ddns..."
		run_optional_start "AliDDNS" /usr/bin/aliddns.sh start
	else
		logger -t "自动启动" "AliDDNS autostart skipped: CA trust bundle is not readable"
	fi
fi

if autostart_nvram_enabled ddnsto_enable; then
logger -t "自动启动" "正在启动 ddnsto..."
run_optional_start "DDNSTO" /usr/bin/ddnsto.sh start
fi

if autostart_nvram_enabled aliyundrive_enable; then
logger -t "自动启动" "正在启动阿里云盘..."
run_optional_start "AliyunDrive WebDAV" /usr/bin/aliyundrive-webdav.sh start
fi

if autostart_nvram_enabled zerotier_enable; then
logger -t "自动启动" "正在启动 zerotier..."
run_optional_start "ZeroTier" /usr/bin/zerotier.sh start
fi

if autostart_nvram_enabled wireguard_enable; then
logger -t "自动启动" "正在启动 wireguard..."
run_optional_start "WireGuard" /usr/bin/wireguard.sh start
fi

frp_launch()
{
	/usr/bin/frp.sh start
}

frp_autostart()
{
	autostart_nvram_read frpc_enable || return $?
	frp_client_enabled=$_autostart_nvram_value
	autostart_nvram_read frps_enable || return $?
	frp_server_enabled=$_autostart_nvram_value
	if [ "$frp_client_enabled" = "1" ] || [ "$frp_server_enabled" = "1" ]; then
		logger -t "自动启动" "正在启动 FRP..."
		frp_launch
		return $?
	fi
	return 0
}

run_optional_start "FRP" frp_autostart
