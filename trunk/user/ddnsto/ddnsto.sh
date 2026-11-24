#!/bin/sh

DDNSTO_BIN="/usr/bin/ddnsto"
DDNSTO_LOCK_WAIT_SECONDS=10
DDNSTO_START_WAIT_ATTEMPTS=5

start_zero() {
	logger -t "DDNSTO" "正在启动DDNSTO"
	# Validate launch prerequisites before disrupting a currently working instance.
	if [ ! -x "$DDNSTO_BIN" ]; then
		logger -t "DDNSTO" "DDNSTO可执行文件缺失或不可执行，取消启动"
		return 1
	fi
	if ddnsto_start_id=$(nvram get ddnsto_id); then
		:
	else
		logger -t "DDNSTO" "无法读取DDNSTO身份标识，取消启动"
		return 1
	fi
	if [ -z "$ddnsto_start_id" ]; then
		logger -t "DDNSTO" "DDNSTO身份标识为空，取消启动"
		return 1
	fi
	if ! kill_z; then
		logger -t "DDNSTO" "停止旧进程失败，取消启动"
		return 1
	fi
	# The daemon must not inherit the short-lived lifecycle lock descriptor.
	"$DDNSTO_BIN" -u "$ddnsto_start_id" >/dev/null 2>&1 9>&- &
	# A successful shell fork is not proof that the daemon remained alive.
	if ddnsto_wait_started; then
		return 0
	else
		ddnsto_start_wait_status=$?
	fi
	case "$ddnsto_start_wait_status" in
		1)
			logger -t "DDNSTO" "启动后在限定时间内未检测到DDNSTO进程"
			;;
		*)
			logger -t "DDNSTO" "启动后无法确认DDNSTO进程状态"
			;;
	esac
	return 1
}

# Return 0 for RUNNING, 1 only for confirmed ABSENT, and 2 for UNKNOWN.
# A failed or malformed pidof result must never authorize a new daemon launch.
ddnsto_process_state() {
	if ddnsto_process_pids=$(pidof ddnsto 2>/dev/null); then
		ddnsto_pidof_status=0
	else
		ddnsto_pidof_status=$?
	fi
	case "$ddnsto_pidof_status" in
		0)
			[ -n "$ddnsto_process_pids" ] || return 2
			case "$ddnsto_process_pids" in
				*[!0123456789\ ]*) return 2 ;;
			esac
			ddnsto_process_count=0
			for ddnsto_process_pid in $ddnsto_process_pids; do
				case "$ddnsto_process_pid" in
					''|0*|1|*[!0-9]*) return 2 ;;
				esac
				[ "${#ddnsto_process_pid}" -le 10 ] || return 2
				[ "$ddnsto_process_pid" -gt 1 ] 2>/dev/null || return 2
				ddnsto_process_count=$((ddnsto_process_count + 1))
			done
			[ "$ddnsto_process_count" -gt 0 ] || return 2
			return 0
			;;
		1)
			[ -z "$ddnsto_process_pids" ] && return 1
			return 2
			;;
		*) return 2 ;;
	esac
}

# Return 0 only for a validated RUNNING inventory. Confirmed ABSENT is retried;
# UNKNOWN process state and a failed wait primitive abort startup immediately.
ddnsto_wait_started() {
	case "$DDNSTO_START_WAIT_ATTEMPTS" in
		''|*[!0-9]*|0) return 2 ;;
	esac
	ddnsto_start_wait=0
	while [ "$ddnsto_start_wait" -lt "$DDNSTO_START_WAIT_ATTEMPTS" ]; do
		if ddnsto_process_state; then
			return 0
		else
			ddnsto_start_state=$?
		fi
		case "$ddnsto_start_state" in
			1) ;;
			*) return 2 ;;
		esac
		ddnsto_start_wait=$((ddnsto_start_wait + 1))
		if [ "$ddnsto_start_wait" -lt "$DDNSTO_START_WAIT_ATTEMPTS" ]; then
			if ! sleep 1; then
				return 2
			fi
		fi
	done
	return 1
}

