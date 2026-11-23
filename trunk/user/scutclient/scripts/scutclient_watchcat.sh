#!/bin/sh
pidfile="${SCUTCLIENT_WATCHCAT_PIDFILE:-/var/scutclient_watchcat.pid}"
lockfile="${pidfile}.lock"
proc_root="${SCUTCLIENT_WATCHCAT_PROC_ROOT:-/proc}"
script_path="/usr/bin/scutclient_watchcat.sh"
net="114.114.114.114 119.29.29.29"
auth_flag="${SCUTCLIENT_WATCHCAT_AUTH_FLAG:-/tmp/scutclient_status}"
scutclient_client_bin="${SCUTCLIENT_WATCHCAT_CLIENT_BIN:-/usr/bin/scutclient.sh}"

# Return 0 only when the PID still identifies this exact watchdog script,
# 1 when it is stale/another process, and 2 when procfs cannot decide.
watchcat_pid_status()
{
	watchcat_pid="$1"
	case "$watchcat_pid" in
		''|*[!0-9]*|0)
			return 1
			;;
	esac

	[ -d "$proc_root" ] && [ -r "$proc_root" ] || return 2
	[ -d "$proc_root/$watchcat_pid" ] || return 1
	[ -r "$proc_root/$watchcat_pid/cmdline" ] || return 2
	process_cmdline=$(tr '\000' ' ' < "$proc_root/$watchcat_pid/cmdline" 2>/dev/null) || return 2
	[ -n "$process_cmdline" ] || return 2
	case " $process_cmdline " in
		*" $script_path "*)
			return 0
			;;
	esac
	return 1
}

# Serialize only the PID claim transaction. Close FD 9 immediately after
# publishing the current PID so child processes and restarted daemons do
# not inherit the lock descriptor.
watchcat_claim_lock()
{
	[ ! -L "$lockfile" ] || return 1
	if [ -e "$lockfile" ] && [ ! -f "$lockfile" ]; then
		return 1
	fi
	lock_umask=$(umask)
	umask 077
	exec 9>>"$lockfile" || {
		umask "$lock_umask"
		return 1
	}
	umask "$lock_umask"
	[ -f "$lockfile" ] && [ ! -L "$lockfile" ] || return 1
	command -v flock >/dev/null 2>&1 || return 1
	if flock -n 9; then
		return 0
	fi
	return 2
}

watchcat_claim_lock
lock_status=$?
case "$lock_status" in
0)
	;;
2)
	exit 0
	;;
*)
	exit 1
	;;
esac

# Claim a new PID file without ever signalling a PID read from stale state.
# A live watchdog already owns the check; an UNKNOWN procfs result fails
# closed instead of replacing an owner whose identity cannot be verified.
watchcat_claim_pidfile()
{
	[ ! -L "$pidfile" ] || return 1
	if [ -e "$pidfile" ] && [ ! -f "$pidfile" ]; then
		return 1
	fi

	if (set -C; printf '%s\n' "$$" > "$pidfile") 2>/dev/null; then
		return 0
	fi
	[ -f "$pidfile" ] && [ ! -L "$pidfile" ] || return 1

	previous_pid=
	IFS= read -r previous_pid < "$pidfile" || :
	watchcat_pid_status "$previous_pid"
	previous_status=$?
	case "$previous_status" in
	0)
		return 2
		;;
	1)
		;;
	*)
		return 1
		;;
	esac

	current_pid=
	IFS= read -r current_pid < "$pidfile" || :
	[ "$current_pid" = "$previous_pid" ] || return 2
	rm -f "$pidfile" || return 1
	(set -C; printf '%s\n' "$$" > "$pidfile") 2>/dev/null || return 2
	return 0
}

watchcat_claim_pidfile
claim_status=$?
exec 9>&-
case "$claim_status" in
0)
	;;
2)
	exit 0
	;;
*)
	exit 1
	;;
esac

watchcat_cleanup_pidfile(){
	# A concurrent replacement must not have its PID file removed by us.
	if [ -f "$pidfile" ] && [ ! -L "$pidfile" ]; then
		current_pid=
		IFS= read -r current_pid < "$pidfile" || :
		[ "$current_pid" = "$" ] && rm -f "$pidfile"
	fi
}

