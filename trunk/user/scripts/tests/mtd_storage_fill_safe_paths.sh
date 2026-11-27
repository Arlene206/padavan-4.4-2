#!/bin/sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
repo_root=$(CDPATH= cd -- "$script_dir/../../../.." && pwd)
sandbox=$(mktemp -d "${TMPDIR:-/tmp}/mtd-storage-fill.XXXXXX")
real_rm=$(command -v rm)
real_mkdir=$(command -v mkdir)
real_cat=$(command -v cat)
real_chmod=$(command -v chmod)
real_mv=$(command -v mv)
cleanup()
{
	"$real_rm" -rf "$sandbox"
}
trap cleanup EXIT
trap 'exit 129' HUP
trap 'exit 130' INT
trap 'exit 143' TERM

mkdir -p "$sandbox/storage" "$sandbox/private" "$sandbox/outside" "$sandbox/bin"
printf '%s\n' outside-sentinel > "$sandbox/outside/sentinel"
real_tar=$(command -v tar)
export REAL_TAR="$real_tar" FILL_TAR_FAIL=0
: > "$sandbox/reset-ss.log"

cat > "$sandbox/bin/logger" <<'EOF_LOGGER'
#!/bin/sh
printf '%s\n' "$*" >> "$LOGGER_LOG"
EOF_LOGGER
cat > "$sandbox/bin/reset_ss.sh" <<'EOF_RESET'
#!/bin/sh
printf '%s\n' "$*" >> "$RESET_SS_LOG"
[ "${RESET_SS_FAIL:-0}" != 1 ] || exit 1
printf '%s\n' '#!/bin/sh' > "$STORAGE_ROOT/start_script.sh"
chmod 755 "$STORAGE_ROOT/start_script.sh"
EOF_RESET
cat > "$sandbox/bin/mkdir" <<'EOF_MKDIR'
#!/bin/sh
for arg do
	[ "${FILL_MKDIR_FAIL_PATH:-}" != "$arg" ] || exit 1
done
exec "$REAL_MKDIR" "$@"
EOF_MKDIR
cat > "$sandbox/bin/cat" <<'EOF_CAT'
#!/bin/sh
[ "${FILL_CAT_FAIL:-0}" != 1 ] || exit 1
exec "$REAL_CAT" "$@"
EOF_CAT
cat > "$sandbox/bin/chmod" <<'EOF_CHMOD'
#!/bin/sh
for arg do
	case "$arg" in
		"$FILL_CHMOD_FAIL_PATH"|"$FILL_CHMOD_FAIL_PATH".tmp.*) [ -z "${FILL_CHMOD_FAIL_PATH:-}" ] || exit 1 ;;
	esac
done
exec "$REAL_CHMOD" "$@"
EOF_CHMOD
cat > "$sandbox/bin/mv" <<'EOF_MV'
#!/bin/sh
[ "${FILL_MV_FAIL:-0}" != 1 ] || exit 1
exec "$REAL_MV" "$@"
EOF_MV
cat > "$sandbox/bin/tar" <<'EOF_TAR'
#!/bin/sh
target=
want_target=0
for arg do
	if [ "$want_target" = 1 ]; then target=$arg; want_target=0
	elif [ "$arg" = "-C" ]; then want_target=1
	fi
done
if [ "${FILL_TAR_FAIL:-0}" = 1 ]; then
	[ -n "$target" ] || exit 99
	printf '%s\n' 'partial extraction must be discarded' > "$target/partial-extract.conf"
	exit 1
fi
exec "$REAL_TAR" "$@"
EOF_TAR
chmod +x "$sandbox/bin/"*

sed \
	-e "s|^dir_storage=.*|dir_storage=\"$sandbox/storage\"|" \
	-e "s|^slk=.*|slk=\"$sandbox/private/storage_locked\"|" \
	-e "s|^lock_dir=.*|lock_dir=\"$sandbox/private\"|" \
	-e "s|^tmp=.*|tmp=\"$sandbox/private/storage.tar\"|" \
	-e "s|^rtbz=.*|rtbz=\"$sandbox/private/storage-restore.tar.bz2\"|" \
	-e "s|chnroute_file=\"/etc_ro/chnroute.bz2\"|chnroute_file=\"$sandbox/chnroute.bz2\"|" \
	"$repo_root/trunk/user/scripts/files/sbin/mtd_storage.sh" > "$sandbox/mtd_storage.sh"
