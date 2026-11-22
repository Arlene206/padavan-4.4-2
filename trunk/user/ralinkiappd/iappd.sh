#!/bin/sh

proc_modules=${IAPPD_PROC_MODULES:-/proc/modules}
proc_interrupts=${IAPPD_PROC_INTERRUPTS:-/proc/interrupts}
log_iapp() { logger -t iappd "$1"; }

iapp_remove_firewall_rule() {
  local proto tag rules rc attempts
  proto="$1"
  case "$proto" in tcp|udp) ;; *) return 2 ;; esac
  tag="ralinkiappd-$proto"
  attempts=0

  command -v iptables-save >/dev/null 2>&1 || {
    log_iapp "iptables-save unavailable; cannot verify $tag cleanup"
    return 127
  }
  command -v iptables >/dev/null 2>&1 || {
    log_iapp "iptables unavailable; cannot remove $tag"
    return 127
  }

  while [ "$attempts" -lt 8 ]; do
    rules=$(iptables-save -t filter) || {
      rc=$?
      log_iapp "cannot inspect filter rules before removing $tag (status=$rc)"
      return "$rc"
    }
    case "$rules" in
      *"$tag"*) ;;
      *) return 0 ;;
    esac
    iptables -D INPUT -i br0 -p "$proto" --dport 3517 \
      -m comment --comment "$tag" -j ACCEPT || {
      rc=$?
      log_iapp "failed to remove $tag firewall rule (status=$rc)"
      return "$rc"
    }
    attempts=$((attempts + 1))
  done

  rules=$(iptables-save -t filter) || {
    rc=$?
    log_iapp "cannot verify bounded $tag cleanup (status=$rc)"
    return "$rc"
  }
  case "$rules" in
    *"$tag"*) log_iapp "too many duplicate $tag rules; cleanup stopped"; return 1 ;;
    *) return 0 ;;
  esac
}

iapp_remove_firewall_rules() {
  local status rc
  status=0
  iapp_remove_firewall_rule tcp || {
    rc=$?
    [ "$status" -ne 0 ] || status="$rc"
  }
  iapp_remove_firewall_rule udp || {
    rc=$?
    [ "$status" -ne 0 ] || status="$rc"
  }
  return "$status"
}


iapp_ensure_firewall_rule() {
  local proto tag rules rc
  proto="$1"
  case "$proto" in tcp|udp) ;; *) return 2 ;; esac
  tag="ralinkiappd-$proto"

  for cmd in grep iptables iptables-save; do
    command -v "$cmd" >/dev/null 2>&1 || {
      log_iapp "required command unavailable while reconciling $tag: $cmd"
      return 127
    }
  done

  rules=$(iptables-save -t filter) || {
    rc=$?
    log_iapp "cannot inspect filter rules while reconciling $tag (status=$rc)"
    return "$rc"
  }
  if printf '%s\n' "$rules" |
      grep -F -- "-A INPUT -i br0 -p $proto " |
      grep -F -- "--dport 3517" |
      grep -F -- "--comment $tag" |
      grep -F -- "-j ACCEPT" >/dev/null 2>&1; then
    return 0
  fi

  case "$rules" in
    *"$tag"*)
      log_iapp "found a conflicting rule using owned tag $tag; refusing to add a duplicate"
      return 1
      ;;
  esac

  iptables -A INPUT -i br0 -p "$proto" --dport 3517 -m comment --comment "$tag" -j ACCEPT || {
    rc=$?
    log_iapp "failed to restore missing $tag firewall rule (status=$rc)"
    return "$rc"
  }

  rules=$(iptables-save -t filter) || {
    rc=$?
    log_iapp "cannot verify restored $tag firewall rule (status=$rc)"
    return "$rc"
  }
  if ! printf '%s\n' "$rules" |
      grep -F -- "-A INPUT -i br0 -p $proto " |
      grep -F -- "--dport 3517" |
      grep -F -- "--comment $tag" |
      grep -F -- "-j ACCEPT" >/dev/null 2>&1; then
    log_iapp "restored $tag firewall rule did not appear in filter inventory"
    return 1
  fi
  return 0
}

process_state() {
  command -v pidof >/dev/null 2>&1 || { log_iapp "pidof unavailable"; return 127; }
  pidof ralinkiappd >/dev/null 2>&1; rc=$?
  case "$rc" in 0) return 0 ;; 1) return 1 ;; *) log_iapp "pidof failed (status=$rc)"; return "$rc" ;; esac
}

wait_running() {
  n=0
  while [ "$n" -lt 5 ]; do
    process_state; rc=$?
    case "$rc" in 0) return 0 ;; 1) ;; *) return "$rc" ;; esac
    n=$((n + 1))
    [ "$n" -lt 5 ] && { sleep 1 || return $?; }
  done
  log_iapp "ralinkiappd did not remain running after launch"
  return 1
}

wait_stopped() {
  n=0
  while [ "$n" -lt 3 ]; do
    sleep 1 || return $?
    process_state; rc=$?
    case "$rc" in 0) ;; 1) return 0 ;; *) return "$rc" ;; esac
    n=$((n + 1))
  done
  return 1
}

