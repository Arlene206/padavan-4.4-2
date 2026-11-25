#!/bin/sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
repo_root=$(CDPATH= cd -- "$script_dir/../../../.." && pwd)
sandbox=$(mktemp -d "${TMPDIR:-/tmp}/rwfs2ubi-status.XXXXXX")
real_rm=$(command -v rm)
real_mkdir=$(command -v mkdir)
trap 'status=$?; "$real_rm" -rf "$sandbox"; exit "$status"' EXIT HUP INT TERM

fail()
{
	echo "RWFS-to-UBI status regression: FAIL: $*" >&2
	exit 1
}

prepare_case()
{
	name=$1
	base="$sandbox/$name"
	"$real_mkdir" -p "$base/bin" "$base/sbin" "$base/proc" "$base/dev" "$base/private"
	: > "$base/dev/mtd5"
	cat > "$base/proc/mtd" <<'EOF_MTD'
dev:    size   erasesize  name
mtd4: 00010000 00001000 "RWFS_BACKUP"
mtd5: 00010000 00001000 "RWFS"
EOF_MTD
	CALL_LOG="$base/calls.log"
	UBIDETACH_COUNT="$base/ubidetach.count"
	export CALL_LOG UBIDETACH_COUNT
	: > "$CALL_LOG"
	: > "$UBIDETACH_COUNT"

	cat > "$base/bin/mount" <<'EOF_MOUNT'
#!/bin/sh
printf '%s\n' "${MOUNT_OUTPUT:-}"
exit "${MOUNT_STATUS:-0}"
EOF_MOUNT
	cat > "$base/bin/mtd_write" <<'EOF_MTD_WRITE'
#!/bin/sh
printf '%s\n' "mtd_write $*" >> "$CALL_LOG"
exit "${MTD_WRITE_STATUS:-0}"
EOF_MTD_WRITE
	cat > "$base/bin/flock" <<'EOF_FLOCK'
#!/bin/sh
printf '%s\n' "flock $*" >> "$CALL_LOG"
exit "${FLOCK_STATUS:-0}"
EOF_FLOCK
	cat > "$base/bin/nvram" <<'EOF_NVRAM'
#!/bin/sh
printf '%s\n' "nvram $*" >> "$CALL_LOG"
case "$1" in
	set) exit "${NVRAM_SET_STATUS:-0}" ;;
	commit) exit "${NVRAM_COMMIT_STATUS:-0}" ;;
	*) exit 64 ;;
esac
EOF_NVRAM
	cat > "$base/sbin/ubiattach" <<'EOF_UBI'
#!/bin/sh
tool=${0##*/}
printf '%s\n' "$tool $*" >> "$CALL_LOG"
case "$tool" in
	ubiformat) exit "${UBIFORMAT_STATUS:-0}" ;;
	ubiattach) exit "${UBIATTACH_STATUS:-0}" ;;
	ubimkvol) exit "${UBIMKVOL_STATUS:-0}" ;;
	ubidetach)
		count=$(cat "$UBIDETACH_COUNT")
		count=$((count + 1))
		printf '%s\n' "$count" > "$UBIDETACH_COUNT"
		if [ "$count" -eq 1 ]; then
			exit "${UBIDETACH_INITIAL_STATUS:-1}"
		fi
		exit "${UBIDETACH_FINAL_STATUS:-0}"
		;;
esac
exit 64
EOF_UBI
	ln "$base/sbin/ubiattach" "$base/sbin/ubiformat"
	ln "$base/sbin/ubiattach" "$base/sbin/ubimkvol"
	ln "$base/sbin/ubiattach" "$base/sbin/ubidetach"
	chmod +x "$base/bin/mount" "$base/bin/mtd_write" "$base/bin/flock" "$base/bin/nvram" "$base/sbin/ubiattach"

	# Use explicit wrappers so both host sh and BusyBox ash resolve the
	# same firmware applets through this isolated command PATH.
	if [ -n "${BUSYBOX_BIN:-}" ]; then
		[ -x "$BUSYBOX_BIN" ] || fail "BUSYBOX_BIN is not executable"
		cat > "$base/bin/awk" <<'EOF_AWK'
#!/bin/sh
exec "$BUSYBOX_BIN" awk "$@"
EOF_AWK
		cat > "$base/bin/cat" <<'EOF_CAT'
