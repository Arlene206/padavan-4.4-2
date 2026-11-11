#ifndef PADAVAN_OPENVPN_CONFIG_FILE_H
#define PADAVAN_OPENVPN_CONFIG_FILE_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

/* Keep a staged configuration private until every write has succeeded. */
static inline FILE *
openvpn_config_file_open(const char *path, char *staged_path,
                         size_t staged_path_size)
{
	int fd = -1;
	int path_len;
	int saved_errno;
	FILE *fp;
	struct stat st;

	if (!path || !path[0] || !staged_path || staged_path_size == 0) {
		errno = EINVAL;
		return NULL;
	}

	path_len = snprintf(staged_path, staged_path_size, "%s.tmp.XXXXXX", path);
	if (path_len < 0 || (size_t)path_len >= staged_path_size) {
		staged_path[0] = '\0';
		errno = ENAMETOOLONG;
		return NULL;
	}

	fd = mkstemp(staged_path);
	if (fd < 0)
		return NULL;

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
	if (fd >= 0)
		(void)close(fd);
	(void)unlink(staged_path);
	errno = saved_errno;
	return NULL;
}

static inline int
openvpn_config_file_publish(FILE **stream, const char *staged_path,
                            const char *path, mode_t mode)
{
	int fd;
	int sync_result;
	int saved_errno;
	FILE *fp;

	if (!stream || !*stream || !staged_path || !staged_path[0] ||
	    !path || !path[0]) {
		errno = EINVAL;
		return -1;
	}

	fp = *stream;
	if (fflush(fp) != 0)
		goto failed;
	if (ferror(fp)) {
		errno = EIO;
		goto failed;
	}

	fd = fileno(fp);
	if (fd < 0)
		goto failed;
	do {
		sync_result = fsync(fd);
	} while (sync_result != 0 && errno == EINTR);
	if (sync_result != 0)
		goto failed;

	/* The stage remains owner-only while data is being generated. */
	if (fchmod(fd, mode) != 0)
		goto failed;
	do {
		sync_result = fsync(fd);
	} while (sync_result != 0 && errno == EINTR);
	if (sync_result != 0)
		goto failed;

	if (fclose(fp) != 0) {
		*stream = NULL;
		if (!errno)
			errno = EIO;
		goto failed;
	}
	*stream = NULL;

	if (rename(staged_path, path) != 0)
		goto failed;

	return 0;

failed:
	saved_errno = errno ? errno : EIO;
	if (*stream) {
		(void)fclose(*stream);
		*stream = NULL;
	}
	(void)unlink(staged_path);
	errno = saved_errno;
	return -1;
}

#endif /* PADAVAN_OPENVPN_CONFIG_FILE_H */
