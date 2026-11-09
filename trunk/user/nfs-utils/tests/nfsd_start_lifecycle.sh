#!/bin/sh
set -eu

repo_root=$(CDPATH= cd -- "$(dirname "$0")/../../../.." && pwd)
source_script="$repo_root/trunk/user/nfs-utils/nfsd.sh"
tmp_dir=$(mktemp -d /tmp/nfsd-start-regression.XXXXXX)
trap 'rm -rf "$tmp_dir"' EXIT HUP INT TERM

mkdir -p "$tmp_dir/sbin" "$tmp_dir/bin" "$tmp_dir/etc" \
	"$tmp_dir/proc/fs/nfsd" "$tmp_dir/var/lib/nfs"
printf 'nodev\tnfsd\n' > "$tmp_dir/proc/filesystems"
: > "$tmp_dir/proc/mounts"
: > "$tmp_dir/proc/fs/nfsd/max_block_size"

sed \
	-e "s|/sbin/rpcbind|$tmp_dir/sbin/rpcbind|g" \
	-e "s|/sbin/exportfs|$tmp_dir/sbin/exportfs|g" \
	-e "s|/sbin/rpc.nfsd|$tmp_dir/sbin/rpc.nfsd|g" \
	-e "s|/sbin/rpc.mountd|$tmp_dir/sbin/rpc.mountd|g" \
	-e "s|/proc/fs/nfsd|$tmp_dir/proc/fs/nfsd|g" \
	-e "s|/var/lib/nfs|$tmp_dir/var/lib/nfs|g" \
	-e "s|/etc/exports|$tmp_dir/etc/exports|g" \
	-e "s|/proc/filesystems|$tmp_dir/proc/filesystems|g" \
	-e "s|/proc/mounts|$tmp_dir/proc/mounts|g" \
	"$source_script" > "$tmp_dir/nfsd.sh"

cat > "$tmp_dir/bin/mock-command" <<'MOCK'
#!/bin/sh
name=${0##*/}
printf '%s %s\n' "$name" "$*" >> "$COMMAND_LOG"

case "$name" in
	pidof)
		case "$1" in
			rpcbind) state_file="$STATE_DIR/rpcbind" ;;
			nfsd) state_file="$STATE_DIR/nfsd" ;;
			rpc.mountd) state_file="$STATE_DIR/rpc.mountd" ;;
			*) exit 1 ;;
		esac
		if [ -f "$state_file" ]; then
			echo 123
			exit 0
		fi
		exit 1
		;;
	logger|sleep)
		exit 0
		;;
	rpcbind|rpc.nfsd|rpc.mountd)
		[ "$FAIL_COMMAND" != "$name" ] || exit 17
		case "$name" in
			rpcbind) state_file="$STATE_DIR/rpcbind" ;;
			rpc.nfsd) state_file="$STATE_DIR/nfsd" ;;
			rpc.mountd) state_file="$STATE_DIR/rpc.mountd" ;;
		esac
		[ "$NO_PID_COMMAND" = "$name" ] || : > "$state_file"
		exit 0
		;;
	exportfs|modprobe|mount|umount|killall)
		[ "$FAIL_COMMAND" != "$name" ] || exit 17
		case "$name" in
			mount)
				printf 'mock %s nfsd rw 0 0\n' "$4" >> "$MOUNTS_FILE"
				;;
			umount)
				awk -v path="$1" '$2 != path' "$MOUNTS_FILE" > "$MOUNTS_FILE.tmp"
				mv "$MOUNTS_FILE.tmp" "$MOUNTS_FILE"
				;;
			killall)
				for target in "$@"; do :; done
				case "$target" in
					nfsd) rm -f "$STATE_DIR/nfsd" ;;
					rpc.mountd) rm -f "$STATE_DIR/rpc.mountd" ;;
				esac
				;;
		esac
		exit 0
		;;
esac
exit 0
MOCK
chmod +x "$tmp_dir/bin/mock-command"
for command in pidof logger sleep mount modprobe killall umount; do
	ln -s mock-command "$tmp_dir/bin/$command"
