#!/bin/sh
pidfile="${SCUTCLIENT_WATCHCAT_PIDFILE:-/var/scutclient_watchcat.pid}"
lockfile="${pidfile}.lock"
proc_root="${SCUTCLIENT_WATCHCAT_PROC_ROOT:-/proc}"
script_path="/usr/bin/scutclient_watchcat.sh"
net="114.114.114.114 119.29.29.29"
auth_flag="/tmp/scutclient_status"

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

go_exit(){
	# A concurrent replacement must not have its PID file removed by us.
	if [ -f "$pidfile" ] && [ ! -L "$pidfile" ]; then
		current_pid=
		IFS= read -r current_pid < "$pidfile" || :
		[ "$current_pid" = "$$" ] && rm -f "$pidfile"
	fi
	exit 0
}

auth(){
	if [ $(date '+%H') -lt 6 ]; then
		# if NTP failed or force re-auth (default), then restart scutclient
		if [ $(date '+%Y') -lt 2019 ] || [ "$(nvram get scutclient_wdg_force)" != "0" ]; then
			/usr/bin/scutclient.sh restart nolog > /dev/null 2>&1
		fi
	else
		logger -t "scutclient_watchcat" "Connection failed ($1), restart scutclient!"
		/usr/bin/scutclient.sh restart > /dev/null 2>&1
	fi
	go_exit
}

if [ "$(nvram get scutclient_watchcat)" = "1" ] && \
  [ "$(nvram get scutclient_enable)" = "1" ] && \
  [ "$(mtk_esw 11)" != "WAN ports link state: 0" ]; then

	if [ "$(nvram get scutclient_wdg_flag)" = "1" ]; then
		if [ -f "$auth_flag" ] && [ "$(cat "$auth_flag")" = "1" ]; then
			auth "flag"
		fi
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