go_exit(){
	watchcat_cleanup_pidfile
	exit 0
}

watchcat_fail(){
	watchcat_failure_status=$1
	case "$watchcat_failure_status" in
		''|*[!0-9]*|0)
			watchcat_failure_status=1
			;;
	esac
	logger -t "scutclient_watchcat" "watchdog action aborted: $2 (status=$watchcat_failure_status)"
	watchcat_cleanup_pidfile
	exit "$watchcat_failure_status"
}

# Distinguish a missing key (successful empty value) from an NVRAM backend
# failure. The legacy empty defaults are interpreted by the individual
# selector below, but read errors must never silently disable monitoring or
# trigger a speculative re-authentication.
watchcat_read_checked(){
	watchcat_key=$1
	watchcat_value=$(nvram get_checked "$watchcat_key")
	watchcat_read_status=$?
	if [ "$watchcat_read_status" -ne 0 ]; then
		logger -t "scutclient_watchcat" "cannot read NVRAM setting $watchcat_key (status=$watchcat_read_status)"
		return "$watchcat_read_status"
	fi
	return 0
}

watchcat_read_boolean(){
	if watchcat_read_checked "$1"; then
		:
	else
		return $?
	fi
	case "$watchcat_value" in
		''|0)
			watchcat_enabled=0
			;;
		1)
			watchcat_enabled=1
			;;
		*)
			logger -t "scutclient_watchcat" "invalid boolean NVRAM setting $1"
			return 2
			;;
	esac
	return 0
}

watchcat_read_force(){
	if watchcat_read_checked scutclient_wdg_force; then
		:
	else
		return $?
	fi
	case "$watchcat_value" in
		''|1)
			# Preserve the legacy default: a missing force flag means re-auth.
			watchcat_force=1
			;;
		0)
			watchcat_force=0
			;;
		*)
			logger -t "scutclient_watchcat" "invalid boolean NVRAM setting scutclient_wdg_force"
			return 2
			;;
	esac
	return 0
}

auth(){
	if [ "$(date '+%H')" -lt 6 ]; then
		# NTP failure forces re-auth; otherwise honor the checked UI selector.
		if [ "$(date '+%Y')" -lt 2019 ]; then
			"$scutclient_client_bin" restart nolog > /dev/null 2>&1
		else
			if watchcat_read_force; then
				:
			else
				watchcat_auth_status=$?
				watchcat_fail "$watchcat_auth_status" "cannot verify force re-auth policy"
			fi
			[ "$watchcat_force" = "1" ] &&
				"$scutclient_client_bin" restart nolog > /dev/null 2>&1
		fi
	else
		logger -t "scutclient_watchcat" "Connection failed ($1), restart scutclient!"
		"$scutclient_client_bin" restart > /dev/null 2>&1
	fi
	go_exit
}

if watchcat_read_boolean scutclient_watchcat; then
	:
else
	watchcat_config_status=$?
	watchcat_fail "$watchcat_config_status" "cannot verify watchdog enable state"
fi
[ "$watchcat_enabled" = "1" ] || go_exit

if watchcat_read_boolean scutclient_enable; then
	:
else
	watchcat_config_status=$?
	watchcat_fail "$watchcat_config_status" "cannot verify SCUTClient enable state"
fi
[ "$watchcat_enabled" = "1" ] || go_exit

if [ "$(mtk_esw 11)" != "WAN ports link state: 0" ]; then
	if watchcat_read_boolean scutclient_wdg_flag; then
		:
	else
		watchcat_config_status=$?
		watchcat_fail "$watchcat_config_status" "cannot verify watchdog auth flag"
	fi
	if [ "$watchcat_enabled" = "1" ] && [ -f "$auth_flag" ] &&
	   [ "$(cat "$auth_flag")" = "1" ]; then
		auth "flag"
	fi

	for n in $net; do
		/bin/ping -c 3 "$n" -w 5 >/dev/null 2>&1
		if [ "$?" = "0" ]; then
			go_exit
		fi
	done
	auth "ping"
else
	go_exit
fi
