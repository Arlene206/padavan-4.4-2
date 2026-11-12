#!/bin/sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname "$0")/../../../.." && pwd)
SCRIPT="$ROOT/trunk/user/adguardhome/adguardhome.sh"
TEST_TMP=$(mktemp -d "${TMPDIR:-/tmp}/adguardhome-config.XXXXXX")
TEST_CONFIG="$TEST_TMP/adg.sh"
TEST_SIDE_EFFECTS="$TEST_TMP/start-side-effects.log"
export ADG_CONFIG_FILE="$TEST_CONFIG"
trap 'rm -rf "$TEST_TMP"' EXIT HUP INT TERM

fail() {
	echo "AdGuardHome config security regression: FAIL: $*" >&2
	exit 1
}

mode_of() {
	stat -c '%a' "$1"
}

# Source the real production function without executing a service action.
set -- __test__
. "$SCRIPT" >/dev/null 2>&1

# New default config must be private even under a permissive caller umask.
OLD_UMASK=$(umask)
umask 000
getconfig || fail "default config generation failed"
umask "$OLD_UMASK"
[ -f "$TEST_CONFIG" ] || fail "default config was not published"
[ "$(mode_of "$TEST_CONFIG")" = 600 ] || fail "generated config is not mode 0600"
grep -Fq 'auth_pass: adguardhome' "$TEST_CONFIG" || fail "default UI credential missing"
grep -Fq 'private_key: ""' "$TEST_CONFIG" || fail "TLS private-key field missing"

# Existing operator config content is preserved while world-readable mode is repaired.
printf '%s\n' 'auth_name: operator' 'auth_pass: keep-this-secret' > "$TEST_CONFIG"
chmod 644 "$TEST_CONFIG"
cp "$TEST_CONFIG" "$TEST_TMP/expected.yml"
getconfig || fail "existing config permission repair failed"
cmp "$TEST_TMP/expected.yml" "$TEST_CONFIG" || fail "existing config content changed"
[ "$(mode_of "$TEST_CONFIG")" = 600 ] || fail "existing config was left world-readable"

# Never follow a symlink at the privileged config path.
printf '%s\n' 'outside-secret: unchanged' > "$TEST_TMP/outside.yml"
chmod 600 "$TEST_TMP/outside.yml"
ln -s "$TEST_TMP/outside.yml" "$TEST_TMP/config-link"
ADG_CONFIG_FILE="$TEST_TMP/config-link"
export ADG_CONFIG_FILE
if getconfig; then
	fail "symlink config destination was accepted"
fi
[ -L "$ADG_CONFIG_FILE" ] || fail "config symlink was replaced or removed"
grep -Fqx 'outside-secret: unchanged' "$TEST_TMP/outside.yml" || fail "symlink target was modified"
[ "$(mode_of "$TEST_TMP/outside.yml")" = 600 ] || fail "symlink target permissions were modified"

# Reject a FIFO instead of blocking on a privileged redirection.
mkfifo "$TEST_TMP/config-fifo"
ADG_CONFIG_FILE="$TEST_TMP/config-fifo"
export ADG_CONFIG_FILE
if getconfig; then
	fail "FIFO config destination was accepted"
fi
[ -p "$ADG_CONFIG_FILE" ] || fail "FIFO config destination was changed"

# A failed atomic rename must leave no partial destination or staged secret.
(
	ADG_CONFIG_FILE="$TEST_TMP/rename-fail.yml"
	export ADG_CONFIG_FILE
	mv() { return 1; }
	set -- __test__
	. "$SCRIPT" >/dev/null 2>&1
	if getconfig; then
		exit 1
	fi
	[ ! -e "$ADG_CONFIG_FILE" ] || exit 1
	set -- "$ADG_CONFIG_FILE.tmp."*
	[ ! -e "$1" ] || exit 1
) || fail "rename failure did not clean the private stage"

