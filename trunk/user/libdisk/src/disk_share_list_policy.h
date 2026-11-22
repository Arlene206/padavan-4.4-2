#ifndef PADAVAN_DISK_SHARE_LIST_POLICY_H
#define PADAVAN_DISK_SHARE_LIST_POLICY_H

#include <stddef.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

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

/* Build a bounded per-account metadata pathname without accepting truncation. */
static inline int
disk_share_format_account_var_path(char *buffer, size_t buffer_size,
                                   const char *mount_path,
                                   const char *account)
{
	int path_len;

	if (!buffer || buffer_size == 0 || !mount_path || !*mount_path ||
	    !account || !*account) {
		errno = EINVAL;
		return -1;
	}

	errno = 0;
	path_len = snprintf(buffer, buffer_size, "%s/.__%s_var.txt",
	                    mount_path, account);
	if (path_len < 0) {
		if (errno == 0)
			errno = EIO;
		return -1;
	}
	if ((size_t)path_len >= buffer_size) {
		errno = ENAMETOOLONG;
		return -1;
	}
	return 0;
}


/* Rewrite one complete permission record without modifying the input buffer.
 * A NULL replacement removes the record; otherwise it renames only its key. */
static inline int
disk_share_permission_rewrite(const char *source, const char *target,
                              const char *replacement, char **updated,
                              size_t *updated_len)
{
	size_t source_len, target_len, prefix_len, replacement_len, suffix_len;
	size_t offset, i, result_len;
	const char *match = NULL, *after;
	char *result;

	if (updated)
		*updated = NULL;
	if (updated_len)
		*updated_len = 0;
	if (!source || !target || !*target || !updated || !updated_len) {
		errno = EINVAL;
		return -1;
	}

	source_len = strlen(source);
	target_len = strlen(target);
	if (target_len > source_len) {
		errno = ENOENT;
		return -1;
	}

	for (offset = 0; offset <= source_len - target_len; ++offset) {
		if (offset != 0 && source[offset - 1] != '\n')
			continue;
		for (i = 0; i < target_len; ++i) {
			if (tolower((unsigned char)source[offset + i]) !=
			    tolower((unsigned char)target[i]))
				break;
		}
		if (i == target_len) {
			match = source + offset;
			break;
		}
	}
	if (!match) {
		errno = ENOENT;
		return -1;
	}

	prefix_len = (size_t)(match - source);
	if (replacement) {
		after = match + target_len;
		replacement_len = strlen(replacement);
	} else {
		after = (const char *)memchr(match, '\n', source_len - prefix_len);
		if (after)
			++after;
		else
			after = source + source_len;
		replacement_len = 0;
	}
	suffix_len = source_len - (size_t)(after - source);
	if (prefix_len > (size_t)-1 - replacement_len ||
	    prefix_len + replacement_len > (size_t)-1 - suffix_len ||
	    prefix_len + replacement_len + suffix_len == (size_t)-1) {
		errno = EOVERFLOW;
		return -1;
	}
	result_len = prefix_len + replacement_len + suffix_len;
	result = (char *)malloc(result_len + 1);
	if (!result)
		return -1;

	if (prefix_len)
		memcpy(result, source, prefix_len);
	if (replacement_len)
		memcpy(result + prefix_len, replacement, replacement_len);
	if (suffix_len)
		memcpy(result + prefix_len + replacement_len, after, suffix_len);
	result[result_len] = '\0';

	*updated = result;
	*updated_len = result_len;
	return 0;
}


/* Missing source metadata is an idempotent no-op; preserve other rename errors. */
static inline int
disk_share_account_rename_status(int rename_result, int saved_errno)
{
	if (rename_result == 0 || saved_errno == ENOENT)
		return 0;
	return saved_errno != 0 ? saved_errno : EIO;
}

#endif /* PADAVAN_DISK_SHARE_LIST_POLICY_H */

