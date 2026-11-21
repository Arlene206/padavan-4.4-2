#!/bin/sh
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
. "$HERE/../smartdns_dnsmasq_config.sh"
T=$(mktemp -d "${TMPDIR:-/tmp}/smartdns-dnsmasq.XXXXXX")
trap 'rm -rf "$T"' 0
trap 'exit 1' HUP INT TERM
fail() { echo "SmartDNS DNSMasq regression: FAIL: $*" >&2; exit 1; }
pass() { echo "SmartDNS DNSMasq regression: PASS: $*"; }
count_line() { grep -Fxc "$1" "$2" || :; }
write_managed_fixture() {
	cat > "$1" <<'CONFIG'
# user resolver settings
no-resolv
server=127.0.0.1#6053
port=0
# smartdns-managed-dnsmasq-begin
no-resolv
server=127.0.0.1#6053
port=0
# smartdns-managed-dnsmasq-end
# user tail
no-resolv-extra
http-port=0
CONFIG
}
F="$T/dnsmasq.conf"
write_managed_fixture "$F"
smartdns_dnsmasq_remove_saved_directives "$F" 1 6053 managed-v1 || fail "managed block cleanup failed"
[ "$(count_line no-resolv "$F")" -eq 1 ] || fail "user no-resolv directive was removed"
[ "$(count_line 'server=127.0.0.1#6053' "$F")" -eq 1 ] || fail "user upstream was removed"
[ "$(count_line port=0 "$F")" -eq 1 ] || fail "user port=0 was removed"
! grep -Fq 'smartdns-managed-dnsmasq-' "$F" || fail "managed markers remained"
grep -Fxq '# user tail' "$F" || fail "user tail was removed"
grep -Fxq 'no-resolv-extra' "$F" || fail "near-match directive was removed"
cp "$F" "$T/after-managed"
smartdns_dnsmasq_remove_saved_directives "$F" 1 6053 managed-v1 || fail "repeat managed cleanup failed"
cmp -s "$F" "$T/after-managed" || fail "managed cleanup is not idempotent"
pass "marked cleanup preserves user-owned exact directives and is idempotent"

cat > "$F" <<'CONFIG'
port=0
no-resolv
# smartdns-managed-dnsmasq-begin
port=0
# smartdns-managed-dnsmasq-end
server=127.0.0.1#53
CONFIG
smartdns_dnsmasq_remove_saved_directives "$F" 2 53 managed-v1 || fail "port 53 marked cleanup failed"
[ "$(count_line port=0 "$F")" -eq 1 ] || fail "user port=0 was removed for port 53"
grep -Fxq no-resolv "$F" || fail "unmanaged no-resolv was removed"
grep -Fxq 'server=127.0.0.1#53' "$F" || fail "unmanaged server directive was removed"
pass "port 53 marker cleanup leaves same-valued user directives intact"

cat > "$F" <<'CONFIG'
no-resolv
server=127.0.0.1#6053
port=0
# no-resolv
no-resolv-extra
server=127.0.0.1#60530
port=0 # user note
http-port=0
CONFIG
smartdns_dnsmasq_remove_saved_directives "$F" 1 6053 legacy || fail "legacy exact cleanup failed"
! grep -Fxq no-resolv "$F" || fail "legacy no-resolv remained"
! grep -Fxq 'server=127.0.0.1#6053' "$F" || fail "legacy upstream remained"
grep -Fxq '# no-resolv' "$F" || fail "legacy cleanup removed a comment"
grep -Fxq no-resolv-extra "$F" || fail "legacy cleanup removed a near match"
grep -Fxq 'server=127.0.0.1#60530' "$F" || fail "legacy cleanup removed another port"
grep -Fxq 'port=0' "$F" || fail "legacy cleanup removed port=0 for a non-53 port"
grep -Fxq 'port=0 # user note' "$F" || fail "legacy cleanup removed annotated port"
pass "upgrade cleanup removes only exact legacy directives"

cat > "$F" <<'CONFIG'
user-before
# smartdns-managed-dnsmasq-begin
no-resolv
user-after
CONFIG
cp "$F" "$T/bad-marker.before"
if smartdns_dnsmasq_remove_saved_directives "$F" 1 6053 managed-v1; then fail "incomplete managed block accepted"; fi
cmp -s "$F" "$T/bad-marker.before" || fail "incomplete block changed live config"
pass "malformed marker block fails closed"

ln -s "$F" "$T/config-link"
if smartdns_dnsmasq_remove_saved_directives "$T/config-link" 1 6053 managed-v1; then fail "symlink config accepted"; fi
[ -L "$T/config-link" ] || fail "symlink replaced"
cmp -s "$F" "$T/bad-marker.before" || fail "symlink target changed"
pass "symlink configuration is refused"

smartdns_dnsmasq_remove_saved_directives "$T/missing.conf" 1 6053 managed-v1 || fail "missing file was not a no-op"
pass "missing configuration is a safe no-op"

