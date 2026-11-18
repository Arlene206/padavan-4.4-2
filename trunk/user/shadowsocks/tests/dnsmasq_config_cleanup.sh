#!/bin/sh
set -eu
HERE=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
HELPER="$HERE/../scripts/dnsmasq_config_lifecycle.sh"
TMP_DIR=$(mktemp -d "${TMPDIR:-/tmp}/dnsmasq-cleanup.XXXXXX")
trap 'rm -rf "$TMP_DIR"' EXIT HUP INT TERM
. "$HELPER"
fail() { echo "DNSMasq exact directive cleanup regression: FAIL: $*" >&2; exit 1; }
CONFIG="$TMP_DIR/dnsmasq.conf"
cat > "$CONFIG" <<'EOF_CONFIG'
# retain this comment: conf-dir=/tmp/dnsmasq.dom/
  conf-dir=/tmp/dnsmasq.dom/  
conf-dir=/tmp/dnsmasq.dom
conf-file=/tmp/dnsmasq.dom/keep.conf
conf-file=/tmp/dnsmasq.dom/gfwlist_list.conf
conf-dir=/tmp/dnsmasq.dom-extra/
conf-dir=/etc/storage/dnsmasq-ss.d
conf-dir=/etc/storage/dnsmasq-ss.d-extra
conf-dir=/etc/storage/dnsmasq.oversea
conf-dir=/etc/storage/dnsmasq.oversea-custom
conf-dir=/tmp/cdn
conf-dir=/tmp/cdn-cache
no-resolv
no-resolv-extra
server=127.0.0.1
server=127.0.0.1#5353
server=127.0.0.1#65353
server=127.0.0.1#65354
server=/gfwlist.example/127.0.0.1#5353
ipset=/gfwlist.example/gfwlist
address=/cdn.example.com/0.0.0.0
EOF_CONFIG
for directive in \
 'conf-dir=/tmp/dnsmasq.dom/' \
 'conf-dir=/tmp/dnsmasq.dom' \
 'conf-file=/tmp/dnsmasq.dom/gfwlist_list.conf' \
 'conf-dir=/etc/storage/dnsmasq-ss.d' \
 'conf-dir=/etc/storage/dnsmasq.oversea' \
 'conf-dir=/tmp/cdn' \
 'no-resolv' \
 'server=127.0.0.1' \
 'server=127.0.0.1#65353'; do
 ss_dnsmasq_remove_exact_directive "$CONFIG" "$directive" || fail "could not remove $directive"
done
for directive in \
 'conf-dir=/tmp/dnsmasq.dom/' \
 'conf-dir=/tmp/dnsmasq.dom' \
 'conf-file=/tmp/dnsmasq.dom/gfwlist_list.conf' \
 'conf-dir=/etc/storage/dnsmasq-ss.d' \
 'conf-dir=/etc/storage/dnsmasq.oversea' \
 'conf-dir=/tmp/cdn' \
 'no-resolv' \
 'server=127.0.0.1' \
 'server=127.0.0.1#65353'; do
 if grep -Fqx "$directive" "$CONFIG"; then fail "owned directive remained: $directive"; fi
done
for preserved in \
 '# retain this comment: conf-dir=/tmp/dnsmasq.dom/' \
 'conf-file=/tmp/dnsmasq.dom/keep.conf' \
 'conf-dir=/tmp/dnsmasq.dom-extra/' \
 'conf-dir=/etc/storage/dnsmasq-ss.d-extra' \
 'conf-dir=/etc/storage/dnsmasq.oversea-custom' \
 'conf-dir=/tmp/cdn-cache' \
 'no-resolv-extra' \
 'server=127.0.0.1#5353' \
 'server=127.0.0.1#65354' \
 'server=/gfwlist.example/127.0.0.1#5353' \
 'ipset=/gfwlist.example/gfwlist' \
 'address=/cdn.example.com/0.0.0.0'; do
 grep -Fqx "$preserved" "$CONFIG" || fail "unrelated directive/comment was removed: $preserved"
done
cp "$CONFIG" "$TMP_DIR/expected-idempotent.conf"
ss_dnsmasq_remove_exact_directive "$CONFIG" 'conf-dir=/tmp/dnsmasq.dom/' || fail "idempotent cleanup returned failure"
cmp -s "$CONFIG" "$TMP_DIR/expected-idempotent.conf" || fail "idempotent cleanup changed an already-clean file"
ss_dnsmasq_remove_exact_directive "$TMP_DIR/missing.conf" 'no-resolv' || fail "missing config should be a no-op"
if ss_dnsmasq_remove_exact_directive "$CONFIG"; then fail "invalid argument count was accepted"; else
 [ "$?" -eq 2 ] || fail "invalid argument count status was not 2"
fi
cp "$CONFIG" "$TMP_DIR/symlink-target"
ln -s "$TMP_DIR/symlink-target" "$TMP_DIR/symlink.conf"
if ss_dnsmasq_remove_exact_directive "$TMP_DIR/symlink.conf" 'no-resolv'; then fail "symlink config was modified"; else
 [ "$?" -eq 1 ] || fail "symlink rejection status was not 1"
fi
cmp -s "$CONFIG" "$TMP_DIR/symlink-target" || fail "symlink target changed"
echo "DNSMasq exact directive cleanup regression: PASS"
