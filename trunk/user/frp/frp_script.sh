#!/bin/sh
#from hiboy
# FRP runtime configs contain the shared token and (for FRPS) dashboard
# credentials. Change all placeholders before enabling either service.
umask 077
frpc_enable=$(nvram get frpc_enable)
frps_enable=$(nvram get frps_enable)
frp_token='CHANGE_ME_TO_A_UNIQUE_RANDOM_TOKEN'
frp_dashboard_user='CHANGE_ME'
frp_dashboard_password='CHANGE_ME_TO_A_UNIQUE_PASSWORD'
frp_runtime_dir='/tmp/frp'
frp_client_tmp=
frp_server_tmp=

frp_validate_credentials()
{
	if [ "$frpc_enable" = "1" ] || [ "$frps_enable" = "1" ]; then
		case "$frp_token" in
			''|'CHANGE_ME_TO_A_UNIQUE_RANDOM_TOKEN'|*[!A-Za-z0-9_-]*)
				return 1
				;;
		esac
		[ "${#frp_token}" -ge 24 ] || return 1
	fi

	if [ "$frps_enable" = "1" ]; then
		case "$frp_dashboard_user" in
			''|CHANGE_ME|admin|root|*[!A-Za-z0-9_-]*)
				return 1
				;;
		esac
		[ "${#frp_dashboard_user}" -ge 4 ] || return 1
		case "$frp_dashboard_password" in
			''|CHANGE_ME_TO_A_UNIQUE_PASSWORD|admin|password|*[!A-Za-z0-9_.@-]*)
				return 1
				;;
		esac
		[ "${#frp_dashboard_password}" -ge 16 ] || return 1
	fi
	return 0
}

frp_prepare_runtime_dir()
{
	frp_runtime_dir="$1"
	frp_expected_owner="$2"

	[ -d /tmp ] && [ ! -L /tmp ] || return 1
	set -- $(ls -lnd /tmp 2>/dev/null) || return 1
	[ "$3" = "0" ] || return 1
	case "$1" in
		*t) ;;
		*) return 1 ;;
	esac

	if [ -L "$frp_runtime_dir" ] ||
	   { [ -e "$frp_runtime_dir" ] && [ ! -d "$frp_runtime_dir" ]; }; then
		return 1
	fi
	if [ ! -e "$frp_runtime_dir" ]; then
		mkdir "$frp_runtime_dir" || return 1
	fi
	[ -d "$frp_runtime_dir" ] && [ ! -L "$frp_runtime_dir" ] || return 1
	set -- $(ls -lnd "$frp_runtime_dir" 2>/dev/null) || return 1
	[ "$3" = "$frp_expected_owner" ] || return 1
	chmod 700 "$frp_runtime_dir" || return 1
	return 0
}

frp_validate_config_target()
{
	frp_config_target="$1"
	case "$frp_config_target" in
		"$frp_runtime_dir/myfrpc.ini"|"$frp_runtime_dir/myfrps.ini") ;;
		*) return 1 ;;
	esac
	if [ -L "$frp_config_target" ] ||
	   { [ -e "$frp_config_target" ] && [ ! -f "$frp_config_target" ]; }; then
		return 1
	fi
	return 0
}

frp_stage_client_config()
{
	frp_client_tmp=$(mktemp "$frp_runtime_dir/.myfrpc.ini.XXXXXX") || {
		frp_client_tmp=
		return 1
	}
	if ! cat > "$frp_client_tmp" <<-FRP_CLIENT_CONFIG
		# ==========客户端配置：==========
		[common]
		server_addr = 1192.0.0.3
		server_port = 7000
		token = $frp_token

		[web]
		remote_port = 6000
		type = http
		local_ip = 192.168.2.1
		local_port = 80
		subdomain = test
		# host_header_rewrite 可按实际内网域名配置
		# ====================
		FRP_CLIENT_CONFIG
	then
		rm -f "$frp_client_tmp"
		frp_client_tmp=
		return 1
	fi
	if ! chmod 0600 "$frp_client_tmp"; then
		rm -f "$frp_client_tmp"
		frp_client_tmp=
		return 1
	fi
	return 0
}

