#define _GNU_SOURCE
#include "scutclient_credentials.h"

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef O_NOFOLLOW
#error "SCUTClient credential files require O_NOFOLLOW support"
#endif
#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif

#define SCUTCLIENT_CREDENTIALS_MAX 4096

#ifndef SCUTCLIENT_CREDENTIALS_READ
#define SCUTCLIENT_CREDENTIALS_READ read
#endif

static void scutclient_credentials_wipe(void *memory, size_t length)
{
	volatile unsigned char *cursor = (volatile unsigned char *)memory;

	while (length-- > 0)
		*cursor++ = 0;
}

int scutclient_load_credentials_file(const char *path, char **username, char **password)
{
	int fd = -1;
	int saved_errno = 0;
	struct stat opened_stat;
	struct stat path_stat;
	char buffer[SCUTCLIENT_CREDENTIALS_MAX + 1];
	size_t used = 0;
	ssize_t count;
	char *user_start;
	char *user_end;
	char *pass_start;
	char *pass_end;
	char *user_copy = NULL;
	char *pass_copy = NULL;
	size_t user_length;
	size_t pass_length;

	if (!path || !path[0] || !username || !password) {
		errno = EINVAL;
		return -1;
	}
	*username = NULL;
	*password = NULL;

	fd = open(path, O_RDONLY | O_NOFOLLOW | O_CLOEXEC);
	if (fd < 0)
		return -1;

	if (fstat(fd, &opened_stat) < 0)
		goto fail;
	if (!S_ISREG(opened_stat.st_mode) || opened_stat.st_uid != geteuid() ||
	    (opened_stat.st_mode & 0777) != 0600 || opened_stat.st_nlink != 1 ||
	    opened_stat.st_size < 1 ||
	    opened_stat.st_size > SCUTCLIENT_CREDENTIALS_MAX) {
		errno = EPERM;
		goto fail;
	}

	/* The wrapper stores this in a private mode-0700 directory. Verify inode. */
	if (lstat(path, &path_stat) < 0)
		goto fail;
	if (!S_ISREG(path_stat.st_mode) || path_stat.st_dev != opened_stat.st_dev ||
	    path_stat.st_ino != opened_stat.st_ino) {
		errno = EPERM;
		goto fail;
	}
	if (unlink(path) < 0)
		goto fail;

	/* Remove the pathname before reading or parsing credential bytes. */
	while (used < sizeof(buffer)) {
		count = SCUTCLIENT_CREDENTIALS_READ(fd, buffer + used, sizeof(buffer) - used);
		if (count < 0) {
			if (errno == EINTR)
				continue;
			goto fail;
		}
		if (count == 0)
			break;
		used += (size_t)count;
	}
	if (used == sizeof(buffer) || used != (size_t)opened_stat.st_size) {
		errno = EIO;
		goto fail;
	}
	if (close(fd) < 0) {
		fd = -1;
		goto fail;
	}
	fd = -1;

	if (used < 2 || buffer[used - 1] != '\n' ||
	    memchr(buffer, '\0', used) != NULL) {
		errno = EINVAL;
		goto fail;
	}

	user_end = memchr(buffer, '\n', used);
	if (!user_end || (size_t)(user_end - buffer) <= sizeof("username=") - 1 ||
	    memcmp(buffer, "username=", sizeof("username=") - 1) != 0) {
		errno = EINVAL;
		goto fail;
	}
	pass_start = user_end + 1;
	pass_end = memchr(pass_start, '\n', (size_t)((buffer + used) - pass_start));
	if (!pass_end || pass_end != buffer + used - 1 ||
	    (size_t)(pass_end - pass_start) <= sizeof("password=") - 1 ||
	    memcmp(pass_start, "password=", sizeof("password=") - 1) != 0) {
		errno = EINVAL;
		goto fail;
	}

	user_start = buffer + sizeof("username=") - 1;
	user_length = (size_t)(user_end - user_start);
	pass_start += sizeof("password=") - 1;
	pass_length = (size_t)(pass_end - pass_start);
	if (memchr(user_start, '\r', user_length) ||
	    memchr(pass_start, '\r', pass_length)) {
		errno = EINVAL;
		goto fail;
	}

	user_copy = malloc(user_length + 1);
	pass_copy = malloc(pass_length + 1);
	if (!user_copy || !pass_copy) {
		errno = ENOMEM;
		goto fail;
	}
	memcpy(user_copy, user_start, user_length);
	user_copy[user_length] = '\0';
	memcpy(pass_copy, pass_start, pass_length);
	pass_copy[pass_length] = '\0';

	scutclient_credentials_wipe(buffer, sizeof(buffer));
	*username = user_copy;
	*password = pass_copy;
	return 0;

fail:
	saved_errno = errno ? errno : EINVAL;
	if (fd >= 0)
		close(fd);
	if (user_copy) {
		scutclient_credentials_wipe(user_copy, strlen(user_copy));
		free(user_copy);
	}
	if (pass_copy) {
		scutclient_credentials_wipe(pass_copy, strlen(pass_copy));
		free(pass_copy);
	}
	scutclient_credentials_wipe(buffer, sizeof(buffer));
	errno = saved_errno;
	return -1;
}
