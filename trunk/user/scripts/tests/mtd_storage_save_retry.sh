#!/bin/sh
set -eu
script_dir=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
repo_root=$(CDPATH= cd -- "$script_dir/../../../.." && pwd)
sandbox=$(mktemp -d "${TMPDIR:-/tmp}/mtd-storage-retry.XXXXXX")
trap 'rm -rf "$sandbox"' EXIT HUP INT TERM
mkdir -p "$sandbox/storage" "$sandbox/private" "$sandbox/bin"
printf '%s\n' 'mtd5: 00010000 00010000 "Storage"' > "$sandbox/proc-mtd"
printf '%s\n' 'value=one' > "$sandbox/storage/config.conf"
sed \
  -e "s|dir_storage=\"/etc/storage\"|dir_storage=\"$sandbox/storage\"|" \
  -e "s|slk=\"/tmp/.httpd-private/.storage_locked\"|slk=\"$sandbox/private/.storage_locked\"|" \
  -e "s|^lock_dir=.*|lock_dir=\"$sandbox/private\"|" \
  -e "s|tmp=\"/tmp/.httpd-private/storage.tar\"|tmp=\"$sandbox/private/storage.tar\"|" \
  -e "s|hsh=\"/tmp/.httpd-private/storage_md5\"|hsh=\"$sandbox/private/storage_md5\"|" \
  -e "s|/proc/mtd|$sandbox/proc-mtd|g" \
  "$repo_root/trunk/user/scripts/files/sbin/mtd_storage.sh" > "$sandbox/mtd_storage.sh"
cat > "$sandbox/bin/logger" <<'EOF'
#!/bin/sh
exit 0
EOF
cat > "$sandbox/bin/mtd_write" <<'EOF'
#!/bin/sh
printf '%s\n' "$*" >> "$MTD_WRITE_LOG"
[ "${MTD_WRITE_FAIL:-0}" != 1 ]
EOF
chmod +x "$sandbox/bin/logger" "$sandbox/bin/mtd_write"
if [ -n "${BUSYBOX_BIN:-}" ]; then
	ln -s "$BUSYBOX_BIN" "$sandbox/bin/flock"
fi
export PATH="$sandbox/bin:$PATH"
export MTD_WRITE_LOG="$sandbox/mtd-write.log"
run_mtd()
{
	if [ -n "${BUSYBOX_BIN:-}" ]; then
		"$BUSYBOX_BIN" ash "$sandbox/mtd_storage.sh" "$1"
	else
		sh "$sandbox/mtd_storage.sh" "$1"
	fi
}
MTD_WRITE_FAIL=0 run_mtd save
[ -s "$sandbox/private/storage_md5" ] || { echo "initial hash checkpoint missing" >&2; exit 1; }

# A process holding the operation lock excludes every concurrent MTD command.
(
	exec 8>"$sandbox/private/.mtd_storage.lock"
	flock -n 8
	if run_mtd save; then
		echo "concurrent MTD save bypassed the operation lock" >&2
		exit 1
	fi
)
[ "$(wc -l < "$MTD_WRITE_LOG")" -eq 1 ]

printf '%s\n' 'value=two' > "$sandbox/storage/config.conf"
if MTD_WRITE_FAIL=1 run_mtd save; then
	echo "injected MTD failure unexpectedly succeeded" >&2
	exit 1
fi
MTD_WRITE_FAIL=0 run_mtd save
write_count=$(wc -l < "$MTD_WRITE_LOG")
[ "$write_count" -eq 3 ] || { echo "expected three MTD attempts, got $write_count" >&2; exit 1; }
MTD_WRITE_FAIL=0 run_mtd save
write_count=$(wc -l < "$MTD_WRITE_LOG")
[ "$write_count" -eq 3 ] || { echo "unchanged save unexpectedly flashed again: $write_count" >&2; exit 1; }
echo "MTD storage save retry regression: PASS"