#!/bin/sh
exec "$BUSYBOX_BIN" cat "$@"
EOF_CAT
		chmod +x "$base/bin/awk" "$base/bin/cat"
	fi

	sed \
		-e "s|^sbin_dir=.*|sbin_dir=\"$base/sbin\"|" \
		-e "s|^proc_mtd=.*|proc_mtd=\"$base/proc/mtd\"|" \
		-e "s|^dev_dir=.*|dev_dir=\"$base/dev\"|" \
		-e "s|^operation_lock=.*|operation_lock=\"$base/private/rwfs2ubi.lock\"|" \
		"$repo_root/trunk/user/scripts/files/sbin/rwfs2ubi.sh" > "$base/rwfs2ubi.sh"

	PATH="$base/bin:$PATH"
	export PATH
	MOUNT_OUTPUT=
	MOUNT_STATUS=0
	MTD_WRITE_STATUS=0
	FLOCK_STATUS=0
	UBIFORMAT_STATUS=0
	UBIATTACH_STATUS=0
	UBIMKVOL_STATUS=0
	UBIDETACH_INITIAL_STATUS=1
	UBIDETACH_FINAL_STATUS=0
	NVRAM_SET_STATUS=0
	NVRAM_COMMIT_STATUS=0
	export MOUNT_OUTPUT MOUNT_STATUS MTD_WRITE_STATUS FLOCK_STATUS UBIFORMAT_STATUS
	export UBIATTACH_STATUS UBIMKVOL_STATUS UBIDETACH_INITIAL_STATUS
	export UBIDETACH_FINAL_STATUS NVRAM_SET_STATUS NVRAM_COMMIT_STATUS
}

run_conversion()
{
	if [ -n "${BUSYBOX_BIN:-}" ]; then
		PATH="$base/bin:$PATH" "$BUSYBOX_BIN" ash "$base/rwfs2ubi.sh"
	else
		PATH="$base/bin:$PATH" sh "$base/rwfs2ubi.sh"
	fi
}

expect_failure()
{
	if run_conversion; then
		fail "injected $1 failure unexpectedly succeeded"
	fi
}

assert_logged()
{
	grep -Fqx -- "$1" "$CALL_LOG" || fail "missing command: $1"
}

assert_not_logged()
{
	if grep -Fqx -- "$1" "$CALL_LOG"; then
		fail "unexpected command after failed prerequisite: $1"
	fi
}

# Ignore a substring decoy, select the exact RWFS partition, and keep working
# when the optional pre-format detach reports that no attachment exists.
prepare_case success
MOUNT_OUTPUT="/dev/ubi50_0 on /media/other type ubifs (rw)"
export MOUNT_OUTPUT
run_conversion || fail "valid conversion failed"
assert_logged "mtd_write erase $base/dev/mtd5"
assert_logged "ubiformat $base/dev/mtd5 -y -q"
assert_logged "ubiattach -p $base/dev/mtd5 -d 5"
assert_logged "ubimkvol $base/dev/ubi5 -m -N rwfs"
assert_logged "nvram set mtd_rwfs_mount=1"
assert_logged "nvram commit"
[ "$(grep -Fc 'ubidetach -p ' "$CALL_LOG")" -eq 2 ] || fail "expected preflight and final detach attempts"
awk '
	/mtd_write erase/ { erase = NR }
	/ubiformat / { format = NR }
	/^ubiattach / { attach = NR }
	/^ubimkvol / { volume = NR }
	/^nvram set / { setting = NR }
	/^nvram commit$/ { commit = NR }
	/^ubidetach / { detach = NR }
	END { exit !(erase < format && format < attach && attach < volume &&
		     volume < setting && setting < commit && commit < detach) }
' "$CALL_LOG" || fail "successful operation order changed"

# A concurrent conversion must fail before inspecting or changing the MTD.
prepare_case conversion-lock-contention
FLOCK_STATUS=1
export FLOCK_STATUS
expect_failure "conversion lock contention"
grep -Fqx 'flock -n 9' "$CALL_LOG"
assert_not_logged "mtd_write erase $base/dev/mtd5"

# A real mounted RWFS volume is rejected before the first detach/erase.
prepare_case mounted-ubi
MOUNT_OUTPUT="/dev/ubi5_0 on /media/mtd_rwfs type ubifs (rw)"
export MOUNT_OUTPUT
expect_failure "mounted UBI"
[ ! -s "$CALL_LOG" ] || fail "mounted UBI caused a destructive command"

