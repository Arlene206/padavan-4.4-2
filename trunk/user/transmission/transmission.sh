#!/bin/sh

#######################################################################
# (1) run process from superuser root (less security)
# (0) run process from unprivileged user "nobody" (more security)
SVC_ROOT=0

# process priority (0-normal, 19-lowest)
SVC_PRIORITY=5
#######################################################################

SVC_NAME="Transmission"
SVC_PATH="/usr/bin/transmission-daemon"
DIR_LINK="/mnt/transmission"

transmission_start_fail()
{
	# A failed noclobber open does not transfer ownership of a pre-existing path.
	if [ "$CONFIG_TMP_OWNED" = "1" ] && [ -n "$CONFIG_TMP" ]; then
		rm -f "$CONFIG_TMP" >/dev/null 2>&1 || true
	fi
	CONFIG_TMP=""
	CONFIG_TMP_OWNED=0
	logger -t "$SVC_NAME" "Cannot start: $1" >/dev/null 2>&1 || true
	echo "[FAILED]"
	return 1
}
transmission_require_dir()
{
	if [ ! -d "$1" ]; then
		if ! mkdir -p "$1" || [ ! -d "$1" ]; then
			transmission_start_fail "cannot create required directory: $1"
			return 1
		fi
	fi
	return 0
}

# Match Transmission only when its procfs executable and start time agree.
# Linux truncates TASK_COMM_LEN to 15 bytes ("transmission-da").
transmission_proc_stat()
{
	transmission_stat_pid="$1"
	transmission_stat_root="${TRANSMISSION_PROC_ROOT:-/proc}"
	transmission_stat_dir="$transmission_stat_root/$transmission_stat_pid"
	[ -d "$transmission_stat_dir" ] || return 1
	if IFS= read -r transmission_stat_line < "$transmission_stat_dir/stat"; then
		:
	else
		[ -d "$transmission_stat_dir" ] && return 2
		return 1
	fi
	case "$transmission_stat_line" in
		"$transmission_stat_pid ("*) ;;
		*) return 2 ;;
	esac
	transmission_stat_comm=${transmission_stat_line#*(}
	transmission_stat_comm=${transmission_stat_comm%)*}
	transmission_stat_fields=${transmission_stat_line##*) }
	if [ "${IFS+x}" = x ]; then
		transmission_stat_ifs_set=1
		transmission_stat_saved_ifs=$IFS
	else
		transmission_stat_ifs_set=0
	fi
	case "$-" in *f*) transmission_stat_glob_disabled=1 ;; *) transmission_stat_glob_disabled=0 ;; esac
	set -f
	IFS=' '
	set -- $transmission_stat_fields
	if [ "$transmission_stat_ifs_set" -eq 1 ]; then
		IFS=$transmission_stat_saved_ifs
	else
		unset IFS
	fi
	[ "$transmission_stat_glob_disabled" -eq 1 ] || set +f
	[ "$#" -ge 20 ] || return 2
	transmission_stat_state="$1"
	transmission_stat_start_time="${20}"
	case "$transmission_stat_state" in
		R|S|D|Z|T|t|X|x|K|W|P|I) ;;
		*) return 2 ;;
	esac
	case "$transmission_stat_start_time" in ''|*[!0-9]*) return 2 ;; esac
	case "$transmission_stat_state" in Z|X|x) return 1 ;; esac
	return 0
}