done
for command in rpcbind exportfs rpc.nfsd rpc.mountd; do
	ln -s ../bin/mock-command "$tmp_dir/sbin/$command"
done

export COMMAND_LOG="$tmp_dir/commands.log"
export STATE_DIR="$tmp_dir/state"
export MOUNTS_FILE="$tmp_dir/proc/mounts"
mkdir -p "$STATE_DIR"
export PATH="$tmp_dir/bin:$PATH"

reset_fixture()
{
	rm -f "$STATE_DIR/rpcbind" "$STATE_DIR/nfsd" "$STATE_DIR/rpc.mountd"
	rm -f "$tmp_dir/etc/exports" "$tmp_dir/var/lib/nfs/etab" \
		"$tmp_dir/var/lib/nfs/rmtab" "$tmp_dir/var/lib/nfs/xtab"
	: > "$COMMAND_LOG"
	: > "$MOUNTS_FILE"
	printf 'nodev\tnfsd\n' > "$tmp_dir/proc/filesystems"
	: > "$tmp_dir/proc/fs/nfsd/max_block_size"
}

run_command()
{
	fail_command=$1
	no_pid_command=$2
	action=$3
	if FAIL_COMMAND="$fail_command" NO_PID_COMMAND="$no_pid_command" \
		/bin/sh "$tmp_dir/nfsd.sh" "$action"; then
		status=0
	else
		status=$?
	fi
	printf '%s\n' "$status"
}

assert_command()
{
	grep -q "^$1 " "$COMMAND_LOG" || {
		echo "expected command not executed: $1" >&2
		exit 1
	}
}

assert_no_command()
{
	if grep -q "^$1 " "$COMMAND_LOG"; then
		echo "unexpected command executed: $1" >&2
		exit 1
	fi
}

assert_failed()
{
	[ "$1" -ne 0 ] || {
		echo "$2 unexpectedly returned success" >&2
		exit 1
	}
}

reset_fixture
status=$(run_command rpcbind "" start)
assert_failed "$status" "rpcbind failure"
assert_command rpcbind
assert_no_command exportfs

reset_fixture
status=$(run_command "" rpcbind start)
assert_failed "$status" "rpcbind process check"
assert_command rpcbind
assert_no_command exportfs

reset_fixture
: > "$tmp_dir/proc/filesystems"
status=$(run_command modprobe "" start)
assert_failed "$status" "nfsd module failure"
assert_command modprobe
assert_no_command mount
assert_no_command exportfs

reset_fixture
status=$(run_command mount "" start)
assert_failed "$status" "nfsd control filesystem mount failure"
assert_command mount
assert_no_command exportfs

reset_fixture
printf 'mock %s nfsd rw 0 0\n' "$tmp_dir/proc/fs/nfsd" > "$MOUNTS_FILE"
status=$(run_command mount "" start)
assert_failed "$status" "rpc_pipefs mount failure"
assert_command mount
assert_no_command exportfs

reset_fixture
status=$(run_command exportfs "" start)
assert_failed "$status" "exportfs failure"
assert_command exportfs
assert_no_command rpc.nfsd
assert_no_command rpc.mountd

reset_fixture
status=$(run_command rpc.nfsd "" start)
assert_failed "$status" "rpc.nfsd failure"
assert_command rpc.nfsd
assert_no_command rpc.mountd
if grep -q " $tmp_dir/var/lib/nfs/rpc_pipefs " "$MOUNTS_FILE"; then
	echo "rpc_pipefs remained mounted after nfsd startup failure" >&2
	exit 1
fi

reset_fixture
status=$(run_command rpc.mountd "" start)
assert_failed "$status" "rpc.mountd failure"
assert_command rpc.nfsd
assert_command rpc.mountd
[ ! -f "$STATE_DIR/nfsd" ] || {
	echo "nfsd process survived rpc.mountd startup rollback" >&2
	exit 1
}
[ ! -s "$MOUNTS_FILE" ] || {
	echo "NFS pseudo-filesystems remained mounted after rpc.mountd startup rollback" >&2
	exit 1
}
[ ! -f "$tmp_dir/var/lib/nfs/etab" ] || {
	echo "etab was not removed after successful startup rollback" >&2
	exit 1
}

