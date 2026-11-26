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
ADG_WORKDIR="$TEST_TMP/AdGuardHome persistent data"
export ADG_WORKDIR
adg_prepare_workdir || fail "persistent work directory creation failed"
[ "$(stat -c '%a' "$ADG_WORKDIR")" = 700 ] || fail "persistent work directory is not mode 0700"
printf '%s\n' "keep-filter-state" > "$ADG_WORKDIR/filter-state"

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

# Unsafe working-directory destinations must fail closed without following links.
(
  TEST_ADG_WORKDIR_TARGET="$TEST_TMP/workdir-target"
  TEST_ADG_WORKDIR_LINK="$TEST_TMP/workdir-link"
  TEST_ADG_WORKDIR_FIFO="$TEST_TMP/workdir-fifo"
  mkdir -p "$TEST_ADG_WORKDIR_TARGET"
  ln -s "$TEST_ADG_WORKDIR_TARGET" "$TEST_ADG_WORKDIR_LINK"
  ADG_WORKDIR="$TEST_ADG_WORKDIR_LINK"
  if adg_prepare_workdir; then exit 1; fi
  [ -d "$TEST_ADG_WORKDIR_TARGET" ] || exit 1
  mkfifo "$TEST_ADG_WORKDIR_FIFO"
  ADG_WORKDIR="$TEST_ADG_WORKDIR_FIFO"
  if adg_prepare_workdir; then exit 1; fi
  ADG_WORKDIR="$TEST_TMP/workdir-chmod-failure"
  mkdir -p "$ADG_WORKDIR"
  chmod() { return 1; }
  if adg_prepare_workdir; then exit 1; fi
  ADG_WORKDIR="$TEST_TMP/workdir-owner-mismatch"
  mkdir -p "$ADG_WORKDIR"
  (
    id() { printf '0\\n'; }
    stat() { printf '1\\n'; }
    if adg_prepare_workdir; then exit 1; fi
  ) || exit 1
) || fail "persistent work directory accepted an unsafe path, owner mismatch or permission failure"

echo "AdGuardHome persistent work directory symlink, type, ownership and mode policy: PASS"

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
  logger() { return 0; }
  TEST_DNS_REDIRECT_READ_STATUS=0
  nvram() {
    [ "$1" = get_checked ] && [ "$2" = adg_redirect ] || return 1
    [ "$TEST_DNS_REDIRECT_READ_STATUS" -eq 0 ] || return "$TEST_DNS_REDIRECT_READ_STATUS"
    printf '%s\n' 1
  }

  printf '%s\n' 'domain-needed' 'server=9.9.9.9' 'no-resolv' 'server=127.0.0.1#5353' > "$TEST_ADG_DNS_CONFIG"
  if ! change_dns; then exit 1; fi
  [ "$(grep -Fxc 'no-resolv' "$TEST_ADG_DNS_CONFIG")" = 1 ] || exit 1
  [ "$(grep -Fxc 'server=127.0.0.1#5335' "$TEST_ADG_DNS_CONFIG")" = 1 ] || exit 1
  ! grep -Fqx 'server=127.0.0.1#5353' "$TEST_ADG_DNS_CONFIG" || exit 1
  [ "$TEST_DNS_RESTART_CALLS" = 1 ] || exit 1

  # A checked selector backend error must not be mistaken for mode 0.
  cp "$TEST_ADG_DNS_CONFIG" "$TEST_TMP/dns-before-selector-error"
  TEST_DNS_REDIRECT_READ_STATUS=71
  if change_dns; then exit 1; fi
  cmp "$TEST_TMP/dns-before-selector-error" "$TEST_ADG_DNS_CONFIG" || exit 1
  [ "$TEST_DNS_RESTART_CALLS" = 1 ] || exit 1
  TEST_DNS_REDIRECT_READ_STATUS=0

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

