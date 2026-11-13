#!/bin/sh
#
# Bounded service shutdown without retaining process-ID snapshots across TERM.

ss_any_named_process_running() {
	for ss_process_name in "$@"; do
		if pidof "$ss_process_name" >/dev/null 2>&1; then
			return 0
		fi
	done
	return 1
}

ss_stop_named_processes() {
	if [ "$#" -eq 0 ]; then
		return 0
	fi

	for ss_process_name in "$@"; do
		if pidof "$ss_process_name" >/dev/null 2>&1; then
			log "Closing $ss_process_name process..."
			killall -q "$ss_process_name" >/dev/null 2>&1
		fi
	done

	ss_wait_count=0
	while ss_any_named_process_running "$@" && [ "$ss_wait_count" -lt 3 ]; do
		sleep 1
		ss_wait_count=$((ss_wait_count + 1))
	done

	# A fresh name lookup prevents a delayed signal to a recycled PID.
	for ss_process_name in "$@"; do
		if pidof "$ss_process_name" >/dev/null 2>&1; then
			log "Force-closing $ss_process_name process..."
			killall -q -KILL "$ss_process_name" >/dev/null 2>&1
		fi
	done

	ss_wait_count=0
	while ss_any_named_process_running "$@" && [ "$ss_wait_count" -lt 1 ]; do
		sleep 1
		ss_wait_count=$((ss_wait_count + 1))
	done
	if ss_any_named_process_running "$@"; then
		for ss_process_name in "$@"; do
			if pidof "$ss_process_name" >/dev/null 2>&1; then
				log "Failed to stop $ss_process_name: process remains active"
			fi
		done
		return 1
	fi
	return 0
}

ss_any_process_pattern_running() {
	pgrep -f "$1" >/dev/null 2>&1
}

ss_stop_process_pattern() {
	ss_process_pattern=$1
	[ -n "$ss_process_pattern" ] || return 1

	if pgrep -f "$ss_process_pattern" >/dev/null 2>&1; then
		log "Closing process matching $ss_process_pattern..."
		pkill -TERM -f "$ss_process_pattern" >/dev/null 2>&1
	fi

	ss_wait_count=0
	while ss_any_process_pattern_running "$ss_process_pattern" &&
	      [ "$ss_wait_count" -lt 3 ]; do
		sleep 1
		ss_wait_count=$((ss_wait_count + 1))
	done

	# pkill re-matches the full command line at escalation time. Do not reuse
	# a PID list captured before the grace period.
	if pgrep -f "$ss_process_pattern" >/dev/null 2>&1; then
		log "Force-closing process matching $ss_process_pattern..."
		pkill -KILL -f "$ss_process_pattern" >/dev/null 2>&1
	fi

	ss_wait_count=0
	while ss_any_process_pattern_running "$ss_process_pattern" &&
	      [ "$ss_wait_count" -lt 1 ]; do
		sleep 1
		ss_wait_count=$((ss_wait_count + 1))
	done
	if ss_any_process_pattern_running "$ss_process_pattern"; then
		log "Failed to stop process matching $ss_process_pattern"
		return 1
	fi
	return 0
}
