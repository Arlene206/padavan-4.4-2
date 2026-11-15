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
func_start()
{
	# Avoid treating an inherited environment variable as our cleanup target.
	CONFIG_TMP=""
	CONFIG_TMP_OWNED=0
	# Make sure already running
	if [ -n "`pidof transmission-daemon`" ] ; then
		return 0
	fi
	
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
		echo "[  OK  ]"
		logger -t "$SVC_NAME" "daemon is started"
		return 0
	else
		echo "[FAILED]"
		logger -t "$SVC_NAME" "daemon start command failed" >/dev/null 2>&1
		return 1
	fi
}

func_stop()
{
	# Treat pidof failures as an unknown process state, not as proof of absence.
	tr_pid=`pidof transmission-daemon 2>/dev/null`
	tr_pidof_status=$?
	if [ "$tr_pidof_status" -gt 1 ] ||
		{ [ "$tr_pidof_status" -ne 0 ] && [ -n "$tr_pid" ]; }; then
		echo "[FAILED]"
		logger -t "$SVC_NAME" "Cannot stop: unable to inspect daemon PIDs" >/dev/null 2>&1 || true
		return 1
	fi
	if [ -z "$tr_pid" ]; then
		return 0
	fi

	echo -n "Stopping $SVC_NAME:."

	# A failed killall may be a race; the following PID snapshots determine
	# whether the daemon is actually stopped.
	killall -q transmission-daemon >/dev/null 2>&1 || true

	# Gracefully wait at most 25 seconds for the daemon to exit.
	i=0
	while [ -n "$tr_pid" ] && [ "$i" -le 25 ]; do
		echo -n "."
		sleep 1
		tr_pid=`pidof transmission-daemon 2>/dev/null`
		tr_pidof_status=$?
		if [ "$tr_pidof_status" -gt 1 ] ||
			{ [ "$tr_pidof_status" -ne 0 ] && [ -n "$tr_pid" ]; }; then
			echo "[FAILED]"
			logger -t "$SVC_NAME" "Cannot stop: unable to inspect daemon PIDs" >/dev/null 2>&1 || true
			return 1
		fi
		i=$((i + 1))
	done

	if [ -n "$tr_pid" ]; then
		# pidof returns a whitespace-separated PID list. Validate every token
		# before signaling so malformed output cannot become a kill argument.
		case "$tr_pid" in
			*[!0-9 ]*)
				echo "[FAILED]"
				logger -t "$SVC_NAME" "Cannot stop: invalid daemon PID list" >/dev/null 2>&1 || true
				return 1
				;;
		esac
		for tr_pid_one in $tr_pid; do
			kill -9 "$tr_pid_one" >/dev/null 2>&1 || true
		done
		sleep 1
		tr_pid=`pidof transmission-daemon 2>/dev/null`
		tr_pidof_status=$?
		if [ "$tr_pidof_status" -gt 1 ] ||
			{ [ "$tr_pidof_status" -ne 0 ] && [ -n "$tr_pid" ]; }; then
			echo "[FAILED]"
			logger -t "$SVC_NAME" "Cannot stop: unable to verify daemon exit" >/dev/null 2>&1 || true
			return 1
		fi
		if [ -n "$tr_pid" ]; then
			echo "[FAILED]"
			logger -t "$SVC_NAME" "Cannot stop: daemon PIDs remain after SIGKILL: $tr_pid" >/dev/null 2>&1 || true
			return 1
		fi
		echo "[KILLED]"
		logger -t "$SVC_NAME" "Daemon required SIGKILL after graceful stop timeout" >/dev/null 2>&1 || true
	else
		echo "[  OK  ]"
	fi
	return 0
}

func_reload()
{
	if [ -n "`pidof transmission-daemon`" ] ; then
		echo -n "Reload $SVC_NAME config:."
		killall -SIGHUP transmission-daemon
		echo "[  OK  ]"
	fi
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
	func_stop
	func_start
	;;
*)
	echo "Usage: $0 {start|stop|reload|restart}"
	exit 1
	;;
esac

