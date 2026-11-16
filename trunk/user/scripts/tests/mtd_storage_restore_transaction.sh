#!/bin/sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
repo_root=$(CDPATH= cd -- "$script_dir/../../../.." && pwd)
sandbox=$(mktemp -d "${TMPDIR:-/tmp}/mtd-storage-restore.XXXXXX")
trap 'rm -rf "$sandbox"' EXIT HUP INT TERM

real_tar=$(command -v tar)
mkdir -p "$sandbox/storage" "$sandbox/private" "$sandbox/incoming" "$sandbox/bin"
printf '%s\n' 'old=preserve-me' > "$sandbox/storage/original.conf"
printf '%s\n' '#!/bin/sh' > "$sandbox/incoming/start_script.sh"
printf '%s\n' 'new=restored-value' > "$sandbox/incoming/new.conf"
chmod 755 "$sandbox/incoming/start_script.sh"
printf '%s\n' 'dev:    size   erasesize  name' 'mtd5: 00010000 00010000 "Storage"' > "$sandbox/proc-mtd"
"$real_tar" -cjf "$sandbox/private/storage.tar.bz2" -C "$sandbox/incoming" .

# Redirect firmware paths into the fixture and replace only the service
# launcher. The restore function itself remains the production implementation.
sed \
  -e "s|^dir_storage=.*|dir_storage=\"$sandbox/storage\"|" \
  -e "s|^slk=.*|slk=\"$sandbox/private/storage_locked\"|" \
  -e "s|^tmp=.*|tmp=\"$sandbox/private/storage.tar\"|" \
  -e "s|proc_root=\"/proc\"|proc_root=\"$sandbox/proc\"|" \
  -e "s|tmp_storage=\"/tmp/.httpd-private/storage-restore\"|tmp_storage=\"$sandbox/restore\"|" \
  -e "s|/sbin/rstats|$sandbox/bin/rstats|" \
  "$repo_root/trunk/user/scripts/files/sbin/mtd_storage.sh" > "$sandbox/mtd_storage.sh"
chmod +x "$sandbox/mtd_storage.sh"

cat > "$sandbox/bin/cat" <<'EOF'
#!/bin/sh
if [ "$#" -eq 1 ] && [ "$1" = "/proc/mtd" ]; then
	exec /bin/cat "$PROC_MTD"
fi
exec /bin/cat "$@"
EOF
cat > "$sandbox/bin/cp" <<'EOF'
#!/bin/sh
[ "${RESTORE_FAIL_CP:-0}" != 1 ] || exit 1
exec /bin/cp "$@"
EOF
cat > "$sandbox/bin/mv" <<'EOF'
#!/bin/sh
count=0
[ -f "$MV_COUNT_FILE" ] && count=$(/bin/cat "$MV_COUNT_FILE")
count=$((count + 1))
printf '%s\n' "$count" > "$MV_COUNT_FILE"
[ "${RESTORE_FAIL_MV_AT:-0}" != "$count" ] || exit 1
exec /bin/mv "$@"
EOF
cat > "$sandbox/bin/killall" <<'EOF'
#!/bin/sh
printf '%s\n' "$*" >> "$RSTAT_KILL_LOG"
[ "${RSTAT_STOP_FAIL:-0}" != 1 ] || exit 1
for stat_file in "$RSTAT_PROC_ROOT"/[0-9]*/stat; do
	[ -r "$stat_file" ] || continue
	IFS= read -r stat_line < "$stat_file" || continue
	case "$stat_line" in
		*"(rstats) "*) rm -rf "${stat_file%/stat}" ;;
	esac
done
exit 0
EOF
cat > "$sandbox/bin/sleep" <<'EOF'
#!/bin/sh
exit 0
EOF
cat > "$sandbox/bin/logger" <<'EOF'
#!/bin/sh
exit 0
EOF
cat > "$sandbox/bin/rstats" <<'EOF'
#!/bin/sh
printf '%s\n' started >> "$RSTAT_LOG"
mkdir -p "$RSTAT_PROC_ROOT/321"
printf '%s\n' '321 (rstats) S 1 321 321' > "$RSTAT_PROC_ROOT/321/stat"
EOF
chmod +x "$sandbox/bin/"*

