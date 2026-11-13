#!/bin/sh

DDNSTO_BIN="/usr/bin/ddnsto"
DDNSTO_LOCK_WAIT_SECONDS=10

start_zero() {
	logger -t "DDNSTO" "正在启动DDNSTO"
	if ! kill_z; then
		logger -t "DDNSTO" "停止旧进程失败，取消启动"
		return 1
	fi
	# The daemon must not inherit the short-lived lifecycle lock descriptor.
	"$DDNSTO_BIN" -u "$(nvram get ddnsto_id)" >/dev/null 2>&1 9>&- &
}

ddnsto_wait_stopped() {
	ddnsto_wait=0
	while [ "$ddnsto_wait" -lt 5 ]; do
		if ! pidof ddnsto >/dev/null 2>&1; then
			return 0
		fi
		ddnsto_wait=$((ddnsto_wait + 1))
		sleep 1
	done
	! pidof ddnsto >/dev/null 2>&1
}

kill_z() {
	if ! pidof ddnsto >/dev/null 2>&1; then
		return 0
	fi
	logger -t "DDNSTO" "关闭进程..."
	killall ddnsto >/dev/null 2>&1
	if ddnsto_wait_stopped; then
		return 0
	fi
	logger -t "DDNSTO" "正常停止超时，强制结束进程..."
	killall -9 ddnsto >/dev/null 2>&1
	if ddnsto_wait_stopped; then
		return 0
	fi
	logger -t "DDNSTO" "进程仍未退出，保留服务状态并取消后续启动"
	return 1
}

stop_zero() {
	kill_z
}

ddnsto_lifecycle_lock_path() {
	printf '%s\n' '/tmp/.ddnsto.lifecycle.lock'
}

# Keep one private inode for the entire boot. Never unlink a flock file:
# replacing it while held would allow concurrent launchers to lock different inodes.
ddnsto_lifecycle_lock_prepare() {
	ddnsto_lock_file="$1"
	[ -n "$ddnsto_lock_file" ] || return 1
	case "$ddnsto_lock_file" in
		*/*) ddnsto_lock_dir="${ddnsto_lock_file%/*}" ;;
		*) ddnsto_lock_dir=. ;;
	esac
	[ -n "$ddnsto_lock_dir" ] || ddnsto_lock_dir=/

	if [ -L "$ddnsto_lock_dir" ] || [ ! -d "$ddnsto_lock_dir" ]; then
		logger -t "DDNSTO" "生命周期锁目录不是普通目录"
		return 1
	fi
	ddnsto_lock_uid=$(id -u) || return 1
	ddnsto_dir_info=$(stat -c '%u %a' "$ddnsto_lock_dir") || return 1
	set -- $ddnsto_dir_info
	if [ "$#" -ne 2 ] || [ "$1" != "$ddnsto_lock_uid" ]; then
		logger -t "DDNSTO" "生命周期锁目录所有者不安全"
		return 1
	fi
	ddnsto_dir_mode=$((0$2))
	if [ $((ddnsto_dir_mode & 0022)) -ne 0 ] &&
	   [ $((ddnsto_dir_mode & 01000)) -eq 0 ]; then
		logger -t "DDNSTO" "生命周期锁目录可被其他用户写入且没有sticky保护"
		return 1
	fi

	if [ -L "$ddnsto_lock_file" ]; then
		logger -t "DDNSTO" "生命周期锁文件不能是符号链接"
		return 1
	fi
	if [ ! -e "$ddnsto_lock_file" ]; then
		(umask 077; set -C; : > "$ddnsto_lock_file") 2>/dev/null || :
	fi
	if [ -L "$ddnsto_lock_file" ] || [ ! -f "$ddnsto_lock_file" ]; then
		logger -t "DDNSTO" "生命周期锁文件不是普通文件"
		return 1
	fi
	ddnsto_file_info=$(stat -c '%u %a %h' "$ddnsto_lock_file") || return 1
	set -- $ddnsto_file_info
	if [ "$#" -ne 3 ] || [ "$1" != "$ddnsto_lock_uid" ] ||
	   [ "$2" != 600 ] || [ "$3" != 1 ]; then
		logger -t "DDNSTO" "生命周期锁文件权限、所有者或链接数不安全"
		return 1
	fi
	return 0
}

# FD 9 belongs to this short-lived launcher; bounded contention keeps PID 1 responsive.
ddnsto_lifecycle_lock_execute() {
	ddnsto_lock_file="$1"
	ddnsto_wait_seconds="$2"
	shift 2
	case "$ddnsto_wait_seconds" in
		''|*[!0-9]*) return 1 ;;
	esac
	[ "$#" -gt 0 ] || return 1

	if ( : <&9 ) 2>/dev/null || ( : >&9 ) 2>/dev/null; then
		logger -t "DDNSTO" "生命周期锁FD 9已被占用，拒绝运行"
		return 1
	fi

	ddnsto_old_umask=$(umask)
	umask 077
	if ! exec 9>>"$ddnsto_lock_file"; then
		umask "$ddnsto_old_umask"
		logger -t "DDNSTO" "无法打开生命周期锁文件"
		return 1
	fi
	umask "$ddnsto_old_umask"

	ddnsto_lock_attempt=0
	while ! flock -n 9; do
		if [ "$ddnsto_lock_attempt" -ge "$ddnsto_wait_seconds" ]; then
			exec 9>&-
			logger -t "DDNSTO" "等待生命周期锁超时"
			return 1
		fi
		ddnsto_lock_attempt=$((ddnsto_lock_attempt + 1))
		sleep 1
	done

	if "$@"; then
		ddnsto_action_status=0
	else
		ddnsto_action_status=$?
	fi
	exec 9>&-
	return "$ddnsto_action_status"
}

ddnsto_lifecycle_locked_action() {
	case "$1" in
		start)
			if [ "$(nvram get ddnsto_enable)" = "1" ]; then
				start_zero
			fi
			;;
		stop)
			stop_zero
			;;
		restart)
			stop_zero || return $?
			if [ "$(nvram get ddnsto_enable)" = "1" ]; then
				start_zero
			fi
			;;
		*)
			return 2
			;;
	esac
}

ddnsto_lifecycle_lock_run() {
	ddnsto_action="$1"
	case "$ddnsto_action" in
		start|stop|restart) ;;
		*) return 2 ;;
	esac
	ddnsto_lock_file=$(ddnsto_lifecycle_lock_path) || return 1
	ddnsto_lifecycle_lock_prepare "$ddnsto_lock_file" || {
		logger -t "DDNSTO" "生命周期锁准备失败，取消服务操作"
		return 1
	}
	command -v flock >/dev/null 2>&1 || {
		logger -t "DDNSTO" "系统缺少flock，拒绝无锁执行服务操作"
		return 1
	}
	ddnsto_lifecycle_lock_execute "$ddnsto_lock_file" "$DDNSTO_LOCK_WAIT_SECONDS" \
		ddnsto_lifecycle_locked_action "$ddnsto_action"
}

case "${1:-}" in
start|stop|restart)
	ddnsto_lifecycle_lock_run "$1"
	;;
*)
	echo "check"
	;;
esac