reset_fixture
status=$(run_command "" rpc.nfsd start)
assert_failed "$status" "rpc.nfsd false success"
assert_command rpc.nfsd
assert_no_command rpc.mountd

reset_fixture
status=$(run_command "" rpc.mountd start)
assert_failed "$status" "rpc.mountd false success"
assert_command rpc.nfsd
assert_command rpc.mountd
[ ! -f "$STATE_DIR/nfsd" ] || {
	echo "nfsd process survived mountd false-success rollback" >&2
	exit 1
}

reset_fixture
: > "$STATE_DIR/rpcbind"
: > "$STATE_DIR/nfsd"
: > "$STATE_DIR/rpc.mountd"
: > "$tmp_dir/etc/exports"
status=$(run_command exportfs "" start)
assert_failed "$status" "running-service exportfs failure"
assert_command exportfs
assert_no_command rpc.nfsd
[ -f "$STATE_DIR/nfsd" ] && [ -f "$STATE_DIR/rpc.mountd" ] || {
	echo "running service was disturbed after exportfs failure" >&2
	exit 1
}

reset_fixture
: > "$STATE_DIR/rpcbind"
: > "$STATE_DIR/nfsd"
printf 'recovery-state\n' > "$tmp_dir/var/lib/nfs/etab"
status=$(run_command "" "" start)
assert_failed "$status" "partial NFS daemon state"
[ "$(cat "$tmp_dir/var/lib/nfs/etab")" = "recovery-state" ] || {
	echo "partial-service start erased the previous export recovery state" >&2
	exit 1
}
assert_no_command exportfs

reset_fixture
: > "$STATE_DIR/nfsd"
: > "$STATE_DIR/rpc.mountd"
: > "$tmp_dir/etc/exports"
status=$(run_command "" "" reload)
assert_failed "$status" "reload without rpcbind"
assert_no_command exportfs

reset_fixture
: > "$STATE_DIR/rpcbind"
: > "$STATE_DIR/nfsd"
status=$(run_command "" "" reload)
assert_failed "$status" "reload without exports file"
assert_no_command exportfs

reset_fixture
: > "$STATE_DIR/rpcbind"
: > "$STATE_DIR/nfsd"
: > "$STATE_DIR/rpc.mountd"
: > "$tmp_dir/etc/exports"
status=$(run_command exportfs "" reload)
assert_failed "$status" "reload exportfs failure"
assert_command exportfs

: > "$COMMAND_LOG"
status=$(run_command "" "" reload)
[ "$status" -eq 0 ] || {
	echo "successful NFS reload returned $status" >&2
	exit 1
}
assert_command exportfs
assert_no_command rpc.nfsd

reset_fixture
status=$(run_command "" "" start)
[ "$status" -eq 0 ] || {
	echo "successful NFS start returned $status" >&2
	exit 1
}
assert_command rpcbind
assert_command mount
assert_command exportfs
assert_command rpc.nfsd
assert_command rpc.mountd
exportfs_line=$(grep -n '^exportfs -r$' "$COMMAND_LOG" | head -n 1 | cut -d: -f1)
nfsd_line=$(grep -n '^rpc.nfsd ' "$COMMAND_LOG" | cut -d: -f1)
mountd_line=$(grep -n '^rpc.mountd ' "$COMMAND_LOG" | cut -d: -f1)
[ "$exportfs_line" -lt "$nfsd_line" ] && [ "$nfsd_line" -lt "$mountd_line" ] || {
	echo "NFS start commands ran out of order" >&2
	exit 1
}
[ -f "$STATE_DIR/nfsd" ] && [ -f "$STATE_DIR/rpc.mountd" ] || {
	echo "NFS start returned success without both daemons" >&2
	exit 1
}

echo "NFS daemon start/reload failure propagation and rollback: PASS"