# Exercise the same stable-inode flock protocol used by RC/UI service actions.
(
  TEST_ADG_LOCK="$TEST_TMP/.adguardhome.lifecycle.lock"
  adg_lifecycle_lock_path() { echo "$TEST_ADG_LOCK"; }
  logger() { return 0; }

  adg_lifecycle_lock_prepare "$TEST_ADG_LOCK" || exit 1
  [ -f "$TEST_ADG_LOCK" ] && [ ! -L "$TEST_ADG_LOCK" ] || exit 1
  TEST_ADG_LOCK_INFO="$(stat -c '%u %a %h' "$TEST_ADG_LOCK")"
  set -- $TEST_ADG_LOCK_INFO
  [ "$#" = 3 ] && [ "$2" = 600 ] && [ "$3" = 1 ] || exit 1
  TEST_ADG_LOCK_INODE="$(stat -c '%i' "$TEST_ADG_LOCK")"

  # Refuse a permissive lock and hardlink aliases rather than chmod/follow them.
  chmod 644 "$TEST_ADG_LOCK"
  if adg_lifecycle_lock_prepare "$TEST_ADG_LOCK"; then exit 1; fi
  chmod 600 "$TEST_ADG_LOCK"
  ln "$TEST_ADG_LOCK" "$TEST_TMP/adguardhome.lifecycle.hardlink"
  if adg_lifecycle_lock_prepare "$TEST_ADG_LOCK"; then exit 1; fi
  rm -f "$TEST_TMP/adguardhome.lifecycle.hardlink"
  printf '%s\n' 'lock-target-unchanged' > "$TEST_TMP/adguardhome.lifecycle.target"
  ln -s "$TEST_TMP/adguardhome.lifecycle.target" "$TEST_TMP/adguardhome.lifecycle.symlink"
  if adg_lifecycle_lock_prepare "$TEST_TMP/adguardhome.lifecycle.symlink"; then exit 1; fi
  grep -Fqx 'lock-target-unchanged' "$TEST_TMP/adguardhome.lifecycle.target" || exit 1
  rm -f "$TEST_TMP/adguardhome.lifecycle.symlink" "$TEST_TMP/adguardhome.lifecycle.target"

  # FD 9 is a reserved lock descriptor; never replace a caller's open FD.
  exec 9>"$TEST_TMP/adguardhome.fd9-held"
  if adg_lifecycle_lock_execute "$TEST_ADG_LOCK" 0 true; then exit 1; fi
  exec 9>&-

  # A real pair of concurrent processes must execute their critical sections
  # serially, not interleave stop/start state changes.
  TEST_ADG_LOCK_LOG="$TEST_TMP/adguardhome.lifecycle.concurrent.log"
  : > "$TEST_ADG_LOCK_LOG"
  adg_test_locked_action() {
    printf 'begin:%s\n' "$1" >> "$TEST_ADG_LOCK_LOG"
    /bin/sleep 0.25
    printf 'end:%s\n' "$1" >> "$TEST_ADG_LOCK_LOG"
    return 0
  }
  adg_lifecycle_lock_execute "$TEST_ADG_LOCK" 10 adg_test_locked_action first &
  TEST_ADG_LOCK_FIRST=$!
  /bin/sleep 0.05
  adg_lifecycle_lock_execute "$TEST_ADG_LOCK" 10 adg_test_locked_action second &
  TEST_ADG_LOCK_SECOND=$!
  wait "$TEST_ADG_LOCK_FIRST" || exit 1
  wait "$TEST_ADG_LOCK_SECOND" || exit 1
  [ "$(cat "$TEST_ADG_LOCK_LOG")" = "$(printf 'begin:first\nend:first\nbegin:second\nend:second')" ] || exit 1

  # An action failure must still close FD 9 and allow the next owner in.
  adg_test_failed_action() { return 1; }
  if adg_lifecycle_lock_execute "$TEST_ADG_LOCK" 0 adg_test_failed_action; then exit 1; fi
  adg_test_ok_action() { return 0; }
  adg_lifecycle_lock_execute "$TEST_ADG_LOCK" 0 adg_test_ok_action || exit 1

  # Kernel ownership, not a stale PID file, controls reclamation after death.
  TEST_ADG_LOCK_READY="$TEST_TMP/adguardhome.lifecycle.owner-ready"
  rm -f "$TEST_ADG_LOCK_READY"
  (
    exec 9>>"$TEST_ADG_LOCK"
    flock -n 9 || exit 1
    : > "$TEST_ADG_LOCK_READY"
    exec /bin/sleep 30
  ) &
  TEST_ADG_LOCK_OWNER=$!
  TEST_ADG_LOCK_WAIT=0
  while [ ! -e "$TEST_ADG_LOCK_READY" ] && [ "$TEST_ADG_LOCK_WAIT" -lt 40 ]; do
    /bin/sleep 0.05
    TEST_ADG_LOCK_WAIT=$((TEST_ADG_LOCK_WAIT + 1))
  done
  [ -e "$TEST_ADG_LOCK_READY" ] || exit 1
  if adg_lifecycle_lock_execute "$TEST_ADG_LOCK" 0 adg_test_ok_action; then exit 1; fi
  kill -9 "$TEST_ADG_LOCK_OWNER" 2>/dev/null || exit 1
  wait "$TEST_ADG_LOCK_OWNER" 2>/dev/null || :
  adg_lifecycle_lock_execute "$TEST_ADG_LOCK" 0 adg_test_ok_action || exit 1
  [ "$(stat -c '%i' "$TEST_ADG_LOCK")" = "$TEST_ADG_LOCK_INODE" ] || exit 1
) || fail "lifecycle flock policy, serialization or stale-owner recovery failed"

echo "AdGuardHome lifecycle lock stable inode, permissions and stale-owner recovery: PASS"
echo "AdGuardHome lifecycle lock excludes concurrent transitions and releases after failed actions: PASS"

# Verify restart is serialized as one stop + conditional start operation.
(
  TEST_ADG_LOCK="$TEST_TMP/.adguardhome.lifecycle.lock"
  TEST_ADG_RESTART_LOG="$TEST_TMP/adguardhome-restart.log"
  : > "$TEST_ADG_RESTART_LOG"
  stop_adg() { echo stop >> "$TEST_ADG_RESTART_LOG"; }
  start_adg() { echo start >> "$TEST_ADG_RESTART_LOG"; }
  TEST_ADG_ENABLE_VALUE=1
  TEST_ADG_ENABLE_READ_STATUS=0
  nvram() {
    [ "$1" = get_checked ] && [ "$2" = adg_enable ] || return 1
    [ "$TEST_ADG_ENABLE_READ_STATUS" -eq 0 ] || return "$TEST_ADG_ENABLE_READ_STATUS"
    printf '%s\n' "$TEST_ADG_ENABLE_VALUE"
  }
  adg_lifecycle_lock_prepare "$TEST_ADG_LOCK" || exit 1
  adg_lifecycle_lock_execute "$TEST_ADG_LOCK" 0 adg_lifecycle_locked_action restart || exit 1
  [ "$(cat "$TEST_ADG_RESTART_LOG")" = "$(printf 'stop\nstart')" ] || exit 1

  # A failed checked NVRAM read must leave the running daemon untouched.
  : > "$TEST_ADG_RESTART_LOG"
  TEST_ADG_ENABLE_READ_STATUS=71
  if adg_lifecycle_lock_execute "$TEST_ADG_LOCK" 0 adg_lifecycle_locked_action restart; then
    exit 1
  else
    TEST_ADG_ACTION_STATUS=$?
  fi
  [ "$TEST_ADG_ACTION_STATUS" -eq 71 ] || exit 1
  [ ! -s "$TEST_ADG_RESTART_LOG" ] || exit 1

  # An invalid non-empty selector is not an explicit disable.
  TEST_ADG_ENABLE_READ_STATUS=0
  for TEST_ADG_ENABLE_VALUE in unexpected -1 2 '1;reboot'; do
    : > "$TEST_ADG_RESTART_LOG"
    if adg_lifecycle_lock_execute "$TEST_ADG_LOCK" 0 adg_lifecycle_locked_action restart; then
      exit 1
    else
      TEST_ADG_ACTION_STATUS=$?
    fi
    [ "$TEST_ADG_ACTION_STATUS" -eq 22 ] || exit 1
    [ ! -s "$TEST_ADG_RESTART_LOG" ] || exit 1
  done
) || fail "restart did not keep stop and conditional start in a single locked action"