export PATH="$sandbox/bin:$PATH"
export PROC_MTD="$sandbox/proc-mtd"
export MV_COUNT_FILE="$sandbox/mv-count"
export RSTAT_LOG="$sandbox/rstats-starts"
export RSTAT_PROC_ROOT="$sandbox/proc"
export RSTAT_KILL_LOG="$sandbox/rstats-kill.log"
mkdir -p "$RSTAT_PROC_ROOT/321"
printf '%s\n' '321 (rstats) S 1 321 321' > "$RSTAT_PROC_ROOT/321/stat"
: > "$MV_COUNT_FILE"
: > "$RSTAT_LOG"
: > "$RSTAT_KILL_LOG"

run_restore()
{
	"$sandbox/mtd_storage.sh" restore >/dev/null 2>&1
}

touch "$sandbox/private/storage_locked"
if RESTORE_FAIL_CP=1 run_restore; then
	echo "injected restore staging-copy failure unexpectedly succeeded" >&2
	exit 1
fi
grep -Fqx 'old=preserve-me' "$sandbox/storage/original.conf"
test ! -e "$sandbox/storage/new.conf"
test -f "$sandbox/private/storage.tar.bz2"
test -f "$sandbox/private/storage_locked"

# An incomplete procfs inventory is UNKNOWN, not ABSENT; do not signal or
# replace the live storage directory when any numeric process entry is unreadable.
mkdir -p "$RSTAT_PROC_ROOT/1"
printf '%s\n' 'malformed stat record' > "$RSTAT_PROC_ROOT/1/stat"
: > "$MV_COUNT_FILE"
if run_restore; then
	echo "incomplete procfs inventory unexpectedly authorized storage restore" >&2
	exit 1
fi
grep -Fqx 'old=preserve-me' "$sandbox/storage/original.conf"
test -f "$sandbox/proc/321/stat"
test -s "$sandbox/private/storage.tar.bz2"
test -f "$sandbox/private/storage_locked"
test ! -s "$RSTAT_KILL_LOG"
test "$(/bin/cat "$MV_COUNT_FILE")" -eq 0
rm -rf "$RSTAT_PROC_ROOT/1"

# A failed name-wide stop must not be treated as success or permit directory
# replacement. The still-running daemon and the previous settings must remain.
: > "$MV_COUNT_FILE"
if RSTAT_STOP_FAIL=1 run_restore; then
	echo "failed rstats stop unexpectedly authorized storage restore" >&2
	exit 1
fi
grep -Fqx 'old=preserve-me' "$sandbox/storage/original.conf"
test -f "$sandbox/proc/321/stat"
test -s "$RSTAT_KILL_LOG"
test -s "$sandbox/private/storage.tar.bz2"
test -f "$sandbox/private/storage_locked"
test "$(/bin/cat "$MV_COUNT_FILE")" -eq 0

: > "$MV_COUNT_FILE"
if RESTORE_FAIL_MV_AT=2 run_restore; then
	echo "injected restore install-rename failure unexpectedly succeeded" >&2
	exit 1
fi
grep -Fqx 'old=preserve-me' "$sandbox/storage/original.conf"
test ! -e "$sandbox/storage/new.conf"
test -f "$sandbox/private/storage.tar.bz2"
test -f "$sandbox/private/storage_locked"
set -- "$sandbox"/storage.rollback.*
test ! -e "$1"

: > "$MV_COUNT_FILE"
run_restore
test ! -e "$sandbox/storage/original.conf"
grep -Fqx 'new=restored-value' "$sandbox/storage/new.conf"
test -f "$sandbox/storage/start_script.sh"
test ! -e "$sandbox/private/storage.tar.bz2"
test ! -e "$sandbox/private/storage_locked"
set -- "$sandbox"/storage.rollback.*
test ! -e "$1"
test -s "$RSTAT_LOG"

echo "MTD storage restore staging and rollback regression: PASS"