# A failed chmod must not publish the staged default file.
(
	ADG_CONFIG_FILE="$TEST_TMP/chmod-fail.yml"
	export ADG_CONFIG_FILE
	chmod() { return 1; }
	set -- __test__
	. "$SCRIPT" >/dev/null 2>&1
	if getconfig; then
		exit 1
	fi
	[ ! -e "$ADG_CONFIG_FILE" ] || exit 1
	set -- "$ADG_CONFIG_FILE.tmp."*
	[ ! -e "$1" ] || exit 1
) || fail "chmod failure published a config or leaked its stage"

# If safe config preparation fails, startup must not touch DNS/firewall state.
# Keep the side-effect mocks scoped so the later firewall regression sources the
# real set_iptable()/clear_iptable() functions.
(
	ADG_CONFIG_FILE="$TEST_TMP/config-link"
	export ADG_CONFIG_FILE
	change_dns() { echo change_dns >> "$TEST_SIDE_EFFECTS"; }
	set_iptable() { echo set_iptable >> "$TEST_SIDE_EFFECTS"; }
	logger() { return 0; }
	if start_adg; then
		exit 1
	fi
	[ ! -s "$TEST_SIDE_EFFECTS" ] || exit 1
) || fail "startup changed DNS/firewall before config validation"

echo "AdGuardHome secret config mode, symlink rejection and atomic publication: PASS"

# Verify resolver edits are staged, checked and rolled back byte-for-byte when
# dnsmasq rejects the new configuration.
(
  TEST_ADG_DNS_CONFIG="$TEST_TMP/dnsmasq.conf"
  TEST_DNS_RESTART_CALLS=0
  TEST_DNS_RESTART_MODE=success
  adg_dnsmasq_config_path() { echo "$TEST_ADG_DNS_CONFIG"; }
  adg_restart_dhcpd() {
    TEST_DNS_RESTART_CALLS=$((TEST_DNS_RESTART_CALLS + 1))
    case "$TEST_DNS_RESTART_MODE:$TEST_DNS_RESTART_CALLS" in
      fail-first:1|fail-all:*)
        return 1
        ;;
      *)
        return 0
        ;;
    esac
  }
  nvram() {
    [ "$1" = get ] && [ "$2" = adg_redirect ] || return 1
    echo 1
  }

  printf '%s\n' 'domain-needed' 'server=9.9.9.9' 'no-resolv' 'server=127.0.0.1#5353' > "$TEST_ADG_DNS_CONFIG"
  if ! change_dns; then exit 1; fi
  [ "$(grep -Fxc 'no-resolv' "$TEST_ADG_DNS_CONFIG")" = 1 ] || exit 1
  [ "$(grep -Fxc 'server=127.0.0.1#5335' "$TEST_ADG_DNS_CONFIG")" = 1 ] || exit 1
  ! grep -Fqx 'server=127.0.0.1#5353' "$TEST_ADG_DNS_CONFIG" || exit 1
  [ "$TEST_DNS_RESTART_CALLS" = 1 ] || exit 1

  TEST_DNS_RESTART_CALLS=0
  TEST_DNS_RESTART_MODE=success
  if ! del_dns; then exit 1; fi
  ! grep -Fqx 'no-resolv' "$TEST_ADG_DNS_CONFIG" || exit 1
  ! grep -Fqx 'server=127.0.0.1#5335' "$TEST_ADG_DNS_CONFIG" || exit 1
  [ "$TEST_DNS_RESTART_CALLS" = 1 ] || exit 1

  # A failed enable reload must restore the exact previous resolver bytes and
  # make a second reload attempt with the restored configuration.
  printf '%s\n' 'domain-needed' 'no-resolv' 'server=8.8.8.8' > "$TEST_ADG_DNS_CONFIG"
  cp "$TEST_ADG_DNS_CONFIG" "$TEST_TMP/dns-before-enable-failure"
  TEST_DNS_RESTART_CALLS=0
  TEST_DNS_RESTART_MODE=fail-first
  if change_dns; then exit 1; fi
  cmp "$TEST_TMP/dns-before-enable-failure" "$TEST_ADG_DNS_CONFIG" || exit 1
  [ "$TEST_DNS_RESTART_CALLS" = 2 ] || exit 1

  # Failed disable reload likewise restores the active AdGuard resolver.
  printf '%s\n' 'domain-needed' 'no-resolv' 'server=127.0.0.1#5335' > "$TEST_ADG_DNS_CONFIG"
  cp "$TEST_ADG_DNS_CONFIG" "$TEST_TMP/dns-before-disable-failure"
  TEST_DNS_RESTART_CALLS=0
  TEST_DNS_RESTART_MODE=fail-first
  if del_dns; then exit 1; fi
  cmp "$TEST_TMP/dns-before-disable-failure" "$TEST_ADG_DNS_CONFIG" || exit 1
  [ "$TEST_DNS_RESTART_CALLS" = 2 ] || exit 1
) || fail "dnsmasq resolver update did not preserve the previous config on failure"

