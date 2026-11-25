#!/bin/sh

#######################################################################
# (1) run process from superuser root (less security)
# (0) run process from unprivileged user "nobody" (more security)
SVC_ROOT=0

# process priority (0-normal, 19-lowest)
SVC_PRIORITY=3
#######################################################################

SVC_NAME="Aria2"
SVC_PATH="/usr/bin/aria2c"
DIR_LINK="/mnt/aria"


# aria2c is explicitly daemonized with -D; start-stop-daemon returning only
# confirms that the short-lived launcher exited. Require the child PID.
aria_wait_started()
{
	attempt=0
	while [ "$attempt" -lt 4 ]; do
		if aria_ready_pids=$(pidof aria2c 2>/dev/null); then
			aria_ready_status=0
		else
			aria_ready_status=$?
		fi
		case "$aria_ready_status" in
		0)
			if [ -z "$aria_ready_pids" ]; then
				logger -t "$SVC_NAME" "Cannot verify startup: pidof returned success without a PID"
				return 1
			fi
			case "$aria_ready_pids" in
				*[!0-9\ ]*)
					logger -t "$SVC_NAME" "Cannot verify startup: invalid daemon PID list"
					return 1
					;;
			esac
			for aria_ready_pid in $aria_ready_pids; do
				case "$aria_ready_pid" in
					0*|1|*[!0-9]*)
						logger -t "$SVC_NAME" "Cannot verify startup: unsafe daemon PID"
						return 1
					;;
				esac
				done
			return 0
			;;
		1)
			if [ -n "$aria_ready_pids" ]; then
				logger -t "$SVC_NAME" "Cannot verify startup: inconsistent daemon process state"
				return 1
			fi
			;;
		*)
			logger -t "$SVC_NAME" "Cannot verify startup: pidof failed (status=$aria_ready_status)"
			return 1
			;;
		esac
		attempt=$((attempt + 1))
		[ "$attempt" -lt 4 ] || break
		sleep 1 || {
			logger -t "$SVC_NAME" "Cannot verify startup: readiness wait was interrupted"
			return 1
		}
	done

	logger -t "$SVC_NAME" "launcher returned success but daemon did not appear"
	return 1
}


aria_generate_rpc_secret()
{
	aria_rpc_secret=$(hexdump -v -n 16 -e '16/1 "%02x"' /dev/urandom 2>/dev/null) || return 1
	case "$aria_rpc_secret" in
		''|*[!0123456789abcdef]*) return 1 ;;
	esac
	[ "${#aria_rpc_secret}" -eq 32 ]
}

aria_read_rpc_secret()
{
	if [ -L "$aria_rpc_secret_file" ] || [ ! -f "$aria_rpc_secret_file" ] || [ ! -r "$aria_rpc_secret_file" ]; then
		return 1
	fi
	if ! chmod 600 "$aria_rpc_secret_file"; then
		return 1
	fi
	aria_secret_bytes=$(wc -c < "$aria_rpc_secret_file" 2>/dev/null) || return 1
	aria_secret_lines=$(wc -l < "$aria_rpc_secret_file" 2>/dev/null) || return 1
	[ "$aria_secret_bytes" -eq 33 ] && [ "$aria_secret_lines" -eq 1 ] || return 1
	IFS= read -r aria_rpc_secret < "$aria_rpc_secret_file" || return 1
	case "$aria_rpc_secret" in
		''|*[!0123456789abcdef]*) return 1 ;;
	esac
	[ "${#aria_rpc_secret}" -eq 32 ]
}

