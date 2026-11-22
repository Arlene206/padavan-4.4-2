#ifndef PADAVAN_DISK_SHARE_LIST_POLICY_H
#define PADAVAN_DISK_SHARE_LIST_POLICY_H

#include <stddef.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>

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


/*
 * Flush a complete share-list snapshot and report every stream failure. The
 * caller writes to a temporary file and publishes it only after fsync/close.
 */
static inline int
disk_share_list_write_snapshot(FILE *stream, int folder_count,
                               char *const *folder_list)
{
	int i;

	if (!stream || folder_count < 0 ||
	    (folder_count > 0 && !folder_list)) {
		errno = EINVAL;
		return -1;
	}

	if (fprintf(stream, "sh_num=%d\n", folder_count) < 0)
		return -1;
	for (i = 0; i < folder_count; ++i) {
		if (!folder_list[i]) {
			errno = EINVAL;
			return -1;
		}
		if (fprintf(stream, "sh_name%d=%s\n", i, folder_list[i]) < 0)
			return -1;
	}
	if (fflush(stream) != 0)
		return -1;

	return 0;
}

/*
 * Write all default permission records and surface buffered I/O errors.
 * The caller publishes this content only after fsync and successful close.
 */
static inline int
disk_share_write_account_defaults(FILE *stream, int folder_count,
                                  char *const *folder_list,
                                  int samba_right, int ftp_right,
                                  int dms_right)
{
	int i;

	if (!stream || folder_count < 0 ||
	    (folder_count > 0 && !folder_list)) {
		errno = EINVAL;
		return -1;
	}

	for (i = 0; i < folder_count; ++i) {
		if (!folder_list[i]) {
			errno = EINVAL;
			return -1;
		}
		if (fprintf(stream, "*%s=%d%d%d\n", folder_list[i],
		             samba_right, ftp_right, dms_right) < 0)
			return -1;
	}
	if (fflush(stream) != 0)
		return -1;

	return 0;
}

#endif /* PADAVAN_DISK_SHARE_LIST_POLICY_H */

