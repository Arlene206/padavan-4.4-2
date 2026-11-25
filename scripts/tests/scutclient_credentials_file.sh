#!/bin/sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")/../.." && pwd)
TEST_TMP=$(mktemp -d "${TMPDIR:-/tmp}/scutclient-credentials-build.XXXXXX")
trap 'rm -rf "$TEST_TMP"' EXIT HUP INT TERM

${CC:-cc} -std=c99 -Wall -Wextra -Werror "$ROOT/scripts/tests/scutclient_credentials_file.c" -o "$TEST_TMP/credentials-test"
"$TEST_TMP/credentials-test"

ARCHIVE="$ROOT/trunk/user/scutclient/scutclient-a58996875c61d0cdf8ec435057befc1c87bd6863.tar.gz"
tar -xzf "$ARCHIVE" -C "$TEST_TMP"
MAIN_SOURCE=$(find "$TEST_TMP" -type f -path "*/src/main.c" -print -quit)
[ -n "$MAIN_SOURCE" ] || { echo "FAIL: pinned SCUTClient archive has no src/main.c" >&2; exit 1; }
SOURCE_DIR=$(dirname "$(dirname "$MAIN_SOURCE")")
PATCH_FILE="$ROOT/trunk/user/scutclient/patches/0001-private-credentials-file.patch"
patch --dry-run -d "$SOURCE_DIR" -p1 < "$PATCH_FILE"
patch -d "$SOURCE_DIR" -p1 < "$PATCH_FILE"
cp "$ROOT/trunk/user/scutclient/patches/scutclient_credentials.c" "$SOURCE_DIR/src/scutclient_credentials.c"
cp "$ROOT/trunk/user/scutclient/patches/scutclient_credentials.h" "$SOURCE_DIR/src/scutclient_credentials.h"
cmake -S "$SOURCE_DIR" -B "$TEST_TMP/build" -DCMAKE_BUILD_TYPE=Release -DCMAKE_POLICY_VERSION_MINIMUM=3.5
cmake --build "$TEST_TMP/build" --parallel 2
if "$TEST_TMP/build/scutclient" --credentials-file "$TEST_TMP/missing-credentials" >/dev/null 2>&1; then
  echo "FAIL: patched SCUTClient accepted a missing credential file" >&2; exit 1;
fi
echo "Pinned SCUTClient private credential overlay/build: PASS"
