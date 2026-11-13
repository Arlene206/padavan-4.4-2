#!/bin/sh

dns_forwarder_lifecycle_lock_path() {
  echo /tmp/.dns-forwarder.lifecycle.lock
}

# Keep the lock inode stable: unlinking it while another process holds flock(2)
# would allow a concurrent caller to lock a different inode.
dns_forwarder_lifecycle_lock_prepare() {
  lock_file="$1"
  [ -n "$lock_file" ] || return 1

  case "$lock_file" in
    */*) lock_dir="${lock_file%/*}" ;;
    *) lock_dir=. ;;
  esac
  [ -n "$lock_dir" ] || lock_dir=/

  if [ -L "$lock_dir" ] || [ ! -d "$lock_dir" ]; then
    logger -t "DNS Forwarder" "lifecycle lock directory is not a real directory"
    return 1
  fi

  lock_uid="$(id -u)" || return 1
  dir_info="$(stat -c '%u %a' "$lock_dir")" || return 1
  set -- $dir_info
  if [ "$#" -ne 2 ] || [ "$1" != "$lock_uid" ]; then
    logger -t "DNS Forwarder" "lifecycle lock directory has an unexpected owner"
    return 1
  fi
  dir_mode=$((0$2))
  if [ $((dir_mode & 0022)) -ne 0 ] &&
     [ $((dir_mode & 01000)) -eq 0 ]; then
    logger -t "DNS Forwarder" "lifecycle lock directory is writable without sticky protection"
    return 1
  fi

  if [ -L "$lock_file" ]; then
    logger -t "DNS Forwarder" "lifecycle lock file must not be a symlink"
    return 1
  fi
  if [ ! -e "$lock_file" ]; then
    (umask 077; set -C; : > "$lock_file") 2>/dev/null || :
  fi
  if [ -L "$lock_file" ] || [ ! -f "$lock_file" ]; then
    logger -t "DNS Forwarder" "lifecycle lock path is not a regular file"
    return 1
  fi

  file_info="$(stat -c '%u %a %h' "$lock_file")" || return 1
  set -- $file_info
  if [ "$#" -ne 3 ] || [ "$1" != "$lock_uid" ] ||
     [ "$2" != 600 ] || [ "$3" != 1 ]; then
    logger -t "DNS Forwarder" "lifecycle lock file owner, mode or link count is unsafe"
    return 1
  fi
  return 0
}

# FD 9 remains open for the operation, but must never leak into the daemon.
dns_forwarder_lifecycle_lock_execute() {
  lock_file="$1"
  wait_seconds="$2"
  shift 2
  case "$wait_seconds" in
    ''|*[!0-9]*) return 1 ;;
  esac
  [ "$#" -gt 0 ] || return 1

  if ( : <&9 ) 2>/dev/null || ( : >&9 ) 2>/dev/null; then
    logger -t "DNS Forwarder" "reserved lifecycle lock FD 9 is already occupied"
    return 1
  fi

  old_umask="$(umask)"
  umask 077
  if ! exec 9>>"$lock_file"; then
    umask "$old_umask"
    logger -t "DNS Forwarder" "cannot open lifecycle lock file"
    return 1
  fi
  umask "$old_umask"

  attempt=0
  while ! flock -n 9; do
    if [ "$attempt" -ge "$wait_seconds" ]; then
      exec 9>&-
      logger -t "DNS Forwarder" "timed out waiting for lifecycle lock"
      return 1
    fi
    attempt=$((attempt + 1))
    sleep 1
  done

  if "$@"; then
    action_status=0
  else
    action_status=$?
  fi
  exec 9>&-
  return "$action_status"
}

dns_forwarder_is_running() {
  pidof dns-forwarder >/dev/null 2>&1
}

func_stop() {
  if ! dns_forwarder_is_running; then
    return 0
  fi

  # A failed signal is harmless only if the process disappeared concurrently.
  if ! killall -q dns-forwarder && dns_forwarder_is_running; then
    logger -t "DNS Forwarder" "failed to signal the existing daemon"
  fi

  attempt=0
  while dns_forwarder_is_running; do
    if [ "$attempt" -ge 5 ]; then
      logger -t "DNS Forwarder" "daemon is still running after the bounded stop wait"
      return 1
    fi
    attempt=$((attempt + 1))
    sleep 1
  done
  return 0
}

func_start() {
  enabled="$(nvram get dns_forwarder_enable)" || return 1
  [ "$enabled" = "1" ] || return 0

  bind_address="$(nvram get dns_forwarder_bind)" || return 1
  bind_port="$(nvram get dns_forwarder_port)" || return 1
  server="$(nvram get dns_forwarder_server)" || return 1

  # Make start idempotent and protect against duplicate/stale RC requests.
  if dns_forwarder_is_running; then
    return 0
  fi

  if start-stop-daemon -S -b -x /usr/bin/dns-forwarder -- \
      -b "$bind_address" -p "$bind_port" -s "$server" 9>&-; then
    :
  else
    launch_status=$?
    logger -t "DNS Forwarder" "daemon launcher failed (status=$launch_status)"
    return "$launch_status"
  fi

  attempt=0
  while ! dns_forwarder_is_running; do
    if [ "$attempt" -ge 3 ]; then
      logger -t "DNS Forwarder" "launcher returned success but no daemon appeared"
      return 1
    fi
    attempt=$((attempt + 1))
    sleep 1
  done
  return 0
}

dns_forwarder_locked_action() {
  case "$1" in
    start)
      func_start
      ;;
    stop)
      func_stop
      ;;
    restart)
      # Never launch a replacement over a process which failed to stop.
      if ! func_stop; then
        logger -t "DNS Forwarder" "restart aborted because stop did not complete"
        return 1
      fi
      func_start
      ;;
    *)
      return 2
      ;;
  esac
}

case "$1" in
  start|stop|restart) action="$1" ;;
  *)
    echo "Usage: $0 { start | stop | restart }"
    exit 1
    ;;
esac

lock_file="$(dns_forwarder_lifecycle_lock_path)" || exit 1
if ! dns_forwarder_lifecycle_lock_prepare "$lock_file"; then
  logger -t "DNS Forwarder" "cannot safely prepare lifecycle lock; refusing service operation"
  exit 1
fi
command -v flock >/dev/null 2>&1 || {
  logger -t "DNS Forwarder" "flock is unavailable; refusing an unlocked service operation"
  exit 1
}
command -v pidof >/dev/null 2>&1 || {
  logger -t "DNS Forwarder" "pidof is unavailable; refusing an unverified service operation"
  exit 1
}

dns_forwarder_lifecycle_lock_execute "$lock_file" 10 \
  dns_forwarder_locked_action "$action"
