#!/bin/sh

NAME=aliyundrive-webdav
ALD_BIN=/usr/bin/aliyundrive-webdav
ALD_STOP_WAIT_SECONDS=5
ALD_START_WAIT_SECONDS=10
ALD_LOCK_FILE=${ALD_LOCK_FILE:-/var/lock/aliyundrive-webdav.lock}

ald_wait_stopped() {
  ald_wait=0
  while [ "$ald_wait" -lt "$ALD_STOP_WAIT_SECONDS" ]; do
    if ! pidof "$NAME" >/dev/null 2>&1; then
      return 0
    fi
    sleep 1
    ald_wait=$((ald_wait + 1))
  done

  if pidof "$NAME" >/dev/null 2>&1; then
    return 1
  fi
  return 0
}


ald_wait_started() {
  ald_wait=0
  while [ "$ald_wait" -lt "$ALD_START_WAIT_SECONDS" ]; do
    if pidof "$NAME" >/dev/null 2>&1; then
      return 0
    fi
    sleep 1
    ald_wait=$((ald_wait + 1))
  done

  if pidof "$NAME" >/dev/null 2>&1; then
    return 0
  fi
  return 1
}

ald_run_locked() {
  case "$ALD_LOCK_FILE" in
    */*)
      ald_lock_dir=${ALD_LOCK_FILE%/*}
      [ -n "$ald_lock_dir" ] || ald_lock_dir=/
      ;;
    *)
      ald_lock_dir=.
      ;;
  esac
  if ! mkdir -p "$ald_lock_dir"; then
    logger -t "阿里云盘" "无法创建生命周期锁目录"
    return 1
  fi
  # Keep one stable inode: never unlink the lock file after releasing it.
  (umask 077; : >> "$ALD_LOCK_FILE") || {
    logger -t "阿里云盘" "无法打开生命周期锁文件"
    return 1
  }
  exec 9>>"$ALD_LOCK_FILE"
  if ! flock -n 9; then
    exec 9>&-
    logger -t "阿里云盘" "另一个生命周期操作正在执行"
    return 1
  fi

  "$@"
  ald_status=$?
  flock -u 9 >/dev/null 2>&1 || :
  exec 9>&-
  return "$ald_status"
}

ald_kill_unlocked() {
  NAME=aliyundrive-webdav
  if ! pidof "$NAME" >/dev/null 2>&1; then
    return 0
  fi

  logger -t "阿里云盘" "关闭进程..."
  killall "$NAME" >/dev/null 2>&1
  if ald_wait_stopped; then
    return 0
  fi

  logger -t "阿里云盘" "正常停止超时，尝试强制停止"
  # Re-enumerate by process name; never signal a cached numeric PID.
  killall -9 "$NAME" >/dev/null 2>&1
  if ald_wait_stopped; then
    return 0
  fi

  logger -t "阿里云盘" "进程仍在运行，保留运行状态"
  return 1
}

ald_start_unlocked() {
  logger -t "阿里云盘" "正在启动..."
  NAME=aliyundrive-webdav
  enable="$(nvram get aliyundrive_enable)"
  case "$enable" in
    1|on|true|yes|enabled)
      # Repeated RC/UI start notifications must not launch duplicate servers.
      if pidof "$NAME" >/dev/null 2>&1; then
        logger -t "阿里云盘" "服务已在运行，跳过重复启动"
        return 0
      fi

      refresh_token="$(nvram get ald_refresh_token)"
      auth_user="$(nvram get ald_auth_user)"
      auth_password="$(nvram get ald_auth_password)"
      read_buf_size="$(nvram get ald_read_buffer_size)"
      cache_size="$(nvram get ald_cache_size)"
      cache_ttl="$(nvram get ald_cache_ttl)"
      host="$(nvram get ald_host)"
      port="$(nvram get ald_port)"
      root="$(nvram get ald_root)"
      domain_id="$(nvram get ald_domain_id)"

      # Non-secret NVRAM values stay individually quoted in argv. The bundled
      # v1.2.1 CLI advertises the credential env bindings verified under MIPSel QEMU.
      set -- -I
      if [ "$domain_id" = "99999" ]; then
        set -- "$@" --domain-id "$domain_id"
      else
        case "$(nvram get ald_no_trash)" in
          1|on|true|yes|enabled)
            set -- "$@" --no-trash
            ;;
          *) ;;
        esac

        case "$(nvram get ald_read_only)" in
          1|on|true|yes|enabled)
            set -- "$@" --read-only
            ;;
          *) ;;
        esac
      fi

      REFRESH_TOKEN="$refresh_token" WEBDAV_AUTH_USER="$auth_user" WEBDAV_AUTH_PASSWORD="$auth_password" \
        "$ALD_BIN" "$@" --host "$host" --root "$root" -p "$port" \
        -S "$read_buf_size" --cache-size "$cache_size" \
        --cache-ttl "$cache_ttl" --workdir "/tmp/$NAME" >/dev/null 2>&1 9>&- &
      if ! ald_wait_started; then
        logger -t "阿里云盘" "启动后未能确认服务进程就绪"
        return 1
      fi
      ;;
    *)
      ald_kill_unlocked
      ;;
  esac
}

kill_ald() {
  ald_run_locked ald_kill_unlocked
}

start_ald() {
  ald_run_locked ald_start_unlocked
}

stop_ald() {
  kill_ald
}

case "$1" in
  start)
    start_ald
    ;;
  stop)
    stop_ald
    ;;
  *)
    echo "check"
    ;;
esac
