#ifndef PADAVAN_STORAGE_MEDIA_LINK_POLICY_H
#define PADAVAN_STORAGE_MEDIA_LINK_POLICY_H

#include <errno.h>
#include <string.h>

/* Restrict the privileged publisher to fixed in-tree storage compatibility links. */
static inline int
storage_media_link_pair_status(const char *dest_dir, const char *link_path)
{
	if (!dest_dir || !link_path)
		return EINVAL;
	if ((strcmp(dest_dir, "transmission") == 0 &&
	     strcmp(link_path, "/mnt/transmission") == 0) ||
	    (strcmp(dest_dir, "aria") == 0 &&
	     strcmp(link_path, "/mnt/aria") == 0) ||
	    (strcmp(dest_dir, ".dms") == 0 &&
	     strcmp(link_path, "/mnt/minidlna") == 0) ||
	    (strcmp(dest_dir, ".itunes") == 0 &&
	     strcmp(link_path, "/mnt/firefly") == 0))
		return 0;
	return EINVAL;
}

static inline int
storage_media_link_parent_status(int stat_status, int stat_error,
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
storage_media_link_destination_status(int lstat_status, int saved_error,
                                       int is_symlink)
{
	if (lstat_status == 0)
		return is_symlink ? 0 : EEXIST;
	if (saved_error == ENOENT)
		return 0;
	return saved_error > 0 ? saved_error : EIO;
}

/* User-writable removable volumes must not redirect a root daemon through
 * a symlinked database/config directory. */
static inline int
storage_media_link_target_status(int lstat_status, int saved_error,
                                 int is_directory, int is_symlink)
{
	if (lstat_status != 0)
		return saved_error > 0 ? saved_error : EIO;
	if (is_symlink)
		return EPERM;
	if (!is_directory)
		return ENOTDIR;
	return 0;
}

#endif /* PADAVAN_STORAGE_MEDIA_LINK_POLICY_H */
