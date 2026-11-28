#!/bin/sh
#
# Bounded service shutdown. Process lookup states: 0=running, 1=absent,
# 2=unknown/error. Unknown inventory never authorizes a signal.

ss_validate_pid_list() {
	ss_pid_list=$1
	[ -n "$ss_pid_list" ] || return 2
	printf '%s\n' "$ss_pid_list" | awk '
		{
			for (i = 1; i <= NF; i++) {
				if ($i !~ /^[1-9][0-9]*$/) bad = 1
				count++
			}
		}
		END { if (bad || count == 0) exit 1; exit 0 }
	' >/dev/null 2>&1 || return 2
	return 0
}


# BusyBox procps_scan skips entries it cannot read. A pidof/pgrep status 1
# is only a candidate ABSENT result; confirm it against every visible PID.
ss_procfs_process_count() {
	[ "$#" -eq 2 ] || return 2
	ss_proc_mode=$1
	ss_proc_query=$2
	[ -n "$ss_proc_query" ] || return 2
	case "$ss_proc_mode" in name|pattern) ;; *) return 2 ;; esac
	ss_proc_root=${SS_PROC_ROOT:-/proc}
	[ -d "$ss_proc_root" ] && [ -r "$ss_proc_root" ] && [ -x "$ss_proc_root" ] || return 2
	command -v ls >/dev/null 2>&1 || return 2
	command -v awk >/dev/null 2>&1 || return 2
	command -v tr >/dev/null 2>&1 || return 2
	command -v grep >/dev/null 2>&1 || return 2
	ss_proc_entries=$(ls -1 "$ss_proc_root" 2>/dev/null) || return 2
	[ -n "$ss_proc_entries" ] || return 2
	ss_proc_numeric_found=0
	ss_proc_match_count=0
	ss_proc_newline='
