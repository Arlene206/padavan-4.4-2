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
ADG_CONFIG_FILE="$TEST_TMP/config-link"
export ADG_CONFIG_FILE
change_dns() { echo change_dns >> "$TEST_SIDE_EFFECTS"; }
set_iptable() { echo set_iptable >> "$TEST_SIDE_EFFECTS"; }
logger() { return 0; }
if start_adg; then
	fail "startup succeeded with a symlink config"
fi
[ ! -s "$TEST_SIDE_EFFECTS" ] || fail "startup changed DNS/firewall before config validation"

echo "AdGuardHome secret config mode, symlink rejection and atomic publication: PASS"

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
	return 0
}

clear_iptable() {
	echo clear_iptable >> "$TEST_ADG_CLEANUP_LOG"
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
[ "$(cat "$TEST_ADG_CLEANUP_LOG")" = "$(printf 'rm:-rf /tmp/AdGuardHome\ndel_dns\nclear_iptable')" ] || fail "no-process cleanup ordering changed"

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
[ "$(tail -n 3 "$TEST_ADG_CLEANUP_LOG")" = "$(printf 'rm:-rf /tmp/AdGuardHome\ndel_dns\nclear_iptable')" ] || fail "cleanup did not follow confirmed daemon exit"

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
[ "$(tail -n 3 "$TEST_ADG_CLEANUP_LOG")" = "$(printf 'rm:-rf /tmp/AdGuardHome\ndel_dns\nclear_iptable')" ] || fail "cleanup did not follow confirmed SIGKILL exit"

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
[ ! -s "$TEST_ADG_CLEANUP_LOG" ] || fail "failed stop removed runtime state or traffic redirection"

echo "AdGuardHome duplicate-start and bounded stop lifecycle regression: PASS"
