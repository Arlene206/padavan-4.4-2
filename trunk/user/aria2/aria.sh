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
# Match Aria2 only when its procfs executable and start time agree.
# Linux comm is checked separately from the executable inode.
aria_proc_stat()
{
	aria_stat_pid="$1"
	aria_stat_root="${ARIA_PROC_ROOT:-/proc}"
	aria_stat_dir="$aria_stat_root/$aria_stat_pid"
	[ -d "$aria_stat_dir" ] || return 1
	if IFS= read -r aria_stat_line < "$aria_stat_dir/stat"; then
		:
	else
		[ -d "$aria_stat_dir" ] && return 2
		return 1
	fi
	case "$aria_stat_line" in
		"$aria_stat_pid ("*) ;;
		*) return 2 ;;
	esac
	aria_stat_comm=${aria_stat_line#*(}
	aria_stat_comm=${aria_stat_comm%)*}
	aria_stat_fields=${aria_stat_line##*) }
	if [ "${IFS+x}" = x ]; then
		aria_stat_ifs_set=1
		aria_stat_saved_ifs=$IFS
	else
		aria_stat_ifs_set=0
	fi
	case "$-" in *f*) aria_stat_glob_disabled=1 ;; *) aria_stat_glob_disabled=0 ;; esac
	set -f
	IFS=' '
	set -- $aria_stat_fields
	if [ "$aria_stat_ifs_set" -eq 1 ]; then
		IFS=$aria_stat_saved_ifs
	else
		unset IFS
	fi
	[ "$aria_stat_glob_disabled" -eq 1 ] || set +f
	[ "$#" -ge 20 ] || return 2
	aria_stat_state="$1"
	aria_stat_start_time="${20}"
	case "$aria_stat_state" in
		R|S|D|Z|T|t|X|x|K|W|P|I) ;;
		*) return 2 ;;
	esac
	case "$aria_stat_start_time" in ''|*[!0-9]*) return 2 ;; esac
	case "$aria_stat_state" in Z|X|x) return 1 ;; esac
	return 0
}

# Return 0 for the exact executable/start-time identity, 1 for absent or
# replaced PID, and 2 when procfs cannot provide a complete identity.
aria_process_identity_matches()
{
	aria_identity_pid="$1"
	aria_identity_required_start="$2"
	aria_identity_root="${ARIA_PROC_ROOT:-/proc}"
	case "$aria_identity_pid" in ''|0*|*[!0-9]*) return 1 ;; esac
	aria_identity_dir="$aria_identity_root/$aria_identity_pid"
	aria_proc_stat "$aria_identity_pid"
	aria_identity_stat_status=$?
	[ "$aria_identity_stat_status" -eq 0 ] || return "$aria_identity_stat_status"
	case "$aria_stat_comm" in
		aria2c) ;;
		*) return 1 ;;
	esac
	if [ -z "$aria_expected_exe" ]; then
		aria_expected_exe=$(stat -L -c '%d:%i' "$SVC_PATH" 2>/dev/null) || return 2
	fi
	aria_actual_exe=$(stat -L -c '%d:%i' "$aria_identity_dir/exe" 2>/dev/null) || {
		[ -d "$aria_identity_dir" ] && return 2
		return 1
	}
	[ "$aria_actual_exe" = "$aria_expected_exe" ] || return 1
	if [ -n "$aria_identity_required_start" ] &&
	   [ "$aria_stat_start_time" != "$aria_identity_required_start" ]; then
		return 1
	fi
	aria_identity_start_time="$aria_stat_start_time"
	return 0
}

