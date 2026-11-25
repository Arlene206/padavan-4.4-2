#!/bin/sh
set -u
base=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
tmp=$(mktemp -d "${TMPDIR:-/tmp}/iappd-test.XXXXXX") || exit 1
trap 'rm -rf "$tmp"' EXIT HUP INT TERM
mkdir -p "$tmp/bin" "$tmp/state" "$tmp/proc"
: > "$tmp/commands"; : > "$tmp/daemon"
PROC_ROOT="$tmp/proc"
printf "mt76x3_ap\n" > "$tmp/modules"
printf "rai0: 1\n" > "$tmp/interrupts"
cat > "$tmp/mock" <<'EOF'
#!/bin/sh
name=${0##*/}
echo "$name $*" >> "$COMMAND_LOG"
case "$name" in
  ralinkiappd)
    echo "$*" >> "$DAEMON_LOG"
    if [ "$MOCK_START" -eq 0 ] || [ "$MOCK_PARTIAL_START" -eq 1 ]; then
      : > "$STATE_DIR/daemon"
      mkdir -p "$PROC_ROOT/123"
      printf '%s\n' '123 (ralinkiappd) S 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19' > "$PROC_ROOT/123/stat"; ln -sf /sbin/ralinkiappd "$PROC_ROOT/123/exe"
    fi
    if [ "$MOCK_EXIT_AFTER_LAUNCH" -eq 1 ]; then
      rm -f "$STATE_DIR/daemon"
      rm -rf "$PROC_ROOT/123"
    fi
    exit "$MOCK_START" ;;
  sysctl) exit "$MOCK_SYSCTL" ;;
  iptables)
    if [ "$1" = "-A" ]; then
      append_count=$(grep -c '^iptables -A ' "$COMMAND_LOG")
      if [ "$MOCK_IPTABLES_FAIL_AT" -gt 0 ] &&
         [ "$append_count" -eq "$MOCK_IPTABLES_FAIL_AT" ]; then
        exit "$MOCK_IPTABLES"
      fi
      shift
      printf '%s\n' "-A $*" >> "$RULES_FILE"
      exit 0
    fi
    if [ "$1" = "-D" ]; then
      [ "$MOCK_IPTABLES_DELETE" -eq 0 ] || exit "$MOCK_IPTABLES_DELETE"
      shift
      target="-A"
      for arg in "$@"; do target="$target $arg"; done
      awk -v target="$target" 'BEGIN { removed=0 } $0 == target && !removed { removed=1; next } { print } END { if (!removed) exit 1 }' "$RULES_FILE" > "$RULES_FILE.tmp" || exit 93
      mv "$RULES_FILE.tmp" "$RULES_FILE" || exit 94
      exit 0
    fi
    if [ "$1" = "-C" ]; then
      [ "$MOCK_IPTABLES_CHECK" -eq 0 ] || exit "$MOCK_IPTABLES_CHECK"
      shift
      target="-A"
      for arg in "$@"; do target="$target $arg"; done
      grep -Fqx -- "$target" "$RULES_FILE"
      exit $?
    fi
    exit 0 ;;
  iptables-save)
    [ "$MOCK_IPTABLES_SAVE" -eq 0 ] || exit "$MOCK_IPTABLES_SAVE"
    [ "$1" = "-t" ] && [ "$2" = "filter" ] || exit 95
    cat "$RULES_FILE"
    exit 0 ;;

  sleep|logger) exit 0 ;;
  *) exit 127 ;;
esac
EOF
chmod +x "$tmp/mock"
for c in ralinkiappd sysctl iptables iptables-save sleep logger; do ln -s "$tmp/mock" "$tmp/bin/$c"; done
cat > "$tmp/kill-helper" <<'EOF'
#!/bin/sh
signal=$1; pid=$2
echo "signal $signal $pid" >> "$COMMAND_LOG"
[ "$MOCK_KILL" -eq 0 ] || exit "$MOCK_KILL"
if [ "$MOCK_REMOVE" -eq 1 ]; then
  rm -f "$STATE_DIR/daemon"
  rm -rf "$PROC_ROOT/$pid"
fi
exit 0
EOF
chmod +x "$tmp/kill-helper"
ln -s /bin/grep "$tmp/bin/grep"
export PATH="$tmp/bin:$PATH" STATE_DIR="$tmp/state" COMMAND_LOG="$tmp/commands" DAEMON_LOG="$tmp/daemon" RULES_FILE="$tmp/state/firewall.rules"
: > "$RULES_FILE"
export PROC_ROOT IAPPD_PROC_ROOT="$PROC_ROOT"
export IAPPD_KILL_HELPER="$tmp/kill-helper"
export MOCK_START=0 MOCK_PARTIAL_START=0 MOCK_EXIT_AFTER_LAUNCH=0 MOCK_SYSCTL=0 MOCK_IPTABLES=0 MOCK_IPTABLES_FAIL_AT=0 MOCK_IPTABLES_DELETE=0 MOCK_IPTABLES_SAVE=0 MOCK_IPTABLES_CHECK=0 MOCK_KILL=0 MOCK_REMOVE=1
export IAPPD_PROC_MODULES="$tmp/modules" IAPPD_PROC_INTERRUPTS="$tmp/interrupts"
fail() { echo "$*" >&2; exit 1; }
status_count=0
status() {
  status_count=$((status_count + 1))
  [ "$1" -eq "$2" ] || fail "status assertion #$status_count expected $2, got $1"
}
run() { /bin/sh "$base/iappd.sh" "$1"; }
iapp_source="$base/src/rtmpiapp.c"
iapp_util_source="$base/src/mt_iapp_util.c"
grep -Fq '#include "ft_client_hash_policy.h"' "$iapp_util_source" ||
  fail "IAPP FT client utility must use the bounded hash-chain policy"