# Return 0 for the exact executable/start-time identity, 1 for absent or
# replaced PID, and 2 when procfs cannot provide a complete identity.
transmission_process_identity_matches()
{
	transmission_identity_pid="$1"
	transmission_identity_required_start="$2"
	transmission_identity_root="${TRANSMISSION_PROC_ROOT:-/proc}"
	case "$transmission_identity_pid" in ''|0*|*[!0-9]*) return 1 ;; esac
	transmission_identity_dir="$transmission_identity_root/$transmission_identity_pid"
	transmission_proc_stat "$transmission_identity_pid"
	transmission_identity_stat_status=$?
	[ "$transmission_identity_stat_status" -eq 0 ] || return "$transmission_identity_stat_status"
	case "$transmission_stat_comm" in
		transmission-da|transmission-daemon) ;;
		*) return 1 ;;
	esac
	if [ -z "$transmission_expected_exe" ]; then
		transmission_expected_exe=$(stat -L -c '%d:%i' "$SVC_PATH" 2>/dev/null) || return 2
	fi
	transmission_actual_exe=$(stat -L -c '%d:%i' "$transmission_identity_dir/exe" 2>/dev/null) || {
		[ -d "$transmission_identity_dir" ] && return 2
		return 1
	}
	[ "$transmission_actual_exe" = "$transmission_expected_exe" ] || return 1
	if [ -n "$transmission_identity_required_start" ] &&
	   [ "$transmission_stat_start_time" != "$transmission_identity_required_start" ]; then
		return 1
	fi
	transmission_identity_start_time="$transmission_stat_start_time"
	return 0
}

transmission_process_state_once()
{
	transmission_process_pids=
	transmission_process_root="${TRANSMISSION_PROC_ROOT:-/proc}"
	[ -d "$transmission_process_root" ] &&
		[ -r "$transmission_process_root" ] &&
		[ -x "$transmission_process_root" ] || return 2
	command -v stat >/dev/null 2>&1 || return 2
	transmission_expected_exe=$(stat -L -c '%d:%i' "$SVC_PATH" 2>/dev/null) || return 2
	case "$transmission_expected_exe" in
		''|*[!0-9:]*|:*|*:|*:*:*) return 2 ;;
	esac
	transmission_numeric_found=0
	for transmission_proc_dir in "$transmission_process_root"/[0-9]*; do
		[ -d "$transmission_proc_dir" ] || continue
		transmission_proc_pid=${transmission_proc_dir##*/}
		case "$transmission_proc_pid" in ''|0*|*[!0-9]*) return 2 ;; esac
		transmission_numeric_found=1
		if transmission_process_identity_matches "$transmission_proc_pid" ""; then
			transmission_process_pids="${transmission_process_pids}${transmission_process_pids:+ }$transmission_proc_pid:$transmission_identity_start_time"
		else
			transmission_identity_status=$?
			case "$transmission_identity_status" in
				1) ;;
				*) return 2 ;;
			esac
		fi
	done
	[ "$transmission_numeric_found" -eq 1 ] || return 2
	[ -n "$transmission_process_pids" ] && return 0
	return 1
}

transmission_process_state()
{
	if transmission_process_state_once; then
		return 0
	else
		transmission_process_state_status=$?
	fi
	case "$transmission_process_state_status" in
		1) transmission_process_state_once; return $? ;;
		*) return 2 ;;
	esac
}

transmission_wait_started()
{
	transmission_start_attempt=0
	while [ "$transmission_start_attempt" -lt 4 ]; do
		if transmission_process_state; then
			return 0
		else
			transmission_start_state=$?
		fi
		case "$transmission_start_state" in
			1) ;;
			*) return 2 ;;
		esac
		transmission_start_attempt=$((transmission_start_attempt + 1))
		[ "$transmission_start_attempt" -lt 4 ] || break
		sleep 1 || return 2
	done
	logger -t "$SVC_NAME" "launcher returned success but daemon did not appear" >/dev/null 2>&1 || true
	return 1
}

transmission_signal_identity()
{
	transmission_signal_name="$1"
	transmission_signal_pid="$2"
	transmission_signal_start="$3"
	case "$transmission_signal_name" in HUP|TERM|KILL|9) ;; *) return 2 ;; esac
	if transmission_process_identity_matches "$transmission_signal_pid" "$transmission_signal_start"; then
		:
	else
		transmission_signal_identity_status=$?
		[ "$transmission_signal_identity_status" -eq 1 ] && return 0
		return 2
	fi
	if kill "-$transmission_signal_name" "$transmission_signal_pid" >/dev/null 2>&1; then
		return 0
	else
		transmission_signal_status=$?
	fi
	if transmission_process_identity_matches "$transmission_signal_pid" "$transmission_signal_start"; then
		[ "$transmission_signal_status" -ne 0 ] || transmission_signal_status=1
		return "$transmission_signal_status"
	else
		transmission_signal_identity_status=$?
		[ "$transmission_signal_identity_status" -eq 1 ] && return 0
		return 2
	fi
}

