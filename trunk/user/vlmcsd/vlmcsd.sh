#!/bin/sh

func_start(){
	if vlmcsd -A 30d -R 30d; then
		logger -st "vlmcsd" "KMS Activation server is running."
		return 0
	else
		launch_status=$?
		logger -st "vlmcsd" "KMS Activation server launch failed (status=$launch_status)."
		return "$launch_status"
	fi
}

func_stop(){
	killall -q vlmcsd
	logger -st "vlmcsd" "KMS Activation server is stoping."
}

case "$1" in
start)
	func_start
	;;
stop)
	func_stop
	;;
restart)
	func_stop
	func_start
	;;
*)
	echo "Usage: $0 { start | stop | restart }"
	exit 1
	;;
esac