'
	ss_proc_name_prefix=$(printf '%.15s' "$ss_proc_query")
	while IFS= read -r ss_proc_pid; do
		case "$ss_proc_pid" in ''|*[!0-9]*) continue ;; 0*) return 2 ;; esac
		ss_proc_dir="$ss_proc_root/$ss_proc_pid"
		[ -d "$ss_proc_dir" ] || continue
		ss_proc_numeric_found=1
		if ss_proc_stat=$(cat "$ss_proc_dir/stat" 2>/dev/null); then :; else
			[ -d "$ss_proc_dir" ] || continue
			return 2
		fi
		case "$ss_proc_stat" in "$ss_proc_pid ("*) ;; *) return 2 ;; esac
		if ss_proc_state=$(printf '%s\n' "$ss_proc_stat" | awk -v expected_pid="$ss_proc_pid" '
			NR != 1 { invalid = 1 }
			NR == 1 {
				if (index($0, expected_pid " (") != 1) invalid = 1
				sub(/^.*\) /, "")
				if (NF < 20 || $1 !~ /^[RSDZTtXxKWPI]$/ || $20 !~ /^[0-9]+$/)
					invalid = 1
				else task_state = $1
			}
			END { if (invalid || NR != 1) exit 1; print task_state }
		'); then :; else
			[ -d "$ss_proc_dir" ] || continue
			return 2
		fi
		case "$ss_proc_state" in Z|X|x) continue ;; esac
		if [ "$ss_proc_mode" = name ]; then
			if ss_proc_arguments=$(tr '\000' '\n' < "$ss_proc_dir/cmdline" 2>/dev/null); then :; else
				[ -d "$ss_proc_dir" ] || continue
				return 2
			fi
			ss_proc_argv0=${ss_proc_arguments%%"$ss_proc_newline"*}
			ss_proc_argv1=
			case "$ss_proc_arguments" in *"$ss_proc_newline"*)
				ss_proc_rest=${ss_proc_arguments#*"$ss_proc_newline"}
				ss_proc_argv1=${ss_proc_rest%%"$ss_proc_newline"*}
				;;
			esac
			ss_proc_argv0=${ss_proc_argv0##*/}
			ss_proc_argv1=${ss_proc_argv1##*/}
			ss_proc_comm=${ss_proc_stat#*(}
			ss_proc_comm=${ss_proc_comm%)*}
			ss_proc_comm_prefix=$(printf '%.15s' "$ss_proc_comm")
			ss_proc_is_match=0
			[ "$ss_proc_argv0" = "$ss_proc_query" ] && ss_proc_is_match=1
			if [ "$ss_proc_comm_prefix" = "$ss_proc_name_prefix" ]; then
				if [ "${#ss_proc_comm}" -lt 15 ] || [ "$ss_proc_argv1" = "$ss_proc_query" ]; then
					ss_proc_is_match=1
				fi
			fi
			[ "$ss_proc_is_match" -eq 0 ] || ss_proc_match_count=$((ss_proc_match_count + 1))
		else
			if ss_proc_arguments=$(tr '\000\n' '  ' < "$ss_proc_dir/cmdline" 2>/dev/null); then :; else
				[ -d "$ss_proc_dir" ] || continue
				return 2
			fi
			# BusyBox pgrep falls back to comm for an empty cmdline.
			[ -n "$ss_proc_arguments" ] || ss_proc_arguments=${ss_proc_stat#*(}
			ss_proc_arguments=${ss_proc_arguments%)*}
			if printf '%s\n' "$ss_proc_arguments" | grep -E -q -- "$ss_proc_query"; then
				ss_proc_match_count=$((ss_proc_match_count + 1))
			else
				ss_proc_grep_status=$?
				[ "$ss_proc_grep_status" -eq 1 ] || return 2
			fi
		fi
	done <<EOF_PROCFS_ENTRIES
$ss_proc_entries
EOF_PROCFS_ENTRIES
	[ "$ss_proc_numeric_found" -eq 1 ] || return 2
	if [ "$ss_proc_match_count" -eq 0 ]; then
		if [ "$ss_proc_mode" = name ]; then
			if ss_proc_post_output=$(pidof "$ss_proc_query" 2>/dev/null); then ss_proc_post_status=0; else ss_proc_post_status=$?; fi
		else
			if ss_proc_post_output=$(pgrep -f "$ss_proc_query" 2>/dev/null); then ss_proc_post_status=0; else ss_proc_post_status=$?; fi
		fi
		case "$ss_proc_post_status" in
			0)
				ss_validate_pid_list "$ss_proc_post_output" || return 2
				ss_proc_match_count=$(printf '%s\n' "$ss_proc_post_output" | awk '{ count += NF } END { print count + 0 }')
				;;
			1) [ -z "$ss_proc_post_output" ] || return 2 ;;
			*) return 2 ;;
		esac
	fi
	printf '%s\n' "$ss_proc_match_count"
	[ "$ss_proc_match_count" -gt 0 ] && return 0
	return 1
}

ss_process_pid_state() {
	ss_process_name=$1
	if ss_pid_output=$(pidof "$ss_process_name" 2>/dev/null); then ss_pid_status=0; else ss_pid_status=$?; fi
	case "$ss_pid_status" in
		0) ss_validate_pid_list "$ss_pid_output" || return 2; return 0 ;;
		1)
			[ -z "$ss_pid_output" ] || return 2
			ss_procfs_process_count name "$ss_process_name" >/dev/null
			return $?
			;;
		*) return 2 ;;
	esac
}

