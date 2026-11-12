#!/bin/sh
set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
SOURCE_DIR="$SCRIPT_DIR/../src"
TMP_DIR=$(mktemp -d "/tmp/aliddns-hmac-sha1.XXXXXX") || exit 1
HELPER_PID=""

cleanup()
{
    if [ -n "$HELPER_PID" ]; then
        kill "$HELPER_PID" 2>/dev/null || :
        wait "$HELPER_PID" 2>/dev/null || :
    fi
    rm -rf "$TMP_DIR"
}
trap cleanup EXIT
trap 'exit 1' HUP INT TERM

cc -std=gnu89 -Wall -Wextra -Werror -fsanitize=address,undefined \
    -fno-omit-frame-pointer "$SOURCE_DIR/aliddns-hmac-sha1.c" \
    -lcrypto -o "$TMP_DIR/aliddns-hmac-sha1"

fail()
{
    echo "AliDDNS HMAC-SHA1 regression: FAIL: $1" >&2
    exit 1
}

run_case()
{
    test_key=$1
    test_message=$2
    expected=$(printf '%s' "$test_message" |
        openssl dgst -sha1 -hmac "$test_key" -binary | openssl base64) ||
        fail "OpenSSL reference computation failed"
    actual=$(printf '%s\000%s' "$test_key" "$test_message" |
        ASAN_OPTIONS=detect_leaks=1 "$TMP_DIR/aliddns-hmac-sha1") ||
        fail "EVP_MAC helper failed"
    [ "$actual" = "$expected" ] || fail "HMAC output differs from OpenSSL dgst"
}

# Covers the exact legacy key suffix and canonical query byte sequence.
run_case 'aliddns-test-secret&' \
    'GET&%2F&AccessKeyId=test-key&Action=DescribeSubDomainRecords&Format=json&Version=2015-01-09'
# Quote, backslash, ampersand, spaces and line breaks remain data.
run_case 'quote" slash\\ amp& space ' 'line one
line two'
run_case '' ''

# RFC 2202 HMAC-SHA1 test case 1.
rfc_key=$(awk 'BEGIN { for (i = 0; i < 20; i++) printf "%c", 11 }')
rfc_actual=$(printf '%s\000%s' "$rfc_key" 'Hi There' |
    ASAN_OPTIONS=detect_leaks=1 "$TMP_DIR/aliddns-hmac-sha1") ||
    fail "RFC 2202 vector failed"
[ "$rfc_actual" = 'thcxhlUFcmTii8C2+zeMjvFGvgA=' ] ||
    fail "RFC 2202 HMAC-SHA1 vector mismatch"

# Exercise several read() chunks; the message is streamed without buffering.
long_message=$(awk 'BEGIN { for (i = 0; i < 1200; i++) printf "0123456789abcdef" }')
run_case 'long-message-secret&' "$long_message"

# Hold the helper open on a FIFO and inspect the actual process argv/env.
process_secret="aliddns-process-list-secret-$$"
mkfifo "$TMP_DIR/input"
ASAN_OPTIONS=detect_leaks=1 "$TMP_DIR/aliddns-hmac-sha1" \
    < "$TMP_DIR/input" > "$TMP_DIR/output" 2> "$TMP_DIR/helper.err" &
HELPER_PID=$!
exec 3> "$TMP_DIR/input"
sleep 1
if [ -r "/proc/$HELPER_PID/cmdline" ]; then
    if tr '\000' '\n' < "/proc/$HELPER_PID/cmdline" |
        grep -F "$process_secret" >/dev/null; then
        fail "secret leaked into helper argv"
    fi
    if tr '\000' '\n' < "/proc/$HELPER_PID/environ" |
        grep -F "$process_secret" >/dev/null; then
        fail "secret leaked into helper environment"
    fi
fi
printf '%s\000%s' "$process_secret" 'GET&%2F&argv-check' >&3
exec 3>&-
wait "$HELPER_PID" || fail "process-inspection helper exited unsuccessfully"
HELPER_PID=""
expected=$(printf '%s' 'GET&%2F&argv-check' |
    openssl dgst -sha1 -hmac "$process_secret" -binary | openssl base64)
actual=$(cat "$TMP_DIR/output")
[ "$actual" = "$expected" ] || fail "process-inspection HMAC result mismatch"
echo "AccessKeySecret absent from HMAC helper argv and environment: PASS"

if printf '%s' 'missing delimiter' |
    "$TMP_DIR/aliddns-hmac-sha1" >/dev/null 2>&1; then
    fail "missing key delimiter was accepted"
fi
long_key=$(awk 'BEGIN { for (i = 0; i < 4097; i++) printf "x" }')
if printf '%s\000%s' "$long_key" 'message' |
    "$TMP_DIR/aliddns-hmac-sha1" >/dev/null 2>&1; then
    fail "oversized key was accepted"
fi

echo "AliDDNS HMAC-SHA1 EVP_MAC compatibility, byte equivalence and input-boundary regression: PASS"
