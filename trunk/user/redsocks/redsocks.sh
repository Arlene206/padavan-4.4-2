#!/bin/bash
# Github: https://github.com/SuzukiHonoka
# Made by starx.

BINARY_NAME="redsocks"
BIN_DIR="/usr/bin"
STORAGE_DIR="/etc/storage"
#BINARY_PATH="$STORAGE_DIR/$BINARY_NAME/$BINARY_NAME"
BINARY_PATH="$BIN_DIR/$BINARY_NAME"
RED_SOCKS_PROC_ROOT="${RED_SOCKS_PROC_ROOT:-/proc}"
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

# Capture PID, kernel start time and executable identity before signalling.
# /proc/PID/stat field 22 fences PID reuse; the executable link prevents a
# same-comm process from being mistaken for the configured Redsocks binary.
declare -a redsocks_pids=()
declare -A redsocks_identity_start=()

redsocks_read_stat_identity()
{
	local pid="$1" stat_line stat_fields_raw
	local -a stat_fields
	case "$pid" in ''|0*|*[!0-9]*) return 2 ;; esac
	[ "${#pid}" -le 10 ] || return 2
	if ! IFS= read -r stat_line < "$RED_SOCKS_PROC_ROOT/$pid/stat"; then
		[ -d "$RED_SOCKS_PROC_ROOT/$pid" ] && return 2
		return 1
	fi
	if [[ ! "$stat_line" =~ ^([0-9]+)\ \((.*)\)\ (.*)$ ]]; then return 2; fi
	[ "${BASH_REMATCH[1]}" = "$pid" ] || return 2
	redsocks_stat_comm=${BASH_REMATCH[2]}
	stat_fields_raw=${BASH_REMATCH[3]}
	read -r -a stat_fields <<< "$stat_fields_raw"
	[ "${#stat_fields[@]}" -ge 20 ] || return 2
	case "${stat_fields[0]}" in R|S|D|Z|T|t|X|x|K|W|P|I) ;; *) return 2 ;; esac
	case "${stat_fields[19]}" in ''|*[!0-9]*) return 2 ;; esac
	redsocks_stat_state="${stat_fields[0]}"
	redsocks_stat_start_time="${stat_fields[19]}"
	return 0
}

redsocks_read_identity()
{
	local pid="$1" expected_exe actual_exe first_comm first_state first_start status
	redsocks_read_stat_identity "$pid"
	status=$?
	[ "$status" -eq 0 ] || return "$status"
	first_comm="$redsocks_stat_comm"
	first_state="$redsocks_stat_state"
	first_start="$redsocks_stat_start_time"
	case "$first_state" in Z|X|x) return 1 ;; esac
	[ "$first_comm" = "$BINARY_NAME" ] || return 2
	expected_exe=$(readlink -f "$BINARY_PATH" 2>/dev/null) || return 2
	actual_exe=$(readlink "$RED_SOCKS_PROC_ROOT/$pid/exe" 2>/dev/null) || {
		[ -d "$RED_SOCKS_PROC_ROOT/$pid" ] && return 2
		return 1
	}
	[ "$actual_exe" = "$expected_exe" ] || return 2
	# stat and exe are separate procfs reads. Reject a PID recycled while
	# collecting identity instead of combining metadata from two processes.
	redsocks_read_stat_identity "$pid"
	status=$?
	[ "$status" -eq 0 ] || return "$status"
	[ "$redsocks_stat_comm" = "$first_comm" ] &&
		[ "$redsocks_stat_start_time" = "$first_start" ] || return 2
	case "$redsocks_stat_state" in Z|X|x) return 1 ;; esac
	redsocks_identity_start_time="$first_start"
	return 0
}

