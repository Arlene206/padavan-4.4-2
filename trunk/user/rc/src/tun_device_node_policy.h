#ifndef PADAVAN_TUN_DEVICE_NODE_POLICY_H
#define PADAVAN_TUN_DEVICE_NODE_POLICY_H

#include <sys/stat.h>
#include <sys/types.h>

#define TUN_DEVICE_NODE_PATH "/dev/net/tun"
#define TUN_DEVICE_NODE_MAJOR 10
#define TUN_DEVICE_NODE_MINOR 200
#define TUN_DEVICE_NODE_MODE 0666

/*
 * OpenVPN server mode drops to nobody:nogroup. Preserve root's full
 * directory access, but keep the path searchable by both group and other
 * users; never allow non-root writes or special directory bits.
 */
static inline int
tun_device_directory_is_secure(const struct stat *st)
{
	mode_t mode;

	if (!st || !S_ISDIR(st->st_mode) ||
	    st->st_uid != (uid_t)0 || st->st_gid != (gid_t)0)
		return 0;

	mode = st->st_mode & 07777;
	if ((mode & 0700) != 0700 || (mode & 0011) != 0011 ||
	    (mode & 0022) != 0 || (mode & 07000) != 0)
		return 0;

	return 1;
}

/*
 * OpenVPN's TUN endpoint must be the kernel character device, not a regular
 * file, symlink target, or another device accidentally occupying the path.
 * Accept stricter read/write permissions, but never execute or special bits.
 */
static inline int
tun_device_node_is_secure(const struct stat *st, dev_t expected_device)
{
	mode_t mode;

	if (!st || !S_ISCHR(st->st_mode) || st->st_rdev != expected_device)
		return 0;
	if (st->st_uid != (uid_t)0 || st->st_gid != (gid_t)0 ||
	    st->st_nlink != 1)
		return 0;

	mode = st->st_mode & 07777;
	if ((mode & 0600) != 0600 || (mode & 0111) != 0 ||
	    (mode & 07000) != 0)
		return 0;

	return 1;
}

#endif /* PADAVAN_TUN_DEVICE_NODE_POLICY_H */
