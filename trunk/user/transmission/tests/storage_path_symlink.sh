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
DIR_LINK="$REAL_DIR"
transmission_require_storage_root || fail "real storage root was rejected"
DIR_LINK="$LINK_DIR"
if transmission_require_storage_root > "$TMP_DIR/root-symlink.out" 2>&1; then
    fail "symlinked storage root was accepted"
else
    status=$?
fi
[ "$status" -eq 1 ] || fail "symlinked storage root returned status $status"
[ "$(cat "$REAL_DIR/marker")" = preserve ] || fail "symlinked root validation changed target content"

PARENT_REAL="$TMP_DIR/parent-real"
PARENT_LINK="$TMP_DIR/parent-link"
mkdir -p "$PARENT_REAL/transmission"
ln -s "$PARENT_REAL" "$PARENT_LINK"
DIR_LINK="$PARENT_LINK/transmission"
if transmission_require_storage_root > "$TMP_DIR/parent-symlink.out" 2>&1; then
    fail "storage root beneath a symlinked parent was accepted"
else
    status=$?
fi
[ "$status" -eq 1 ] || fail "symlinked storage parent returned status $status"
[ -d "$PARENT_REAL/transmission" ] || fail "symlinked parent validation damaged the real path"

DIR_LINK="$TMP_DIR/missing-root"
if transmission_require_storage_root > "$TMP_DIR/missing-root.out" 2>&1; then
    fail "missing storage root was accepted"
else
    status=$?
fi
[ "$status" -eq 1 ] || fail "missing storage root returned status $status"
[ ! -e "$DIR_LINK" ] || fail "missing storage root was unexpectedly created"
grep -F 'transmission_require_storage_root || return 1' "$SCRIPT" >/dev/null ||
    fail "func_start bypasses the storage-root symlink guard"

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
echo "Transmission storage and parent directory symlinks fail closed: PASS"