redsocks_process_state()
{
	local pid status
	local -a parsed_pids=()
	redsocks_pids=()
	redsocks_identity_start=()
	if redsocks_pid_output=$(pidof "$BINARY_NAME" 2>/dev/null); then
		redsocks_pid_status=0
	else
		redsocks_pid_status=$?
	fi
	case "$redsocks_pid_status" in
	0)
		[ -n "$redsocks_pid_output" ] || return 2
		case "$redsocks_pid_output" in *[!0123456789\ ]*) return 2 ;; esac
		read -r -a parsed_pids <<< "$redsocks_pid_output"
		[ "${#parsed_pids[@]}" -gt 0 ] || return 2
		for pid in "${parsed_pids[@]}"; do
			case "$pid" in ''|0*|1|*[!0-9]*) return 2 ;; esac
			[ "${#pid}" -le 10 ] || return 2
			[ -z "${redsocks_identity_start[$pid]+present}" ] || return 2
			redsocks_read_identity "$pid"
			status=$?
			[ "$status" -eq 0 ] || return 2
			redsocks_pids+=("$pid")
			redsocks_identity_start["$pid"]="$redsocks_identity_start_time"
		done
		return 0
		;;
	1) [ -z "$redsocks_pid_output" ] && return 1; return 2 ;;
	*) return 2 ;;
	esac
}

# Only signal members of func_stop's original PID/start-time snapshot. A
# replacement daemon or recycled PID makes the stop UNKNOWN rather than
# allowing a name-wide killall to hit an unrelated process.
redsocks_signal_snapshot()
{
	local requested_signal="$1" pid status
	if redsocks_process_state; then
		:
	else
		status=$?
		[ "$status" -eq 1 ] && return 0
		return 2
	fi
	for pid in "${redsocks_pids[@]}"; do
		[ -n "${redsocks_target_start[$pid]+present}" ] || return 2
		[ "${redsocks_identity_start[$pid]}" = "${redsocks_target_start[$pid]}" ] || return 2
	done
	for pid in "${redsocks_pids[@]}"; do
		redsocks_read_identity "$pid" || return 2
		[ "$redsocks_identity_start_time" = "${redsocks_target_start[$pid]}" ] || return 2
		kill "-$requested_signal" "$pid" >/dev/null 2>&1 || :
	done
	return 0
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

redsocks_ipv4_valid()
{
	local address="$1" octet
	local -a octets
	[[ "$address" =~ ^[0-9]{1,3}(\.[0-9]{1,3}){3}$ ]] || return 1
	IFS=. read -r -a octets <<< "$address"
	[ "${#octets[@]}" -eq 4 ] || return 1
	for octet in "${octets[@]}"; do
		# iptables/libxtables parses IPv4 components with base 0.
		case "$octet" in 0) ;; 0*) return 1 ;; esac
		[ "$octet" -le 255 ] 2>/dev/null || return 1
	done
	return 0
}

redsocks_iptables_delete_all()
{
	local table="$1" check_status attempt=0
	shift
	while [ "$attempt" -lt 64 ]; do
		if iptables -t "$table" -C "$@" >/dev/null 2>&1; then
			if iptables -t "$table" -D "$@" >/dev/null 2>&1; then
				:
			else
				return $?
			fi
		else
			check_status=$?
			[ "$check_status" -eq 1 ] && return 0
			return "$check_status"
		fi
		attempt=$((attempt + 1))
	done
	if iptables -t "$table" -C "$@" >/dev/null 2>&1; then
		return 1
	else
		check_status=$?
		[ "$check_status" -eq 1 ] && return 0
		return "$check_status"
	fi
}

func_clean()
{
	redsocks_iptables_delete_all nat PREROUTING -p tcp -j "$CHAIN_NAME" || return $?
	redsocks_iptables_delete_all nat OUTPUT -p tcp -j "$CHAIN_NAME" || return $?
	if iptables -t nat -S "$CHAIN_NAME" >/dev/null 2>&1; then
		iptables -t nat -F "$CHAIN_NAME" >/dev/null 2>&1 || return $?
		iptables -t nat -X "$CHAIN_NAME" >/dev/null 2>&1 || return $?
	else
		redsocks_chain_status=$?
		[ "$redsocks_chain_status" -eq 1 ] && return 0
		return "$redsocks_chain_status"
	fi
	return 0
}

redsocks_iptables_rollback()
{
	local original_status="$1" cleanup_status
	if func_clean; then
		:
	else
		cleanup_status=$?
		logger -t "$BINARY_NAME" "firewall rollback failed (status=$cleanup_status; original=$original_status)"
	fi
	return "$original_status"
}

redsocks_iptables_add_rule()
{
	local add_status
	if iptables -t nat -A "$@" >/dev/null 2>&1; then
		return 0
	else
		add_status=$?
	fi
	redsocks_iptables_rollback "$add_status"
}