transmission_signal_snapshot()
{
	transmission_snapshot_signal="$1"
	transmission_snapshot_pids="$2"
	for transmission_snapshot_entry in $transmission_snapshot_pids; do
		transmission_snapshot_pid=${transmission_snapshot_entry%%:*}
		transmission_snapshot_start=${transmission_snapshot_entry#*:}
		transmission_signal_identity "$transmission_snapshot_signal" \
			"$transmission_snapshot_pid" "$transmission_snapshot_start" || return $?
	done
	return 0
}

transmission_wait_stopped()
{
	transmission_wait_limit="$1"
	case "$transmission_wait_limit" in ''|*[!0-9]*) return 2 ;; esac
	transmission_wait_count=0
	while :; do
		if transmission_process_state; then
			transmission_wait_state=0
		else
			transmission_wait_state=$?
		fi
		case "$transmission_wait_state" in
			1) return 0 ;;
			0) ;;
			*) return 2 ;;
		esac
		[ "$transmission_wait_count" -lt "$transmission_wait_limit" ] || return 1
		sleep 1 || return 2
		transmission_wait_count=$((transmission_wait_count + 1))
	done
}

func_start()
{
	# Avoid treating an inherited environment variable as our cleanup target.
	CONFIG_TMP=""
	CONFIG_TMP_OWNED=0
	# Refuse duplicate launch on an exact executable identity; UNKNOWN is
	# never interpreted as a clean absence.
	if transmission_process_state; then
		return 0
	else
		tr_start_state=$?
	fi
	case "$tr_start_state" in
		1) ;;
		*) transmission_start_fail "cannot verify existing daemon identity"; return 2 ;;
	esac
	
	echo -n "Starting $SVC_NAME:."
	
	if [ ! -d "${DIR_LINK}" ] ; then
		echo "[FAILED], unable to find target dir!"
		return 1
	fi

	export TR_CURL_SSL_NO_VERIFY=1
	
	DIR_CFG="${DIR_LINK}/config"
	DIR_DL1="`cd \"$DIR_LINK\"; dirname \"$(pwd -P)\"`/Downloads"
	[ ! -d "$DIR_DL1" ] && DIR_DL1="${DIR_LINK}/downloads"
	DIR_DL2="${DIR_LINK}/incomplete"
	DIR_DL3="${DIR_LINK}/watch"
	
	# Do not start with missing storage directories or failed watch permissions.
	transmission_require_dir "$DIR_CFG" || return 1
	transmission_require_dir "$DIR_DL1" || return 1
	transmission_require_dir "$DIR_DL2" || return 1
	transmission_require_dir "$DIR_DL3" || return 1
	if ! chmod 777 "$DIR_DL3"; then
		transmission_start_fail "cannot set watch directory permissions"
		return 1
	fi
	
	tr_pport=`nvram get trmd_pport`
	tr_rport=`nvram get trmd_rport`
	
	# Generate default settings into a private same-directory file, then publish atomically.
	CONFIG_FILE="$DIR_CFG/settings.json"
	if [ -L "$CONFIG_FILE" ] || { [ -e "$CONFIG_FILE" ] && [ ! -f "$CONFIG_FILE" ]; }; then
		transmission_start_fail "settings path is not a regular file"
		return 1
	fi
	if [ ! -s "$CONFIG_FILE" ]; then
		CONFIG_TMP="${CONFIG_FILE}.tmp.$$"
		CONFIG_TMP_OWNED=0
		if ! (umask 077; set -C; : > "$CONFIG_TMP") 2>/dev/null; then
			transmission_start_fail "cannot create private settings staging file"
			return 1
		fi
		CONFIG_TMP_OWNED=1
		tr_user=`nvram get http_username`
		tr_pass=`nvram get http_passwd`
		if ! "$SVC_PATH" -w "$DIR_DL1" --incomplete-dir "$DIR_DL2" -c "$DIR_DL3" --no-incomplete-dir -y -L 90 -l 30 --no-utp -M -t -u "$tr_user" -v "$tr_pass" -P "$tr_pport" -p "$tr_rport" -x /var/run/transmission.pid -d 2>"$CONFIG_TMP"; then
			transmission_start_fail "default settings generator failed"
			return 1
		fi
		if [ ! -s "$CONFIG_TMP" ] || ! grep -q '"umask": 18,' "$CONFIG_TMP"; then
			transmission_start_fail "default settings output is empty or lacks the expected umask"
			return 1
		fi
		if ! sed -i 's|"umask": 18,|"umask": 0,|g' "$CONFIG_TMP"; then
			transmission_start_fail "default settings transform failed"
			return 1
		fi
		if [ ! -s "$CONFIG_TMP" ] || ! grep -q '"umask": 0,' "$CONFIG_TMP"; then
			transmission_start_fail "default settings transform output is empty or has an invalid umask"
			return 1
		fi
		if ! mv "$CONFIG_TMP" "$CONFIG_FILE"; then
			transmission_start_fail "default settings publication failed"
			return 1
		fi
		CONFIG_TMP=""
		CONFIG_TMP_OWNED=0
	fi
	if [ -L "$CONFIG_FILE" ] || [ ! -f "$CONFIG_FILE" ] || [ ! -s "$CONFIG_FILE" ]; then
		transmission_start_fail "published settings are missing or empty"
		return 1
	fi
	
	svc_user=""
	
	# check start-stop-daemon stuff
	if [ $SVC_ROOT -eq 0 ] ; then
		if ! chmod 777 "${DIR_LINK}"; then
			transmission_start_fail "cannot set transmission storage directory permissions"
			return 1
		fi
		if ! chown -R nobody "$DIR_CFG"; then
			transmission_start_fail "cannot assign config directory ownership"
			return 1
		fi
		if ! chown -R nobody "$DIR_DL2"; then
			transmission_start_fail "cannot assign incomplete directory ownership"
			return 1
		fi
		if ! chmod -R 777 "$DIR_DL1"; then
			transmission_start_fail "cannot set download directory permissions"
			return 1
		fi
		svc_user=" -c nobody"
	fi
	
	if start-stop-daemon -S -N $SVC_PRIORITY$svc_user -x $SVC_PATH -- -g "$DIR_CFG" -P "$tr_pport" -p "$tr_rport" -e "${DIR_LINK}/transmission.log" ; then
		if ! transmission_wait_started; then
			echo "[FAILED]"
			return 1
		fi
		echo "[  OK  ]"
		# Logging is best-effort: the daemon is already running and ready.
		logger -t "$SVC_NAME" "daemon is started" >/dev/null 2>&1 || true
		return 0
	else
		echo "[FAILED]"
		logger -t "$SVC_NAME" "daemon start command failed" >/dev/null 2>&1
		return 1
	fi
}

