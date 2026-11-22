#ifndef PADAVAN_DISK_SHARE_LIST_POLICY_H
#define PADAVAN_DISK_SHARE_LIST_POLICY_H

#include <stddef.h>
#include <dirent.h>
#include <errno.h>

/*
 * Reset caller-owned outputs before rejecting invalid inputs. This keeps
 * error cleanup safe in callers that always release the returned list.
 */
static inline int
disk_share_list_prepare_outputs(const char *mount_path, int *sh_num,
                                char ***folder_list)
{
	if (sh_num)
		*sh_num = 0;
	if (folder_list)
		*folder_list = NULL;

	if (!mount_path || !*mount_path || !sh_num || !folder_list)
		return -1;

	return 0;
}

/*
 * Distinguish end-of-directory from a failed readdir(). POSIX specifies
 * errno is meaningful on NULL only when it was cleared before the call.
 * Return 1 for an entry, 0 for clean EOF, and -1 for an I/O failure.
 */
typedef struct dirent *(*disk_share_readdir_fn)(DIR *);

static inline int
disk_share_list_next_entry(DIR *directory, disk_share_readdir_fn read_entry,
                           struct dirent **entry, int *read_error)
{
	if (entry)
		*entry = NULL;
	if (read_error)
		*read_error = EINVAL;
	if (!read_entry || !entry || !read_error) {
		errno = EINVAL;
		return -1;
	}

	errno = 0;
	*entry = read_entry(directory);
	*read_error = errno;
	if (*entry) {
		*read_error = 0;
		return 1;
	}

	return *read_error != 0 ? -1 : 0;
}

#endif /* PADAVAN_DISK_SHARE_LIST_POLICY_H */

