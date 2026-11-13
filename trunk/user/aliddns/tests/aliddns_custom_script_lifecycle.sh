#!/bin/sh
set -eu

TEST_DIR="$(mktemp -d)"
trap 'rm -rf "$TEST_DIR"' EXIT HUP INT TERM
SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
SOURCE="$SCRIPT_DIR/../aliddns.sh"
TEMPLATE="$SCRIPT_DIR/../ddns_script.sh"
FUNCTIONS="$TEST_DIR/functions.sh"
STORAGE_DIR="$TEST_DIR/storage"
ROMFS_DIR="$TEST_DIR/romfs"
mkdir -p "$STORAGE_DIR" "$ROMFS_DIR"
: > "$FUNCTIONS"

# Extract production functions without executing the router start/keep loop.
sed -n '/^aliddns_ipv4_valid() {/,/^}/p' "$SOURCE" >> "$FUNCTIONS"
sed -n '/^aliddns_ipv6_valid() {/,/^}/p' "$SOURCE" >> "$FUNCTIONS"
sed -n '/^arDdnsCheck() {/,/^}/p' "$SOURCE" >> "$FUNCTIONS"
sed -n '/^initconfig () {/,/^}/p' "$SOURCE" >> "$FUNCTIONS"
. "$FUNCTIONS"

ALIDDNS_DDNS_SCRIPT_PATH="$STORAGE_DIR/ddns_script.sh"
ALIDDNS_DDNS_TEMPLATE_PATH="$ROMFS_DIR/ddns_script.sh"
export ALIDDNS_DDNS_SCRIPT_PATH ALIDDNS_DDNS_TEMPLATE_PATH
cp "$TEMPLATE" "$ALIDDNS_DDNS_TEMPLATE_PATH"

INFO_LOG="$TEST_DIR/info.log"
UPDATE_LOG="$TEST_DIR/update.log"
: > "$INFO_LOG"
: > "$UPDATE_LOG"

logger() { :; }
sleep() { :; }
arDdnsInfo() {
	printf '%s\n' "$1" >> "$INFO_LOG"
	return 2
}
arDdnsUpdate() {
	printf '%s\n' "$1 $2 $hostIP" >> "$UPDATE_LOG"
	printf '%s\n' "updated"
	return 0
}
fail() {
	echo "AliDDNS custom-script lifecycle regression: FAIL: $1" >&2
	exit 1
}
info_count() { wc -l < "$INFO_LOG"; }
update_count() { wc -l < "$UPDATE_LOG"; }

# Expected API status 2 is part of the normal no-record branch; temporarily
# disable errexit while exercising the production function.
run_check() {
	set +e
	arDdnsCheck "$@"
	check_status=$?
	set -e
	return "$check_status"
}

# A valid operator script is sourced and its IPv4 value reaches the API update.
printf '%s\n' "arIpAddress='198.51.100.44'" > "$ALIDDNS_DDNS_SCRIPT_PATH"
IPv6=0
if ! run_check example.com www >/dev/null 2>&1; then
	fail "valid custom IPv4 provider was rejected"
fi
[ "$(info_count)" -eq 1 ] || fail "valid custom IPv4 did not reach record comparison"
[ "$(update_count)" -eq 1 ] || fail "valid custom IPv4 did not reach update"
grep -Fq "198.51.100.44" "$UPDATE_LOG" || fail "valid IPv4 value was not passed to the update"

# A missing script must not reuse the previous domain's address or reach the API.
rm -f "$ALIDDNS_DDNS_SCRIPT_PATH"
before_info="$(info_count)"
before_update="$(update_count)"
if run_check example.com www >/dev/null 2>&1; then
	fail "missing custom provider file was accepted"
fi
[ "$(info_count)" -eq "$before_info" ] || fail "missing source reached record comparison"
[ "$(update_count)" -eq "$before_update" ] || fail "missing source reached API update"

# A provider that assigns an address and then exits unsuccessfully is not trusted.
printf '%s\n' "arIpAddress='198.51.100.45'" "false" > "$ALIDDNS_DDNS_SCRIPT_PATH"
if run_check example.com www >/dev/null 2>&1; then
	fail "nonzero custom provider was accepted"
fi
[ "$(info_count)" -eq "$before_info" ] || fail "failed source reached record comparison"

# Strict IPv4/IPv6 validation occurs before the AliDNS record lookup.
printf '%s\n' "arIpAddress='999.1.1.1'" > "$ALIDDNS_DDNS_SCRIPT_PATH"
IPv6=0
if run_check example.com www >/dev/null 2>&1; then
	fail "invalid custom IPv4 was accepted"
