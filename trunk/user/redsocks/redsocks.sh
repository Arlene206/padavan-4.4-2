#!/bin/bash
# Github: https://github.com/SuzukiHonoka
# Made by starx.

BINARY_NAME="redsocks"
BIN_DIR="/usr/bin"
STORAGE_DIR="/etc/storage"
#BINARY_PATH="$STORAGE_DIR/$BINARY_NAME/$BINARY_NAME"
BINARY_PATH="$BIN_DIR/$BINARY_NAME"
REDSOCKS_DIR="$STORAGE_DIR/$BINARY_NAME"
REDSOCKS_CONF="$REDSOCKS_DIR/$BINARY_NAME.conf"
CNZONE_FILE="$REDSOCKS_DIR/cn.zone"
CHAIN_NAME="REDSOCKS"
SET_NAME="china"
LOCK="/tmp/set_lock"
MODE=false

SOCKS5_IP="192.168.31.99"
SOCKS5_PORT="10808"
REMOTE_IP="127.0.0.1"

ARG1=$1
ARG2=$2
ARG3=$3

func_redsocks(){
cd $REDSOCKS_DIR
$BINARY_PATH $REDSOCKS_CONF
}

func_save(){
/sbin/mtd_storage.sh save
}

redsocks_process_state()
{
	if redsocks_pid_output=$(pidof "$BINARY_NAME" 2>/dev/null); then
		redsocks_pid_status=0
	else
		redsocks_pid_status=$?
	fi
	case "$redsocks_pid_status" in
	0)
		[ -n "$redsocks_pid_output" ] || return 2
		case "$redsocks_pid_output" in
			*[!0123456789\ ]*) return 2 ;;
		esac
		redsocks_pid_count=0
		for redsocks_pid in $redsocks_pid_output; do
			case "$redsocks_pid" in
				''|0*|1|*[!0-9]*) return 2 ;;
			esac
			[ "${#redsocks_pid}" -le 10 ] || return 2
			[ "$redsocks_pid" -gt 1 ] 2>/dev/null || return 2
			redsocks_pid_count=$((redsocks_pid_count + 1))
		done
		[ "$redsocks_pid_count" -gt 0 ] || return 2
		return 0
		;;
	1)
		[ -z "$redsocks_pid_output" ] && return 1
		return 2
		;;
	*) return 2 ;;
	esac
}

redsocks_wait_stopped()
{
	redsocks_wait=0
	while [ "$redsocks_wait" -lt 5 ]; do
		if redsocks_process_state; then
			redsocks_state=0
		else
			redsocks_state=$?
		fi
		case "$redsocks_state" in
			1) return 0 ;;
			0) ;;
			*) return 2 ;;
		esac
		sleep 1 || return 2
		redsocks_wait=$((redsocks_wait + 1))
	done
	if redsocks_process_state; then
		return 1
	else
		redsocks_state=$?
	fi
	case "$redsocks_state" in
		1) return 0 ;;
		0) return 1 ;;
		*) return 2 ;;
	esac
}

redsocks_wait_started()
{
	redsocks_wait=0
	while [ "$redsocks_wait" -lt 5 ]; do
		if redsocks_process_state; then
			return 0
		else
			redsocks_state=$?
		fi
		case "$redsocks_state" in
			1) ;;
			*) return 2 ;;
		esac
		redsocks_wait=$((redsocks_wait + 1))
		[ "$redsocks_wait" -lt 5 ] || break
		sleep 1 || return 2
	done
	return 1
}

# daemon=on makes a successful command exit only a launch acknowledgement.
# Do not report STARTED until pidof confirms the detached child is present.
redsocks_start_and_verify()
{
	func_redsocks
	redsocks_launch_status=$?
	if [ "$redsocks_launch_status" -ne 0 ]; then
		logger -t "$BINARY_NAME" "launch failed (status=$redsocks_launch_status)"
		return "$redsocks_launch_status"
	fi

	if redsocks_wait_started; then
		return 0
	else
		redsocks_readiness_status=$?
	fi
	case "$redsocks_readiness_status" in
		1)
			logger -t "$BINARY_NAME" "startup timed out; daemon did not become RUNNING"
			return 1
			;;
		*)
			logger -t "$BINARY_NAME" "startup state is UNKNOWN; refusing success (status=$redsocks_readiness_status)"
			return 2
			;;
	esac
}

func_start(){
MODE=true
if redsocks_process_state; then
	if ! func_stop; then
		logger -t "$BINARY_NAME" "existing process could not be stopped; refusing duplicate launch"
		return 1
	fi
	logger -t "$BINARY_NAME" "ALREADY RUNNING: KILLED"
else
	redsocks_state=$?
	if [ "$redsocks_state" -ne 1 ]; then
		logger -t "$BINARY_NAME" "process inventory unknown; refusing duplicate launch"
		return 1
	fi
fi
if [ ! -f $REDSOCKS_CONF ]
then
	if  [ -n "$ARG2" ]
	then
		if [ -n "$ARG3" ]
		then
			SOCKS5_IP=$ARG2
			SOCKS5_PORT=$ARG3
			if [ ! -d $REDSOCKS_DIR ]
			then
			mkdir -p $REDSOCKS_DIR	
			fi
		cat > $REDSOCKS_CONF <<EOF
base {
log_debug = off;
log_info = off;
redirector = iptables;
daemon = on;
redsocks_conn_max = 10000;
rlimit_nofile = 10240;
}
redsocks {
local_ip = 0.0.0.0;
local_port = 12345;
ip = $SOCKS5_IP;
port = $SOCKS5_PORT;
type = socks5;
}
EOF
func_save
logger -t $BINARY_NAME "CONFIG FILE SAVED."
redsocks_start_and_verify || return $?
fi
else
	logger -t $BINARY_NAME "CONFIG FILE NOT FOUND"
	return 0
fi
else
	redsocks_start_and_verify || return $?
	logger -t $BINARY_NAME "STARTED."
fi
}