prepare_case mounted-ubiblock
MOUNT_OUTPUT="/dev/ubiblock5_0 on /media/mtd_rwfs type squashfs (ro)"
export MOUNT_OUTPUT
expect_failure "mounted ubiblock"
[ ! -s "$CALL_LOG" ] || fail "mounted ubiblock caused a destructive command"

prepare_case mounted-mtd
MOUNT_OUTPUT="/dev/mtdblock5 on /media/mtd_rwfs type jffs2 (rw)"
export MOUNT_OUTPUT
expect_failure "mounted MTD block"
[ ! -s "$CALL_LOG" ] || fail "mounted MTD caused a destructive command"

prepare_case mount-inventory-failure
MOUNT_STATUS=2
export MOUNT_STATUS
expect_failure "mount inventory"
[ ! -s "$CALL_LOG" ] || fail "unknown mount inventory caused a destructive command"

prepare_case malformed-mount-inventory
MOUNT_OUTPUT="malformed mount record"
export MOUNT_OUTPUT
expect_failure "malformed mount inventory"
[ ! -s "$CALL_LOG" ] || fail "malformed mount inventory caused a destructive command"

prepare_case empty-mount-inventory
MOUNT_OUTPUT=
export MOUNT_OUTPUT
expect_failure "empty mount inventory"
[ ! -s "$CALL_LOG" ] || fail "empty mount inventory caused a destructive command"

prepare_case missing-rwfs
cat > "$base/proc/mtd" <<'EOF_MISSING'
dev:    size   erasesize  name
mtd4: 00010000 00001000 "RWFS_BACKUP"
EOF_MISSING
expect_failure "missing exact RWFS partition"
[ ! -s "$CALL_LOG" ] || fail "missing partition caused a destructive command"

prepare_case duplicate-rwfs
cat >> "$base/proc/mtd" <<'EOF_DUPLICATE'
mtd6: 00010000 00001000 "RWFS"
EOF_DUPLICATE
expect_failure "ambiguous RWFS partition"
[ ! -s "$CALL_LOG" ] || fail "ambiguous partition caused a destructive command"

prepare_case erase-failure
MTD_WRITE_STATUS=1
export MTD_WRITE_STATUS
expect_failure "MTD erase"
assert_logged "mtd_write erase $base/dev/mtd5"
assert_not_logged "ubiformat $base/dev/mtd5 -y -q"
assert_not_logged "ubiattach -p $base/dev/mtd5 -d 5"

prepare_case format-failure
UBIFORMAT_STATUS=2
export UBIFORMAT_STATUS
expect_failure "UBI format"
assert_logged "ubiformat $base/dev/mtd5 -y -q"
assert_not_logged "ubiattach -p $base/dev/mtd5 -d 5"

prepare_case attach-failure
UBIATTACH_STATUS=3
export UBIATTACH_STATUS
expect_failure "UBI attach"
assert_logged "ubiattach -p $base/dev/mtd5 -d 5"
[ "$(grep -Fc 'ubidetach -p ' "$CALL_LOG")" -eq 2 ] || fail "failed attach did not trigger cleanup detach"
assert_not_logged "ubimkvol $base/dev/ubi5 -m -N rwfs"

prepare_case volume-failure
UBIMKVOL_STATUS=4
export UBIMKVOL_STATUS
expect_failure "UBI volume creation"
assert_logged "ubimkvol $base/dev/ubi5 -m -N rwfs"
assert_not_logged "nvram set mtd_rwfs_mount=1"
assert_not_logged "nvram commit"
assert_logged "ubidetach -p $base/dev/mtd5"

prepare_case nvram-set-failure
NVRAM_SET_STATUS=5
export NVRAM_SET_STATUS
expect_failure "NVRAM set"
assert_logged "nvram set mtd_rwfs_mount=1"
assert_not_logged "nvram commit"
assert_logged "ubidetach -p $base/dev/mtd5"

prepare_case nvram-commit-failure
NVRAM_COMMIT_STATUS=6
export NVRAM_COMMIT_STATUS
expect_failure "NVRAM commit"
assert_logged "nvram set mtd_rwfs_mount=1"
assert_logged "nvram commit"
assert_logged "ubidetach -p $base/dev/mtd5"

prepare_case final-detach-failure
UBIDETACH_FINAL_STATUS=7
export UBIDETACH_FINAL_STATUS
expect_failure "final detach"
assert_logged "nvram commit"

echo "RWFS-to-UBI flash, mount and NVRAM status regression: PASS"
