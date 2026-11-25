#!/bin/sh
set -eu
script_dir=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
repo_root=$(CDPATH= cd -- "$script_dir/../../../.." && pwd)
sandbox=$(mktemp -d "${TMPDIR:-/tmp}/mtd-storage-load.XXXXXX")
real_rm=$(command -v rm)
trap 'status=$?; "$real_rm" -rf "$sandbox"; exit "$status"' EXIT
trap 'exit 129' HUP
trap 'exit 130' INT
trap 'exit 143' TERM
mkdir -p "$sandbox/etc/storage" "$sandbox/private" "$sandbox/bin" "$sandbox/archive"
printf '%s\n' 'mtd100: 00010000 00010000 "Storage"' > "$sandbox/proc-mtd"
printf '%s\n' 'previous configuration' > "$sandbox/etc/storage/previous.conf"
printf '%s\n' 'previous-checkpoint' > "$sandbox/private/storage_md5"
: > "$sandbox/private/.storage_locked"
printf '%s\n' 'new configuration' > "$sandbox/archive/config.conf"
printf '%s\n' 'second entry' > "$sandbox/archive/secondary.conf"
real_tar=$(command -v tar)
"$real_tar" -cf "$sandbox/full.tar" -C "$sandbox/archive" config.conf secondary.conf
sed \
  -e "s|^dir_storage=.*|dir_storage=\"$sandbox/etc/storage\"|" \
  -e "s|^slk=.*|slk=\"$sandbox/private/.storage_locked\"|" \
  -e "s|^lock_dir=.*|lock_dir=\"$sandbox/private\"|" \
  -e "s|^tmp=.*|tmp=\"$sandbox/private/storage.tar\"|" \
  -e "s|^hsh=.*|hsh=\"$sandbox/private/storage_md5\"|" \
  -e "s|cat /proc/mtd|cat \"$sandbox/proc-mtd\"|" \
  "$repo_root/trunk/user/scripts/files/sbin/mtd_storage.sh" > "$sandbox/mtd_storage.sh"

cat > "$sandbox/bin/logger" <<'EOF'
#!/bin/sh
exit 0
EOF
cat > "$sandbox/bin/rm" <<'EOF_RM'
#!/bin/sh
for arg do
	[ "${FAIL_RM_PATH:-}" != "$arg" ] || exit 1
done
exec "$REAL_RM" "$@"
EOF_RM
cat > "$sandbox/bin/bzcat" <<'EOF'
#!/bin/sh
printf '%s\n' "$*" >> "$BZCAT_LOG"
[ "${BZCAT_EMPTY:-0}" = 1 ] || cat "$BZCAT_FIXTURE"
exit "${BZCAT_STATUS:-0}"
EOF
cat > "$sandbox/bin/tar" <<'EOF'
#!/bin/sh
printf '%s\n' "$*" >> "$TAR_LOG"
if [ "${TAR_FAIL:-0}" = 1 ]; then
	target=
	want_target=0
	for arg do
		if [ "$want_target" = 1 ]; then target=$arg; want_target=0
		elif [ "$arg" = "-C" ]; then want_target=1
		fi
	done
	[ -n "$target" ] || exit 99
	printf '%s\n' 'partial extraction must not be published' > "$target/partial-extract.conf"
	exit 2
fi
exec "$REAL_TAR" "$@"
EOF
cat > "$sandbox/bin/mv" <<'EOF'
#!/bin/sh
count=0
[ ! -f "$MV_COUNT_FILE" ] || count=$(/bin/cat "$MV_COUNT_FILE")
count=$((count + 1))
printf '%s\n' "$count" > "$MV_COUNT_FILE"
[ "${MV_FAIL_AT:-0}" != "$count" ] || exit 1
exec /bin/mv "$@"
EOF
chmod +x "$sandbox/bin/logger" "$sandbox/bin/rm" "$sandbox/bin/bzcat" "$sandbox/bin/tar" "$sandbox/bin/mv"
export REAL_RM="$real_rm" PATH="$sandbox/bin:$PATH" BZCAT_FIXTURE="$sandbox/full.tar"
export TAR_LOG="$sandbox/tar.log" REAL_TAR="$real_tar" MV_COUNT_FILE="$sandbox/mv-count"
export BZCAT_LOG="$sandbox/bzcat.log"
: > "$BZCAT_LOG"

run_load()
{
	if [ -n "${BUSYBOX_BIN:-}" ]; then "$BUSYBOX_BIN" ash "$sandbox/mtd_storage.sh" load
	else sh "$sandbox/mtd_storage.sh" load
	fi
}
assert_old_storage()
{
	[ "$(cat "$sandbox/etc/storage/previous.conf")" = "previous configuration" ] || exit 1
	[ ! -e "$sandbox/etc/storage/partial-extract.conf" ] || exit 1
	[ "$(cat "$sandbox/private/storage_md5")" = "previous-checkpoint" ] || exit 1
	[ -f "$sandbox/private/.storage_locked" ] || exit 1
	for scratch in "$sandbox/etc"/.storage.load.* "$sandbox/etc"/.storage.load-backup.*; do
		[ ! -e "$scratch" ] || { echo "failed load left scratch data: $scratch" >&2; exit 1; }
	done
}

# Exact partition lookup must reject missing, malformed, or duplicate labels
# before reading from a device node.
printf '%s\n' 'mtd9: 00010000 00010000 "StorageBackup"' > "$sandbox/proc-mtd"
: > "$BZCAT_LOG"
if run_load; then echo "missing exact Storage partition unexpectedly succeeded" >&2; exit 1; fi
[ ! -s "$BZCAT_LOG" ] || { echo "missing partition reached bzcat" >&2; exit 1; }

