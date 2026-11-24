#!/bin/sh
# SmartDNS process inventory is tri-state: RUNNING, ABSENT, or UNKNOWN.
# Adbyby uses a separate generic read-only name-based process probe.
smartdns_named_process_state()
{
    _smartdns_name=$1
    [ "$#" -eq 1 ] && [ -n "$_smartdns_name" ] || return 2
    _smartdns_pids=$(pidof "$_smartdns_name" 2>/dev/null)
    _smartdns_pid_status=$?
    case "$_smartdns_pid_status" in
        0) [ -n "$_smartdns_pids" ] || return 2 ;;
        1) [ -z "$_smartdns_pids" ] && return 1; return 2 ;;
        *) return 2 ;;
    esac
    case "$_smartdns_pids" in *[!0123456789\ ]*) return 2 ;; esac
    case $- in
        *f*) _smartdns_restore_glob=0 ;;
        *) set -f; _smartdns_restore_glob=1 ;;
    esac
    set -- $_smartdns_pids
    [ "$_smartdns_restore_glob" -eq 0 ] || set +f
    [ "$#" -gt 0 ] || return 2
    for _smartdns_pid in "$@"; do
        case "$_smartdns_pid" in ''|0*|*[!0-9]*) return 2 ;; esac
    done
    return 0
}

# Return start_time only when /proc proves this PID is the configured SmartDNS
# executable with the complete argv tuple used by Start_smartdns().
# Status 1 means absent/non-matching; 2 means identity unknown.
smartdns_process_identity()
{
    _smartdns_identity_pid=$1
    case "$_smartdns_identity_pid" in ''|0|1|0*|*[!0-9]*) return 1 ;; esac
    _smartdns_proc_root=/proc
    [ -n "$SMARTDNS_PROC_ROOT" ] && _smartdns_proc_root=$SMARTDNS_PROC_ROOT
    _smartdns_proc_dir="$_smartdns_proc_root/$_smartdns_identity_pid"
    [ -d "$_smartdns_proc_dir" ] || return 1
    [ -L "$_smartdns_proc_dir/exe" ] || return 2

    _smartdns_expected_exe=$smartdns_Bin
    [ -n "$_smartdns_expected_exe" ] || _smartdns_expected_exe=/usr/bin/smartdns
    _smartdns_actual_exe=$(readlink "$_smartdns_proc_dir/exe" 2>/dev/null) || return 2
    case "$_smartdns_actual_exe" in
        "$_smartdns_expected_exe"|"$_smartdns_expected_exe (deleted)") ;;
        *) return 1 ;;
    esac

    [ -r "$_smartdns_proc_dir/stat" ] && [ -f "$_smartdns_proc_dir/stat" ] || return 2
    _smartdns_start_time=$(awk '
        NR != 1 { invalid = 1 }
        NR == 1 {
            sub(/^.*\) /, "")
            if (NF < 20 || $1 !~ /^[A-Za-z]$/ ||
                $20 !~ /^[0-9]+$/ ||
                $1 == "Z" || $1 == "X" || $1 == "x")
                invalid = 1
            else
                start_time = $20
        }
        END {
            if (invalid || NR != 1)
                exit 1
            print start_time
        }
    ' "$_smartdns_proc_dir/stat" 2>/dev/null) || return 2
    case "$_smartdns_start_time" in ''|*[!0-9]*) return 2 ;; esac

    [ -r "$_smartdns_proc_dir/cmdline" ] &&
        [ -f "$_smartdns_proc_dir/cmdline" ] &&
        [ -s "$_smartdns_proc_dir/cmdline" ] || return 2
    _smartdns_expected_config=$smartdns_Conf
    [ -n "$_smartdns_expected_config" ] || _smartdns_expected_config=/etc/storage/smartdns.conf
    if ! tr '\000' '\n' < "$_smartdns_proc_dir/cmdline" 2>/dev/null |
        awk -v executable="$_smartdns_expected_exe" -v config="$_smartdns_expected_config" '
            NR == 1 { if ($0 != executable) invalid = 1; next }
            NR == 2 { if ($0 != "-f") invalid = 1; next }
            NR == 3 { if ($0 != "-c") invalid = 1; next }
            NR == 4 { if ($0 != config) invalid = 1; next }
            NR == 5 {
                if ($0 != "" && $0 != "-S")
                    invalid = 1
                optional_arg = 1
                next
            }
            { invalid = 1 }
            END {
                if ((NR == 4 || (NR == 5 && optional_arg)) && !invalid)
                    exit 0
                exit 1
            }
        '; then
        return 1
    fi
    printf '%s\n' "$_smartdns_start_time"
}

