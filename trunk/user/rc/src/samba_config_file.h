#ifndef PADAVAN_SAMBA_CONFIG_FILE_H
#define PADAVAN_SAMBA_CONFIG_FILE_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>

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
	return 0;
}

#endif /* PADAVAN_SAMBA_CONFIG_FILE_H */
