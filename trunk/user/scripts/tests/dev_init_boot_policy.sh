#!/bin/sh
set -eu

repo_root=$(CDPATH= cd -- "$(dirname "$0")/../../../.." && pwd)
policy=$(sed -n '/^required_mount()/,/^}/p; /^required_dir()/,/^}/p' \
	"$repo_root/trunk/user/scripts/files/sbin/dev_init.sh")
[ -n "$policy" ] || {
	echo "could not extract dev_init required setup helpers" >&2
	exit 1
}
eval "$policy"

mount_calls=0
mount_status=0
mount()
{
	mount_calls=$((mount_calls + 1))
	return "$mount_status"
}

mkdir_calls=0
mkdir_status=0
mkdir()
{
	mkdir_calls=$((mkdir_calls + 1))
	[ "$1" = "-p" ] || return 90
	return "$mkdir_status"
}

chmod_calls=0
chmod_status=0
chmod()
{
	chmod_calls=$((chmod_calls + 1))
	return "$chmod_status"
}

required_mount -t proc proc /proc
[ "$mount_calls" -eq 1 ] || exit 2
mount_status=1
if required_mount -t sysfs sysfs /sys; then
	echo "required_mount accepted a failed mount" >&2
	exit 3
fi
[ "$mount_calls" -eq 2 ] || exit 4

required_dir 700 /tmp/rc_notification
[ "$mkdir_calls" -eq 1 ] && [ "$chmod_calls" -eq 1 ] || exit 5
mkdir_status=1
if required_dir 700 /tmp/rc_action_incomplete; then
	echo "required_dir accepted failed mkdir" >&2
	exit 6
fi
[ "$chmod_calls" -eq 1 ] || exit 7

mkdir_status=0
chmod_status=1
if required_dir 700 /tmp/.httpd-private; then
	echo "required_dir accepted failed chmod" >&2
	exit 8
fi
[ "$mkdir_calls" -eq 3 ] && [ "$chmod_calls" -eq 2 ] || exit 9

echo "dev_init required mount/directory failure policy: PASS"
