#!/bin/sh
# SmartDNS process state and stop helpers.
# Return states: 0 running, 1 absent, 2 unknown.
smartdns_process_state()
{
    _smartdns_pid_output=$(pidof smartdns 2>/dev/null)
    _smartdns_pid_status=$?
    case "$_smartdns_pid_status" in
        0)
            [ -n "$_smartdns_pid_output" ] || return 2
            ;;
        1)
            [ -z "$_smartdns_pid_output" ] && return 1
            return 2
            ;;
        *)
            return 2
            ;;
    esac

    case "$_smartdns_pid_output" in
        *[!0123456789\ ]*) return 2 ;;
    esac

    case $- in
        *f*) _smartdns_restore_noglob=0 ;;
        *) set -f; _smartdns_restore_noglob=1 ;;
    esac
    set -- $_smartdns_pid_output
    [ "$_smartdns_restore_noglob" -eq 0 ] || set +f
    [ "$#" -gt 0 ] || return 2
    for _smartdns_pid in "$@"; do
        case "$_smartdns_pid" in
            ''|0*|*[!0-9]*) return 2 ;;
        esac
    done
    return 0
}

smartdns_wait_stopped()
{
    _smartdns_attempt=0
    while [ "$_smartdns_attempt" -lt 3 ]; do
        smartdns_process_state
        _smartdns_state=$?
        case "$_smartdns_state" in
            0) ;;
            1) return 0 ;;
            *) return 2 ;;
        esac
        _smartdns_attempt=$((_smartdns_attempt + 1))
        [ "$_smartdns_attempt" -lt 3 ] && sleep 1
    done
    return 1
}

smartdns_stop_process()
{
    smartdns_process_state
    _smartdns_state=$?
    case "$_smartdns_state" in
        1) return 0 ;;
        0) ;;
        *) return 2 ;;
    esac

    # Verify actual process absence after each signal; killall's exit status is
    # not a reliable postcondition when a daemon exits between scan and signal.
    killall smartdns >/dev/null 2>&1 || true
    smartdns_wait_stopped
    _smartdns_stop_status=$?
    case "$_smartdns_stop_status" in
        0) return 0 ;;
        2) return 2 ;;
    esac

    killall -9 smartdns >/dev/null 2>&1 || true
    smartdns_wait_stopped
}
