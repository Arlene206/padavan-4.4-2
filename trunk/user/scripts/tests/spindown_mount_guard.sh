#!/bin/sh
set -eu

script=\${1:-trunk/user/scripts/files/sbin/spindown.sh}
tmp=$(mktemp -d)
trap 'rm -rf "$tmp"' 0 HUP INT TERM

sed -n '/^spindown_mount_status() {$/,/^}$/p' "$script" > "$tmp/mount-function.sh"
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

# A missing/unreadable mount table is not evidence that the disk is unused.
spindown_mount_status sda "$tmp/missing" && status=0 || status=$?
[ "$status" -eq 2 ]
mkdir "$tmp/not-a-file"
spindown_mount_status sda "$tmp/not-a-file" && status=0 || status=$?
[ "$status" -eq 2 ]

grep -Fq 'mount_status=$?' "$script"
grep -Fq '*) exit 1 ;; # cannot prove the disk is unused' "$script"
echo "spindown mount-table fail-closed guard: PASS"