echo "AdGuardHome dnsmasq resolver publication, status and rollback: PASS"

# Model the v4/v6 PREROUTING rule set without requiring host netfilter privileges.
TEST_ADG_IPV4_RULES="$TEST_TMP/iptables-v4.rules"
TEST_ADG_IPV6_RULES="$TEST_TMP/iptables-v6.rules"
TEST_ADG_RULE_LOG="$TEST_TMP/iptables.log"
TEST_ADG_REDIRECT_MODE=2
TEST_ADG_IFCONFIG_CALLS=0
TEST_ADG_FAIL_ADD=
TEST_ADG_FAIL_CHECK=
TEST_ADG_IPV6_LIST_CALLS=0
TEST_ADG_IPV6_LIST_FAIL_ON=0
TEST_ADG_V4_TCP_RULE='PREROUTING -p tcp -m addrtype --dst-type LOCAL ! -d 127.0.0.0/8 --dport 53 -j REDIRECT --to-ports 5335'
TEST_ADG_V4_UDP_RULE='PREROUTING -p udp -m addrtype --dst-type LOCAL ! -d 127.0.0.0/8 --dport 53 -j REDIRECT --to-ports 5335'
TEST_ADG_V6_TCP_RULE='PREROUTING -p tcp -m addrtype --dst-type LOCAL ! -d ::1/128 ! -d fe80::/10 --dport 53 -j REDIRECT --to-ports 5335'
TEST_ADG_V6_UDP_RULE='PREROUTING -p udp -m addrtype --dst-type LOCAL ! -d ::1/128 ! -d fe80::/10 --dport 53 -j REDIRECT --to-ports 5335'
: > "$TEST_ADG_IPV4_RULES"
: > "$TEST_ADG_IPV6_RULES"
: > "$TEST_ADG_RULE_LOG"

nvram() {
	[ "$1" = get ] && [ "$2" = adg_redirect ] || return 1
	echo "$TEST_ADG_REDIRECT_MODE"
}

ifconfig() {
	TEST_ADG_IFCONFIG_CALLS=$((TEST_ADG_IFCONFIG_CALLS + 1))
	echo "inet addr:$TEST_ADG_IFCONFIG_ADDR"
}

logger() {
	return 0
}