func_clean(){
iptables -t nat -D PREROUTING `iptables -t nat -L PREROUTING --line-numbers|grep $CHAIN_NAME|head -n 1|tr -cd "[0-9]"`
iptables -t nat -D OUTPUT `iptables -t nat -L OUTPUT --line-numbers|grep $CHAIN_NAME|head -n 1|tr -cd "[0-9]"`
iptables -t nat -F $CHAIN_NAME
iptables -t nat -X $CHAIN_NAME
}

func_iptables(){
if [ -n "$ARG2" ]
then
	REMOTE_IP=$ARG2
else
	logger -t $BINARY_NAME "REMOTE_IP NOT FOUND!"
	return 0
fi

func_clean

if [ ! -f $CNZONE_FILE ]
then
	wget -P $REDSOCKS_DIR http://www.ipdeny.com/ipblocks/data/countries/cn.zone
	func_save
	logger -t "SET FILE SAVED."
fi

if [ ! -f $LOCK ]
then
	ipset destroy $SET_NAME
	ipset -N $SET_NAME hash:net
	for i in $(cat ${REDSOCKS_DIR}/cn.zone ); do ipset -A china $i; done
	touch $LOCK
	logger -t $BINARY_NAME "SET LOCKED"
fi

iptables -t nat -N $CHAIN_NAME
iptables -t nat -A $CHAIN_NAME -d $REMOTE_IP -j RETURN
iptables -t nat -A $CHAIN_NAME -d 0.0.0.0/8 -j RETURN
iptables -t nat -A $CHAIN_NAME -d 10.0.0.0/8 -j RETURN
iptables -t nat -A $CHAIN_NAME -d 127.0.0.0/8 -j RETURN
iptables -t nat -A $CHAIN_NAME -d 169.254.0.0/16 -j RETURN
iptables -t nat -A $CHAIN_NAME -d 172.16.0.0/12 -j RETURN
iptables -t nat -A $CHAIN_NAME -d 192.168.0.0/16 -j RETURN
iptables -t nat -A $CHAIN_NAME -d 224.0.0.0/4 -j RETURN
iptables -t nat -A $CHAIN_NAME -d 240.0.0.0/4 -j RETURN
iptables -t nat -A $CHAIN_NAME -m set --match-set china dst -j RETURN
iptables -t nat -A $CHAIN_NAME -p tcp -j REDIRECT --to-ports 12345 
iptables -t nat -A PREROUTING -p tcp -j $CHAIN_NAME
iptables -t nat -A OUTPUT -p tcp -j $CHAIN_NAME
}

func_stop(){
	if redsocks_process_state; then
		redsocks_state=0
	else
		redsocks_state=$?
	fi
	case "$redsocks_state" in
		1) return 0 ;;
		0) ;;
		*)
			logger -t "$BINARY_NAME" "process inventory unknown; refusing stop"
			return 1
			;;
	esac

	killall -q "$BINARY_NAME" >/dev/null 2>&1 || :
	if redsocks_wait_stopped; then
		logger -t "$BINARY_NAME" "KILLED"
		return 0
	else
		redsocks_wait_status=$?
	fi
	case "$redsocks_wait_status" in
		1) ;;
		*)
			logger -t "$BINARY_NAME" "TERM postcondition unknown; refusing KILL escalation"
			return 1
			;;
	esac

	if redsocks_process_state; then
		redsocks_state=0
	else
		redsocks_state=$?
	fi
	case "$redsocks_state" in
		1) return 0 ;;
		0) ;;
		*)
			logger -t "$BINARY_NAME" "pre-KILL process inventory unknown; refusing signal"
			return 1
			;;
	esac

	killall -q -9 "$BINARY_NAME" >/dev/null 2>&1 || :
	if redsocks_wait_stopped; then
		logger -t "$BINARY_NAME" "KILLED"
		return 0
	fi
	logger -t "$BINARY_NAME" "process remains running or unknown after KILL"
	return 1
}

if [ "${RED_SOCKS_TEST_MODE:-0}" != 1 ]; then
case "$ARG1" in
start)
	func_start
	;;
stop)
	func_stop
	;;
iptables)
	func_iptables
	;;
clean)
	func_clean
	;;
restart)
	func_stop && func_start
	;;
*)
	echo "Usage: $0 {start [ ARG2:Server IP ARG3:Server PORT ]|stop|iptables [ ARG1:Server IP ]|restart}"
	exit 1
	;;
esac
fi