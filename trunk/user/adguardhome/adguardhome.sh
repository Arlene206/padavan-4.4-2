#!/bin/sh

ADG_START_WAIT_ATTEMPTS=5
ADG_PROC_ROOT='/proc'
ADG_EXECUTABLE='/usr/bin/AdGuardHome'

adg_lifecycle_lock_path() {
  echo /tmp/.adguardhome.lifecycle.lock
}

# Keep the lock inode stable. A stale file is harmless: flock(2) releases the
# kernel lock when the last descriptor closes, so never unlink this file.
adg_lifecycle_lock_prepare() {
  adg_lock_file="$1"
  [ -n "$adg_lock_file" ] || return 1

  case "$adg_lock_file" in
    */*) adg_lock_dir="${adg_lock_file%/*}" ;;
    *) adg_lock_dir=. ;;
  esac
  [ -n "$adg_lock_dir" ] || adg_lock_dir=/
  if [ -L "$adg_lock_dir" ] || [ ! -d "$adg_lock_dir" ]; then
    logger -t "AdGuardHome" "生命周期锁目录不是普通目录"
    return 1
  fi

  adg_lock_uid="$(id -u)" || return 1
  adg_dir_info="$(stat -c '%u %a' "$adg_lock_dir")" || return 1
  set -- $adg_dir_info
  if [ "$#" -ne 2 ] || [ "$1" != "$adg_lock_uid" ]; then
    logger -t "AdGuardHome" "生命周期锁目录所有者不安全"
    return 1
  fi
  adg_dir_mode=$((0$2))
  if [ $((adg_dir_mode & 0022)) -ne 0 ] &&
     [ $((adg_dir_mode & 01000)) -eq 0 ]; then
    logger -t "AdGuardHome" "生命周期锁目录可被其他用户写入且没有sticky保护"
    return 1
  fi

  if [ -L "$adg_lock_file" ]; then
    logger -t "AdGuardHome" "生命周期锁文件不能是符号链接"
    return 1
  fi
  if [ ! -e "$adg_lock_file" ]; then
    # noclobber makes first creation exclusive; umask protects the inode.
    (umask 077; set -C; : > "$adg_lock_file") 2>/dev/null || :
  fi
  if [ -L "$adg_lock_file" ] || [ ! -f "$adg_lock_file" ]; then
    logger -t "AdGuardHome" "生命周期锁文件不是普通文件"
    return 1
  fi

  adg_file_info="$(stat -c '%u %a %h' "$adg_lock_file")" || return 1
  set -- $adg_file_info
  if [ "$#" -ne 3 ] || [ "$1" != "$adg_lock_uid" ] ||
     [ "$2" != 600 ] || [ "$3" != 1 ]; then
    logger -t "AdGuardHome" "生命周期锁文件权限、所有者或链接数不安全"
    return 1
  fi
  return 0
}

# FD 9 is reserved for this short-lived launcher. BusyBox 1.24 flock supports
# locking an inherited descriptor; the shell keeps the same open description
# locked until the action ends. A bounded retry avoids indefinite RC stalls.
adg_lifecycle_lock_execute() {
  adg_lock_file="$1"
  adg_wait_seconds="$2"
  shift 2
  case "$adg_wait_seconds" in
    ''|*[!0-9]*) return 1 ;;
  esac
  [ "$#" -gt 0 ] || return 1

  if ( : <&9 ) 2>/dev/null || ( : >&9 ) 2>/dev/null; then
    logger -t "AdGuardHome" "生命周期锁FD 9已被占用，拒绝运行"
    return 1
  fi

  adg_old_umask="$(umask)"
  umask 077
  if ! exec 9>>"$adg_lock_file"; then
    umask "$adg_old_umask"
    logger -t "AdGuardHome" "无法打开生命周期锁文件"
    return 1
  fi
  umask "$adg_old_umask"

  adg_lock_attempt=0
  while ! flock -n 9; do
    if [ "$adg_lock_attempt" -ge "$adg_wait_seconds" ]; then
      exec 9>&-
      logger -t "AdGuardHome" "等待生命周期锁超时"
      return 1
    fi
    adg_lock_attempt=$((adg_lock_attempt + 1))
    sleep 1
  done

  if "$@"; then
    adg_action_status=0
  else
    adg_action_status=$?
  fi
  exec 9>&-
  return "$adg_action_status"
}

# Checked enable-state preflight prevents transient/corrupt NVRAM values
# from being mistaken for "disabled" after a running daemon is stopped.
adg_read_enable_state() {
  if adg_enable_value=$(nvram get_checked adg_enable); then
    case "$adg_enable_value" in
      ''|0|1)
        return 0
        ;;
      *)
        logger -t "AdGuardHome" "AdGuardHome enable selector is invalid; refusing to change process state"
        return 22
        ;;
    esac
  else
    adg_enable_status=$?
  fi
  logger -t "AdGuardHome" "无法读取AdGuardHome启用状态(status=$adg_enable_status)，取消重启"
  [ "$adg_enable_status" -ne 0 ] || adg_enable_status=1
  return "$adg_enable_status"
}

adg_lifecycle_locked_action() {
  case "$1" in
    start)
      start_adg
      ;;
    stop)
      stop_adg
      ;;
    restart)
      # Read and validate the enable selector before stopping a healthy daemon.
      adg_read_enable_state || return $?
      # Stop + optional start is one transaction under one inter-process lock.
      if ! stop_adg; then
        return 1
      fi
      if [ "$adg_enable_value" = 1 ]; then
        start_adg
      fi
      ;;
    *)
      return 2
      ;;
  esac
}

adg_lifecycle_lock_run() {
  adg_action="$1"
  adg_wait_seconds="${2:-10}"
  case "$adg_action" in
    start|stop|restart) ;;
    *) return 2 ;;
  esac
  adg_lock_file="$(adg_lifecycle_lock_path)" || return 1
  if ! adg_lifecycle_lock_prepare "$adg_lock_file"; then
    logger -t "AdGuardHome" "生命周期锁准备失败，取消服务操作"
    return 1
  fi
  command -v flock >/dev/null 2>&1 || {
    logger -t "AdGuardHome" "系统缺少flock，拒绝无锁执行生命周期操作"
    return 1
  }
  adg_lifecycle_lock_execute "$adg_lock_file" "$adg_wait_seconds" \
    adg_lifecycle_locked_action "$adg_action"
}


adg_dnsmasq_config_path() {
  echo /etc/storage/dnsmasq/dnsmasq.conf
}

adg_restart_dhcpd() {
  # The RC helper may detach dnsmasq; do not leak the launcher lock FD.
  /sbin/restart_dhcpd 9>&-
}

# Update dnsmasq config by same-directory staging and preserve the exact prior
# file until the daemon has accepted the new configuration.
adg_update_dns_config() {
  adg_dns_action="$1"
  adg_dns_conf="$(adg_dnsmasq_config_path)" || return 1

  if [ -L "$adg_dns_conf" ] || [ ! -f "$adg_dns_conf" ]; then
    logger -t "AdGuardHome" "DNSMasq配置不是普通文件，拒绝修改"
    return 1
  fi

  adg_dns_tmp="$(mktemp "${adg_dns_conf}.adg-new.XXXXXX")" || {
    logger -t "AdGuardHome" "无法创建DNSMasq配置暂存文件"
    return 1
  }
  adg_dns_backup="$(mktemp "${adg_dns_conf}.adg-old.XXXXXX")" || {
    rm -f "$adg_dns_tmp"
    logger -t "AdGuardHome" "无法创建DNSMasq配置回滚文件"
    return 1
  }

  if ! cp -p "$adg_dns_conf" "$adg_dns_tmp" ||
     ! cp -p "$adg_dns_conf" "$adg_dns_backup"; then
    rm -f "$adg_dns_tmp" "$adg_dns_backup"
    logger -t "AdGuardHome" "备份或暂存DNSMasq配置失败"
    return 1
  fi

  case "$adg_dns_action" in
    enable)
      if ! sed -i '/^no-resolv[[:space:]]*$/d; /^server=127\.0\.0\.1/d' "$adg_dns_tmp" ||
         ! printf '%s\n' 'no-resolv' 'server=127.0.0.1#5335' >> "$adg_dns_tmp"; then
        rm -f "$adg_dns_tmp" "$adg_dns_backup"
        logger -t "AdGuardHome" "生成AdGuardHome DNSMasq配置失败"
        return 1
      fi
      ;;
    disable)
      if ! sed -i '/^no-resolv[[:space:]]*$/d; /^server=127\.0\.0\.1#5335[[:space:]]*$/d' "$adg_dns_tmp"; then
        rm -f "$adg_dns_tmp" "$adg_dns_backup"
        logger -t "AdGuardHome" "清理AdGuardHome DNSMasq配置失败"
        return 1
      fi
      ;;
    *)
      rm -f "$adg_dns_tmp" "$adg_dns_backup"
      return 1
      ;;
  esac

  if ! mv -f "$adg_dns_tmp" "$adg_dns_conf"; then
    rm -f "$adg_dns_tmp" "$adg_dns_backup"
    logger -t "AdGuardHome" "发布DNSMasq配置失败"
    return 1
  fi

  if adg_restart_dhcpd; then
    if ! rm -f "$adg_dns_backup"; then
      logger -t "AdGuardHome" "删除DNSMasq回滚副本失败"
    fi
    return 0
  fi

  logger -t "AdGuardHome" "DNSMasq重载失败，恢复原配置"
  if ! mv -f "$adg_dns_backup" "$adg_dns_conf"; then
    logger -t "AdGuardHome" "恢复DNSMasq原配置失败，保留回滚副本: $adg_dns_backup"
    rm -f "$adg_dns_tmp"
    return 1
  fi
  rm -f "$adg_dns_tmp"
  if ! adg_restart_dhcpd; then
    logger -t "AdGuardHome" "DNSMasq回滚配置重载也失败"
  fi
  return 1
}

change_dns() {
  [ "$(nvram get adg_redirect)" = 1 ] || return 0
  if ! adg_update_dns_config enable; then
    logger -t "AdGuardHome" "添加DNS转发到5335端口失败"
    return 1
  fi
  logger -t "AdGuardHome" "添加DNS转发到5335端口"
  return 0
}

del_dns() {
  adg_update_dns_config disable
}

adg_remove_redirect_rule()
{
	adg_iptables="$1"
	shift
	adg_remove_count=0

	# Bound duplicate cleanup and distinguish "no match" from xtables errors.
	while [ "$adg_remove_count" -lt 64 ]; do
		"$adg_iptables" -t nat -C PREROUTING "$@" >/dev/null 2>&1
		adg_check_status=$?
		case "$adg_check_status" in
			0)
				if ! "$adg_iptables" -t nat -D PREROUTING "$@" >/dev/null 2>&1; then
					return 1
				fi
				adg_remove_count=$((adg_remove_count + 1))
				;;
			1)
				return 0
				;;
			*)
				logger -t "AdGuardHome" "查询DNS重定向规则失败"
				return "$adg_check_status"
				;;
		esac
	done

	# A 65th duplicate is abnormal; fail closed instead of looping forever.
	"$adg_iptables" -t nat -C PREROUTING "$@" >/dev/null 2>&1
	adg_check_status=$?
	case "$adg_check_status" in
		0)
			logger -t "AdGuardHome" "DNS重定向规则数量超过清理上限"
			return 1
			;;
		1)
			return 0
			;;
		*)
			logger -t "AdGuardHome" "验证DNS重定向清理结果失败"
			return "$adg_check_status"
			;;
	esac
}

set_iptable()
{
	# Route-derived LOCAL matching follows DHCP/PPPoE and IPv6 address changes;
	# it does not pin rules to a snapshot of interface addresses.
	if ! clear_iptable; then
		logger -t "AdGuardHome" "清理旧DNS重定向规则失败"
		return 1
	fi

	if [ "$(nvram get adg_redirect)" = 2 ]; then
		if ! iptables -t nat -A PREROUTING -p tcp -m addrtype --dst-type LOCAL ! -d 127.0.0.0/8 --dport 53 -j REDIRECT --to-ports 5335 >/dev/null 2>&1; then
			logger -t "AdGuardHome" "添加IPv4 TCP DNS重定向失败"
			return 1
		fi
		if ! iptables -t nat -A PREROUTING -p udp -m addrtype --dst-type LOCAL ! -d 127.0.0.0/8 --dport 53 -j REDIRECT --to-ports 5335 >/dev/null 2>&1; then
			logger -t "AdGuardHome" "添加IPv4 UDP DNS重定向失败"
			clear_iptable
			return 1
		fi

		# ip6tables is not installed on IPv6-disabled firmware profiles.
		# If the binary exists but table probing fails, do not silently publish
		# an IPv4-only redirect set on a dual-stack firmware.
		if command -v ip6tables >/dev/null 2>&1; then
			if ! ip6tables -t nat -L PREROUTING -n >/dev/null 2>&1; then
				logger -t "AdGuardHome" "检查IPv6 DNS NAT表失败"
				clear_iptable
				return 1
			fi
			if ! ip6tables -t nat -A PREROUTING -p tcp -m addrtype --dst-type LOCAL ! -d ::1/128 ! -d fe80::/10 --dport 53 -j REDIRECT --to-ports 5335 >/dev/null 2>&1; then
				logger -t "AdGuardHome" "添加IPv6 TCP DNS重定向失败"
				clear_iptable
				return 1
			fi
			if ! ip6tables -t nat -A PREROUTING -p udp -m addrtype --dst-type LOCAL ! -d ::1/128 ! -d fe80::/10 --dport 53 -j REDIRECT --to-ports 5335 >/dev/null 2>&1; then
				logger -t "AdGuardHome" "添加IPv6 UDP DNS重定向失败"
				clear_iptable
				return 1
			fi
		fi
		logger -t "AdGuardHome" "重定向53端口"
	fi
	return 0
}

clear_iptable()
{
	adg_cleanup_status=0

	adg_remove_redirect_rule iptables -p tcp -m addrtype --dst-type LOCAL ! -d 127.0.0.0/8 --dport 53 -j REDIRECT --to-ports 5335 || adg_cleanup_status=1
	adg_remove_redirect_rule iptables -p udp -m addrtype --dst-type LOCAL ! -d 127.0.0.0/8 --dport 53 -j REDIRECT --to-ports 5335 || adg_cleanup_status=1

	# A missing binary is normal on IPv6-disabled profiles; a present but
	# unusable nat table is an error and must not be reported as clean.
	if command -v ip6tables >/dev/null 2>&1; then
		if ip6tables -t nat -L PREROUTING -n >/dev/null 2>&1; then
			adg_remove_redirect_rule ip6tables -p tcp -m addrtype --dst-type LOCAL ! -d ::1/128 ! -d fe80::/10 --dport 53 -j REDIRECT --to-ports 5335 || adg_cleanup_status=1
			adg_remove_redirect_rule ip6tables -p udp -m addrtype --dst-type LOCAL ! -d ::1/128 ! -d fe80::/10 --dport 53 -j REDIRECT --to-ports 5335 || adg_cleanup_status=1
		else
			logger -t "AdGuardHome" "检查IPv6 DNS NAT表失败"
			adg_cleanup_status=1
		fi
	fi

	return "$adg_cleanup_status"
}

getconfig(){
  adg_file="${ADG_CONFIG_FILE:-/etc/storage/adg.sh}"

  # This YAML contains the web UI password and may contain a TLS private key.
  # Never follow a link or write credentials into a non-regular destination.
  if [ -L "$adg_file" ]; then
    logger -t "AdGuardHome" "配置文件是符号链接，拒绝启动"
    return 1
  fi
  if [ -e "$adg_file" ] && [ ! -f "$adg_file" ]; then
    logger -t "AdGuardHome" "配置文件不是普通文件，拒绝启动"
    return 1
  fi

  if [ ! -f "$adg_file" ] || [ ! -s "$adg_file" ]; then
    # Publish a complete default file from a private same-directory stage.
    adg_tmp="$(mktemp "${adg_file}.tmp.XXXXXX")" || {
      logger -t "AdGuardHome" "无法创建私有配置暂存文件"
      return 1
    }
    if ! (
      umask 077
      cat > "$adg_tmp" <<-\EEE
bind_host: 0.0.0.0
bind_port: 3030
auth_name: adguardhome
auth_pass: adguardhome
language: zh-cn
rlimit_nofile: 0
dns:
  bind_host: 0.0.0.0
  port: 5335
  protection_enabled: true
  filtering_enabled: true
  blocking_mode: nxdomain
  blocked_response_ttl: 10
  querylog_enabled: true
  ratelimit: 20
  ratelimit_whitelist: []
  refuse_any: true
  bootstrap_dns:
  - 223.5.5.5
  all_servers: true
  allowed_clients: []
  disallowed_clients: []
  blocked_hosts: []
  parental_sensitivity: 0
  parental_enabled: false
  safesearch_enabled: false
  safebrowsing_enabled: false
  resolveraddress: ""
  upstream_dns:
  - 223.5.5.5
tls:
  enabled: false
  server_name: ""
  force_https: false
  port_https: 443
  port_dns_over_tls: 853
  certificate_chain: ""
  private_key: ""
filters:
- enabled: true
  url: https://adguardteam.github.io/AdGuardSDNSFilter/Filters/filter.txt
  name: AdGuard Simplified Domain Names filter
  id: 1
- enabled: true
  url: https://adaway.org/hosts.txt
  name: AdAway
  id: 2
user_rules: []
dhcp:
  enabled: false
  interface_name: ""
  gateway_ip: ""
  subnet_mask: ""
  range_start: ""
  range_end: ""
  lease_duration: 86400
  icmp_timeout_msec: 1000
log_file: ""
verbose: false
schema_version: 3

EEE
    ); then
      rm -f "$adg_tmp"
      logger -t "AdGuardHome" "生成默认配置失败"
      return 1
    fi
    if ! chmod 600 "$adg_tmp"; then
      rm -f "$adg_tmp"
      logger -t "AdGuardHome" "无法设置默认配置权限"
      return 1
    fi
    if ! mv -f "$adg_tmp" "$adg_file"; then
      rm -f "$adg_tmp"
      logger -t "AdGuardHome" "无法发布默认配置"
      return 1
    fi
  fi

  # Existing configurations can also contain secrets; repair permissive modes
  # on every start and fail closed if confidentiality cannot be enforced.
  if ! chmod 600 "$adg_file"; then
    logger -t "AdGuardHome" "无法保护配置文件权限，拒绝启动"
    return 1
  fi
  return 0
}

start_adg(){
  # Avoid duplicate daemon instances and repeated DNS/firewall mutations.
  if adg_process_state; then
    logger -t "AdGuardHome" "AdGuardHome 已在运行，跳过重复启动"
    return 0
  else
    adg_start_state=$?
  fi
  if [ "$adg_start_state" -ne 1 ]; then
    logger -t "AdGuardHome" "无法确认AdGuardHome进程状态，拒绝启动"
    return 1
  fi

  # Do not alter resolver/firewall state until the credential-bearing config
  # has been validated, privately published and made owner-readable only.
  if ! getconfig; then
    logger -t "AdGuardHome" "配置准备失败，取消启动"
    return 1
  fi
  mkdir -p /tmp/AdGuardHome
	mkdir -p /etc/storage/AdGuardHome
	if ! set_iptable; then
		logger -t "AdGuardHome" "DNS 防火墙规则准备失败，取消启动"
		return 1
	fi
	if ! change_dns; then
		logger -t "AdGuardHome" "DNSMasq配置或重载失败，取消启动"
		if ! clear_iptable; then
			logger -t "AdGuardHome" "DNS失败后的防火墙回滚也失败"
		fi
		return 1
	fi
	logger -t "AdGuardHome" "启动 AdGuardHome"
	# The background daemon must not inherit the launcher lock descriptor.
	( exec 9>&-; AdGuardHome -c "$adg_file" -w /tmp/AdGuardHome -v ) &
  if adg_wait_started; then
    return 0
  else
    adg_start_wait_status=$?
  fi
  case "$adg_start_wait_status" in
    1)
      logger -t "AdGuardHome" "启动后在限定时间内未检测到AdGuardHome进程，回滚DNS配置和防火墙规则"
      if ! del_dns; then
        logger -t "AdGuardHome" "启动超时后的DNS配置回滚失败"
      fi
      if ! clear_iptable; then
        logger -t "AdGuardHome" "启动超时后的防火墙回滚失败"
      fi
      ;;
    *)
      # UNKNOWN may mean that the daemon is alive; do not dismantle traffic
      # state that a live AdGuardHome instance could still depend on.
      logger -t "AdGuardHome" "启动后无法确认AdGuardHome进程状态，保留DNS和防火墙配置"
      ;;
  esac
  return 1
}

# pidof only enumerates candidates. Bind each one to the installed binary,
# the service's exact argument tuple and the kernel PID start_time.
adg_process_identity() {
  adg_identity_pid="$1"
  case "$adg_identity_pid" in
    ''|0|1|0*|*[!0-9]*) return 1 ;;
  esac
  adg_identity_dir="$ADG_PROC_ROOT/$adg_identity_pid"
  [ -d "$adg_identity_dir" ] || return 1
  [ -L "$adg_identity_dir/exe" ] || return 2
  adg_identity_exe=$(readlink "$adg_identity_dir/exe" 2>/dev/null) || return 2
  case "$adg_identity_exe" in
    "$ADG_EXECUTABLE"|"$ADG_EXECUTABLE (deleted)") ;;
    *) return 1 ;;
  esac

  [ -r "$adg_identity_dir/stat" ] && [ -f "$adg_identity_dir/stat" ] || return 2
  adg_identity_start=$(awk '
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
  ' "$adg_identity_dir/stat" 2>/dev/null) || return 2
  case "$adg_identity_start" in
    ''|*[!0-9]*) return 2 ;;
  esac

  [ -r "$adg_identity_dir/cmdline" ] &&
    [ -f "$adg_identity_dir/cmdline" ] &&
    [ -s "$adg_identity_dir/cmdline" ] || return 2
  adg_identity_config="${ADG_CONFIG_FILE:-/etc/storage/adg.sh}"
  if ! tr '\000' '\n' < "$adg_identity_dir/cmdline" 2>/dev/null |
    awk -v config="$adg_identity_config" -v workdir="/tmp/AdGuardHome" '
      NR == 1 {
        executable = $0
        sub(/^.*\//, "", executable)
        if (executable != "AdGuardHome")
          invalid = 1
        next
      }
      NR == 2 { if ($0 != "-c") invalid = 1; next }
      NR == 3 { if ($0 != config) invalid = 1; next }
      NR == 4 { if ($0 != "-w") invalid = 1; next }
      NR == 5 { if ($0 != workdir) invalid = 1; next }
      NR == 6 { if ($0 != "-v") invalid = 1; next }
      { invalid = 1 }
      END {
        if (NR == 6 && !invalid)
          exit 0
        exit 1
      }
    '; then
    return 1
  fi
  printf '%s\n' "$adg_identity_start"
}

# Return 0 for RUNNING, 1 only for confirmed ABSENT, and 2 for UNKNOWN.
# A pidof name match alone never authorizes a duplicate start or a signal.
adg_process_state() {
  adg_process_pids=
  adg_process_ids=
  if adg_pidof_output=$(pidof AdGuardHome 2>/dev/null); then
    adg_pidof_status=0
  else
    adg_pidof_status=$?
  fi
  case "$adg_pidof_status" in
    0)
      [ -n "$adg_pidof_output" ] || return 2
      case "$adg_pidof_output" in
        *[!0123456789\ ]*) return 2 ;;
      esac
      for adg_candidate_pid in $adg_pidof_output; do
        case "$adg_candidate_pid" in
          ''|0*|1|*[!0-9]*) return 2 ;;
        esac
        [ "${#adg_candidate_pid}" -le 10 ] || return 2
        [ "$adg_candidate_pid" -gt 1 ] 2>/dev/null || return 2
        if adg_candidate_start=$(adg_process_identity "$adg_candidate_pid"); then
          if [ -n "$adg_process_pids" ]; then
            adg_process_pids="$adg_process_pids $adg_candidate_pid"
            adg_process_ids="$adg_process_ids $adg_candidate_pid:$adg_candidate_start"
          else
            adg_process_pids="$adg_candidate_pid"
            adg_process_ids="$adg_candidate_pid:$adg_candidate_start"
          fi
        else
          adg_candidate_status=$?
          case "$adg_candidate_status" in
            1) ;; # vanished or not the configured AdGuardHome process
            *) return 2 ;; # procfs could not prove process identity
          esac
        fi
      done
      [ -n "$adg_process_pids" ] && return 0
      [ -f "$ADG_PROC_ROOT/1/stat" ] &&
        [ -r "$ADG_PROC_ROOT/1/stat" ] && return 1
      return 2
      ;;
    1)
      [ -z "$adg_pidof_output" ] || return 2
      [ -f "$ADG_PROC_ROOT/1/stat" ] &&
        [ -r "$ADG_PROC_ROOT/1/stat" ] && return 1
      return 2
      ;;
    *) return 2 ;;
  esac
}

# Return 0 only after a validated RUNNING snapshot, 1 on confirmed ABSENT timeout,
# and 2 when the process inventory or wait primitive is UNKNOWN.
adg_wait_started() {
  case "$ADG_START_WAIT_ATTEMPTS" in
    ""|*[!0-9]*|0) return 2 ;;
  esac
  adg_start_wait=0
  while [ "$adg_start_wait" -lt "$ADG_START_WAIT_ATTEMPTS" ]; do
    if adg_process_state; then
      return 0
    else
      adg_start_state=$?
    fi
    case "$adg_start_state" in
      1) ;;
      *) return 2 ;;
    esac
    adg_start_wait=$((adg_start_wait + 1))
    if [ "$adg_start_wait" -lt "$ADG_START_WAIT_ATTEMPTS" ]; then
      if ! sleep 1; then
        return 2
      fi
    fi
  done
  return 1
}

# Return 0 when absent, 1 when still running after five polls, 2 when UNKNOWN.
adg_wait_stopped() {
  adg_wait=0
  while [ "$adg_wait" -lt 5 ]; do
    if adg_process_state; then
      adg_wait_state=0
    else
      adg_wait_state=$?
    fi
    case "$adg_wait_state" in
      1) return 0 ;;
      0) ;;
      *) return 2 ;;
    esac
    if ! sleep 1; then
      return 2
    fi
    adg_wait=$((adg_wait + 1))
  done
  if adg_process_state; then
    return 1
  else
    adg_wait_state=$?
  fi
  case "$adg_wait_state" in
    1) return 0 ;;
    2) return 2 ;;
    *) return 1 ;;
  esac
}

adg_stop_daemon() {
  if adg_process_state; then
    adg_stop_state=0
  else
    adg_stop_state=$?
  fi
  case "$adg_stop_state" in
    1) return 0 ;;
    0) adg_stop_process_ids="$adg_process_ids" ;;
    *)
      logger -t "AdGuardHome" "无法验证AdGuardHome进程身份，拒绝发送信号"
      return 1
      ;;
  esac

  # Recheck executable, argv and start_time immediately before every signal.
  # Linux 4.4 has no pidfd, so a final validate-to-kill race remains.
  adg_term_status=0
  for adg_stop_identity in $adg_stop_process_ids; do
    case "$adg_stop_identity" in *:*) ;; *) return 1 ;; esac
    adg_stop_pid="${adg_stop_identity%%:*}"
    adg_stop_start="${adg_stop_identity#*:}"
    if adg_current_start=$(adg_process_identity "$adg_stop_pid"); then
      [ "$adg_current_start" = "$adg_stop_start" ] || continue
    else
      adg_identity_status=$?
      case "$adg_identity_status" in
        1) continue ;;
        *) return 1 ;;
      esac
    fi
    if kill -TERM "$adg_stop_pid" >/dev/null 2>&1; then
      :
    else
      adg_signal_status=$?
      if [ "$adg_term_status" -eq 0 ]; then
        adg_term_status=$adg_signal_status
        [ "$adg_term_status" -ne 0 ] || adg_term_status=1
      fi
    fi
  done
  if [ "$adg_term_status" -ne 0 ]; then
    logger -t "AdGuardHome" "TERM信号返回错误(status=$adg_term_status)，继续验证进程状态"
  fi
  if adg_wait_stopped; then
    return 0
  else
    adg_wait_status=$?
  fi
  case "$adg_wait_status" in
    1) adg_kill_process_ids="$adg_process_ids" ;;
    *)
      logger -t "AdGuardHome" "TERM后进程状态未知，拒绝升级信号"
      return 1
      ;;
  esac

  logger -t "AdGuardHome" "正常停止超时，尝试强制停止"
  # adg_wait_stopped's final RUNNING result refreshed PID:start_time identities.
  adg_kill_status=0
  for adg_kill_identity in $adg_kill_process_ids; do
    case "$adg_kill_identity" in *:*) ;; *) return 1 ;; esac
    adg_kill_pid="${adg_kill_identity%%:*}"
    adg_kill_start="${adg_kill_identity#*:}"
    if adg_current_start=$(adg_process_identity "$adg_kill_pid"); then
      [ "$adg_current_start" = "$adg_kill_start" ] || continue
    else
      adg_identity_status=$?
      case "$adg_identity_status" in
        1) continue ;;
        *) return 1 ;;
      esac
    fi
    if kill -9 "$adg_kill_pid" >/dev/null 2>&1; then
      :
    else
      adg_signal_status=$?
      if [ "$adg_kill_status" -eq 0 ]; then
        adg_kill_status=$adg_signal_status
        [ "$adg_kill_status" -ne 0 ] || adg_kill_status=1
      fi
    fi
  done
  if [ "$adg_kill_status" -ne 0 ]; then
    logger -t "AdGuardHome" "SIGKILL信号返回错误(status=$adg_kill_status)，继续验证进程状态"
  fi
  if adg_wait_stopped; then
    return 0
  else
    adg_wait_status=$?
  fi
  case "$adg_wait_status" in
    1)
      logger -t "AdGuardHome" "SIGKILL后进程仍在运行"
      ;;
    *)
      logger -t "AdGuardHome" "SIGKILL后无法确认AdGuardHome进程状态"
      ;;
  esac
  return 1
}

stop_adg(){
  logger -t "AdGuardHome" "停止 AdGuardHome"

  # Keep the daemon alive until resolver publication/reload is confirmed. On
  # failure, del_dns() restores the previous file and stop remains retryable.
  if ! clear_iptable; then
    logger -t "AdGuardHome" "DNS 防火墙规则清理失败，保留运行状态"
    return 1
  fi
  if ! del_dns; then
    logger -t "AdGuardHome" "DNSMasq配置或重载失败，保留AdGuardHome进程和工作目录"
    return 1
  fi
  if ! adg_stop_daemon; then
    logger -t "AdGuardHome" "进程仍在运行，保留工作目录"
    return 1
  fi
  rm -rf /tmp/AdGuardHome
}

case "${1:-}" in
start|stop|restart)
	adg_lifecycle_lock_run "$1"
	exit $?
	;;
*)
	echo "check"
	;;
esac
