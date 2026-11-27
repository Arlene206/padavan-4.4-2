#!/bin/sh

dir_storage="${SSHD_STORAGE_DIR:-/etc/storage/openssh}"
sshd_config="${SSHD_CONFIG_FILE:-$dir_storage/sshd_config}"

rsa_key="${SSHD_RSA_KEY:-$dir_storage/ssh_host_rsa_key}"
dsa_key="${SSHD_DSA_KEY:-$dir_storage/ssh_host_dsa_key}"
ed25519_key="${SSHD_ED25519_KEY:-$dir_storage/ssh_host_ed25519_key}"
old_path="${SSHD_OLD_PATH:-/etc/storage}"
sshd_bin="${SSHD_BIN:-/usr/sbin/sshd}"
ssh_keygen_bin="${SSHD_KEYGEN_BIN:-/usr/bin/ssh-keygen}"
utmp_file="${SSHD_UTMP_FILE:-/var/run/utmp}"
pid_file="${SSHD_PID_FILE:-/var/run/sshd.pid}"
PROCESS_LIFECYCLE_LIB="${PROCESS_LIFECYCLE_LIB:-/usr/lib/process_lifecycle.sh}"

func_config_path_check()
{
	if [ -L "$sshd_config" ] ||
	   { [ -e "$sshd_config" ] && [ ! -f "$sshd_config" ]; }; then
		logger -t "sshd" "refusing unsafe configuration path"
		return 1
	fi
	return 0
}

func_create_config()
{
	func_config_path_check || return $?
	if [ -e "$sshd_config" ]; then
		logger -t "sshd" "refusing to replace an existing configuration"
		return 1
	fi
		config_tmp_id=$(stat -L -c '%i' /proc/self 2>/dev/null)
	config_tmp_id_status=$?
	if [ "$config_tmp_id_status" -ne 0 ]; then
		logger -t "sshd" "cannot obtain unique configuration staging ID (status=$config_tmp_id_status)"
		return "$config_tmp_id_status"
	fi
	case "$config_tmp_id" in
		''|*[!0-9]*)
			logger -t "sshd" "invalid configuration staging ID"
			return 1
			;;
	esac
	config_tmp="${sshd_config}.tmp.${config_tmp_id}"
	if (umask 077; set -C; : > "$config_tmp") 2>/dev/null; then
		:
	else
		config_tmp_status=$?
		logger -t "sshd" "cannot create private configuration staging file (status=$config_tmp_status)"
		return "$config_tmp_status"
	fi
	cat > "$config_tmp" <<EOF
# The strategy used for options in this file is to specify options with
# their default value where possible, but leave them commented.
# Uncommented options override the default value.

#Port 22
#AddressFamily any
#ListenAddress 0.0.0.0
#ListenAddress ::

# The default requires explicit activation of protocol 1
Protocol 2

# HostKey for protocol version 1
#HostKey ${dir_storage}/ssh_host_key

# HostKeys for protocol version 2
HostKey ${rsa_key}
HostKey ${dsa_key}
#HostKey ${dir_storage}/ssh_host_ecdsa_key
HostKey ${ed25519_key}

# Lifetime and size of ephemeral version 1 server key
#KeyRegenerationInterval 1h
#ServerKeyBits 1024

# Ciphers and keying
#RekeyLimit default none

# Logging
# obsoletes QuietMode and FascistLogging
#SyslogFacility AUTH
#LogLevel INFO

# Authentication:

LoginGraceTime 1m
PermitRootLogin yes
#StrictModes yes
MaxAuthTries 3
#MaxSessions 10

#RSAAuthentication yes
#PubkeyAuthentication yes

# The default is to check both .ssh/authorized_keys and .ssh/authorized_keys2
# but this is overridden so installations will only check .ssh/authorized_keys
AuthorizedKeysFile	.ssh/authorized_keys

#AuthorizedPrincipalsFile none

#AuthorizedKeysCommand none
#AuthorizedKeysCommandUser nobody

# For this to work you will also need host keys in ${dir_storage}/ssh_known_hosts
#RhostsRSAAuthentication no
# similar for protocol version 2
#HostbasedAuthentication no
# Change to yes if you don't trust ~/.ssh/known_hosts for
# RhostsRSAAuthentication and HostbasedAuthentication
IgnoreUserKnownHosts yes
# Don't read the user's ~/.rhosts and ~/.shosts files
#IgnoreRhosts yes

# To disable tunneled clear text passwords, change to no here!
#PasswordAuthentication yes
#PermitEmptyPasswords no

# Change to no to disable s/key passwords
ChallengeResponseAuthentication no

AllowAgentForwarding no
#AllowTcpForwarding yes
#GatewayPorts no
#X11Forwarding no
#X11DisplayOffset 10
#X11UseLocalhost yes
#PermitTTY yes
#PrintMotd yes
#PrintLastLog yes
#TCPKeepAlive yes
#UseLogin no
#PermitUserEnvironment no
#Compression delayed
#ClientAliveInterval 0
#ClientAliveCountMax 3
#UseDNS no
PidFile /var/run/sshd.pid
#MaxStartups 10:30:100
#PermitTunnel no
#ChrootDirectory none
#VersionAddendum none

