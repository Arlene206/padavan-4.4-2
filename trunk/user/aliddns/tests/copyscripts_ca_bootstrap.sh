#!/bin/sh
set -eu

TEST_DIR="$(mktemp -d)"
trap 'rm -rf "$TEST_DIR"' EXIT HUP INT TERM
SCRIPT_DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
SOURCE="$SCRIPT_DIR/../../scripts/files/usr/bin/copyscripts.sh"
FUNCTIONS="$TEST_DIR/functions.sh"
: > "$FUNCTIONS"
sed -n '/^install_ca_trust_store() {/,/^}/p' "$SOURCE" > "$FUNCTIONS"
. "$FUNCTIONS"

COPYSCRIPTS_CERT_ARCHIVE="$TEST_DIR/certs.tgz"
COPYSCRIPTS_CERT_DIR="$TEST_DIR/ssl"
export COPYSCRIPTS_CERT_ARCHIVE COPYSCRIPTS_CERT_DIR
MOCK_MKDIR_FAIL=0
MOCK_TAR_MODE=success
TAR_CALLS=0
LOG="$TEST_DIR/log"

logger() {
	printf '%s\n' "$*" >> "$LOG"
}

mkdir() {
	if [ "$MOCK_MKDIR_FAIL" -ne 0 ]; then
		return 1
	fi
	command mkdir "$@"
}

tar() {
	TAR_CALLS=$((TAR_CALLS + 1))
	case "$MOCK_TAR_MODE" in
		success|fail-after-extract)
			command mkdir -p "$COPYSCRIPTS_CERT_DIR/certs"
			printf '%s\n' "test CA bundle" > "$COPYSCRIPTS_CERT_DIR/certs/ca-certificates.crt"
			;;
		missing-output)
			return 0
			;;
		fail|fail-after-extract)
			return 2
			;;
		*)
			return 2
			;;
	esac
	return 0
}

fail() {
	echo "CA trust-store bootstrap regression: FAIL: $1" >&2
	exit 1
}

reset_case() {
	rm -rf "$COPYSCRIPTS_CERT_DIR"
	: > "$LOG"
	TAR_CALLS=0
	MOCK_MKDIR_FAIL=0
	MOCK_TAR_MODE=success
}

# A successful extraction is accepted only after the expected bundle is readable.
reset_case
install_ca_trust_store || fail "valid CA extraction was rejected"
[ -r "$COPYSCRIPTS_CERT_DIR/certs/ca-certificates.crt" ] ||
	fail "successful extraction did not leave a readable bundle"

# A directory-creation failure must stop before invoking tar.
reset_case
MOCK_MKDIR_FAIL=1
if install_ca_trust_store; then fail "directory-creation failure was accepted"; fi
[ "$TAR_CALLS" -eq 0 ] || fail "tar ran after CA directory creation failed"

# A failed tar must remain a failure even when it left a partial readable file.
reset_case
MOCK_TAR_MODE=fail-after-extract
if install_ca_trust_store; then fail "failed tar with partial output was accepted"; fi
[ -r "$COPYSCRIPTS_CERT_DIR/certs/ca-certificates.crt" ] ||
	fail "partial extraction fixture did not create its readable marker"

# A zero tar status without the expected bundle must not be treated as success.
reset_case
MOCK_TAR_MODE=missing-output
if install_ca_trust_store; then fail "missing postcondition was accepted"; fi
[ ! -r "$COPYSCRIPTS_CERT_DIR/certs/ca-certificates.crt" ] ||
	fail "missing-output fixture unexpectedly created a bundle"

echo "CA trust-store bootstrap success regression: PASS"
echo "CA trust-store bootstrap failure propagation regression: PASS"
