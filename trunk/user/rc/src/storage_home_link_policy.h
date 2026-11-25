#ifndef PADAVAN_STORAGE_HOME_LINK_POLICY_H
#define PADAVAN_STORAGE_HOME_LINK_POLICY_H

#include <errno.h>

static inline int
storage_home_link_restore_status(int already_directory,
				 int unlink_status, int unlink_error,
				 int symlink_status, int symlink_error)
{
	if (already_directory)
		return 0;
	if (unlink_status != 0 && unlink_error != ENOENT)
		return unlink_error > 0 ? unlink_error : EIO;
	if (symlink_status != 0)
		return symlink_error > 0 ? symlink_error : EIO;
	return 0;
}

#endif /* PADAVAN_STORAGE_HOME_LINK_POLICY_H */