adg_test_iptables() {
	adg_family="$1"
	shift
	case "$adg_family" in
		ipv4) adg_state="$TEST_ADG_IPV4_RULES" ;;
		ipv6) adg_state="$TEST_ADG_IPV6_RULES" ;;
		*) return 1 ;;
	esac

	printf '%s %s\n' "$adg_family" "$*" >> "$TEST_ADG_RULE_LOG"
	[ "$1" = -t ] && [ "$2" = nat ] || return 1
	shift 2
	adg_operation="$1"
	shift
	[ "$1" = PREROUTING ] || return 1
	adg_rule="$1"
	shift
	for adg_arg in "$@"; do
		adg_rule="$adg_rule $adg_arg"
	done

	case "$adg_operation" in
		-L)
			return 0
			;;
		-A)
			if [ -n "$TEST_ADG_FAIL_ADD" ] && [ "$adg_family:$adg_rule" = "$TEST_ADG_FAIL_ADD" ]; then
				return 1
			fi
			printf '%s\n' "$adg_rule" >> "$adg_state"
			return 0
			;;
		-C)
			if [ -n "$TEST_ADG_FAIL_CHECK" ] && [ "$adg_family:$adg_rule" = "$TEST_ADG_FAIL_CHECK" ]; then
				return 2
			fi
			grep -Fqx "$adg_rule" "$adg_state"
			return $?
			;;
		-D)
			adg_tmp="$adg_state.tmp"
			if awk -v wanted="$adg_rule" '
				!removed && $0 == wanted { removed = 1; next }
				{ print }
				END { if (!removed) exit 1 }
			' "$adg_state" > "$adg_tmp"; then
				mv "$adg_tmp" "$adg_state"
				return 0
			fi
			rm -f "$adg_tmp"
			return 1
			;;
		*)
			return 1
			;;
	esac
}

iptables() {
	adg_test_iptables ipv4 "$@"
}

ip6tables() {
	if [ "$1" = -t ] && [ "$2" = nat ] && [ "$3" = -L ]; then
		TEST_ADG_IPV6_LIST_CALLS=$((TEST_ADG_IPV6_LIST_CALLS + 1))
		if [ "$TEST_ADG_IPV6_LIST_FAIL_ON" = "$TEST_ADG_IPV6_LIST_CALLS" ]; then
			return 1
		fi
	fi
	adg_test_iptables ipv6 "$@"
}

if ! set_iptable; then
	fail "initial address-independent IPv4/IPv6 redirect setup failed"
fi
[ "$(wc -l < "$TEST_ADG_IPV4_RULES" | tr -d ' ')" = 2 ] || fail "IPv4 redirect setup did not publish exactly two protocol rules (mode=$(nvram get adg_redirect); calls=$(cat "$TEST_ADG_RULE_LOG"); state=$(cat "$TEST_ADG_IPV4_RULES"))"
[ "$(wc -l < "$TEST_ADG_IPV6_RULES" | tr -d ' ')" = 2 ] || fail "IPv6 redirect setup did not publish exactly two protocol rules"
grep -Fqx "$TEST_ADG_V4_TCP_RULE" "$TEST_ADG_IPV4_RULES" || fail "IPv4 TCP rule is not route-derived LOCAL"
grep -Fqx "$TEST_ADG_V4_UDP_RULE" "$TEST_ADG_IPV4_RULES" || fail "IPv4 UDP rule is not route-derived LOCAL"
grep -Fqx "$TEST_ADG_V6_TCP_RULE" "$TEST_ADG_IPV6_RULES" || fail "IPv6 TCP rule is not route-derived LOCAL"
grep -Fqx "$TEST_ADG_V6_UDP_RULE" "$TEST_ADG_IPV6_RULES" || fail "IPv6 UDP rule is not route-derived LOCAL"

# Seed duplicates as if an earlier daemon restart had appended a second copy.
cp "$TEST_ADG_IPV4_RULES" "$TEST_TMP/iptables-v4.duplicate"
cp "$TEST_ADG_IPV6_RULES" "$TEST_TMP/iptables-v6.duplicate"
cat "$TEST_TMP/iptables-v4.duplicate" >> "$TEST_ADG_IPV4_RULES"
cat "$TEST_TMP/iptables-v6.duplicate" >> "$TEST_ADG_IPV6_RULES"
TEST_ADG_IFCONFIG_ADDR=198.51.100.2
if ! set_iptable; then
	fail "repeated redirect setup failed after simulated interface address change"