redsocks_iptables_add_rule_once()
{
	local check_status
	if iptables -t nat -C "$@" >/dev/null 2>&1; then
		return 0
	else
		check_status=$?
	fi
	case "$check_status" in
		1) redsocks_iptables_add_rule "$@" ;;
		*) redsocks_iptables_rollback "$check_status" ;;
	esac
}

func_iptables()
{
	local ipset_entry ipset_status
	if [ -n "$ARG2" ]; then
		REMOTE_IP="$ARG2"
	else
		logger -t "$BINARY_NAME" "REMOTE_IP NOT FOUND!"
		return 2
	fi
	if ! redsocks_ipv4_valid "$REMOTE_IP"; then
		logger -t "$BINARY_NAME" "invalid REMOTE_IP: $REMOTE_IP"
		return 2
	fi
	if [ ! -d "$REDSOCKS_DIR" ]; then
		mkdir -p "$REDSOCKS_DIR" || return $?
	fi
	func_clean || return $?

	if [ ! -s "$CNZONE_FILE" ]; then
		wget -P "$REDSOCKS_DIR" http://www.ipdeny.com/ipblocks/data/countries/cn.zone || return $?
		[ -s "$CNZONE_FILE" ] || return 1
		func_save || return $?
	fi
	if [ ! -f "$LOCK" ]; then
		ipset destroy "$SET_NAME" >/dev/null 2>&1 || :
		ipset -N "$SET_NAME" hash:net || return $?
		while IFS= read -r ipset_entry || [ -n "$ipset_entry" ]; do
			[ -n "$ipset_entry" ] || continue
			if ipset -A "$SET_NAME" "$ipset_entry"; then
				:
			else
				ipset_status=$?
				ipset destroy "$SET_NAME" >/dev/null 2>&1 || :
				return "$ipset_status"
			fi
		done < "$CNZONE_FILE"
		if touch "$LOCK"; then
			:
		else
			ipset_status=$?
			ipset destroy "$SET_NAME" >/dev/null 2>&1 || :
			return "$ipset_status"
		fi
		logger -t "$BINARY_NAME" "SET LOCKED"
	fi

	iptables -t nat -N "$CHAIN_NAME" >/dev/null 2>&1 || return $?
	redsocks_iptables_add_rule "$CHAIN_NAME" -d "$REMOTE_IP" -j RETURN || return $?
	redsocks_iptables_add_rule "$CHAIN_NAME" -d 0.0.0.0/8 -j RETURN || return $?
	redsocks_iptables_add_rule "$CHAIN_NAME" -d 10.0.0.0/8 -j RETURN || return $?
	redsocks_iptables_add_rule "$CHAIN_NAME" -d 127.0.0.0/8 -j RETURN || return $?
	redsocks_iptables_add_rule "$CHAIN_NAME" -d 169.254.0.0/16 -j RETURN || return $?
	redsocks_iptables_add_rule "$CHAIN_NAME" -d 172.16.0.0/12 -j RETURN || return $?
	redsocks_iptables_add_rule "$CHAIN_NAME" -d 192.168.0.0/16 -j RETURN || return $?
	redsocks_iptables_add_rule "$CHAIN_NAME" -d 224.0.0.0/4 -j RETURN || return $?
	redsocks_iptables_add_rule "$CHAIN_NAME" -d 240.0.0.0/4 -j RETURN || return $?
	redsocks_iptables_add_rule "$CHAIN_NAME" -m set --match-set "$SET_NAME" dst -j RETURN || return $?
	redsocks_iptables_add_rule "$CHAIN_NAME" -p tcp -j REDIRECT --to-ports 12345 || return $?
	redsocks_iptables_add_rule_once PREROUTING -p tcp -j "$CHAIN_NAME" || return $?
	redsocks_iptables_add_rule_once OUTPUT -p tcp -j "$CHAIN_NAME" || return $?
	return 0
}

func_stop(){
	local pid
	local -A redsocks_target_start=()
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
	for pid in "${redsocks_pids[@]}"; do
		redsocks_target_start["$pid"]="${redsocks_identity_start[$pid]}"
	done
	if ! redsocks_signal_snapshot TERM; then
		logger -t "$BINARY_NAME" "daemon identity changed before TERM; refusing signal"
		return 1
	fi
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
	if ! redsocks_signal_snapshot KILL; then
		logger -t "$BINARY_NAME" "daemon identity changed before KILL; refusing escalation"
		return 1
	fi
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