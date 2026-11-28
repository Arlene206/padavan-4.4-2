#ifndef PADAVAN_SAMBA_CONFIG_FILE_H
#define PADAVAN_SAMBA_CONFIG_FILE_H

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

/* Persist the directory entry as well as the staged file contents. */
static int
samba_config_file_sync_parent(const char *target_path)
{
	char *parent_path, *slash;
	size_t path_length;
	int directory_fd = -1, result = -1, saved_errno = 0;
	int sync_status;

	if (!target_path || !*target_path) {
		errno = EINVAL;
		return -1;
	}
	path_length = strlen(target_path);
	parent_path = (char *)malloc(path_length + 1);
	if (!parent_path)
		return -1;
	memcpy(parent_path, target_path, path_length + 1);

	slash = strrchr(parent_path, '/');
	if (!slash) {
		parent_path[0] = '.';
		parent_path[1] = '\0';
	} else if (slash == parent_path) {
		slash[1] = '\0';
	} else {
		*slash = '\0';
	}

	directory_fd = open(parent_path, O_RDONLY | O_DIRECTORY | O_NOFOLLOW);
	if (directory_fd < 0)
		goto out;
	do {
		errno = 0;
		sync_status = fsync(directory_fd);
	} while (sync_status != 0 && errno == EINTR);
	if (sync_status != 0)
		goto out;
	if (close(directory_fd) != 0) {
		directory_fd = -1;
		goto out;
	}
	directory_fd = -1;
	result = 0;

out:
	saved_errno = errno;
	if (directory_fd >= 0)
		close(directory_fd);
	free(parent_path);
	if (result != 0)
		errno = saved_errno ? saved_errno : EIO;
	return result;
}

/* Stage beside the destination so the final rename is atomic. */
static int
samba_config_file_open(const char *target_path, char *staged_path,
                       size_t staged_path_size, FILE **stream)
{
	int fd, path_len, saved_errno;

	if (!stream) {
		errno = EINVAL;
		return -1;
	}
	*stream = NULL;
	if (!target_path || !*target_path || !staged_path ||
	    staged_path_size == 0) {
		errno = EINVAL;
		return -1;
	}
	staged_path[0] = '\0';

	path_len = snprintf(staged_path, staged_path_size, "%s.XXXXXX",
			    target_path);
	if (path_len < 0 || (size_t)path_len >= staged_path_size) {
		staged_path[0] = '\0';
		errno = ENAMETOOLONG;
		return -1;
	}

	fd = mkstemp(staged_path);
	if (fd < 0) {
		saved_errno = errno ? errno : EIO;
		staged_path[0] = '\0';
		errno = saved_errno;
		return -1;
	}
	if (fchmod(fd, 0644) != 0)
		goto failed;

	*stream = fdopen(fd, "w");
	if (!*stream)
		goto failed;

	return 0;

failed:
	saved_errno = errno ? errno : EIO;
	close(fd);
	unlink(staged_path);
	staged_path[0] = '\0';
	errno = saved_errno;
	return -1;
}

/* Close and discard an incomplete candidate without touching the live file. */
static void
samba_config_file_discard(FILE **stream, char *staged_path)
{
	int saved_errno = errno;

	if (stream && *stream) {
		fclose(*stream);
		*stream = NULL;
	}
	if (staged_path && staged_path[0]) {
		unlink(staged_path);
		staged_path[0] = '\0';
	}
	errno = saved_errno;
}

/* Publish only after buffered writes and close have both succeeded. */
static int
samba_config_file_publish(FILE **stream, char *staged_path,
                          const char *target_path)
{
	FILE *fp;
	int failed = 0, saved_errno = 0;

	if (!stream || !*stream || !staged_path || !*staged_path ||
	    !target_path || !*target_path) {
		errno = EINVAL;
		return -1;
	}

	fp = *stream;
	*stream = NULL;

	errno = 0;
	if (ferror(fp)) {
		failed = 1;
		saved_errno = errno ? errno : EIO;
	}
	if (fflush(fp) != 0) {
		if (!saved_errno)
			saved_errno = errno ? errno : EIO;
		failed = 1;
	}
	if (!failed && fsync(fileno(fp)) != 0) {
		if (!saved_errno)
			saved_errno = errno ? errno : EIO;
		failed = 1;
	}
	if (fclose(fp) != 0) {
		if (!saved_errno)
			saved_errno = errno ? errno : EIO;
		failed = 1;
	}

	if (failed) {
		unlink(staged_path);
		staged_path[0] = '\0';
		errno = saved_errno ? saved_errno : EIO;
		return -1;
	}

	if (rename(staged_path, target_path) != 0) {
		saved_errno = errno ? errno : EIO;
		unlink(staged_path);
		staged_path[0] = '\0';
		errno = saved_errno;
		return -1;
	}

	staged_path[0] = '\0';
	/*
	 * The file contents are synced before rename; persist the directory entry
	 * before reporting the new configuration as published.
	 */
	if (samba_config_file_sync_parent(target_path) != 0)
		return -1;
	return 0;
}

#endif /* PADAVAN_SAMBA_CONFIG_FILE_H */
