#!/bin/sh

NAME=aliyundrive-webdav
ALD_BIN=/usr/bin/aliyundrive-webdav
ALD_WORK_DIR="/tmp/$NAME"
ALD_STOP_WAIT_SECONDS=5
ALD_START_WAIT_SECONDS=10
# dev_init provisions this owner-only lock in sticky /tmp before starting mdev.
ALD_LOCK_FILE=${ALD_LOCK_FILE:-/tmp/aliyundrive-webdav.lock}

ald_process_state() {
  # pidof has three outcomes: RUNNING, confirmed ABSENT, or UNKNOWN.
  if ald_pid_output=$(pidof "$NAME" 2>/dev/null); then
    ald_pid_status=0
  else
    ald_pid_status=$?
  fi
  case "$ald_pid_status" in
    0)
      [ -n "$ald_pid_output" ] || return 2
      case "$ald_pid_output" in
        *[!0123456789\ ]*) return 2 ;;
      esac
      ald_pid_count=0
      for ald_pid in $ald_pid_output; do
        case "$ald_pid" in
          ''|0*|1|*[!0-9]*) return 2 ;;
        esac
        [ "${#ald_pid}" -le 10 ] || return 2
        [ "$ald_pid" -gt 1 ] 2>/dev/null || return 2
        ald_pid_count=$((ald_pid_count + 1))
      done
      [ "$ald_pid_count" -gt 0 ] || return 2
      return 0
      ;;
    1)
      [ -z "$ald_pid_output" ] && return 1
      return 2
      ;;
    *) return 2 ;;
  esac
}

ald_wait_stopped() {
  ald_wait=0
  while [ "$ald_wait" -lt "$ALD_STOP_WAIT_SECONDS" ]; do
    if ald_process_state; then
      ald_state=0
    else
      ald_state=$?
    fi
    case "$ald_state" in
      1) return 0 ;;
      0) ;;
      *) return 2 ;;
    esac
    sleep 1 || return 2
    ald_wait=$((ald_wait + 1))
  done

  if ald_process_state; then
    return 1
  else
    ald_state=$?
  fi
  case "$ald_state" in
    1) return 0 ;;
    0) return 1 ;;
    *) return 2 ;;
  esac
}


ald_nvram_get() {
  # Missing keys are valid empty values; backend read failures are not.
  if ald_value=$(nvram get_checked "$1"); then
    printf '%s' "$ald_value"
    return 0
  else
    ald_status=$?
  fi
  logger -t "阿里云盘" "NVRAM key read failed: $1 (status=$ald_status)"
  return 1
}

ald_http_ready() {
  ald_http_status_line=$(
    printf 'HEAD / HTTP/1.0\r\nHost: localhost\r\nConnection: close\r\n\r\n' |
      nc -w 1 "$1" "$2" 2>/dev/null | head -n 1
  ) || return 1
  ald_http_protocol=${ald_http_status_line%% *}
  ald_http_status=${ald_http_status_line#* }
  ald_http_status=${ald_http_status%% *}
  case "$ald_http_protocol" in
    HTTP/1.0|HTTP/1.1) ;;
    *) return 1 ;;
  esac
  case "$ald_http_status" in
    2[0-9][0-9]|3[0-9][0-9]|401|403|404|405) return 0 ;;
    *) return 1 ;;
  esac
}

ald_wait_started() {
  ald_wait=0
  while [ "$ald_wait" -lt "$ALD_START_WAIT_SECONDS" ]; do
    if ald_process_state; then
      ald_state=0
    else
      ald_state=$?
    fi
    case "$ald_state" in
      0)
        if ald_http_ready "$host" "$port"; then
          return 0
        fi
        ;;
      1) ;;
      *) return 2 ;;
    esac
    sleep 1 || return 2
    ald_wait=$((ald_wait + 1))
  done

  if ald_process_state; then
    if ald_http_ready "$host" "$port"; then
      return 0
    fi
    return 1
  else
    ald_state=$?
  fi
  case "$ald_state" in
    1) return 1 ;;
    *) return 2 ;;
  esac
}

