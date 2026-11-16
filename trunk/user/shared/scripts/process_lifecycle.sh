#!/bin/sh
# Shared POSIX process lifecycle primitives.
# pl_process_state NAME: 0=RUNNING, 1=confirmed ABSENT, 2=UNKNOWN.
pl_process_state()
{
    pl_name="$1"
    [ -n "$pl_name" ] || return 2
    command -v pidof >/dev/null 2>&1 || return 2
    if pl_pid_output=$(pidof "$pl_name" 2>/dev/null); then pl_pid_status=0; else pl_pid_status=$?; fi
    case "$pl_pid_status" in
        0)
            [ -n "$pl_pid_output" ] || return 2
            printf '%s\n' "$pl_pid_output" |
                awk '{ for (i = 1; i <= NF; i++) { if ($i !~ /^[1-9][0-9]*$/) invalid = 1; count++ } }
                     END { if (invalid || count == 0) exit 1; exit 0 }' >/dev/null 2>&1 || return 2
            return 0 ;;
        1) [ -z "$pl_pid_output" ] && return 1; return 2 ;;
        *) return 2 ;;
    esac
}
pl_wait_stopped()
{
    pl_wait_name="$1"; pl_wait_limit="$2"
    case "$pl_wait_limit" in ''|*[!0-9]*) return 2 ;; esac
    pl_wait_count=0
    while :; do
        if pl_process_state "$pl_wait_name"; then pl_wait_state=0; else pl_wait_state=$?; fi
        case "$pl_wait_state" in 1) return 0 ;; 0) ;; *) return 2 ;; esac
        [ "$pl_wait_count" -lt "$pl_wait_limit" ] || return 1
        sleep 1 || return 2
        pl_wait_count=$((pl_wait_count + 1))
    done
}
# TERM first; revalidate before SIGKILL. Only a fresh ABSENT snapshot succeeds.
pl_stop_named()
{
    pl_stop_name="$1"; pl_term_wait="$2"; pl_kill_wait="$3"
    [ -n "$pl_stop_name" ] || return 2
    if pl_process_state "$pl_stop_name"; then pl_stop_state=0; else pl_stop_state=$?; fi
    case "$pl_stop_state" in 1) return 0 ;; 0) ;; *) return 2 ;; esac
    command -v killall >/dev/null 2>&1 || return 2
    killall -q "$pl_stop_name" >/dev/null 2>&1 || :
    if pl_wait_stopped "$pl_stop_name" "$pl_term_wait"; then return 0; else pl_wait_status=$?; fi
    case "$pl_wait_status" in 2) return 2 ;; 1) ;; *) return 2 ;; esac
    # A fresh inventory is mandatory before escalation.
    if pl_process_state "$pl_stop_name"; then pl_stop_state=0; else pl_stop_state=$?; fi
    case "$pl_stop_state" in 1) return 0 ;; 0) ;; *) return 2 ;; esac
    killall -q -9 "$pl_stop_name" >/dev/null 2>&1 || :
    if pl_wait_stopped "$pl_stop_name" "$pl_kill_wait"; then return 0; else pl_wait_status=$?; fi
    case "$pl_wait_status" in 1) return 1 ;; *) return 2 ;; esac
}