grep -Fq 'ft_client_hash_find_free_slot(pFtTable->ft_sta_info' "$iapp_util_source" ||
  fail "IAPP FT client insertion must fail closed when no free slot exists"
grep -Fq 'ft_client_hash_append(&pFtTable->hash[HashIdx]' "$iapp_util_source" ||
  fail "IAPP FT client insertion must use bounded hash-chain append"
grep -Fq 'ft_client_hash_unlink(&pFtTable->hash[HashIdx]' "$iapp_util_source" ||
  fail "IAPP FT client deletion must unlink before slot release"
delete_unlink_line=$(grep -n -F 'ft_client_hash_unlink(&pFtTable->hash[HashIdx]' "$iapp_util_source" | cut -d: -f1)
delete_clear_line=$(grep -n -F 'ft_entry->used = 0;' "$iapp_util_source" | tail -1 | cut -d: -f1)
[ -n "$delete_unlink_line" ] && [ -n "$delete_clear_line" ] &&
  [ "$delete_unlink_line" -lt "$delete_clear_line" ] ||
  fail "IAPP FT client deletion must detach a node before marking its slot free"

eth_guard_line=$(grep -n -F "iapp_ifname_arg_valid(strlen(pArgv[0]), sizeof(pCtrlBK->IfNameEth))" "$iapp_source" | cut -d: -f1)
eth_store_line=$(grep -n -F "strcpy(pCtrlBK->IfNameEth, pArgv[0]);" "$iapp_source" | cut -d: -f1)
[ -n "$eth_guard_line" ] && [ -n "$eth_store_line" ] && [ "$eth_guard_line" -lt "$eth_store_line" ] ||
  fail "IAPP -e parser must bound the Ethernet interface name before copying"
wlan_guard_line=$(grep -n -F "iapp_ifname_arg_valid(strlen(pArgv[0]), sizeof(pCtrlBK->IfNameWlan))" "$iapp_source" | cut -d: -f1)
wlan_store_line=$(grep -n -F "strcpy(pCtrlBK->IfNameWlan, pArgv[0]);" "$iapp_source" | cut -d: -f1)
[ -n "$wlan_guard_line" ] && [ -n "$wlan_store_line" ] && [ "$wlan_guard_line" -lt "$wlan_store_line" ] ||
  fail "IAPP -w parser must bound the wireless interface name before copying"