# AWK's END block must not replace an input/read failure with a successful
# no-match result. Exercise every preliminary scan using a failing awk shim.
mkdir "$T/failing-bin"
cat > "$T/failing-bin/awk" <<'AWK_FAIL'
#!/bin/sh
exit 37
AWK_FAIL
chmod 0700 "$T/failing-bin/awk"
saved_path=$PATH
PATH="$T/failing-bin:$PATH"
export PATH

cat > "$F" <<'CONFIG'
no-resolv
server=127.0.0.1#6053
# smartdns-managed-dnsmasq-begin
no-resolv
# smartdns-managed-dnsmasq-end
user-tail
CONFIG
cp "$F" "$T/awk-failure.before"
if smartdns_dnsmasq_remove_exact_directives "$F" no-resolv; then
	scan_status=0
else
	scan_status=$?
fi
[ "$scan_status" -eq 37 ] || fail "exact-directive scan failure status was masked: $scan_status"
cmp -s "$F" "$T/awk-failure.before" || fail "exact-directive scan failure changed live config"

if smartdns_dnsmasq_remove_managed_block "$F"; then
	scan_status=0
else
	scan_status=$?
fi
[ "$scan_status" -eq 37 ] || fail "managed-block scan failure status was masked: $scan_status"
cmp -s "$F" "$T/awk-failure.before" || fail "managed-block scan failure changed live config"

if smartdns_dnsmasq_publish_managed_block "$F" no-resolv; then
	scan_status=0
else
	scan_status=$?
fi
[ "$scan_status" -eq 37 ] || fail "managed-block publish scan failure status was masked: $scan_status"
cmp -s "$F" "$T/awk-failure.before" || fail "publish scan failure changed live config"

PATH=$saved_path
export PATH
pass "all DNSMasq preliminary AWK scan failures propagate without changing live config"

cat > "$F" <<'CONFIG'
# user resolver before SmartDNS
no-resolv
server=127.0.0.1#6053
port=0
# user resolver after SmartDNS
CONFIG
cp "$F" "$T/publish.before"
smartdns_dnsmasq_publish_managed_block "$F" no-resolv 'server=127.0.0.1#6053' port=0 || fail "atomic managed-block publication failed"
grep -Fxq '# smartdns-managed-dnsmasq-begin' "$F" || fail "managed begin marker missing"
grep -Fxq '# smartdns-managed-dnsmasq-end' "$F" || fail "managed end marker missing"
[ "$(grep -Fxc no-resolv "$F")" -eq 2 ] || fail "publisher replaced or dropped user resolver"
[ "$(grep -Fxc 'server=127.0.0.1#6053' "$F")" -eq 2 ] || fail "publisher replaced or dropped user upstream"
[ "$(grep -Fxc port=0 "$F")" -eq 2 ] || fail "publisher replaced or dropped user port directive"
smartdns_dnsmasq_remove_saved_directives "$F" 1 6053 managed-v1 || fail "published managed-block cleanup failed"
cmp -s "$F" "$T/publish.before" || fail "managed block cleanup did not restore exact original file"
pass "managed-block publication is atomic and preserves same-valued user directives"

write_managed_fixture "$F"
cp "$F" "$T/publish-existing.before"
if smartdns_dnsmasq_publish_managed_block "$F" no-resolv; then fail "publisher accepted an existing marker block"; fi
cmp -s "$F" "$T/publish-existing.before" || fail "refused duplicate publication changed the file"
pass "duplicate or interrupted managed blocks fail closed"

write_managed_fixture "$F"
cp "$F" "$T/before-mv"
mv() { return 37; }
if smartdns_dnsmasq_remove_saved_directives "$F" 1 6053 managed-v1; then fail "rename failure ignored"; else status=$?; fi
unset -f mv
[ "$status" -eq 37 ] || fail "rename status not propagated"
cmp -s "$F" "$T/before-mv" || fail "rename failure modified live config"
for tmp in "$F".tmp.*; do [ ! -e "$tmp" ] || fail "temporary file leaked"; done
pass "rename failure preserves original config and cleans temporary file"

cat > "$F" <<'CONFIG'
user-before
user-after
CONFIG
cp "$F" "$T/publish-before-mv"
mv() { return 41; }
if smartdns_dnsmasq_publish_managed_block "$F" no-resolv; then fail "publish rename failure ignored"; else status=$?; fi
unset -f mv
[ "$status" -eq 41 ] || fail "publish rename status not propagated"
cmp -s "$F" "$T/publish-before-mv" || fail "failed publication changed live DNSMasq configuration"
for tmp in "$F".tmp.*; do [ ! -e "$tmp" ] || fail "publish leaked temporary file"; done
pass "failed managed-block publication leaves original config intact"

[ "$(smartdns_dnsmasq_normalize_port 00053)" = 53 ] || fail "port normalization failed"
for bad in '' 0 65536 999999 abc 000000; do
	if smartdns_dnsmasq_normalize_port "$bad" >/dev/null; then fail "invalid port accepted"; fi
done
echo "SmartDNS DNSMasq configuration preservation regression: PASS"