# Wait for a named daemon to appear. Confirmed absence is retryable; an
# invalid or failed process inventory is UNKNOWN and must fail closed.
ss_process_pid_count() {
	[ "$#" -eq 1 ] || return 2
	ss_count_process_name=$1
	[ -n "$ss_count_process_name" ] || return 2
	if ss_count_pid_output=$(pidof "$ss_count_process_name" 2>/dev/null); then ss_count_pid_status=0; else ss_count_pid_status=$?; fi
	case "$ss_count_pid_status" in
		0)
			ss_validate_pid_list "$ss_count_pid_output" || return 2
			printf '%s\n' "$ss_count_pid_output" | awk '{ count += NF } END { print count + 0 }'
			;;
		1)
			[ -z "$ss_count_pid_output" ] || return 2
			if ss_procfs_process_count name "$ss_count_process_name"; then return 0; else
				ss_procfs_status=$?
				[ "$ss_procfs_status" -eq 1 ] && return 0
				return 2
			fi
			;;
		*) return 2 ;;
	esac
}

# Wait for a newly launched process instance; pre-existing siblings do not count.
ss_wait_named_process_count() {
	[ "$#" -eq 3 ] || return 2
	ss_start_process_name=$1
	ss_start_expected_count=$2
	ss_start_wait_limit=$3
	[ -n "$ss_start_process_name" ] || return 2
	case "$ss_start_expected_count" in
		''|*[!0-9]*) return 2 ;;
	esac
	case "$ss_start_wait_limit" in
		''|*[!0-9]*) return 2 ;;
	esac
	[ "$ss_start_expected_count" -ge 1 ] || return 2
	[ "$ss_start_wait_limit" -le 30 ] || return 2

	ss_start_wait_count=0
	while :; do
		if ss_start_process_count=$(ss_process_pid_count "$ss_start_process_name"); then
			:
		else
			ss_start_status=$?
			return "$ss_start_status"
		fi
		[ "$ss_start_process_count" -ge "$ss_start_expected_count" ] && return 0
		[ "$ss_start_wait_count" -lt "$ss_start_wait_limit" ] || return 1
		sleep 1 || return 2
		ss_start_wait_count=$((ss_start_wait_count + 1))
	done
}

ss_wait_named_process_started() {
	[ "$#" -eq 2 ] || return 2
	ss_wait_named_process_count "$1" 1 "$2"
}

ss_process_pattern_state() {
	ss_process_pattern=$1
	[ -n "$ss_process_pattern" ] || return 2
	if ss_pid_output=$(pgrep -f "$ss_process_pattern" 2>/dev/null); then ss_pid_status=0; else ss_pid_status=$?; fi
	case "$ss_pid_status" in
		0) ss_validate_pid_list "$ss_pid_output" || return 2; return 0 ;;
		1)
			[ -z "$ss_pid_output" ] || return 2
			ss_procfs_process_count pattern "$ss_process_pattern" >/dev/null
			return $?
			;;
		*) return 2 ;;
	esac
}

ss_process_pattern_count() {
	[ "$#" -eq 1 ] || return 2
	ss_count_pattern=$1
	[ -n "$ss_count_pattern" ] || return 2
	if ss_pattern_pid_output=$(pgrep -f "$ss_count_pattern" 2>/dev/null); then ss_pattern_pid_status=0; else ss_pattern_pid_status=$?; fi
	case "$ss_pattern_pid_status" in
		0)
			ss_validate_pid_list "$ss_pattern_pid_output" || return 2
			printf '%s\n' "$ss_pattern_pid_output" | awk '{ count += NF } END { print count + 0 }'
			;;
		1)
			[ -z "$ss_pattern_pid_output" ] || return 2
			if ss_procfs_process_count pattern "$ss_count_pattern"; then return 0; else
				ss_procfs_status=$?
				[ "$ss_procfs_status" -eq 1 ] && return 0
				return 2
			fi
			;;
		*) return 2 ;;
	esac
}

