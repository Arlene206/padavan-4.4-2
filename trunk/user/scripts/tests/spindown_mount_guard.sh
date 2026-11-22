#!/bin/sh
set -eu

script=${1:-trunk/user/scripts/files/sbin/spindown.sh}
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' 0 HUP INT TERM

sed -n '/^spindown_mount_status() {$/,/^# Refuse to spin down/{ /^# Refuse to spin down/d; p; }' "$script" > "$tmp/mount-function.sh"
[ -s "$tmp/mount-function.sh" ] || {
	echo "could not extract spindown_mount_status" >&2
	exit 1
}
. "$tmp/mount-function.sh"

cat > "$tmp/mounts" <<'EOF'
/dev/sda /mnt/usb ext4 rw 0 0
/dev/sda1 /mnt/usb/data ext4 rw 0 0
/dev/sda1p2 /mnt/usb/odd ext4 rw 0 0
EOF
spindown_mount_status sda "$tmp/mounts" && status=0 || status=$?
[ "$status" -eq 0 ]

spindown_mount_status sda1 "$tmp/mounts" && status=0 || status=$?
[ "$status" -eq 0 ]

cat > "$tmp/mounts" <<'EOF'
/dev/sdaa /mnt/other ext4 rw 0 0
/dev/sdafoo /mnt/not-a-partition ext4 rw 0 0
EOF
spindown_mount_status sda "$tmp/mounts" && status=0 || status=$?
[ "$status" -eq 1 ]

# A malformed/truncated record makes the inventory UNKNOWN, even if no
# row names the target disk. It must not be accepted as proof of absence.
cat > "$tmp/mounts" <<'EOF'
/dev/sdb1 /mnt/other ext4 rw 0 0
/dev/sdc1 /mnt/truncated ext4 rw
EOF
spindown_mount_status sda "$tmp/mounts" && status=0 || status=$?
[ "$status" -eq 2 ]

# A successfully read but empty mount inventory is incomplete, not proof
# that no filesystems are mounted.
: > "$tmp/empty"
spindown_mount_status sda "$tmp/empty" && status=0 || status=$?
[ "$status" -eq 2 ]

# A missing/unreadable mount table is not evidence that the disk is unused.
spindown_mount_status sda "$tmp/missing" && status=0 || status=$?
[ "$status" -eq 2 ]
mkdir "$tmp/not-a-file"
spindown_mount_status sda "$tmp/not-a-file" && status=0 || status=$?
[ "$status" -eq 2 ]

grep -Fq 'mount_status=$?' "$script"
grep -Fq '*) exit 1 ;; # cannot prove the disk is unused' "$script"
echo "spindown mount-table fail-closed guard: PASS"

# Exercise the real entry point against a disposable mount table. The script
# must share automount.sh's lock across both its inventory check and hdparm.
real_flock=$(command -v flock) || {
	echo "flock is required for the mount/spindown serialization regression" >&2
	exit 1
}
mkdir -p "$tmp/bin" "$tmp/sys/block/sda"
printf '0\n' > "$tmp/sys/block/sda/removable"
cat > "$tmp/bin/hdparm" <<'EOF_HDPARM'
#!/bin/sh
printf '%s\n' "$*" >> "$HDPARM_LOG"
EOF_HDPARM
cat > "$tmp/bin/flock" <<'EOF_FLOCK'
#!/bin/sh
: > "$FLOCK_ATTEMPT_MARKER"
exec "$FLOCK_REAL" "$@"
EOF_FLOCK
chmod 755 "$tmp/bin/hdparm" "$tmp/bin/flock"
sed \
	-e "s|/sbin/hdparm|$tmp/bin/hdparm|g" \
	-e "s|/proc/mounts|$tmp/mounts|g" \
	-e "s|/sys/block|$tmp/sys/block|g" \
	-e "s|/var/lock/automount_mountpoint.lock|$tmp/mountpoint.lock|g" \
	"$script" > "$tmp/spindown.sh"
chmod 755 "$tmp/spindown.sh"
# A complete non-empty mount table with only another disk proves sda is unused.
printf '/dev/sdb1 /mnt/other ext4 rw 0 0\\n' > "$tmp/mounts"
: > "$tmp/hdparm.log"
env PATH="$tmp/bin:$PATH" FLOCK_REAL="$real_flock" \
	FLOCK_ATTEMPT_MARKER="$tmp/flock-attempted" HDPARM_LOG="$tmp/hdparm.log" \
	sh "$tmp/spindown.sh" sda
grep -Fqx -- '-y /dev/sda' "$tmp/hdparm.log"

# Hold the same lock automount holds, start spindown while blocked, then add
# a mount record before releasing the lock. A stale pre-lock snapshot would
# incorrectly call hdparm; a correct implementation sees the new mount.
: > "$tmp/hdparm.log"
: > "$tmp/mounts"
rm -f "$tmp/holder-ready" "$tmp/release-holder" "$tmp/flock-attempted"
(
	exec 8>>"$tmp/mountpoint.lock"
	"$real_flock" 8
	: > "$tmp/holder-ready"
	while [ ! -e "$tmp/release-holder" ]; do sleep 0.01; done
) &
holder_pid=$!
attempts=0
while [ ! -e "$tmp/holder-ready" ] && [ "$attempts" -lt 200 ]; do
	sleep 0.01
	attempts=$((attempts + 1))
done
[ -e "$tmp/holder-ready" ] || {
	echo "could not acquire automount lock in regression fixture" >&2
	touch "$tmp/release-holder"
	wait "$holder_pid" || :
	exit 1
}
env PATH="$tmp/bin:$PATH" FLOCK_REAL="$real_flock" \
	FLOCK_ATTEMPT_MARKER="$tmp/flock-attempted" HDPARM_LOG="$tmp/hdparm.log" \
	sh "$tmp/spindown.sh" sda > "$tmp/spindown.out" 2>&1 &
spindown_pid=$!
attempts=0
while [ ! -e "$tmp/flock-attempted" ] && [ "$attempts" -lt 200 ]; do
	sleep 0.01
	attempts=$((attempts + 1))
done
[ -e "$tmp/flock-attempted" ] || {
	echo "spindown did not attempt the shared automount lock" >&2
	touch "$tmp/release-holder"
	wait "$holder_pid" || :
	wait "$spindown_pid" || :
	exit 1
}
sleep 0.05
[ ! -s "$tmp/hdparm.log" ] || {
	echo "spindown ran hdparm before acquiring the mount lock" >&2
	touch "$tmp/release-holder"
	wait "$holder_pid" || :
	wait "$spindown_pid" || :
	exit 1
}
printf '/dev/sda1 /mnt/usb ext4 rw 0 0\n' > "$tmp/mounts"
: > "$tmp/release-holder"
wait "$holder_pid"
if wait "$spindown_pid"; then
	echo "spindown accepted a disk mounted while it waited for the lock" >&2
	exit 1
fi
[ ! -s "$tmp/hdparm.log" ] || {
	echo "spindown issued hdparm after a concurrent mount" >&2
	exit 1
}
echo "spindown and automount shared-lock race regression: PASS"
