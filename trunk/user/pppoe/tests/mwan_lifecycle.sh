#!/usr/bin/env bash
set -euo pipefail
ROOT=${1:?repository root required}
SCRIPT="$ROOT/trunk/user/pppoe/mwan.sh"
T=$(mktemp -d "${TMPDIR:-/tmp}/mwan-lifecycle.XXXXXX")
trap 'rm -rf "$T"' EXIT HUP INT TERM
BIN="$T/bin"; OPTIONS="$T/ppp"; SYS_NET="$T/sys/class/net"
LOG="$T/commands.log"; RUN_SCRIPT="$T/mwan-test.sh"
mkdir -p "$BIN" "$OPTIONS" "$SYS_NET/eth3"
printf 'aa:bb:cc:dd:ee:ff\n' > "$SYS_NET/eth3/address"
cat > "$BIN/nvram" <<'MOCK'
#!/bin/sh
[ "$1" = get ] || exit 90
[ "${FAIL_NVRAM:-}" != "$2" ] || exit 53
case "$2" in
 pppoesync_enable) value=${TEST_SYNC:-0} ;;
 pppoe_num) value=${TEST_PPP_NUM:-2} ;;
 vlan_filter) value=${TEST_VLAN_ENABLE:-0} ;;
 vlan_vid_cpu) value=${TEST_VLAN_ID:-35} ;;
 wan_pppoe_username) value=${TEST_USER:-normal-user} ;;
 wan_pppoe_passwd) value=${TEST_PASS:-normal-pass} ;;
 wan_pppoe_mtu) value=${TEST_MTU:-1492} ;;
 wan_pppoe_mru) value=${TEST_MRU:-1492} ;;
 *) exit 91 ;;
