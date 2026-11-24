#ifndef PADAVAN_DISK_SHARE_LIST_POLICY_H
#define PADAVAN_DISK_SHARE_LIST_POLICY_H

#include <stddef.h>
#include <limits.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/*
 * Remove one leaf from the directory descriptor used for enumeration. Never
 * rebuild a pathname after readdir(): the mountpoint may have been renamed or
 * replaced while the scan is in progress.
 */
static inline int
disk_share_unlink_stale_metadata_entry(int directory_fd,
                                       const char *entry_name)
{
	if (directory_fd < 0 || !entry_name || !*entry_name ||
	    strchr(entry_name, '/') || !strcmp(entry_name, ".") ||
	    !strcmp(entry_name, "..")) {
		errno = EINVAL;
		return -1;
	}
	return unlinkat(directory_fd, entry_name, 0);
}


/*
 * Publish a stream to a regular metadata file relative to a pinned directory.
 * The temporary file is private and unique, and both creation and rename are
 * descriptor-relative so a mountpoint path swap cannot redirect publication.
 */
typedef int (*disk_share_atomic_stream_writer_fn)(FILE *, void *);

static inline int
disk_share_write_stream_atomic_at(int directory_fd, const char *leaf,
                                  disk_share_atomic_stream_writer_fn writer,
                                  void *context, int preserve_existing_mode)
{
	struct stat existing_stat;
	char temp_leaf[NAME_MAX + 1];
	FILE *stream = NULL;
	unsigned long attempt;
	int fd = -1, temp_created = 0, have_existing = 0, saved_errno;

	if (directory_fd < 0 || !leaf || !*leaf ||
	    strchr(leaf, '/') || !strcmp(leaf, ".") || !strcmp(leaf, "..") ||
	    !writer || (preserve_existing_mode != 0 &&
	                preserve_existing_mode != 1)) {
		errno = EINVAL;
		return -1;
	}

	if (fstatat(directory_fd, leaf, &existing_stat,
	            AT_SYMLINK_NOFOLLOW) == 0) {
		if (!S_ISREG(existing_stat.st_mode)) {
			errno = EINVAL;
			return -1;
		}
		have_existing = 1;
	} else if (errno != ENOENT) {
		return -1;
	}

	for (attempt = 0; attempt < 128; ++attempt) {
		int name_length;

		name_length = snprintf(temp_leaf, sizeof(temp_leaf),
		                       ".%s.tmp.%ld.%lu", leaf,
		                       (long)getpid(), attempt);
		if (name_length < 0 || (size_t)name_length >= sizeof(temp_leaf)) {
			errno = ENAMETOOLONG;
			return -1;
		}
		fd = openat(directory_fd, temp_leaf,
		            O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC,
		            0600);
		if (fd >= 0) {
			temp_created = 1;
			break;
		}
		if (errno != EEXIST)
			return -1;
	}
	if (fd < 0) {
		errno = EEXIST;
		return -1;
	}

	if (preserve_existing_mode && have_existing &&
	    fchmod(fd, existing_stat.st_mode & 0777) != 0)
		goto failed;

	stream = fdopen(fd, "w");
	if (!stream)
		goto failed;
	fd = -1;

	/* A successful fstatat() need not clear errno; don't leak stale errors
	 * into a callback that fails without setting its own errno. */
	errno = 0;
	/* A successful fstatat() does not clear errno. Avoid carrying a stale
	 * lookup error into a callback that fails without setting errno. */
	errno = 0;
	if (writer(stream, context) != 0) {
		if (!errno)
			errno = EIO;
		goto failed;
	}
	if (fflush(stream) != 0)
		goto failed;
	if (fsync(fileno(stream)) != 0)
		goto failed;
	if (fclose(stream) != 0) {
		stream = NULL;
		goto failed;
	}
	stream = NULL;

	if (renameat(directory_fd, temp_leaf, directory_fd, leaf) != 0)
		goto failed;
	temp_created = 0;
	return 0;

failed:
	saved_errno = errno ? errno : EIO;
	if (stream)
		(void)fclose(stream);
	else if (fd >= 0)
		(void)close(fd);
	if (temp_created)
		(void)unlinkat(directory_fd, temp_leaf, 0);
	errno = saved_errno;
	return -1;
}


struct disk_share_atomic_buffer_context {
	const char *contents;
	size_t length;
};

static inline int
disk_share_write_atomic_buffer(FILE *stream, void *opaque)
{
	struct disk_share_atomic_buffer_context *context =
		(struct disk_share_atomic_buffer_context *)opaque;

	if (!context || (!context->contents && context->length != 0)) {
		errno = EINVAL;
		return -1;
	}
	if (context->length > 0 &&
	    fwrite(context->contents, 1, context->length, stream) != context->length) {
		if (!errno)
			errno = EIO;
		return -1;
	}
	return 0;
}


static inline int
disk_share_replace_file_atomic_at(int directory_fd, const char *leaf,
                                  const char *contents, size_t contents_len)
{
	struct disk_share_atomic_buffer_context context;

	if (!contents && contents_len != 0) {
		errno = EINVAL;
		return -1;
	}
	context.contents = contents;
	context.length = contents_len;
	return disk_share_write_stream_atomic_at(
		directory_fd, leaf, disk_share_write_atomic_buffer, &context, 1);
}


