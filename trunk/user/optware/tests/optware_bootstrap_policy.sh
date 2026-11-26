#!/bin/sh
set -eu

test_dir=$(mktemp -d /tmp/optware-bootstrap-test.XXXXXX)
trap 'rm -rf "$test_dir"' 0
trap 'exit 1' HUP INT TERM
. "$(dirname "$0")/../optware_bootstrap_policy.sh"

source_installer="$test_dir/generic.raw"
patched_installer="$test_dir/generic.sh"
cat > "$source_installer" <<'EOF'
#!/bin/sh
TYPE='alternative'
ARCH=mipselsf-k3.4
URL=http://bin.entware.net/${ARCH}/installer
wget $URL/opkg -O /opt/bin/opkg
chmod 755 /opt/bin/opkg
wget $URL/opkg.conf -O /opt/etc/opkg.conf
/opt/bin/opkg update
/opt/bin/opkg install busybox
/opt/bin/opkg install entware-opt
echo done
EOF

optware_bootstrap_patch_installer "$source_installer" "$patched_installer"
[ "$(sed -n '1p' "$patched_installer")" = '#!/bin/sh' ]
[ "$(sed -n '2p' "$patched_installer")" = 'set -e' ]
grep -q '^URL=https://bin.entware.net/${ARCH}/installer$' "$patched_installer"
[ "$(grep -c '^wget -T 60 ' "$patched_installer")" -eq 2 ]
grep -q '^if ! sed -i "s|http://bin.entware.net/|https://bin.entware.net/|g" /opt/etc/opkg.conf; then$' "$patched_installer"
grep -q '^if grep -q "http://" /opt/etc/opkg.conf; then$' "$patched_installer"
! grep -q 'http://' "$patched_installer"
sh -n "$patched_installer"
echo "Entware installer gains fail-fast HTTPS downloads and bounded timeouts: PASS"

cat > "$test_dir/opkg.conf" <<'EOF'
src/gz entware http://bin.entware.net/mipselsf-k3.4/installer
src/gz entware-extra http://bin.entware.net/mipselsf-k3.4/installer-extra
EOF
sed -i 's|http://bin.entware.net/|https://bin.entware.net/|g' "$test_dir/opkg.conf"
if grep -q 'http://' "$test_dir/opkg.conf"; then
    echo "insecure package feed survived the TLS rewrite" >&2
    exit 1
fi
grep -q '^src/gz entware https://' "$test_dir/opkg.conf"
grep -q '^src/gz entware-extra https://' "$test_dir/opkg.conf"
echo "Entware package feeds are upgraded to HTTPS before opkg update: PASS"

sed 's|http://bin.entware.net/|http://attacker.invalid/|' "$source_installer" > "$test_dir/unknown.raw"
if optware_bootstrap_patch_installer "$test_dir/unknown.raw" "$test_dir/unknown.sh"; then
    echo "unknown HTTP origin was accepted" >&2
    exit 1
fi
[ ! -e "$test_dir/unknown.sh" ]
echo "unknown installer HTTP origins fail closed: PASS"

grep -v 'wget $URL/opkg.conf' "$source_installer" > "$test_dir/missing.raw"
if optware_bootstrap_patch_installer "$test_dir/missing.raw" "$test_dir/missing.sh"; then
    echo "installer without opkg.conf download anchor was accepted" >&2
    exit 1
fi
[ ! -e "$test_dir/missing.sh" ]
cat "$source_installer" > "$test_dir/duplicate.raw"
printf '%s\n' 'wget $URL/opkg.conf -O /opt/etc/opkg.conf' >> "$test_dir/duplicate.raw"
if optware_bootstrap_patch_installer "$test_dir/duplicate.raw" "$test_dir/duplicate.sh"; then
    echo "duplicate installer download anchors were accepted" >&2
    exit 1
fi
[ ! -e "$test_dir/duplicate.sh" ]
echo "missing and duplicate upstream download anchors fail closed: PASS"

ln -s "$source_installer" "$test_dir/source-link"
if optware_bootstrap_patch_installer "$test_dir/source-link" "$test_dir/link.sh"; then
    echo "symlink installer source was accepted" >&2
    exit 1
fi
printf '%s\n' 'preserve-existing' > "$test_dir/existing.sh"
if optware_bootstrap_patch_installer "$source_installer" "$test_dir/existing.sh"; then
    echo "existing installer destination was overwritten" >&2
    exit 1
fi
[ "$(cat "$test_dir/existing.sh")" = 'preserve-existing' ]
echo "installer patching rejects symlink inputs and preserves existing targets: PASS"
