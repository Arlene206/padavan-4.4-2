#!/bin/bash
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
SCRIPT="$HERE/../detect.sh"
T=$(mktemp -d /tmp/mwan-test.XXXXXX)
BIN="$T/bin" LOG="$T/log" LOCK="$T/lock" PWNED="$T/pwned"
mkdir -p "$BIN"
trap 'rm -rf "$T"' EXIT HUP INT TERM
fail() { echo "PPPoE-MWAN regression: FAIL: $*" >&2; exit 1; }
cat > "$BIN/nvram" <<'MOCK'
#!/bin/sh
[ "$1" = get ] || exit 90
case "$2" in
 pppoe_num) echo "${PPP_NUM:-2}";;
 pppoemwan_443) echo 1;;
 pppoemwan_rules_x) echo 1;;
 pppoemwan_staticnum_x) echo 1;;
 pppoemwan_ip_x0) echo 203.0.113.7;;
 pppoemwan_interface_x0) echo 2;;
 *) exit 91;;
esac
MOCK
cat > "$BIN/ifconfig" <<'MOCK'
#!/bin/sh
case "$1" in
 br0) [ "${FAIL_BR0:-0}" != 1 ] || exit 32; echo 'br0 Link encap:Ethernet'; echo ' inet addr:192.168.50.1 Bcast:192.168.50.255 Mask:255.255.255.0';;
 ppp0) echo ' inet addr:198.51.100.2 P-t-P:198.51.100.1 Mask:255.255.255.255';;
 ppp1) [ "${PPP1_UP:-1}" = 1 ] || exit 1; echo ' inet addr:203.0.113.2 P-t-P:203.0.113.1 Mask:255.255.255.255';;
 *) exit 1;;
esac
MOCK
cat > "$BIN/iptables-save" <<'MOCK'
#!/bin/sh
[ "${FAIL_SAVE:-}" != "${2:-}" ] || exit 33
[ "${2:-}" != mangle ] || { echo '-A PREROUTING -i br0 -m comment --comment "remove br0" -j ACCEPT'; if [ "${MALICIOUS:-0}" = 1 ]; then printf '%s\n' "-A FORWARD -m comment --comment '\$(touch $PWNED)' -j ACCEPT"; fi; }
MOCK
cat > "$BIN/iptables" <<'MOCK'
#!/bin/sh
printf 'iptables' >> "$LOG"; for x in "$@"; do printf '\t%s' "$x" >> "$LOG"; done; echo >> "$LOG"
case " $* " in *" ${FAIL_IPTABLES:-__never__} "*) exit 37;; esac
MOCK
cat > "$BIN/ip" <<'MOCK'
#!/bin/sh
printf 'ip' >> "$LOG"; for x in "$@"; do printf '\t%s' "$x" >> "$LOG"; done; echo >> "$LOG"
case " $* " in *" ${FAIL_IP:-__never__} "*) exit 39;; esac
MOCK
cat > "$BIN/logger" <<'MOCK'
#!/bin/sh
exit 0
MOCK
cat > "$BIN/rm" <<'MOCK'
#!/bin/sh
case " $* " in *" $LOCK "*) [ "${FAIL_CLEANUP:-0}" != 1 ] || exit 41;; esac
exec /bin/rm "$@"
MOCK
chmod +x "$BIN"/*
export PATH="$BIN:$PATH" LOG LOCK PWNED MWAN_LOCKFILE="$LOCK"
reset() { /bin/rm -f "$LOCK" "$LOG" "$PWNED"; : > "$LOG"; unset FAIL_BR0 FAIL_SAVE FAIL_IPTABLES FAIL_IP FAIL_CLEANUP MALICIOUS PPP_NUM PPP1_UP; }
run() { set +e; bash "$SCRIPT"; RC=$?; set -e; }
reset; run; [ "$RC" = 0 ] || fail "normal run status $RC"; [ ! -e "$LOCK" ] || fail 'normal run leaked lock'
grep -Fq $'iptables\t-t\tmangle\t-D\tPREROUTING\t-i\tbr0\t-m\tcomment\t--comment\tremove br0\t-j\tACCEPT' "$LOG" || fail 'quoted iptables-save comment parsing'
grep -Fq $'ip\troute\treplace\tdefault\tnexthop\tvia\t198.51.100.2\tdev\tppp0\tweight\t1\tnexthop\tvia\t203.0.113.2\tdev\tppp1\tweight\t1' "$LOG" || fail 'multipath default route'
reset; export FAIL_SAVE=mangle; run; [ "$RC" = 33 ] || fail "iptables-save status $RC"; [ ! -e "$LOCK" ] || fail 'save failure leaked lock'
reset; export FAIL_IPTABLES='--dport 443'; run; [ "$RC" = 37 ] || fail "iptables failure status $RC"
reset; export FAIL_IP='route replace'; run; [ "$RC" = 39 ] || fail "route failure status $RC"; grep -Fq $'ip\troute\tflush\tcache' "$LOG" || fail 'cache flush skipped after route failure'
reset; export FAIL_BR0=1; run; [ "$RC" = 32 ] || fail "br0 failure status $RC"; [ ! -e "$LOCK" ] || fail 'br0 error leaked lock'
reset; export MALICIOUS=1; run; [ "$RC" = 0 ] || fail "quoted metacharacter status $RC"; [ ! -e "$PWNED" ] || fail 'iptables-save metacharacter executed'
reset; export FAIL_CLEANUP=1; run; [ "$RC" = 1 ] || fail "cleanup status $RC"; [ -e "$LOCK" ] || fail 'cleanup fixture removed lock'
reset; export FAIL_IPTABLES='--dport 443' FAIL_CLEANUP=1; run; [ "$RC" = 37 ] || fail "cleanup overwrote first error with $RC"
reset; export PPP1_UP=0; run; [ "$RC" = 0 ] || fail "missing ppp1 status $RC"; ! grep -Fq '203.0.113.2' "$LOG" || fail 'absent ppp1 appeared in route'
reset; export PPP_NUM=08; run; [ "$RC" = 0 ] || fail "decimal 08 status $RC"
reset; export PPP_NUM=999999999999999999999; run; [ "$RC" = 2 ] || fail "oversized PPP count status $RC"
reset; ! grep -Eq '(^|[[:space:]])eval[[:space:]]' "$SCRIPT" || fail 'unsafe eval remains'
! grep -Eq '^[[:space:]]*set[[:space:]]+-e' "$SCRIPT" || fail 'unexpected errexit mode'
echo 'PPPoE detect status and command-safety regression: PASS'
