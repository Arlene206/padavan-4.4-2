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
chmod +x "$tmpdir/bin/logger" "$tmpdir/bin/sleep" "$tmpdir/bin/umount"
PATH="$tmpdir/bin:$PATH"
export PATH

cat > "$tmpdir/services-fail/S01-first" <<'MOCK'
#!/bin/sh
printf 'S01-first\n' >> "$SERVICE_LOG"
exit 0
MOCK
cat > "$tmpdir/services-fail/S02-fail" <<'MOCK'
#!/bin/sh
printf 'S02-fail\n' >> "$SERVICE_LOG"
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
cat > "$tmpdir/services-ok/S02-last" <<'MOCK'
#!/bin/sh
printf 'S02-last\n' >> "$SERVICE_LOG"
exit 0
MOCK
chmod +x "$tmpdir"/services-fail/S??* "$tmpdir"/services-ok/S??*

. "$helper"

SERVICE_LOG="$tmpdir/service.log"
export SERVICE_LOG
if optware_lifecycle_stop_services "$tmpdir/services-fail"; then
	echo "service stop failure was not propagated" >&2
	exit 1
fi
expected=$(printf 'S03-last\nS02-fail\nS01-first')
[ "$(cat "$SERVICE_LOG")" = "$expected" ] || {
	echo "failed service did not preserve reverse order / continue attempts" >&2
	exit 1
}

: > "$SERVICE_LOG"
if ! optware_lifecycle_stop_services "$tmpdir/services-ok"; then
	echo "successful service stop set returned failure" >&2
	exit 1
fi
expected=$(printf 'S02-last\nS01-first')
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

echo "Optware teardown aggregates service-stop failures and propagates /opt unmount retry status: PASS"
