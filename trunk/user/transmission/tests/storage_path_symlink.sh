#!/bin/sh
set -eu
TEST_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
SCRIPT="$TEST_DIR/../transmission.sh"
TMP_DIR=$(mktemp -d "${TMPDIR:-/tmp}/transmission-storage-path.XXXXXX")
trap 'rm -rf "$TMP_DIR"' 0
trap 'exit 1' HUP INT TERM
fail() { echo "Transmission storage path symlink regression: FAIL: $*" >&2; exit 1; }
sed '/^case "$1" in/,$d' "$SCRIPT" > "$TMP_DIR/functions.sh"
[ -s "$TMP_DIR/functions.sh" ] || fail "could not extract lifecycle functions"
logger() { :; }
CONFIG_TMP=
CONFIG_TMP_OWNED=0
. "$TMP_DIR/functions.sh"
REAL_DIR="$TMP_DIR/real-config"
NEW_DIR="$TMP_DIR/new-config"
LINK_DIR="$TMP_DIR/config-link"
DANGLING_LINK="$TMP_DIR/dangling-link"
mkdir -p "$REAL_DIR"
printf 'preserve\n' > "$REAL_DIR/marker"
ln -s "$REAL_DIR" "$LINK_DIR"
ln -s "$TMP_DIR/missing-target" "$DANGLING_LINK"
transmission_require_dir "$REAL_DIR" || fail "real directory was rejected"
transmission_require_dir "$NEW_DIR" || fail "new directory could not be created"
[ -d "$NEW_DIR" ] || fail "new directory was not published"
if transmission_require_dir "$LINK_DIR" > "$TMP_DIR/symlink.out" 2>&1; then
    fail "directory symlink was accepted"
else
    status=$?
fi
[ "$status" -eq 1 ] || fail "directory symlink returned status $status"
[ -f "$REAL_DIR/marker" ] || fail "symlink target marker disappeared"
[ "$(cat "$REAL_DIR/marker")" = preserve ] || fail "symlink target content changed"
if transmission_require_dir "$DANGLING_LINK" > "$TMP_DIR/dangling.out" 2>&1; then
    fail "dangling directory symlink was accepted"
else
    status=$?
fi
[ "$status" -eq 1 ] || fail "dangling symlink returned status $status"
[ ! -e "$TMP_DIR/missing-target" ] || fail "dangling symlink target was created"
for directory in DIR_CFG DIR_DL1 DIR_DL2 DIR_DL3; do
    grep -F "transmission_require_dir \"\$$directory\"" "$SCRIPT" >/dev/null ||
        fail "$directory bypasses the checked directory helper"
done
echo "Transmission storage directory symlinks fail closed: PASS"
