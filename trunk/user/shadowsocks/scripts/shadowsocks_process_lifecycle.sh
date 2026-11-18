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

ss_process_pid_state() {
	ss_process_name=$1
	ss_pid_output=$(pidof "$ss_process_name" 2>/dev/null)
	ss_pid_status=$?
	case "$ss_pid_status" in
		0) ss_validate_pid_list "$ss_pid_output" || return 2; return 0 ;;
		1) [ -z "$ss_pid_output" ] && return 1; return 2 ;;
		*) return 2 ;;
	esac
}

# Wait for a named daemon to appear. Confirmed absence is retryable; an
# invalid or failed process inventory is UNKNOWN and must fail closed.
ss_process_pid_count() {
	[ "$#" -eq 1 ] || return 2
	ss_count_process_name=$1
	[ -n "$ss_count_process_name" ] || return 2
	ss_count_pid_output=$(pidof "$ss_count_process_name" 2>/dev/null)
	ss_count_pid_status=$?
	case "$ss_count_pid_status" in
		0)
			ss_validate_pid_list "$ss_count_pid_output" || return 2
			printf '%s\n' "$ss_count_pid_output" | awk '{ count += NF } END { print count + 0 }'
			;;
		1)
			[ -z "$ss_count_pid_output" ] || return 2
			printf '0\n'
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
	ss_pid_output=$(pgrep -f "$ss_process_pattern" 2>/dev/null)
	ss_pid_status=$?
	case "$ss_pid_status" in
		0) ss_validate_pid_list "$ss_pid_output" || return 2; return 0 ;;
		1) [ -z "$ss_pid_output" ] && return 1; return 2 ;;
		*) return 2 ;;
	esac
}

ss_process_pattern_count() {
	[ "$#" -eq 1 ] || return 2
	ss_count_pattern=$1
	[ -n "$ss_count_pattern" ] || return 2
	ss_pattern_pid_output=$(pgrep -f "$ss_count_pattern" 2>/dev/null)
	ss_pattern_pid_status=$?
	case "$ss_pattern_pid_status" in
		0)
			ss_validate_pid_list "$ss_pattern_pid_output" || return 2
			printf '%s\n' "$ss_pattern_pid_output" | awk '{ count += NF } END { print count + 0 }'
			;;
		1)
			[ -z "$ss_pattern_pid_output" ] || return 2
			printf '0\n'
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

ss_stop_named_processes() {
	[ "$#" -gt 0 ] || return 0
	if ss_named_processes_state "$@"; then ss_state=0; else ss_state=$?; fi
	case "$ss_state" in 1) return 0 ;; 0) ;; *) return 2 ;; esac

	# Fresh validated lookups avoid stale PID snapshots and fail closed.
	for ss_process_name in "$@"; do
		if ss_process_pid_state "$ss_process_name"; then ss_state=0; else ss_state=$?; fi
		case "$ss_state" in 1) continue ;; 0) ;; *) return 2 ;; esac
		log "Closing $ss_process_name process..."
		if killall -q "$ss_process_name" >/dev/null 2>&1; then
			:
		else
			if ss_process_pid_state "$ss_process_name"; then ss_state=0; else ss_state=$?; fi
			case "$ss_state" in 1) ;; 2) return 2 ;; *) return 1 ;; esac
		fi
	done

	if ss_wait_named_processes 3 "$@"; then return 0; else ss_wait_status=$?; fi
	case "$ss_wait_status" in 2) return 2 ;; 1) ;; *) return 1 ;; esac

	# A new successful name lookup is required before KILL escalation.
	for ss_process_name in "$@"; do
		if ss_process_pid_state "$ss_process_name"; then ss_state=0; else ss_state=$?; fi
		case "$ss_state" in 1) continue ;; 0) ;; *) return 2 ;; esac
		log "Force-closing $ss_process_name process..."
		if killall -q -KILL "$ss_process_name" >/dev/null 2>&1; then
			:
		else
			if ss_process_pid_state "$ss_process_name"; then ss_state=0; else ss_state=$?; fi
			case "$ss_state" in 1) ;; 2) return 2 ;; *) return 1 ;; esac
		fi
	done

	if ss_wait_named_processes 1 "$@"; then return 0; else ss_wait_status=$?; fi
	case "$ss_wait_status" in 1) return 1 ;; *) return 2 ;; esac
}

ss_stop_process_pattern() {
	ss_process_pattern=$1
	[ -n "$ss_process_pattern" ] || return 1
	if ss_process_pattern_state "$ss_process_pattern"; then ss_state=0; else ss_state=$?; fi
	case "$ss_state" in 1) return 0 ;; 0) ;; *) return 2 ;; esac

	log "Closing process matching $ss_process_pattern..."
	if pkill -TERM -f "$ss_process_pattern" >/dev/null 2>&1; then
		:
	else
		if ss_process_pattern_state "$ss_process_pattern"; then ss_state=0; else ss_state=$?; fi
		case "$ss_state" in 1) return 0 ;; 2) return 2 ;; *) return 1 ;; esac
	fi
	if ss_wait_process_pattern 3 "$ss_process_pattern"; then return 0; else ss_wait_status=$?; fi
	case "$ss_wait_status" in 2) return 2 ;; 1) ;; *) return 1 ;; esac

	# pgrep must succeed again before pkill rematches the command line.
	if ss_process_pattern_state "$ss_process_pattern"; then ss_state=0; else ss_state=$?; fi
	case "$ss_state" in 1) return 0 ;; 0) ;; *) return 2 ;; esac
	log "Force-closing process matching $ss_process_pattern..."
	if pkill -KILL -f "$ss_process_pattern" >/dev/null 2>&1; then
		:
	else
		if ss_process_pattern_state "$ss_process_pattern"; then ss_state=0; else ss_state=$?; fi
		case "$ss_state" in 1) return 0 ;; 2) return 2 ;; *) return 1 ;; esac
	fi
	if ss_wait_process_pattern 1 "$ss_process_pattern"; then return 0; else ss_wait_status=$?; fi
	case "$ss_wait_status" in 1) return 1 ;; *) return 2 ;; esac
}