/*
 * Classify a storage entry without following symbolic links. Missing entries
 * are stale cache data and are skipped; other stat errors invalidate the scan
 * so callers never publish an incomplete share inventory.
 */
static inline int
disk_share_folder_entry_status(int stat_result, int stat_error, mode_t mode)
{
	if (stat_result == 0)
		return S_ISDIR(mode) ? 1 : 0;
	if (stat_error == ENOENT)
		return 0;
	errno = stat_error ? stat_error : EIO;
	return -1;
}


/*
 * Only real, valid, non-hidden, non-system directories are shareable.
 * A removable-volume folder-list cache is untrusted input, so callers must
 * apply this policy to cached names as well as names returned by readdir().
 */
static inline int
disk_share_folder_name_is_visible(const char *name, int valid_name,
                                  int system_folder)
{
	return name && *name && name[0] != '.' && valid_name && !system_folder;
}

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

	if (!mount_path || !*mount_path || !sh_num || !folder_list) {
		errno = EINVAL;
		return -1;
	}

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


/*
 * Locate one complete ACL record by its case-insensitive "*folder=" key.
 * Returns 1 when unique, 0 when absent, and -1 for malformed/duplicate data.
 * The offset points to the record key; callers must not use substring matches
 * because removable-volume permission files are untrusted input.
 */
