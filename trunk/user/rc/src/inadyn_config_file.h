#ifndef PADAVAN_INADYN_CONFIG_FILE_H
#define PADAVAN_INADYN_CONFIG_FILE_H

#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif


/* Parse checked NVRAM integers without turning backend failures into defaults. */
static inline int
inadyn_config_file_read_integer_status(int api_result, int saved_errno,
                                       const char *value, int default_value,
                                       int minimum, int maximum, int *result)
{
	const unsigned char *cursor;
	char *end;
	long parsed;

	if (!result || minimum > maximum ||
	    default_value < minimum || default_value > maximum)
		return EINVAL;
	if (api_result < 0)
		return saved_errno > 0 ? saved_errno : EIO;
	if (api_result == 0) {
		*result = default_value;
		return 0;
	}
	if (api_result != 1 || !value)
		return EIO;

	cursor = (const unsigned char *)value;
	while (*cursor && isspace(*cursor))
		++cursor;
	if (!*cursor) {
		*result = default_value;
		return 0;
	}
	if (!isdigit(*cursor))
		return EINVAL;

	errno = 0;
	parsed = strtol(value, &end, 10);
	if (errno == ERANGE)
		return ERANGE;
	if (end == value)
		return EINVAL;
	cursor = (const unsigned char *)end;
	while (*cursor && isspace(*cursor))
		++cursor;
	if (*cursor || parsed < minimum || parsed > maximum)
		return EINVAL;

	*result = (int)parsed;
	return 0;
}

/* Do not leave DDNS usernames/passwords in the PID 1 heap after use.
 * Volatile stores prevent the compiler from optimizing away the wipe. */
static inline void
inadyn_config_file_clear_sensitive(void *buffer, size_t length)
{
	volatile unsigned char *cursor = (volatile unsigned char *)buffer;

	if (!cursor)
		return;
	while (length-- > 0)
		*cursor++ = 0;
}

/* Emit a libConfuse string fragment. Never let NVRAM values create new
 * directives or change the surrounding quoted-string syntax. */
static inline int
inadyn_config_file_write_escaped(FILE *fp, const char *value)
{
	const unsigned char *cursor;

	if (!fp || !value) {
		errno = EINVAL;
		return -1;
	}

	for (cursor = (const unsigned char *)value; *cursor; cursor++) {
		switch (*cursor) {
		case '\\':
			if (fputs("\\\\", fp) == EOF)
				return -1;
			break;
		case '"':
			if (fputs("\\\"", fp) == EOF)
				return -1;
			break;
		case '\n':
			if (fputs("\\n", fp) == EOF)
				return -1;
			break;
		case '\r':
			if (fputs("\\r", fp) == EOF)
				return -1;
			break;
		case '\t':
			if (fputs("\\t", fp) == EOF)
				return -1;
			break;
		default:
			if (*cursor < 0x20 || *cursor == 0x7f) {
				errno = EINVAL;
				return -1;
			}
			if (fputc(*cursor, fp) == EOF)
				return -1;
			break;
		}
	}

	return 0;
}

static inline int
inadyn_config_file_write_quoted_setting(FILE *fp, const char *prefix,
                                        const char *value, const char *suffix)
{
	if (!fp || !prefix || !value || !suffix) {
		errno = EINVAL;
		return -1;
	}
	if (fputs(prefix, fp) == EOF ||
	    inadyn_config_file_write_escaped(fp, value) != 0 ||
	    fputs(suffix, fp) == EOF)
		return -1;

	return 0;
}

/*
 * Keep the last-known-good config reachable until the new daemon is proven
 * RUNNING. A same-directory hard link makes restoration an atomic rename.
 */
struct inadyn_config_file_snapshot {
	char backup_path[PATH_MAX];
	dev_t device;
	ino_t inode;
	int had_original;
	int active;
};

static inline int
inadyn_config_file_sync_directory(const char *directory)
{
	int fd, flags = O_RDONLY, result, saved_errno;
	if (!directory || !directory[0]) { errno = EINVAL; return -1; }
#if !defined(O_DIRECTORY) || !defined(O_NOFOLLOW)
	errno = ENOTSUP;
	return -1;
#else
	flags |= O_DIRECTORY | O_NOFOLLOW;
#endif
	fd = open(directory, flags);
	if (fd < 0) return -1;
	do { result = fsync(fd); } while (result != 0 && errno == EINTR);
	if (result != 0) {
		saved_errno = errno ? errno : EIO;
		(void)close(fd); errno = saved_errno; return -1;
	}
	if (close(fd) != 0) return -1;
	errno = 0;
	return 0;
}

static inline int
inadyn_config_file_sync_parent(const char *path)
{
	char parent[PATH_MAX];
	char *slash;
	size_t length;
	if (!path || !path[0]) { errno = EINVAL; return -1; }
	length = strlen(path);
	if (length >= sizeof(parent)) { errno = ENAMETOOLONG; return -1; }
	memcpy(parent, path, length + 1);
	slash = strrchr(parent, '/');
	if (!slash) { parent[0] = '.'; parent[1] = '\0'; }
	else if (slash == parent) slash[1] = '\0';
	else *slash = '\0';
	return inadyn_config_file_sync_directory(parent);
}

