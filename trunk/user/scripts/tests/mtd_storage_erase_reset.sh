#!/bin/sh
set -eu
script_dir=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
repo_root=$(CDPATH= cd -- "$script_dir/../../../.." && pwd)
sandbox=$(mktemp -d "${TMPDIR:-/tmp}/mtd-storage-erase-reset.XXXXXX")
real_rm=$(command -v rm)
real_mkdir=$(command -v mkdir)
real_touch=$(command -v touch)
trap 'status=$?; "$real_rm" -rf "$sandbox"; exit "$status"' EXIT
"$real_mkdir" -p "$sandbox/bin"
cat > "$sandbox/bin/logger" <<'EOF_LOGGER'
#!/bin/sh
printf '%s\n' "$*" >> "$LOGGER_LOG"
EOF_LOGGER
cat > "$sandbox/bin/mtd_write" <<'EOF_MTD'
#!/bin/sh
printf '%s\n' "$*" >> "$MTD_WRITE_LOG"
printf '%s\n' "mtd_write $*" >> "$EVENT_LOG"
[ "${MTD_WRITE_STATUS:-0}" = 0 ]
EOF_MTD
cat > "$sandbox/bin/rm" <<'EOF_RM'
#!/bin/sh
for arg do
	[ "${FAIL_RM_PATH:-}" != "$arg" ] || exit 1
done
exec "$REAL_RM" "$@"
EOF_RM
cat > "$sandbox/bin/mkdir" <<'EOF_MKDIR'
#!/bin/sh
for arg do
	[ "${FAIL_MKDIR_PATH:-}" != "$arg" ] || exit 1
done
exec "$REAL_MKDIR" "$@"
EOF_MKDIR
cat > "$sandbox/bin/touch" <<'EOF_TOUCH'
#!/bin/sh
for arg do
	[ "${FAIL_TOUCH_PATH:-}" != "$arg" ] || exit 1
done
exec "$REAL_TOUCH" "$@"
EOF_TOUCH
chmod +x "$sandbox/bin/"*
export REAL_RM="$real_rm" REAL_MKDIR="$real_mkdir" REAL_TOUCH="$real_touch"
export PATH="$sandbox/bin:$PATH"

prepare_case()
{
	name=$1
	base="$sandbox/$name"
	"$real_mkdir" -p "$base/storage" "$base/private"
	printf '%s\n' 'old configuration' > "$base/storage/old.conf"
	printf '%s\n' 'stale-checkpoint' > "$base/private/storage_md5"
	printf '%s\n' 'dev:    size   erasesize  name' 'mtd5: 00010000 00010000 "Storage"' > "$base/proc-mtd"
	export CALL_LOG="$base/calls.log" MTD_WRITE_LOG="$base/mtd-write.log" LOGGER_LOG="$base/logger.log" EVENT_LOG="$base/events.log"
	: > "$CALL_LOG"
	: > "$MTD_WRITE_LOG"
	: > "$LOGGER_LOG"
	: > "$EVENT_LOG"
	FAIL_RM_PATH=
	FAIL_MKDIR_PATH=
	FAIL_TOUCH_PATH=
	MTD_WRITE_STATUS=0
	FILL_FAIL=0
	START_FAIL=0
	STOP_APPS_FAIL=0
	export FAIL_RM_PATH FAIL_MKDIR_PATH FAIL_TOUCH_PATH MTD_WRITE_STATUS FILL_FAIL START_FAIL STOP_APPS_FAIL
	export STOP_LOG="$base/stop-apps.log"
	: > "$STOP_LOG"
	sed \
		-e "s|^dir_storage=.*|dir_storage=\"$base/storage\"|" \
		-e "s|^slk=.*|slk=\"$base/private/.storage_locked\"|" \
  -e "s|^lock_dir=.*|lock_dir=\"$base/private\"|" \
		-e "s|^tmp=.*|tmp=\"$base/private/storage.tar\"|" \
		-e "s|^hsh=.*|hsh=\"$base/private/storage_md5\"|" \
		-e "s|cat /proc/mtd|cat \"$base/proc-mtd\"|" \
		"$repo_root/trunk/user/scripts/files/sbin/mtd_storage.sh" > "$base/source.sh"
	sed '/^case "\$1" in$/,$d' "$base/source.sh" > "$base/mtd_storage.sh"
	cat >> "$base/mtd_storage.sh" <<'EOF_MOCK_FUNCTIONS'
func_stop_apps()
{
	printf '%s\n' stop >> "$STOP_LOG"
	printf '%s\n' stop >> "$EVENT_LOG"
	[ "${STOP_APPS_FAIL:-0}" != 1 ]
}
func_fill()
{
	printf '%s\n' fill >> "$CALL_LOG"
	[ "${FILL_FAIL:-0}" != 1 ]
}
func_start_apps()
{
	printf '%s\n' start >> "$CALL_LOG"
	[ "${START_FAIL:-0}" != 1 ]
}
EOF_MOCK_FUNCTIONS
	sed -n '/^case "\$1" in$/,$p' "$base/source.sh" >> "$base/mtd_storage.sh"
}
run_mtd()
{
	if [ -n "${BUSYBOX_BIN:-}" ]; then
		"$BUSYBOX_BIN" ash "$1" "$2"
	else
		sh "$1" "$2"
	fi
}
expect_failure()
{
	if run_mtd "$base/mtd_storage.sh" "$1"; then
		echo "injected $2 failure unexpectedly succeeded" >&2
		exit 1
	fi
}
assert_old_storage()
{
	[ -f "$base/storage/old.conf" ]
	[ "$(cat "$base/storage/old.conf")" = "old configuration" ]
}
assert_no_service_calls() { [ ! -s "$CALL_LOG" ]; }
assert_empty_storage()
{
	[ -d "$base/storage" ]
	[ -z "$(ls -A "$base/storage")" ]
}

