#!/bin/sh

NAME=aliyundrive-webdav
ALD_BIN=/usr/bin/aliyundrive-webdav
ALD_STOP_WAIT_SECONDS=5

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

kill_ald() {
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

start_ald() {
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

      # Keep each NVRAM value as exactly one argv item. Do not let spaces,
      # wildcard characters or shell metacharacters reshape the daemon argv.
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

      "$ALD_BIN" "$@" --host "$host" --root "$root" -p "$port" \
        -r "$refresh_token" -U "$auth_user" -W "$auth_password" \
        -S "$read_buf_size" --cache-size "$cache_size" \
        --cache-ttl "$cache_ttl" --workdir "/tmp/$NAME" >/dev/null 2>&1 &
      ;;
    *)
      kill_ald
      ;;
  esac
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
