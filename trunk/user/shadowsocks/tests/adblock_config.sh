#!/bin/sh
set -eu

TEST_DIR=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
TMP_DIR=$(mktemp -d "${TMPDIR:-/tmp}/shadowsocks-adblock.XXXXXX")
trap 'rm -rf "$TMP_DIR"' EXIT HUP INT TERM
SCRIPT="$TEST_DIR/../scripts/shadowsocks.sh"
OUTPUT="$TMP_DIR/anti-ad-for-dnsmasq.conf"

sed -n '/^# Publish either a dnsmasq address configuration/,/^}/p' "$SCRIPT" > "$TMP_DIR/helper.sh"
[ -s "$TMP_DIR/helper.sh" ] || {
	echo "could not extract shell-commented production adblock publisher" >&2
	exit 1
}
. "$TMP_DIR/helper.sh"

cat > "$TMP_DIR/dnsmasq.conf" <<'EOF_DNSMASQ'
# Adblock rules for dnsmasq
address=/ads.example.com/0.0.0.0
address=/tracker.example.net/0.0.0.0
EOF_DNSMASQ
cat > "$TMP_DIR/dnsmasq.expected" <<'EOF_EXPECTED'
address=/ads.example.com/0.0.0.0
address=/tracker.example.net/0.0.0.0
EOF_EXPECTED
ss_publish_adblock_config "$TMP_DIR/dnsmasq.conf" "$OUTPUT"
cmp -s "$TMP_DIR/dnsmasq.expected" "$OUTPUT" || {
	echo "dnsmasq address rules were not preserved safely" >&2
	exit 1
}

cat > "$TMP_DIR/adblock.txt" <<'EOF_ADBLOCK'
! Adblock list comment
||ads.example.com^
||tracker.example.net^
@@||allowed.example.org^
||wild.*.example.com^
||path.example.com^$script
EOF_ADBLOCK
cat > "$TMP_DIR/expected.conf" <<'EOF_EXPECTED'
address=/ads.example.com/0.0.0.0
address=/tracker.example.net/0.0.0.0
EOF_EXPECTED
ss_publish_adblock_config "$TMP_DIR/adblock.txt" "$OUTPUT"
cmp -s "$TMP_DIR/expected.conf" "$OUTPUT" || {
	echo "simple Adblock domain rules were not converted correctly" >&2
	exit 1
}

cp "$OUTPUT" "$TMP_DIR/last-good.conf"
# Mixed dnsmasq config must not smuggle arbitrary directives.
cat > "$TMP_DIR/injected-dnsmasq.conf" <<'EOF_INJECTED'
address=/ads.example.com/0.0.0.0
dhcp-script=/tmp/attacker.sh
conf-file=/tmp/attacker.conf
EOF_INJECTED
if ss_publish_adblock_config "$TMP_DIR/injected-dnsmasq.conf" "$OUTPUT"; then
	echo "untrusted dnsmasq directives were incorrectly accepted" >&2
	exit 1
fi
cmp -s "$TMP_DIR/last-good.conf" "$OUTPUT" || {
	echo "untrusted dnsmasq directives replaced last-good config" >&2
	exit 1
}

printf '%s\n' '! comments only' '@@||allowed.example.com^' '||wild.*.example.com^' > "$TMP_DIR/unsupported.txt"
if ss_publish_adblock_config "$TMP_DIR/unsupported.txt" "$OUTPUT"; then
	echo "unsupported-only Adblock input was incorrectly accepted" >&2
	exit 1
fi
cmp -s "$TMP_DIR/last-good.conf" "$OUTPUT" || {
	echo "invalid Adblock input replaced the last known-good config" >&2
	exit 1
}
: > "$TMP_DIR/empty.txt"
if ss_publish_adblock_config "$TMP_DIR/empty.txt" "$OUTPUT"; then
	echo "empty Adblock input was incorrectly accepted" >&2
	exit 1
fi
cmp -s "$TMP_DIR/last-good.conf" "$OUTPUT" || {
	echo "empty input replaced the last known-good config" >&2
	exit 1
}
for temp_file in "$OUTPUT".tmp.* "$OUTPUT".rules.*; do
	[ ! -e "$temp_file" ] || {
		echo "temporary publisher file leaked: $temp_file" >&2
		exit 1
	}
done

echo "Shadowsocks adblock config publication regression: PASS"