ss_wait_process_pattern_count() {
	[ "$#" -eq 3 ] || return 2
	ss_pattern_wait_name=$1
	ss_pattern_wait_expected=$2
	ss_pattern_wait_limit=$3
	[ -n "$ss_pattern_wait_name" ] || return 2
	case "$ss_pattern_wait_expected" in
		''|*[!0-9]*) return 2 ;;
	esac
	case "$ss_pattern_wait_limit" in
		''|*[!0-9]*) return 2 ;;
	esac
	[ "$ss_pattern_wait_expected" -ge 1 ] || return 2
	[ "$ss_pattern_wait_limit" -le 30 ] || return 2

	ss_pattern_wait_count=0
	while :; do
		if ss_pattern_running_count=$(ss_process_pattern_count "$ss_pattern_wait_name"); then
			:
		else
			ss_pattern_wait_status=$?
			return "$ss_pattern_wait_status"
		fi
		[ "$ss_pattern_running_count" -ge "$ss_pattern_wait_expected" ] && return 0
		[ "$ss_pattern_wait_count" -lt "$ss_pattern_wait_limit" ] || return 1
		sleep 1 || return 2
		ss_pattern_wait_count=$((ss_pattern_wait_count + 1))
	done
}

ss_named_processes_state() {
	ss_any_running=0
	ss_any_unknown=0
	for ss_process_name in "$@"; do
		if ss_process_pid_state "$ss_process_name"; then
			ss_any_running=1
		else
			ss_lookup_status=$?
			[ "$ss_lookup_status" -eq 1 ] || ss_any_unknown=1
		fi
	done
	[ "$ss_any_unknown" -eq 0 ] || return 2
	[ "$ss_any_running" -eq 1 ] && return 0
	return 1
}

ss_wait_named_processes() {
	ss_wait_limit=$1
	shift
	ss_wait_count=0
	while :; do
		if ss_named_processes_state "$@"; then ss_wait_state=0; else ss_wait_state=$?; fi
		case "$ss_wait_state" in 0) ;; 1) return 0 ;; *) return 2 ;; esac
		[ "$ss_wait_count" -lt "$ss_wait_limit" ] || return 1
		sleep 1 || return 2
		ss_wait_count=$((ss_wait_count + 1))
	done
}

ss_wait_process_pattern() {
	ss_wait_limit=$1
	ss_process_pattern=$2
	ss_wait_count=0
	while :; do
		if ss_process_pattern_state "$ss_process_pattern"; then ss_wait_state=0; else ss_wait_state=$?; fi
		case "$ss_wait_state" in 0) ;; 1) return 0 ;; *) return 2 ;; esac
		[ "$ss_wait_count" -lt "$ss_wait_limit" ] || return 1
		sleep 1 || return 2
		ss_wait_count=$((ss_wait_count + 1))
	done
}


# Resolve the installed executable path before inspecting or signalling a daemon.
# These are the ROMFS paths used by the Shadowsocks package and RC.
ss_expected_executable() {
	case "$1" in
		/user/sbin/*|/usr/bin/*)
			printf '%s\n' "$1"
			;;
		dnsmasq)
			printf '%s\n' /user/sbin/dnsmasq
			;;
		''|*[!A-Za-z0-9._-]*)
			return 2
			;;
		*)
			printf '/usr/bin/%s\n' "$1"
			;;
	esac
}

ss_named_executable_matches() {
	ss_match_name=${1##*/}
	ss_match_exe=$2
	case "$ss_match_name" in
		v2ray|xray)
			case "$ss_match_exe" in
				/usr/bin/v2ray|/usr/bin/xray) return 0 ;;
			esac
			;;
		dnsmasq)
			[ "$ss_match_exe" = /user/sbin/dnsmasq ]
			;;
		*)
			[ "$ss_match_exe" = "/usr/bin/$ss_match_name" ]
			;;
	esac
}

ss_shell_executable_matches() {
	case "$1" in
		/bin/busybox|/bin/sh|/bin/ash) return 0 ;;
		*) return 1 ;;
	esac
}

