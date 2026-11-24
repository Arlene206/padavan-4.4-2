#ifndef PADAVAN_OPENVPN_CLIENT_SECRET_FILE_H
#define PADAVAN_OPENVPN_CLIENT_SECRET_FILE_H

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

/* Publish credentials only after a private, same-directory staging file is
 * complete. mkstemp creates mode 0600 regardless of the caller's umask. */
static inline int
openvpn_client_secret_publish(const char *path, const char *username,
                              const char *password)
{
	char staged_path[PATH_MAX];
	int fd = -1;
	int sync_result;
	int saved_errno;
	int path_len;
	FILE *fp = NULL;
	struct stat st;

	if (!path || !path[0] || !username || !password) {
		errno = EINVAL;
		return -1;
	}

	path_len = snprintf(staged_path, sizeof(staged_path), "%s.tmp.XXXXXX", path);
	if (path_len < 0 || (size_t)path_len >= sizeof(staged_path)) {
		errno = ENAMETOOLONG;
		return -1;
	}

	fd = mkstemp(staged_path);
	if (fd < 0)
		return -1;

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
	fd = -1; /* fp owns the descriptor from here onward. */

	if (fprintf(fp, "%s\n%s\n", username, password) < 0) {
		if (!errno)
			errno = EIO;
		goto failed;
	}
	if (fflush(fp) != 0 || ferror(fp)) {
		if (!errno)
			errno = EIO;
		goto failed;
	}

	do {
		sync_result = fsync(fileno(fp));
	} while (sync_result != 0 && errno == EINTR);
	if (sync_result != 0)
		goto failed;

	if (fclose(fp) != 0) {
		fp = NULL;
		if (!errno)
			errno = EIO;
		goto failed;
	}
	fp = NULL;

	if (rename(staged_path, path) != 0)
		goto failed;

	return 0;

failed:
	saved_errno = errno ? errno : EIO;
	if (fp)
		(void)fclose(fp);
	else if (fd >= 0)
		(void)close(fd);
	(void)unlink(staged_path);
	errno = saved_errno;
	return -1;
}

#endif /* PADAVAN_OPENVPN_CLIENT_SECRET_FILE_H */
