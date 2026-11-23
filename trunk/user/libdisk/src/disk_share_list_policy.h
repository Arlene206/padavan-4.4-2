#ifndef PADAVAN_DISK_SHARE_LIST_POLICY_H
#define PADAVAN_DISK_SHARE_LIST_POLICY_H

#include <stddef.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <sys/wait.h>
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


/*
 * Apply multiple permission-file replacements as one compensating batch.
 * Original contents remain available until the enclosing directory operation
 * commits; rollback touches only files whose replacement actually succeeded.
 */
struct disk_share_permission_batch_entry {
	char path[256];
	char *before;
	size_t before_len;
	char *after;
	size_t after_len;
	int needs_update;
	int applied;
};

typedef int (*disk_share_permission_batch_writer)(
	const char *, const char *, size_t);

static inline int
disk_share_permission_batch_rollback(
	struct disk_share_permission_batch_entry *entries, int count,
	disk_share_permission_batch_writer write_file, int *rollback_error)
{
	int i, saved_error = 0;

	if (rollback_error)
		*rollback_error = 0;
	if (count < 0 || (count > 0 && !entries) ||
	    !write_file || !rollback_error) {
		errno = EINVAL;
		return -1;
	}

	for (i = count; i > 0; --i) {
		struct disk_share_permission_batch_entry *entry = &entries[i - 1];

		if (!entry->applied)
			continue;
		if (!entry->path[0] || !entry->before) {
			if (saved_error == 0)
				saved_error = EINVAL;
			continue;
		}
		errno = 0;
		if (write_file(entry->path, entry->before, entry->before_len) != 0) {
			if (saved_error == 0)
				saved_error = errno ? errno : EIO;
		} else {
			entry->applied = 0;
		}
	}

	*rollback_error = saved_error;
	if (saved_error != 0) {
		errno = saved_error;
		return -1;
	}
	return 0;
}

static inline int
disk_share_permission_batch_apply(
	struct disk_share_permission_batch_entry *entries, int count,
	disk_share_permission_batch_writer write_file, int *rollback_error)
{
	int i, operation_error;

	if (rollback_error)
		*rollback_error = 0;
	if (count < 0 || (count > 0 && !entries) ||
	    !write_file || !rollback_error) {
		errno = EINVAL;
		return -1;
	}

	for (i = 0; i < count; ++i) {
		struct disk_share_permission_batch_entry *entry = &entries[i];

		if (!entry->needs_update)
			continue;
		if (!entry->path[0] || !entry->before || !entry->after) {
			operation_error = EINVAL;
			goto compensate;
		}
		errno = 0;
		if (write_file(entry->path, entry->after, entry->after_len) != 0) {
			operation_error = errno ? errno : EIO;
			goto compensate;
		}
		entry->applied = 1;
	}
	return 0;

compensate:
	if (disk_share_permission_batch_rollback(
		    entries, i, write_file, rollback_error) != 0) {
		errno = EIO;
		return -1;
	}
	errno = operation_error;
	return -1;
}


/*
 * Permission metadata belongs to its directory. If rollback cannot remove a
 * newly-created directory (for example, another writer populated it), keep
 * its ACL rows instead of leaving a live share with missing permissions.
 */
static inline int
disk_share_folder_rollback_can_remove_permissions(int directory_remove_result,
                                                    int directory_remove_errno)
{
	return directory_remove_result == 0 || directory_remove_errno == ENOENT;
}


/*
 * Reserve a hidden staging directory with mkdir's exclusive creation
 * semantics. lstat()+rename() could replace a raced empty destination.
 */
typedef int (*disk_share_staging_directory_create_fn)(const char *);

static inline int
disk_share_reserve_staging_directory(
	const char *mount_path, char *staging_dir, size_t staging_size,
	long process_id, disk_share_staging_directory_create_fn create_directory)
{
	unsigned int attempt;
	int path_len, saved_errno;

	if (!mount_path || !*mount_path || !staging_dir || staging_size == 0 ||
	    !create_directory) {
		errno = EINVAL;
		return -1;
	}

	for (attempt = 0; attempt < 32; ++attempt) {
		path_len = snprintf(staging_dir, staging_size,
		                    "%s/.share-delete-%ld-%u",
		                    mount_path, process_id, attempt);
		if (path_len < 0 || (size_t)path_len >= staging_size) {
			errno = ENAMETOOLONG;
			return -1;
		}
		errno = 0;
		if (create_directory(staging_dir) == 0) {
			errno = 0;
			return 0;
		}
		saved_errno = errno ? errno : EIO;
		if (saved_errno != EEXIST) {
			errno = saved_errno;
			return -1;
		}
	}

	errno = EEXIST;
	return -1;
}


