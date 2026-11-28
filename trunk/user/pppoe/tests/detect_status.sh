#!/bin/bash
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
SCRIPT="$HERE/../detect.sh"
T=$(mktemp -d /tmp/mwan-test.XXXXXX)
BIN="$T/bin" LOG="$T/log" LOCK="$T/lock" PWNED="$T/pwned" SLEEP_LOG="$T/sleep.log" SYS_NET="$T/sys/class/net"
mkdir -p "$BIN" "$SYS_NET/ppp0" "$SYS_NET/ppp1"
trap 'rm -rf "$T"' EXIT HUP INT TERM
fail() { echo "PPPoE-MWAN regression: FAIL: $*" >&2; exit 1; }
cat > "$BIN/nvram" <<'MOCK'
#!/bin/sh
[ "$1" = get_checked ] || exit 90
[ "${FAIL_NVRAM:-}" != "$2" ] || exit 53
case "$2" in
 pppoe_num) echo "${PPP_NUM:-2}";;
 pppoemwan_443) echo 1;;
 pppoemwan_rules_x) echo 1;;
 pppoemwan_staticnum_x) echo 1;;
 pppoemwan_ip_x0) echo "${STATIC_IP:-203.0.113.7}";;
 pppoemwan_interface_x0) echo 2;;
 *) exit 91;;
esac
MOCK
cat > "$BIN/ifconfig" <<'MOCK'
#!/bin/sh
case "$1" in
 br0) [ "${FAIL_BR0:-0}" != 1 ] || exit 32; echo 'br0 Link encap:Ethernet'; echo " inet addr:192.168.50.1 Bcast:192.168.50.255 Mask:${LAN_MASK:-255.255.255.0}";;
 ppp0) echo "ifconfig ppp0" >> "$LOG"; [ "${FAIL_PPP0:-0}" != 1 ] || exit 44; echo ' inet addr:198.51.100.2 P-t-P:198.51.100.1 Mask:255.255.255.255';;
 ppp1) echo "ifconfig ppp1" >> "$LOG"; [ "${PPP1_UP:-1}" = 1 ] || exit 1; echo ' inet addr:203.0.113.2 P-t-P:203.0.113.1 Mask:255.255.255.255';;
 *) exit 1;;
esac
MOCK
cat > "$BIN/iptables-save" <<'MOCK'
#!/bin/sh
[ "${FAIL_SAVE:-}" != "${2:-}" ] || exit 33
if [ "${MALFORMED:-0}" = 1 ] && [ "${2:-}" = mangle ]; then
	printf '%s\n' '-A PREROUTING -i br0 -m comment --comment "unterminated br0'
	exit 0
fi
if [ "${2:-}" = mangle ]; then
	echo '-A PREROUTING -i br0 -m comment --comment "Padavan-PPPoE-MWAN" -j ACCEPT'
	echo '-A PREROUTING -m conntrack --ctstate NEW -j ACCEPT'
	echo '-A FORWARD -i br0 -j ACCEPT'
	echo '-A INPUT -m comment --comment "unrelated br0 rule" -j ACCEPT'
	if [ "${MALICIOUS:-0}" = 1 ]; then printf '%s\n' "-A FORWARD -m comment --comment '\$(touch $PWNED)' -j ACCEPT"; fi
fi
if [ "${2:-}" = nat ]; then
	echo '-A POSTROUTING -o ppp0 -j MASQUERADE'
	echo '-A PREROUTING -i ppp1 -j ACCEPT'
	echo '-A POSTROUTING -o ppp2 -m comment --comment "Padavan-PPPoE-MWAN" -j FULLCONENAT'