aria_load_or_create_rpc_secret()
{
	aria_rpc_secret_file="${DIR_CFG}/aria2.rpc-secret"
	if [ -L "$aria_rpc_secret_file" ]; then
		logger -t "$SVC_NAME" "Cannot start: RPC secret path is a symlink" >/dev/null 2>&1 || true
		return 1
	fi
	if [ -e "$aria_rpc_secret_file" ]; then
		if ! aria_read_rpc_secret; then
			logger -t "$SVC_NAME" "Cannot start: stored RPC secret is invalid" >/dev/null 2>&1 || true
			return 1
		fi
		return 0
	fi

	if ! aria_generate_rpc_secret; then
		logger -t "$SVC_NAME" "Cannot start: cannot generate a random RPC secret" >/dev/null 2>&1 || true
		return 1
	fi
	aria_generated_secret=$aria_rpc_secret
	aria_secret_tmp="${aria_rpc_secret_file}.tmp.$$"
	if ! (umask 077; set -C; : > "$aria_secret_tmp") 2>/dev/null; then
		logger -t "$SVC_NAME" "Cannot create private RPC secret staging file" >/dev/null 2>&1 || true
		return 1
	fi
	if ! printf '%s\n' "$aria_generated_secret" > "$aria_secret_tmp" ||
	   ! chmod 600 "$aria_secret_tmp"; then
		rm -f "$aria_secret_tmp"
		logger -t "$SVC_NAME" "Cannot stage private RPC secret" >/dev/null 2>&1 || true
		return 1
	fi

	if ln "$aria_secret_tmp" "$aria_rpc_secret_file" 2>/dev/null; then
		rm -f "$aria_secret_tmp"
		aria_rpc_secret=$aria_generated_secret
		return 0
	fi
	rm -f "$aria_secret_tmp"
	if ! aria_read_rpc_secret; then
		logger -t "$SVC_NAME" "Cannot publish or read the private RPC secret" >/dev/null 2>&1 || true
		return 1
	fi
	return 0
}