static inline int
disk_share_permission_find_unique_record(const char *source, const char *target,
                                          size_t *record_offset)
{
	size_t source_len, target_len, offset, i, value_offset;
	int found = 0;

	if (record_offset)
		*record_offset = 0;
	if (!source || !target || !*target || !record_offset) {
		errno = EINVAL;
		return -1;
	}

	source_len = strlen(source);
	target_len = strlen(target);
	if (target[target_len - 1] != '=') {
		errno = EINVAL;
		return -1;
	}
	if (target_len > source_len) {
		errno = ENOENT;
		return 0;
	}

	for (offset = 0; offset <= source_len - target_len; ++offset) {
		if (offset != 0 && source[offset - 1] != '\n')
			continue;
		for (i = 0; i < target_len; ++i) {
			if (tolower((unsigned char)source[offset + i]) !=
			    tolower((unsigned char)target[i]))
				break;
		}
		if (i != target_len)
			continue;
		if (found) {
			errno = EEXIST;
			*record_offset = 0;
			return -1;
		}

		value_offset = offset + target_len;
		if (source_len - value_offset < 4 ||
		    source[value_offset + 3] != '\n' ||
		    source[value_offset] < '0' || source[value_offset] > '3' ||
		    source[value_offset + 1] < '0' ||
		    source[value_offset + 1] > '3' ||
		    source[value_offset + 2] < '0' ||
		    source[value_offset + 2] > '3') {
			errno = EINVAL;
			*record_offset = 0;
			return -1;
		}
		found = 1;
		*record_offset = offset;
	}

	if (!found) {
		errno = ENOENT;
		return 0;
	}
	return 1;
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
			if (match) {
				/*
				 * A permission key must have one authoritative record.
				 * Refuse ambiguous metadata rather than updating/removing
				 * only the first duplicate and leaving stale rights behind.
				 */
				errno = EEXIST;
				return -1;
			}
			match = source + offset;
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


/*
 * Rename a directory entry without following its final component or any
 * parent symlink. The caller pins both parent directories before reaching
 * this helper, so renameat() remains anchored if a mount path is replaced.
 */
static inline int
disk_share_rename_directory_entry_nofollow(int old_directory_fd,
                                            const char *old_leaf,
                                            int new_directory_fd,
                                            const char *new_leaf)
{
	struct stat source_status, target_status;

	if (old_directory_fd < 0 || new_directory_fd < 0 ||
	    !old_leaf || !*old_leaf || !new_leaf || !*new_leaf ||
	    strchr(old_leaf, '/') || strchr(new_leaf, '/') ||
	    !strcmp(old_leaf, ".") || !strcmp(old_leaf, "..") ||
	    !strcmp(new_leaf, ".") || !strcmp(new_leaf, "..")) {
		errno = EINVAL;
		return -1;
	}
	if (fstatat(old_directory_fd, old_leaf, &source_status,
	            AT_SYMLINK_NOFOLLOW) != 0)
		return -1;
	if (!S_ISDIR(source_status.st_mode)) {
		errno = ENOTDIR;
		return -1;
	}
	if (fstatat(new_directory_fd, new_leaf, &target_status,
	            AT_SYMLINK_NOFOLLOW) == 0) {
		errno = EEXIST;
		return -1;
	}
	if (errno != ENOENT)
		return -1;

	return renameat(old_directory_fd, old_leaf,
	                new_directory_fd, new_leaf);
}


/*
 * Create or remove a single directory entry through its already-pinned parent.
 * These helpers never follow a final symlink and reject path-like leaf names.
 */
static inline int
disk_share_create_directory_entry_nofollow(int directory_fd,
                                           const char *leaf, mode_t mode)
{
	if (directory_fd < 0 || !leaf || !*leaf || strchr(leaf, '/') ||
	    !strcmp(leaf, ".") || !strcmp(leaf, "..")) {
		errno = EINVAL;
		return -1;
	}
	return mkdirat(directory_fd, leaf, mode);
}


static inline int
disk_share_remove_directory_entry_nofollow(int directory_fd,
                                           const char *leaf)
{
	struct stat status;

	if (directory_fd < 0 || !leaf || !*leaf || strchr(leaf, '/') ||
	    !strcmp(leaf, ".") || !strcmp(leaf, "..")) {
		errno = EINVAL;
		return -1;
	}
	if (fstatat(directory_fd, leaf, &status, AT_SYMLINK_NOFOLLOW) != 0)
		return -1;
	if (!S_ISDIR(status.st_mode)) {
		errno = ENOTDIR;
		return -1;
	}
	return unlinkat(directory_fd, leaf, AT_REMOVEDIR);
}


typedef int (*disk_share_folder_move_fn)(const char *, const char *);
typedef int (*disk_share_folder_refresh_fn)(const char *);

static inline int
disk_share_folder_rename_batch(
	const char *old_path, const char *new_path, const char *mount_path,
	struct disk_share_permission_batch_entry *entries, int count,
	disk_share_permission_batch_writer write_file,
	disk_share_folder_move_fn move_path,
	disk_share_folder_refresh_fn refresh_snapshot, int *rollback_error)
{
	int operation_error, compensation_error = 0;

	if (rollback_error)
		*rollback_error = 0;
	if (!old_path || !*old_path || !new_path || !*new_path ||
	    !mount_path || !*mount_path || count < 0 ||
	    (count > 0 && !entries) || !write_file || !move_path ||
	    !refresh_snapshot || !rollback_error) {
		errno = EINVAL;
		return -1;
	}

	if (move_path(old_path, new_path) != 0)
		return -1;
	if (refresh_snapshot(mount_path) != 0)
		goto compensate;
	if (disk_share_permission_batch_apply(
		    entries, count, write_file, &compensation_error) != 0)
		goto compensate;
	return 0;

compensate:
	operation_error = errno ? errno : EIO;
	/*
	 * apply() attempts its own rollback. A second pass retries any entry
	 * still marked applied before the directory is restored.
	 */
	(void)disk_share_permission_batch_rollback(
		entries, count, write_file, &compensation_error);
	if (move_path(new_path, old_path) != 0 && compensation_error == 0)
		compensation_error = errno ? errno : EIO;
	if (refresh_snapshot(mount_path) != 0 && compensation_error == 0)
		compensation_error = errno ? errno : EIO;

	*rollback_error = compensation_error;
	if (compensation_error != 0) {
		errno = EIO;
		return -1;
	}
	errno = operation_error;
	return -1;
}


typedef int (*disk_share_folder_remove_fn)(const char *);

static inline int
disk_share_folder_delete_batch(
	const char *folder_path, const char *staging_path, const char *mount_path,
	struct disk_share_permission_batch_entry *entries, int count,
	disk_share_permission_batch_writer write_file,
	disk_share_folder_move_fn move_path,
	disk_share_folder_remove_fn remove_path,
	disk_share_folder_refresh_fn refresh_snapshot,
	int *directory_staged, int *rollback_error)
{
	int operation_error, compensation_error = 0;

	if (directory_staged)
		*directory_staged = 0;
	if (rollback_error)
		*rollback_error = 0;
	if (!folder_path || !*folder_path || !staging_path || !*staging_path ||
	    !mount_path || !*mount_path || count < 0 ||
	    (count > 0 && !entries) || !write_file || !move_path ||
	    !remove_path || !refresh_snapshot || !directory_staged ||
	    !rollback_error) {
		errno = EINVAL;
		return -1;
	}

	if (move_path(folder_path, staging_path) != 0)
		return -1;
	*directory_staged = 1;
	if (refresh_snapshot(mount_path) != 0)
		goto compensate;
	if (disk_share_permission_batch_apply(
		    entries, count, write_file, &compensation_error) != 0)
		goto compensate;
	if (remove_path(staging_path) != 0)
		goto compensate;

	*directory_staged = 0;
	return 0;

compensate:
	operation_error = errno ? errno : EIO;
	/*
	 * apply() already attempts rollback; retry entries left marked applied
	 * before restoring the directory and refreshing the visible inventory.
	 */
	(void)disk_share_permission_batch_rollback(
		entries, count, write_file, &compensation_error);
	if (move_path(staging_path, folder_path) != 0) {
		if (compensation_error == 0)
			compensation_error = errno ? errno : EIO;
	} else {
		*directory_staged = 0;
	}
	if (refresh_snapshot(mount_path) != 0 && compensation_error == 0)
		compensation_error = errno ? errno : EIO;

	*rollback_error = compensation_error;
	if (compensation_error != 0) {
		errno = EIO;
		return -1;
	}
	errno = operation_error;
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

