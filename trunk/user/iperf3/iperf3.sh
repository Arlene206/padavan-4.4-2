#!/bin/sh

IPERF3_BIN="${IPERF3_BIN:-/bin/iperf3}"
LOG_TAG="iperf3"

log_message() {
    logger -t "$LOG_TAG" "$1"
}

# Return 0=RUNNING, 1=confirmed ABSENT, 2=UNKNOWN. BusyBox pidof can
# return 1 for both an empty result and a failed /proc scan, so validate the
# status/output pair before using process state to authorize start or stop.
iperf3_process_state() {
    if iperf3_pid_output=$(pidof iperf3 2>/dev/null); then
        iperf3_pidof_status=0
    else
        iperf3_pidof_status=$?
    fi
    case "$iperf3_pidof_status" in
        0)
            [ -n "$iperf3_pid_output" ] || return 2
            case "$iperf3_pid_output" in
                *[!0123456789\ ]*) return 2 ;;
            esac
            case $- in
                *f*) iperf3_restore_noglob=0 ;;
                *) set -f; iperf3_restore_noglob=1 ;;
            esac
            set -- $iperf3_pid_output
            [ "$iperf3_restore_noglob" -eq 0 ] || set +f
            [ "$#" -gt 0 ] || return 2
            for iperf3_pid in "$@"; do
                case "$iperf3_pid" in
                    ''|0*|*[!0-9]*) return 2 ;;
                esac
            done
            return 0
            ;;
        1)
            [ -z "$iperf3_pid_output" ] && return 1
            return 2
            ;;
        *)
            return 2
            ;;
    esac
}

is_running() {
    iperf3_process_state
}

check_start_tools() {
    command -v pidof >/dev/null 2>&1 || {
        log_message "pidof is unavailable; refusing an unverified start"
        return 127
    }
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

func_stop() {
    command -v pidof >/dev/null 2>&1 || {
        log_message "pidof is unavailable; refusing an unverified stop"
        return 127
    }
    command -v killall >/dev/null 2>&1 || {
        log_message "killall is unavailable"
        return 127
    }

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

    if killall -q iperf3; then
        :
    else
        signal_status=$?
        if is_running; then
            log_message "server stop signal failed (status=$signal_status)"
            return "$signal_status"
        else
            process_status=$?
        fi
        case "$process_status" in
            1) return 0 ;;
            *)
                log_message "Cannot verify server after failed stop signal (status=$process_status)"
                return 2
                ;;
        esac
    fi

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
            log_message "server is still running after the bounded stop wait"
            return 1
        fi
        attempt=$((attempt + 1))
        sleep 1
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
