#!/bin/sh

IPERF3_BIN="${IPERF3_BIN:-/bin/iperf3}"
IPERF3_PROCESS_HELPER="${IPERF3_PROCESS_HELPER:-/usr/lib/process_lifecycle.sh}"
PL_PROC_ROOT="${IPERF3_PROC_ROOT:-/proc}"
LOG_TAG="iperf3"
export PL_PROC_ROOT

log_message() {
    logger -t "$LOG_TAG" "$1"
}

# The shared tri-state helper confirms pidof's ambiguous status-1 result by
# scanning and validating the complete procfs inventory, including a
# post-scan pidof check. Missing helper or incomplete procfs is UNKNOWN.
if [ ! -r "$IPERF3_PROCESS_HELPER" ]; then
    log_message "shared process lifecycle helper is unavailable: $IPERF3_PROCESS_HELPER"
    exit 2
fi
. "$IPERF3_PROCESS_HELPER" || {
    log_message "shared process lifecycle helper could not be loaded"
    exit 2
}

iperf3_process_state() {
    pl_process_state iperf3
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