echo "AdGuardHome restart is a single locked stop/start transaction: PASS"
echo "AdGuardHome restart preserves a healthy daemon on enable-read errors or invalid selectors: PASS"

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

TEST_ADG_REDIRECT_READ_STATUS=0
nvram() {
	[ "$1" = get_checked ] && [ "$2" = adg_redirect ] || return 1
	[ "$TEST_ADG_REDIRECT_READ_STATUS" -eq 0 ] || return "$TEST_ADG_REDIRECT_READ_STATUS"
	printf '%s\n' "$TEST_ADG_REDIRECT_MODE"
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

# Selector read failures and unsupported values must be rejected before cleanup.
: > "$TEST_ADG_IPV4_RULES"
: > "$TEST_ADG_IPV6_RULES"
: > "$TEST_ADG_RULE_LOG"
printf '%s\n' keep-v4 > "$TEST_ADG_IPV4_RULES"
printf '%s\n' keep-v6 > "$TEST_ADG_IPV6_RULES"
TEST_ADG_REDIRECT_READ_STATUS=71
if set_iptable; then
	fail "firewall setup accepted a failed checked redirect selector read"
fi
[ "$(cat "$TEST_ADG_IPV4_RULES")" = keep-v4 ] || fail "selector read error deleted IPv4 rules before failing"
[ "$(cat "$TEST_ADG_IPV6_RULES")" = keep-v6 ] || fail "selector read error deleted IPv6 rules before failing"
[ ! -s "$TEST_ADG_RULE_LOG" ] || fail "selector read error touched firewall state"
TEST_ADG_REDIRECT_READ_STATUS=0
for TEST_ADG_REDIRECT_MODE in 3 -1 '1;reboot'; do
	: > "$TEST_ADG_RULE_LOG"
	if set_iptable; then fail "unsupported redirect selector was accepted: $TEST_ADG_REDIRECT_MODE"; fi
	[ "$(cat "$TEST_ADG_IPV4_RULES")" = keep-v4 ] || fail "invalid selector deleted IPv4 state"
	[ "$(cat "$TEST_ADG_IPV6_RULES")" = keep-v6 ] || fail "invalid selector deleted IPv6 state"
	[ ! -s "$TEST_ADG_RULE_LOG" ] || fail "invalid selector touched firewall state"
done
TEST_ADG_REDIRECT_MODE=2
: > "$TEST_ADG_IPV4_RULES"
: > "$TEST_ADG_IPV6_RULES"
: > "$TEST_ADG_RULE_LOG"

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

# A failed or invalid redirect-mode read must abort before mkdir/firewall/DNS/daemon effects.
(
	TEST_ADG_START_EFFECTS="$TEST_TMP/start-selector-failure.log"
	: > "$TEST_ADG_START_EFFECTS"
	pidof() { return 1; }
	getconfig() { echo getconfig >> "$TEST_ADG_START_EFFECTS"; adg_file="$TEST_CONFIG"; return 0; }
	mkdir() { echo mkdir >> "$TEST_ADG_START_EFFECTS"; return 0; }
	set_iptable() { echo set_iptable >> "$TEST_ADG_START_EFFECTS"; return 0; }
	change_dns() { echo change_dns >> "$TEST_ADG_START_EFFECTS"; return 0; }
	AdGuardHome() { echo launch >> "$TEST_ADG_START_EFFECTS"; }
	logger() { return 0; }
	for TEST_ADG_SELECTOR_CASE in backend invalid; do
		if [ "$TEST_ADG_SELECTOR_CASE" = backend ]; then
			TEST_ADG_REDIRECT_READ_STATUS=71
		else
			TEST_ADG_REDIRECT_READ_STATUS=0
			TEST_ADG_REDIRECT_MODE=3
		fi
		: > "$TEST_ADG_START_EFFECTS"
		if start_adg; then exit 1; fi
		[ "$(cat "$TEST_ADG_START_EFFECTS")" = getconfig ] || exit 1
	done
	TEST_ADG_REDIRECT_READ_STATUS=0
	TEST_ADG_REDIRECT_MODE=2
) || fail "redirect-mode preflight did not block startup side effects"
echo "AdGuardHome redirect selector checked-read and fail-closed validation: PASS"

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
  TEST_ADG_START_LOGGER="$TEST_TMP/start-dns-failure.logger.log"
  : > "$TEST_ADG_START_EFFECTS"
  : > "$TEST_ADG_START_LOGGER"
  pidof() { return 1; }
  getconfig() { return 0; }
  set_iptable() { echo set_iptable >> "$TEST_ADG_START_EFFECTS"; return 0; }
  change_dns() { echo change_dns >> "$TEST_ADG_START_EFFECTS"; return 1; }
  clear_iptable() { echo clear_iptable >> "$TEST_ADG_START_EFFECTS"; return 0; }
  AdGuardHome() { echo launch >> "$TEST_ADG_START_EFFECTS"; }
  logger() { echo "$*" >> "$TEST_ADG_START_LOGGER"; }
  if start_adg; then exit 1; fi
  [ "$(cat "$TEST_ADG_START_EFFECTS")" = "$(printf 'set_iptable\nchange_dns')" ] || exit 1
  grep -q "保留DNS防火墙拦截规则" "$TEST_ADG_START_LOGGER" || exit 1
) || fail "start continued after resolver reload failure"

