#ifndef PADAVAN_NVRAM_DEVICE_NODE_POLICY_H
#define PADAVAN_NVRAM_DEVICE_NODE_POLICY_H

#include <sys/stat.h>
#include <sys/types.h>

#define NVRAM_DEVICE_NODE_PATH "/dev/nvram"
#define NVRAM_DEVICE_NODE_MAJOR 228
#define NVRAM_DEVICE_NODE_MINOR 0
#define NVRAM_DEVICE_NODE_MODE 0600

/*
 * The NVRAM ioctl interface can change persistent router configuration.
 * Only PID 1/root-owned code may access this device; shared libnvram callers
 * serialize through the separate root-private flock before issuing ioctls.
 */
static inline int
nvram_device_node_is_secure(const struct stat *st, dev_t expected_device)
{
	if (!st || !S_ISCHR(st->st_mode) || st->st_rdev != expected_device)
		return 0;
	if (st->st_uid != (uid_t)0 || st->st_gid != (gid_t)0)
		return 0;
	if (st->st_nlink != 1 ||
	    (st->st_mode & 07777) != NVRAM_DEVICE_NODE_MODE)
		return 0;
	return 1;
}

#endif /* PADAVAN_NVRAM_DEVICE_NODE_POLICY_H */
