#!/bin/sh
frpc_enable=`nvram get frpc_enable`
frps_enable=`nvram get frps_enable`
http_username=`nvram get http_username`
FRP_CRON_ROOT="/etc/storage/cron"
FRP_CRONTAB_DIR="$FRP_CRON_ROOT/crontabs"
FRP_WATCHDOG_CRON='*/1 * * * * /bin/sh /usr/bin/frp.sh C >/dev/null 2>&1'

check_frp () 
{
	check_net
	result_net=$?
	if [ "$result_net" = "1" ]; then
		# frp_script.sh restarts both enabled daemons. Decide once after checking
		# every enabled service: background startup may not expose either PID yet.
		need_start=0
		if [ "$frpc_enable" = "1" ] && [ -z "$(pidof frpc)" ]; then
			need_start=1
		fi
		if [ "$frps_enable" = "1" ] && [ -z "$(pidof frps)" ]; then
			need_start=1
		fi
		if [ "$need_start" = "1" ]; then
			frp_start
		fi
	fi
	return 0
}

check_net() 
{
	if /bin/ping -c 3 223.5.5.5 -w 5 >/dev/null 2>&1; then
		return 1
	fi
	logger -t "frp" "检测到互联网未能成功访问,稍后再尝试启动frp"
	return 2
}

frp_run_script()
{
	frp_script_path="${FRP_SCRIPT_PATH:-/etc/storage/frp_script.sh}"
	if [ -L "$frp_script_path" ] || [ ! -f "$frp_script_path" ]; then
		logger -t "frp" "拒绝执行缺失或非普通文件形式的FRP配置脚本。"
		return 1
	fi
	/bin/sh "$frp_script_path"
}

# Add/remove only FRP's exact watchdog entry. Stage outside the crontabs scan
# directory, preserve every operator entry, and atomically replace one file.
frp_manage_watchdog_cron()
{
	(
		frp_cron_action="$1"
		case "$frp_cron_action" in
			add|remove) ;;
			*) logger -t "frp" "拒绝未知的crontab操作。"; exit 1 ;;
		esac

		case "$http_username" in
			''|.*|*[!A-Za-z0-9_.-]*)
				logger -t "frp" "http_username不是安全的crontab文件名，拒绝修改。"
				exit 1
				;;
		esac
		[ "${#http_username}" -le 32 ] || {
			logger -t "frp" "http_username超过crontab文件名长度上限。"
			exit 1
		}

		[ "$FRP_CRONTAB_DIR" = "$FRP_CRON_ROOT/crontabs" ] || exit 1
		frp_cron_root_parent="${FRP_CRON_ROOT%/*}"
		[ -d "$frp_cron_root_parent" ] && [ ! -L "$frp_cron_root_parent" ] || exit 1
		[ -d "$FRP_CRON_ROOT" ] && [ ! -L "$FRP_CRON_ROOT" ] || exit 1
		[ -d "$FRP_CRONTAB_DIR" ] && [ ! -L "$FRP_CRONTAB_DIR" ] || exit 1
		frp_cron_file="$FRP_CRONTAB_DIR/$http_username"
		if [ -L "$frp_cron_file" ] ||
		   { [ -e "$frp_cron_file" ] && [ ! -f "$frp_cron_file" ]; }; then
			logger -t "frp" "拒绝修改非普通文件或符号链接crontab。"
			exit 1
		fi
		if [ "$frp_cron_action" = "remove" ] &&
		   [ ! -e "$frp_cron_file" ] && [ ! -L "$frp_cron_file" ]; then
			exit 0
		fi

		umask 077
		frp_cron_tmp=$(mktemp "$FRP_CRON_ROOT/.frp-crontab.XXXXXX") || exit 1
		trap 'rm -f "$frp_cron_tmp"' 0
		trap 'exit 1' HUP INT TERM
		if [ -f "$frp_cron_file" ]; then
			awk -v managed="$FRP_WATCHDOG_CRON" '$0 != managed { print }' \
				"$frp_cron_file" > "$frp_cron_tmp" || exit 1
		else
			: > "$frp_cron_tmp" || exit 1
		fi
		if [ "$frp_cron_action" = "add" ]; then
			printf '%s\n' "$FRP_WATCHDOG_CRON" >> "$frp_cron_tmp" || exit 1
		fi
		chmod 0600 "$frp_cron_tmp" || exit 1
		mv -f "$frp_cron_tmp" "$frp_cron_file" || {
			logger -t "frp" "原子更新FRP crontab失败。"
			exit 1
		}
		trap - 0 HUP INT TERM
		exit 0
	)
}

frp_start () 
{
	if [ "$frpc_enable" = "0" ] && [ "$frps_enable" = "0" ]; then
		# RC/UI restart still calls start when both roles are disabled.
		# Reuse stop cleanup and do not publish an idle watchdog or run user code.
		frp_close
		return $?
	fi
	if ! frp_run_script; then
		logger -t "frp" "FRP启动脚本执行失败，保留现有crontab。"
		return 1
	fi
	if ! frp_manage_watchdog_cron add; then
		logger -t "frp" "FRP watchdog任务未能安全写入crontab。"
		return 1
	fi
	[ ! -z "\`pidof frpc\`" ] && logger -t "frp" "frpc启动成功"
	[ ! -z "\`pidof frps\`" ] && logger -t "frp" "frps启动成功"
	return 0
}

frp_close () 
{
	if [ "$frpc_enable" = "0" ]; then
		if [ ! -z "`pidof frpc`" ]; then
		killall -9 frpc frp_script.sh
		[ -z "`pidof frpc`" ] && logger -t "frp" "已停止 frpc"
	    fi
	fi
	if [ "$frps_enable" = "0" ]; then
		if [ ! -z "`pidof frps`" ]; then
		killall -9 frps frp_script.sh
		[ -z "`pidof frps`" ] && logger -t "frp" "已停止 frps"
	    fi
	fi
	if [ "$frpc_enable" = "0" ] && [ "$frps_enable" = "0" ]; then
		if ! frp_manage_watchdog_cron remove; then
			logger -t "frp" "无法安全移除FRP watchdog crontab任务。"
			return 1
		fi
    fi
	return 0
}


case $1 in
start)
	frp_start
	;;
stop)
	frp_close
	;;
C)
	check_frp
	;;
esac