printf '%s\n' 'mtd100: 00010000 00010000 "Storage"' 'mtd101: 00010000 00010000 "Storage"' > "$sandbox/proc-mtd"
: > "$BZCAT_LOG"
if run_load; then echo "duplicate Storage partitions unexpectedly succeeded" >&2; exit 1; fi
[ ! -s "$BZCAT_LOG" ] || { echo "ambiguous partition reached bzcat" >&2; exit 1; }

printf '%s\n' 'mtdbad: 00010000 00010000 "Storage"' > "$sandbox/proc-mtd"
: > "$BZCAT_LOG"
if run_load; then echo "malformed Storage MTD record unexpectedly succeeded" >&2; exit 1; fi
[ ! -s "$BZCAT_LOG" ] || { echo "malformed partition reached bzcat" >&2; exit 1; }

printf '%s\n' 'mtd100: 00010000 00010000 "Storage"' > "$sandbox/proc-mtd"
: > "$BZCAT_LOG"
export BZCAT_STATUS=2 TAR_FAIL=0 MV_FAIL_AT=0 BZCAT_EMPTY=1
: > "$TAR_LOG"
if run_load; then echo "blank MTD unexpectedly loaded" >&2; exit 1; fi
[ ! -s "$TAR_LOG" ] || { echo "tar ran after blank MTD decompression failed" >&2; exit 1; }
[ "$(cat "$sandbox/etc/storage/previous.conf")" = "previous configuration" ] || exit 1
[ ! -e "$sandbox/etc/storage/partial-extract.conf" ] || exit 1
[ ! -e "$sandbox/private/storage_md5" ] || exit 1
[ ! -e "$sandbox/private/.storage_locked" ] || exit 1

# A blank partition must remain save-locked if its stale hash cannot be removed.
printf '%s\n' 'previous-checkpoint' > "$sandbox/private/storage_md5"
: > "$sandbox/private/.storage_locked"
export BZCAT_STATUS=2 BZCAT_EMPTY=1 FAIL_RM_PATH="$sandbox/private/storage_md5"
if run_load; then echo "zero-byte MTD hash-removal failure unexpectedly succeeded" >&2; exit 1; fi
assert_old_storage
unset FAIL_RM_PATH

# A nominally successful empty stream is still not a valid storage archive.
printf '%s\n' 'previous-checkpoint' > "$sandbox/private/storage_md5"
: > "$sandbox/private/.storage_locked"
export BZCAT_STATUS=0 BZCAT_EMPTY=1
if run_load; then echo "empty successful decompression unexpectedly succeeded" >&2; exit 1; fi
[ ! -e "$sandbox/private/storage_md5" ] || exit 1
[ ! -e "$sandbox/private/.storage_locked" ] || exit 1

# The same hash-removal failure must retain the lock when bzcat returns zero.
printf '%s\n' 'previous-checkpoint' > "$sandbox/private/storage_md5"
: > "$sandbox/private/.storage_locked"
export BZCAT_STATUS=0 BZCAT_EMPTY=1 FAIL_RM_PATH="$sandbox/private/storage_md5"
if run_load; then echo "empty successful MTD hash-removal failure unexpectedly succeeded" >&2; exit 1; fi
assert_old_storage
unset FAIL_RM_PATH

# A nonempty failed stream is corruption, not an erase: retain old state.
printf '%s\n' 'previous-checkpoint' > "$sandbox/private/storage_md5"
: > "$sandbox/private/.storage_locked"
export BZCAT_STATUS=2 BZCAT_EMPTY=0
if run_load; then echo "partial decompression failure unexpectedly succeeded" >&2; exit 1; fi
[ ! -s "$TAR_LOG" ] || { echo "tar ran after decompression failed" >&2; exit 1; }
assert_old_storage

export BZCAT_STATUS=0 TAR_FAIL=1
if run_load; then echo "partial extraction unexpectedly succeeded" >&2; exit 1; fi
assert_old_storage

export TAR_FAIL=0 MV_FAIL_AT=2
: > "$MV_COUNT_FILE"
if run_load; then echo "injected storage publish failure unexpectedly succeeded" >&2; exit 1; fi
assert_old_storage

export MV_FAIL_AT=0
: > "$MV_COUNT_FILE"
if ! run_load; then echo "valid storage load unexpectedly failed" >&2; exit 1; fi
[ "$(cat "$sandbox/etc/storage/config.conf")" = "new configuration" ] || exit 1
[ "$(cat "$sandbox/etc/storage/secondary.conf")" = "second entry" ] || exit 1
[ ! -e "$sandbox/etc/storage/previous.conf" ] || exit 1
[ -s "$sandbox/private/storage_md5" ] || exit 1
[ ! -e "$sandbox/private/.storage_locked" ] || exit 1
for scratch in "$sandbox/etc"/.storage.load.* "$sandbox/etc"/.storage.load-backup.*; do
	[ ! -e "$scratch" ] || { echo "successful load left scratch data: $scratch" >&2; exit 1; }
done
[ -s "$BZCAT_LOG" ] || { echo "MTD load did not read the selected partition" >&2; exit 1; }
grep -Fqx '/dev/mtdblock100' "$BZCAT_LOG" || { echo "MTD index >=100 was truncated: $(cat "$BZCAT_LOG")" >&2; exit 1; }
if grep -Fqx '/dev/mtdblock10' "$BZCAT_LOG"; then
	echo "MTD load selected truncated /dev/mtdblock10" >&2
	exit 1
fi
echo "MTD storage load transactional extraction regression: PASS"
