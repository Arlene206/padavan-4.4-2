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
	-e "s|nfsd_kill_command=\"kill\"|nfsd_kill_command=\"$tmp_dir/bin/kill\"|" \
	"$source_script" > "$tmp_dir/nfsd.sh"

cat > "$tmp_dir/bin/mock-command" <<'MOCK'
#!/bin/sh
name=${0##*/}
printf '%s %s\n' "$name" "$*" >> "$COMMAND_LOG"

write_proc_entry()
{
	process_pid=$1
	process_name=$2
	mkdir -p "$PROC_ROOT/$process_pid"
	printf '%s\n' "$process_name" > "$PROC_ROOT/$process_pid/comm"
	printf '%s (%s) S 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 12345\n' \
		"$process_pid" "$process_name" > "$PROC_ROOT/$process_pid/stat"
}

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
	logger)
		[ "$FAIL_LOGGER" != "1" ] || exit 19
		exit 0
		;;
	sleep)
		if [ -n "$DELAY_PID_COMMAND" ] &&
		   [ -f "$STATE_DIR/delay-$DELAY_PID_COMMAND" ]; then
			delay_count_file="$STATE_DIR/delay-$DELAY_PID_COMMAND.count"
			delay_count=0
			[ -r "$delay_count_file" ] && delay_count=$(cat "$delay_count_file")
			delay_count=$((delay_count + 1))
			echo "$delay_count" > "$delay_count_file"
			if [ "$delay_count" -ge "$DELAY_PID_AFTER" ]; then
				rm -f "$STATE_DIR/delay-$DELAY_PID_COMMAND"
				case "$DELAY_PID_COMMAND" in
					rpcbind) state_file="$STATE_DIR/rpcbind"; process_pid=125; process_name=rpcbind ;;
					rpc.mountd) state_file="$STATE_DIR/rpc.mountd"; process_pid=124; process_name=rpc.mountd ;;
					*) exit 2 ;;
				esac
				: > "$state_file"
				write_proc_entry "$process_pid" "$process_name"
			fi
		fi
		exit 0
		;;
	rpcbind|rpc.nfsd|rpc.mountd)
		[ "$FAIL_COMMAND" != "$name" ] || exit 17
		case "$name" in
			rpcbind) state_file="$STATE_DIR/rpcbind"; process_pid=125; process_name=rpcbind ;;
			rpc.nfsd) state_file="$STATE_DIR/nfsd"; process_pid=123; process_name=nfsd ;;
			rpc.mountd) state_file="$STATE_DIR/rpc.mountd"; process_pid=124; process_name=rpc.mountd ;;
		esac
		if [ "$DELAY_PID_COMMAND" = "$name" ]; then
			: > "$STATE_DIR/delay-$name"
		elif [ "$NO_PID_COMMAND" != "$name" ]; then
			: > "$state_file"
			write_proc_entry "$process_pid" "$process_name"
		fi
		exit 0
		;;
	kill)
		signal_name=$1
		process_pid=$2
		[ "$FAIL_COMMAND" != "kill" ] || exit 17
		case "$process_pid" in
			123) state_file="$STATE_DIR/nfsd" ;;
			124) state_file="$STATE_DIR/rpc.mountd" ;;
			*) exit 2 ;;
		esac
		case "$signal_name" in
			-TERM|-2|-9)
				rm -f "$state_file"
				rm -rf "$PROC_ROOT/$process_pid"
				;;
			*) exit 2 ;;
		esac
		exit 0
		;;
	exportfs|modprobe|mount|umount)
		[ "$FAIL_COMMAND" != "$name" ] || exit 17
		case "$name" in
			mount)
				printf '%s %s %s rw 0 0\n' "$3" "$4" "$2" >> "$MOUNTS_FILE"
				;;
			umount)
				awk -v path="$1" '$2 != path' "$MOUNTS_FILE" > "$MOUNTS_FILE.tmp"
				mv "$MOUNTS_FILE.tmp" "$MOUNTS_FILE"
				;;
			killall)
				for target in "$@"; do :; done
				case "$target" in
					nfsd) rm -f "$STATE_DIR/nfsd"; rm -rf "$PROC_ROOT/123" ;;
					rpc.mountd) rm -f "$STATE_DIR/rpc.mountd"; rm -rf "$PROC_ROOT/124" ;;
				esac
				;;
		esac
		exit 0
		;;
esac
exit 0
MOCK
chmod +x "$tmp_dir/bin/mock-command"
for command in pidof logger sleep mount modprobe kill umount; do
	ln -s mock-command "$tmp_dir/bin/$command"
done
for command in rpcbind exportfs rpc.nfsd rpc.mountd; do
	ln -s ../bin/mock-command "$tmp_dir/sbin/$command"
done