fi
[ "$TEST_ADG_IFCONFIG_CALLS" = 0 ] || fail "redirect rules still depend on enumerated interface address snapshots"
[ "$(wc -l < "$TEST_ADG_IPV4_RULES" | tr -d ' ')" = 2 ] || fail "repeated start left duplicate IPv4 redirect rules"
[ "$(wc -l < "$TEST_ADG_IPV6_RULES" | tr -d ' ')" = 2 ] || fail "repeated start left duplicate IPv6 redirect rules"

# The baseline rule plus 64 additional copies makes 65 total.
# One bounded cleanup pass must remove 64 and leave exactly one for retry.
adg_test_duplicate_count=0
while [ "$adg_test_duplicate_count" -lt 64 ]; do
	printf '%s\n' "$TEST_ADG_V4_TCP_RULE" >> "$TEST_ADG_IPV4_RULES"
	adg_test_duplicate_count=$((adg_test_duplicate_count + 1))
done
if clear_iptable; then
	fail "redirect cleanup ignored the 64-rule safety bound"
fi
[ "$(grep -Fxc "$TEST_ADG_V4_TCP_RULE" "$TEST_ADG_IPV4_RULES")" = 1 ] || fail "bounded cleanup did not leave exactly the unprocessed 65th duplicate"
clear_iptable || fail "bounded duplicate cleanup could not resume"
[ ! -s "$TEST_ADG_IPV4_RULES" ] && [ ! -s "$TEST_ADG_IPV6_RULES" ] || fail "bounded duplicate cleanup left residual rules"

# An xtables check error is not the same as an absent exact rule.
if ! set_iptable; then
	fail "redirect setup before xtables check failure failed"
fi
TEST_ADG_FAIL_CHECK="ipv4:$TEST_ADG_V4_TCP_RULE"
if clear_iptable; then
	fail "cleanup treated an xtables -C error as an absent rule"
fi
[ "$(grep -Fxc "$TEST_ADG_V4_TCP_RULE" "$TEST_ADG_IPV4_RULES")" = 1 ] || fail "failed xtables check deleted the rule without confirmation"
TEST_ADG_FAIL_CHECK=
clear_iptable || fail "cleanup did not recover after xtables check error"

# A present but unreadable IPv6 nat table must fail setup and roll IPv4 back.
TEST_ADG_IPV6_LIST_CALLS=0
TEST_ADG_IPV6_LIST_FAIL_ON=2
if set_iptable; then
	fail "set_iptable ignored an IPv6 NAT table probe failure"
fi
[ ! -s "$TEST_ADG_IPV4_RULES" ] && [ ! -s "$TEST_ADG_IPV6_RULES" ] || fail "failed IPv6 table probe left IPv4 startup redirects behind"
TEST_ADG_IPV6_LIST_FAIL_ON=0

# Cleanup must not treat an IPv6 nat table probe error as an IPv6-disabled profile.
if ! set_iptable; then
	fail "redirect setup before IPv6 cleanup probe failure failed"
fi
TEST_ADG_IPV6_LIST_CALLS=0
TEST_ADG_IPV6_LIST_FAIL_ON=1
if clear_iptable; then
	fail "clear_iptable ignored an IPv6 NAT table probe failure"
fi
[ ! -s "$TEST_ADG_IPV4_RULES" ] || fail "IPv4 cleanup failed before reporting IPv6 probe error"
[ "$(wc -l < "$TEST_ADG_IPV6_RULES" | tr -d ' ')" = 2 ] || fail "failed IPv6 cleanup probe removed or falsely cleared IPv6 redirect rules"
TEST_ADG_IPV6_LIST_FAIL_ON=0
clear_iptable || fail "IPv6 cleanup did not recover after table probe failure"

# Disabling redirect mode must remove all exact rules and publish none.
TEST_ADG_REDIRECT_MODE=1
if ! set_iptable; then
	fail "redirect-mode transition cleanup failed"