aria_secure_rpc_config()
{
	if [ -L "$FILE_CONF" ] || [ ! -f "$FILE_CONF" ] || [ ! -r "$FILE_CONF" ]; then
		logger -t "$SVC_NAME" "Cannot start: RPC config is not a readable regular file" >/dev/null 2>&1 || true
		return 1
	fi

	aria_rpc_listener_all=0
	if grep -Eq '^[[:space:]]*rpc-listen-all[[:space:]]*=[[:space:]]*true([[:space:]]*(#.*)?)?$' "$FILE_CONF"; then
		aria_rpc_listener_all=1
	else
		aria_grep_status=$?
		[ "$aria_grep_status" -eq 1 ] || {
			logger -t "$SVC_NAME" "Cannot inspect RPC listener config (status=$aria_grep_status)" >/dev/null 2>&1 || true
			return 1
		}
	fi

	aria_rpc_authenticated=0
	if grep -Eq '^[[:space:]]*rpc-secret[[:space:]]*=[[:space:]]*[^[:space:]]' "$FILE_CONF"; then
		aria_rpc_authenticated=1
	else
		aria_grep_status=$?
		[ "$aria_grep_status" -eq 1 ] || {
			logger -t "$SVC_NAME" "Cannot inspect RPC secret config (status=$aria_grep_status)" >/dev/null 2>&1 || true
			return 1
		}
	fi
	if [ "$aria_rpc_authenticated" -eq 0 ]; then
		if grep -Eq '^[[:space:]]*rpc-user[[:space:]]*=[[:space:]]*[^[:space:]]' "$FILE_CONF"; then
			if grep -Eq '^[[:space:]]*rpc-passwd[[:space:]]*=[[:space:]]*[^[:space:]]' "$FILE_CONF"; then
				aria_rpc_authenticated=1
			else
				aria_grep_status=$?
				[ "$aria_grep_status" -eq 1 ] || {
					logger -t "$SVC_NAME" "Cannot inspect RPC password config (status=$aria_grep_status)" >/dev/null 2>&1 || true
					return 1
				}
			fi
		else
			aria_grep_status=$?
			[ "$aria_grep_status" -eq 1 ] || {
				logger -t "$SVC_NAME" "Cannot inspect RPC user config (status=$aria_grep_status)" >/dev/null 2>&1 || true
				return 1
			}
		fi
	fi

	if [ "$aria_rpc_authenticated" -eq 1 ]; then
		if ! chmod 600 "$FILE_CONF"; then
			logger -t "$SVC_NAME" "Cannot protect existing RPC credentials" >/dev/null 2>&1 || true
			return 1
		fi
		return 0
	fi
	if [ "$aria_rpc_listener_all" -eq 0 ]; then
		return 0
	fi

	if ! aria_load_or_create_rpc_secret; then
		return 1
	fi
	CONFIG_TMP="${FILE_CONF}.rpc-auth.tmp.$$"
	if ! (umask 077; set -C; : > "$CONFIG_TMP") 2>/dev/null; then
		logger -t "$SVC_NAME" "Cannot create private RPC config staging file" >/dev/null 2>&1 || true
		return 1
	fi
	if ! sed '/^[[:space:]]*rpc-secret[[:space:]]*=/d' "$FILE_CONF" > "$CONFIG_TMP" ||
	   ! printf '\n# Migrated by Aria2 launcher: use the token in aria2.rpc-secret for AriaNg.\nrpc-secret=%s\n' "$aria_rpc_secret" >> "$CONFIG_TMP"; then
		rm -f "$CONFIG_TMP"
		logger -t "$SVC_NAME" "Cannot stage authenticated RPC config" >/dev/null 2>&1 || true
		return 1
	fi
	if ! chmod 600 "$CONFIG_TMP" || ! mv -f "$CONFIG_TMP" "$FILE_CONF"; then
		rm -f "$CONFIG_TMP"
		logger -t "$SVC_NAME" "Cannot publish protected RPC config" >/dev/null 2>&1 || true
		return 1
	fi
	logger -t "$SVC_NAME" "Migrated unauthenticated all-interface RPC config; set AriaNg RPC Secret Token from aria2.rpc-secret" >/dev/null 2>&1 || true
	return 0
}
func_start()
{
	# Resolve the daemon state from both pidof's status and output.
	aria_start_pids=$(pidof aria2c 2>/dev/null)
	aria_start_pidof_status=$?
	case "$aria_start_pidof_status" in
		0)
			if [ -z "$aria_start_pids" ]; then
				echo "[FAILED]"
				logger -t "$SVC_NAME" "Cannot start: unable to determine daemon process state" >/dev/null 2>&1 || true
				return 1
			fi
			case "$aria_start_pids" in
				*[!0-9\ ]*)
					echo "[FAILED]"
					logger -t "$SVC_NAME" "Cannot start: invalid daemon PID list" >/dev/null 2>&1 || true
					return 1
					;;
			esac
			aria_start_pid_count=0
			for aria_start_pid in $aria_start_pids; do
				aria_start_pid_count=$((aria_start_pid_count + 1))
				case "$aria_start_pid" in
					0*|1|*[!0-9]*)
						echo "[FAILED]"
						logger -t "$SVC_NAME" "Cannot start: invalid daemon PID list" >/dev/null 2>&1 || true
						return 1
						;;
				esac
			done
			if [ "$aria_start_pid_count" -eq 0 ]; then
				echo "[FAILED]"
				logger -t "$SVC_NAME" "Cannot start: invalid daemon PID list" >/dev/null 2>&1 || true
				return 1
			fi
			return 0
			;;
		1)
			if [ -n "$aria_start_pids" ]; then
				echo "[FAILED]"
				logger -t "$SVC_NAME" "Cannot start: inconsistent daemon process state" >/dev/null 2>&1 || true
				return 1
			fi
			;;
		*)
			echo "[FAILED]"
			logger -t "$SVC_NAME" "Cannot start: unable to determine daemon process state" >/dev/null 2>&1 || true
			return 1
			;;
	esac

	echo -n "Starting $SVC_NAME:."

	if [ ! -d "${DIR_LINK}" ] ; then
		echo "[FAILED]"
		logger -t "$SVC_NAME" "Cannot start: unable to find target dir!"
		return 1
	fi

	DIR_CFG="${DIR_LINK}/config"
	DIR_DL1="`cd \"$DIR_LINK\"; dirname \"$(pwd -P)\"`/Downloads"
	[ ! -d "$DIR_DL1" ] && DIR_DL1="${DIR_LINK}/downloads"

	if [ ! -d "$DIR_CFG" ] && ! mkdir -p "$DIR_CFG" ; then
		logger -t "$SVC_NAME" "Cannot start: unable to create config directory" >/dev/null 2>&1
		return 1
	fi

	FILE_CONF="$DIR_CFG/aria2.conf"
	FILE_LIST="$DIR_CFG/incomplete.lst"

	if ! touch "$FILE_LIST" ; then
		logger -t "$SVC_NAME" "Cannot start: unable to create session list" >/dev/null 2>&1
		return 1
	fi

	aria_pport=`nvram get aria_pport`
	aria_rport=`nvram get aria_rport`

	[ -z "$aria_rport" ] && aria_rport="6800"
	[ -z "$aria_pport" ] && aria_pport="16888"

	if [ ! -f "$FILE_CONF" ] ; then
		if ! aria_load_or_create_rpc_secret; then return 1; fi
		if [ ! -d "$DIR_DL1" ] && ! mkdir -p "$DIR_DL1" ; then
			logger -t "$SVC_NAME" "Cannot start: unable to create downloads directory" >/dev/null 2>&1
			return 1
		fi
		if ! chmod -R 777 "$DIR_DL1" ; then
			logger -t "$SVC_NAME" "Cannot start: unable to set downloads directory permissions" >/dev/null 2>&1
			return 1
		fi
		CONFIG_TMP="${FILE_CONF}.tmp.$$"
		if ! (umask 077; set -C; : > "$CONFIG_TMP") 2>/dev/null; then
			logger -t "$SVC_NAME" "Cannot create private default config staging file" >/dev/null 2>&1 || true
			return 1
		fi
		if ! cat > "$CONFIG_TMP" <<EOF

