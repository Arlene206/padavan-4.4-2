#!/bin/sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
repo_root=$(CDPATH= cd -- "$script_dir/../../../.." && pwd)
sandbox=$(mktemp -d "${TMPDIR:-/tmp}/mtd-storage-restore.XXXXXX")
rstats_daemon_pid_file="$sandbox/rstats-daemon.pid"
cleanup()
{
	if [ -s "$rstats_daemon_pid_file" ]; then
		daemon_pid=$(/bin/cat "$rstats_daemon_pid_file")
		case "$daemon_pid" in
			''|*[!0-9]*) ;;
			*) kill "$daemon_pid" 2>/dev/null || : ;;
		esac
	fi
	rm -rf "$sandbox"
}
trap cleanup EXIT
trap 'exit 1' HUP INT TERM

real_tar=$(command -v tar)
mkdir -p "$sandbox/storage" "$sandbox/private" "$sandbox/incoming" "$sandbox/bin"
printf '%s\n' 'old=preserve-me' > "$sandbox/storage/original.conf"
printf '%s\n' '#!/bin/sh' > "$sandbox/incoming/start_script.sh"
printf '%s\n' 'new=restored-value' > "$sandbox/incoming/new.conf"
chmod 755 "$sandbox/incoming/start_script.sh"
printf '%s\n' 'dev:    size   erasesize  name' 'mtd5: 00010000 00010000 "Storage"' > "$sandbox/proc-mtd"
"$real_tar" -cjf "$sandbox/private/storage-restore.tar.bz2" -C "$sandbox/incoming" .
# Keep an independent backup/download artifact present throughout restore.
printf '%s\n' 'existing backup archive' > "$sandbox/private/storage.tar.bz2"

# Redirect firmware paths into the fixture and replace only the service
# launcher. The restore function itself remains the production implementation.
sed \
  -e "s|^dir_storage=.*|dir_storage=\"$sandbox/storage\"|" \
  -e "s|^slk=.*|slk=\"$sandbox/private/storage_locked\"|" \
  -e "s|^lock_dir=.*|lock_dir=\"$sandbox/private\"|" \
  -e "s|^tmp=.*|tmp=\"$sandbox/private/storage.tar\"|" \
  -e "s|^rtbz=.*|rtbz=\"$sandbox/private/storage-restore.tar.bz2\"|" \
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
cat > "$sandbox/bin/touch" <<'EOF'
#!/bin/sh
for path do
	[ "${RESTORE_FAIL_TOUCH_PATH:-}" != "$path" ] || exit 1
done
exec /bin/touch "$@"
EOF
cat > "$sandbox/bin/mv" <<'EOF'
#!/bin/sh
count=0
[ -f "$MV_COUNT_FILE" ] && count=$(/bin/cat "$MV_COUNT_FILE")
count=$((count + 1))
printf '%s\n' "$count" > "$MV_COUNT_FILE"
case ",${RESTORE_FAIL_MV_AT:-}," in
	*,"$count",*) exit 1 ;;
esac
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
if [ "${RSTAT_DAEMON_TEST:-0}" = 1 ]; then
	# Mimic the long-lived rstats daemon retaining inherited non-stdio FDs.
	/bin/sleep 30 </dev/null >/dev/null 2>&1 &
	echo "$!" > "$RSTAT_DAEMON_PID"
fi
EOF
chmod +x "$sandbox/bin/"*

export PATH="$sandbox/bin:$PATH"
export PROC_MTD="$sandbox/proc-mtd"
export MV_COUNT_FILE="$sandbox/mv-count"
export RSTAT_LOG="$sandbox/rstats-starts"
export RSTAT_PROC_ROOT="$sandbox/proc"
export RSTAT_KILL_LOG="$sandbox/rstats-kill.log"
export RSTAT_DAEMON_PID="$rstats_daemon_pid_file"
mkdir -p "$RSTAT_PROC_ROOT/321"
printf '%s\n' '321 (rstats) S 1 321 321' > "$RSTAT_PROC_ROOT/321/stat"
: > "$MV_COUNT_FILE"
: > "$RSTAT_LOG"
: > "$RSTAT_KILL_LOG"

run_restore()
{
	"$sandbox/mtd_storage.sh" restore >/dev/null 2>&1
}

# Restore must establish the same save barrier used by the save command before
# inspecting the archive or touching the live tree.
if RESTORE_FAIL_TOUCH_PATH="$sandbox/private/storage_locked" run_restore; then
	echo "restore lock creation failure unexpectedly succeeded" >&2
	exit 1