chmod +x "$sandbox/mtd_storage.sh"

export PATH="$sandbox/bin:$PATH"
export REAL_MKDIR="$real_mkdir" REAL_CAT="$real_cat" REAL_CHMOD="$real_chmod" REAL_MV="$real_mv"
export FILL_MKDIR_FAIL_PATH= FILL_CAT_FAIL=0 FILL_CHMOD_FAIL_PATH= RESET_SS_FAIL=0
export LOGGER_LOG="$sandbox/logger.log" RESET_SS_LOG="$sandbox/reset-ss.log"
export STORAGE_ROOT="$sandbox/storage"

run_fill()
{
	if [ -n "${BUSYBOX_BIN:-}" ]; then
		"$BUSYBOX_BIN" ash "$sandbox/mtd_storage.sh" fill
	else
		sh "$sandbox/mtd_storage.sh" fill
	fi
}
reset_case()
{
	"$real_rm" -rf "$sandbox/storage"
	mkdir -p "$sandbox/storage"
	: > "$RESET_SS_LOG"
	: > "$LOGGER_LOG"
	FILL_TAR_FAIL=0
	FILL_MKDIR_FAIL_PATH=
	FILL_CAT_FAIL=0
	FILL_CHMOD_FAIL_PATH=
	FILL_MV_FAIL=0
	RESET_SS_FAIL=0
	export FILL_MKDIR_FAIL_PATH FILL_CAT_FAIL FILL_CHMOD_FAIL_PATH FILL_MV_FAIL RESET_SS_FAIL
}

# A nested directory symlink must not let dnsmasq defaults escape /etc/storage.
reset_case
ln -s "$sandbox/outside" "$sandbox/storage/dnsmasq"
if run_fill >/dev/null 2>&1; then
	echo "dnsmasq directory symlink unexpectedly accepted" >&2
	exit 1
fi
test ! -e "$sandbox/outside/dnsmasq.conf"
test ! -s "$RESET_SS_LOG"
grep -Fq 'Unsafe storage directory component:' "$LOGGER_LOG"

# The validator must inspect every ancestor, not just the final crontabs path.
reset_case
ln -s "$sandbox/outside" "$sandbox/storage/cron"
if run_fill >/dev/null 2>&1; then
	echo "cron parent symlink unexpectedly accepted" >&2
	exit 1
fi
test ! -e "$sandbox/outside/crontabs"
test ! -s "$RESET_SS_LOG"

# Existing config symlinks are not trusted merely because test -f follows them.
reset_case
mkdir -p "$sandbox/storage/dnsmasq"
ln -s "$sandbox/outside/sentinel" "$sandbox/storage/dnsmasq/dnsmasq.conf"
if run_fill >/dev/null 2>&1; then
	echo "dnsmasq config symlink unexpectedly accepted" >&2
	exit 1
fi
test "$(cat "$sandbox/outside/sentinel")" = outside-sentinel
test ! -s "$RESET_SS_LOG"
grep -Fq 'Unsafe storage file target:' "$LOGGER_LOG"

# A symlinked boot script must not be treated as an already-installed script.
reset_case
ln -s "$sandbox/outside/sentinel" "$sandbox/storage/start_script.sh"
if run_fill >/dev/null 2>&1; then
	echo "start script symlink unexpectedly accepted" >&2
	exit 1
fi
test "$(cat "$sandbox/outside/sentinel")" = outside-sentinel
test ! -s "$RESET_SS_LOG"
# dev_init copies authorized_keys as root; reject symlinks before that privileged read.
reset_case
ln -s "$sandbox/outside/sentinel" "$sandbox/storage/authorized_keys"
if run_fill >/dev/null 2>&1; then
	echo "authorized_keys symlink unexpectedly accepted" >&2
	exit 1
fi
test "$(cat "$sandbox/outside/sentinel")" = outside-sentinel
test ! -s "$RESET_SS_LOG"
grep -Fq "Unsafe storage file target:" "$LOGGER_LOG"

# A failed default-file heredoc write must stop fill before chmod/next file.
reset_case
FILL_CAT_FAIL=1
export FILL_CAT_FAIL
if run_fill >/dev/null 2>&1; then
	echo "failed default file write unexpectedly succeeded" >&2
	exit 1
