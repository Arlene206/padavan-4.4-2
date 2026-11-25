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
[ -f "$tmp/valid-dest/certs/ca-certificates.crt" ]
[ -s "$tmp/valid-dest/certs/ca-certificates.crt" ]
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

# An archive may exit successfully while containing an empty CA bundle.
mkdir -p "$tmp/empty-root/certs"
: > "$tmp/empty-root/certs/ca-certificates.crt"
tar -czf "$tmp/empty.tgz" -C "$tmp/empty-root" certs
COPYSCRIPTS_CERT_ARCHIVE="$tmp/empty.tgz"
COPYSCRIPTS_CERT_DIR="$tmp/empty-dest"
export COPYSCRIPTS_CERT_ARCHIVE COPYSCRIPTS_CERT_DIR
if install_ca_trust_store; then
	echo "empty CA bundle was accepted" >&2
	exit 1
fi

# A directory can be readable too, but it is not a certificate bundle.
mkdir -p "$tmp/directory-root/certs/ca-certificates.crt"
tar -czf "$tmp/directory.tgz" -C "$tmp/directory-root" certs
COPYSCRIPTS_CERT_ARCHIVE="$tmp/directory.tgz"
COPYSCRIPTS_CERT_DIR="$tmp/directory-dest"
export COPYSCRIPTS_CERT_ARCHIVE COPYSCRIPTS_CERT_DIR
if install_ca_trust_store; then
	echo "directory at CA bundle path was accepted" >&2
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
grep -Fq 'copyscripts_seed_status=0' "$copyscripts"
grep -Fq 'if [ "$copyscripts_ca_status" -ne 0 ] || [ "$copyscripts_seed_status" -ne 0 ]; then' "$copyscripts"
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
copyscripts_seed_status=0
logger() { :; }
. "$tmp/seed-function.sh"

seed_storage_file "$tmp/source.sh" "$tmp/storage/default.sh" 755
[ "$copyscripts_seed_status" -eq 0 ]
[ "$(cat "$tmp/storage/default.sh")" = "factory default" ]
[ "$(stat -c %a "$tmp/storage/default.sh")" = 755 ]
[ "$(find "$tmp/storage" -name 'default.sh.tmp.*' | wc -l)" -eq 0 ]

# Existing operator-managed regular files are preserved.
printf 'operator config\n' > "$tmp/storage/existing.sh"
chmod 600 "$tmp/storage/existing.sh"
seed_storage_file "$tmp/source.sh" "$tmp/storage/existing.sh" 755
[ "$(cat "$tmp/storage/existing.sh")" = "operator config" ]
[ "$(stat -c %a "$tmp/storage/existing.sh")" = 600 ]

# FRP credentials must use exclusive publication and stay private even when
# an operator-managed regular file already exists.
copyscripts_seed_status=0
seed_storage_file "$tmp/source.sh" "$tmp/storage/frp-new.sh" 700 1
[ "$copyscripts_seed_status" -eq 0 ]
[ "$(cat "$tmp/storage/frp-new.sh")" = "factory default" ]
[ "$(stat -c %a "$tmp/storage/frp-new.sh")" = 700 ]
[ "$(find "$tmp/storage" -name 'frp-new.sh.tmp.*' | wc -l)" -eq 0 ]

printf 'operator FRP credentials\n' > "$tmp/storage/frp-existing.sh"
chmod 644 "$tmp/storage/frp-existing.sh"
seed_storage_file "$tmp/source.sh" "$tmp/storage/frp-existing.sh" 700 1
[ "$(cat "$tmp/storage/frp-existing.sh")" = "operator FRP credentials" ]
[ "$(stat -c %a "$tmp/storage/frp-existing.sh")" = 700 ]

# A failed private-mode repair must block boot preparation.
printf 'operator FRP credentials\n' > "$tmp/storage/frp-mode-failure.sh"
chmod 644 "$tmp/storage/frp-mode-failure.sh"
copyscripts_seed_status=0
chmod() { return 1; }
if seed_storage_file "$tmp/source.sh" "$tmp/storage/frp-mode-failure.sh" 700 1; then
	echo "FRP mode-repair failure was accepted" >&2
	exit 1
