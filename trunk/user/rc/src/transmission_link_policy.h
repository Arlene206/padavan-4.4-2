#ifndef PADAVAN_TRANSMISSION_LINK_POLICY_H
#define PADAVAN_TRANSMISSION_LINK_POLICY_H

#include <errno.h>

static inline int
transmission_link_parent_status(int stat_status, int stat_error,
                                int is_directory, int is_symlink,
                                unsigned long owner, unsigned long mode)
{
	if (stat_status != 0)
		return stat_error > 0 ? stat_error : EIO;
	if (!is_directory || is_symlink || owner != 0 || (mode & 0022UL) != 0)
		return EPERM;
	return 0;
}

static inline int
transmission_link_destination_status(int lstat_status, int saved_error,
                                     int is_symlink)
{
	if (lstat_status == 0)
		return is_symlink ? 0 : EEXIST;
	if (saved_error == ENOENT)
		return 0;
	return saved_error > 0 ? saved_error : EIO;
}

#endif /* PADAVAN_TRANSMISSION_LINK_POLICY_H */