# Exercise daemon stop/start gates with a process-name-only mock.
TEST_ADG_STATE="$TEST_TMP/adguardhome.state"
TEST_ADG_KILL_LOG="$TEST_TMP/adguardhome.kill.log"
TEST_ADG_SLEEP_LOG="$TEST_TMP/adguardhome.sleep.log"
TEST_ADG_CLEANUP_LOG="$TEST_TMP/adguardhome.cleanup.log"
TEST_ADG_EFFECTS_LOG="$TEST_TMP/adguardhome.start-effects.log"
export TEST_ADG_STATE TEST_ADG_KILL_LOG TEST_ADG_SLEEP_LOG
export TEST_ADG_CLEANUP_LOG TEST_ADG_EFFECTS_LOG
TEST_ADG_PID_OUTPUT=24680
export TEST_ADG_PID_OUTPUT
TEST_ADG_PID_LOG=$TEST_TMP/pid-signals.log
export TEST_ADG_PID_LOG
ADG_PROC_ROOT="$TEST_TMP/proc"
ADG_EXECUTABLE="$TEST_TMP/AdGuardHome"
: > "$ADG_EXECUTABLE"
chmod 755 "$ADG_EXECUTABLE"
mkdir -p "$ADG_PROC_ROOT/1"
printf '1 (init) S 0 1 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 100\n' > "$ADG_PROC_ROOT/1/stat"
adg_test_add_proc_pid()
{
	adg_test_pid="$1"
	adg_test_exe="$2"
	adg_test_config="$3"
	adg_test_start="${4:-1000}"
	mkdir -p "$ADG_PROC_ROOT/$adg_test_pid"
	ln -sfn "$adg_test_exe" "$ADG_PROC_ROOT/$adg_test_pid/exe"
	printf '%s (AdGuardHome) S 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 %s\n' \
		"$adg_test_pid" "$adg_test_start" > "$ADG_PROC_ROOT/$adg_test_pid/stat"
	printf 'AdGuardHome\0-c\0%s\0-w\0%s\0-v\0' \
		"$adg_test_config" "$ADG_WORKDIR" > "$ADG_PROC_ROOT/$adg_test_pid/cmdline"
}
TEST_ADG_PROC_CASE=valid

pidof() {
	[ "$1" = AdGuardHome ] || return 1
	case "$(cat "$TEST_ADG_STATE")" in
		running)
			for adg_test_pid in ${TEST_ADG_PID_OUTPUT:-24680}; do
				case "$TEST_ADG_PROC_CASE" in
					wrong-exe)
						adg_test_add_proc_pid "$adg_test_pid" /bin/sleep "${ADG_CONFIG_FILE:-/etc/storage/adg.sh}"
						;;
					wrong-argv)
						adg_test_add_proc_pid "$adg_test_pid" "$ADG_EXECUTABLE" "$TEST_TMP/decoy.yml"
						;;
					bad-stat)
						adg_test_add_proc_pid "$adg_test_pid" "$ADG_EXECUTABLE" "${ADG_CONFIG_FILE:-/etc/storage/adg.sh}"
						printf 'truncated\n' > "$ADG_PROC_ROOT/$adg_test_pid/stat"
						;;
					*)
						adg_test_add_proc_pid "$adg_test_pid" "$ADG_EXECUTABLE" "${ADG_CONFIG_FILE:-/etc/storage/adg.sh}"
						;;
				esac
			done
			printf '%s\n' "${TEST_ADG_PID_OUTPUT:-24680}"
			return 0
			;;
		error|unknown) return 2 ;;
		malformed) echo "24680 invalid"; return 0 ;;
		empty-success) return 0 ;;
		inconsistent) echo 24680; return 1 ;;
		*) return 1 ;;
	esac
}

killall() {
	printf 'killall:%s\n' "$*" >> "$TEST_ADG_KILL_LOG"
	return 0
}

kill() {
	case "$1" in
		-TERM) echo AdGuardHome >> "$TEST_ADG_KILL_LOG" ;;
		-9) echo "-9 AdGuardHome" >> "$TEST_ADG_KILL_LOG" ;;
	esac
	printf '%s %s\n' "$1" "$2" >> "$TEST_ADG_PID_LOG"
	case "$TEST_ADG_MODE:$1" in
		graceful:-TERM|nonzero-graceful:-TERM)
			echo stopped > "$TEST_ADG_STATE"
			;;
		unknown-after-term:-TERM)
			echo error > "$TEST_ADG_STATE"
			;;
		-9-exits:-9)
			echo stopped > "$TEST_ADG_STATE"
			;;
	esac
	[ "$TEST_ADG_MODE:$1" != nonzero-graceful:-TERM ] || return 29
	return 0
}

