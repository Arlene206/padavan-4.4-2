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
        1)
            [ -z "$pl_pid_output" ] || return 2
            # pidof status 1 is ambiguous on BusyBox: it can mean either
            # "not found" or an incomplete procfs scan. Confirm absence by
            # enumerating every visible numeric PID and reading its stat file.
            pl_proc_root="${PL_PROC_ROOT:-/proc}"
            if [ ! -d "$pl_proc_root" ] || [ ! -r "$pl_proc_root" ] || [ ! -x "$pl_proc_root" ]; then
                return 2
            fi
            command -v ls >/dev/null 2>&1 || return 2
            pl_proc_listing=$(ls -1 "$pl_proc_root" 2>/dev/null) || return 2
            # procfs names are newline-delimited. Do not use the caller's
            # IFS for this inventory: callers may deliberately set IFS=:.
            while IFS= read -r pl_proc_pid; do
                case "$pl_proc_pid" in ''|*[!0-9]*) continue ;; esac
                pl_proc_dir="$pl_proc_root/$pl_proc_pid"
                [ -d "$pl_proc_dir" ] || continue
                if IFS= read -r pl_proc_stat < "$pl_proc_dir/stat"; then
                    :
                else
                    # PID exit between enumeration and read is normal; a
                    # still-present directory with unreadable stat is UNKNOWN.
                    [ -d "$pl_proc_dir" ] || continue
                    return 2
                fi
                # The PID field must match the procfs directory. Parsing
                # only a command name and a nonempty state can misclassify a
                # truncated or malformed stat record as a live task.
                case "$pl_proc_stat" in
                    "$pl_proc_pid ("*) ;;
                    *) return 2 ;;
                esac
                pl_proc_name=${pl_proc_stat#*(}
                pl_proc_name=${pl_proc_name%)*}
                pl_proc_fields=${pl_proc_stat##*) }
                # A valid /proc/PID/stat record contains fields 3 through 22.
                # Checking only PID/state accepted newline-terminated truncated
                # records as a complete process observation.
                # Preserve both the value and set/unset state of IFS. An
                # unset IFS means the shell's default whitespace separators;
                # restoring it as an explicitly empty value changes caller behavior.
                if [ "${IFS+x}" = x ]; then
                    pl_proc_ifs_was_set=1
                    pl_proc_saved_ifs=$IFS
                else
                    pl_proc_ifs_was_set=0
                fi
                case "$-" in *f*) pl_proc_glob_disabled=1 ;; *) pl_proc_glob_disabled=0 ;; esac
                set -f
                IFS=' '
                set -- $pl_proc_fields
                if [ "$pl_proc_ifs_was_set" -eq 1 ]; then
                    IFS=$pl_proc_saved_ifs
                else
                    unset IFS
                fi
                [ "$pl_proc_glob_disabled" -eq 1 ] || set +f
                [ "$#" -ge 20 ] || return 2
                pl_proc_state=$1
                case "${20}" in ''|*[!0-9]*) return 2 ;; esac
                case "$pl_proc_state" in
                    R|S|D|Z|T|t|X|x|K|W|P|I) ;;
                    *) return 2 ;;
                esac
                case "$pl_proc_state" in Z|X|x) continue ;; esac
                [ "$pl_proc_name" = "$pl_name" ] && return 0
            done <<EOF
$pl_proc_listing
EOF
            return 1 ;;
        *) return 2 ;;
    esac
}
# pl_wait_started NAME INITIAL_WAIT PROBES RETRY_WAIT:
# 0=confirmed RUNNING, 1=confirmed ABSENT after bounded probes,
# 2=UNKNOWN, invalid arguments or an interrupted wait.
pl_wait_started()
{
    pl_start_name="$1"
    pl_start_initial="$2"
    pl_start_attempts="$3"
    pl_start_retry="$4"
    [ -n "$pl_start_name" ] || return 2
    for pl_start_value in "$pl_start_initial" "$pl_start_attempts" "$pl_start_retry"; do
        case "$pl_start_value" in ''|*[!0-9]*) return 2 ;; esac
    done
    [ "$pl_start_attempts" -gt 0 ] || return 2
    if [ "$pl_start_initial" -gt 0 ]; then
        sleep "$pl_start_initial" || return 2
    fi
    pl_start_attempt=0
    while [ "$pl_start_attempt" -lt "$pl_start_attempts" ]; do
        if pl_process_state "$pl_start_name"; then pl_start_state=0; else pl_start_state=$?; fi
        case "$pl_start_state" in 0) return 0 ;; 1) ;; *) return 2 ;; esac
        pl_start_attempt=$((pl_start_attempt + 1))
        [ "$pl_start_attempt" -lt "$pl_start_attempts" ] || return 1
        if [ "$pl_start_retry" -gt 0 ]; then sleep "$pl_start_retry" || return 2; fi
    done
    return 1
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


