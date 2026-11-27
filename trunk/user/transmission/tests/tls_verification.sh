#!/bin/sh
set -eu

TEST_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
SCRIPT="$TEST_DIR/../transmission.sh"
TMP_DIR=$(mktemp -d "${TMPDIR:-/tmp}/transmission-tls.XXXXXX")
trap 'rm -rf "$TMP_DIR"' 0
trap 'exit 1' HUP INT TERM

fail()
{
    echo "Transmission tracker TLS verification regression: FAIL: $*" >&2
    exit 1
}

# The launcher must not enable Transmission's explicit TLS verification bypass.
if grep -Eq '^[[:space:]]*(export[[:space:]]+)?TR_CURL_SSL_NO_VERIFY[[:space:]]*=' "$SCRIPT"; then
    fail "launcher assigns the TLS verification bypass variable"
fi

# Run the exact top-level launcher preamble with an inherited insecure value.
# It must be cleared before any lifecycle function can launch a daemon.
sed '/^transmission_start_fail()/,$d' "$SCRIPT" > "$TMP_DIR/launcher-header.sh"
[ -s "$TMP_DIR/launcher-header.sh" ] || fail "could not extract launcher preamble"
(
    TR_CURL_SSL_NO_VERIFY=1
    export TR_CURL_SSL_NO_VERIFY
    . "$TMP_DIR/launcher-header.sh"
    [ "${TR_CURL_SSL_NO_VERIFY+x}" != x ]
) || fail "launcher inherited TR_CURL_SSL_NO_VERIFY and disabled tracker verification"

echo "Transmission tracker TLS certificate verification default: PASS"