aria_process_state_once()
{
	aria_process_pids=
	aria_process_root="${ARIA_PROC_ROOT:-/proc}"
	[ -d "$aria_process_root" ] &&
		[ -r "$aria_process_root" ] &&
		[ -x "$aria_process_root" ] || return 2
	command -v stat >/dev/null 2>&1 || return 2
	aria_expected_exe=$(stat -L -c '%d:%i' "$SVC_PATH" 2>/dev/null) || return 2
	case "$aria_expected_exe" in
		''|*[!0-9:]*|:*|*:|*:*:*) return 2 ;;
	esac
	aria_numeric_found=0
	for aria_proc_dir in "$aria_process_root"/[0-9]*; do
		[ -d "$aria_proc_dir" ] || continue
		aria_proc_pid=${aria_proc_dir##*/}
		case "$aria_proc_pid" in ''|0*|*[!0-9]*) return 2 ;; esac
		aria_numeric_found=1
		if aria_process_identity_matches "$aria_proc_pid" ""; then
			aria_process_pids="${aria_process_pids}${aria_process_pids:+ }$aria_proc_pid:$aria_identity_start_time"
		else
			aria_identity_status=$?
			case "$aria_identity_status" in
				1) ;;
				*) return 2 ;;
			esac
		fi
	done
	[ "$aria_numeric_found" -eq 1 ] || return 2
	[ -n "$aria_process_pids" ] && return 0
	return 1
}

aria_process_state()
{
	if aria_process_state_once; then
		return 0
	else
		aria_process_state_status=$?
	fi
	case "$aria_process_state_status" in
		1) aria_process_state_once; return $? ;;
		*) return 2 ;;
	esac
}

aria_wait_started()
{
	aria_start_attempt=0
	while [ "$aria_start_attempt" -lt 4 ]; do
		if aria_process_state; then
			return 0
		else
			aria_start_state=$?
		fi
		case "$aria_start_state" in
			1) ;;
			*) return 2 ;;
		esac
		aria_start_attempt=$((aria_start_attempt + 1))
		[ "$aria_start_attempt" -lt 4 ] || break
		sleep 1 || return 2
	done
	logger -t "$SVC_NAME" "launcher returned success but daemon did not appear" >/dev/null 2>&1 || true
	return 1
}

aria_signal_identity()
{
	aria_signal_name="$1"
	aria_signal_pid="$2"
	aria_signal_start="$3"
	case "$aria_signal_name" in HUP|TERM|KILL|9) ;; *) return 2 ;; esac
	if aria_process_identity_matches "$aria_signal_pid" "$aria_signal_start"; then
		:
	else
		aria_signal_identity_status=$?
		[ "$aria_signal_identity_status" -eq 1 ] && return 0
		return 2
	fi
	if kill "-$aria_signal_name" "$aria_signal_pid" >/dev/null 2>&1; then
		return 0
	else
		aria_signal_status=$?
	fi
	if aria_process_identity_matches "$aria_signal_pid" "$aria_signal_start"; then
		[ "$aria_signal_status" -ne 0 ] || aria_signal_status=1
		return "$aria_signal_status"
	else
		aria_signal_identity_status=$?
		[ "$aria_signal_identity_status" -eq 1 ] && return 0
		return 2
	fi
}

aria_signal_snapshot()
{
	aria_snapshot_signal="$1"
	aria_snapshot_pids="$2"
	for aria_snapshot_entry in $aria_snapshot_pids; do
		aria_snapshot_pid=${aria_snapshot_entry%%:*}
		aria_snapshot_start=${aria_snapshot_entry#*:}
		aria_signal_identity "$aria_snapshot_signal" \
			"$aria_snapshot_pid" "$aria_snapshot_start" || return $?
	done
	return 0
}

aria_wait_stopped()
{
	aria_wait_limit="$1"
	case "$aria_wait_limit" in ''|*[!0-9]*) return 2 ;; esac
	aria_wait_count=0
	while :; do
		if aria_process_state; then
			aria_wait_state=0
		else
			aria_wait_state=$?
		fi
		case "$aria_wait_state" in
			1) return 0 ;;
			0) ;;
			*) return 2 ;;
		esac
		[ "$aria_wait_count" -lt "$aria_wait_limit" ] || return 1
		sleep 1 || return 2
		aria_wait_count=$((aria_wait_count + 1))
	done
}

# Storage-volume entries are not trusted. Reject symlinked child directories
# before creating files below them or applying recursive permissions.
aria_require_dir()
{
	aria_required_dir="$1"
	if [ -L "$aria_required_dir" ]; then
		logger -t "$SVC_NAME" "Cannot start: storage directory is a symlink: $aria_required_dir" >/dev/null 2>&1 || true
		return 1
	fi
	if [ ! -d "$aria_required_dir" ]; then
		if ! mkdir -p "$aria_required_dir" ||
		   [ -L "$aria_required_dir" ] || [ ! -d "$aria_required_dir" ]; then
			logger -t "$SVC_NAME" "Cannot create safe storage directory: $aria_required_dir" >/dev/null 2>&1 || true
			return 1
		fi
	fi
	return 0
}