# Read Linux /proc/PID/stat fields 3 and 22. The command name is parenthesized
# and may itself contain spaces or ')', so locate the final ") " delimiter.
ss_read_proc_identity() {
	[ "$#" -eq 1 ] || return 2
	local ss_identity_pid="$1"
	local ss_identity_dir="${SS_PROC_ROOT:-/proc}/$1"
	local ss_identity_stat
	local ss_identity_values

	case "$ss_identity_pid" in ''|*[!0-9]*|0*) return 2 ;; esac
	if [ ! -d "$ss_identity_dir" ]; then return 1; fi
	if ss_identity_stat=$(cat "$ss_identity_dir/stat" 2>/dev/null); then
		:
	else
		[ -d "$ss_identity_dir" ] || return 1
		return 2
	fi
	ss_identity_values=$(printf '%s\n' "$ss_identity_stat" | awk -v expected_pid="$ss_identity_pid" '
		NR != 1 { invalid = 1 }
		NR == 1 {
			if (index($0, expected_pid " (") != 1) invalid = 1
			tail = $0
			sub(/^[0-9]+ \(/, "", tail)
			close_pos = 0
			for (i = length(tail); i >= 1; i--) {
				if (substr(tail, i, 2) == ") ") {
					close_pos = i
					break
				}
			}
			if (!close_pos) invalid = 1
			else {
				rest = substr(tail, close_pos + 2)
				count = split(rest, field, /[[:space:]]+/)
				if (count < 20 ||
				    field[1] !~ /^[RSDZTtXxKWPI]$/ ||
				    field[20] !~ /^[0-9]+$/)
					invalid = 1
				else
					printf "%s %s\n", field[1], field[20]
			}
		}
		END { if (invalid || NR != 1) exit 1 }
	') || {
		[ -d "$ss_identity_dir" ] || return 1
		return 2
	}
	case "$ss_identity_values" in
		[RSDZTtXxKWPI]' '*[0-9]*) ;;
		*) return 2 ;;
	esac
	SS_PROC_TASK_STATE=${ss_identity_values%% *}
	SS_PROC_START_TIME=${ss_identity_values#* }
	return 0
}

# True only when argv[1] is the exact monitor script path. A loose pgrep
# expression would also match unrelated shells whose arguments mention it.
ss_script_argv_matches() {
	[ "$#" -eq 2 ] || return 2
	local ss_script_dir="$1"
	local ss_script_path="$2"
	local ss_script_args
	local ss_script_newline='
'
	if ss_script_args=$(tr '\000' '\n' < "$ss_script_dir/cmdline" 2>/dev/null); then
		:
	else
		[ -d "$ss_script_dir" ] || return 1
		return 2
	fi
	ss_script_argv0=${ss_script_args%%"$ss_script_newline"*}
	case "$ss_script_args" in
		*"$ss_script_newline"*)
			ss_script_rest=${ss_script_args#*"$ss_script_newline"}
			ss_script_argv1=${ss_script_rest%%"$ss_script_newline"*}
			;;
		*)
			return 1
			;;
	esac
	ss_script_argv0=${ss_script_argv0##*/}
	case "$ss_script_argv0" in sh|ash|busybox) ;; *) return 1 ;; esac
	[ "$ss_script_argv1" = "$ss_script_path" ]
}

