#!/bin/sh

IPERF3_BIN="${IPERF3_BIN:-/bin/iperf3}"
IPERF3_PROC_ROOT="${IPERF3_PROC_ROOT:-/proc}"
LOG_TAG="iperf3"

log_message() {
    logger -t "$LOG_TAG" "$1"
}

# Match only the server started by this wrapper. A comm/pidof match alone can
# include an unrelated executable or an iperf3 client; never signal either.
# Capture procfs start_time and revalidate it before each signal to reject PID
# reuse. Linux 4.4 has no pidfd, so a small validate-to-signal race remains.
iperf3_read_identity() {
    iperf3_identity_pid="$1"
    iperf3_identity_start="$2"
    iperf3_identity_dir="$IPERF3_PROC_ROOT/$iperf3_identity_pid"
    [ -d "$iperf3_identity_dir" ] || return 1

    iperf3_identity_summary=$(
        awk -v expected_pid="$iperf3_identity_pid" '
            NR != 1 { invalid = 1; next }
            NR == 1 {
                prefix = expected_pid " ("
                if (index($0, prefix) != 1) {
                    invalid = 1
                    next
                }
                tail = substr($0, length(prefix) + 1)
                close_pos = 0
                for (i = length(tail); i >= 1; i--) {
                    if (substr(tail, i, 2) == ") ") {
                        close_pos = i
                        break
                    }
                }
                if (!close_pos) {
                    invalid = 1
                    next
                }
                comm = substr(tail, 1, close_pos - 1)
                rest = substr(tail, close_pos + 2)
                count = split(rest, field, /[[:space:]]+/)
                if (count < 20 ||
                    field[1] !~ /^[RSDZTtXxKWPI]$/ ||
                    field[20] !~ /^[0-9]+$/) {
                    invalid = 1
                    next
                }
                if (comm != "iperf3" || field[1] ~ /^[ZXx]$/)
                    print "OTHER"
                else
                    print field[1], field[20]
            }
            END {
                if (invalid || NR != 1)
                    exit 1
            }
        ' "$iperf3_identity_dir/stat" 2>/dev/null
    ) || {
        [ -d "$iperf3_identity_dir" ] || return 1
        return 2
    }
    case "$iperf3_identity_summary" in
        OTHER) return 1 ;;
        '') return 2 ;;
    esac
    set -- $iperf3_identity_summary
    iperf3_identity_state=$1
    iperf3_identity_start_time=$2
    case "$iperf3_identity_start_time" in
        ''|*[!0-9]*) return 2 ;;
    esac
    if [ -n "$iperf3_identity_start" ] &&
       [ "$iperf3_identity_start_time" != "$iperf3_identity_start" ]; then
        return 1
    fi

    iperf3_identity_exe=$(readlink "$iperf3_identity_dir/exe" 2>/dev/null) || {
        [ -d "$iperf3_identity_dir" ] || return 1
        return 2
    }
    [ "$iperf3_identity_exe" = "$IPERF3_EXPECTED_EXE" ] || return 1

    iperf3_identity_cmdline=$(tr '\000' '\n' < "$iperf3_identity_dir/cmdline" 2>/dev/null) || {
        [ -d "$iperf3_identity_dir" ] || return 1
        return 2
    }
    [ -n "$iperf3_identity_cmdline" ] || return 2
    case "$iperf3_identity_cmdline" in
        *'
'*) iperf3_identity_args=${iperf3_identity_cmdline#*'
'} ;;
        *) return 1 ;;
    esac
    iperf3_identity_argv1=${iperf3_identity_args%%'
'*}
    [ "$iperf3_identity_argv1" = "-s" ] || return 1
    IPERF3_PROCESS_START_TIME=$iperf3_identity_start_time
    return 0
}

