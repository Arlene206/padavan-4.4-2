#!/bin/sh
set -eu

copyscripts=${1:-trunk/user/scripts/files/usr/bin/copyscripts.sh}
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' 0 HUP INT TERM

sed -n '/^install_ca_trust_store() {$/,/^}$/p' "$copyscripts" > "$tmp/ca-function.sh"
[ -s "$tmp/ca-function.sh" ] || {
	echo "could not extract install_ca_trust_store" >&2
	exit 1
}
logger()
{
	:
}
. "$tmp/ca-function.sh"

mkdir -p "$tmp/valid-root/certs"
printf 'test CA bundle\n' > "$tmp/valid-root/certs/ca-certificates.crt"
tar -czf "$tmp/valid.tgz" -C "$tmp/valid-root" certs
COPYSCRIPTS_CERT_ARCHIVE="$tmp/valid.tgz"
COPYSCRIPTS_CERT_DIR="$tmp/valid-dest"
export COPYSCRIPTS_CERT_ARCHIVE COPYSCRIPTS_CERT_DIR
install_ca_trust_store
[ -r "$tmp/valid-dest/certs/ca-certificates.crt" ]

printf 'not a gzip archive\n' > "$tmp/invalid.tgz"
COPYSCRIPTS_CERT_ARCHIVE="$tmp/invalid.tgz"
COPYSCRIPTS_CERT_DIR="$tmp/invalid-dest"
export COPYSCRIPTS_CERT_ARCHIVE COPYSCRIPTS_CERT_DIR
if install_ca_trust_store; then
	echo "invalid CA archive was accepted" >&2
	exit 1
fi

mkdir -p "$tmp/missing-root"
printf 'unrelated file\n' > "$tmp/missing-root/not-a-ca-bundle"
tar -czf "$tmp/missing.tgz" -C "$tmp/missing-root" not-a-ca-bundle
COPYSCRIPTS_CERT_ARCHIVE="$tmp/missing.tgz"
COPYSCRIPTS_CERT_DIR="$tmp/missing-dest"
export COPYSCRIPTS_CERT_ARCHIVE COPYSCRIPTS_CERT_DIR
if install_ca_trust_store; then
	echo "archive without the CA bundle was accepted" >&2
	exit 1
fi

mkdir -p "$tmp/not-a-directory"
: > "$tmp/not-a-directory/file"
COPYSCRIPTS_CERT_ARCHIVE="$tmp/valid.tgz"
COPYSCRIPTS_CERT_DIR="$tmp/not-a-directory/file/subdir"
export COPYSCRIPTS_CERT_ARCHIVE COPYSCRIPTS_CERT_DIR
if install_ca_trust_store; then
	echo "uncreatable CA directory was accepted" >&2
	exit 1
fi

grep -Fq 'copyscripts_ca_status=0' "$copyscripts"
grep -Fq 'if ! install_ca_trust_store; then' "$copyscripts"
grep -Fq 'copyscripts_ca_status=1' "$copyscripts"
grep -Fq 'exit "$copyscripts_ca_status"' "$copyscripts"
echo "copyscripts CA trust-store result propagation: PASS"

# Exercise the production default-file helper against ordinary files and
# hostile leaf/parent symlinks without touching /etc/storage.
sed -e "s|/etc/storage|$tmp/storage|g" \
	-n -e '/^seed_storage_file() {$/,/^}$/p' "$copyscripts" > "$tmp/seed-function.sh"
[ -s "$tmp/seed-function.sh" ] || {
	echo "could not extract seed_storage_file" >&2
	exit 1
}
mkdir -p "$tmp/storage/nested" "$tmp/external"
printf 'factory default\n' > "$tmp/source.sh"
logger() { :; }
. "$tmp/seed-function.sh"

seed_storage_file "$tmp/source.sh" "$tmp/storage/default.sh" 755
[ "$(cat "$tmp/storage/default.sh")" = "factory default" ]
[ "$(stat -c %a "$tmp/storage/default.sh")" = 755 ]
[ "$(find "$tmp/storage" -name 'default.sh.tmp.*' | wc -l)" -eq 0 ]

# Existing operator-managed regular files are preserved.
printf 'operator config\n' > "$tmp/storage/existing.sh"
chmod 600 "$tmp/storage/existing.sh"
seed_storage_file "$tmp/source.sh" "$tmp/storage/existing.sh" 755
[ "$(cat "$tmp/storage/existing.sh")" = "operator config" ]
[ "$(stat -c %a "$tmp/storage/existing.sh")" = 600 ]

# A symlink to an existing sensitive file and a dangling symlink are both
# refused without following either destination.
printf 'outside sentinel\n' > "$tmp/external/sentinel"
ln -s "$tmp/external/sentinel" "$tmp/storage/linked.sh"
if seed_storage_file "$tmp/source.sh" "$tmp/storage/linked.sh" 755; then
	echo "symlink destination was accepted" >&2
	exit 1
fi
[ "$(cat "$tmp/external/sentinel")" = "outside sentinel" ]
[ -L "$tmp/storage/linked.sh" ]
ln -s "$tmp/external/absent" "$tmp/storage/dangling.sh"
if seed_storage_file "$tmp/source.sh" "$tmp/storage/dangling.sh" 755; then
	echo "dangling symlink destination was accepted" >&2
	exit 1
fi
[ ! -e "$tmp/external/absent" ]
[ -L "$tmp/storage/dangling.sh" ]

# A symlinked parent must not redirect a nested default-file write.
ln -s "$tmp/external" "$tmp/storage/parent-link"
if seed_storage_file "$tmp/source.sh" "$tmp/storage/parent-link/escaped.sh" 755; then
	echo "symlink parent directory was accepted" >&2
	exit 1
fi
[ ! -e "$tmp/external/escaped.sh" ]

# Non-regular destinations and paths outside the storage root fail closed.
mkdir "$tmp/storage/not-a-file"
if seed_storage_file "$tmp/source.sh" "$tmp/storage/not-a-file" 755; then
	echo "directory destination was accepted" >&2
	exit 1
fi
if seed_storage_file "$tmp/source.sh" "$tmp/outside.sh" 755; then
	echo "destination outside storage root was accepted" >&2
	exit 1
fi

grep -Fq 'seed_storage_file "/etc_ro/koolproxy_rules_script.sh" "/etc/storage/koolproxy_rules_script.sh" 755' "$copyscripts"
grep -Fq 'seed_storage_file "/etc_ro/oversea_list.conf" "/etc/storage/dnsmasq.oversea/oversea_list.conf" 755' "$copyscripts"
echo "copyscripts default-file symlink-safe publication: PASS"