fi
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
cat > "$BIN/sleep" <<'MOCK'
#!/bin/sh
[ "$#" -eq 1 ] && [ "$1" = 1 ] || exit 92
printf '%s\n' "$1" >> "$SLEEP_LOG"
MOCK
cat > "$BIN/rm" <<'MOCK'
#!/bin/sh
case " $* " in *" $LOCK "*) [ "${FAIL_CLEANUP:-0}" != 1 ] || exit 41;; esac
exec /bin/rm "$@"
MOCK
chmod +x "$BIN"/*
export PATH="$BIN:$PATH" LOG LOCK PWNED SLEEP_LOG MWAN_LOCKFILE="$LOCK" MWAN_SYS_CLASS_NET="$SYS_NET"
reset() { /bin/rm -f "$LOCK" "$LOG" "$PWNED" "$SLEEP_LOG"; : > "$LOG"; : > "$SLEEP_LOG"; mkdir -p "$SYS_NET/ppp0" "$SYS_NET/ppp1"; unset FAIL_BR0 FAIL_SAVE FAIL_IPTABLES FAIL_IP FAIL_CLEANUP FAIL_PPP0 FAIL_NVRAM MALFORMED MALICIOUS PPP_NUM PPP1_UP STATIC_IP LAN_MASK; }
run() { set +e; bash "$SCRIPT"; RC=$?; set -e; }

assert_config_read_failure_preserves_policy() {
	if grep -Eq '^iptables[[:space:]]+-t[[:space:]]+(mangle|nat)[[:space:]]+(-D|-A)([[:space:]]|$)' "$LOG"; then
		fail 'firewall policy changed after an NVRAM preflight failure'
	fi
	if grep -Eq '^ip[[:space:]]+(rule[[:space:]]+add|route[[:space:]]+(add|replace))([[:space:]]|$)' "$LOG"; then
		fail 'policy/default routes changed after an NVRAM preflight failure'
	fi
	[ ! -e "$LOCK" ] || fail 'NVRAM preflight failure leaked the lock'
}

assert_cleanup_failure_stops_policy() {
	if grep -Eq '^iptables[[:space:]]+-t[[:space:]]+(mangle|nat)[[:space:]]+-A([[:space:]]|$)' "$LOG"; then
		fail 'new firewall rules were added after old-policy cleanup failed'
	fi
	if grep -Eq '^ip[[:space:]]+(rule[[:space:]]+add|route[[:space:]]+(add|replace))([[:space:]]|$)' "$LOG"; then
		fail 'policy/default routes were changed after old-policy cleanup failed'
	fi
	grep -Fq $'ip\troute\tflush\tcache' "$LOG" || fail 'route-cache cleanup was skipped'
}

reset; export FAIL_NVRAM=pppoe_num; run; [ "$RC" = 53 ] || fail "PPP count read status $RC"; assert_config_read_failure_preserves_policy
echo 'PPPoE-MWAN PPP count read failure preserves the old policy: PASS'
reset; export FAIL_NVRAM=pppoemwan_443; run; [ "$RC" = 53 ] || fail "443 selector read status $RC"; assert_config_read_failure_preserves_policy
echo 'PPPoE-MWAN 443 selector read failure preserves the old policy: PASS'
reset; export FAIL_NVRAM=pppoemwan_rules_x; run; [ "$RC" = 53 ] || fail "static-rule enable read status $RC"; assert_config_read_failure_preserves_policy
echo 'PPPoE-MWAN static-rule enable read failure preserves the old policy: PASS'
reset; export FAIL_NVRAM=pppoemwan_staticnum_x; run; [ "$RC" = 53 ] || fail "static-rule count read status $RC"; assert_config_read_failure_preserves_policy
echo 'PPPoE-MWAN static-rule count read failure preserves the old policy: PASS'
reset; export FAIL_NVRAM=pppoemwan_ip_x0; run; [ "$RC" = 53 ] || fail "static IP read status $RC"; assert_config_read_failure_preserves_policy
echo 'PPPoE-MWAN static-IP read failure preserves the old policy: PASS'

reset; export STATIC_IP=010.0.0.7; run
[ "$RC" = 2 ] || fail "octal-ambiguous static IPv4 status $RC"
assert_config_read_failure_preserves_policy
echo 'PPPoE-MWAN rejects octal-ambiguous static IPv4 before policy cleanup: PASS'
reset; export FAIL_NVRAM=pppoemwan_interface_x0; run; [ "$RC" = 53 ] || fail "static interface read status $RC"; assert_config_read_failure_preserves_policy
echo 'PPPoE-MWAN static-interface read failure preserves the old policy: PASS'

reset; export LAN_MASK=255.255.0.0; run
[ "$RC" = 0 ] || fail "configured /16 LAN netmask status $RC"
grep -Fq $'iptables\t-t\tmangle\t-A\tPREROUTING\t-m\tcomment\t--comment\tPadavan-PPPoE-MWAN\t-s\t192.168.50.1/16' "$LOG" || fail "mangle rule did not use the configured /16 LAN prefix"
grep -Fq $'iptables\t-t\tnat\t-A\tPOSTROUTING\t-m\tcomment\t--comment\tPadavan-PPPoE-MWAN\t-o\tppp0\t-s\t192.168.50.1/16' "$LOG" || fail "NAT rule did not use the configured /16 LAN prefix"
echo 'PPPoE-MWAN uses the configured /16 LAN subnet for mangle and NAT rules: PASS'

reset; export LAN_MASK=255.0.255.0; run
[ "$RC" = 2 ] || fail "non-contiguous LAN netmask status $RC"
assert_config_read_failure_preserves_policy
echo 'PPPoE-MWAN rejects non-contiguous bridge netmasks before policy cleanup: PASS'

reset; run; [ "$RC" = 0 ] || fail "normal run status $RC"; [ ! -e "$LOCK" ] || fail 'normal run leaked lock'
grep -Fq $'iptables\t-t\tmangle\t-D\tPREROUTING\t-i\tbr0\t-m\tcomment\t--comment\tPadavan-PPPoE-MWAN\t-j\tACCEPT' "$LOG" || fail 'exact owned mangle rule was not removed'
grep -Fq $'iptables\t-t\tnat\t-D\tPOSTROUTING\t-o\tppp2\t-m\tcomment\t--comment\tPadavan-PPPoE-MWAN\t-j\tFULLCONENAT' "$LOG" || fail 'exact owned NAT rule was not removed'
! grep -Fq $'iptables\t-t\tmangle\t-D\tPREROUTING\t-m\tconntrack\t--ctstate\tNEW' "$LOG" || fail 'unrelated conntrack rule was removed'
! grep -Fq $'iptables\t-t\tmangle\t-D\tFORWARD\t-i\tbr0\t-j\tACCEPT' "$LOG" || fail 'unrelated bridge rule was removed'
! grep -Fq $'iptables\t-t\tmangle\t-D\tINPUT\t-m\tcomment\t--comment\tunrelated br0 rule\t-j\tACCEPT' "$LOG" || fail 'unrelated comment rule was removed'
! grep -Fq $'iptables\t-t\tnat\t-D\tPOSTROUTING\t-o\tppp0\t-j\tMASQUERADE' "$LOG" || fail 'primary PPP masquerade rule was removed'
! grep -Fq $'iptables\t-t\tnat\t-D\tPREROUTING\t-i\tppp1\t-j\tACCEPT' "$LOG" || fail 'unrelated PPP NAT rule was removed'
grep -Fq $'ip\troute\treplace\tdefault\tnexthop\tvia\t198.51.100.2\tdev\tppp0\tweight\t1\tnexthop\tvia\t203.0.113.2\tdev\tppp1\tweight\t1' "$LOG" || fail 'multipath default route'
echo 'PPPoE-MWAN cleanup deletes only rules with the exact owner comment: PASS'
reset; export FAIL_SAVE=mangle; run; [ "$RC" = 33 ] || fail "iptables-save status $RC"; [ ! -e "$LOCK" ] || fail 'save failure leaked lock'; assert_cleanup_failure_stops_policy
echo 'PPPoE-MWAN mangle snapshot failure blocks new policy: PASS'
reset; export FAIL_SAVE=nat; run; [ "$RC" = 33 ] || fail "NAT snapshot status $RC"; assert_cleanup_failure_stops_policy
echo 'PPPoE-MWAN NAT snapshot failure blocks new policy: PASS'
reset; export FAIL_IPTABLES='-D'; run; [ "$RC" = 37 ] || fail "old-rule deletion status $RC"; assert_cleanup_failure_stops_policy
echo 'PPPoE-MWAN old-rule deletion failure blocks new policy: PASS'
reset; export MALFORMED=1; run; [ "$RC" = 2 ] || fail "malformed saved rule status $RC"; assert_cleanup_failure_stops_policy
echo 'PPPoE-MWAN malformed saved rule blocks new policy: PASS'
reset; export FAIL_IPTABLES='--dport 443'; run; [ "$RC" = 37 ] || fail "iptables failure status $RC"
reset; export FAIL_IP='route replace'; run; [ "$RC" = 39 ] || fail "route failure status $RC"; grep -Fq $'ip\troute\tflush\tcache' "$LOG" || fail 'cache flush skipped after route failure'
reset; export FAIL_BR0=1; run; [ "$RC" = 32 ] || fail "br0 failure status $RC"; [ ! -e "$LOCK" ] || fail 'br0 error leaked lock'
reset; export MALICIOUS=1; run; [ "$RC" = 0 ] || fail "quoted metacharacter status $RC"; [ ! -e "$PWNED" ] || fail 'iptables-save metacharacter executed'
reset; export FAIL_CLEANUP=1; run; [ "$RC" = 1 ] || fail "cleanup status $RC"; [ -e "$LOCK" ] || fail 'cleanup fixture removed lock'
reset; export FAIL_IPTABLES='--dport 443' FAIL_CLEANUP=1; run; [ "$RC" = 37 ] || fail "cleanup overwrote first error with $RC"
reset; export FAIL_PPP0=1; run; [ "$RC" = 44 ] || fail "existing ppp0 ifconfig failure status $RC"; grep -Fq '203.0.113.2' "$LOG" || fail 'ppp1 reconciliation stopped after ppp0 probe failure'; ! grep -Fq '198.51.100.2' "$LOG" || fail 'failed ppp0 probe was used in policy'; echo 'PPPoE-MWAN reports ifconfig errors on existing PPP interfaces and continues independent peers: PASS'
reset; /bin/rm -rf "$SYS_NET/ppp1"; run; [ "$RC" = 0 ] || fail "missing ppp1 status $RC"; ! grep -Fq '203.0.113.2' "$LOG" || fail 'absent ppp1 appeared in route'; ! grep -Fq 'ifconfig ppp1' "$LOG" || fail 'ifconfig ran for an interface absent from sysfs'
reset; export PPP_NUM=08; run; [ "$RC" = 0 ] || fail "decimal 08 status $RC"
reset; export PPP_NUM=999999999999999999999; run; [ "$RC" = 2 ] || fail "oversized PPP count status $RC"
reset
printf 'foreign lock owner\n' > "$LOCK"
run
[ "$RC" = 1 ] || fail "contended lock timeout status $RC"
[ "$(wc -l < "$SLEEP_LOG" | tr -d ' ')" -eq 30 ] || fail "lock wait was not bounded to 30 seconds"
[ "$(cat "$LOCK")" = 'foreign lock owner' ] || fail 'timed-out waiter removed or modified a foreign lock'
[ ! -s "$LOG" ] || fail 'commands ran without acquiring the shared lock'
echo 'PPPoE-MWAN lock contention is bounded and foreign lock preserved: PASS'
reset; ! grep -Eq '(^|[[:space:]])eval[[:space:]]' "$SCRIPT" || fail 'unsafe eval remains'
! grep -Eq '^[[:space:]]*set[[:space:]]+-e' "$SCRIPT" || fail 'unexpected errexit mode'
echo 'PPPoE detect status and command-safety regression: PASS'
