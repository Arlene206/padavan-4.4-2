#!/bin/sh
set -eu

script_dir=$(CDPATH= cd -- "$(dirname "$0")" && pwd)
helper="$script_dir/../files/sbin/optware_lifecycle_lock.sh"
tmpdir=$(mktemp -d "/tmp/padavan-optware-teardown.XXXXXX")
cleanup()
{
	rm -rf "$tmpdir"
}
trap cleanup 0 HUP INT TERM

mkdir -p "$tmpdir/bin" "$tmpdir/services-fail" "$tmpdir/services-ok"

cat > "$tmpdir/bin/logger" <<'MOCK'
#!/bin/sh
exit 0
MOCK
cat > "$tmpdir/bin/sleep" <<'MOCK'
#!/bin/sh
exit 0
MOCK
cat > "$tmpdir/bin/umount" <<'MOCK'
#!/bin/sh
count=0
[ ! -f "$UMOUNT_LOG" ] || count=$(cat "$UMOUNT_LOG")
count=$((count + 1))
printf '%s\n' "$count" > "$UMOUNT_LOG"
case "$UMOUNT_MODE:$count" in
	success:*) exit 0 ;;
	retry:1) exit 1 ;;
	retry:*) exit 0 ;;
	fail:*) exit 1 ;;
	*) exit 2 ;;
esac
MOCK
cat > "$tmpdir/bin/swapoff" <<'MOCK'
#!/bin/sh
[ "$1" = "/opt/.swap" ] || exit 2
if flock -n 9 2>/dev/null; then
	echo "swapoff inherited Optware lifecycle lock fd 9" >&2
	exit 90
fi
printf '%s\n' "$1" >> "$SWAPOFF_LOG"
case "$SWAPOFF_MODE" in
	success)
		printf 'Filename Type Size Used Priority\n' > "$SWAPS_FILE"
		exit 0
		;;
	fail)
		exit 1
		;;
	remain)
		exit 0
		;;
	*)
		exit 2
		;;
esac
MOCK
chmod +x "$tmpdir/bin/logger" "$tmpdir/bin/sleep" "$tmpdir/bin/umount" "$tmpdir/bin/swapoff"
PATH="$tmpdir/bin:$PATH"
export PATH

cat > "$tmpdir/services-fail/S01-first" <<'MOCK'
#!/bin/sh
printf 'S01-first\n' >> "$SERVICE_LOG"
exit 0
MOCK
cat > "$tmpdir/services-fail/S02 fail with space" <<'MOCK'
#!/bin/sh
printf 'S02 fail with space\n' >> "$SERVICE_LOG"
exit 1
MOCK
cat > "$tmpdir/services-fail/S03-last" <<'MOCK'
#!/bin/sh
printf 'S03-last\n' >> "$SERVICE_LOG"
exit 0
MOCK
cat > "$tmpdir/services-ok/S01-first" <<'MOCK'
#!/bin/sh
printf 'S01-first\n' >> "$SERVICE_LOG"
exit 0
MOCK
cat > "$tmpdir/services-ok/S02 last with space" <<'MOCK'
#!/bin/sh
printf 'S02 last with space\n' >> "$SERVICE_LOG"
exit 0
MOCK
chmod +x "$tmpdir"/services-fail/S??* "$tmpdir"/services-ok/S??*

. "$helper"

LOCK_FILE="$tmpdir/optware-lifecycle.lock"
optware_lifecycle_lock_acquire "$LOCK_FILE"

SERVICE_LOG="$tmpdir/service.log"
export SERVICE_LOG
if optware_lifecycle_stop_services "$tmpdir/services-fail"; then
	echo "service stop failure was not propagated" >&2
	exit 1
fi
expected=$(printf 'S03-last\nS02 fail with space\nS01-first')
[ "$(cat "$SERVICE_LOG")" = "$expected" ] || {
	echo "failed service did not preserve reverse order / continue attempts" >&2
	exit 1
}

: > "$SERVICE_LOG"
if ! optware_lifecycle_stop_services "$tmpdir/services-ok"; then
	echo "successful service stop set returned failure" >&2
	exit 1
fi
expected=$(printf 'S02 last with space\nS01-first')
[ "$(cat "$SERVICE_LOG")" = "$expected" ] || {
	echo "successful service stop order mismatch" >&2
	exit 1
}

UMOUNT_LOG="$tmpdir/umount.calls"
export UMOUNT_LOG
run_umount_test()
{
	UMOUNT_MODE=$1
	expected_status=$2
	expected_calls=$3
	export UMOUNT_MODE
	: > "$UMOUNT_LOG"
	if optware_lifecycle_umount_opt; then
		actual_status=0
	else
		actual_status=$?
	fi
	[ "$actual_status" -eq "$expected_status" ] || {
		echo "umount mode $UMOUNT_MODE returned $actual_status, expected $expected_status" >&2
		exit 1
	}
	actual_calls=$(cat "$UMOUNT_LOG")
	[ "$actual_calls" -eq "$expected_calls" ] || {
		echo "umount mode $UMOUNT_MODE made $actual_calls attempts, expected $expected_calls" >&2
		exit 1
	}
}