static inline int
inadyn_config_file_snapshot_matches(
	const struct inadyn_config_file_snapshot *snapshot)
{
	struct stat backup;
	if (!snapshot || !snapshot->active || !snapshot->had_original ||
	    !snapshot->backup_path[0]) { errno = EINVAL; return -1; }
	if (lstat(snapshot->backup_path, &backup) != 0) return -1;
	if (!S_ISREG(backup.st_mode) || backup.st_uid != geteuid() ||
	    (backup.st_mode & 077) != 0 ||
	    (backup.st_nlink != 1 && backup.st_nlink != 2) ||
	    backup.st_dev != snapshot->device ||
	    backup.st_ino != snapshot->inode) { errno = EAGAIN; return -1; }
	return 0;
}

static inline int
inadyn_config_file_snapshot_begin(
	const char *destination, struct inadyn_config_file_snapshot *snapshot)
{
	struct stat original, backup, current;
	unsigned int attempt;
	int length, linked = 0, saved_errno;
	if (!destination || !destination[0] || !snapshot) {
		errno = EINVAL; return -1;
	}
	memset(snapshot, 0, sizeof(*snapshot));
	if (lstat(destination, &original) != 0) {
		if (errno != ENOENT) return -1;
		if (inadyn_config_file_sync_parent(destination) != 0) return -1;
		snapshot->active = 1;
		return 0;
	}
	if (!S_ISREG(original.st_mode) || original.st_uid != geteuid() ||
	    original.st_nlink != 1 || (original.st_mode & 077) != 0) {
		errno = EPERM; return -1;
	}
	for (attempt = 0; attempt < 32; ++attempt) {
		length = snprintf(snapshot->backup_path, sizeof(snapshot->backup_path),
				 "%s.rollback.%ld.%u", destination, (long)getpid(), attempt);
		if (length < 0 || (size_t)length >= sizeof(snapshot->backup_path)) {
			snapshot->backup_path[0] = '\0'; errno = ENAMETOOLONG; return -1;
		}
		if (link(destination, snapshot->backup_path) == 0) { linked = 1; break; }
		if (errno != EEXIST) { snapshot->backup_path[0] = '\0'; return -1; }
	}
	if (!linked) { snapshot->backup_path[0] = '\0'; errno = EEXIST; return -1; }
	if (lstat(snapshot->backup_path, &backup) != 0 ||
	    lstat(destination, &current) != 0 ||
	    !S_ISREG(backup.st_mode) || backup.st_dev != original.st_dev ||
	    backup.st_ino != original.st_ino || backup.st_nlink != 2 ||
	    current.st_dev != original.st_dev || current.st_ino != original.st_ino) {
		saved_errno = errno ? errno : EAGAIN; goto failed;
	}
	if (inadyn_config_file_sync_parent(snapshot->backup_path) != 0) {
		saved_errno = errno ? errno : EIO; goto failed;
	}
	snapshot->device = original.st_dev;
	snapshot->inode = original.st_ino;
	snapshot->had_original = 1;
	snapshot->active = 1;
	return 0;
failed:
	if (unlink(snapshot->backup_path) == 0 || errno == ENOENT) {
		(void)inadyn_config_file_sync_parent(snapshot->backup_path);
		snapshot->backup_path[0] = '\0';
	}
	errno = saved_errno ? saved_errno : EIO;
	return -1;
}

static inline int
inadyn_config_file_snapshot_restore(
	const char *destination, struct inadyn_config_file_snapshot *snapshot)
{
	struct stat current;

	if (!destination || !destination[0] || !snapshot || !snapshot->active) {
		errno = EINVAL; return -1;
	}
	if (snapshot->had_original) {
		if (inadyn_config_file_snapshot_matches(snapshot) != 0) return -1;
		/*
		 * POSIX rename is a no-op when source and destination are hard links
		 * to the same inode. Remove our extra link explicitly in that case,
		 * otherwise old-config write failures leave st_nlink=2 and prevent
		 * the previous daemon from passing its private-config checks.
		 */
		if (lstat(destination, &current) == 0) {
			if (S_ISREG(current.st_mode) &&
			    current.st_dev == snapshot->device &&
			    current.st_ino == snapshot->inode) {
				if (unlink(snapshot->backup_path) != 0) return -1;
			} else if (rename(snapshot->backup_path, destination) != 0) {
				return -1;
			}
		} else if (errno == ENOENT) {
			if (rename(snapshot->backup_path, destination) != 0) return -1;
		} else {
			return -1;
		}
		snapshot->backup_path[0] = '\0';
	} else if (unlink(destination) != 0 && errno != ENOENT) return -1;
	snapshot->active = 0;
	snapshot->had_original = 0;
	return inadyn_config_file_sync_parent(destination);
}