# Return a complete snapshot as "PID START_TIME" lines. Status 1 means ABSENT;
# status 2 means procfs or candidate identity is incomplete (UNKNOWN).
ss_process_identity_snapshot() {
	[ "$#" -eq 2 ] || return 2
	local ss_snapshot_mode="$1"
	local ss_snapshot_target="$2"
	local ss_snapshot_root="${SS_PROC_ROOT:-/proc}"
	local ss_snapshot_entries
	local ss_snapshot_pid
	local ss_snapshot_dir
	local ss_snapshot_comm
	local ss_snapshot_exe
	local ss_snapshot_has_exe
	local ss_snapshot_candidate
	local ss_snapshot_expected
	local ss_snapshot_comm_prefix
	local ss_snapshot_actual_prefix
	local ss_snapshot_status
	local ss_snapshot_output=""
	local ss_snapshot_numeric_seen=0

	case "$ss_snapshot_mode" in
		named)
			ss_snapshot_expected=$(ss_expected_executable "$ss_snapshot_target") || return 2
			ss_snapshot_comm_prefix=$(printf '%.15s' "${ss_snapshot_target##*/}")
			;;
		script)
			[ "$ss_snapshot_target" = /usr/bin/ss-monitor ] || return 2
			command -v tr >/dev/null 2>&1 || return 2
			;;
		*)
			return 2
			;;
	esac
	[ -d "$ss_snapshot_root" ] && [ -r "$ss_snapshot_root" ] && [ -x "$ss_snapshot_root" ] || return 2
	command -v ls >/dev/null 2>&1 || return 2
	command -v cat >/dev/null 2>&1 || return 2
	command -v readlink >/dev/null 2>&1 || return 2
	if ss_snapshot_entries=$(ls -1 "$ss_snapshot_root" 2>/dev/null); then
		:
	else
		return 2
	fi
	[ -n "$ss_snapshot_entries" ] || return 2

	while IFS= read -r ss_snapshot_pid; do
		case "$ss_snapshot_pid" in
			''|*[!0-9]*) continue ;;
			0*) return 2 ;;
		esac
		ss_snapshot_dir="$ss_snapshot_root/$ss_snapshot_pid"
		[ -d "$ss_snapshot_dir" ] || continue
		ss_snapshot_numeric_seen=1
		if ss_snapshot_comm=$(cat "$ss_snapshot_dir/comm" 2>/dev/null); then
			:
		else
			[ -d "$ss_snapshot_dir" ] || continue
			return 2
		fi

		ss_snapshot_exe=""
		ss_snapshot_has_exe=0
		if [ -L "$ss_snapshot_dir/exe" ]; then
			if ss_snapshot_exe=$(readlink "$ss_snapshot_dir/exe" 2>/dev/null); then
				ss_snapshot_has_exe=1
			else
				[ -d "$ss_snapshot_dir" ] || continue
				return 2
			fi
		fi

		ss_snapshot_candidate=0
		if [ "$ss_snapshot_mode" = named ]; then
			if [ "$ss_snapshot_has_exe" -eq 1 ]; then
				if ss_named_executable_matches "$ss_snapshot_target" "$ss_snapshot_exe"; then
					ss_snapshot_candidate=1
				else
					# An executable still mapped from the expected installation
					# path but marked deleted is not a trustworthy replacement.
					if [ "$ss_snapshot_exe" = "$ss_snapshot_expected (deleted)" ]; then
						return 2
					fi
					case "${ss_snapshot_target##*/}" in
						v2ray|xray)
							case "$ss_snapshot_exe" in
								'/usr/bin/v2ray (deleted)'|'/usr/bin/xray (deleted)') return 2 ;;
							esac
							;;
					esac
					# A same-name comm with a different executable is unverified.
					# Never signal it, and never report the service as absent.
					ss_snapshot_actual_prefix=$(printf '%.15s' "$ss_snapshot_comm")
					if [ "$ss_snapshot_actual_prefix" = "$ss_snapshot_comm_prefix" ]; then
						return 2
					fi
				fi
			else
				ss_snapshot_actual_prefix=$(printf '%.15s' "$ss_snapshot_comm")
				if [ "$ss_snapshot_actual_prefix" = "$ss_snapshot_comm_prefix" ]; then
					# A possible target whose executable link cannot be read
					# cannot safely be classified as either ours or foreign.
					return 2
				fi
			fi
		else
			if [ "$ss_snapshot_has_exe" -eq 1 ]; then
				if ss_shell_executable_matches "$ss_snapshot_exe"; then
					if ss_script_argv_matches "$ss_snapshot_dir" "$ss_snapshot_target"; then
						ss_snapshot_candidate=1
					else
						ss_snapshot_status=$?
						[ "$ss_snapshot_status" -eq 1 ] || return 2
					fi
				fi
			else
				case "$ss_snapshot_comm" in
					sh|ash|busybox)
						if ss_script_argv_matches "$ss_snapshot_dir" "$ss_snapshot_target"; then
							return 2
						else
							ss_snapshot_status=$?
							[ "$ss_snapshot_status" -eq 1 ] || return 2
						fi
						;;
				esac
			fi
		fi
		[ "$ss_snapshot_candidate" -eq 1 ] || continue

		if ss_read_proc_identity "$ss_snapshot_pid"; then
			:
		else
			ss_snapshot_status=$?
			[ "$ss_snapshot_status" -eq 1 ] && continue
			return 2
		fi
		case "$SS_PROC_TASK_STATE" in Z|X|x) continue ;; esac
		ss_snapshot_output="$ss_snapshot_output$ss_snapshot_pid $SS_PROC_START_TIME