fi
FILL_CAT_FAIL=0
export FILL_CAT_FAIL
test ! -e "$sandbox/storage/started_script.sh"
test -z "$(find "$sandbox/storage" -name 'started_script.sh.tmp.*' -print)"
grep -Fq 'Cannot write default storage file:' "$LOGGER_LOG"

# A chmod failure is also fatal; do not continue with non-executable hooks.
reset_case
FILL_CHMOD_FAIL_PATH="$sandbox/storage/started_script.sh"
export FILL_CHMOD_FAIL_PATH
if run_fill >/dev/null 2>&1; then
	echo "failed default file mode change unexpectedly succeeded" >&2
	exit 1
fi
FILL_CHMOD_FAIL_PATH=
export FILL_CHMOD_FAIL_PATH
test ! -e "$sandbox/storage/started_script.sh"
test -z "$(find "$sandbox/storage" -name 'started_script.sh.tmp.*' -print)"
test ! -e "$sandbox/storage/shutdown_script.sh"
grep -Fq 'Cannot set default file mode:' "$LOGGER_LOG"

# A failed atomic rename must leave no final file or staging residue.
reset_case
FILL_MV_FAIL=1
export FILL_MV_FAIL
if run_fill >/dev/null 2>&1; then
	echo "default file publication failure unexpectedly succeeded" >&2
	exit 1
fi
FILL_MV_FAIL=0
export FILL_MV_FAIL
test ! -e "$sandbox/storage/started_script.sh"
test -z "$(find "$sandbox/storage" -name 'started_script.sh.tmp.*' -print)"
grep -Fq 'Cannot publish default storage file:' "$LOGGER_LOG"

# A required default directory failure must stop fill and reach its caller.
reset_case
FILL_MKDIR_FAIL_PATH="$sandbox/storage/cron/crontabs"
export FILL_MKDIR_FAIL_PATH
if run_fill >/dev/null 2>&1; then
	echo "storage directory creation failure unexpectedly succeeded" >&2
	exit 1
fi
test ! -d "$sandbox/storage/cron/crontabs"
test ! -s "$RESET_SS_LOG"
grep -Fq 'Cannot create storage directory:' "$LOGGER_LOG"

# A failed start-script generator must not be reported as a successful fill.
reset_case
RESET_SS_FAIL=1
export RESET_SS_FAIL
if run_fill >/dev/null 2>&1; then
	echo "failed reset_ss start-script generation unexpectedly succeeded" >&2
	exit 1
fi
grep -Fqx -- '-a' "$RESET_SS_LOG"
grep -Fq 'Cannot generate default start script:' "$LOGGER_LOG"

# Failed bundled archive extraction must not leave a partial directory that
# causes later boot fills to mistake an incomplete tree for a completed one.
reset_case
printf '%s\n' 'trusted chnroute fixture' > "$sandbox/chnroute.bz2"
if FILL_TAR_FAIL=1 run_fill >/dev/null 2>&1; then
	echo "partial chnroute extraction unexpectedly succeeded" >&2
	exit 1
fi
test ! -e "$sandbox/storage/chinadns"
test ! -s "$RESET_SS_LOG"
grep -Fq 'Cannot extract chnroute archive:' "$LOGGER_LOG"

# A clean storage tree must still complete normal default generation.
# The preceding failure test deliberately leaves a corrupt archive payload.
reset_case
"$real_rm" -f "$sandbox/chnroute.bz2"
if ! run_fill >/dev/null 2>&1; then
	echo "normal MTD storage fill unexpectedly failed" >&2
	exit 1
fi
test -s "$sandbox/storage/start_script.sh"
test -s "$sandbox/storage/started_script.sh"
test -s "$sandbox/storage/dnsmasq/dnsmasq.conf"
test -s "$sandbox/storage/inadyn/inadyn.conf"
test -s "$sandbox/storage/wlan/AP.dat"
test -x "$sandbox/storage/started_script.sh"
test -z "$(find "$sandbox/storage" -name '*.tmp.*' -print)"
grep -Fqx -- '-a' "$RESET_SS_LOG"

echo "MTD storage fill symlink-path regression: PASS"