sleep() {
	printf '%s\n' "$*" >> "$TEST_ADG_SLEEP_LOG"
	if [ "${TEST_ADG_REPLACE_PID_ON_SLEEP:-0}" = 1 ]; then
		TEST_ADG_PID_OUTPUT=13579
		export TEST_ADG_PID_OUTPUT
		TEST_ADG_REPLACE_PID_ON_SLEEP=0
	fi
	if [ -n "${TEST_ADG_START_TRANSITION:-}" ]; then
		/bin/sleep 0.05
		printf '%s\n' "$TEST_ADG_START_TRANSITION" > "$TEST_ADG_STATE"
		TEST_ADG_START_TRANSITION=
	fi
	[ "${TEST_ADG_SLEEP_FAIL:-0}" = 0 ]
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
: > "$TEST_ADG_PID_LOG"
: > "$TEST_ADG_SLEEP_LOG"
: > "$TEST_ADG_CLEANUP_LOG"
: > "$TEST_ADG_EFFECTS_LOG"
echo running > "$TEST_ADG_STATE"
getconfig() { echo getconfig >> "$TEST_ADG_EFFECTS_LOG"; return 0; }
change_dns() { echo change_dns >> "$TEST_ADG_EFFECTS_LOG"; }
set_iptable() {
	echo set_iptable >> "$TEST_ADG_CLEANUP_LOG"
	[ "${TEST_ADG_SET_IPTABLE_FAIL:-0}" = 0 ]
}

# A symlinked persistent data directory must stop startup before traffic changes.
(
  ADG_WORKDIR="$TEST_TMP/unsafe-start-workdir"
  mkdir -p "$TEST_TMP/unsafe-start-target"
  ln -s "$TEST_TMP/unsafe-start-target" "$ADG_WORKDIR"
  echo stopped > "$TEST_ADG_STATE"
  : > "$TEST_ADG_EFFECTS_LOG"
  if start_adg; then exit 1; fi
  [ "$(cat "$TEST_ADG_EFFECTS_LOG")" = getconfig ] || exit 1
) || fail "unsafe persistent work directory changed DNS/firewall state"
# The state is stored in a shared fixture file, so restore RUNNING after the
# subshell before the duplicate-start regression below.
echo running > "$TEST_ADG_STATE"
: > "$TEST_ADG_EFFECTS_LOG"

TEST_ADG_MODE=unused
if ! start_adg; then
	fail "duplicate start was not treated as an already-running service"
fi
[ ! -s "$TEST_ADG_EFFECTS_LOG" ] || fail "duplicate start changed config/DNS/firewall state or launched again"

# Unknown process inventory must not be treated as ABSENT on start.
echo error > "$TEST_ADG_STATE"
: > "$TEST_ADG_EFFECTS_LOG"
if start_adg; then
	fail "start accepted an UNKNOWN process inventory"
fi
[ ! -s "$TEST_ADG_EFFECTS_LOG" ] || fail "UNKNOWN start changed config/DNS/firewall state or launched a duplicate"
echo running > "$TEST_ADG_STATE"

# No-process stop should clean stale traffic state while retaining persistent filter/query data.
: > "$TEST_ADG_KILL_LOG"
: > "$TEST_ADG_PID_LOG"
: > "$TEST_ADG_SLEEP_LOG"
: > "$TEST_ADG_CLEANUP_LOG"
echo stopped > "$TEST_ADG_STATE"
TEST_ADG_MODE=unused
if ! stop_adg; then
	fail "stop with no running daemon failed"
fi
[ ! -s "$TEST_ADG_KILL_LOG" ] || fail "no-process stop sent a signal"
[ ! -s "$TEST_ADG_SLEEP_LOG" ] || fail "no-process stop waited"
[ "$(cat "$TEST_ADG_CLEANUP_LOG")" = "$(printf 'clear_iptable\ndel_dns\n')" ] || fail "no-process cleanup ordering changed"
grep -Fqx "keep-filter-state" "$ADG_WORKDIR/filter-state" || fail "no-process stop deleted persistent filter/query state"

# A same-name PID with another executable or a different config argv is not
# this managed service and must never receive TERM or SIGKILL.
for TEST_ADG_PROC_CASE in wrong-exe wrong-argv; do
	: > "$TEST_ADG_KILL_LOG"
	: > "$TEST_ADG_PID_LOG"
	: > "$TEST_ADG_SLEEP_LOG"
	: > "$TEST_ADG_CLEANUP_LOG"
	echo running > "$TEST_ADG_STATE"
	if adg_process_state; then fail "AdGuardHome accepted an unowned process candidate: $TEST_ADG_PROC_CASE"; else adg_identity_state=$?; fi
	[ "$adg_identity_state" -eq 1 ] || fail "unowned process did not classify as ABSENT: $TEST_ADG_PROC_CASE"
	if ! stop_adg; then fail "stop refused confirmed-absent decoy case: $TEST_ADG_PROC_CASE"; fi
	[ ! -s "$TEST_ADG_PID_LOG" ] || fail "AdGuardHome signalled a same-name unowned process: $TEST_ADG_PROC_CASE"
done

# Matching executable with malformed proc metadata is UNKNOWN, not a reason
# to signal or clear the managed work directory.
: > "$TEST_ADG_KILL_LOG"
: > "$TEST_ADG_PID_LOG"
: > "$TEST_ADG_SLEEP_LOG"
: > "$TEST_ADG_CLEANUP_LOG"
echo running > "$TEST_ADG_STATE"
TEST_ADG_PROC_CASE=bad-stat
if adg_process_state; then fail "malformed AdGuardHome proc metadata was accepted"; else adg_identity_state=$?; fi
[ "$adg_identity_state" -eq 2 ] || fail "malformed AdGuardHome proc metadata was not UNKNOWN"
if stop_adg; then fail "stop accepted UNKNOWN AdGuardHome executable metadata"; fi
[ ! -s "$TEST_ADG_PID_LOG" ] || fail "AdGuardHome signalled a process with unverified metadata"
[ "$(cat "$TEST_ADG_CLEANUP_LOG")" = "$(printf 'clear_iptable\ndel_dns')" ] ||
	fail "UNKNOWN process identity removed workdir or skipped pre-stop cleanup"
TEST_ADG_PROC_CASE=valid



# pidof errors, malformed successful output and inconsistent status/output
# are UNKNOWN; none may be interpreted as a confirmed ABSENT process.
for unknown_state in error malformed empty-success inconsistent; do
	: > "$TEST_ADG_KILL_LOG"
: > "$TEST_ADG_PID_LOG"
	: > "$TEST_ADG_SLEEP_LOG"
	: > "$TEST_ADG_CLEANUP_LOG"
	echo "$unknown_state" > "$TEST_ADG_STATE"
	TEST_ADG_MODE=unused
	if stop_adg; then
		fail "stop accepted UNKNOWN process inventory: $unknown_state"
	fi
	[ ! -s "$TEST_ADG_KILL_LOG" ] || fail "stop signalled AdGuardHome with UNKNOWN inventory: $unknown_state"
	[ ! -s "$TEST_ADG_SLEEP_LOG" ] || fail "stop waited with UNKNOWN initial inventory: $unknown_state"
	[ "$(cat "$TEST_ADG_CLEANUP_LOG")" = "$(printf 'clear_iptable\ndel_dns')" ] ||
		fail "stop removed workdir or changed pre-stop cleanup for UNKNOWN inventory: $unknown_state"
done

# Every PID in a validated RUNNING snapshot receives TERM individually.
: > "$TEST_ADG_KILL_LOG"
: > "$TEST_ADG_PID_LOG"
: > "$TEST_ADG_SLEEP_LOG"
: > "$TEST_ADG_CLEANUP_LOG"
echo running > "$TEST_ADG_STATE"
TEST_ADG_PID_OUTPUT='12345 23456'
export TEST_ADG_PID_OUTPUT
TEST_ADG_MODE=graceful
if ! stop_adg; then fail "multi-PID graceful stop failed"; fi
[ "$(cat "$TEST_ADG_PID_LOG")" = "$(printf '%s\n' '-TERM 12345' '-TERM 23456')" ] ||
	fail "stop did not signal every validated PID individually"
! grep -q '^killall:' "$TEST_ADG_KILL_LOG" || fail "stop used process-name-wide killall"
TEST_ADG_PID_OUTPUT=24680
export TEST_ADG_PID_OUTPUT

# UNKNOWN after TERM blocks SIGKILL escalation.
: > "$TEST_ADG_KILL_LOG"
: > "$TEST_ADG_PID_LOG"
: > "$TEST_ADG_SLEEP_LOG"
: > "$TEST_ADG_CLEANUP_LOG"
echo running > "$TEST_ADG_STATE"
TEST_ADG_MODE=unknown-after-term
if stop_adg; then fail "stop accepted UNKNOWN state after TERM"; fi
[ "$(cat "$TEST_ADG_KILL_LOG")" = AdGuardHome ] || fail "UNKNOWN after TERM triggered name-wide SIGKILL"
[ ! -s "$TEST_ADG_SLEEP_LOG" ] || fail "UNKNOWN after TERM did not abort immediately"
[ "$(cat "$TEST_ADG_CLEANUP_LOG")" = "$(printf 'clear_iptable\ndel_dns')" ] ||
	fail "UNKNOWN after TERM removed workdir or skipped resolver cleanup"

# A failed wait primitive is UNKNOWN and also blocks escalation.
: > "$TEST_ADG_KILL_LOG"
: > "$TEST_ADG_PID_LOG"
: > "$TEST_ADG_SLEEP_LOG"
: > "$TEST_ADG_CLEANUP_LOG"
echo running > "$TEST_ADG_STATE"
TEST_ADG_MODE=unused
TEST_ADG_SLEEP_FAIL=1
if stop_adg; then fail "stop accepted a failed wait primitive"; fi
[ "$(cat "$TEST_ADG_KILL_LOG")" = AdGuardHome ] || fail "sleep failure triggered SIGKILL"
[ "$(wc -l < "$TEST_ADG_SLEEP_LOG" | tr -d ' ')" = 1 ] || fail "sleep failure was not observed once"
[ "$(cat "$TEST_ADG_CLEANUP_LOG")" = "$(printf 'clear_iptable\ndel_dns')" ] ||
	fail "sleep failure removed workdir or skipped resolver cleanup"
TEST_ADG_SLEEP_FAIL=0

# A failed TERM command is not decisive when a fresh snapshot confirms ABSENT.
: > "$TEST_ADG_KILL_LOG"
: > "$TEST_ADG_PID_LOG"
: > "$TEST_ADG_SLEEP_LOG"
: > "$TEST_ADG_CLEANUP_LOG"
echo running > "$TEST_ADG_STATE"
TEST_ADG_MODE=nonzero-graceful
if ! stop_adg; then fail "nonzero TERM status overrode confirmed ABSENT"; fi
[ "$(cat "$TEST_ADG_KILL_LOG")" = AdGuardHome ] || fail "failed TERM used an unexpected signal"
[ ! -s "$TEST_ADG_SLEEP_LOG" ] || fail "failed TERM waited after confirmed ABSENT"

# A daemon that exits on TERM must not receive SIGKILL.
: > "$TEST_ADG_KILL_LOG"
: > "$TEST_ADG_PID_LOG"
: > "$TEST_ADG_SLEEP_LOG"
: > "$TEST_ADG_CLEANUP_LOG"
echo running > "$TEST_ADG_STATE"
TEST_ADG_MODE=graceful
if ! stop_adg; then
	fail "graceful daemon stop failed"
fi
[ "$(cat "$TEST_ADG_KILL_LOG")" = AdGuardHome ] || fail "graceful stop sent force kill or used unexpected process identity"
[ ! -s "$TEST_ADG_SLEEP_LOG" ] || fail "graceful daemon exit was not observed immediately"
[ "$(tail -n 3 "$TEST_ADG_CLEANUP_LOG")" = "$(printf 'clear_iptable\ndel_dns\n')" ] || fail "cleanup did not follow confirmed daemon exit"
grep -Fqx "keep-filter-state" "$ADG_WORKDIR/filter-state" || fail "graceful stop deleted persistent service data"

# A TERM-resistant daemon is escalated only after five bounded polls.
: > "$TEST_ADG_KILL_LOG"
: > "$TEST_ADG_PID_LOG"
: > "$TEST_ADG_SLEEP_LOG"
: > "$TEST_ADG_CLEANUP_LOG"
echo running > "$TEST_ADG_STATE"
TEST_ADG_MODE=-9-exits
TEST_ADG_REPLACE_PID_ON_SLEEP=1
if ! stop_adg; then
	fail "TERM-resistant daemon did not stop after name-based SIGKILL"
fi
[ "$(cat "$TEST_ADG_KILL_LOG")" = "$(printf 'AdGuardHome\n-9 AdGuardHome')" ] || fail "stop did not perform TERM then fresh name-based SIGKILL"
[ "$(cat "$TEST_ADG_PID_LOG")" = "$(printf '%s\n' '-TERM 24680' '-9 13579')" ] || fail "SIGKILL did not use the freshly validated replacement PID"
[ "$(wc -l < "$TEST_ADG_SLEEP_LOG" | tr -d ' ')" = 5 ] || fail "TERM grace period was not bounded to five polls"
[ "$(tail -n 3 "$TEST_ADG_CLEANUP_LOG")" = "$(printf 'clear_iptable\ndel_dns\n')" ] || fail "cleanup did not follow confirmed SIGKILL exit"
grep -Fqx "keep-filter-state" "$ADG_WORKDIR/filter-state" || fail "SIGKILL stop deleted persistent service data"
# The next stuck-daemon case begins a fresh process fixture; do not leak the
# replacement PID from this scenario into its initial TERM snapshot.
TEST_ADG_PID_OUTPUT=24680
export TEST_ADG_PID_OUTPUT

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
: > "$TEST_ADG_PID_LOG"
: > "$TEST_ADG_SLEEP_LOG"
: > "$TEST_ADG_CLEANUP_LOG"
echo running > "$TEST_ADG_STATE"
TEST_ADG_MODE=unused
TEST_ADG_DEL_DNS_FAIL=1
if stop_adg; then
  fail "stop reported success after resolver removal failed"
fi
[ "$(cat "$TEST_ADG_CLEANUP_LOG")" = "$(printf 'clear_iptable\ndel_dns\nset_iptable')" ] || fail "resolver failure did not restore DNS interception before returning"
[ ! -s "$TEST_ADG_KILL_LOG" ] || fail "stop signalled AdGuardHome before resolver removal was confirmed"
[ "$(cat "$TEST_ADG_STATE")" = running ] || fail "resolver failure did not leave the daemon available"
TEST_ADG_DEL_DNS_FAIL=0

# A daemon that survives SIGKILL must retain its workdir and traffic state.
: > "$TEST_ADG_KILL_LOG"
: > "$TEST_ADG_PID_LOG"
: > "$TEST_ADG_SLEEP_LOG"
: > "$TEST_ADG_CLEANUP_LOG"
echo running > "$TEST_ADG_STATE"
TEST_ADG_MODE=stuck
if stop_adg; then
	fail "stop reported success while AdGuardHome remained alive after SIGKILL"
fi
[ "$(cat "$TEST_ADG_KILL_LOG")" = "$(printf 'AdGuardHome\n-9 AdGuardHome')" ] || fail "stuck daemon did not receive TERM then name-based SIGKILL"
[ "$(cat "$TEST_ADG_PID_LOG")" = "$(printf '%s\n' '-TERM 24680' '-9 24680')" ] || fail "stuck daemon did not receive per-PID SIGKILL"
[ "$(wc -l < "$TEST_ADG_SLEEP_LOG" | tr -d ' ')" = 10 ] || fail "TERM/KILL waits were not bounded to five polls each"
[ "$(cat "$TEST_ADG_CLEANUP_LOG")" = "$(printf 'clear_iptable\ndel_dns')" ] || fail "failed stop removed the workdir or skipped pre-stop resolver cleanup"

# Detached startup is successful only after the process-name inventory confirms RUNNING.
(
  TEST_ADG_COMMAND_MARKER="$TEST_TMP/adguardhome-command-injected"
  TEST_ADG_ARGV_LOG="$TEST_TMP/adguardhome.argv"
  TEST_ADG_ARGV_EXPECTED="$TEST_TMP/adguardhome.argv.expected"
  ADG_CONFIG_FILE="$TEST_TMP/config with spaces; touch $TEST_TMP/adguardhome-command-injected #"
  export TEST_ADG_COMMAND_MARKER TEST_ADG_ARGV_LOG ADG_CONFIG_FILE
  getconfig() { adg_file="$ADG_CONFIG_FILE"; echo getconfig >> "$TEST_ADG_EFFECTS_LOG"; return 0; }
  set_iptable() { echo set_iptable >> "$TEST_ADG_EFFECTS_LOG"; return 0; }
  change_dns() { echo change_dns >> "$TEST_ADG_EFFECTS_LOG"; return 0; }
  AdGuardHome() {
    printf '<%s>\n' "$@" >> "$TEST_ADG_ARGV_LOG"
    echo launch >> "$TEST_ADG_EFFECTS_LOG"
    return 0
  }

  : > "$TEST_ADG_EFFECTS_LOG"
  : > "$TEST_ADG_ARGV_LOG"
  : > "$TEST_ADG_CLEANUP_LOG"
  : > "$TEST_ADG_SLEEP_LOG"
  echo stopped > "$TEST_ADG_STATE"
  TEST_ADG_START_TRANSITION=running
  TEST_ADG_SLEEP_FAIL=0
  if ! start_adg; then exit 1; fi
  [ "$(cat "$TEST_ADG_STATE")" = running ] || exit 1
  [ "$(wc -l < "$TEST_ADG_SLEEP_LOG" | tr -d ' ')" = 1 ] || exit 1
  /bin/sleep 0.05
  grep -Fqx launch "$TEST_ADG_EFFECTS_LOG" || exit 1
  printf '<%s>\n' -c "$ADG_CONFIG_FILE" -w "$ADG_WORKDIR" -v > "$TEST_ADG_ARGV_EXPECTED"
  cmp "$TEST_ADG_ARGV_EXPECTED" "$TEST_ADG_ARGV_LOG" || exit 1
  [ ! -e "$TEST_ADG_COMMAND_MARKER" ] || exit 1
  [ ! -s "$TEST_ADG_CLEANUP_LOG" ] || exit 1

  # A confirmed ABSENT timeout must restore resolver state before firewall rules.
  : > "$TEST_ADG_EFFECTS_LOG"
  : > "$TEST_ADG_CLEANUP_LOG"
  : > "$TEST_ADG_SLEEP_LOG"
  echo stopped > "$TEST_ADG_STATE"
  TEST_ADG_START_TRANSITION=
  if start_adg; then exit 1; fi
  /bin/sleep 0.05
  grep -Fqx launch "$TEST_ADG_EFFECTS_LOG" || exit 1
  [ "$(cat "$TEST_ADG_CLEANUP_LOG")" = "$(printf 'del_dns\nclear_iptable')" ] || exit 1
  [ "$(wc -l < "$TEST_ADG_SLEEP_LOG" | tr -d ' ')" = "$((ADG_START_WAIT_ATTEMPTS - 1))" ] || exit 1

  # If resolver rollback fails after a confirmed startup timeout, retain the
  # redirect rules rather than allowing direct DNS to bypass AdGuard policy.
  : > "$TEST_ADG_EFFECTS_LOG"
  : > "$TEST_ADG_CLEANUP_LOG"
  : > "$TEST_ADG_SLEEP_LOG"
  echo stopped > "$TEST_ADG_STATE"
  TEST_ADG_START_TRANSITION=
  TEST_ADG_DEL_DNS_FAIL=1
  if start_adg; then exit 1; fi
  [ "$(cat "$TEST_ADG_CLEANUP_LOG")" = del_dns ] || exit 1
  TEST_ADG_DEL_DNS_FAIL=0

  # UNKNOWN immediately after launch must fail closed without tearing down
  # DNS/firewall state that a possibly-running daemon may still need.
  : > "$TEST_ADG_CLEANUP_LOG"
  : > "$TEST_ADG_SLEEP_LOG"
  echo stopped > "$TEST_ADG_STATE"
  TEST_ADG_START_TRANSITION=error
  if start_adg; then exit 1; fi
  [ "$(cat "$TEST_ADG_STATE")" = error ] || exit 1
  [ "$(wc -l < "$TEST_ADG_SLEEP_LOG" | tr -d ' ')" = 1 ] || exit 1
  [ ! -s "$TEST_ADG_CLEANUP_LOG" ] || exit 1

  # A failed wait primitive is UNKNOWN too and must not trigger rollback.
  : > "$TEST_ADG_CLEANUP_LOG"
  : > "$TEST_ADG_SLEEP_LOG"
  echo stopped > "$TEST_ADG_STATE"
  TEST_ADG_START_TRANSITION=
  TEST_ADG_SLEEP_FAIL=1
  if start_adg; then exit 1; fi
  [ ! -s "$TEST_ADG_CLEANUP_LOG" ] || exit 1
  [ "$(wc -l < "$TEST_ADG_SLEEP_LOG" | tr -d ' ')" = 1 ] || exit 1
  TEST_ADG_SLEEP_FAIL=0
) || fail "AdGuardHome detached startup postcondition or fail-closed rollback failed"

echo "AdGuardHome command argv quoting and metacharacter safety: PASS"
echo "AdGuardHome stop signals only validated PID snapshots: PASS"
echo "AdGuardHome detached startup RUNNING postcondition, timeout rollback and UNKNOWN preservation: PASS"
echo "AdGuardHome duplicate-start and bounded stop lifecycle regression: PASS"