# Return 0 when absent, 1 when still running after five polls, 2 when UNKNOWN.
ddnsto_wait_stopped() {
	ddnsto_wait=0
	while [ "$ddnsto_wait" -lt 5 ]; do
		if ddnsto_process_state; then
			ddnsto_wait_state=0
		else
			ddnsto_wait_state=$?
		fi
		case "$ddnsto_wait_state" in
			1) return 0 ;;
			0) ;;
			*) return 2 ;;
		esac
		if ! sleep 1; then
			return 2
		fi
		ddnsto_wait=$((ddnsto_wait + 1))
	done
	if ddnsto_process_state; then
		return 1
	else
		ddnsto_wait_state=$?
	fi
	case "$ddnsto_wait_state" in
		1) return 0 ;;
		2) return 2 ;;
		*) return 1 ;;
	esac
}

kill_z() {
	if ddnsto_process_state; then
		ddnsto_stop_state=0
	else
		ddnsto_stop_state=$?
	fi
	case "$ddnsto_stop_state" in
		1) return 0 ;;
		0) ;;
		*)
			logger -t "DDNSTO" "无法读取DDNSTO进程状态，拒绝发送信号"
			return 1
			;;
	esac

	logger -t "DDNSTO" "关闭进程..."
	if killall ddnsto >/dev/null 2>&1; then
		ddnsto_term_status=0
	else
		ddnsto_term_status=$?
		logger -t "DDNSTO" "TERM信号命令返回错误(status=$ddnsto_term_status)，继续验证进程状态"
	fi
	if ddnsto_wait_stopped; then
		return 0
	else
		ddnsto_wait_status=$?
	fi
	case "$ddnsto_wait_status" in
		1) ;;
		*)
			logger -t "DDNSTO" "TERM后进程状态未知，拒绝升级信号"
			return 1
			;;
	esac

	logger -t "DDNSTO" "正常停止超时，强制结束进程..."
	if killall -9 ddnsto >/dev/null 2>&1; then
		ddnsto_kill_status=0
	else
		ddnsto_kill_status=$?
		logger -t "DDNSTO" "SIGKILL命令返回错误(status=$ddnsto_kill_status)，继续验证进程状态"
	fi
	if ddnsto_wait_stopped; then
		return 0
	else
		ddnsto_wait_status=$?
	fi
	case "$ddnsto_wait_status" in
		1) logger -t "DDNSTO" "SIGKILL后DDNSTO仍在运行" ;;
		*) logger -t "DDNSTO" "SIGKILL后无法确认DDNSTO进程状态" ;;
	esac
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

# Preserve NVRAM command failures instead of treating them as a disabled service.
# Restart performs this preflight before stopping a currently working daemon.
ddnsto_read_enable_state() {
	if ddnsto_enable_value=$(nvram get ddnsto_enable); then
		case "$ddnsto_enable_value" in
			''|0|1)
				return 0
				;;
			*)
				logger -t "DDNSTO" "DDNSTO enable selector is invalid; refusing to change process state"
				return 22
				;;
		esac
	else
		ddnsto_enable_status=$?
	fi
	logger -t "DDNSTO" "无法读取DDNSTO启用状态(status=$ddnsto_enable_status)，取消服务操作"
	[ "$ddnsto_enable_status" -ne 0 ] || ddnsto_enable_status=1
	return "$ddnsto_enable_status"
}

ddnsto_lifecycle_locked_action() {
	case "$1" in
		start)
			ddnsto_read_enable_state || return $?
			if [ "$ddnsto_enable_value" = "1" ]; then
				start_zero
			fi
			;;
		stop)
			stop_zero
			;;
		restart)
			ddnsto_read_enable_state || return $?
			stop_zero || return $?
			if [ "$ddnsto_enable_value" = "1" ]; then
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
	if ! ddnsto_lifecycle_lock_prepare "$ddnsto_lock_file"; then
		logger -t "DDNSTO" "生命周期锁准备失败，取消服务操作"
		return 1
	fi
	if ! command -v flock >/dev/null 2>&1; then
		logger -t "DDNSTO" "系统缺少flock，拒绝无锁执行服务操作"
		return 1
	fi
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