fi
[ ! -s "$TEST_ADG_IPV4_RULES" ] || fail "IPv4 redirect rules survived redirect-mode disable"
[ ! -s "$TEST_ADG_IPV6_RULES" ] || fail "IPv6 redirect rules survived redirect-mode disable"
clear_iptable || fail "repeated empty redirect cleanup failed"
[ ! -s "$TEST_ADG_IPV4_RULES" ] && [ ! -s "$TEST_ADG_IPV6_RULES" ] || fail "idempotent redirect cleanup recreated rules"

# A failed IPv6 UDP append must roll back rules already published for both families.
TEST_ADG_REDIRECT_MODE=2
TEST_ADG_FAIL_ADD="ipv6:$TEST_ADG_V6_UDP_RULE"
if set_iptable; then
	fail "redirect setup succeeded after injected IPv6 UDP rule failure"
fi
[ ! -s "$TEST_ADG_IPV4_RULES" ] && [ ! -s "$TEST_ADG_IPV6_RULES" ] || fail "failed IPv6 publication did not roll back IPv4/IPv6 rules"
TEST_ADG_FAIL_ADD=
echo "AdGuardHome address-independent IPv4/IPv6 DNS redirects, duplicate cleanup and rollback: PASS"

# A firewall setup error must block resolver mutation and daemon launch.
(
	TEST_ADG_START_EFFECTS="$TEST_TMP/start-firewall-failure.log"
	: > "$TEST_ADG_START_EFFECTS"
	pidof() { return 1; }
	getconfig() { adg_file="$TEST_CONFIG"; return 0; }
	mkdir() { return 0; }
	set_iptable() { printf 'set_iptable\n' >> "$TEST_ADG_START_EFFECTS"; return 1; }
	change_dns() { printf 'change_dns\n' >> "$TEST_ADG_START_EFFECTS"; }
	AdGuardHome() { printf 'launch\n' >> "$TEST_ADG_START_EFFECTS"; }
	if start_adg; then
		fail "start_adg ignored a firewall setup failure"
	fi
	[ "$(cat "$TEST_ADG_START_EFFECTS")" = set_iptable ] || fail "failed firewall setup changed DNS or launched the daemon"
) || fail "start firewall fail-closed regression failed"

# A resolver publication/reload failure must roll firewall changes back and
# prevent daemon launch.
(
  TEST_ADG_START_EFFECTS="$TEST_TMP/start-dns-failure.log"
  : > "$TEST_ADG_START_EFFECTS"
  pidof() { return 1; }
  getconfig() { return 0; }
  mkdir() { return 0; }
  set_iptable() { echo set_iptable >> "$TEST_ADG_START_EFFECTS"; return 0; }
  change_dns() { echo change_dns >> "$TEST_ADG_START_EFFECTS"; return 1; }
  clear_iptable() { echo clear_iptable >> "$TEST_ADG_START_EFFECTS"; return 0; }
  AdGuardHome() { echo launch >> "$TEST_ADG_START_EFFECTS"; }
  logger() { return 0; }
  if start_adg; then exit 1; fi
  [ "$(cat "$TEST_ADG_START_EFFECTS")" = "$(printf 'set_iptable\\nchange_dns\\nclear_iptable')" ] || exit 1
) || fail "start continued after resolver reload failure"

# Exercise daemon stop/start gates with a process-name-only mock.
TEST_ADG_STATE="$TEST_TMP/adguardhome.state"
TEST_ADG_KILL_LOG="$TEST_TMP/adguardhome.kill.log"
TEST_ADG_SLEEP_LOG="$TEST_TMP/adguardhome.sleep.log"
TEST_ADG_CLEANUP_LOG="$TEST_TMP/adguardhome.cleanup.log"
TEST_ADG_EFFECTS_LOG="$TEST_TMP/adguardhome.start-effects.log"
export TEST_ADG_STATE TEST_ADG_KILL_LOG TEST_ADG_SLEEP_LOG
export TEST_ADG_CLEANUP_LOG TEST_ADG_EFFECTS_LOG