# no default banner path
#Banner none

# override default of no subsystems
Subsystem	sftp	/usr/libexec/sftp-server

EOF
	config_write_status=$?
	if [ "$config_write_status" -ne 0 ]; then
		rm -f "$config_tmp"
		logger -t "sshd" "configuration staging write failed (status=$config_write_status)"
		return "$config_write_status"
	fi
	if chmod 644 "$config_tmp"; then :; else
		config_chmod_status=$?
		rm -f "$config_tmp"
		logger -t "sshd" "configuration staging permission update failed (status=$config_chmod_status)"
		return "$config_chmod_status"
	fi
	func_config_path_check || { config_path_status=$?; rm -f "$config_tmp"; return "$config_path_status"; }
	if [ -e "$sshd_config" ]; then
		rm -f "$config_tmp"
		logger -t "sshd" "configuration path appeared during publication"
		return 1
	fi
	if mv "$config_tmp" "$sshd_config"; then
		return 0
	else
		config_publish_status=$?
		rm -f "$config_tmp"
		logger -t "sshd" "configuration publication failed (status=$config_publish_status)"
		return "$config_publish_status"
	fi
}

func_validate_host_key()
{
	key_path="$1"
	if [ -L "$key_path" ] ||
	   { [ -e "$key_path" ] && [ ! -f "$key_path" ]; }; then
		logger -t "sshd" "refusing unsafe host-key path: $key_path"
		return 1
	fi
	[ -f "$key_path" ] || return 0
	if [ ! -s "$key_path" ]; then
		logger -t "sshd" "host key is empty: $key_path"
		return 1
	fi
	key_links=$(stat -c '%h' "$key_path" 2>/dev/null)
	key_links_status=$?
	if [ "$key_links_status" -ne 0 ]; then
		logger -t "sshd" "cannot inspect host-key link count (status=$key_links_status): $key_path"
		return "$key_links_status"
	fi
	case "$key_links" in
		1) ;;
		''|*[!0-9]*)
			logger -t "sshd" "invalid host-key link count: $key_path"
			return 2
			;;
		*)
			logger -t "sshd" "refusing multiply-linked host key: $key_path"
			return 1
			;;
	esac
	if chmod 600 "$key_path"; then :; else
		key_chmod_status=$?
		logger -t "sshd" "cannot restrict host-key permissions (status=$key_chmod_status): $key_path"
		return "$key_chmod_status"
	fi
	if "$ssh_keygen_bin" -y -P "" -f "$key_path" >/dev/null 2>&1; then
		return 0
	else
		key_validation_status=$?
		logger -t "sshd" "host-key validation failed (status=$key_validation_status): $key_path"
		return "$key_validation_status"
	fi
}

func_validate_host_keys()
{
	func_validate_host_key "$rsa_key" || return $?
	func_validate_host_key "$dsa_key" || return $?
	func_validate_host_key "$ed25519_key"
}

func_start()
{
	func_config_path_check
	config_path_status=$?
	if [ "$config_path_status" -ne 0 ]; then
		return "$config_path_status"
	fi
	if [ ! -d "$dir_storage" ]; then
		if mkdir -p -m 755 "$dir_storage"; then
			:
		else
			storage_status=$?
			logger -t "sshd" "cannot create host-key storage (status=$storage_status)"
			return "$storage_status"
		fi
	fi

	rm -f "${old_path}/sshd_config"
	for i in ssh_host_rsa_key ssh_host_dsa_key ; do
		[ -f "${old_path}/${i}" ] && mv -n "${old_path}/${i}" "$dir_storage"
		[ -f "${old_path}/${i}.pub" ] && mv -n "${old_path}/${i}.pub" "$dir_storage"
	done

	func_validate_host_keys
	key_validation_status=$?
	if [ "$key_validation_status" -ne 0 ]; then
		return "$key_validation_status"
	fi
	if [ ! -f "$rsa_key" ] || [ ! -f "$dsa_key" ] || [ ! -f "$ed25519_key" ] ; then
		if "$ssh_keygen_bin" -A; then
			:
		else
			keygen_status=$?
			logger -t "sshd" "host-key generation failed (status=$keygen_status)"
			return "$keygen_status"
		fi
		func_validate_host_keys
		key_validation_status=$?
		if [ "$key_validation_status" -ne 0 ]; then
			return "$key_validation_status"
		fi
	fi

	if [ ! -f "$sshd_config" ] ; then
		func_create_config
		config_status=$?
		if [ "$config_status" -ne 0 ]; then
			logger -t "sshd" "configuration creation failed (status=$config_status)"
			return "$config_status"
		fi
	fi

	if touch "$utmp_file"; then
		:
	else
		utmp_status=$?
		logger -t "sshd" "cannot prepare utmp (status=$utmp_status)"
		return "$utmp_status"
	fi

	if [ -n "$1" ] ; then
		if "$sshd_bin" -o PasswordAuthentication=no; then
			return 0
		else
			launch_status=$?
			logger -t "sshd" "daemon launch failed (status=$launch_status)"
			return "$launch_status"
		fi
	else
		if "$sshd_bin"; then
			return 0
		else
			launch_status=$?
			logger -t "sshd" "daemon launch failed (status=$launch_status)"
			return "$launch_status"
		fi
	fi
}

