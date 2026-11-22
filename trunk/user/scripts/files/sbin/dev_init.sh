#!/bin/sh

required_mount()
{
	if ! mount "$@"; then
		echo "dev_init: required mount failed: $*" >&2
		return 1
	fi
}

required_dir()
{
	dir_mode="$1"
	shift
	if ! mkdir -p "$@"; then
		echo "dev_init: required directory creation failed: $*" >&2
		return 1
	fi
	if ! chmod "$dir_mode" "$@"; then
		echo "dev_init: required directory permission setup failed: $*" >&2
		return 1
	fi
	return 0
}

required_lock_file()
{
	lock_path=$1
	lock_old_umask=$(umask)
	umask 077
	# noclobber makes creation exclusive; a pre-existing file or symlink is rejected.
	if ! ( set -C; : > "$lock_path" ); then
		umask "$lock_old_umask"
		echo "dev_init: cannot exclusively create storage lock $lock_path" >&2
		return 1
	fi
	umask "$lock_old_umask"
	if [ -L "$lock_path" ] || [ ! -f "$lock_path" ]; then
		echo "dev_init: storage lock is not a regular file: $lock_path" >&2
		return 1
	fi
	return 0
}

required_mount -t proc proc /proc || exit 1
required_mount -t sysfs sysfs /sys || exit 1
[ -d /proc/bus/usb ] && mount -t usbfs usbfs /proc/bus/usb

size_tmp="24M"
size_var="4M"
size_etc="6M"

if [ "$1" == "-l" ] ; then
	size_tmp="8M"
	size_var="1M"
fi

required_mount -t tmpfs tmpfs /dev   -o size=8K || exit 1
required_mount -t tmpfs tmpfs /etc   -o size=$size_etc,noatime || exit 1
mount -t tmpfs tmpfs /home  -o size=1M
mount -t tmpfs tmpfs /media -o size=8K
mount -t tmpfs tmpfs /mnt   -o size=8K
required_mount -t tmpfs tmpfs /tmp   -o size=$size_tmp || exit 1
required_dir 1777 /tmp || exit 1
# mdev may start disk hotplug workers. Pre-create a root-owned lock in
# sticky /tmp before scanning devices; callers must never create lock paths.
required_lock_file /tmp/automount_mountpoint.lock || exit 1
required_lock_file /tmp/automount_fs_module.lock || exit 1
required_lock_file /tmp/aliyundrive-webdav.lock || exit 1
required_lock_file /tmp/optware-lifecycle.lock || exit 1

# PID 1 notification queues carry privileged service actions.
required_dir 700 /tmp/rc_notification || exit 1
required_dir 700 /tmp/rc_action_incomplete || exit 1

required_mount -t tmpfs tmpfs /var   -o size=$size_var || exit 1

required_dir 755 /dev/pts || exit 1
required_mount -t devpts devpts /dev/pts || exit 1

mount -t debugfs debugfs /sys/kernel/debug

ln -sf /etc_ro/mdev.conf /etc/mdev.conf
mdev -s || exit 1

# create dirs
mkdir -p -m 777 /var/lock
mkdir -p -m 777 /var/locks
mkdir -p -m 777 /var/private
mkdir -p -m 700 /var/empty
mkdir -p -m 777 /var/lib
mkdir -p -m 777 /var/log
mkdir -p -m 777 /var/run
mkdir -p -m 777 /var/tmp
mkdir -p -m 777 /var/spool
mkdir -p -m 777 /var/lib/misc
mkdir -p -m 777 /var/state
mkdir -p -m 777 /var/state/parport
mkdir -p -m 777 /var/state/parport/svr_statue
mkdir -p -m 777 /tmp/var
mkdir -p -m 777 /tmp/hashes
mkdir -p -m 777 /tmp/modem
required_dir 700 /tmp/.httpd-private || exit 1
mkdir -p -m 700 /home/root
mkdir -p -m 700 /home/root/.ssh
mkdir -p -m 755 /etc/storage
mkdir -p -m 755 /etc/ssl
mkdir -p -m 755 /etc/Wireless
mkdir -p -m 750 /etc/Wireless/RT2860
mkdir -p -m 750 /etc/Wireless/iNIC

# mount cgroupfs if kernel provides cgroups
if [ -e /proc/cgroups ] && [ -d /sys/fs/cgroup ]; then
	if ! mountpoint -q /sys/fs/cgroup; then
		mount -t tmpfs -o uid=0,gid=0,mode=0755 cgroup /sys/fs/cgroup
	fi
	(cd /sys/fs/cgroup
	for sys in $(awk '!/^#/ { if ($4 == 1) print $1 }' /proc/cgroups); do
		mkdir -p $sys
		if ! mountpoint -q $sys; then
			if ! mount -n -t cgroup -o $sys cgroup $sys; then
				rmdir $sys || true
			fi
		fi
	done)
	if [ -e /sys/fs/cgroup/memory/memory.use_hierarchy ]; then
		echo 1 > /sys/fs/cgroup/memory/memory.use_hierarchy
	fi
fi

# extract storage files
mtd_storage.sh load

touch /etc/resolv.conf

if [ -f /etc_ro/openssl.cnf ]; then
	cp -f /etc_ro/openssl.cnf /etc/ssl
fi

# create symlinks
ln -sf /home/root /home/admin
ln -sf /proc/mounts /etc/mtab
ln -sf /etc_ro/ethertypes /etc/ethertypes
ln -sf /etc_ro/protocols /etc/protocols
ln -sf /etc_ro/services /etc/services
ln -sf /etc_ro/shells /etc/shells
ln -sf /etc_ro/profile /etc/profile
ln -sf /etc_ro/e2fsck.conf /etc/e2fsck.conf
ln -sf /etc_ro/ipkg.conf /etc/ipkg.conf

# tune linux kernel
echo 65536        > /proc/sys/fs/file-max
echo "1024 65535" > /proc/sys/net/ipv4/ip_local_port_range

# fill storage
mtd_storage.sh fill

# prepare ssh authorized_keys
if [ -f /etc/storage/authorized_keys ] ; then
	cp -f /etc/storage/authorized_keys /home/root/.ssh
	chmod 600 /home/root/.ssh/authorized_keys
fi

# setup htop default color
if [ -f /usr/bin/htop ]; then
	mkdir -p /home/root/.config/htop
	echo "color_scheme=6" > /home/root/.config/htop/htoprc
fi

# perform start script (best effort; do not mask required setup failures)
if [ -x /etc/storage/start_script.sh ] ; then
	/etc/storage/start_script.sh
fi

exit 0