fi
unset -f chmod
[ "$copyscripts_seed_status" -eq 1 ]
[ "$(stat -c %a "$tmp/storage/frp-mode-failure.sh")" = 644 ]

# The packaged DDNS default uses the same atomic, no-follow publisher.
# An operator-managed regular file remains untouched; a symlink is refused.
copyscripts_seed_status=0
seed_storage_file "$tmp/source.sh" "$tmp/storage/ddns_script.sh" 755
[ "$copyscripts_seed_status" -eq 0 ]
[ "$(cat "$tmp/storage/ddns_script.sh")" = "factory default" ]
printf '%s\n' 'operator DDNS script' > "$tmp/storage/ddns-existing.sh"
chmod 600 "$tmp/storage/ddns-existing.sh"
seed_storage_file "$tmp/source.sh" "$tmp/storage/ddns-existing.sh" 755
[ "$(cat "$tmp/storage/ddns-existing.sh")" = "operator DDNS script" ]
[ "$(stat -c %a "$tmp/storage/ddns-existing.sh")" = 600 ]
printf 'outside sentinel\n' > "$tmp/external/sentinel"
ln -s "$tmp/external/sentinel" "$tmp/storage/ddns-linked.sh"
copyscripts_seed_status=0
if seed_storage_file "$tmp/source.sh" "$tmp/storage/ddns-linked.sh" 755; then
	echo "DDNS symlink destination was accepted" >&2
	exit 1
fi
[ "$copyscripts_seed_status" -eq 1 ]
[ "$(cat "$tmp/external/sentinel")" = "outside sentinel" ]
[ -L "$tmp/storage/ddns-linked.sh" ]

# A symlink to an existing sensitive file and a dangling symlink are both
# refused without following either destination.
printf 'outside sentinel\n' > "$tmp/external/sentinel"
ln -s "$tmp/external/sentinel" "$tmp/storage/linked.sh"
copyscripts_seed_status=0
if seed_storage_file "$tmp/source.sh" "$tmp/storage/linked.sh" 755; then
	echo "symlink destination was accepted" >&2
	exit 1
fi
[ "$copyscripts_seed_status" -eq 1 ]
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
grep -Fq 'seed_storage_file "/etc_ro/frp_script.sh" "$frp_script_path" 700 1' "$copyscripts"
grep -Fq 'seed_storage_file "/etc_ro/ddns_script.sh" "$ddns_script_path" 755' "$copyscripts"
if grep -Fq 'cp -f /etc_ro/ddns_script.sh "$ddns_script_path"' "$copyscripts"; then
	echo "DDNS default bypasses the exclusive storage publication helper" >&2
	exit 1
fi
if grep -Fq 'cp -f /etc_ro/frp_script.sh "$frp_script_path"' "$copyscripts"; then
	echo "FRP defaults bypass the exclusive storage publication helper" >&2
	exit 1
fi
echo "copyscripts default-file and private FRP publication: PASS"

# Exercise the exact final status gate. A failed CA extraction or a failed
# default-file publication must both be visible to PID 1; clean setup succeeds.
sed -n '/copyscripts_ca_status.*copyscripts_seed_status/,/^exit 0$/p' \
	"$copyscripts" > "$tmp/status-gate.sh"
[ -s "$tmp/status-gate.sh" ] || {
	echo "could not extract copyscripts result gate" >&2
	exit 1
}
expect_status_gate()
{
	expected=$1
	ca_status=$2
	seed_status=$3
	if ( copyscripts_ca_status=$ca_status; copyscripts_seed_status=$seed_status; . "$tmp/status-gate.sh" ); then
		actual=0
	else
		actual=$?
	fi
	[ "$actual" -eq "$expected" ] || {
		echo "copyscripts result gate returned $actual, expected $expected (CA=$ca_status seed=$seed_status)" >&2
		exit 1
	}
}
expect_status_gate 0 0 0
expect_status_gate 1 1 0
expect_status_gate 1 0 1
echo "copyscripts CA and default-seed status aggregation: PASS"