func_stop()
{
	if transmission_process_state; then
		tr_stop_state=0
	else
		tr_stop_state=$?
	fi
	case "$tr_stop_state" in
		1) return 0 ;;
		0) ;;
		*) echo "[FAILED]"; logger -t "$SVC_NAME" "Cannot stop: daemon identity is unknown" >/dev/null 2>&1 || true; return 2 ;;
	esac
	tr_stop_snapshot="$transmission_process_pids"
	echo -n "Stopping $SVC_NAME:."

	if transmission_signal_snapshot TERM "$tr_stop_snapshot"; then
		:
	else
		tr_signal_status=$?
		echo "[FAILED]"
		logger -t "$SVC_NAME" "Cannot stop: TERM failed or process identity became unknown (status=$tr_signal_status)" >/dev/null 2>&1 || true
		return "$tr_signal_status"
	fi
	if transmission_wait_stopped 25; then
		echo "[  OK  ]"
		return 0
	else
		tr_wait_status=$?
	fi
	case "$tr_wait_status" in
		1) ;;
		*) echo "[FAILED]"; logger -t "$SVC_NAME" "Cannot stop: process identity became unknown while waiting" >/dev/null 2>&1 || true; return 2 ;;
	esac

	# Refresh the exact executable/start-time snapshot before escalation.
	if transmission_process_state; then
		tr_stop_snapshot="$transmission_process_pids"
	else
		tr_stop_state=$?
		[ "$tr_stop_state" -eq 1 ] && { echo "[  OK  ]"; return 0; }
		echo "[FAILED]"
		logger -t "$SVC_NAME" "Cannot stop: daemon identity is unknown before SIGKILL" >/dev/null 2>&1 || true
		return 2
	fi
	if transmission_signal_snapshot 9 "$tr_stop_snapshot"; then
		:
	else
		tr_signal_status=$?
		echo "[FAILED]"
		logger -t "$SVC_NAME" "Cannot stop: SIGKILL failed or process identity became unknown (status=$tr_signal_status)" >/dev/null 2>&1 || true
		return "$tr_signal_status"
	fi
	if transmission_wait_stopped 2; then
		echo "[KILLED]"
		logger -t "$SVC_NAME" "Daemon required SIGKILL after graceful stop timeout" >/dev/null 2>&1 || true
		return 0
	else
		tr_wait_status=$?
	fi
	case "$tr_wait_status" in
		1) echo "[FAILED]"; logger -t "$SVC_NAME" "Cannot stop: verified daemon remains after SIGKILL" >/dev/null 2>&1 || true; return 1 ;;
		*) echo "[FAILED]"; logger -t "$SVC_NAME" "Cannot stop: daemon identity became unknown after SIGKILL" >/dev/null 2>&1 || true; return 2 ;;
	esac
}

