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