func_stop()
{
	if [ ! -f "$PROCESS_LIFECYCLE_LIB" ] ||
	   [ -L "$PROCESS_LIFECYCLE_LIB" ] ||
	   [ ! -r "$PROCESS_LIFECYCLE_LIB" ]; then
		logger -t "sshd" "stop refused: shared process lifecycle helper is missing or unsafe"
		return 2
	fi
	. "$PROCESS_LIFECYCLE_LIB" || {
		logger -t "sshd" "stop refused: cannot load shared process lifecycle helper"
		return 2
	}
	if ! command -v pl_stop_named >/dev/null 2>&1; then
		logger -t "sshd" "stop refused: shared process lifecycle helper is incomplete"
		return 2
	fi

	if pl_stop_named sshd 3 2; then
		return 0
	else
		stop_status=$?
		logger -t "sshd" "daemon stop was not confirmed (status=$stop_status)"
		return "$stop_status"
	fi
}

# Resolve an sshd PID to a stable procfs start-time identity.
# PID files are untrusted state: never signal a recycled PID or another daemon.
sshd_process_start_time()
{
	local pid="$1"
	local proc_root="${SSHD_PROC_ROOT:-/proc}"
	local process_dir="$proc_root/$pid"
	local process_comm stat_line start_time

	case "$pid" in
	''|*[!0-9]*|0|1)
		return 1
		;;
	esac
	[ "$pid" -gt 1 ] 2>/dev/null || return 1
	[ -d "$process_dir" ] || return 1
	IFS= read -r process_comm < "$process_dir/comm" || return 1
	[ "$process_comm" = "sshd" ] || return 1
	IFS= read -r stat_line < "$process_dir/stat" || return 1
	case "$stat_line" in
	"$pid ("*") "*) ;;
	*) return 1 ;;
	esac
	start_time=$(printf '%s\n' "$stat_line" |
		awk '{ sub(/^.*\) /, ""); if (NF >= 20) print $20 }') || return 1
	case "$start_time" in
	''|*[!0-9]*)
		return 1
		;;
	esac
	printf '%s\n' "$start_time"
}

func_reload()
{
	local pid pid_again start_time current_start_time reload_status

	if [ ! -f "$pid_file" ] || [ -L "$pid_file" ] || [ ! -r "$pid_file" ]; then
		logger -t "sshd" "reload refused: PID file is missing, unreadable or unsafe"
		return 1
	fi
	pid=$(cat "$pid_file" 2>/dev/null) || {
		logger -t "sshd" "reload refused: cannot read PID file"
		return 1
	}
	case "$pid" in
	''|*[!0-9]*|0|1)
		logger -t "sshd" "reload refused: PID file does not contain one valid PID"
		return 1
		;;
	esac
	[ "$pid" -gt 1 ] 2>/dev/null || {
		logger -t "sshd" "reload refused: PID is outside the valid process range"
		return 1
	}
	start_time=$(sshd_process_start_time "$pid") || {
		logger -t "sshd" "reload refused: PID does not identify a live sshd process"
		return 1
	}
	if [ -L "$pid_file" ] || [ ! -f "$pid_file" ]; then
		logger -t "sshd" "reload refused: PID file changed during validation"
		return 1
	fi
	pid_again=$(cat "$pid_file" 2>/dev/null) || {
		logger -t "sshd" "reload refused: PID file disappeared during validation"
		return 1
	}
	if [ "$pid_again" != "$pid" ]; then
		logger -t "sshd" "reload refused: PID file changed during validation"
		return 1
	fi
	current_start_time=$(sshd_process_start_time "$pid") || {
		logger -t "sshd" "reload refused: sshd process disappeared during validation"
		return 1
	}
	if [ "$current_start_time" != "$start_time" ]; then
		logger -t "sshd" "reload refused: PID identity changed during validation"
		return 1
	fi

	kill -HUP "$pid"
	reload_status=$?
	if [ "$reload_status" -ne 0 ]; then
		logger -t "sshd" "reload signal failed for PID $pid (status=$reload_status)"
	fi
	return "$reload_status"
}

action_status=0
case "$1" in
start)
	func_start "$2"
	action_status=$?
	;;
stop)
	func_stop
	action_status=$?
	;;
reload)
	func_reload
	action_status=$?
	;;
*)
	echo "Usage: $0 {start|stop|reload}"
	action_status=2
	;;
esac

exit "$action_status"