"
	done <<EOF_PROC_IDENTITY
$ss_snapshot_entries
EOF_PROC_IDENTITY
	[ "$ss_snapshot_numeric_seen" -eq 1 ] || return 2
	[ -n "$ss_snapshot_output" ] || return 1
	printf '%s' "$ss_snapshot_output"
	return 0
}

# Recheck PID, start_time and executable immediately before every signal.
# Linux 4.4 lacks pidfd, so a narrow check-to-signal race remains.
ss_process_identity_matches() {
	[ "$#" -eq 4 ] || return 2
	local ss_identity_mode="$1"
	local ss_identity_target="$2"
	local ss_identity_pid="$3"
	local ss_identity_start="$4"
	local ss_identity_dir="${SS_PROC_ROOT:-/proc}/$3"
	local ss_identity_exe
	local ss_identity_status

	if ss_read_proc_identity "$ss_identity_pid"; then
		:
	else
		return $?
	fi
	[ "$SS_PROC_START_TIME" = "$ss_identity_start" ] || return 1
	case "$SS_PROC_TASK_STATE" in Z|X|x) return 1 ;; esac
	if [ ! -L "$ss_identity_dir/exe" ]; then
		[ -d "$ss_identity_dir" ] || return 1
		return 2
	fi
	if ss_identity_exe=$(readlink "$ss_identity_dir/exe" 2>/dev/null); then
		:
	else
		[ -d "$ss_identity_dir" ] || return 1
		return 2
	fi
	case "$ss_identity_mode" in
		named)
			if ss_named_executable_matches "$ss_identity_target" "$ss_identity_exe"; then
				return 0
			fi
			return 1
			;;
		script)
			ss_shell_executable_matches "$ss_identity_exe" || return 1
			if ss_script_argv_matches "$ss_identity_dir" "$ss_identity_target"; then
				return 0
			else
				ss_identity_status=$?
				return "$ss_identity_status"
			fi
			;;
		*)
			return 2
			;;
	esac
}

ss_wait_identity_absent() {
	[ "$#" -eq 3 ] || return 2
	local ss_wait_mode="$1"
	local ss_wait_target="$2"
	local ss_wait_limit="$3"
	local ss_wait_count=0
	local ss_wait_snapshot
	local ss_wait_status

	case "$ss_wait_limit" in ''|*[!0-9]*) return 2 ;; esac
	[ "$ss_wait_limit" -le 30 ] || return 2
	while :; do
		if ss_wait_snapshot=$(ss_process_identity_snapshot "$ss_wait_mode" "$ss_wait_target"); then
			ss_wait_status=0
		else
			ss_wait_status=$?
		fi
		case "$ss_wait_status" in
			1) return 0 ;;
			2) return 2 ;;
			0) ;;
			*) return 2 ;;
		esac
		[ "$ss_wait_count" -lt "$ss_wait_limit" ] || return 1
		sleep 1 || return 2
		ss_wait_count=$((ss_wait_count + 1))
	done
}