[ "$(grep -F -c "iapp_ifname_arg_valid(strlen(pArgv[0])," "$iapp_source")" -eq 2 ] ||
  fail "IAPP -e and -w parser must both validate fixed-size interface names"
grep -Fq "iapp_ioctl_ifname_arg_valid(" "$iapp_source" ||
  fail "IAPP -wi parser must validate interface count and name length"
parser_guard_line=$(grep -n -F "iapp_ioctl_ifname_arg_valid(" "$iapp_source" | head -1 | cut -d: -f1)
parser_store_line=$(grep -n -F "strcpy(pCtrlBK->IfNameWlanIoctl[pCtrlBK->IfNameWlanCount]," "$iapp_source" | head -1 | cut -d: -f1)
[ -n "$parser_guard_line" ] && [ -n "$parser_store_line" ] && [ "$parser_guard_line" -lt "$parser_store_line" ] ||
  fail "IAPP -wi parser must validate before writing the fixed interface array"
udp_guard_line=$(grep -n -F "iapp_wifi_index_valid(if_idx, MAX_WIFI_COUNT, pCtrlBK->IfNameWlanCount)" "$iapp_source" | cut -d: -f1)
udp_mac_line=$(grep -n -F "pCtrlBK->IfNameWlanMAC[if_idx]" "$iapp_source" | cut -d: -f1)
[ -n "$udp_guard_line" ] && [ -n "$udp_mac_line" ] && [ "$udp_guard_line" -lt "$udp_mac_line" ] ||
  fail "IAPP UDP sender must validate the BSS index before reading its MAC array"
grep -Fq "iapp_wifi_index_valid(" "$base/src/iapp_ioctl_ifname_policy.h" ||
  fail "IAPP fixed-array index policy helper is missing"
grep -Fq "iapp_rx_payload_fits(" "$iapp_source" ||
  fail "IAPP FT payload handlers must validate actual receive bounds"
grep -Fq "pSendSB->Length != sizeof(kdp_info)" "$iapp_source" ||
  fail "IAPP FT security block must bound the fixed stack destination"
grep -Fq "pApIB->Length))" "$iapp_source" ||
  fail "IAPP AP information must bound its declared payload by received bytes"
grep -Fq "SizeRcvMsg < (INT32)FT_RRB_HEADER_SIZE" "$iapp_source" ||
  fail "IAPP raw RRB must verify the fixed frame header before field access"
grep -Fq "iapp_rx_payload_fits((size_t)ReceivedLength" "$iapp_source" ||
  fail "IAPP raw RRB must bound aligned encrypted length by the actual frame"
grep -Fq "iapp_rx_buffer_append_fits((size_t)ReceivedLength" "$iapp_source" ||
  fail "IAPP raw RRB must bound driver-returned plaintext length before copying"
grep -Fq "RT_SIGNAL_STRUC_HDR_SIZE + FT_KDP_EVT_HEADER_SIZE" "$iapp_source" ||
  fail "IAPP raw driver signal must contain its fixed event prefix"
grep -Fq "(size_t)pEvtHeader->EventLen" "$iapp_source" ||
  fail "IAPP driver event payload must fit the actual signal bytes"
grep -Fq 'SizeRcvMsg - (INT32)sizeof(FT_ETH_HEADER)' "$iapp_source" ||
  fail "IAPP raw driver must pass the signal length without the Ethernet header"
grep -Fq 'received_len != (INT32)sizeof(*pNotify)' "$iapp_source" ||
  fail "IAPP MOVE_NOTIFY must match its complete received frame size"
grep -Fq 'SizeRcvMsg != (INT32)sizeof(*pNotify)' "$iapp_source" ||
  fail "IAPP ADD_NOTIFY must match its complete received frame size"
[ "$(grep -F -c 'iapp_rx_frame_length_matches((size_t)SizeRcvMsg' "$iapp_source")" -eq 2 ] ||
  fail "IAPP TCP and UDP must match declared Length to actual received bytes"
grep -Fq 'SizeRcvMsg != (INT32)sizeof(*SM_p)' "$iapp_source" ||
  fail "IAPP security-monitor event must match its fixed frame size"
[ "$(grep -F -c 'iapp_rx_buffer_append_fits((size_t)ReceivedLength, 0' "$iapp_source")" -eq 2 ] ||
  fail "TCP and UDP decrypt outputs must fit their original received buffers"
tcp_length_guard=$(grep -n -F 'Invalid TCP decrypt output length' "$iapp_source" | cut -d: -f1)
udp_length_guard=$(grep -n -F 'Invalid UDP decrypt output length' "$iapp_source" | cut -d: -f1)
[ -n "$tcp_length_guard" ] && [ -n "$udp_length_guard" ] ||
  fail "TCP and UDP decrypt output guards must remain present outside feature conditionals"
grep -Fq 'iapp_rx_buffer_append_fits(sizeof(RT_SIGNAL_STRUC), 0' "$iapp_source" ||
  fail "driver signal-context ioctl output must fit its allocated structure"
grep -Fq 'Query signal context ioctl failed' "$iapp_source" ||
  fail "driver signal-context ioctl failure must not forward uninitialized bytes"
grep -Fq 'iapp_rx_length_within_capacity(' "$iapp_source" ||
  fail "IAPP encryption must validate signed driver-returned output lengths"
[ "$(grep -F -c 'IAPP_EncryptFrame(' "$iapp_source")" -eq 7 ] ||
  fail "all six transmit encryption paths must use the checked wrapper"
grep -Fq 'IAPP_TCP_ReadFrame(SockNew, pPktBuf, IAPP_MAX_RCV_PKT_SIZE)' "$iapp_source" ||
  fail "IAPP TCP receive must accumulate fragmented frames through EOF"
grep -Fq '#define IAPP_TCP_FRAME_TIMEOUT_SEC 5' "$iapp_source" ||
  fail "IAPP TCP frame receive must have a total peer timeout"
grep -Fq 'IAPP_TCP_Wait(Socket, Deadline, FALSE)' "$iapp_source" ||
  fail "IAPP TCP reader must enforce its absolute deadline before each read"
grep -Fq 'Socket >= FD_SETSIZE' "$iapp_source" ||
  fail "IAPP TCP select helper must reject descriptors outside fd_set capacity"
grep -Fq 'if (BytesRead == 0)' "$iapp_source" ||
  fail "IAPP TCP frame reader must stop only at connection EOF"
grep -Fq 'errno == EINTR' "$iapp_source" ||
  fail "IAPP TCP frame reader must retry interrupted reads"
grep -Fq 'iapp_rx_buffer_append_fits(' "$iapp_source" ||
  fail "IAPP TCP frame reader must enforce receive-buffer capacity"
[ "$(grep -F -c 'IAPP_TCP_WriteFrame(' "$iapp_source")" -eq 5 ] ||
  fail "all IAPP TCP send paths must use the complete-write helper"
grep -Fq 'if (BytesSent == 0)' "$iapp_source" ||
  fail "IAPP TCP complete-write helper must reject zero-byte writes"
grep -Fq '#ifdef MSG_NOSIGNAL' "$iapp_source" ||
  fail "IAPP TCP writes must suppress SIGPIPE on platforms supporting MSG_NOSIGNAL"
grep -Fq 'send(Socket, Buffer + TotalSent, Length - TotalSent, MSG_NOSIGNAL)' "$iapp_source" ||
  fail "IAPP TCP complete-write helper must use MSG_NOSIGNAL when available"
grep -Fq '#ifdef IAPP_OS_LINUX' "$iapp_source" ||
  fail "Linux IAPP daemon must explicitly protect pipe writes from SIGPIPE"
grep -Fq 'signal(SIGPIPE, SIG_IGN)' "$iapp_source" ||
  fail "IAPP daemon must ignore SIGPIPE so driver-pipe EPIPE is handled"
[ "$(grep -F -c 'IAPP_TCP_Connect(SocketPeer' "$iapp_source")" -eq 4 ] ||
  fail "all four IAPP TCP send paths must use the bounded connect helper"
grep -Fq 'OriginalFlags | O_NONBLOCK' "$iapp_source" ||
  fail "Linux IAPP TCP connect must use non-blocking mode"
grep -Fq 'IAPP_TCP_Wait(Socket, Deadline, TRUE)' "$iapp_source" ||
  fail "Linux IAPP TCP connect must enforce its absolute timeout"
grep -Fq 'getsockopt(Socket, SOL_SOCKET, SO_ERROR' "$iapp_source" ||
  fail "Linux IAPP TCP connect must check asynchronous connection errors"
grep -Fq 'WaitStatus = IAPP_TCP_Wait(Socket, Deadline, TRUE)' "$iapp_source" ||
  fail "Linux TCP frame writes must enforce the absolute send deadline"
grep -Fq 'errno == EAGAIN || errno == EWOULDBLOCK' "$iapp_source" ||
  fail "Linux TCP nonblocking frame writes must retry when the socket would block"
grep -Fq 'write_fail:' "$iapp_source" ||
  fail "TCP frame write failure path must compile on non-Linux targets too"
[ "$(grep -F -c 'OriginalFlags | O_NONBLOCK' "$iapp_source")" -eq 2 ] ||
  fail "Linux connect and write helpers must each use nonblocking socket mode"
grep -Fq 'fcntl(Socket, F_SETFL, OriginalFlags)' "$iapp_source" ||
  fail "IAPP TCP connect must restore the original socket flags"
ap_info_guard_line=$(grep -n -F "pApIB->Length))" "$iapp_source" | cut -d: -f1)
ap_info_copy_line=$(grep -n -F "IAPP_MEM_MOVE(OidReq->Buf+FT_IP_ADDRESS_SIZE, pApIB->IB, pApIB->Length)" "$iapp_source" | cut -d: -f1)
[ -n "$ap_info_guard_line" ] && [ -n "$ap_info_copy_line" ] && [ "$ap_info_guard_line" -lt "$ap_info_copy_line" ] ||
  fail "IAPP AP information payload must be copied only after the receive-length guard"
[ "$(grep -F -c "SizeRcvMsg < (INT32)sizeof(RT_IAPP_HEADER)" "$iapp_source")" -eq 2 ] ||
  fail "IAPP TCP and UDP handlers must reject truncated headers"
grep -Fq "iapp_ioctl_ifname_select(" "$iapp_source" ||
  fail "IAPP ioctl path must use the bounded interface selector"
grep -Fq "ApIdx, sizeof(IfName), FT_KDP_DEFAULT_IF_WLAN_IOCTL" "$iapp_source" ||
  fail "IAPP ioctl selector must receive the BSS index, output bound and fallback"
grep -Fq "strlen(pCtrlBK->IfNameWlanIoctl[ApIdx])" "$iapp_source" &&
  fail "IAPP ioctl path must not read the interface array before validating ApIdx"

iapp_ifname_test="$tmp/iapp_ioctl_ifname_policy"
cc -std=gnu89 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer "$base/tests/iapp_ioctl_ifname_policy.c" -o "$iapp_ifname_test" ||
  fail "could not compile IAPP interface and receive-length regression"
ASAN_OPTIONS=detect_leaks=1 "$iapp_ifname_test" || fail "IAPP interface and receive-length regression failed"
grep -Fq 'if (listen(pCtrlBK->SocketTcpRcv, 128) < 0)' "$iapp_source" ||
  fail "IAPP TCP listener setup must fail closed when listen() fails"
grep -A4 -F 'if (listen(pCtrlBK->SocketTcpRcv, 128) < 0)' "$iapp_source" |
  grep -Fq 'goto label_fail;' ||
  fail "IAPP listen failure must close all partially opened sockets"
grep -A4 -F 'if (listen(pCtrlBK->SocketTcpRcv, 128) < 0)' "$iapp_source" |
  grep -Fq 'Listen Rcv TCP failed' ||
  fail "IAPP listen failure must be logged"
tcp_reuse_line=$(grep -n -F 'setsockopt(pCtrlBK->SocketTcpRcv, SOL_SOCKET, SO_REUSEADDR,' "$iapp_source" | cut -d: -f1)
tcp_bind_line=$(grep -n -F 'if (bind(pCtrlBK->SocketTcpRcv,' "$iapp_source" | cut -d: -f1)
[ -n "$tcp_reuse_line" ] && [ -n "$tcp_bind_line" ] && [ "$tcp_reuse_line" -lt "$tcp_bind_line" ] ||
  fail "IAPP TCP SO_REUSEADDR must be set before bind()"
grep -A5 -F 'setsockopt(pCtrlBK->SocketTcpRcv, SOL_SOCKET, SO_REUSEADDR,' "$iapp_source" |
  grep -Fq 'goto label_fail;' ||
  fail "IAPP TCP SO_REUSEADDR failure must close partial socket state"
iapp_reuse_test="$tmp/iapp_tcp_restart_reuseaddr"
cc -std=c99 -Wall -Wextra -Werror -D_POSIX_C_SOURCE=200809L \
  "$base/tests/iapp_tcp_restart_reuseaddr.c" -o "$iapp_reuse_test" ||
  fail "could not compile IAPP TCP restart/reuse-address regression"
"$iapp_reuse_test" || fail "IAPP TCP TIME_WAIT restart regression failed"
capture_status() { "$@"; rc=$?; printf "%s" "$rc"; }
mark_running() { mkdir -p "$PROC_ROOT/123"; printf '%s\n' '123 (ralinkiappd) S 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19' > "$PROC_ROOT/123/stat"; ln -sf /sbin/ralinkiappd "$PROC_ROOT/123/exe"; : > "$STATE_DIR/daemon"; }
mark_stopped() { rm -f "$STATE_DIR/daemon"; rm -rf "$PROC_ROOT/123"; }
reset() { mark_stopped; rm -rf "$PROC_ROOT/234"; IAPPD_PROC_ROOT="$PROC_ROOT"; export IAPPD_PROC_ROOT; : > "$COMMAND_LOG"; : > "$DAEMON_LOG"; : > "$RULES_FILE"; MOCK_START=0 MOCK_PARTIAL_START=0 MOCK_EXIT_AFTER_LAUNCH=0 MOCK_IPTABLES=0 MOCK_IPTABLES_FAIL_AT=0 MOCK_IPTABLES_DELETE=0 MOCK_IPTABLES_SAVE=0 MOCK_IPTABLES_CHECK=0 MOCK_KILL=0 MOCK_REMOVE=1; }

reset
IAPPD_PROC_ROOT="$tmp/missing-proc"
export IAPPD_PROC_ROOT
rc=$(capture_status run start); status "$rc" 2
! grep -q "^ralinkiappd " "$COMMAND_LOG" || fail "started with unavailable process inventory"
! grep -q "^iptables -A " "$COMMAND_LOG" || fail "published firewall with unavailable process inventory"

reset
mark_running
IAPPD_PROC_ROOT="$tmp/missing-proc"
export IAPPD_PROC_ROOT
rc=$(capture_status run stop); status "$rc" 2
[ -f "$STATE_DIR/daemon" ] || fail "unknown inventory changed daemon state"
! grep -q "^signal " "$COMMAND_LOG" || fail "stopped with unavailable process inventory"
IAPPD_PROC_ROOT="$PROC_ROOT"
export IAPPD_PROC_ROOT

# A same-name process with a different executable is not the daemon.
# Stop must not send it any signal, even though its comm field is spoofed.
reset
mkdir -p "$PROC_ROOT/234"
printf '%s\n' '234 (ralinkiappd) S 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19' > "$PROC_ROOT/234/stat"
ln -s /bin/sleep "$PROC_ROOT/234/exe"
rc=$(capture_status run test); status "$rc" 1
rc=$(capture_status run stop); status "$rc" 0
[ -d "$PROC_ROOT/234" ] || fail "stop removed same-name non-daemon process"
! grep -q "^signal " "$COMMAND_LOG" || fail "stop signalled same-name non-daemon process"

# A matching comm whose executable symlink cannot be inspected is UNKNOWN;
# neither startup nor shutdown may act on an incomplete identity snapshot.
reset
mkdir -p "$PROC_ROOT/234"
printf '%s\n' '234 (ralinkiappd) S 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19' > "$PROC_ROOT/234/stat"
rc=$(capture_status run start); status "$rc" 2
rc=$(capture_status run stop); status "$rc" 2
! grep -q "^ralinkiappd " "$COMMAND_LOG" || fail "started with unreadable executable identity"
! grep -q "^signal " "$COMMAND_LOG" || fail "signalled with unreadable executable identity"

# A PID that remains present but cannot be inspected makes the snapshot
# UNKNOWN; do not interpret a partial listing as confirmed absence.
reset
mkdir -p "$PROC_ROOT/234/stat"
rc=$(capture_status run start); status "$rc" 2
! grep -q "^ralinkiappd " "$COMMAND_LOG" || fail "started with an unreadable process stat"
rm -rf "$PROC_ROOT/234"

# Zombies are not live service instances and must not suppress a fresh launch.
reset
mkdir -p "$PROC_ROOT/123"
printf '%s\n' '123 (ralinkiappd) Z 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19' > "$PROC_ROOT/123/stat"
rc=$(capture_status run start); status "$rc" 0
[ -f "$STATE_DIR/daemon" ] || fail "zombie process suppressed service launch"
grep -q "^ralinkiappd " "$COMMAND_LOG" || fail "zombie process was treated as running"

reset; MOCK_START=53; MOCK_PARTIAL_START=1
rc=$(capture_status run start); status "$rc" 53
[ ! -f "$STATE_DIR/daemon" ] || fail "launcher failure left a partially started daemon"
grep -q "^signal TERM 123$" "$COMMAND_LOG" || fail "launcher failure skipped targeted daemon rollback"
! grep -q "^iptables -A " "$COMMAND_LOG" || fail "configured firewall after launcher failure"

# A successful launcher that exits before the bounded readiness check
# must not be reported as a successful service start or publish firewall rules.
reset; MOCK_EXIT_AFTER_LAUNCH=1
rc=$(capture_status run start); status "$rc" 1
[ ! -f "$STATE_DIR/daemon" ] || fail "readiness failure left daemon marker"
! grep -q "^iptables -A " "$COMMAND_LOG" || fail "published firewall after readiness failure"
grep -q "did not remain running after launch" "$COMMAND_LOG" || fail "readiness timeout was not logged"

# Cleanup failure must not replace the startup error being reported.
reset; MOCK_START=53; MOCK_PARTIAL_START=1; MOCK_KILL=54
rc=$(capture_status run start); status "$rc" 53
[ -f "$STATE_DIR/daemon" ] || fail "failed cleanup unexpectedly changed daemon state"
grep -q "daemon/firewall startup rollback failed" "$COMMAND_LOG" || fail "failed daemon/firewall rollback was not logged"

# A failed first firewall rule stops the daemon without deleting a rule that
# this invocation did not successfully create.
reset; MOCK_IPTABLES=42; MOCK_IPTABLES_FAIL_AT=1
rc=$(capture_status run start); status "$rc" 42
[ ! -f "$STATE_DIR/daemon" ] || fail "TCP rule failure left daemon running"
grep -q "^signal TERM 123$" "$COMMAND_LOG" || fail "TCP rule failure skipped targeted daemon rollback"
[ "$(grep -c "^iptables -D " "$COMMAND_LOG")" -eq 0 ] || fail "removed an unowned firewall rule"

# If UDP publication fails after TCP succeeds, remove the TCP rule created by
# this attempt and stop the daemon while preserving the original error.
reset; MOCK_IPTABLES=42; MOCK_IPTABLES_FAIL_AT=2
rc=$(capture_status run start); status "$rc" 42
[ ! -f "$STATE_DIR/daemon" ] || fail "UDP rule failure left daemon running"
grep -Fq "iptables -D INPUT -i br0 -p tcp --dport 3517 -m comment --comment ralinkiappd-tcp -j ACCEPT" "$COMMAND_LOG" || fail "UDP failure did not roll back owned TCP rule"
[ "$(grep -c "^iptables -D " "$COMMAND_LOG")" -eq 1 ] || fail "unexpected firewall rollback rule count"
! grep -q "iptables -D INPUT -i br0 -p udp " "$COMMAND_LOG" || fail "removed UDP rule not owned by this start"

reset
IAPPD_PROC_MODULES="$tmp/missing-modules"
export IAPPD_PROC_MODULES
rc=$(capture_status run start); status "$rc" 2
! grep -q "^ralinkiappd " "$COMMAND_LOG" || fail "launched with unreadable module inventory"
! grep -q "^iptables -A " "$COMMAND_LOG" || fail "published firewall with unreadable module inventory"
grep -q "cannot inspect radio module inventory" "$COMMAND_LOG" || fail "module inventory failure was not logged"

reset
printf 'other_module\n' > "$tmp/modules"
IAPPD_PROC_MODULES="$tmp/modules"
IAPPD_PROC_INTERRUPTS="$tmp/missing-interrupts"
export IAPPD_PROC_MODULES IAPPD_PROC_INTERRUPTS
rc=$(capture_status run start); status "$rc" 2
! grep -q "^ralinkiappd " "$COMMAND_LOG" || fail "launched with unreadable interrupt inventory"
! grep -q "^iptables -A " "$COMMAND_LOG" || fail "published firewall with unreadable interrupt inventory"
grep -q "cannot inspect radio interrupt inventory" "$COMMAND_LOG" || fail "interrupt inventory failure was not logged"

# A complete inventory with neither radio marker preserves the documented
# legacy fallback; only an actual procfs read error is UNKNOWN.
reset
printf 'other_module\n' > "$tmp/modules"
printf 'eth0: 1\n' > "$tmp/interrupts"
IAPPD_PROC_MODULES="$tmp/modules"
IAPPD_PROC_INTERRUPTS="$tmp/interrupts"
export IAPPD_PROC_MODULES IAPPD_PROC_INTERRUPTS
rc=$(capture_status run start); status "$rc" 0
grep -Fq -- "-wi rax0 -wi ra0 -d 0" "$DAEMON_LOG" || fail "valid no-match inventory did not use fallback radio"
IAPPD_PROC_MODULES="$tmp/modules"
IAPPD_PROC_INTERRUPTS="$tmp/interrupts"
export IAPPD_PROC_MODULES IAPPD_PROC_INTERRUPTS

# Restore the normal radio fixture for the pre-existing lifecycle cases below.
printf "mt76x3_ap\n" > "$tmp/modules"
printf "rai0: 1\n" > "$tmp/interrupts"

reset
rc=$(capture_status run start); status "$rc" 0
[ -f "$STATE_DIR/daemon" ] || fail "start did not create daemon marker"
grep -Fq -- "-wi rai0 -d 0" "$DAEMON_LOG" || fail "wrong radio arguments"
[ "$(grep -c "^iptables -A " "$COMMAND_LOG")" -eq 2 ] || fail "missing firewall rules"
[ "$(grep -c "ralinkiappd-" "$RULES_FILE")" -eq 2 ] || fail "missing tagged firewall ownership rules"
count=$(grep -c "^ralinkiappd " "$COMMAND_LOG")
rc=$(capture_status run start); status "$rc" 0
[ "$(grep -c "^ralinkiappd " "$COMMAND_LOG")" -eq "$count" ] || fail "duplicate start"
rc=$(capture_status run stop); status "$rc" 0
[ ! -s "$RULES_FILE" ] || fail "normal stop left owned firewall rules"
[ "$(grep -c "^iptables -D " "$COMMAND_LOG")" -eq 2 ] || fail "normal stop did not delete both owned rules"

# An already-running daemon must repair a missing managed firewall rule rather
# than returning success while its required UDP listener is unreachable.
reset
mark_running
printf '%s\n' '-A INPUT -i br0 -p tcp --dport 3517 -m comment --comment ralinkiappd-tcp -j ACCEPT' > "$RULES_FILE"
rc=$(capture_status run start); status "$rc" 0
[ "$(grep -c "^ralinkiappd " "$COMMAND_LOG")" -eq 0 ] || fail "reconciled start relaunched the daemon"
[ "$(grep -c "^iptables -A " "$COMMAND_LOG")" -eq 1 ] || fail "reconciled start did not append exactly the missing UDP rule"
[ "$(grep -c "ralinkiappd-" "$RULES_FILE")" -eq 2 ] || fail "reconciled start did not restore both managed rules"

# If repair fails, preserve the existing daemon but never report start success.
reset
mark_running
printf '%s\n' '-A INPUT -i br0 -p tcp --dport 3517 -m comment --comment ralinkiappd-tcp -j ACCEPT' > "$RULES_FILE"
MOCK_IPTABLES=47; MOCK_IPTABLES_FAIL_AT=1
rc=$(capture_status run start); status "$rc" 47
[ -f "$STATE_DIR/daemon" ] || fail "firewall repair failure stopped the pre-existing daemon"
[ "$(grep -c "ralinkiappd-" "$RULES_FILE")" -eq 1 ] || fail "failed repair changed existing managed rules"
! grep -q "^ralinkiappd " "$COMMAND_LOG" || fail "firewall repair failure launched another daemon"

# A tagged rule with the wrong target is not accepted as a healthy firewall rule.
# Fail closed rather than treating a tagged DROP rule as an existing ACCEPT rule.
reset
mark_running
printf '%s\n' '-A INPUT -i br0 -p tcp --dport 3517 -m comment --comment ralinkiappd-tcp -j DROP' > "$RULES_FILE"
rc=$(capture_status run start); status "$rc" 1
[ -f "$STATE_DIR/daemon" ] || fail "conflicting tagged rule stopped the pre-existing daemon"
[ "$(grep -c "ralinkiappd-" "$RULES_FILE")" -eq 1 ] || fail "conflicting tagged rule was replaced or duplicated"
[ "$(grep -c "^iptables -A " "$COMMAND_LOG")" -eq 0 ] || fail "added a duplicate rule next to a conflicting tag"
! grep -q "^ralinkiappd " "$COMMAND_LOG" || fail "conflicting tagged rule relaunched the daemon"


# Exact rule identity rejects substring collisions in saved firewall entries.
reset
mark_running
printf '%s\n' '-A INPUT -i br0 -p tcp --dport 35170 -m comment --comment ralinkiappd-tcp-extra -j ACCEPT' > "$RULES_FILE"
rc=$(capture_status run start); status "$rc" 1
[ "$(grep -c "ralinkiappd-" "$RULES_FILE")" -eq 1 ] || fail "substring collision was modified"
[ "$(grep -c "^iptables -A " "$COMMAND_LOG")" -eq 0 ] || fail "added a rule beside conflicting substring tag"

# Duplicate exact owned entries are not a healthy firewall state.
reset
mark_running
printf '%s\n' '-A INPUT -i br0 -p tcp --dport 3517 -m comment --comment ralinkiappd-tcp -j ACCEPT' '-A INPUT -i br0 -p tcp --dport 3517 -m comment --comment ralinkiappd-tcp -j ACCEPT' > "$RULES_FILE"
rc=$(capture_status run start); status "$rc" 1
[ "$(grep -c "ralinkiappd-" "$RULES_FILE")" -eq 2 ] || fail "duplicate exact rules were modified"

# An xtables query error is UNKNOWN, not proof that a rule is absent.
reset
mark_running
MOCK_IPTABLES_CHECK=2
rc=$(capture_status run start); status "$rc" 2
[ -f "$STATE_DIR/daemon" ] || fail "iptables query error changed daemon state"
[ ! -s "$RULES_FILE" ] || fail "iptables query error changed firewall state"

# A stop with no daemon also reconciles stale IAPP-owned rules but preserves
# administrator rules with the same protocol/port tuple and no ownership tag.
reset
printf '%s\n' '-A INPUT -i br0 -p tcp --dport 3517 -j ACCEPT' '-A INPUT -i br0 -p tcp --dport 3517 -m comment --comment ralinkiappd-tcp -j ACCEPT' '-A INPUT -i br0 -p udp --dport 3517 -m comment --comment ralinkiappd-udp -j ACCEPT' > "$RULES_FILE"
rc=$(capture_status run stop); status "$rc" 0
[ "$(wc -l < "$RULES_FILE" | tr -d ' ')" -eq 1 ] || fail "stop removed an unowned rule or retained owned rules"
grep -Fq -- '-A INPUT -i br0 -p tcp --dport 3517 -j ACCEPT' "$RULES_FILE" || fail "untagged administrator rule was removed"

# Startup cleans a stale owned tag before publishing the current two rules.
reset
printf '%s\n' '-A INPUT -i br0 -p tcp --dport 3517 -m comment --comment ralinkiappd-tcp -j ACCEPT' > "$RULES_FILE"
rc=$(capture_status run start); status "$rc" 0
[ "$(grep -c "ralinkiappd-" "$RULES_FILE")" -eq 2 ] || fail "start failed to replace stale tagged rules"
[ "$(grep -c "^iptables -D " "$COMMAND_LOG")" -eq 1 ] || fail "start did not delete the stale tagged rule"

# Restart must not accumulate duplicate firewall rules.
reset; rc=$(capture_status run start); status "$rc" 0
rc=$(capture_status run restart); status "$rc" 0
[ "$(grep -c "ralinkiappd-" "$RULES_FILE")" -eq 2 ] || fail "restart accumulated duplicate tagged rules"

# A cleanup error is surfaced after the daemon is confirmed stopped. A later
# stop can retry cleanup without leaving a daemon behind.
reset; rc=$(capture_status run start); status "$rc" 0
MOCK_IPTABLES_DELETE=55
rc=$(capture_status run stop); status "$rc" 55
[ ! -f "$STATE_DIR/daemon" ] || fail "firewall cleanup failure left daemon running"
[ "$(grep -c "ralinkiappd-" "$RULES_FILE")" -eq 2 ] || fail "failed deletion did not preserve remaining rules"
MOCK_IPTABLES_DELETE=0
rc=$(capture_status run stop); status "$rc" 0
[ ! -s "$RULES_FILE" ] || fail "subsequent stop did not finish firewall cleanup"

reset; MOCK_IPTABLES=42; MOCK_IPTABLES_FAIL_AT=1
rc=$(capture_status run start); status "$rc" 42

reset; mark_running; MOCK_KILL=54
rc=$(capture_status run stop); status "$rc" 54
[ -f "$STATE_DIR/daemon" ] || fail "failed targeted signal lost daemon marker"

reset; mark_running; MOCK_REMOVE=0
rc=$(capture_status run stop); status "$rc" 1
[ -f "$STATE_DIR/daemon" ] || fail "stop postcondition false positive"

reset; mark_running
rc=$(capture_status run stop); status "$rc" 0
[ ! -f "$STATE_DIR/daemon" ] || fail "successful stop left daemon marker"

reset; mark_running; MOCK_KILL=54
rc=$(capture_status run restart); status "$rc" 54
! grep -q "^ralinkiappd " "$COMMAND_LOG" || fail "restart ran after failed stop"

reset
rc=$(capture_status run test); status "$rc" 1
mark_running
rc=$(capture_status run test); status "$rc" 0
rc=$(capture_status run invalid); status "$rc" 1
echo "Ralink IAPP daemon lifecycle regressions: PASS"