pidof() {
	[ "$1" = AdGuardHome ] || return 1
	if [ -f "$TEST_ADG_STATE" ] && [ "$(cat "$TEST_ADG_STATE")" = running ]; then
		echo 24680
		return 0
	fi
	return 1
}

killall() {
	printf '%s\n' "$*" >> "$TEST_ADG_KILL_LOG"
	case "$TEST_ADG_MODE:$*" in
		graceful:AdGuardHome|-9-exits:-9\ AdGuardHome)
			echo stopped > "$TEST_ADG_STATE"
			;;
		*)
			;;
	esac
	return 0
}

sleep() {
	printf '%s\n' "$*" >> "$TEST_ADG_SLEEP_LOG"
	return 0
}

rm() {
	case "$*" in
		"-rf /tmp/AdGuardHome")
			printf 'rm:%s\n' "$*" >> "$TEST_ADG_CLEANUP_LOG"
			return 0
			;;
		*)
			/bin/rm "$@"
			;;
	esac
}

del_dns() {
	echo del_dns >> "$TEST_ADG_CLEANUP_LOG"
	[ "${TEST_ADG_DEL_DNS_FAIL:-0}" = 1 ] && return 1
	return 0
}

clear_iptable() {
	echo clear_iptable >> "$TEST_ADG_CLEANUP_LOG"
	[ "${TEST_ADG_CLEAR_FAIL:-0}" = 1 ] && return 1
	return 0
}

: > "$TEST_ADG_KILL_LOG"
: > "$TEST_ADG_SLEEP_LOG"
: > "$TEST_ADG_CLEANUP_LOG"
: > "$TEST_ADG_EFFECTS_LOG"
echo running > "$TEST_ADG_STATE"
getconfig() { echo getconfig >> "$TEST_ADG_EFFECTS_LOG"; return 0; }
change_dns() { echo change_dns >> "$TEST_ADG_EFFECTS_LOG"; }
set_iptable() { echo set_iptable >> "$TEST_ADG_EFFECTS_LOG"; }
TEST_ADG_MODE=unused
if ! start_adg; then
	fail "duplicate start was not treated as an already-running service"
fi
[ ! -s "$TEST_ADG_EFFECTS_LOG" ] || fail "duplicate start changed config/DNS/firewall state or launched again"

# No-process stop should clean stale DNS/firewall/workdir state without signalling.
: > "$TEST_ADG_KILL_LOG"
: > "$TEST_ADG_SLEEP_LOG"
: > "$TEST_ADG_CLEANUP_LOG"
echo stopped > "$TEST_ADG_STATE"
TEST_ADG_MODE=unused
if ! stop_adg; then
	fail "stop with no running daemon failed"
fi
[ ! -s "$TEST_ADG_KILL_LOG" ] || fail "no-process stop sent a signal"
[ ! -s "$TEST_ADG_SLEEP_LOG" ] || fail "no-process stop waited"
[ "$(cat "$TEST_ADG_CLEANUP_LOG")" = "$(printf 'clear_iptable\ndel_dns\nrm:-rf /tmp/AdGuardHome')" ] || fail "no-process cleanup ordering changed"

# A daemon that exits on TERM must not receive SIGKILL.
: > "$TEST_ADG_KILL_LOG"
: > "$TEST_ADG_SLEEP_LOG"
: > "$TEST_ADG_CLEANUP_LOG"
echo running > "$TEST_ADG_STATE"
TEST_ADG_MODE=graceful
if ! stop_adg; then
	fail "graceful daemon stop failed"
fi
[ "$(cat "$TEST_ADG_KILL_LOG")" = AdGuardHome ] || fail "graceful stop sent force kill or used unexpected process identity"
[ ! -s "$TEST_ADG_SLEEP_LOG" ] || fail "graceful daemon exit was not observed immediately"
[ "$(tail -n 3 "$TEST_ADG_CLEANUP_LOG")" = "$(printf 'clear_iptable\ndel_dns\nrm:-rf /tmp/AdGuardHome')" ] || fail "cleanup did not follow confirmed daemon exit"

