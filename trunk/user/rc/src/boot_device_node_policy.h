#ifndef PADAVAN_RC_BOOT_DEVICE_NODE_POLICY_H
#define PADAVAN_RC_BOOT_DEVICE_NODE_POLICY_H

#include <sys/stat.h>
#include <sys/types.h>

#define RC_BOOT_DEVICE_NODE_MODE 0666

/*
 * Optional hardware accelerator nodes must never resolve to an unrelated
 * device, regular file, or symlink. Keep root ownership and a single link;
 * accept stricter read/write permissions, but not execute or special bits.
 */
static inline int
rc_boot_device_node_is_secure(const struct stat *st, dev_t expected_device)
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

#endif /* PADAVAN_RC_BOOT_DEVICE_NODE_POLICY_H */