prepare_case erase-flash-failure
MTD_WRITE_STATUS=1
expect_failure erase "MTD write"
[ -e "$base/private/.storage_locked" ]
[ -e "$base/private/storage_md5" ]
assert_old_storage
grep -Fqx 'erase Storage' "$MTD_WRITE_LOG"

prepare_case erase-stop-failure
STOP_APPS_FAIL=1
expect_failure erase "rstats stop"
[ -e "$base/private/.storage_locked" ]
[ -e "$base/private/storage_md5" ]
assert_old_storage
[ ! -s "$MTD_WRITE_LOG" ]
grep -Fqx stop "$STOP_LOG"

prepare_case erase-lock-creation-failure
FAIL_TOUCH_PATH="$base/private/.storage_locked"
export FAIL_TOUCH_PATH
expect_failure erase "save-lock creation"
[ ! -e "$base/private/.storage_locked" ]
[ -e "$base/private/storage_md5" ]
assert_old_storage
[ ! -s "$MTD_WRITE_LOG" ]

prepare_case erase-hash-removal-failure
FAIL_RM_PATH="$base/private/storage_md5"
export FAIL_RM_PATH
expect_failure erase "hash removal"
[ -e "$base/private/.storage_locked" ]
[ -e "$base/private/storage_md5" ]
assert_old_storage
grep -Fqx 'erase Storage' "$MTD_WRITE_LOG"

prepare_case erase-directory-removal-failure
FAIL_RM_PATH="$base/storage"
export FAIL_RM_PATH
expect_failure erase "directory removal"
[ -e "$base/private/.storage_locked" ]
[ ! -e "$base/private/storage_md5" ]
assert_old_storage

prepare_case erase-directory-recreation-failure
FAIL_MKDIR_PATH="$base/storage"
export FAIL_MKDIR_PATH
expect_failure erase "directory recreation"
[ -e "$base/private/.storage_locked" ]
[ ! -e "$base/private/storage_md5" ]
[ ! -e "$base/storage" ]

prepare_case erase-success
run_mtd "$base/mtd_storage.sh" erase
[ -e "$base/private/.storage_locked" ]
[ ! -e "$base/private/storage_md5" ]
assert_empty_storage
grep -Fqx stop "$STOP_LOG"
awk '$0 == "stop" { stop = NR } $0 == "mtd_write erase Storage" { erase = NR } END { exit !(stop > 0 && erase > stop) }' "$EVENT_LOG"
assert_no_service_calls

prepare_case reset-lock-creation-failure
FAIL_TOUCH_PATH="$base/private/.storage_locked"
export FAIL_TOUCH_PATH
expect_failure reset "save-lock creation"
assert_old_storage
[ -e "$base/private/storage_md5" ]
assert_no_service_calls

prepare_case reset-stop-failure
STOP_APPS_FAIL=1
expect_failure reset "rstats stop"
[ -e "$base/private/.storage_locked" ]
[ -e "$base/private/storage_md5" ]
assert_old_storage
assert_no_service_calls
grep -Fqx stop "$STOP_LOG"

prepare_case reset-hash-removal-failure
FAIL_RM_PATH="$base/private/storage_md5"
export FAIL_RM_PATH
expect_failure reset "hash removal"
[ -e "$base/private/.storage_locked" ]
[ -e "$base/private/storage_md5" ]
assert_old_storage
assert_no_service_calls

prepare_case reset-directory-removal-failure
FAIL_RM_PATH="$base/storage"
export FAIL_RM_PATH
expect_failure reset "directory removal"
[ -e "$base/private/.storage_locked" ]
[ ! -e "$base/private/storage_md5" ]
assert_old_storage
assert_no_service_calls

prepare_case reset-directory-recreation-failure
FAIL_MKDIR_PATH="$base/storage"
export FAIL_MKDIR_PATH
expect_failure reset "directory recreation"
[ -e "$base/private/.storage_locked" ]
[ ! -e "$base/storage" ]
assert_no_service_calls

prepare_case reset-default-fill-failure
FILL_FAIL=1
expect_failure reset "default fill"
[ -e "$base/private/.storage_locked" ]
grep -Fqx fill "$CALL_LOG"
! grep -Fqx start "$CALL_LOG"

prepare_case reset-lock-release-failure
FAIL_RM_PATH="$base/private/.storage_locked"
export FAIL_RM_PATH
expect_failure reset "save-lock release"
[ -e "$base/private/.storage_locked" ]
assert_empty_storage
grep -Fqx fill "$CALL_LOG"
grep -Fqx start "$CALL_LOG"

prepare_case reset-success
run_mtd "$base/mtd_storage.sh" reset
[ ! -e "$base/private/.storage_locked" ]
[ ! -e "$base/private/storage_md5" ]
assert_empty_storage
grep -Fqx fill "$CALL_LOG"
grep -Fqx start "$CALL_LOG"
echo "MTD storage erase/reset failure-postcondition regression: PASS"