# A TERM-resistant daemon is escalated only after five bounded polls.
: > "$TEST_ADG_KILL_LOG"
: > "$TEST_ADG_SLEEP_LOG"
: > "$TEST_ADG_CLEANUP_LOG"
echo running > "$TEST_ADG_STATE"
TEST_ADG_MODE=-9-exits
if ! stop_adg; then
	fail "TERM-resistant daemon did not stop after name-based SIGKILL"
fi
[ "$(cat "$TEST_ADG_KILL_LOG")" = "$(printf 'AdGuardHome\n-9 AdGuardHome')" ] || fail "stop did not perform TERM then fresh name-based SIGKILL"
[ "$(wc -l < "$TEST_ADG_SLEEP_LOG" | tr -d ' ')" = 5 ] || fail "TERM grace period was not bounded to five polls"
[ "$(tail -n 3 "$TEST_ADG_CLEANUP_LOG")" = "$(printf 'clear_iptable\ndel_dns\nrm:-rf /tmp/AdGuardHome')" ] || fail "cleanup did not follow confirmed SIGKILL exit"

# Firewall cleanup failure preserves runtime and resolver state for retry.
: > "$TEST_ADG_CLEANUP_LOG"
echo stopped > "$TEST_ADG_STATE"
TEST_ADG_CLEAR_FAIL=1
if stop_adg; then
	fail "stop reported success after firewall cleanup failed"
fi
[ "$(cat "$TEST_ADG_CLEANUP_LOG")" = clear_iptable ] || fail "failed firewall cleanup removed runtime state or DNS config"
TEST_ADG_CLEAR_FAIL=0

# Resolver update failure must leave the daemon and workdir available for retry.
: > "$TEST_ADG_KILL_LOG"
: > "$TEST_ADG_SLEEP_LOG"
: > "$TEST_ADG_CLEANUP_LOG"
echo running > "$TEST_ADG_STATE"
TEST_ADG_MODE=unused
TEST_ADG_DEL_DNS_FAIL=1
if stop_adg; then
  fail "stop reported success after resolver removal failed"
fi
[ "$(cat "$TEST_ADG_CLEANUP_LOG")" = "$(printf 'clear_iptable\\ndel_dns')" ] || fail "stop removed workdir or stopped the daemon after resolver failure"
[ ! -s "$TEST_ADG_KILL_LOG" ] || fail "stop signalled AdGuardHome before resolver removal was confirmed"
[ "$(cat "$TEST_ADG_STATE")" = running ] || fail "resolver failure did not leave the daemon available"
TEST_ADG_DEL_DNS_FAIL=0

# A daemon that survives SIGKILL must retain its workdir and traffic state.
: > "$TEST_ADG_KILL_LOG"
: > "$TEST_ADG_SLEEP_LOG"
: > "$TEST_ADG_CLEANUP_LOG"
echo running > "$TEST_ADG_STATE"
TEST_ADG_MODE=stuck
if stop_adg; then
	fail "stop reported success while AdGuardHome remained alive after SIGKILL"
fi
[ "$(cat "$TEST_ADG_KILL_LOG")" = "$(printf 'AdGuardHome\n-9 AdGuardHome')" ] || fail "stuck daemon did not receive TERM then name-based SIGKILL"
[ "$(wc -l < "$TEST_ADG_SLEEP_LOG" | tr -d ' ')" = 10 ] || fail "TERM/KILL waits were not bounded to five polls each"
[ "$(cat "$TEST_ADG_CLEANUP_LOG")" = "$(printf 'clear_iptable\\ndel_dns')" ] || fail "failed stop removed the workdir or skipped pre-stop resolver cleanup"

echo "AdGuardHome duplicate-start and bounded stop lifecycle regression: PASS"