iperf3_scan_once() {
    IPERF3_MATCHES=
    IPERF3_MATCH_COUNT=0
    [ -d "$IPERF3_PROC_ROOT" ] &&
        [ -r "$IPERF3_PROC_ROOT" ] &&
        [ -x "$IPERF3_PROC_ROOT" ] || return 2
    command -v ls >/dev/null 2>&1 || return 2
    command -v awk >/dev/null 2>&1 || return 2
    command -v readlink >/dev/null 2>&1 || return 2
    command -v tr >/dev/null 2>&1 || return 2
    command -v busybox >/dev/null 2>&1 || return 2

    IPERF3_EXPECTED_EXE=$(readlink -f "$IPERF3_BIN" 2>/dev/null) || return 2
    [ -n "$IPERF3_EXPECTED_EXE" ] || return 2
    iperf3_scan_entries=$(ls -1 "$IPERF3_PROC_ROOT" 2>/dev/null) || return 2
    [ -n "$iperf3_scan_entries" ] || return 2
    iperf3_scan_numeric_found=0

    while IFS= read -r iperf3_scan_pid; do
        case "$iperf3_scan_pid" in
            ''|*[!0-9]*) continue ;;
            0*) return 2 ;;
        esac
        iperf3_scan_dir="$IPERF3_PROC_ROOT/$iperf3_scan_pid"
        [ -d "$iperf3_scan_dir" ] || continue
        iperf3_scan_numeric_found=1
        if iperf3_read_identity "$iperf3_scan_pid" ""; then
            if [ -n "$IPERF3_MATCHES" ]; then
                IPERF3_MATCHES="$IPERF3_MATCHES
$iperf3_scan_pid $IPERF3_PROCESS_START_TIME"
            else
                IPERF3_MATCHES="$iperf3_scan_pid $IPERF3_PROCESS_START_TIME"
            fi
            IPERF3_MATCH_COUNT=$((IPERF3_MATCH_COUNT + 1))
        else
            iperf3_scan_status=$?
            case "$iperf3_scan_status" in
                1) ;;
                *) return 2 ;;
            esac
        fi
    done <<EOF_IPERF3_PIDS
$iperf3_scan_entries
EOF_IPERF3_PIDS

    [ "$iperf3_scan_numeric_found" -eq 1 ] || return 2
    [ "$IPERF3_MATCH_COUNT" -gt 0 ] && return 0
    return 1
}

iperf3_process_state() {
    if iperf3_scan_once; then
        return 0
    else
        iperf3_state_status=$?
    fi
    case "$iperf3_state_status" in
        1)
            # A second complete inventory closes the common race where a
            # service appears after the first procfs directory snapshot.
            if iperf3_scan_once; then
                return 0
            else
                iperf3_state_status=$?
            fi
            [ "$iperf3_state_status" -eq 1 ] && return 1
            return 2
            ;;
        *) return 2 ;;
    esac
}

is_running() {
    iperf3_process_state
}

check_start_tools() {
    command -v start-stop-daemon >/dev/null 2>&1 || {
        log_message "start-stop-daemon is unavailable"
        return 127
    }
    return 0
}

func_start() {
    if [ ! -x "$IPERF3_BIN" ]; then
        log_message "server binary is missing or not executable: $IPERF3_BIN"
        return 127
    fi
    if check_start_tools; then
        :
    else
        tool_status=$?
        return "$tool_status"
    fi

    if is_running; then
        return 0
    else
        process_status=$?
    fi
    case "$process_status" in
        1) ;;
        *)
            log_message "Cannot verify existing server state (status=$process_status); refusing start"
            return 2
            ;;
    esac

    if start-stop-daemon -S -b -x "$IPERF3_BIN" -- -s; then
        :
    else
        launch_status=$?
        log_message "server launcher failed (status=$launch_status)"
        return "$launch_status"
    fi

    attempt=0
    while :; do
        if is_running; then
            log_message "server started"
            return 0
        else
            process_status=$?
        fi
        case "$process_status" in
            1) ;;
            *)
                log_message "Cannot verify startup process state (status=$process_status)"
                return 2
                ;;
        esac
        if [ "$attempt" -ge 3 ]; then
            log_message "launcher returned success but no server process appeared"
            return 1
        fi
        attempt=$((attempt + 1))
        sleep 1
    done
}