# pl_process_pattern_state ERE: 0=RUNNING, 1=confirmed ABSENT, 2=UNKNOWN.
# Use pgrep -f for shell-script clients whose process comm is only "sh".
# Verify a pgrep status-1 result against a complete /proc inventory.
# Return 0 when a matching live command line exists, 1 only after a complete
# scan proves absence, and 2 when any part of the inventory is uncertain.
pl_process_pattern_procfs_state()
{
    pl_pattern_scan_pattern="$1"
    [ -n "$pl_pattern_scan_pattern" ] || return 2
    pl_pattern_scan_root=${PL_PROC_ROOT:-/proc}
    [ -d "$pl_pattern_scan_root" ] && [ -r "$pl_pattern_scan_root" ] || return 2
    command -v ls >/dev/null 2>&1 || return 2
    command -v awk >/dev/null 2>&1 || return 2
    command -v tr >/dev/null 2>&1 || return 2
    command -v grep >/dev/null 2>&1 || return 2

    pl_pattern_scan_entries=$(ls -1 "$pl_pattern_scan_root" 2>/dev/null) || return 2
    [ -n "$pl_pattern_scan_entries" ] || return 2

    while IFS= read -r pl_pattern_scan_pid; do
        case "$pl_pattern_scan_pid" in
            ''|*[!0-9]*) continue ;;
            0*) return 2 ;;
        esac
        pl_pattern_scan_dir="$pl_pattern_scan_root/$pl_pattern_scan_pid"
        [ -d "$pl_pattern_scan_dir" ] || continue

        pl_pattern_scan_task_state=$(awk -v expected_pid="$pl_pattern_scan_pid" '
            NR != 1 { invalid = 1 }
            NR == 1 {
                if (index($0, expected_pid " (") != 1)
                    invalid = 1
                sub(/^.*\) /, "")
                if (NF < 20 || $1 !~ /^[RSDZTtXxKWI]$/ ||
                    $20 !~ /^[0-9]+$/)
                    invalid = 1
                else
                    task_state = $1
            }
            END {
                if (invalid || NR != 1)
                    exit 1
                print task_state
            }
        ' "$pl_pattern_scan_dir/stat" 2>/dev/null) || {
            [ -d "$pl_pattern_scan_dir" ] || continue
            return 2
        }
        case "$pl_pattern_scan_task_state" in
            Z|X|x) continue ;;
        esac

        pl_pattern_scan_cmdline="$pl_pattern_scan_dir/cmdline"
        [ -f "$pl_pattern_scan_cmdline" ] &&
            [ -r "$pl_pattern_scan_cmdline" ] || {
                [ -d "$pl_pattern_scan_dir" ] || continue
                return 2
            }
        pl_pattern_scan_arguments=$(tr '\000\n' '  ' < "$pl_pattern_scan_cmdline" 2>/dev/null) || {
            [ -d "$pl_pattern_scan_dir" ] || continue
            return 2
        }
        if printf '%s\n' "$pl_pattern_scan_arguments" |
            grep -E -q -- "$pl_pattern_scan_pattern"; then
            return 0
        else
            pl_pattern_scan_grep_status=$?
            [ "$pl_pattern_scan_grep_status" -eq 1 ] || return 2
        fi
    done <<EOF
$pl_pattern_scan_entries
EOF

    return 1
}

pl_process_pattern_state()
{
    pl_pattern="$1"
    [ -n "$pl_pattern" ] || return 2
    command -v pgrep >/dev/null 2>&1 || return 2

    if pl_pattern_pids=$(pgrep -f "$pl_pattern" 2>/dev/null); then
        [ -n "$pl_pattern_pids" ] || return 2
        printf '%s\n' "$pl_pattern_pids" |
            awk -v self_pid="$$" '
                {
                    for (i = 1; i <= NF; i++) {
                        if ($i !~ /^[1-9][0-9]*$/ || $i == 1 || $i == self_pid)
                            invalid = 1
                        count++
                    }
                }
                END {
                    if (invalid || count == 0)
                        exit 1
                    exit 0
                }
            ' >/dev/null 2>&1 || return 2
        return 0
    else
        pl_pattern_status=$?
        [ "$pl_pattern_status" -eq 1 ] && [ -z "$pl_pattern_pids" ] || return 2
        if pl_process_pattern_procfs_state "$pl_pattern"; then
            return 0
        else
            pl_pattern_scan_status=$?
            [ "$pl_pattern_scan_status" -eq 1 ] || return 2
        fi
        # Close the start race: a package client may appear after the procfs
        # directory snapshot was taken but before the complete scan finished.
        if pl_pattern_pids=$(pgrep -f "$pl_pattern" 2>/dev/null); then
            [ -n "$pl_pattern_pids" ] || return 2
            printf '%s\n' "$pl_pattern_pids" |
                awk -v self_pid="$$" '
                    {
                        for (i = 1; i <= NF; i++) {
                            if ($i !~ /^[1-9][0-9]*$/ || $i == 1 || $i == self_pid)
                                invalid = 1
                            count++
                        }
                    }
                    END {
                        if (invalid || count == 0)
                            exit 1
                        exit 0
                    }
                ' >/dev/null 2>&1 || return 2
            return 0
        else
            pl_pattern_status=$?
            [ "$pl_pattern_status" -eq 1 ] && [ -z "$pl_pattern_pids" ] || return 2
            return 1
        fi
    fi
}
