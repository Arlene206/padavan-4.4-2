#ifndef XUPNPD_CONFIG_FILE_H
#define XUPNPD_CONFIG_FILE_H

#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

static inline int
xupnpd_config_valid_interface(const char *name)
{
	size_t i, length;
	unsigned char c;

	if (name == NULL)
		return 0;
	length = strlen(name);
	if (length == 0 || length >= 16)
		return 0;
	for (i = 0; i < length; i++) {
		c = (unsigned char)name[i];
		if (!((c >= 'a' && c <= 'z') ||
		      (c >= 'A' && c <= 'Z') ||
		      (c >= '0' && c <= '9') ||
		      c == '_' || c == '-' || c == '.'))
			return 0;
	}
	return 1;
}

static inline int
xupnpd_config_valid_ipv4(const char *address)
{
	struct in_addr parsed;

	return address != NULL && inet_pton(AF_INET, address, &parsed) == 1;
}

/* Create an exclusive same-directory temporary file; never follow a
 * pre-created destination.tmp symlink. Preserve regular-file mode. */
static inline int
xupnpd_config_tempfile_open(const char *destination, char *temporary,
			    size_t temporary_size)
{
	struct stat st;
	size_t length;
	mode_t mode = 0644;
	int fd, saved_errno, name_length;

	if (destination == NULL || temporary == NULL) {
		errno = EINVAL;
		return -1;
	}
	length = strlen(destination);
	if (length + sizeof(".tmp.XXXXXX") > temporary_size) {
		errno = ENAMETOOLONG;
		return -1;
	}
	if (lstat(destination, &st) == 0) {
		if (S_ISREG(st.st_mode))
			mode = st.st_mode & 0777;
	} else if (errno != ENOENT) {
		return -1;
	}
	name_length = snprintf(temporary, temporary_size, "%s.tmp.XXXXXX",
			       destination);
	if (name_length < 0 || (size_t)name_length >= temporary_size) {
		errno = ENAMETOOLONG;
		return -1;
	}
	fd = mkstemp(temporary);
	if (fd < 0)
		return -1;
	if (fchmod(fd, mode) == 0)
		return fd;
	saved_errno = errno ? errno : EIO;
	close(fd);
	unlink(temporary);
	errno = saved_errno;
	return -1;
}

#endif
