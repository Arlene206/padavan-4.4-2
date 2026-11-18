#!/bin/sh

PROCESS_LIFECYCLE_LIB="${PROCESS_LIFECYCLE_LIB:-/usr/lib/process_lifecycle.sh}"
if [ ! -r "$PROCESS_LIFECYCLE_LIB" ]; then
	logger -st "vlmcsd" "process lifecycle helper is unavailable"
	exit 127
fi
. "$PROCESS_LIFECYCLE_LIB" || exit 127

func_start(){
	if pl_process_state vlmcsd; then
		logger -st "vlmcsd" "KMS Activation server is already running."
		return 0
	else
		process_status=$?
	fi
	if [ "$process_status" -ne 1 ]; then
		logger -st "vlmcsd" "KMS Activation server state is unknown; refusing duplicate launch."
		return 2
	fi

	if vlmcsd -A 30d -R 30d; then
		if pl_wait_started vlmcsd 1 4 1; then
			logger -st "vlmcsd" "KMS Activation server is running."
			return 0
		else
			readiness_status=$?
			if [ "$readiness_status" -eq 1 ]; then
				logger -st "vlmcsd" "KMS Activation server startup timed out; no daemon process appeared."
			else
				logger -st "vlmcsd" "KMS Activation server startup state could not be verified (status=$readiness_status)."
			fi
			return "$readiness_status"
		fi
	else
		launch_status=$?
		logger -st "vlmcsd" "KMS Activation server launch failed (status=$launch_status)."
		return "$launch_status"
	fi
}

func_stop(){
	if pl_stop_named vlmcsd 3 2; then
		logger -st "vlmcsd" "KMS Activation server stopped."
		return 0
	else
		stop_status=$?
		logger -st "vlmcsd" "KMS Activation server stop failed (status=$stop_status)."
		return "$stop_status"
	fi
}

case "$1" in
start)
	func_start
	;;
stop)
	func_stop
	;;
restart)
	if func_stop; then
		func_start
	else
		stop_status=$?
		logger -st "vlmcsd" "KMS Activation server restart aborted because stop failed (status=$stop_status)."
		exit "$stop_status"
	fi
	;;
*)
	echo "Usage: $0 { start | stop | restart }"
	exit 1
	;;
esac