static inline int
inadyn_config_file_snapshot_commit(
	const char *destination, struct inadyn_config_file_snapshot *snapshot)
{
	if (!destination || !destination[0] || !snapshot || !snapshot->active) {
		errno = EINVAL; return -1;
	}
	if (inadyn_config_file_sync_parent(destination) != 0) return -1;
	if (snapshot->had_original) {
		if (inadyn_config_file_snapshot_matches(snapshot) != 0) return -1;
		if (unlink(snapshot->backup_path) != 0) return -1;
		snapshot->backup_path[0] = '\0';
	}
	snapshot->active = 0;
	snapshot->had_original = 0;
	return inadyn_config_file_sync_parent(destination);
}

/* Stage DDNS settings beside the destination. mkstemp creates an owner-only
 * inode even when PID 1's process-wide umask is 0000. */
static inline FILE *
inadyn_config_file_open(const char *destination, char *staged_path,
                        size_t staged_path_size)
{
	int fd;
	int path_length;
	int saved_errno;
	struct stat st;
	FILE *fp;

	if (!staged_path || staged_path_size == 0) {
		errno = EINVAL;
		return NULL;
	}
	staged_path[0] = '\0';
	if (!destination || !destination[0]) {
		errno = EINVAL;
		return NULL;
	}

	path_length = snprintf(staged_path, staged_path_size, "%s.tmp.XXXXXX",
	                       destination);
	if (path_length < 0 || (size_t)path_length >= staged_path_size) {
		staged_path[0] = '\0';
		errno = ENAMETOOLONG;
		return NULL;
	}

	fd = mkstemp(staged_path);
	if (fd < 0) {
		staged_path[0] = '\0';
		return NULL;
	}

	if (fstat(fd, &st) != 0)
		goto failed;
	if (!S_ISREG(st.st_mode) || st.st_uid != geteuid() || st.st_nlink != 1) {
		errno = EPERM;
		goto failed;
	}
	if (fchmod(fd, S_IRUSR | S_IWUSR) != 0)
		goto failed;

	fp = fdopen(fd, "w");
	if (!fp)
		goto failed;

	return fp;

failed:
	saved_errno = errno ? errno : EIO;
	(void)close(fd);
	if (unlink(staged_path) == 0 || errno == ENOENT)
		staged_path[0] = '\0';
	errno = saved_errno;
	return NULL;
}

/* Discard an unpublished partial config without disturbing the live file. */
static inline int
inadyn_config_file_discard(FILE *fp, char *staged_path)
{
	int failed = 0;
	int saved_errno = 0;

	if (fp && fclose(fp) != 0) {
		failed = 1;
		saved_errno = errno ? errno : EIO;
	}
	if (staged_path && staged_path[0]) {
		if (unlink(staged_path) != 0 && errno != ENOENT) {
			if (!saved_errno)
				saved_errno = errno ? errno : EIO;
			failed = 1;
		} else {
			staged_path[0] = '\0';
		}
	}

	if (failed) {
		errno = saved_errno ? saved_errno : EIO;
		return -1;
	}
	return 0;
}

/* A failed write/flush/fsync/close/rename leaves the last-known-good config
 * in place. rename replaces a destination symlink itself, never its target. */
static inline int
inadyn_config_file_publish(FILE *fp, char *staged_path,
                           const char *destination)
{
	int failed = 0;
	int saved_errno = 0;
	int sync_result;

	if (!fp || !staged_path || !staged_path[0] ||
	    !destination || !destination[0]) {
		if (fp)
			(void)fclose(fp);
		(void)inadyn_config_file_discard(NULL, staged_path);
		errno = EINVAL;
		return -1;
	}

	errno = 0;
	if (fflush(fp) != 0) {
		failed = 1;
		saved_errno = errno ? errno : EIO;
	}
	if (ferror(fp) && !failed) {
		failed = 1;
		saved_errno = errno ? errno : EIO;
	}

	if (!failed) {
		do {
			sync_result = fsync(fileno(fp));
		} while (sync_result != 0 && errno == EINTR);
		if (sync_result != 0) {
			failed = 1;
			saved_errno = errno ? errno : EIO;
		}
	}

	if (fclose(fp) != 0) {
		if (!saved_errno)
			saved_errno = errno ? errno : EIO;
		failed = 1;
	}

	if (failed) {
		(void)inadyn_config_file_discard(NULL, staged_path);
		errno = saved_errno ? saved_errno : EIO;
		return -1;
	}

	if (rename(staged_path, destination) != 0) {
		saved_errno = errno ? errno : EIO;
		(void)inadyn_config_file_discard(NULL, staged_path);
		errno = saved_errno;
		return -1;
	}

	staged_path[0] = '\0';
	return 0;
}

#endif /* PADAVAN_INADYN_CONFIG_FILE_H */
