#ifndef PADAVAN_FTP_CONFIG_FILE_H
#define PADAVAN_FTP_CONFIG_FILE_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

/*
 * Build generated FTP configuration beside its destination. Keeping the
 * temporary file in the same directory makes the final rename atomic.
 */
static int
ftp_config_file_open(const char *target_path, char *staged_path,
                     size_t staged_path_size, FILE **stream)
{
	int fd;
	int path_len;
	int saved_errno;

	if (!target_path || !*target_path || !staged_path ||
	    staged_path_size == 0 || !stream) {
		errno = EINVAL;
		return -1;
	}
	*stream = NULL;
	staged_path[0] = '\0';

	path_len = snprintf(staged_path, staged_path_size, "%s.XXXXXX",
			    target_path);
	if (path_len < 0) {
		staged_path[0] = '\0';
		errno = EINVAL;
		return -1;
	}
	if ((size_t)path_len >= staged_path_size) {
		staged_path[0] = '\0';
		errno = ENAMETOOLONG;
		return -1;
	}

	fd = mkstemp(staged_path);
	if (fd < 0) {
		saved_errno = errno;
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

/*
 * Do not replace a working configuration until all buffered output and the
 * close operation have succeeded. On every failure, discard the staged file.
 */
static int
ftp_config_file_publish(FILE **stream, char *staged_path,
                        const char *target_path)
{
	FILE *fp;
	int saved_errno;

	if (!stream || !*stream || !staged_path || !*staged_path ||
	    !target_path || !*target_path) {
		errno = EINVAL;
		return -1;
	}

	fp = *stream;
	*stream = NULL;

	errno = 0;
	if (ferror(fp) || fflush(fp) != 0) {
		saved_errno = errno ? errno : EIO;
		fclose(fp);
		unlink(staged_path);
		staged_path[0] = '\0';
		errno = saved_errno;
		return -1;
	}

	errno = 0;
	if (fclose(fp) != 0) {
		saved_errno = errno ? errno : EIO;
		unlink(staged_path);
		staged_path[0] = '\0';
		errno = saved_errno;
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

#endif /* PADAVAN_FTP_CONFIG_FILE_H */
