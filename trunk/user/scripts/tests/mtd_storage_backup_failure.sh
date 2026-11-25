#!/bin/sh
set -eu
script_dir=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
repo_root=$(CDPATH= cd -- "$script_dir/../../../.." && pwd)
sandbox=$(mktemp -d "${TMPDIR:-/tmp}/mtd-storage-backup.XXXXXX")
trap 'rm -rf "$sandbox"' EXIT HUP INT TERM
mkdir -p "$sandbox/storage/empty-dir" "$sandbox/private" "$sandbox/bin"
printf '%s\n' 'mtd5: 00010000 00010000 "Storage"' > "$sandbox/proc-mtd"
printf '%s\n' 'backup configuration payload' > "$sandbox/storage/config.conf"
printf '%s\n' 'hidden backup payload' > "$sandbox/storage/.hidden.conf"
real_bzip2=$(command -v bzip2)
real_bzcat=$(command -v bzcat)
real_find=$(command -v find)
sed \
  -e "s|^dir_storage=.*|dir_storage=\"$sandbox/storage\"|" \
  -e "s|^slk=.*|slk=\"$sandbox/private/.storage_locked\"|" \
  -e "s|^lock_dir=.*|lock_dir=\"$sandbox/private\"|" \
  -e "s|^tmp=.*|tmp=\"$sandbox/private/storage.tar\"|" \
  -e "s|^hsh=.*|hsh=\"$sandbox/private/storage_md5\"|" \
  -e "s|/proc/mtd|$sandbox/proc-mtd|g" \
  -e "s|/proc/mtd|$sandbox/proc-mtd|g" \
  "$repo_root/trunk/user/scripts/files/sbin/mtd_storage.sh" > "$sandbox/mtd_storage.sh"
cat > "$sandbox/bin/logger" <<'EOF'
#!/bin/sh
exit 0
EOF
cat > "$sandbox/bin/find" <<'EOF_FIND'
#!/bin/sh
if [ "${FIND_MODE:-valid}" = partial-failure ]; then
	printf '%s\0' './config.conf'
	exit 1
fi
exec "$REAL_FIND" "$@"
EOF_FIND
cat > "$sandbox/bin/bzip2" <<'EOF'
#!/bin/sh
case "${BZIP2_MODE:-valid}:$1" in
	fail:-9)
		cat "$2" > "$2.bz2"
		exit 2
		;;
	oversized:-9)
		head -c 70000 /dev/zero > "$2.bz2"
		rm -f "$2"
		exit 0
		;;
	valid:-9)
		exec "$REAL_BZIP2" "$@"
		;;
esac
exit 91
EOF
cat > "$sandbox/bin/bzcat" <<'EOF'
#!/bin/sh
case "${BZCAT_MODE:-valid}" in
	fail)
		printf '%s\n' 'partial compressed stream'
		exit 2
		;;
	invalid)
		printf '%s\n' 'not a tar archive'
		exit 0
		;;
	*)
		exec "$REAL_BZCAT" "$@"
		;;
esac
EOF
chmod +x "$sandbox/bin/logger" "$sandbox/bin/bzip2" "$sandbox/bin/bzcat" "$sandbox/bin/find"
export PATH="$sandbox/bin:$PATH" REAL_BZIP2="$real_bzip2" REAL_BZCAT="$real_bzcat" REAL_FIND="$real_find"
run_backup()
{
	if [ -n "${BUSYBOX_BIN:-}" ]; then "$BUSYBOX_BIN" ash "$sandbox/mtd_storage.sh" backup
	else sh "$sandbox/mtd_storage.sh" backup
	fi
}
assert_no_archive()
{
	[ ! -e "$sandbox/private/storage.tar.bz2" ] || { echo "failed backup left a partial archive" >&2; exit 1; }
	[ ! -e "$sandbox/private/storage.tar" ] || { echo "failed backup left a temporary tarball" >&2; exit 1; }
}

export FIND_MODE=partial-failure BZIP2_MODE=valid BZCAT_MODE=valid
if run_backup; then echo "partial find inventory unexpectedly produced a successful backup" >&2; exit 1; fi
assert_no_archive

export FIND_MODE=valid BZIP2_MODE=fail BZCAT_MODE=valid
if run_backup; then echo "injected bzip2 failure unexpectedly succeeded" >&2; exit 1; fi
assert_no_archive

export BZIP2_MODE=oversized
if run_backup; then echo "oversized backup unexpectedly succeeded" >&2; exit 1; fi
assert_no_archive

export BZIP2_MODE=valid BZCAT_MODE=fail
if run_backup; then echo "corrupt bzip2 stream unexpectedly succeeded" >&2; exit 1; fi
assert_no_archive

export BZCAT_MODE=invalid
if run_backup; then echo "non-tar backup unexpectedly succeeded" >&2; exit 1; fi
assert_no_archive

export BZCAT_MODE=valid
if ! run_backup; then echo "valid backup unexpectedly failed" >&2; exit 1; fi
[ -s "$sandbox/private/storage.tar.bz2" ] || { echo "valid backup archive missing" >&2; exit 1; }
size=$(stat -c %s "$sandbox/private/storage.tar.bz2")
[ "$size" -ge 16 ] && [ "$size" -le 65536 ] || { echo "valid archive outside MTD size bounds: $size" >&2; exit 1; }
archive_listing=$("$REAL_BZCAT" "$sandbox/private/storage.tar.bz2" | tar tf -)
printf '%s\n' "$archive_listing" | grep -Fqx './config.conf'
printf '%s\n' "$archive_listing" | grep -Fqx './.hidden.conf'
printf '%s\n' "$archive_listing" | grep -Fqx './empty-dir/'
echo "MTD storage backup failure and integrity regression: PASS"