ss_stop_process_identity() {
	[ "$#" -eq 2 ] || return 2
	local ss_stop_mode="$1"
	local ss_stop_target="$2"
	local ss_stop_snapshot
	local ss_stop_status
	local ss_stop_pid
	local ss_stop_start
	local ss_stop_signal_status
	local ss_stop_wait_status

	if ss_stop_snapshot=$(ss_process_identity_snapshot "$ss_stop_mode" "$ss_stop_target"); then
		:
	else
		ss_stop_status=$?
		case "$ss_stop_status" in 1) return 0 ;; 2) return 2 ;; *) return 2 ;; esac
	fi

	while IFS=' ' read -r ss_stop_pid ss_stop_start; do
		[ -n "$ss_stop_pid" ] || continue
		if ss_process_identity_matches "$ss_stop_mode" "$ss_stop_target" "$ss_stop_pid" "$ss_stop_start"; then
			log "Closing $ss_stop_target PID $ss_stop_pid..."
			if kill -TERM "$ss_stop_pid" 2>/dev/null; then
				:
			else
				ss_stop_signal_status=$?
				if ss_process_identity_matches "$ss_stop_mode" "$ss_stop_target" "$ss_stop_pid" "$ss_stop_start"; then
					return "$ss_stop_signal_status"
				else
					ss_stop_status=$?
					[ "$ss_stop_status" -eq 1 ] || return 2
				fi
			fi
		else
			ss_stop_status=$?
			[ "$ss_stop_status" -eq 1 ] || return 2
		fi
	done <<EOF_PROC_TERM
$ss_stop_snapshot
EOF_PROC_TERM

	if ss_wait_identity_absent "$ss_stop_mode" "$ss_stop_target" 3; then
		return 0
	else
		ss_stop_wait_status=$?
	fi
	[ "$ss_stop_wait_status" -eq 2 ] && return 2
	[ "$ss_stop_wait_status" -eq 1 ] || return 2

	# Escalate only members of the original PID/start_time snapshot. A
	# replacement process, including one reusing the same PID and executable,
	# must never inherit the old process's KILL authorization.
	while IFS=' ' read -r ss_stop_pid ss_stop_start; do
		[ -n "$ss_stop_pid" ] || continue
		if ss_process_identity_matches "$ss_stop_mode" "$ss_stop_target" "$ss_stop_pid" "$ss_stop_start"; then
			log "Force-closing $ss_stop_target PID $ss_stop_pid..."
			if kill -KILL "$ss_stop_pid" 2>/dev/null; then
				:
			else
				ss_stop_signal_status=$?
				if ss_process_identity_matches "$ss_stop_mode" "$ss_stop_target" "$ss_stop_pid" "$ss_stop_start"; then
					return "$ss_stop_signal_status"
				else
					ss_stop_status=$?
					[ "$ss_stop_status" -eq 1 ] || return 2
				fi
			fi
		else
			ss_stop_status=$?
			[ "$ss_stop_status" -eq 1 ] || return 2
		fi
	done <<EOF_PROC_KILL
$ss_stop_snapshot
EOF_PROC_KILL

	if ss_wait_identity_absent "$ss_stop_mode" "$ss_stop_target" 1; then
		return 0
	else
		ss_stop_wait_status=$?
	fi
	case "$ss_stop_wait_status" in 1) return 1 ;; *) return 2 ;; esac
}

ss_stop_named_processes() {
	[ "$#" -gt 0 ] || return 0
	local ss_process_name
	local ss_stop_status
	local ss_first_status=0
	for ss_process_name in "$@"; do
		if ss_stop_process_identity named "$ss_process_name"; then
			:
		else
			ss_stop_status=$?
			[ "$ss_first_status" -ne 0 ] || ss_first_status=$ss_stop_status
		fi
	done
	return "$ss_first_status"
}

# Retain the existing caller API but accept only the one intended script.
# This replaces pgrep/pkill regex rematching with exact argv and interpreter
# validation plus PID/start_time-scoped TERM/KILL.
ss_stop_process_pattern() {
	[ "$#" -eq 1 ] || return 2
	[ "$1" = /usr/bin/ss-monitor ] || return 2
	ss_stop_process_identity script "$1"
}

