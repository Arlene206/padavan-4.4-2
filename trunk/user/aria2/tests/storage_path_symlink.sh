#!/bin/sh
set -eu

TEST_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
SCRIPT="$TEST_DIR/../aria.sh"
TMP_DIR=$(mktemp -d "${TMPDIR:-/tmp}/aria2-storage-path.XXXXXX")
trap 'rm -rf "$TMP_DIR"' 0
trap 'exit 1' HUP INT TERM

fail()
{
    echo "Aria2 storage path symlink regression: FAIL: $*" >&2
    exit 1
}

sed '/^case "$1" in/,$d' "$SCRIPT" > "$TMP_DIR/functions.sh"
[ -s "$TMP_DIR/functions.sh" ] || fail "could not extract launcher functions"
logger() { :; }
. "$TMP_DIR/functions.sh"

REAL_DIR="$TMP_DIR/real-config"
NEW_DIR="$TMP_DIR/new-config"
LINK_DIR="$TMP_DIR/config-link"
DANGLING_LINK="$TMP_DIR/dangling-link"
mkdir -p "$REAL_DIR"
printf 'preserve\n' > "$REAL_DIR/marker"
ln -s "$REAL_DIR" "$LINK_DIR"
ln -s "$TMP_DIR/missing-target" "$DANGLING_LINK"

aria_require_dir "$REAL_DIR" || fail "real directory was rejected"
aria_require_dir "$NEW_DIR" || fail "new directory could not be created"
[ -d "$NEW_DIR" ] || fail "new directory was not published"

if aria_require_dir "$LINK_DIR" > "$TMP_DIR/symlink.out" 2>&1; then
    fail "directory symlink was accepted"
else
    status=$?
fi
[ "$status" -eq 1 ] || fail "directory symlink returned status $status"
[ -f "$REAL_DIR/marker" ] || fail "symlink target marker disappeared"
[ "$(cat "$REAL_DIR/marker")" = preserve ] || fail "symlink target contents changed"

if aria_require_dir "$DANGLING_LINK" > "$TMP_DIR/dangling.out" 2>&1; then
    fail "dangling directory symlink was accepted"
else
    status=$?
fi
[ "$status" -eq 1 ] || fail "dangling directory symlink returned status $status"
[ ! -e "$TMP_DIR/missing-target" ] || fail "dangling target directory was created"

REAL_FILE="$REAL_DIR/session.list"
LINK_FILE="$TMP_DIR/session-link"
DANGLING_FILE="$TMP_DIR/session-dangling"
printf 'sentinel\n' > "$REAL_FILE"
ln -s "$REAL_FILE" "$LINK_FILE"
ln -s "$TMP_DIR/missing-file" "$DANGLING_FILE"

aria_validate_file_path "$REAL_FILE" || fail "regular file was rejected"
aria_validate_file_path "$TMP_DIR/absent-file" || fail "missing file path was rejected"
if aria_validate_file_path "$LINK_FILE" > "$TMP_DIR/file-link.out" 2>&1; then
    fail "symlinked config/session file was accepted"
else
    status=$?
fi
[ "$status" -eq 1 ] || fail "symlinked file path returned status $status"
[ "$(cat "$REAL_FILE")" = sentinel ] || fail "symlink target file was modified"
if aria_validate_file_path "$DANGLING_FILE" > "$TMP_DIR/file-dangling.out" 2>&1; then
    fail "dangling config/session file symlink was accepted"
else
    status=$?
fi
[ "$status" -eq 1 ] || fail "dangling file symlink returned status $status"
[ ! -e "$TMP_DIR/missing-file" ] || fail "dangling file symlink target was created"

for expected in \
    'aria_require_dir "$DIR_CFG"' \
    'aria_require_dir "$DIR_DL1"' \
    'aria_validate_file_path "$FILE_CONF"' \
    'aria_validate_file_path "$FILE_LIST"'; do
    grep -F "$expected" "$SCRIPT" >/dev/null || fail "missing production guard: $expected"
done
config_guard_line=$(grep -n -F 'aria_validate_file_path "$FILE_CONF"' "$SCRIPT" | cut -d: -f1)
list_guard_line=$(grep -n -F 'aria_validate_file_path "$FILE_LIST"' "$SCRIPT" | cut -d: -f1)
touch_line=$(grep -n -F 'touch "$FILE_LIST"' "$SCRIPT" | cut -d: -f1)
chmod_line=$(grep -n -F 'chmod -R 777 "$DIR_DL1"' "$SCRIPT" | cut -d: -f1)
[ "$config_guard_line" -lt "$touch_line" ] || fail "config guard must precede session-list touch"
[ "$list_guard_line" -lt "$touch_line" ] || fail "session-list guard must precede touch"
[ "$list_guard_line" -lt "$chmod_line" ] || fail "session-list guard must precede recursive chmod"

echo "Aria2 storage directory and file symlinks fail closed: PASS"