export COMMAND_LOG="$tmp_dir/commands.log"
export STATE_DIR="$tmp_dir/state"
export MOUNTS_FILE="$tmp_dir/proc/mounts"
export PROC_ROOT="$tmp_dir/proc"
export NFSD_PROC_ROOT="$tmp_dir/proc"
mkdir -p "$STATE_DIR"
export PATH="$tmp_dir/bin:$PATH"

mkdir -p "$tmp_dir/no-pidof-bin"
ln -s "$tmp_dir/bin/mock-command" "$tmp_dir/no-pidof-bin/logger"

reset_fixture()
{
	rm -f "$STATE_DIR/rpcbind" "$STATE_DIR/nfsd" "$STATE_DIR/rpc.mountd"
	rm -f "$STATE_DIR"/delay-*
	rm -rf "$PROC_ROOT/1" "$PROC_ROOT/123" "$PROC_ROOT/124" "$PROC_ROOT/125"
	rm -f "$tmp_dir/etc/exports" "$tmp_dir/var/lib/nfs/etab" \
		"$tmp_dir/var/lib/nfs/rmtab" "$tmp_dir/var/lib/nfs/xtab"
	: > "$COMMAND_LOG"
	printf 'rootfs / rootfs rw 0 0\n' > "$MOUNTS_FILE"
	printf 'nodev\tnfsd\n' > "$tmp_dir/proc/filesystems"
	mkdir -p "$PROC_ROOT/self"
	printf 'nfsd-regression\n' > "$PROC_ROOT/self/comm"
	: > "$tmp_dir/proc/fs/nfsd/max_block_size"
}

mark_running_process()
{
	process_name=$1
	case "$process_name" in
		rpcbind) process_pid=125; state_file="$STATE_DIR/rpcbind" ;;
		nfsd) process_pid=123; state_file="$STATE_DIR/nfsd" ;;
		rpc.mountd) process_pid=124; state_file="$STATE_DIR/rpc.mountd" ;;
		*) echo "unsupported test process: $process_name" >&2; exit 1 ;;
	esac
	: > "$state_file"
	mkdir -p "$PROC_ROOT/$process_pid"
	printf '%s\n' "$process_name" > "$PROC_ROOT/$process_pid/comm"
	printf '%s (%s) S 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 12345\n' \
		"$process_pid" "$process_name" > "$PROC_ROOT/$process_pid/stat"
}

prepare_running_nfs()
{
	mark_running_process nfsd
	mark_running_process rpc.mountd
	printf 'nfsd %s nfsd rw 0 0\n' "$tmp_dir/proc/fs/nfsd" >> "$MOUNTS_FILE"
	printf 'rpc_pipefs %s rpc_pipefs rw 0 0\n' "$tmp_dir/var/lib/nfs/rpc_pipefs" >> "$MOUNTS_FILE"
	printf 'recovery-state\n' > "$tmp_dir/var/lib/nfs/etab"
}