start() {
  process_state; rc=$?
  case "$rc" in
    0)
      # A live daemon does not prove that both tagged firewall rules survived.
      # Reconcile missing rules before reporting an idempotent start as healthy.
      log_iapp "already running; reconciling tagged firewall rules"
      iapp_ensure_firewall_rule tcp || return $?
      iapp_ensure_firewall_rule udp || return $?
      return 0
      ;;
    1) ;;
    *) return "$rc" ;;
  esac
  for cmd in grep ralinkiappd iptables iptables-save; do
    command -v "$cmd" >/dev/null 2>&1 || { log_iapp "required command unavailable: $cmd"; return 127; }
  done
  # Remove stale rules from a previous crash before publishing a new daemon.
  iapp_remove_firewall_rules || return $?
  # grep distinguishes an absent match (1) from an unreadable procfs
  # inventory (>1). Never choose a fallback radio when hardware state is unknown.
  grep -q mt76x3_ap "$proc_modules" 2>/dev/null
  match_status=$?
  case "$match_status" in
    0)
      radio=rai0; set -- -wi rai0 -d 0
      ;;
    1)
      grep -q rai0 "$proc_interrupts" 2>/dev/null
      match_status=$?
      case "$match_status" in
        0) radio=rai0; set -- -wi rai0 -wi ra0 -d 0 ;;
        1) radio=rax0; set -- -wi rax0 -wi ra0 -d 0 ;;
        *)
          log_iapp "cannot inspect radio interrupt inventory (status=$match_status)"
          return "$match_status"
          ;;
      esac
      ;;
    *)
      log_iapp "cannot inspect radio module inventory (status=$match_status)"
      return "$match_status"
      ;;
  esac
  # The C daemon forks its own child; foreground invocation exposes fork errors.
  ralinkiappd "$@" || {
    rc=$?
    log_iapp "launcher failed (status=$rc)"
    rollback_start_failure "$rc"
    return $?
  }
  wait_running; rc=$?
  [ "$rc" -eq 0 ] || {
    log_iapp "startup postcondition failed (status=$rc)"
    rollback_start_failure "$rc"
    return $?
  }
  if command -v sysctl >/dev/null 2>&1; then
    for setting in "net.ipv4.neigh.$radio.base_reachable_time_ms=10000" "net.ipv4.neigh.$radio.delay_first_probe_time=1" "net.ipv4.neigh.br0.base_reachable_time_ms=10000" "net.ipv4.neigh.br0.delay_first_probe_time=1" "net.ipv4.neigh.eth2.base_reachable_time_ms=10000" "net.ipv4.neigh.eth2.delay_first_probe_time=1" "net.ipv4.neigh.ra0.base_reachable_time_ms=10000" "net.ipv4.neigh.ra0.delay_first_probe_time=1"; do
      sysctl -wq "$setting" || log_iapp "sysctl update failed: $setting"
    done
  else log_iapp "sysctl unavailable; neighbor timing not updated"; fi
  iptables -A INPUT -i br0 -p tcp --dport 3517 -m comment --comment ralinkiappd-tcp -j ACCEPT || {
    rc=$?
    log_iapp "TCP firewall rule failed (status=$rc)"
    rollback_start_failure "$rc"
    return $?
  }
  iptables -A INPUT -i br0 -p udp --dport 3517 -m comment --comment ralinkiappd-udp -j ACCEPT || {
    rc=$?
    log_iapp "UDP firewall rule failed (status=$rc)"
    rollback_start_failure "$rc"
    return $?
  }
  return 0
}

stop() {
  process_state; rc=$?
  case "$rc" in
    1) iapp_remove_firewall_rules; return $? ;;
    0) ;;
    *) return "$rc" ;;
  esac
  command -v killall >/dev/null 2>&1 || { log_iapp "killall unavailable"; return 127; }
  killall -q ralinkiappd || {
    stop_rc=$?
    process_state; state=$?
    if [ "$state" -eq 1 ]; then
      iapp_remove_firewall_rules
      return $?
    fi
    log_iapp "killall failed (status=$stop_rc)"
    return "$stop_rc"
  }
  wait_stopped; rc=$?
  if [ "$rc" -eq 0 ]; then
    iapp_remove_firewall_rules
    return $?
  fi
  [ "$rc" -eq 1 ] || return "$rc"
  killall -q ralinkiappd || {
    stop_rc=$?
    process_state; state=$?
    if [ "$state" -eq 1 ]; then
      iapp_remove_firewall_rules
      return $?
    fi
    log_iapp "second killall failed (status=$stop_rc)"
    return "$stop_rc"
  }
  wait_stopped; rc=$?
  if [ "$rc" -eq 0 ]; then
    iapp_remove_firewall_rules
    return $?
  fi
  [ "$rc" -eq 1 ] || return "$rc"
  log_iapp "daemon remained running after bounded stop attempts"
  return 1
}


# Roll back only resources created by this invocation, preserving its cause.
rollback_start_failure() {
  rollback_failure_status=$1
  stop || {
    rollback_stop_status=$?
    log_iapp "daemon/firewall startup rollback failed (status=$rollback_stop_status)"
  }
  return "$rollback_failure_status"
}

 restart() { stop; rc=$?; [ "$rc" -eq 0 ] || return "$rc"; start; }
test_running() { process_state; rc=$?; [ "$rc" -ne 1 ] && return "$rc"; log_iapp "daemon is not running"; return 1; }

case "$1" in
  start) start; exit $? ;;
  stop) stop; exit $? ;;
  restart) restart; exit $? ;;
  test) test_running; exit $? ;;
  *) echo "Usage: $0 {start|stop|restart|test}" >&2; exit 1 ;;
esac