func_reload()
{
	if transmission_process_state; then
		tr_reload_state=0
	else
		tr_reload_state=$?
	fi
	case "$tr_reload_state" in
		1) return 0 ;;
		0) ;;
		*) echo "[FAILED]"; logger -t "$SVC_NAME" "Cannot reload: daemon identity is unknown" >/dev/null 2>&1 || true; return 2 ;;
	esac
	tr_reload_snapshot="$transmission_process_pids"
	echo -n "Reload $SVC_NAME config:."
	tr_reload_status=0
	for tr_reload_entry in $tr_reload_snapshot; do
		tr_reload_pid=${tr_reload_entry%%:*}
		tr_reload_start=${tr_reload_entry#*:}
		if transmission_signal_identity HUP "$tr_reload_pid" "$tr_reload_start"; then
			:
		else
			tr_reload_signal_status=$?
			[ "$tr_reload_signal_status" -eq 2 ] && {
				echo "[FAILED]"
				logger -t "$SVC_NAME" "Cannot reload: process identity became unknown" >/dev/null 2>&1 || true
				return 2
			}
			if [ "$tr_reload_status" -eq 0 ]; then
				tr_reload_status=$tr_reload_signal_status
				[ "$tr_reload_status" -ne 0 ] || tr_reload_status=1
			fi
		fi
	done
	if [ "$tr_reload_status" -eq 0 ]; then
		echo "[  OK  ]"
		return 0
	fi
	echo "[FAILED]"
	logger -t "$SVC_NAME" "daemon reload signal failed (status=$tr_reload_status)" >/dev/null 2>&1 || true
	return "$tr_reload_status"
}


case "$1" in
start)
	func_start
	;;
stop)
	func_stop
	;;
reload)
	func_reload
	;;
restart)
	func_stop || exit 1
	func_start
	;;
*)
	echo "Usage: $0 {start|stop|reload|restart}"
	exit 1
	;;
esac