run_command()
{
	fail_command=$1
	no_pid_command=$2
	action=$3
	delay_pid_command=${4:-}
	fail_logger=${5:-}
	delay_pid_after=${6:-1}
	if FAIL_COMMAND="$fail_command" NO_PID_COMMAND="$no_pid_command" \
	   DELAY_PID_COMMAND="$delay_pid_command" DELAY_PID_AFTER="$delay_pid_after" \
	   FAIL_LOGGER="$fail_logger" /bin/sh "$tmp_dir/nfsd.sh" "$action"; then
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

# An accessible empty directory is not a valid procfs inventory.
reset_fixture
mkdir -p "$tmp_dir/empty-proc"
status=$(NFSD_PROC_ROOT="$tmp_dir/empty-proc" run_command "" "" start)
assert_failed "$status" "NFS start with non-proc directory at proc root"
assert_no_command rpcbind
assert_no_command mount
assert_no_command exportfs

# An empty mount table or malformed unrelated row is UNKNOWN, not ABSENT.
reset_fixture
: > "$MOUNTS_FILE"
status=$(run_command "" "" start)
assert_failed "$status" "NFS start with empty mount inventory"
assert_no_command mount
assert_no_command umount

reset_fixture
printf 'malformed mount row\n' >> "$MOUNTS_FILE"
mounts_before=$(cat "$MOUNTS_FILE")
status=$(run_command "" "" start)
assert_failed "$status" "NFS start with malformed unrelated mount row"
assert_no_command mount
assert_no_command umount
[ "$(cat "$MOUNTS_FILE")" = "$mounts_before" ] || {
	echo "NFS start modified an incomplete mount inventory" >&2
	exit 1
}

# Missing or incomplete procfs inventories are UNKNOWN, not ABSENT.
reset_fixture
status=$(NFSD_PROC_ROOT="$tmp_dir/missing-proc" run_command "" "" start)
assert_failed "$status" "NFS start with missing process inventory"
assert_command logger
assert_no_command rpcbind
assert_no_command mount
assert_no_command exportfs

# An unknown process scan must not reset an existing recovery/export database.
reset_fixture
mark_running_process rpcbind
prepare_running_nfs
: > "$tmp_dir/etc/exports"
printf 'preserve-rmtab\n' > "$tmp_dir/var/lib/nfs/rmtab"
printf 'preserve-xtab\n' > "$tmp_dir/var/lib/nfs/xtab"
status=$(NFSD_PROC_ROOT="$tmp_dir/missing-proc" run_command "" "" start)
assert_failed "$status" "NFS start with unknown process state and existing recovery files"
[ "$(cat "$tmp_dir/var/lib/nfs/etab")" = "recovery-state" ]
[ "$(cat "$tmp_dir/var/lib/nfs/rmtab")" = "preserve-rmtab" ]
[ "$(cat "$tmp_dir/var/lib/nfs/xtab")" = "preserve-xtab" ]
assert_no_command rpcbind
assert_no_command exportfs
assert_no_command mount

reset_fixture
mark_running_process rpcbind
mark_running_process nfsd
mark_running_process rpc.mountd
: > "$tmp_dir/etc/exports"
status=$(NFSD_PROC_ROOT="$tmp_dir/missing-proc" run_command "" "" reload)
assert_failed "$status" "NFS reload with missing process inventory"
assert_command logger
assert_no_command exportfs

reset_fixture
mark_running_process rpcbind
mark_running_process nfsd
mark_running_process rpc.mountd
: > "$tmp_dir/etc/exports"
printf 'nfsd %s nfsd rw 0 0\nrpc_pipefs %s rpc_pipefs rw 0 0\n' \
	"$tmp_dir/proc/fs/nfsd" "$tmp_dir/var/lib/nfs/rpc_pipefs" > "$MOUNTS_FILE"
mounts_before=$(cat "$MOUNTS_FILE")
status=$(NFSD_PROC_ROOT="$tmp_dir/missing-proc" run_command "" "" stop)
assert_failed "$status" "NFS stop with missing process inventory"
assert_command logger
assert_no_command kill
assert_no_command exportfs
assert_no_command umount
[ -f "$STATE_DIR/nfsd" ] && [ -f "$STATE_DIR/rpc.mountd" ] || {
	echo "NFS stop with missing procfs disturbed daemon state" >&2
	exit 1
}
[ "$(cat "$MOUNTS_FILE")" = "$mounts_before" ] || {
	echo "NFS stop with missing procfs changed mounted filesystem state" >&2
	exit 1
}

# An empty comm entry for a live numeric PID is UNKNOWN.
reset_fixture
prepare_running_nfs
mkdir -p "$PROC_ROOT/1"
: > "$PROC_ROOT/1/comm"
status=$(run_command "" "" stop)
assert_failed "$status" "NFS stop with malformed process inventory"
assert_no_command killall
assert_no_command exportfs
assert_no_command umount
[ -f "$STATE_DIR/nfsd" ] && [ -f "$STATE_DIR/rpc.mountd" ] || {
	echo "NFS stop disturbed daemon state after an incomplete inventory" >&2
	exit 1
}

# rpcbind is detached too: its child may require more than the old fixed
# one-second pause to become visible. The bounded readiness helper must wait
# through two ABSENT samples and accept the child when the third sample sees it.
reset_fixture
status=$(run_command "" "" start rpcbind "" 2)
[ "$status" -eq 0 ] || {
	echo "NFS start rejected rpcbind child after daemonization delay: $status" >&2
	exit 1
}
assert_command rpcbind
[ -f "$STATE_DIR/rpcbind" ] || {
	echo "NFS start returned success before delayed rpcbind child appeared" >&2
	exit 1
}
[ "$(grep -c '^sleep ' "$COMMAND_LOG")" -eq 2 ] || {
	echo "NFS rpcbind readiness wait did not poll for two delayed intervals" >&2
	exit 1
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
assert_command exportfs
! grep -Eq ' (nfsd|rpc_pipefs) ' "$MOUNTS_FILE" && [ ! -f "$tmp_dir/var/lib/nfs/etab" ] || {
	echo "failed NFS control mount left startup state after successful cleanup" >&2
	exit 1
}

reset_fixture
# Keep the ordinary root mount row in the synthetic /proc/mounts inventory.
# Cleanup of the last NFS mount must not turn the fixture into an impossible
# empty mount table, which the production fail-closed parser correctly rejects.
printf 'nfsd %s nfsd rw 0 0\n' "$tmp_dir/proc/fs/nfsd" >> "$MOUNTS_FILE"
status=$(run_command mount "" start)
assert_failed "$status" "rpc_pipefs mount failure"
assert_command mount
assert_command exportfs
! grep -Eq ' (nfsd|rpc_pipefs) ' "$MOUNTS_FILE" || {
	echo "NFS control filesystem remained mounted after rpc_pipefs startup failure" >&2
	exit 1
}
[ ! -f "$tmp_dir/var/lib/nfs/etab" ] || {
	echo "etab remained after successful rpc_pipefs startup rollback" >&2
	exit 1
}

reset_fixture
status=$(run_command exportfs "" start)
assert_failed "$status" "exportfs failure"
assert_command exportfs
assert_no_command rpc.nfsd
assert_no_command rpc.mountd
[ -s "$MOUNTS_FILE" ] && [ -f "$tmp_dir/var/lib/nfs/etab" ] || {
	echo "exportfs failure discarded recovery state despite failed export withdrawal" >&2
	exit 1
}

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
! grep -Eq ' (nfsd|rpc_pipefs) ' "$MOUNTS_FILE" || {
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
mark_running_process rpcbind
mark_running_process nfsd
mark_running_process rpc.mountd
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
mark_running_process rpcbind
mark_running_process nfsd
printf 'recovery-state\n' > "$tmp_dir/var/lib/nfs/etab"
status=$(run_command "" "" start)
assert_failed "$status" "partial NFS daemon state"
[ "$(cat "$tmp_dir/var/lib/nfs/etab")" = "recovery-state" ] || {
	echo "partial-service start erased the previous export recovery state" >&2
	exit 1
}
assert_no_command exportfs

reset_fixture
mark_running_process nfsd
mark_running_process rpc.mountd
: > "$tmp_dir/etc/exports"
status=$(run_command "" "" reload)
assert_failed "$status" "reload without rpcbind"
assert_no_command exportfs

reset_fixture
mark_running_process rpcbind
mark_running_process nfsd
status=$(run_command "" "" reload)
assert_failed "$status" "reload without exports file"
assert_no_command exportfs

reset_fixture
mark_running_process rpcbind
mark_running_process nfsd
mark_running_process rpc.mountd
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


# rpc.mountd daemonizes: the launcher may return before the child becomes
# visible in procfs. A later poll must accept the child without false rollback.
reset_fixture
status=$(run_command "" "" start rpc.mountd)
[ "$status" -eq 0 ] || {
	echo "NFS start rejected rpc.mountd child after daemonization delay: $status" >&2
	exit 1
}
assert_command rpc.mountd
grep -q '^sleep ' "$COMMAND_LOG" || {
	echo "NFS mountd readiness wait did not poll after launcher success" >&2
	exit 1
}
[ -f "$STATE_DIR/rpc.mountd" ] || {
	echo "NFS start returned success before delayed rpc.mountd child appeared" >&2
	exit 1
}

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

# A noncritical syslog failure after both daemons are ready must not
# turn a successful NFS start into a reported service failure.
reset_fixture
status=$(run_command "" "" start "" 1)
[ "$status" -eq 0 ] || {
	echo "successful NFS start was reported as failed when logger returned nonzero: $status" >&2
	exit 1
}
[ -f "$STATE_DIR/nfsd" ] && [ -f "$STATE_DIR/rpc.mountd" ] || {
	echo "NFS daemon state was lost after noncritical logger failure" >&2
	exit 1
}
assert_command logger

# An unrelated filesystem at the reserved nfsd mountpoint must not be
# accepted as the control filesystem or removed during startup rollback.
reset_fixture
printf 'unrelated-source %s ext4 rw 0 0\n' "$tmp_dir/proc/fs/nfsd" > "$MOUNTS_FILE"
status=$(run_command "" "" start)
assert_failed "$status" "conflicting nfsd mount tuple"
assert_no_command mount
assert_no_command umount
grep -q "^unrelated-source $tmp_dir/proc/fs/nfsd ext4 " "$MOUNTS_FILE" || {
	echo "conflicting nfsd mount was not preserved" >&2
	exit 1
}

# A correct nfsd mount plus a conflicting rpc_pipefs mount must fail closed;
# rollback must not unmount either unrelated or stacked mount state.
reset_fixture
printf 'nfsd %s nfsd rw 0 0\n' "$tmp_dir/proc/fs/nfsd" > "$MOUNTS_FILE"
printf 'unrelated-source %s rpc_pipefs rw 0 0\n' "$tmp_dir/var/lib/nfs/rpc_pipefs" >> "$MOUNTS_FILE"
status=$(run_command "" "" start)
assert_failed "$status" "conflicting rpc_pipefs mount tuple"
assert_no_command mount
assert_no_command umount
grep -q "^unrelated-source $tmp_dir/var/lib/nfs/rpc_pipefs rpc_pipefs " "$MOUNTS_FILE" || {
	echo "conflicting rpc_pipefs mount was not preserved" >&2
	exit 1
}
grep -q "^nfsd $tmp_dir/proc/fs/nfsd nfsd " "$MOUNTS_FILE" || {
	echo "valid nfsd control mount was unexpectedly detached after rollback conflict" >&2
	exit 1
}

# Duplicate/stacked entries are ambiguous even when one row looks correct.
reset_fixture
printf 'nfsd %s nfsd rw 0 0\n' "$tmp_dir/proc/fs/nfsd" > "$MOUNTS_FILE"
printf 'nfsd %s nfsd ro 0 0\n' "$tmp_dir/proc/fs/nfsd" >> "$MOUNTS_FILE"
status=$(run_command "" "" start)
assert_failed "$status" "duplicate nfsd mountpoint entries"
assert_no_command mount
assert_no_command umount
[ "$(wc -l < "$MOUNTS_FILE" | tr -d ' ')" -eq 2 ] || {
	echo "ambiguous stacked nfsd mount entries were modified" >&2
	exit 1
}


# A live matching PID with an unreadable or malformed stat record is UNKNOWN.
# The stop path must take both complete identity snapshots before signalling.
reset_fixture
prepare_running_nfs
rm -f "$PROC_ROOT/123/stat"
mounts_before=$(cat "$MOUNTS_FILE")
status=$(run_command "" "" stop)
assert_failed "$status" "NFS stop with missing nfsd start-time identity"
assert_no_command kill
[ -f "$STATE_DIR/nfsd" ] && [ -f "$STATE_DIR/rpc.mountd" ] || {
	echo "NFS stop signalled a daemon after an incomplete start-time snapshot" >&2
	exit 1
}
[ "$(cat "$MOUNTS_FILE")" = "$mounts_before" ] || {
	echo "NFS stop changed mounts after an incomplete start-time snapshot" >&2
	exit 1
}

reset_fixture
prepare_running_nfs
printf '124 (rpc.mountd) S 1 2\n' > "$PROC_ROOT/124/stat"
status=$(run_command "" "" stop)
assert_failed "$status" "NFS stop with malformed rpc.mountd start-time identity"
assert_no_command kill
[ -f "$STATE_DIR/nfsd" ] && [ -f "$STATE_DIR/rpc.mountd" ] || {
	echo "NFS stop signalled a daemon with malformed proc stat" >&2
	exit 1
}

# Verify stop postconditions under signal, export-withdrawal and unmount failures.
# A failed signal with surviving daemons must not mutate exports or mount state.
reset_fixture
reset_fixture
prepare_running_nfs
status=$(run_command kill "" stop)
assert_failed "$status" "NFS stop with surviving daemons"
[ -f "$STATE_DIR/nfsd" ] && [ -f "$STATE_DIR/rpc.mountd" ] || {
	echo "NFS daemon state disappeared despite injected PID-bound signal failures" >&2
	exit 1
}
[ "$(cat "$tmp_dir/var/lib/nfs/etab")" = "recovery-state" ] || {
	echo "NFS stop discarded export recovery state while daemons survived" >&2
	exit 1
}
[ "$(wc -l < "$MOUNTS_FILE" | tr -d ' ')" -eq 3 ] || {
	echo "NFS stop detached pseudo-filesystems while daemons survived" >&2
	exit 1
}
assert_command kill
assert_no_command exportfs
assert_no_command umount
grep -q '^kill -TERM 124

# Failed rpc_pipefs detach must retain etab and must not continue to nfsd unmount.
reset_fixture
prepare_running_nfs
status=$(run_command umount "" stop)
assert_failed "$status" "NFS stop with rpc_pipefs unmount failure"
[ ! -f "$STATE_DIR/nfsd" ] && [ ! -f "$STATE_DIR/rpc.mountd" ] || {
	echo "NFS daemons remained after mocked successful termination" >&2
	exit 1
}
[ "$(cat "$tmp_dir/var/lib/nfs/etab")" = "recovery-state" ] || {
	echo "NFS stop discarded etab after unmount failure" >&2
	exit 1
}
[ "$(wc -l < "$MOUNTS_FILE" | tr -d ' ')" -eq 3 ] || {
	echo "NFS stop changed mount state after injected unmount failure" >&2
	exit 1
}
assert_command exportfs
assert_command umount
[ "$(grep -c '^umount ' "$COMMAND_LOG")" -eq 1 ] || {
	echo "NFS stop continued unmounting after the first unmount failure" >&2
	exit 1
}

# Failed export withdrawal must keep both pseudo-filesystems and etab in place.
reset_fixture
prepare_running_nfs
status=$(run_command exportfs "" stop)
assert_failed "$status" "NFS stop with export withdrawal failure"
[ ! -f "$STATE_DIR/nfsd" ] && [ ! -f "$STATE_DIR/rpc.mountd" ] || {
	echo "NFS daemons remained after successful mocked termination" >&2
	exit 1
}
[ -f "$tmp_dir/var/lib/nfs/etab" ] || {
	echo "NFS stop removed etab after export withdrawal failure" >&2
	exit 1
}
[ "$(wc -l < "$MOUNTS_FILE" | tr -d ' ')" -eq 3 ] || {
	echo "NFS stop detached pseudo-filesystems after export withdrawal failure" >&2
	exit 1
}
assert_no_command umount

# Full successful stop must remove both daemons, exports and exactly two mounts.
reset_fixture
prepare_running_nfs
status=$(run_command "" "" stop)
[ "$status" -eq 0 ] || {
	echo "successful NFS stop returned $status" >&2
	exit 1
}
[ ! -f "$STATE_DIR/nfsd" ] && [ ! -f "$STATE_DIR/rpc.mountd" ] || {
	echo "successful NFS stop left a server daemon running" >&2
	exit 1
}
! grep -Eq ' (nfsd|rpc_pipefs) ' "$MOUNTS_FILE" || {
	echo "successful NFS stop left pseudo-filesystems mounted" >&2
	exit 1
}
[ ! -f "$tmp_dir/var/lib/nfs/etab" ] || {
	echo "successful NFS stop retained etab after complete teardown" >&2
	exit 1
}
assert_command exportfs
[ "$(grep -c '^umount ' "$COMMAND_LOG")" -eq 2 ] || {
	echo "successful NFS stop did not unmount exactly two pseudo-filesystems" >&2
	exit 1
}

echo "NFS daemon start/reload/stop failure propagation and rollback: PASS"
 "$COMMAND_LOG"
grep -q '^kill -2 123

# Failed rpc_pipefs detach must retain etab and must not continue to nfsd unmount.
reset_fixture
prepare_running_nfs
status=$(run_command umount "" stop)
assert_failed "$status" "NFS stop with rpc_pipefs unmount failure"
[ ! -f "$STATE_DIR/nfsd" ] && [ ! -f "$STATE_DIR/rpc.mountd" ] || {
	echo "NFS daemons remained after mocked successful termination" >&2
	exit 1
}
[ "$(cat "$tmp_dir/var/lib/nfs/etab")" = "recovery-state" ] || {
	echo "NFS stop discarded etab after unmount failure" >&2
	exit 1
}
[ "$(wc -l < "$MOUNTS_FILE" | tr -d ' ')" -eq 3 ] || {
	echo "NFS stop changed mount state after injected unmount failure" >&2
	exit 1
}
assert_command exportfs
assert_command umount
[ "$(grep -c '^umount ' "$COMMAND_LOG")" -eq 1 ] || {
	echo "NFS stop continued unmounting after the first unmount failure" >&2
	exit 1
}

# Failed export withdrawal must keep both pseudo-filesystems and etab in place.
reset_fixture
prepare_running_nfs
status=$(run_command exportfs "" stop)
assert_failed "$status" "NFS stop with export withdrawal failure"
[ ! -f "$STATE_DIR/nfsd" ] && [ ! -f "$STATE_DIR/rpc.mountd" ] || {
	echo "NFS daemons remained after successful mocked termination" >&2
	exit 1
}
[ -f "$tmp_dir/var/lib/nfs/etab" ] || {
	echo "NFS stop removed etab after export withdrawal failure" >&2
	exit 1
}
[ "$(wc -l < "$MOUNTS_FILE" | tr -d ' ')" -eq 3 ] || {
	echo "NFS stop detached pseudo-filesystems after export withdrawal failure" >&2
	exit 1
}
assert_no_command umount

# Full successful stop must remove both daemons, exports and exactly two mounts.
reset_fixture
prepare_running_nfs
status=$(run_command "" "" stop)
[ "$status" -eq 0 ] || {
	echo "successful NFS stop returned $status" >&2
	exit 1
}
[ ! -f "$STATE_DIR/nfsd" ] && [ ! -f "$STATE_DIR/rpc.mountd" ] || {
	echo "successful NFS stop left a server daemon running" >&2
	exit 1
}
! grep -Eq ' (nfsd|rpc_pipefs) ' "$MOUNTS_FILE" || {
	echo "successful NFS stop left pseudo-filesystems mounted" >&2
	exit 1
}
[ ! -f "$tmp_dir/var/lib/nfs/etab" ] || {
	echo "successful NFS stop retained etab after complete teardown" >&2
	exit 1
}
assert_command exportfs
[ "$(grep -c '^umount ' "$COMMAND_LOG")" -eq 2 ] || {
	echo "successful NFS stop did not unmount exactly two pseudo-filesystems" >&2
	exit 1
}

echo "NFS daemon start/reload/stop failure propagation and rollback: PASS"
 "$COMMAND_LOG"
grep -q '^kill -9 124

# Failed rpc_pipefs detach must retain etab and must not continue to nfsd unmount.
reset_fixture
prepare_running_nfs
status=$(run_command umount "" stop)
assert_failed "$status" "NFS stop with rpc_pipefs unmount failure"
[ ! -f "$STATE_DIR/nfsd" ] && [ ! -f "$STATE_DIR/rpc.mountd" ] || {
	echo "NFS daemons remained after mocked successful termination" >&2
	exit 1
}
[ "$(cat "$tmp_dir/var/lib/nfs/etab")" = "recovery-state" ] || {
	echo "NFS stop discarded etab after unmount failure" >&2
	exit 1
}
[ "$(wc -l < "$MOUNTS_FILE" | tr -d ' ')" -eq 3 ] || {
	echo "NFS stop changed mount state after injected unmount failure" >&2
	exit 1
}
assert_command exportfs
assert_command umount
[ "$(grep -c '^umount ' "$COMMAND_LOG")" -eq 1 ] || {
	echo "NFS stop continued unmounting after the first unmount failure" >&2
	exit 1
}

# Failed export withdrawal must keep both pseudo-filesystems and etab in place.
reset_fixture
prepare_running_nfs
status=$(run_command exportfs "" stop)
assert_failed "$status" "NFS stop with export withdrawal failure"
[ ! -f "$STATE_DIR/nfsd" ] && [ ! -f "$STATE_DIR/rpc.mountd" ] || {
	echo "NFS daemons remained after successful mocked termination" >&2
	exit 1
}
[ -f "$tmp_dir/var/lib/nfs/etab" ] || {
	echo "NFS stop removed etab after export withdrawal failure" >&2
	exit 1
}
[ "$(wc -l < "$MOUNTS_FILE" | tr -d ' ')" -eq 3 ] || {
	echo "NFS stop detached pseudo-filesystems after export withdrawal failure" >&2
	exit 1
}
assert_no_command umount

# Full successful stop must remove both daemons, exports and exactly two mounts.
reset_fixture
prepare_running_nfs
status=$(run_command "" "" stop)
[ "$status" -eq 0 ] || {
	echo "successful NFS stop returned $status" >&2
	exit 1
}
[ ! -f "$STATE_DIR/nfsd" ] && [ ! -f "$STATE_DIR/rpc.mountd" ] || {
	echo "successful NFS stop left a server daemon running" >&2
	exit 1
}
! grep -Eq ' (nfsd|rpc_pipefs) ' "$MOUNTS_FILE" || {
	echo "successful NFS stop left pseudo-filesystems mounted" >&2
	exit 1
}
[ ! -f "$tmp_dir/var/lib/nfs/etab" ] || {
	echo "successful NFS stop retained etab after complete teardown" >&2
	exit 1
}
assert_command exportfs
[ "$(grep -c '^umount ' "$COMMAND_LOG")" -eq 2 ] || {
	echo "successful NFS stop did not unmount exactly two pseudo-filesystems" >&2
	exit 1
}

echo "NFS daemon start/reload/stop failure propagation and rollback: PASS"
 "$COMMAND_LOG"
grep -q '^kill -9 123

# Failed rpc_pipefs detach must retain etab and must not continue to nfsd unmount.
reset_fixture
prepare_running_nfs
status=$(run_command umount "" stop)
assert_failed "$status" "NFS stop with rpc_pipefs unmount failure"
[ ! -f "$STATE_DIR/nfsd" ] && [ ! -f "$STATE_DIR/rpc.mountd" ] || {
	echo "NFS daemons remained after mocked successful termination" >&2
	exit 1
}
[ "$(cat "$tmp_dir/var/lib/nfs/etab")" = "recovery-state" ] || {
	echo "NFS stop discarded etab after unmount failure" >&2
	exit 1
}
[ "$(wc -l < "$MOUNTS_FILE" | tr -d ' ')" -eq 3 ] || {
	echo "NFS stop changed mount state after injected unmount failure" >&2
	exit 1
}
assert_command exportfs
assert_command umount
[ "$(grep -c '^umount ' "$COMMAND_LOG")" -eq 1 ] || {
	echo "NFS stop continued unmounting after the first unmount failure" >&2
	exit 1
}

# Failed export withdrawal must keep both pseudo-filesystems and etab in place.
reset_fixture
prepare_running_nfs
status=$(run_command exportfs "" stop)
assert_failed "$status" "NFS stop with export withdrawal failure"
[ ! -f "$STATE_DIR/nfsd" ] && [ ! -f "$STATE_DIR/rpc.mountd" ] || {
	echo "NFS daemons remained after successful mocked termination" >&2
	exit 1
}
[ -f "$tmp_dir/var/lib/nfs/etab" ] || {
	echo "NFS stop removed etab after export withdrawal failure" >&2
	exit 1
}
[ "$(wc -l < "$MOUNTS_FILE" | tr -d ' ')" -eq 3 ] || {
	echo "NFS stop detached pseudo-filesystems after export withdrawal failure" >&2
	exit 1
}
assert_no_command umount

# Full successful stop must remove both daemons, exports and exactly two mounts.
reset_fixture
prepare_running_nfs
status=$(run_command "" "" stop)
[ "$status" -eq 0 ] || {
	echo "successful NFS stop returned $status" >&2
	exit 1
}
[ ! -f "$STATE_DIR/nfsd" ] && [ ! -f "$STATE_DIR/rpc.mountd" ] || {
	echo "successful NFS stop left a server daemon running" >&2
	exit 1
}
! grep -Eq ' (nfsd|rpc_pipefs) ' "$MOUNTS_FILE" || {
	echo "successful NFS stop left pseudo-filesystems mounted" >&2
	exit 1
}
[ ! -f "$tmp_dir/var/lib/nfs/etab" ] || {
	echo "successful NFS stop retained etab after complete teardown" >&2
	exit 1
}
assert_command exportfs
[ "$(grep -c '^umount ' "$COMMAND_LOG")" -eq 2 ] || {
	echo "successful NFS stop did not unmount exactly two pseudo-filesystems" >&2
	exit 1
}

echo "NFS daemon start/reload/stop failure propagation and rollback: PASS"
 "$COMMAND_LOG"
! grep -q '^killall ' "$COMMAND_LOG"

# Failed rpc_pipefs detach must retain etab and must not continue to nfsd unmount.
reset_fixture
prepare_running_nfs
status=$(run_command umount "" stop)
assert_failed "$status" "NFS stop with rpc_pipefs unmount failure"
[ ! -f "$STATE_DIR/nfsd" ] && [ ! -f "$STATE_DIR/rpc.mountd" ] || {
	echo "NFS daemons remained after mocked successful termination" >&2
	exit 1
}
[ "$(cat "$tmp_dir/var/lib/nfs/etab")" = "recovery-state" ] || {
	echo "NFS stop discarded etab after unmount failure" >&2
	exit 1
}
[ "$(wc -l < "$MOUNTS_FILE" | tr -d ' ')" -eq 3 ] || {
	echo "NFS stop changed mount state after injected unmount failure" >&2
	exit 1
}
assert_command exportfs
assert_command umount
[ "$(grep -c '^umount ' "$COMMAND_LOG")" -eq 1 ] || {
	echo "NFS stop continued unmounting after the first unmount failure" >&2
	exit 1
}

# Failed export withdrawal must keep both pseudo-filesystems and etab in place.
reset_fixture
prepare_running_nfs
status=$(run_command exportfs "" stop)
assert_failed "$status" "NFS stop with export withdrawal failure"
[ ! -f "$STATE_DIR/nfsd" ] && [ ! -f "$STATE_DIR/rpc.mountd" ] || {
	echo "NFS daemons remained after successful mocked termination" >&2
	exit 1
}
[ -f "$tmp_dir/var/lib/nfs/etab" ] || {
	echo "NFS stop removed etab after export withdrawal failure" >&2
	exit 1
}
[ "$(wc -l < "$MOUNTS_FILE" | tr -d ' ')" -eq 3 ] || {
	echo "NFS stop detached pseudo-filesystems after export withdrawal failure" >&2
	exit 1
}
assert_no_command umount

# Full successful stop must remove both daemons, exports and exactly two mounts.
reset_fixture
prepare_running_nfs
status=$(run_command "" "" stop)
[ "$status" -eq 0 ] || {
	echo "successful NFS stop returned $status" >&2
	exit 1
}
[ ! -f "$STATE_DIR/nfsd" ] && [ ! -f "$STATE_DIR/rpc.mountd" ] || {
	echo "successful NFS stop left a server daemon running" >&2
	exit 1
}
! grep -Eq ' (nfsd|rpc_pipefs) ' "$MOUNTS_FILE" || {
	echo "successful NFS stop left pseudo-filesystems mounted" >&2
	exit 1
}
[ ! -f "$tmp_dir/var/lib/nfs/etab" ] || {
	echo "successful NFS stop retained etab after complete teardown" >&2
	exit 1
}
assert_command exportfs
[ "$(grep -c '^umount ' "$COMMAND_LOG")" -eq 2 ] || {
	echo "successful NFS stop did not unmount exactly two pseudo-filesystems" >&2
	exit 1
}

echo "NFS daemon start/reload/stop failure propagation and rollback: PASS"
