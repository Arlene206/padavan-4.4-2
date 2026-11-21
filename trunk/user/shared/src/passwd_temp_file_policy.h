#ifndef PADAVAN_PASSWD_TEMP_FILE_POLICY_H
#define PADAVAN_PASSWD_TEMP_FILE_POLICY_H

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static int
passwd_temp_file_open(char *path_template, mode_t mode)
{
	int fd, saved_errno;
	if (!path_template || !path_template[0]) {
		errno = EINVAL;
		return -1;
	}
	fd = mkstemp(path_template);
	if (fd < 0)
		return -1;
	if (fchmod(fd, mode) != 0) {
		saved_errno = errno ? errno : EIO;
		(void)close(fd);
		(void)unlink(path_template);
		errno = saved_errno;
		return -1;
	}
	return fd;
}

typedef struct {
	const char *staged_path;
	const char *destination_path;
	char *backup_path;
	int had_original;
} passwd_file_update_t;

typedef int (*passwd_file_rename_fn)(const char *, const char *);
typedef int (*passwd_file_unlink_fn)(const char *);

/*
 * Retain the previous regular file as a same-filesystem hard link. A missing
 * original is valid and is represented by an empty backup path; other errors
 * fail before any destination is replaced.
 */
static int
passwd_temp_file_backup_create(const char *source_path,
			       char *backup_template, int *had_original)
{
	struct stat source_stat, backup_stat;
	int fd, saved_errno;

	if (!source_path || !source_path[0] ||
	    !backup_template || !backup_template[0] || !had_original) {
		errno = EINVAL;
		return -1;
	}
	*had_original = 0;
	if (lstat(source_path, &source_stat) != 0) {
		saved_errno = errno ? errno : EIO;
		if (saved_errno == ENOENT) {
			backup_template[0] = '\0';
			return 0;
		}
		errno = saved_errno;
		return -1;
	}
	if (!S_ISREG(source_stat.st_mode)) {
		errno = EINVAL;
		return -1;
	}

	fd = mkstemp(backup_template);
	if (fd < 0) {
		backup_template[0] = '\0';
		return -1;
	}
	if (close(fd) != 0) {
		saved_errno = errno ? errno : EIO;
		(void)unlink(backup_template);
		backup_template[0] = '\0';
		errno = saved_errno;
		return -1;
	}
	if (unlink(backup_template) != 0) {
		saved_errno = errno ? errno : EIO;
		errno = saved_errno;
		return -1;
	}
	if (link(source_path, backup_template) != 0) {
		saved_errno = errno ? errno : EIO;
		backup_template[0] = '\0';
		errno = saved_errno;
		return -1;
	}
	if (lstat(backup_template, &backup_stat) != 0 ||
	    !S_ISREG(backup_stat.st_mode) ||
	    backup_stat.st_dev != source_stat.st_dev ||
	    backup_stat.st_ino != source_stat.st_ino) {
		saved_errno = errno ? errno : EIO;
		(void)unlink(backup_template);
		backup_template[0] = '\0';
		errno = saved_errno;
		return -1;
	}
	*had_original = 1;
	return 0;
}

/*
 * Publish a related passwd/group/shadow set as one rollback-capable operation.
 * POSIX cannot atomically rename several paths together, so each old inode is
 * retained until every staged file has been renamed. If a later rename fails,
 * earlier destinations are restored in reverse order. A failed restoration
 * leaves its backup link in place for manual recovery.
 */
static int
passwd_file_publish_transaction(passwd_file_update_t *updates, size_t count,
				 passwd_file_rename_fn rename_fn,
				 passwd_file_unlink_fn unlink_fn,
				 int *rollback_failed)
{
	size_t i, published, failed_at;
	int saved_errno, rollback_errno, cleanup_errno;

	if (!updates || count == 0 || !rename_fn || !unlink_fn ||
	    !rollback_failed) {
		errno = EINVAL;
		return -1;
	}
	*rollback_failed = 0;
	for (i = 0; i < count; ++i) {
		if (!updates[i].staged_path || !updates[i].staged_path[0] ||
		    !updates[i].destination_path || !updates[i].destination_path[0] ||
		    !updates[i].backup_path ||
		    (updates[i].had_original && !updates[i].backup_path[0])) {
			errno = EINVAL;
			return -1;
		}
	}

	published = 0;
	for (i = 0; i < count; ++i) {
		if (rename_fn(updates[i].staged_path,
			      updates[i].destination_path) != 0) {
			saved_errno = errno ? errno : EIO;
			failed_at = i;
			goto rollback;
		}
		published++;
	}

	cleanup_errno = 0;
	for (i = 0; i < count; ++i) {
		if (updates[i].backup_path[0]) {
			if (unlink_fn(updates[i].backup_path) != 0 && errno != ENOENT) {
				if (cleanup_errno == 0)
					cleanup_errno = errno ? errno : EIO;
			} else {
				updates[i].backup_path[0] = '\0';
			}
		}
	}
	if (cleanup_errno != 0) {
		errno = cleanup_errno;
		return -1;
	}
	return 0;

rollback:
	rollback_errno = 0;
	for (i = published; i > 0; --i) {
		passwd_file_update_t *update = &updates[i - 1];

		if (update->had_original) {
			if (!update->backup_path[0] ||
			    rename_fn(update->backup_path,
				      update->destination_path) != 0) {
				if (rollback_errno == 0)
					rollback_errno = errno ? errno : EIO;
			} else {
				update->backup_path[0] = '\0';
			}
		} else if (unlink_fn(update->destination_path) != 0 &&
			   errno != ENOENT) {
			if (rollback_errno == 0)
				rollback_errno = errno ? errno : EIO;
		}
	}
	cleanup_errno = 0;
	for (i = failed_at; i < count; ++i) {
		if (updates[i].backup_path[0]) {
			if (unlink_fn(updates[i].backup_path) != 0 && errno != ENOENT) {
				if (cleanup_errno == 0)
					cleanup_errno = errno ? errno : EIO;
			} else {
				updates[i].backup_path[0] = '\0';
			}
		}
	}
	if (rollback_errno != 0) {
		*rollback_failed = 1;
		errno = rollback_errno;
	} else if (cleanup_errno != 0) {
		errno = cleanup_errno;
	} else {
		errno = saved_errno;
	}
	return -1;
}

#endif