ald_run_locked() {
  case "$ALD_LOCK_FILE" in
    /*)
      ald_lock_dir=${ALD_LOCK_FILE%/*}
      [ -n "$ald_lock_dir" ] || ald_lock_dir=/
      ;;
    *)
      logger -t "阿里云盘" "生命周期锁路径必须是绝对路径"
      return 1
      ;;
  esac

  # /tmp is root-owned and sticky; private test/work directories must not
  # be group/world writable unless sticky. Refuse lock-path substitution.
  if [ -L "$ald_lock_dir" ] || [ ! -d "$ald_lock_dir" ] ||
     [ -L "$ALD_LOCK_FILE" ] || [ ! -f "$ALD_LOCK_FILE" ]; then
    logger -t "阿里云盘" "生命周期锁或其目录不是可信普通路径"
    return 1
  fi
  ald_lock_uid=$(id -u 2>/dev/null) || {
    logger -t "阿里云盘" "无法确认生命周期锁所有者"
    return 1
  }
  ald_lock_dir_owner=$(stat -c %u "$ald_lock_dir" 2>/dev/null) || {
    logger -t "阿里云盘" "无法确认生命周期锁目录所有者"
    return 1
  }
  ald_lock_dir_mode=$(stat -c %a "$ald_lock_dir" 2>/dev/null) || {
    logger -t "阿里云盘" "无法确认生命周期锁目录权限"
    return 1
  }
  case "$ald_lock_dir_mode" in
    ''|*[!01234567]*)
      logger -t "阿里云盘" "生命周期锁目录权限无效"
      return 1
      ;;
  esac
  ald_lock_dir_mode_value=$((0$ald_lock_dir_mode))
  ald_lock_dir_write_bits=$((ald_lock_dir_mode_value & 0022))
  ald_lock_dir_sticky=$((ald_lock_dir_mode_value & 01000))
  if [ "$ald_lock_dir_owner" != "$ald_lock_uid" ] ||
     { [ "$ald_lock_dir_write_bits" -ne 0 ] && [ "$ald_lock_dir_sticky" -eq 0 ]; }; then
    logger -t "阿里云盘" "生命周期锁目录可被其他用户替换"
    return 1
  fi

  ald_lock_owner=$(stat -c %u "$ALD_LOCK_FILE" 2>/dev/null) || {
    logger -t "阿里云盘" "无法确认生命周期锁文件所有者"
    return 1
  }
  ald_lock_links=$(stat -c %h "$ALD_LOCK_FILE" 2>/dev/null) || {
    logger -t "阿里云盘" "无法确认生命周期锁文件链接数"
    return 1
  }
  ald_lock_mode=$(stat -c %a "$ALD_LOCK_FILE" 2>/dev/null) || {
    logger -t "阿里云盘" "无法确认生命周期锁文件权限"
    return 1
  }
  if [ "$ald_lock_owner" != "$ald_lock_uid" ] ||
     [ "$ald_lock_links" != 1 ] || [ "$ald_lock_mode" != 600 ]; then
    logger -t "阿里云盘" "生命周期锁必须是当前用户独占的 0600 普通文件"
    return 1
  fi

  # dev_init creates this inode atomically in sticky /tmp; never create,
  # follow or replace a lock path during a service transition.
  if ! exec 9>>"$ALD_LOCK_FILE"; then
    logger -t "阿里云盘" "无法打开生命周期锁文件"
    return 1
  fi
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

ald_secure_workdir() {
  ald_workdir_owner="$(id -u 2>/dev/null)" || {
    logger -t "阿里云盘" "无法确认工作目录所有者"
    return 1
  }
  if [ -L "$ALD_WORK_DIR" ]; then
    logger -t "阿里云盘" "工作目录不能是符号链接"
    return 1
  fi
  if [ ! -e "$ALD_WORK_DIR" ]; then
    (umask 077; mkdir "$ALD_WORK_DIR") || {
      logger -t "阿里云盘" "无法安全创建工作目录"
      return 1
    }
  fi
  if [ -L "$ALD_WORK_DIR" ] || [ ! -d "$ALD_WORK_DIR" ]; then
    logger -t "阿里云盘" "工作路径不是可信目录"
    return 1
  fi
  ald_workdir_owner_on_disk="$(stat -c %u "$ALD_WORK_DIR" 2>/dev/null)" || {
    logger -t "阿里云盘" "无法读取工作目录所有者"
    return 1
  }
  if [ "$ald_workdir_owner_on_disk" != "$ald_workdir_owner" ]; then
    logger -t "阿里云盘" "工作目录所有者不匹配"
    return 1
  fi
  chmod 700 "$ALD_WORK_DIR" || {
    logger -t "阿里云盘" "无法限制工作目录权限"
    return 1
  }
  ald_token_file="$ALD_WORK_DIR/refresh_token"
  if [ -L "$ald_token_file" ]; then
    logger -t "阿里云盘" "刷新令牌文件不能是符号链接"
    return 1
  fi
  if [ -e "$ald_token_file" ]; then
    if [ ! -f "$ald_token_file" ]; then
      logger -t "阿里云盘" "刷新令牌路径不是普通文件"
      return 1
    fi
    ald_token_owner_on_disk="$(stat -c %u "$ald_token_file" 2>/dev/null)" || {
      logger -t "阿里云盘" "无法读取刷新令牌文件所有者"
      return 1
    }
    if [ "$ald_token_owner_on_disk" != "$ald_workdir_owner" ]; then
      logger -t "阿里云盘" "刷新令牌文件所有者不匹配"
      return 1
    fi
    ald_token_links="$(stat -c %h "$ald_token_file" 2>/dev/null)" || {
      logger -t "阿里云盘" "无法读取刷新令牌文件链接数"
      return 1
    }
    if [ "$ald_token_links" != 1 ]; then
      logger -t "阿里云盘" "刷新令牌文件存在额外硬链接"
      return 1
    fi
    chmod 600 "$ald_token_file" || {
      logger -t "阿里云盘" "无法限制刷新令牌文件权限"
      return 1
    }
  fi
  return 0
}


ald_kill_unlocked() {
  NAME=aliyundrive-webdav
  if ald_process_state; then
    ald_state=0
  else
    ald_state=$?
  fi
  case "$ald_state" in
    1) return 0 ;;
    0) ;;
    *)
      logger -t "阿里云盘" "无法确认进程状态，拒绝发送信号"
      return 1
      ;;
  esac

  logger -t "阿里云盘" "关闭进程..."
  killall "$NAME" >/dev/null 2>&1 || :
  if ald_wait_stopped; then
    return 0
  else
    ald_wait_status=$?
  fi
  case "$ald_wait_status" in
    1) ;;
    *)
      logger -t "阿里云盘" "TERM后进程状态未知，拒绝升级信号"
      return 1
      ;;
  esac

  # A fresh validated name lookup is required before KILL escalation.
  if ald_process_state; then
    ald_state=0
  else
    ald_state=$?
  fi
  case "$ald_state" in
    1) return 0 ;;
    0) ;;
    *)
      logger -t "阿里云盘" "强制停止前进程状态未知，拒绝发送信号"
      return 1
      ;;
  esac

  logger -t "阿里云盘" "正常停止超时，尝试强制停止"
  killall -9 "$NAME" >/dev/null 2>&1 || :
  if ald_wait_stopped; then
    return 0
  fi

  logger -t "阿里云盘" "进程仍在运行或状态未知，保留运行状态"
  return 1
}

ald_start_unlocked() {
  logger -t "阿里云盘" "正在启动..."
  NAME=aliyundrive-webdav
  if enable="$(ald_nvram_get aliyundrive_enable)"; then :; else return 1; fi
  case "$enable" in
    1|on|true|yes|enabled)
      if host="$(ald_nvram_get ald_host)"; then :; else return 1; fi
      if port="$(ald_nvram_get ald_port)"; then :; else return 1; fi

      # A live PID is not service readiness: require an HTTP response as well.
      if ald_process_state; then
        if ald_wait_started; then
          logger -t "阿里云盘" "HTTP 服务已就绪，跳过重复启动"
          return 0
        else
          logger -t "阿里云盘" "服务进程存在，但 HTTP 端点未就绪"
          return 1
        fi
      else
        ald_state=$?
      fi
      if [ "$ald_state" -ne 1 ]; then
        logger -t "阿里云盘" "无法确认进程状态，拒绝启动重复实例"
        return 1
      fi

      if refresh_token="$(ald_nvram_get ald_refresh_token)"; then :; else return 1; fi
      if auth_user="$(ald_nvram_get ald_auth_user)"; then :; else return 1; fi
      if auth_password="$(ald_nvram_get ald_auth_password)"; then :; else return 1; fi
      if read_buf_size="$(ald_nvram_get ald_read_buffer_size)"; then :; else return 1; fi
      if cache_size="$(ald_nvram_get ald_cache_size)"; then :; else return 1; fi
      if cache_ttl="$(ald_nvram_get ald_cache_ttl)"; then :; else return 1; fi
      if root="$(ald_nvram_get ald_root)"; then :; else return 1; fi
      if domain_id="$(ald_nvram_get ald_domain_id)"; then :; else return 1; fi
      if [ "$domain_id" != "99999" ]; then
        if no_trash="$(ald_nvram_get ald_no_trash)"; then :; else return 1; fi
        if read_only="$(ald_nvram_get ald_read_only)"; then :; else return 1; fi
      fi

      # Protect the upstream daemon workdir before it reads or writes persisted tokens.
      ald_secure_workdir || return 1

      # Non-secret NVRAM values stay individually quoted in argv. The bundled
      # v1.2.1 CLI advertises the credential env bindings verified under MIPSel QEMU.
      set -- -I
      if [ "$domain_id" = "99999" ]; then
        set -- "$@" --domain-id "$domain_id"
      else
        case "$no_trash" in
          1|on|true|yes|enabled)
            set -- "$@" --no-trash
            ;;
          *) ;;
        esac

        case "$read_only" in
          1|on|true|yes|enabled)
            set -- "$@" --read-only
            ;;
          *) ;;
        esac
      fi

      (
        umask 077
        exec 9>&-
        RUST_LOG=aliyundrive_webdav=warn \
        REFRESH_TOKEN="$refresh_token" \
        WEBDAV_AUTH_USER="$auth_user" \
        WEBDAV_AUTH_PASSWORD="$auth_password" \
          "$ALD_BIN" "$@" --host "$host" --root "$root" -p "$port" \
          -S "$read_buf_size" --cache-size "$cache_size" \
          --cache-ttl "$cache_ttl" --workdir "$ALD_WORK_DIR" >/dev/null 2>&1 9>&-
      ) &
      if ald_wait_started; then
        :
      else
        ald_start_status=$?
        if [ "$ald_start_status" -eq 2 ]; then
          logger -t "阿里云盘" "启动后进程状态未知"
        else
          logger -t "阿里云盘" "启动后未能确认服务进程就绪"
        fi
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
