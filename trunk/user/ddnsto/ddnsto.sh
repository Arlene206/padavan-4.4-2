#!/bin/sh

start_zero() {
	logger -t "DDNSTO" "正在启动DDNSTO"
	kill_z || {
		logger -t "DDNSTO" "停止旧进程失败，取消启动"
		return 1
	}
	/usr/bin/ddnsto -u "$(nvram get ddnsto_id)" >/dev/null 2>&1 &

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