### XML-RPC
rpc-listen-all=true
rpc-allow-origin-all=true
# AriaNg: copy aria2.rpc-secret into the RPC Secret Token setting.
rpc-secret=$aria_rpc_secret
### Common
dir=$DIR_DL1
max-download-limit=0
max-overall-download-limit=0
disable-ipv6=false

### File
file-allocation=trunc
#file-allocation=falloc
#file-allocation=none
no-file-allocation-limit=10M
allow-overwrite=false
auto-file-renaming=true

### Bittorent
bt-enable-lpd=false
#bt-lpd-interface=eth2.2
bt-max-peers=50
bt-max-open-files=100
bt-request-peer-speed-limit=100K
bt-stop-timeout=0
enable-dht=true
#enable-dht6=false
enable-peer-exchange=true
seed-ratio=1.5
#seed-time=60
max-upload-limit=0
max-overall-upload-limit=0

### FTP/HTTP
ftp-pasv=true
ftp-type=binary
timeout=120
connect-timeout=60
split=8
max-concurrent-downloads=3
max-connection-per-server=8
min-split-size=1M
check-certificate=false

### Log
log=$DIR_CFG/aria2.log
log-level=notice

EOF
		then
			rm -f "$CONFIG_TMP"
			logger -t "$SVC_NAME" "Cannot start: failed to write default config" >/dev/null 2>&1
			return 1
		fi
		if ! chmod 600 "$CONFIG_TMP"; then
			rm -f "$CONFIG_TMP"
			logger -t "$SVC_NAME" "Cannot protect default RPC config" >/dev/null 2>&1 || true
			return 1
		fi
		if ! mv -f "$CONFIG_TMP" "$FILE_CONF"; then
			rm -f "$CONFIG_TMP"
			logger -t "$SVC_NAME" "Cannot publish default config" >/dev/null 2>&1 || true
			return 1
		fi
	fi

	if ! aria_secure_rpc_config; then
		echo "[FAILED]"
		return 1
	fi

	# aria2 needed home dir
	export HOME="$DIR_CFG"

	if [ "`nvram get http_proto`" != "0" ]; then
		SVC_ROOT=1
		SSL_OPT="--rpc-secure=true --rpc-certificate=/etc/storage/https/server.crt --rpc-private-key=/etc/storage/https/server.key"
	else
		SSL_OPT=
	fi

	svc_user=""

	if [ $SVC_ROOT -eq 0 ] ; then
		if ! chmod 777 "${DIR_LINK}" || ! chown -R nobody "$DIR_CFG" ; then
			logger -t "$SVC_NAME" "Cannot start: unable to set service ownership/permissions" >/dev/null 2>&1
			return 1
		fi
		svc_user=" -c nobody"
	fi

	if start-stop-daemon -S -N $SVC_PRIORITY$svc_user -x $SVC_PATH -- \
		-D --enable-rpc=true --conf-path="$FILE_CONF" --input-file="$FILE_LIST" --save-session="$FILE_LIST" \
		--rpc-listen-port="$aria_rport" --listen-port="$aria_pport" --dht-listen-port="$aria_pport" $SSL_OPT ; then
		if ! aria_wait_started; then
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
	# pidof exit 1 with no output means absent; every other inconsistent
	# result is UNKNOWN and must not be treated as a successful stop.
	aria_pid=`pidof aria2c 2>/dev/null`
	aria_pidof_status=$?
	if [ "$aria_pidof_status" -gt 1 ] ||
	    { [ "$aria_pidof_status" -eq 0 ] && [ -z "$aria_pid" ]; } ||
	    { [ "$aria_pidof_status" -ne 0 ] && [ -n "$aria_pid" ]; }; then
		echo "[FAILED]"
		logger -t "$SVC_NAME" "Cannot stop: cannot determine daemon process state" >/dev/null 2>&1 || true
		return 1
	fi
	if [ -z "$aria_pid" ]; then
		return 0
	fi

	# Validate the complete PID snapshot before the broad graceful signal.
	case "$aria_pid" in
		*[!0-9\ ]*)
			echo "[FAILED]"
			logger -t "$SVC_NAME" "Cannot stop: invalid daemon PID list" >/dev/null 2>&1 || true
			return 1
			;;
	esac
	aria_pid_count=0
	for aria_pid_one in $aria_pid; do
		case "$aria_pid_one" in
			''|0*|1|*[!0-9]*)
				echo "[FAILED]"
				logger -t "$SVC_NAME" "Cannot stop: invalid daemon PID list" >/dev/null 2>&1 || true
				return 1
				;;
		esac
		aria_pid_count=$((aria_pid_count + 1))
	done
	if [ "$aria_pid_count" -eq 0 ]; then
		echo "[FAILED]"
		logger -t "$SVC_NAME" "Cannot stop: empty daemon PID list" >/dev/null 2>&1 || true
		return 1
	fi

	echo -n "Stopping $SVC_NAME:."

	# Ask the daemon to stop gracefully; process presence is authoritative.
	killall -q aria2c >/dev/null 2>&1 || true

	# Gracefully wait at most 15 seconds while aria2c stops.
	i=0
	while [ -n "$aria_pid" ] && [ "$i" -lt 15 ]; do
		echo -n "."
		sleep 1
		aria_pid=`pidof aria2c 2>/dev/null`
		aria_pidof_status=$?
		if [ "$aria_pidof_status" -gt 1 ] ||
		    { [ "$aria_pidof_status" -eq 0 ] && [ -z "$aria_pid" ]; } ||
		    { [ "$aria_pidof_status" -ne 0 ] && [ -n "$aria_pid" ]; }; then
			echo "[FAILED]"
			logger -t "$SVC_NAME" "Cannot stop: process inventory became unknown" >/dev/null 2>&1 || true
			return 1
		fi
		i=$((i + 1))
	done

	if [ -n "$aria_pid" ]; then
		# Validate the refreshed snapshot before sending individual signals.
		case "$aria_pid" in
			*[!0-9\ ]*)
				echo "[FAILED]"
				logger -t "$SVC_NAME" "Cannot stop: invalid daemon PID list" >/dev/null 2>&1 || true
				return 1
				;;
		esac
		aria_pid_count=0
		for aria_pid_one in $aria_pid; do
			case "$aria_pid_one" in
				''|0*|1|*[!0-9]*)
					echo "[FAILED]"
					logger -t "$SVC_NAME" "Cannot stop: invalid daemon PID list" >/dev/null 2>&1 || true
					return 1
					;;
			esac
			aria_pid_count=$((aria_pid_count + 1))
		done
		if [ "$aria_pid_count" -eq 0 ]; then
			echo "[FAILED]"
			logger -t "$SVC_NAME" "Cannot stop: empty daemon PID list" >/dev/null 2>&1 || true
			return 1
		fi
		for aria_pid_one in $aria_pid; do
			kill -9 "$aria_pid_one" >/dev/null 2>&1 || true
		done

		# SIGKILL is asynchronous; verify daemon absence before reporting success.
		sleep 1
		aria_pid=`pidof aria2c 2>/dev/null`
		aria_pidof_status=$?
		if [ "$aria_pidof_status" -gt 1 ] ||
		    { [ "$aria_pidof_status" -eq 0 ] && [ -z "$aria_pid" ]; } ||
		    { [ "$aria_pidof_status" -ne 0 ] && [ -n "$aria_pid" ]; }; then
			echo "[FAILED]"
			logger -t "$SVC_NAME" "Cannot stop: process inventory unknown after SIGKILL" >/dev/null 2>&1 || true
			return 1
		fi
		if [ -n "$aria_pid" ]; then
			echo "[FAILED]"
			logger -t "$SVC_NAME" "Cannot stop: processes remain after SIGKILL: $aria_pid" >/dev/null 2>&1 || true
			return 1
		fi
		echo "[KILLED]"
		logger -t "$SVC_NAME" "Cannot stop: Timeout reached! Force killed." >/dev/null 2>&1 || true
	else
		echo "[  OK  ]"
	fi

	return 0
}

