#!/bin/sh

proc_modules=${IAPPD_PROC_MODULES:-/proc/modules}
proc_interrupts=${IAPPD_PROC_INTERRUPTS:-/proc/interrupts}
log_iapp() { logger -t iappd "$1"; }

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
  case "$rc" in 0) log_iapp "already running; duplicate start skipped"; return 0 ;; 1) ;; *) return "$rc" ;; esac
  for cmd in grep ralinkiappd iptables; do
    command -v "$cmd" >/dev/null 2>&1 || { log_iapp "required command unavailable: $cmd"; return 127; }
  done
  if grep -q mt76x3_ap "$proc_modules"; then
    radio=rai0; set -- -wi rai0 -d 0
  elif grep -q rai0 "$proc_interrupts"; then
    radio=rai0; set -- -wi rai0 -wi ra0 -d 0
  else
    radio=rax0; set -- -wi rax0 -wi ra0 -d 0
  fi
  # The C daemon forks its own child; foreground invocation exposes fork errors.
  iappd_tcp_rule_added=0
  iappd_udp_rule_added=0
  ralinkiappd "$@" || {
    rc=$?
    log_iapp "launcher failed (status=$rc)"
    rollback_start_failure "$rc"
    return "$rc"
  }
  wait_running; rc=$?
  [ "$rc" -eq 0 ] || {
    log_iapp "startup postcondition failed (status=$rc)"
    rollback_start_failure "$rc"
    return "$rc"
  }
  if command -v sysctl >/dev/null 2>&1; then
    for setting in "net.ipv4.neigh.$radio.base_reachable_time_ms=10000" "net.ipv4.neigh.$radio.delay_first_probe_time=1" "net.ipv4.neigh.br0.base_reachable_time_ms=10000" "net.ipv4.neigh.br0.delay_first_probe_time=1" "net.ipv4.neigh.eth2.base_reachable_time_ms=10000" "net.ipv4.neigh.eth2.delay_first_probe_time=1" "net.ipv4.neigh.ra0.base_reachable_time_ms=10000" "net.ipv4.neigh.ra0.delay_first_probe_time=1"; do
      sysctl -wq "$setting" || log_iapp "sysctl update failed: $setting"
    done
  else log_iapp "sysctl unavailable; neighbor timing not updated"; fi
  iptables -A INPUT -i br0 -p tcp --dport 3517 -j ACCEPT || {
    rc=$?
    log_iapp "TCP firewall rule failed (status=$rc)"
    rollback_start_failure "$rc"
    return "$rc"
  }
  iappd_tcp_rule_added=1
  iptables -A INPUT -i br0 -p udp --dport 3517 -j ACCEPT || {
    rc=$?
    log_iapp "UDP firewall rule failed (status=$rc)"
    rollback_start_failure "$rc"
    return "$rc"
  }
  iappd_udp_rule_added=1
  return 0
}

stop() {
  process_state; rc=$?
  case "$rc" in 1) return 0 ;; 0) ;; *) return "$rc" ;; esac
  command -v killall >/dev/null 2>&1 || { log_iapp "killall unavailable"; return 127; }
  killall -q ralinkiappd || { stop_rc=$?; process_state; state=$?; [ "$state" -eq 1 ] && return 0; log_iapp "killall failed (status=$stop_rc)"; return "$stop_rc"; }
  wait_stopped; rc=$?
  [ "$rc" -eq 0 ] && return 0
  [ "$rc" -eq 1 ] || return "$rc"
  killall -q ralinkiappd || { stop_rc=$?; process_state; state=$?; [ "$state" -eq 1 ] && return 0; log_iapp "second killall failed (status=$stop_rc)"; return "$stop_rc"; }
  wait_stopped; rc=$?
  [ "$rc" -eq 0 ] && return 0
  [ "$rc" -eq 1 ] || return "$rc"
  log_iapp "daemon remained running after bounded stop attempts"
  return 1
}


# Roll back only resources created by this invocation, preserving its cause.
rollback_start_failure() {
  rollback_failure_status=$1
  if [ "\${iappd_udp_rule_added:-0}" -eq 1 ]; then
    iptables -D INPUT -i br0 -p udp --dport 3517 -j ACCEPT || {
      rollback_rule_status=$?
      log_iapp "UDP firewall rollback failed (status=$rollback_rule_status)"
    }
  fi
  if [ "\${iappd_tcp_rule_added:-0}" -eq 1 ]; then
    iptables -D INPUT -i br0 -p tcp --dport 3517 -j ACCEPT || {
      rollback_rule_status=$?
      log_iapp "TCP firewall rollback failed (status=$rollback_rule_status)"
    }
  fi
  stop || {
    rollback_stop_status=$?
    log_iapp "daemon startup rollback failed (status=$rollback_stop_status)"
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