frp_stage_server_config()
{
	frp_server_tmp=$(mktemp "$frp_runtime_dir/.myfrps.ini.XXXXXX") || {
		frp_server_tmp=
		return 1
	}
	if ! cat > "$frp_server_tmp" <<-FRP_SERVER_CONFIG
		# ==========服务端配置：==========
		[common]
		bind_port = 7000
		dashboard_port = 7500
		dashboard_user = $frp_dashboard_user
		dashboard_pwd = $frp_dashboard_password
		vhost_http_port = 88
		token = $frp_token
		subdomain_host = frps.com
		max_pool_count = 50
		# ====================
		FRP_SERVER_CONFIG
	then
		rm -f "$frp_server_tmp"
		frp_server_tmp=
		return 1
	fi
	if ! chmod 0600 "$frp_server_tmp"; then
		rm -f "$frp_server_tmp"
		frp_server_tmp=
		return 1
	fi
	return 0
}

frp_publish_staged_config()
{
	frp_stage_path="$1"
	frp_config_target="$2"
	[ -n "$frp_stage_path" ] && [ -f "$frp_stage_path" ] &&
		[ ! -L "$frp_stage_path" ] || return 1
	frp_validate_config_target "$frp_config_target" || return 1
	mv -f "$frp_stage_path" "$frp_config_target" || return 1
	return 0
}

frp_cleanup_config_stages()
{
	[ -z "$frp_client_tmp" ] || rm -f "$frp_client_tmp"
	[ -z "$frp_server_tmp" ] || rm -f "$frp_server_tmp"
}

trap 'frp_cleanup_config_stages' 0
trap 'exit 1' HUP INT TERM

# A disabled FRP installation must not recreate configs or restart daemons.
if [ "$frpc_enable" != "1" ] && [ "$frps_enable" != "1" ]; then
	killall frpc frps
	exit 0
fi

if ! frp_validate_credentials; then
	logger -t "frp" "FRP令牌或Dashboard凭据仍为示例值或不符合安全格式，拒绝启动。"
	exit 1
fi
if ! frp_prepare_runtime_dir "$frp_runtime_dir" 0; then
	logger -t "frp" "FRP运行目录不是root拥有的安全目录，拒绝写入配置。"
	exit 1
fi

# Validate every destination and stage every enabled config before replacing
# any live config or stopping a currently running daemon.
if [ "$frpc_enable" = "1" ]; then
	frp_validate_config_target "$frp_runtime_dir/myfrpc.ini" || {
		logger -t "frp" "FRPC配置目标不是安全的普通文件路径。"
		exit 1
	}
	frp_stage_client_config || {
		logger -t "frp" "FRPC配置暂存失败，保留现有配置和运行中的服务。"
		exit 1
	}
fi
if [ "$frps_enable" = "1" ]; then
	frp_validate_config_target "$frp_runtime_dir/myfrps.ini" || {
		logger -t "frp" "FRPS配置目标不是安全的普通文件路径。"
		exit 1
	}
	frp_stage_server_config || {
		logger -t "frp" "FRPS配置暂存失败，保留现有配置和运行中的服务。"
		exit 1
	}
fi

if [ "$frpc_enable" = "1" ]; then
	frp_publish_staged_config "$frp_client_tmp" "$frp_runtime_dir/myfrpc.ini" || {
		logger -t "frp" "FRPC配置原子发布失败，拒绝重启服务。"
		exit 1
	}
	frp_client_tmp=
fi
if [ "$frps_enable" = "1" ]; then
	frp_publish_staged_config "$frp_server_tmp" "$frp_runtime_dir/myfrps.ini" || {
		logger -t "frp" "FRPS配置原子发布失败，拒绝重启服务。"
		exit 1
	}
	frp_server_tmp=
fi

# Only restart daemons after all enabled configs have been staged and published.
killall frpc frps
if [ "$frpc_enable" = "1" ]; then
	frpc -c "$frp_runtime_dir/myfrpc.ini" 2>&1 &
fi
if [ "$frps_enable" = "1" ]; then
	frps -c "$frp_runtime_dir/myfrps.ini" 2>&1 &
fi