fi
grep -Fqx 'old=preserve-me' "$sandbox/storage/original.conf"
test -f "$sandbox/proc/321/stat"
test -s "$sandbox/private/storage-restore.tar.bz2"
grep -Fqx 'existing backup archive' "$sandbox/private/storage.tar.bz2"
test ! -e "$sandbox/private/storage_locked"
test ! -s "$RSTAT_KILL_LOG"
test ! -s "$MV_COUNT_FILE"

if RESTORE_FAIL_CP=1 run_restore; then
	echo "injected restore staging-copy failure unexpectedly succeeded" >&2
	exit 1
fi
grep -Fqx 'old=preserve-me' "$sandbox/storage/original.conf"
test ! -e "$sandbox/storage/new.conf"
test -f "$sandbox/private/storage-restore.tar.bz2"
test -f "$sandbox/private/storage_locked"
# The marker created by restore must make a concurrent MTD save fail before
# it reads or writes the flash partition.
if sh "$sandbox/mtd_storage.sh" save; then
	echo "MTD save ignored the restore save barrier" >&2
	exit 1
fi
test ! -s "$MV_COUNT_FILE"

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
test -s "$sandbox/private/storage-restore.tar.bz2"
test -f "$sandbox/private/storage_locked"
test ! -s "$RSTAT_KILL_LOG"
test ! -s "$MV_COUNT_FILE"
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
test -s "$sandbox/private/storage-restore.tar.bz2"
test -f "$sandbox/private/storage_locked"
test ! -s "$MV_COUNT_FILE"

: > "$MV_COUNT_FILE"
if RESTORE_FAIL_MV_AT=2 run_restore; then
	echo "injected restore install-rename failure unexpectedly succeeded" >&2
	exit 1
fi
grep -Fqx 'old=preserve-me' "$sandbox/storage/original.conf"
test ! -e "$sandbox/storage/new.conf"
test -f "$sandbox/private/storage-restore.tar.bz2"
test -f "$sandbox/private/storage_locked"
set -- "$sandbox"/storage.rollback.*
test ! -e "$1"

# If publishing the new tree and restoring the prior tree both fail, the
# previous settings remain in the rollback directory. Never restart rstats
# against a missing /etc/storage tree.
: > "$MV_COUNT_FILE"
: > "$RSTAT_LOG"
if RESTORE_FAIL_MV_AT="2,3" run_restore; then
	echo "double restore rename failure unexpectedly succeeded" >&2
	exit 1
fi
test ! -e "$sandbox/storage"
set -- "$sandbox"/storage.rollback.*
test -d "$1"
grep -Fqx 'old=preserve-me' "$1/original.conf"
test ! -e "$RSTAT_PROC_ROOT/321/stat"
test ! -s "$RSTAT_LOG"
test -f "$sandbox/private/storage-restore.tar.bz2"
test -f "$sandbox/private/storage_locked"
set -- "$sandbox"/storage.restore.*
test ! -e "$1"
rm -rf "$sandbox"/storage.rollback.*

: > "$MV_COUNT_FILE"
RSTAT_DAEMON_TEST=1
export RSTAT_DAEMON_TEST
run_restore
unset RSTAT_DAEMON_TEST
test -s "$RSTAT_DAEMON_PID"
daemon_pid=$(/bin/cat "$RSTAT_DAEMON_PID")
kill -0 "$daemon_pid" 2>/dev/null || {
	echo "rstats daemon fixture exited before lock inheritance check" >&2
	exit 1
}
if (exec 8>>"$sandbox/private/.mtd_storage.lock"; flock -n 8); then
	:
else
	echo "rstats daemon inherited the MTD operation lock" >&2
	exit 1
fi
kill "$daemon_pid" 2>/dev/null || :
test ! -e "$sandbox/storage/original.conf"
grep -Fqx 'new=restored-value' "$sandbox/storage/new.conf"
test -f "$sandbox/storage/start_script.sh"
test ! -e "$sandbox/private/storage-restore.tar.bz2"
grep -Fqx 'existing backup archive' "$sandbox/private/storage.tar.bz2"
test ! -e "$sandbox/private/storage_locked"
set -- "$sandbox"/storage.rollback.*
test ! -e "$1"
test -s "$RSTAT_LOG"

echo "MTD storage restore staging and rollback regression: PASS"