# pidof supplies candidate PIDs only; it never authorizes process control.
smartdns_process_state()
{
    smartdns_process_pids=
    smartdns_process_ids=
    _smartdns_proc_root=/proc
    [ -n "$SMARTDNS_PROC_ROOT" ] && _smartdns_proc_root=$SMARTDNS_PROC_ROOT
    _smartdns_pid_output=$(pidof smartdns 2>/dev/null)
    _smartdns_pid_status=$?
    case "$_smartdns_pid_status" in
        0)
            [ -n "$_smartdns_pid_output" ] || return 2
            case "$_smartdns_pid_output" in *[!0123456789\ ]*) return 2 ;; esac
            case $- in
                *f*) _smartdns_restore_glob=0 ;;
                *) set -f; _smartdns_restore_glob=1 ;;
            esac
            _smartdns_saved_ifs=$IFS
            IFS=' '
            set -- $_smartdns_pid_output
            IFS=$_smartdns_saved_ifs
            [ "$_smartdns_restore_glob" -eq 0 ] || set +f
            [ "$#" -gt 0 ] || return 2
            for _smartdns_candidate in "$@"; do
                case "$_smartdns_candidate" in ''|0|1|0*|*[!0-9]*) return 2 ;; esac
                if _smartdns_candidate_start=$(smartdns_process_identity "$_smartdns_candidate"); then
                    if [ -n "$smartdns_process_ids" ]; then
                        smartdns_process_ids="$smartdns_process_ids $_smartdns_candidate:$_smartdns_candidate_start"
                        smartdns_process_pids="$smartdns_process_pids $_smartdns_candidate"
                    else
                        smartdns_process_ids="$_smartdns_candidate:$_smartdns_candidate_start"
                        smartdns_process_pids=$_smartdns_candidate
                    fi
                else
                    _smartdns_candidate_status=$?
                    case "$_smartdns_candidate_status" in 1) ;; *) return 2 ;; esac
                fi
            done
            [ -n "$smartdns_process_ids" ] && return 0
            ;;
        1) [ -z "$_smartdns_pid_output" ] || return 2 ;;
        *) return 2 ;;
    esac
    [ -r "$_smartdns_proc_root/1/stat" ] && return 1
    return 2
}

# Return 0 when absent, 1 when still running after bounded wait, 2 unknown.
smartdns_wait_stopped()
{
    _smartdns_wait_count=0
    while [ "$_smartdns_wait_count" -lt 3 ]; do
        if smartdns_process_state; then _smartdns_wait_state=0
        else _smartdns_wait_state=$?; fi
        case "$_smartdns_wait_state" in
            1) return 0 ;;
            2) return 2 ;;
            0) ;;
        esac
        _smartdns_wait_count=$((_smartdns_wait_count + 1))
        if [ "$_smartdns_wait_count" -lt 3 ]; then sleep 1 || return 2; fi
    done
    if smartdns_process_state; then return 1
    else _smartdns_wait_state=$?; fi
    case "$_smartdns_wait_state" in
        1) return 0 ;;
        2) return 2 ;;
        *) return 1 ;;
    esac
}

# Revalidate each PID/start_time immediately before its PID-scoped signal.
smartdns_signal_process_ids()
{
    _smartdns_signal=$1
    _smartdns_signal_ids=$2
    case "$_smartdns_signal" in TERM|KILL) ;; *) return 2 ;; esac
    _smartdns_signal_status=0
    for _smartdns_identity in $_smartdns_signal_ids; do
        case "$_smartdns_identity" in *:*) ;; *) return 2 ;; esac
        _smartdns_signal_pid=$(printf '%s\n' "$_smartdns_identity" | cut -d: -f1)
        _smartdns_signal_start=$(printf '%s\n' "$_smartdns_identity" | cut -d: -f2)
        case "$_smartdns_signal_start" in ''|*[!0-9]*) return 2 ;; esac
        if _smartdns_current_start=$(smartdns_process_identity "$_smartdns_signal_pid"); then
            [ "$_smartdns_current_start" = "$_smartdns_signal_start" ] || continue
        else
            _smartdns_identity_status=$?
            case "$_smartdns_identity_status" in 1) continue ;; *) return 2 ;; esac
        fi
        if kill "-$_smartdns_signal" "$_smartdns_signal_pid" >/dev/null 2>&1; then
            :
        else
            _smartdns_kill_status=$?
            if [ "$_smartdns_signal_status" -eq 0 ]; then
                _smartdns_signal_status=$_smartdns_kill_status
                [ "$_smartdns_signal_status" -ne 0 ] || _smartdns_signal_status=1
            fi
        fi
    done
    return "$_smartdns_signal_status"
}

smartdns_stop_process()
{
    if smartdns_process_state; then _smartdns_state=0
    else _smartdns_state=$?; fi
    case "$_smartdns_state" in
        1) return 0 ;;
        0) _smartdns_stop_ids=$smartdns_process_ids ;;
        *) return 2 ;;
    esac

    # Linux 4.4 has no pidfd; a final validate-to-kill race remains.
    smartdns_signal_process_ids TERM "$_smartdns_stop_ids" || {
        _smartdns_signal_status=$?
        [ "$_smartdns_signal_status" -ne 2 ] || return 2
    }
    smartdns_wait_stopped
    _smartdns_wait_status=$?
    case "$_smartdns_wait_status" in
        0) return 0 ;;
        2) return 2 ;;
        1) _smartdns_kill_ids=$_smartdns_stop_ids ;;
        *) return 2 ;;
    esac
    smartdns_signal_process_ids KILL "$_smartdns_kill_ids" || {
        _smartdns_signal_status=$?
        [ "$_smartdns_signal_status" -ne 2 ] || return 2
    }
    smartdns_wait_stopped
}