# Revalidate executable/argv identity and procfs start_time immediately before
# each PID-scoped signal through the firmware BusyBox kill applet.
iperf3_signal_one() {
    iperf3_signal_pid="$1"
    iperf3_signal_start="$2"
    iperf3_signal_name="$3"
    if iperf3_read_identity "$iperf3_signal_pid" "$iperf3_signal_start"; then
        :
    else
        iperf3_signal_identity_status=$?
        return "$iperf3_signal_identity_status"
    fi
    busybox kill "$iperf3_signal_name" "$iperf3_signal_pid" || {
        IPERF3_SIGNAL_STATUS=$?
        return 3
    }
    return 0
}

iperf3_signal_matches() {
    iperf3_signal_name="$1"
    IPERF3_SIGNAL_FAILURE=0
    while IFS=' ' read -r iperf3_signal_pid iperf3_signal_start; do
        [ -n "$iperf3_signal_pid" ] || continue
        if iperf3_signal_one "$iperf3_signal_pid" "$iperf3_signal_start" "$iperf3_signal_name"; then
            :
        else
            iperf3_signal_result=$?
            case "$iperf3_signal_result" in
                1) ;; # The process exited or its PID was reused.
                2) return 2 ;;
                3) IPERF3_SIGNAL_FAILURE=1 ;;
                *) return 2 ;;
            esac
        fi
    done <<EOF_IPERF3_MATCHES
$IPERF3_MATCHES
EOF_IPERF3_MATCHES
    return 0
}

func_stop() {
    if is_running; then
        process_status=0
    else
        process_status=$?
    fi
    case "$process_status" in
        1) return 0 ;;
        0) ;;
        *)
            log_message "Cannot verify server state (status=$process_status); refusing stop"
            return 2
            ;;
    esac

    iperf3_signal_matches -TERM || {
        signal_status=$?
        log_message "Cannot safely signal server (status=$signal_status)"
        return 2
    }

    attempt=0
    while :; do
        if is_running; then
            process_status=0
        else
            process_status=$?
        fi
        case "$process_status" in
            1) return 0 ;;
            0) ;;
            *)
                log_message "Cannot verify server stop postcondition (status=$process_status)"
                return 2
                ;;
        esac
        if [ "$attempt" -ge 5 ]; then
            break
        fi
        attempt=$((attempt + 1))
        sleep 1 || return 2
    done

    # Re-scan and escalate only against the same executable, server argv and
    # start_time tuple. Never fall back to process-name-wide killall.
    if iperf3_process_state; then
        :
    else
        process_status=$?
        case "$process_status" in
            1) return 0 ;;
            *) return 2 ;;
        esac
    fi
    iperf3_signal_matches -KILL || {
        signal_status=$?
        log_message "Cannot safely escalate server stop (status=$signal_status)"
        return 2
    }

    attempt=0
    while :; do
        if is_running; then
            process_status=0
        else
            process_status=$?
        fi
        case "$process_status" in
            1) return 0 ;;
            0) ;;
            *)
                log_message "Cannot verify server after force-stop (status=$process_status)"
                return 2
                ;;
        esac
        if [ "$attempt" -ge 5 ]; then
            log_message "server is still running after the bounded force-stop wait"
            return 1
        fi
        attempt=$((attempt + 1))
        sleep 1 || return 2
    done
}

case "$1" in
    start)
        func_start
        ;;
    stop)
        func_stop
        ;;
    restart)
        if func_stop; then
            func_start
        else
            stop_status=$?
            log_message "restart aborted because stop failed (status=$stop_status)"
            exit "$stop_status"
        fi
        ;;
    *)
        echo "Usage: $0 { start | stop | restart }"
        exit 2
        ;;
esac