esac
printf '%s\n' "$value"
MOCK
cat > "$BIN/ip" <<'MOCK'
#!/bin/sh
printf 'ip %s\n' "$*" >> "$LOG"
if [ "$1" = link ] && [ "$2" = add ] && [ "${FAIL_IP_ADD:-}" = "$5" ]; then exit 31; fi
exit 0
MOCK
cat > "$BIN/ifconfig" <<'MOCK'
#!/bin/sh
printf 'ifconfig %s\n' "$*" >> "$LOG"
[ "${FAIL_IFCONFIG:-}" != "$1:$2" ] || exit 32
exit 0
MOCK
cat > "$BIN/pppd" <<'MOCK'
#!/bin/sh
printf 'pppd %s\n' "$*" >> "$LOG"
[ "$1" = file ] || exit 90
file=${2##*/}
[ "${FAIL_PPPD_FILE:-}" != "$file" ] || exit 42
exit 0
MOCK
cat > "$BIN/logger" <<'MOCK'
#!/bin/sh
exit 0
MOCK
chmod +x "$BIN"/*
sed -e "s|/tmp/ppp|$OPTIONS|g" -e "s|/sys/class/net|$SYS_NET|g" -e "s|/usr/sbin/pppd|$BIN/pppd|g" "$SCRIPT" > "$RUN_SCRIPT"
export PATH="$BIN:$PATH" LOG
reset() {
  rm -f "$OPTIONS"/options.wan* "$OPTIONS"/.options.wan*
  printf 'aa:bb:cc:dd:ee:ff\n' > "$SYS_NET/eth3/address"
  : > "$LOG"
  unset FAIL_NVRAM FAIL_IP_ADD FAIL_IFCONFIG FAIL_PPPD_FILE || true
  export TEST_SYNC=0 TEST_PPP_NUM=2 TEST_VLAN_ENABLE=0 TEST_VLAN_ID=35
  export TEST_USER=normal-user TEST_PASS=normal-pass TEST_MTU=1492 TEST_MRU=1492
}
run() { set +e; sh "$RUN_SCRIPT" --rc-lifecycle-locked; RC=$?; set -e; }
fail() { echo "FAIL: $*" >&2; exit 1; }

reset; run
[ "$RC" -eq 0 ] || fail "normal start status $RC"
[ -f "$OPTIONS/options.wan0" ] && [ -f "$OPTIONS/options.wan1" ] || fail "per-session files missing"
[ "$(stat -c '%a' "$OPTIONS/options.wan0")" = 600 ] || fail "options.wan0 mode not 0600"
[ "$(stat -c '%a' "$OPTIONS/options.wan1")" = 600 ] || fail "options.wan1 mode not 0600"
grep -Fqx "user 'normal-user'" "$OPTIONS/options.wan0" || fail "username quoting missing"
grep -Fqx "password 'normal-pass'" "$OPTIONS/options.wan0" || fail "password quoting missing"
[ "$(grep -c '^pppd file ' "$LOG")" -eq 2 ] || fail "expected two PPPD launches"
echo 'PPPoE-MWAN writes private per-session option files: PASS'

reset
printf 'aa:bb:cc:dd:ee:11\n' > "$SYS_NET/eth3/address"
run
[ "$RC" -eq 0 ] || fail "parent-MAC collision avoidance status $RC"
grep -Fqx 'ifconfig macvlan1 hw ether aa:bb:cc:dd:ee:12' "$LOG" || fail "first macvlan reused parent MAC"
grep -Fqx 'ifconfig macvlan2 hw ether aa:bb:cc:dd:ee:13' "$LOG" || fail "second macvlan suffix was not kept unique"
echo 'PPPoE-MWAN avoids reusing a parent MAC suffix: PASS'

reset
printf 'aa:bb:cc:dd:ee:10\n' > "$SYS_NET/eth3/address"
run
[ "$RC" -eq 0 ] || fail "reserved-suffix parent status $RC"
grep -Fqx 'ifconfig macvlan1 hw ether aa:bb:cc:dd:ee:11' "$LOG" || fail "reserved parent suffix changed the base sequence"
grep -Fqx 'ifconfig macvlan2 hw ether aa:bb:cc:dd:ee:12' "$LOG" || fail "reserved parent suffix caused duplicate allocation"
echo 'PPPoE-MWAN preserves the virtual MAC sequence for an unused parent suffix: PASS'

reset; export TEST_USER="edison'lab\wan" TEST_PASS="pass with\slash"; run
[ "$RC" -eq 0 ] || fail "quoted credential status $RC"
grep -Fqx "user 'edison\\'lab\\\\wan'" "$OPTIONS/options.wan0" || fail "username escaping incorrect"
grep -Fqx "password 'pass with\\\\slash'" "$OPTIONS/options.wan0" || fail "password escaping incorrect"
echo 'PPPoE-MWAN quotes PPP credentials without altering their value: PASS'

reset; export TEST_USER=$'invalid\nplugin bad'; run
[ "$RC" -ne 0 ] || fail "newline credential accepted"
! grep -q '^pppd file ' "$LOG" || fail "PPPD launched after newline rejection"
[ ! -e "$OPTIONS/options.wan0" ] || fail "options published after invalid credential"
echo 'PPPoE-MWAN rejects line-injected PPP credentials before launch: PASS'

reset; export TEST_PASS=$'invalid\tcontrol'; run
[ "$RC" -ne 0 ] || fail "control password accepted"
! grep -q '^pppd file ' "$LOG" || fail "PPPD launched after control rejection"
echo 'PPPoE-MWAN rejects control-character credentials: PASS'

reset; export FAIL_PPPD_FILE=options.wan0; run
[ "$RC" -eq 42 ] || fail "first PPPD failure masked ($RC)"
[ "$(grep -c '^pppd file ' "$LOG")" -eq 2 ] || fail "later PPPD was not attempted"
echo 'PPPoE-MWAN preserves first PPPD launch failure after later success: PASS'

reset; export FAIL_IP_ADD=macvlan1; run
[ "$RC" -eq 31 ] || fail "macvlan creation status $RC"
! grep -q 'ifconfig macvlan1 ' "$LOG" || fail "ifconfig ran after create failure"
! grep -q 'pppd file .*options.wan0' "$LOG" || fail "PPPD launched for uncreated macvlan"
grep -q 'pppd file .*options.wan1' "$LOG" || fail "second interface not attempted"
echo 'PPPoE-MWAN gates dependent setup and preserves interface failure: PASS'

reset; export FAIL_IFCONFIG=macvlan1:up; run
[ "$RC" -eq 32 ] || fail "ifconfig status $RC"
! grep -q 'pppd file .*options.wan0' "$LOG" || fail "PPPD launched after interface-up failure"
echo 'PPPoE-MWAN does not launch PPPD on a failed macvlan: PASS'

reset; export FAIL_NVRAM=wan_pppoe_passwd; run
[ "$RC" -eq 53 ] || fail "NVRAM status $RC"
! grep -q '^pppd file ' "$LOG" || fail "PPPD launched after NVRAM failure"
echo 'PPPoE-MWAN refuses launch when credential read fails: PASS'

reset; export TEST_PPP_NUM=not-a-number; run
[ "$RC" -eq 2 ] || fail "invalid PPP count status $RC"
! grep -q '^ip link add ' "$LOG" || fail "interface changed with invalid count"
echo 'PPPoE-MWAN validates PPP count before interface changes: PASS'

reset; export TEST_VLAN_ENABLE=1 TEST_VLAN_ID=../bad; run
[ "$RC" -eq 2 ] || fail "invalid VLAN status $RC"
! grep -q '^ip link add ' "$LOG" || fail "interface changed with invalid VLAN"
echo 'PPPoE-MWAN validates VLAN interface input: PASS'

reset; export TEST_SYNC=1 TEST_PPP_NUM=1; run
[ "$RC" -eq 0 ] || fail "sync PPP status $RC"
grep -Fqx 'syncppp 1' "$OPTIONS/options.wan0" || fail "syncppp option missing"
grep -Fqx 'linkname wan0' "$OPTIONS/options.wan0" || fail "linkname mismatch"
echo 'PPPoE-MWAN sync mode keeps per-session link identity: PASS'

reset
printf 'do not follow this symlink\n' > "$T/victim"
ln -s "$T/victim" "$OPTIONS/options.wan0"
run
[ "$RC" -ne 0 ] || fail "symlink option path accepted"
[ "$(cat "$T/victim")" = 'do not follow this symlink' ] || fail "symlink target modified"
! grep -q 'pppd file .*options.wan0' "$LOG" || fail "PPPD launched with symlink options"
echo 'PPPoE-MWAN refuses symlink credential files: PASS'

# mv into an existing directory succeeds but places credentials inside it.
# Refuse that path and continue independent PPP sessions without publishing.
reset
mkdir "$OPTIONS/options.wan0"
run
[ "$RC" -ne 0 ] || fail "directory option path accepted"
[ -d "$OPTIONS/options.wan0" ] || fail "non-regular option path was replaced"
[ -z "$(find "$OPTIONS/options.wan0" -mindepth 1 -print -quit)" ] || fail "staged credentials were moved inside the directory"
! grep -q 'pppd file .*options.wan0' "$LOG" || fail "PPPD launched with directory options path"
grep -q 'pppd file .*options.wan1' "$LOG" || fail "second PPP session did not continue after first path failure"
echo 'PPPoE-MWAN refuses non-regular credential paths: PASS'
