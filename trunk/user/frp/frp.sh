#!/bin/sh
# Mutable NVRAM state is refreshed only after the lifecycle lock is held.
frpc_enable=
frps_enable=
http_username=
FRP_CRON_ROOT="/etc/storage/cron"
FRP_CRONTAB_DIR="$FRP_CRON_ROOT/crontabs"
FRP_WATCHDOG_CRON='*/1 * * * * /bin/sh /usr/bin/frp.sh C >/dev/null 2>&1'

check_frp ()
{
	check_net
	result_net=$?
	if [ "$result_net" = "1" ]; then
		need_start=0
		if [ "$frpc_enable" = "1" ] && ! frp_service_running frpc; then
			need_start=1
		fi
		if [ "$frps_enable" = "1" ] && ! frp_service_running frps; then
			need_start=1
		fi
		if [ "$need_start" = "1" ]; then
			frp_start || return $?
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


# Paths are fixed in production. Tests replace FRP_PROC_ROOT after sourcing.
FRP_RUNTIME_DIR='/tmp/frp'
FRP_PROC_ROOT='/proc'
FRP_START_WAIT_ATTEMPTS=5

frp_service_config_path()
{
	case "$1" in
		frpc) printf '%s\n' "$FRP_RUNTIME_DIR/myfrpc.ini" ;;
		frps) printf '%s\n' "$FRP_RUNTIME_DIR/myfrps.ini" ;;
		*) return 1 ;;
	esac
}

# Match the complete argv tuple, not a process comm name. On Linux 4.4 there
# is no pidfd; callers re-read cmdline immediately before each PID signal.
frp_process_matches()
{
	frp_match_pid="$1"
	frp_match_service="$2"
	case "$frp_match_pid" in
		''|0|1|*[!0-9]*) return 1 ;;
	esac
	frp_match_config=$(frp_service_config_path "$frp_match_service") || return 1
	frp_match_cmdline="$FRP_PROC_ROOT/$frp_match_pid/cmdline"
	[ -r "$frp_match_cmdline" ] && [ -f "$frp_match_cmdline" ] || return 1

	tr '\000' '\n' < "$frp_match_cmdline" 2>/dev/null |
		awk -v expected="$frp_match_service" -v config="$frp_match_config" '
			NR == 1 {
				executable = $0
				sub(/^.*\//, "", executable)
				if (executable != expected)
					invalid = 1
				next
			}
			NR == 2 {
				if ($0 != "-c")
					invalid = 1
				next
			}
			NR == 3 {
				if ($0 != config)
					invalid = 1
				next
			}
			{ invalid = 1 }
			END {
				if (NR == 3 && !invalid)
					exit 0
				exit 1
			}
		'
}

frp_service_pids()
{
	frp_scan_service="$1"
	frp_service_config_path "$frp_scan_service" >/dev/null || return 1
	for frp_proc_dir in "$FRP_PROC_ROOT"/[0-9]*; do
		[ -d "$frp_proc_dir" ] || continue
		frp_scan_pid=$(printf '%s\n' "$frp_proc_dir" | sed 's|.*/||')
		case "$frp_scan_pid" in ''|*[!0-9]*) continue ;; esac
		if frp_process_matches "$frp_scan_pid" "$frp_scan_service"; then
			printf '%s\n' "$frp_scan_pid"
		fi
	done
}

frp_service_running()
{
	[ -n "$(frp_service_pids "$1")" ]
}