run_umount_test success 0 1
run_umount_test retry 0 2
run_umount_test fail 1 2

# Swap release is based on an exact /proc/swaps entry, not file existence or
# a path prefix. Unknown or malformed inventory and failed swapoff are fatal.
SWAPS_FILE="$tmpdir/proc-swaps"
SWAPOFF_LOG="$tmpdir/swapoff.log"
export SWAPS_FILE SWAPOFF_LOG
swap_header='Filename Type Size Used Priority'
: > "$SWAPOFF_LOG"
printf '%s\n' "$swap_header" > "$SWAPS_FILE"
SWAPOFF_MODE=fail
export SWAPOFF_MODE
if ! optware_lifecycle_swapoff_file /opt/.swap "$SWAPS_FILE"; then
	echo "inactive swap file was treated as a teardown failure" >&2
	exit 1
fi
[ ! -s "$SWAPOFF_LOG" ] || {
	echo "inactive swap file unexpectedly invoked swapoff" >&2
	exit 1
}

printf '%s\n' "$swap_header" '/opt/.swap.old file 1024 0 -2' > "$SWAPS_FILE"
if ! optware_lifecycle_swapoff_file /opt/.swap "$SWAPS_FILE"; then
	echo "swap path prefix was mistaken for the exact swap file" >&2
	exit 1
fi
[ ! -s "$SWAPOFF_LOG" ] || {
	echo "prefix-only swap entry unexpectedly invoked swapoff" >&2
	exit 1
}

printf '%s\n' "$swap_header" '/opt/.swap file 1024 1 -2' > "$SWAPS_FILE"
SWAPOFF_MODE=success
export SWAPOFF_MODE
if ! optware_lifecycle_swapoff_file /opt/.swap "$SWAPS_FILE"; then
	echo "successful swapoff and inactive postcondition were rejected" >&2
	exit 1
fi
[ "$(wc -l < "$SWAPOFF_LOG")" -eq 1 ] || {
	echo "active swap was not released exactly once" >&2
	exit 1
}
[ "$(cat "$SWAPS_FILE")" = "$swap_header" ] || {
	echo "successful swapoff mock did not publish the inactive inventory" >&2
	exit 1
}

: > "$SWAPOFF_LOG"
printf '%s\n' "$swap_header" '/opt/.swap file 1024 1 -2' > "$SWAPS_FILE"
SWAPOFF_MODE=fail
export SWAPOFF_MODE
if optware_lifecycle_swapoff_file /opt/.swap "$SWAPS_FILE"; then
	echo "swapoff command failure was reported as success" >&2
	exit 1
fi
[ "$(wc -l < "$SWAPOFF_LOG")" -eq 1 ] || {
	echo "active swapoff failure was not attempted exactly once" >&2
	exit 1
}

: > "$SWAPOFF_LOG"
printf '%s\n' "$swap_header" '/opt/.swap file 1024 1 -2' > "$SWAPS_FILE"
SWAPOFF_MODE=remain
export SWAPOFF_MODE
if optware_lifecycle_swapoff_file /opt/.swap "$SWAPS_FILE"; then
	echo "swap remaining active after a successful command was accepted" >&2
	exit 1
fi

: > "$SWAPOFF_LOG"
rm -f "$SWAPS_FILE"
SWAPOFF_MODE=success
export SWAPOFF_MODE
if optware_lifecycle_swapoff_file /opt/.swap "$SWAPS_FILE"; then
	echo "unreadable swap inventory was treated as inactive" >&2
	exit 1
fi
[ ! -s "$SWAPOFF_LOG" ] || {
	echo "swapoff was attempted without a readable inventory" >&2
	exit 1
}

printf '%s\n' "$swap_header" '/opt/.swap file invalid used -2' > "$SWAPS_FILE"
if optware_lifecycle_swapoff_file /opt/.swap "$SWAPS_FILE"; then
	echo "malformed swap inventory was treated as inactive" >&2
	exit 1
fi
[ ! -s "$SWAPOFF_LOG" ] || {
	echo "swapoff was attempted with a malformed inventory" >&2
	exit 1
}

# Missing service directories fail closed, while an existing empty init.d
# directory is a valid installation with no registered services.
if optware_lifecycle_stop_services "$tmpdir/missing"; then
	echo "missing service directory was treated as a successful stop" >&2
	exit 1
fi
mkdir -p "$tmpdir/services-empty"
if ! optware_lifecycle_stop_services "$tmpdir/services-empty"; then
	echo "empty service directory was treated as a failed stop" >&2
	exit 1
fi

optware_lifecycle_lock_release
echo "Optware teardown, exact swap inventory, and fail-closed swapoff policy: PASS"
