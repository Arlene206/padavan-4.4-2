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