frp_run_script()
{
	frp_script_path="${FRP_SCRIPT_PATH:-/etc/storage/frp_script.sh}"
	if [ -L "$frp_script_path" ] || [ ! -f "$frp_script_path" ]; then
		logger -t "frp" "拒绝执行缺失或非普通文件形式的FRP配置脚本。"
		return 1
	fi
	# Do not execute a legacy 0755 copy if boot-time permission repair failed.
	if ! chmod 700 "$frp_script_path"; then
		logger -t "frp" "无法将FRP配置脚本收紧为仅所有者可访问，拒绝执行。"
		return 1
	fi
	/bin/sh "$frp_script_path" 9>&-
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

frp_wait_started()
{
	frp_start_attempt=0
	while [ "$frp_start_attempt" -lt "$FRP_START_WAIT_ATTEMPTS" ]; do
		frp_start_ready=1
		if [ "$frpc_enable" = "1" ] && ! frp_service_running frpc; then
			frp_start_ready=0
		fi
		if [ "$frps_enable" = "1" ] && ! frp_service_running frps; then
			frp_start_ready=0
		fi
		[ "$frp_start_ready" = "1" ] && return 0
		frp_start_attempt=$((frp_start_attempt + 1))
		if [ "$frp_start_attempt" -lt "$FRP_START_WAIT_ATTEMPTS" ]; then
			sleep 1
		fi
	done
	return 1
}

frp_start ()
{
	if [ "$frpc_enable" = "0" ] && [ "$frps_enable" = "0" ]; then
		frp_close
		return $?
	fi

	if ! frp_stop_daemons; then
		logger -t "frp" "旧FRP进程未能安全停止，拒绝启动替代实例。"
		return 1
	fi
	if ! frp_run_script; then
		logger -t "frp" "FRP启动脚本执行失败，保留现有crontab。"
		return 1
	fi
	if ! frp_wait_started; then
		logger -t "frp" "FRP守护进程未在有界时间内就绪，拒绝报告启动成功。"
		frp_stop_daemons || logger -t "frp" "FRP启动失败后的残留进程清理未完成。"
		return 1
	fi
	if ! frp_manage_watchdog_cron add; then
		logger -t "frp" "FRP watchdog任务未能安全写入crontab。"
		frp_stop_daemons || logger -t "frp" "crontab失败后的FRP进程清理未完成。"
		return 1
	fi
	[ "$frpc_enable" != "1" ] || logger -t "frp" "frpc启动成功"
	[ "$frps_enable" != "1" ] || logger -t "frp" "frps启动成功"
	return 0
}

frp_any_running()
{
	frp_service_running frpc || frp_service_running frps
}

# Revalidate each process immediately before signalling. This narrows, but on
# Linux 4.4 cannot eliminate, PID reuse between /proc inspection and kill(2).
frp_signal_all()
{
	frp_signal="$1"
	case "$frp_signal" in TERM|KILL) ;; *) return 1 ;; esac
	for frp_signal_service in frpc frps; do
		frp_signal_pids=$(frp_service_pids "$frp_signal_service") || return 1
		for frp_signal_pid in $frp_signal_pids; do
			frp_process_matches "$frp_signal_pid" "$frp_signal_service" || continue
			kill "-$frp_signal" "$frp_signal_pid" >/dev/null 2>&1 || :
		done
	done
	return 0
}

frp_wait_stopped()
{
	frp_wait_limit="$1"
	frp_waited=0
	while [ "$frp_waited" -lt "$frp_wait_limit" ]; do
		frp_any_running || return 0
		sleep 1
		frp_waited=$((frp_waited + 1))
	done
	! frp_any_running
}

frp_stop_daemons()
{
	if frp_any_running; then
		frp_signal_all TERM || return 1
		if ! frp_wait_stopped 5; then
			frp_signal_all KILL || return 1
			if ! frp_wait_stopped 2; then
				logger -t "frp" "匹配配置的FRP进程在有界TERM/KILL后仍未退出。"
				return 1
			fi
		fi
	fi
	return 0
}

frp_close ()
{
	frp_close_status=0
	if ! frp_manage_watchdog_cron remove; then
		logger -t "frp" "无法安全移除FRP watchdog crontab任务。"
		frp_close_status=1
	fi
	frp_stop_daemons || frp_close_status=1
	return "$frp_close_status"
}

frp_refresh_nvram_state()
{
	frpc_enable=$(nvram get frpc_enable) || return 1
	frps_enable=$(nvram get frps_enable) || return 1
	http_username=$(nvram get http_username) || return 1

	case "$frpc_enable" in 0|1) ;; *) return 1 ;; esac
	case "$frps_enable" in 0|1) ;; *) return 1 ;; esac
	return 0
}

frp_lifecycle_lock_path()
{
	printf '%s\n' '/tmp/.frp.lifecycle.lock'
}

