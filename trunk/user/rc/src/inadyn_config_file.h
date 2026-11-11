#ifndef PADAVAN_INADYN_CONFIG_FILE_H
#define PADAVAN_INADYN_CONFIG_FILE_H

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

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

	if (!destination || !destination[0] || !staged_path || staged_path_size == 0) {
		errno = EINVAL;
		return NULL;
	}
	staged_path[0] = '\0';

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
