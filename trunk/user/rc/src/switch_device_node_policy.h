#ifndef PADAVAN_SWITCH_DEVICE_NODE_POLICY_H
#define PADAVAN_SWITCH_DEVICE_NODE_POLICY_H

#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

/*
 * Switch-control nodes are created by the privileged router daemon. Treat an
 * existing path as untrusted: mknod(EEXIST) is not proof that the path names
 * the expected character device. lstat prevents following a final symlink.
 */
static inline int
rc_switch_device_node_is_secure(const struct stat *st, mode_t expected_mode,
                                dev_t expected_device)
{
	if (st == 0)
		return 0;
	if ((expected_mode & S_IFMT) != S_IFCHR ||
	    (expected_mode & 07777) != 0600)
		return 0;
	if ((st->st_mode & (S_IFMT | 07777)) !=
	    (expected_mode & (S_IFMT | 07777)))
		return 0;
	if (st->st_uid != 0 || st->st_gid != 0 || st->st_nlink != 1)
		return 0;
	if (st->st_rdev != expected_device)
		return 0;
	return 1;
}

static inline int
rc_switch_device_node_ensure(const char *path, mode_t expected_mode,
                             dev_t expected_device)
{
	struct stat st;
	int saved_errno;

	if (path == 0 || path[0] != '/' ||
	    (expected_mode & S_IFMT) != S_IFCHR ||
	    (expected_mode & 07777) != 0600)
		return -EINVAL;

	if (mknod(path, expected_mode, expected_device) != 0 && errno != EEXIST) {
		saved_errno = errno;
		return -(saved_errno != 0 ? saved_errno : EIO);
	}
	if (lstat(path, &st) != 0) {
		saved_errno = errno;
		return -(saved_errno != 0 ? saved_errno : EIO);
	}
	if (!rc_switch_device_node_is_secure(&st, expected_mode, expected_device))
		return -EPERM;
	return 0;
}

#endif /* PADAVAN_SWITCH_DEVICE_NODE_POLICY_H */