/* Missing source metadata is an idempotent no-op; preserve other rename errors. */
static inline int
disk_share_account_rename_status(int rename_result, int saved_errno)
{
	if (rename_result == 0 || saved_errno == ENOENT)
		return 0;
	return saved_errno != 0 ? saved_errno : EIO;
}

/*
 * Apply per-volume account metadata renames as one compensating batch.
 * Every path is validated by the caller before this helper is entered. If a
 * later rename fails, undo earlier moves in reverse order. rollback_error
 * distinguishes an ordinary rename failure from an incomplete compensation.
 */
struct disk_share_account_rename_entry {
	char source[256];
	char destination[256];
	int renamed;
	struct disk_share_account_rename_entry *previous;
	struct disk_share_account_rename_entry *next;
};

typedef int (*disk_share_rename_fn)(const char *, const char *);

static inline int
disk_share_account_rename_entries_apply(
	struct disk_share_account_rename_entry *entries,
	disk_share_rename_fn rename_path, int *rollback_error)
{
	struct disk_share_account_rename_entry *entry, *undo;
	int operation_errno = 0, saved_rollback_error = 0;

	if (rollback_error)
		*rollback_error = 0;
	if (!entries || !rename_path || !rollback_error) {
		errno = EINVAL;
		return -1;
	}
	for (entry = entries; entry; entry = entry->next) {
		entry->renamed = 0;
		errno = 0;
		if (rename_path(entry->source, entry->destination) != 0) {
			operation_errno = errno ? errno : EIO;
			for (undo = entry->previous; undo; undo = undo->previous) {
				if (!undo->renamed)
					continue;
				errno = 0;
				if (rename_path(undo->destination, undo->source) != 0) {
					if (saved_rollback_error == 0)
						saved_rollback_error = errno ? errno : EIO;
				} else {
					undo->renamed = 0;
				}
			}
			*rollback_error = saved_rollback_error;
			errno = operation_errno;
			return -1;
		}
		entry->renamed = 1;
	}
	return 0;
}

/* Convert system(3)'s wait status to a stable errno-style failure result. */
static inline int
disk_share_command_status(int wait_status, int saved_errno)
{
	if (wait_status == -1)
		return saved_errno != 0 ? saved_errno : EIO;
	if (!WIFEXITED(wait_status) || WEXITSTATUS(wait_status) != 0)
		return EIO;
	return 0;
}

/*
 * Undo a previously successful account metadata rename batch when the
 * corresponding NVRAM account update cannot be committed. Keep renamed set
 * on failure so the caller can report incomplete compensation.
 */
static inline int
disk_share_account_rename_entries_rollback(
	struct disk_share_account_rename_entry *entries,
	disk_share_rename_fn rename_path, int *rollback_error)
{
	struct disk_share_account_rename_entry *entry;
	int saved_error = 0;

	if (rollback_error)
		*rollback_error = 0;
	if (!entries || !rename_path || !rollback_error) {
		errno = EINVAL;
		return -1;
	}
	for (entry = entries; entry->next; entry = entry->next)
		;
	for (; entry; entry = entry->previous) {
		if (!entry->renamed)
			continue;
		errno = 0;
		if (rename_path(entry->destination, entry->source) != 0) {
			if (saved_error == 0)
				saved_error = errno ? errno : EIO;
		} else {
			entry->renamed = 0;
		}
	}
	*rollback_error = saved_error;
	if (saved_error != 0) {
		errno = saved_error;
		return -1;
	}
	return 0;
}

/* Roll back only metadata files created by the current add-account attempt. */
struct disk_share_account_created_file_entry {
	char path[256];
	char mount_path[256];
	int created;
	struct disk_share_account_created_file_entry *previous;
	struct disk_share_account_created_file_entry *next;
};

typedef int (*disk_share_unlink_fn)(const char *);

static inline int
disk_share_account_created_files_rollback(
	struct disk_share_account_created_file_entry *entries,
	disk_share_unlink_fn unlink_path, int *rollback_error)
{
	struct disk_share_account_created_file_entry *entry;
	int saved_error = 0;

	if (rollback_error)
		*rollback_error = 0;
	if (!entries || !unlink_path || !rollback_error) {
		errno = EINVAL;
		return -1;
	}
	for (entry = entries; entry->next; entry = entry->next)
		;
	for (; entry; entry = entry->previous) {
		if (!entry->created)
			continue;
		errno = 0;
		if (unlink_path(entry->path) != 0) {
			if (errno == ENOENT) {
				entry->created = 0;
				continue;
			}
			if (saved_error == 0)
				saved_error = errno ? errno : EIO;
		} else {
			entry->created = 0;
		}
	}
	*rollback_error = saved_error;
	if (saved_error != 0) {
		errno = saved_error;
		return -1;
	}
	return 0;
}

#endif /* PADAVAN_DISK_SHARE_LIST_POLICY_H */

