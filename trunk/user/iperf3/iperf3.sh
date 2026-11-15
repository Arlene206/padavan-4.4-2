#!/bin/sh

IPERF3_BIN="${IPERF3_BIN:-/bin/iperf3}"
LOG_TAG="iperf3"

log_message() {
    logger -t "$LOG_TAG" "$1"
}

is_running() {
    pidof iperf3 >/dev/null 2>&1
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
    fi

    if start-stop-daemon -S -b -x "$IPERF3_BIN" -- -s; then
        :
    else
        launch_status=$?
        log_message "server launcher failed (status=$launch_status)"
        return "$launch_status"
    fi

    attempt=0
    while ! is_running; do
        if [ "$attempt" -ge 3 ]; then
            log_message "launcher returned success but no server process appeared"
            return 1
        fi
        attempt=$((attempt + 1))
        sleep 1
    done

    log_message "server started"
    return 0
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

    if ! is_running; then
        return 0
    fi

    if killall -q iperf3; then
        :
    else
        signal_status=$?
        if is_running; then
            log_message "server stop signal failed (status=$signal_status)"
            return "$signal_status"
        fi
    fi

    attempt=0
    while is_running; do
        if [ "$attempt" -ge 5 ]; then
            log_message "server is still running after the bounded stop wait"
            return 1
        fi
        attempt=$((attempt + 1))
        sleep 1
    done
    return 0
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