# Keep one root-private inode for the entire boot. Never unlink a flock file:
# replacing it while held would allow another launcher to acquire a second lock.
frp_lifecycle_lock_prepare()
{
	frp_lock_file="$1"
	[ -n "$frp_lock_file" ] || return 1
	case "$frp_lock_file" in
		*/*) frp_lock_dir="${frp_lock_file%/*}" ;;
		*) frp_lock_dir=. ;;
	esac
	[ -n "$frp_lock_dir" ] || frp_lock_dir=/

	if [ -L "$frp_lock_dir" ] || [ ! -d "$frp_lock_dir" ]; then
		logger -t "frp" "生命周期锁目录不是普通目录。"
		return 1
	fi
	frp_lock_uid=$(id -u) || return 1
	frp_lock_dir_info=$(stat -c '%u %a' "$frp_lock_dir") || return 1
	set -- $frp_lock_dir_info
	if [ "$#" -ne 2 ] || [ "$1" != "$frp_lock_uid" ]; then
		logger -t "frp" "生命周期锁目录所有者不安全。"
		return 1
	fi
	frp_lock_dir_mode=$((0$2))
	if [ $((frp_lock_dir_mode & 0022)) -ne 0 ] &&
	   [ $((frp_lock_dir_mode & 01000)) -eq 0 ]; then
		logger -t "frp" "生命周期锁目录可被其他用户写入且没有sticky保护。"
		return 1
	fi

	if [ -L "$frp_lock_file" ]; then
		logger -t "frp" "生命周期锁文件不能是符号链接。"
		return 1
	fi
	if [ ! -e "$frp_lock_file" ]; then
		(umask 077; set -C; : > "$frp_lock_file") 2>/dev/null || :
	fi
	if [ -L "$frp_lock_file" ] || [ ! -f "$frp_lock_file" ]; then
		logger -t "frp" "生命周期锁文件不是普通文件。"
		return 1
	fi
	frp_lock_file_info=$(stat -c '%u %a %h' "$frp_lock_file") || return 1
	set -- $frp_lock_file_info
	if [ "$#" -ne 3 ] || [ "$1" != "$frp_lock_uid" ] ||
	   [ "$2" != 600 ] || [ "$3" != 1 ]; then
		logger -t "frp" "生命周期锁文件权限、所有者或链接数不安全。"
		return 1
	fi
	return 0
}

# FD 9 is reserved for this short-lived launcher. A bounded wait keeps PID 1
# responsive when a watchdog, UI restart and shutdown overlap.
frp_lifecycle_lock_execute()
{
	frp_lock_file="$1"
	frp_lock_wait_seconds="$2"
	shift 2
	case "$frp_lock_wait_seconds" in ''|*[!0-9]*) return 1 ;; esac
	[ "$#" -gt 0 ] || return 1

	if ( : <&9 ) 2>/dev/null || ( : >&9 ) 2>/dev/null; then
		logger -t "frp" "生命周期锁FD 9已被占用，拒绝运行。"
		return 1
	fi
	frp_lock_old_umask=$(umask)
	umask 077
	if ! exec 9>>"$frp_lock_file"; then
		umask "$frp_lock_old_umask"
		logger -t "frp" "无法打开生命周期锁文件。"
		return 1
	fi
	umask "$frp_lock_old_umask"

	frp_lock_attempt=0
	while ! flock -n 9; do
		if [ "$frp_lock_attempt" -ge "$frp_lock_wait_seconds" ]; then
			exec 9>&-
			logger -t "frp" "等待生命周期锁超时，取消本次操作。"
			return 1
		fi
		frp_lock_attempt=$((frp_lock_attempt + 1))
		sleep 1
	done

	if "$@"; then
		frp_lock_action_status=0
	else
		frp_lock_action_status=$?
	fi
	exec 9>&-
	return "$frp_lock_action_status"
}

frp_lifecycle_locked_action()
{
	frp_refresh_nvram_state || {
		logger -t "frp" "无法读取有效的FRP角色配置，拒绝执行生命周期操作。"
		return 1
	}
	case "$1" in
		start)
			frp_start
			;;
		stop)
			frp_close
			;;
		restart)
			frp_close || return $?
			frp_start
			;;
		C)
			check_frp
			;;
		*)
			return 2
			;;
	esac
}

frp_lifecycle_lock_run()
{
	case "$1" in start|stop|restart|C) ;; *) return 2 ;; esac
	frp_lock_file=$(frp_lifecycle_lock_path) || return 1
	frp_lifecycle_lock_prepare "$frp_lock_file" || {
		logger -t "frp" "生命周期锁准备失败，取消FRP操作。"
		return 1
	}
	command -v flock >/dev/null 2>&1 || {
		logger -t "frp" "系统缺少flock，拒绝无锁执行生命周期操作。"
		return 1
	}
	frp_lifecycle_lock_execute "$frp_lock_file" 30 \
		frp_lifecycle_locked_action "$1"
}

case "${1:-}" in
start|stop|restart|C)
	frp_lifecycle_lock_run "$1"
	;;
esac