# Config and session paths must be absent or regular files. In particular,
# touch and chmod would otherwise follow a pre-existing symlink as root.
aria_validate_file_path()
{
	aria_checked_file="$1"
	if [ -L "$aria_checked_file" ] ||
	   { [ -e "$aria_checked_file" ] && [ ! -f "$aria_checked_file" ]; }; then
		logger -t "$SVC_NAME" "Cannot start: storage file path is not a regular file: $aria_checked_file" >/dev/null 2>&1 || true
		return 1
	fi
	return 0
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
	# Procfs must confirm absence before a new daemon can be launched.
	if aria_process_state; then
		return 0
	else
		aria_start_state=$?
	fi
	case "$aria_start_state" in
		1) ;;
		*) echo "[FAILED]"; logger -t "$SVC_NAME" "Cannot start: daemon identity is unknown" >/dev/null 2>&1 || true; return 2 ;;
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

	# Refuse linked config/download directories before any file creation or
	# recursive chmod/chown can follow a path out of the storage volume.
	aria_require_dir "$DIR_CFG" || return 1
	aria_require_dir "$DIR_DL1" || return 1

	FILE_CONF="$DIR_CFG/aria2.conf"
	FILE_LIST="$DIR_CFG/incomplete.lst"
	aria_validate_file_path "$FILE_CONF" || return 1
	aria_validate_file_path "$FILE_LIST" || return 1

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
# Verify HTTPS server certificates and hostnames by default.
check-certificate=true

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
	if aria_process_state; then
		tr_stop_state=0
	else
		tr_stop_state=$?
	fi
	case "$tr_stop_state" in
		1) return 0 ;;
		0) ;;
		*) echo "[FAILED]"; logger -t "$SVC_NAME" "Cannot stop: daemon identity is unknown" >/dev/null 2>&1 || true; return 2 ;;
	esac
	tr_stop_snapshot="$aria_process_pids"
	echo -n "Stopping $SVC_NAME:."

	if aria_signal_snapshot TERM "$tr_stop_snapshot"; then
		:
	else
		tr_signal_status=$?
		echo "[FAILED]"
		logger -t "$SVC_NAME" "Cannot stop: TERM failed or process identity became unknown (status=$tr_signal_status)" >/dev/null 2>&1 || true
		return "$tr_signal_status"
	fi
	if aria_wait_stopped 15; then
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
	if aria_process_state; then
		tr_stop_snapshot="$aria_process_pids"
	else
		tr_stop_state=$?
		[ "$tr_stop_state" -eq 1 ] && { echo "[  OK  ]"; return 0; }
		echo "[FAILED]"
		logger -t "$SVC_NAME" "Cannot stop: daemon identity is unknown before SIGKILL" >/dev/null 2>&1 || true
		return 2
	fi
	if aria_signal_snapshot 9 "$tr_stop_snapshot"; then
		:
	else
		tr_signal_status=$?
		echo "[FAILED]"
		logger -t "$SVC_NAME" "Cannot stop: SIGKILL failed or process identity became unknown (status=$tr_signal_status)" >/dev/null 2>&1 || true
		return "$tr_signal_status"
	fi
	if aria_wait_stopped 2; then
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
	if aria_process_state; then
		tr_reload_state=0
	else
		tr_reload_state=$?
	fi
	case "$tr_reload_state" in
		1) echo "Error: $SVC_NAME is not started!"; return 1 ;;
		0) ;;
		*) echo "[FAILED]"; logger -t "$SVC_NAME" "Cannot reload: daemon identity is unknown" >/dev/null 2>&1 || true; return 2 ;;
	esac
	tr_reload_snapshot="$aria_process_pids"
	echo -n "Reload $SVC_NAME config:."
	tr_reload_status=0
	for tr_reload_entry in $tr_reload_snapshot; do
		tr_reload_pid=${tr_reload_entry%%:*}
		tr_reload_start=${tr_reload_entry#*:}
		if aria_signal_identity HUP "$tr_reload_pid" "$tr_reload_start"; then
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