func_reload()
{
	# A failed pidof scan is not proof that aria2c is absent. Validate the
	# entire PID snapshot before sending any signal.
	aria_reload_pids=`pidof aria2c 2>/dev/null`
	aria_reload_pidof_status=$?
	case "$aria_reload_pidof_status" in
		0)
			if [ -z "$aria_reload_pids" ]; then
				echo "Error: unable to determine $SVC_NAME process state!"
				logger -t "$SVC_NAME" "Cannot reload: pidof returned success without a PID" >/dev/null 2>&1 || true
				return 1
			fi
			case "$aria_reload_pids" in
				*[!0-9\ ]*)
					echo "Error: invalid $SVC_NAME process list!"
					logger -t "$SVC_NAME" "Cannot reload: invalid daemon PID list" >/dev/null 2>&1 || true
					return 1
					;;
			esac
			aria_reload_pid_count=0
			for aria_reload_pid in $aria_reload_pids; do
				aria_reload_pid_count=$((aria_reload_pid_count + 1))
				case "$aria_reload_pid" in
					''|0*|1|*[!0-9]*)
						echo "Error: invalid $SVC_NAME process list!"
						logger -t "$SVC_NAME" "Cannot reload: unsafe daemon PID" >/dev/null 2>&1 || true
						return 1
						;;
				esac
			done
			if [ "$aria_reload_pid_count" -eq 0 ]; then
				echo "Error: unable to determine $SVC_NAME process state!"
				logger -t "$SVC_NAME" "Cannot reload: empty daemon PID list" >/dev/null 2>&1 || true
				return 1
			fi
			;;
		1)
			if [ -z "$aria_reload_pids" ]; then
				echo "Error: $SVC_NAME is not started!"
			else
				echo "Error: unable to determine $SVC_NAME process state!"
			fi
			return 1
			;;
		*)
			echo "Error: unable to determine $SVC_NAME process state!"
			logger -t "$SVC_NAME" "Cannot reload: pidof failed (status=$aria_reload_pidof_status)" >/dev/null 2>&1 || true
			return 1
			;;
	esac

	echo -n "Reload $SVC_NAME config:."
	for aria_reload_pid in $aria_reload_pids; do
		if kill -1 "$aria_reload_pid"; then
			:
		else
			aria_reload_kill_status=$?
			echo "[FAILED]"
			logger -t "$SVC_NAME" "Cannot reload: SIGHUP failed for PID $aria_reload_pid (status=$aria_reload_kill_status)" >/dev/null 2>&1 || true
			return "$aria_reload_kill_status"
		fi
	done
	echo "[  OK  ]"
	return 0
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