fi
[ "$(info_count)" -eq "$before_info" ] || fail "invalid IPv4 reached record comparison"

printf '%s\n' "arIpAddress='2001:db8:::1'" > "$ALIDDNS_DDNS_SCRIPT_PATH"
IPv6=1
if run_check example.com www >/dev/null 2>&1; then
	fail "invalid custom IPv6 was accepted"
fi
[ "$(info_count)" -eq "$before_info" ] || fail "invalid IPv6 reached record comparison"

printf '%s\n' "arIpAddress='2001:db8::44'" > "$ALIDDNS_DDNS_SCRIPT_PATH"
if ! run_check example.com www >/dev/null 2>&1; then
	fail "valid custom IPv6 provider was rejected"
fi
[ "$(info_count)" -eq "$((before_info + 1))" ] || fail "valid IPv6 did not reach record comparison"
[ "$(update_count)" -eq "$((before_update + 1))" ] || fail "valid IPv6 did not reach update"
grep -Fq "2001:db8::44" "$UPDATE_LOG" || fail "valid IPv6 value was not passed to the update"

# A symlinked script is refused before it can inject a stale address.
rm -f "$ALIDDNS_DDNS_SCRIPT_PATH"
ln -s "$TEST_DIR/does-not-exist" "$ALIDDNS_DDNS_SCRIPT_PATH"
if run_check example.com www >/dev/null 2>&1; then
	fail "symlinked custom provider was accepted"
fi
[ "$(info_count)" -eq "$((before_info + 1))" ] || fail "symlinked script reached record comparison"
rm -f "$ALIDDNS_DDNS_SCRIPT_PATH"

# A nonempty administrator script is preserved byte-for-byte.
printf '%s\n' '# operator-owned script' "arIpAddress='198.51.100.99'" > "$ALIDDNS_DDNS_SCRIPT_PATH"
cp "$ALIDDNS_DDNS_SCRIPT_PATH" "$TEST_DIR/custom.expected"
initconfig || fail "initializer rejected a regular custom script"
cmp -s "$ALIDDNS_DDNS_SCRIPT_PATH" "$TEST_DIR/custom.expected" ||
	fail "initializer overwrote a nonempty administrator script"

# Missing and empty scripts are seeded from the packaged, read-only ROMFS file.
rm -f "$ALIDDNS_DDNS_SCRIPT_PATH"
initconfig || fail "initializer failed to seed a missing script"
cmp -s "$ALIDDNS_DDNS_SCRIPT_PATH" "$ALIDDNS_DDNS_TEMPLATE_PATH" ||
	fail "seeded script differs from the packaged ROMFS template"
[ -x "$ALIDDNS_DDNS_SCRIPT_PATH" ] || fail "seeded default script is not executable"
: > "$ALIDDNS_DDNS_SCRIPT_PATH"
initconfig || fail "initializer failed to replace an empty script"
cmp -s "$ALIDDNS_DDNS_SCRIPT_PATH" "$ALIDDNS_DDNS_TEMPLATE_PATH" ||
	fail "empty script was not replaced with the packaged template"

# A symlink destination is never overwritten, and its target stays untouched.
rm -f "$ALIDDNS_DDNS_SCRIPT_PATH"
printf '%s\n' "untouched target" > "$TEST_DIR/custom.target"
ln -s "$TEST_DIR/custom.target" "$ALIDDNS_DDNS_SCRIPT_PATH"
if initconfig; then fail "initializer accepted a symlink destination"; fi
[ -L "$ALIDDNS_DDNS_SCRIPT_PATH" ] || fail "initializer replaced the symlink itself"
[ "$(cat "$TEST_DIR/custom.target")" = "untouched target" ] ||
	fail "initializer modified the symlink target"

# Missing packaged defaults fail closed and do not create an empty override.
rm -f "$ALIDDNS_DDNS_SCRIPT_PATH"
mv "$ALIDDNS_DDNS_TEMPLATE_PATH" "$TEST_DIR/template.saved"
if initconfig; then fail "initializer succeeded without a packaged default"; fi
[ ! -e "$ALIDDNS_DDNS_SCRIPT_PATH" ] || fail "initializer created an empty script without a template"

for leftover in "$STORAGE_DIR"/.ddns_script.*; do
	[ ! -e "$leftover" ] || fail "initializer leaked staging directory $leftover"
done

echo "AliDDNS custom-script source failure and strict address validation regression: PASS"
echo "AliDDNS ROMFS template seeding, atomic replacement and symlink rejection regression: PASS"
