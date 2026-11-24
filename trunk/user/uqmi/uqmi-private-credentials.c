#define _GNU_SOURCE
#include "uqmi-private-credentials.h"

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef O_NOFOLLOW
#error "uqmi private credentials require O_NOFOLLOW support"
#endif
#ifndef O_DIRECTORY
#error "uqmi private credentials require O_DIRECTORY support"
#endif
#ifndef AT_SYMLINK_NOFOLLOW
#error "uqmi private credentials require AT_SYMLINK_NOFOLLOW support"
#endif
#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif

static void
uqmi_private_credentials_wipe(void *memory, size_t length)
{
	volatile unsigned char *cursor = (volatile unsigned char *)memory;

	while (length-- > 0)
		*cursor++ = 0;
}

void
uqmi_private_credentials_clear(struct uqmi_private_credentials *credentials)
{
	if (credentials)
		uqmi_private_credentials_wipe(credentials, sizeof(*credentials));
}

static int
uqmi_private_credentials_parse(const char *buffer, size_t length,
			       struct uqmi_private_credentials *credentials)
{
	struct uqmi_private_credentials parsed;
	size_t offset = 0;

	memset(&parsed, 0, sizeof(parsed));
	if (!buffer || !credentials || length == 0 ||
	    buffer[length - 1] != '\n' || memchr(buffer, '\0', length)) {
		errno = EINVAL;
		return -1;
	}

	while (offset < length) {
		const char *line = buffer + offset;
		const char *line_end = memchr(line, '\n', length - offset);
		const char *value;
		size_t line_length, value_length;

		if (!line_end) {
			errno = EINVAL;
			goto fail;
		}
		line_length = (size_t)(line_end - line);
		if (line_length == 0 || memchr(line, '\r', line_length)) {
			errno = EINVAL;
			goto fail;
		}

		if (line_length >= sizeof("pin=") - 1 &&
		    memcmp(line, "pin=", sizeof("pin=") - 1) == 0) {
			if (parsed.has_pin) {
				errno = EINVAL;
				goto fail;
			}
			value = line + sizeof("pin=") - 1;
			value_length = line_length - (sizeof("pin=") - 1);
			if (value_length == 0 || value_length > UQMI_PRIVATE_PIN_SIZE) {
				errno = EINVAL;
				goto fail;
			}
			memcpy(parsed.pin, value, value_length);
			parsed.pin[value_length] = '\0';
			parsed.has_pin = 1;
		} else if (line_length >= sizeof("username=") - 1 &&
			   memcmp(line, "username=", sizeof("username=") - 1) == 0) {
			if (parsed.has_username) {
				errno = EINVAL;
				goto fail;
			}
			value = line + sizeof("username=") - 1;
			value_length = line_length - (sizeof("username=") - 1);
			if (value_length > UQMI_PRIVATE_VALUE_SIZE) {
				errno = EINVAL;
				goto fail;
			}
			memcpy(parsed.username, value, value_length);
			parsed.username[value_length] = '\0';
			parsed.has_username = 1;
		} else if (line_length >= sizeof("password=") - 1 &&
			   memcmp(line, "password=", sizeof("password=") - 1) == 0) {
			if (parsed.has_password) {
				errno = EINVAL;
				goto fail;
			}
			value = line + sizeof("password=") - 1;
			value_length = line_length - (sizeof("password=") - 1);
			if (value_length > UQMI_PRIVATE_VALUE_SIZE) {
				errno = EINVAL;
				goto fail;
			}
			memcpy(parsed.password, value, value_length);
			parsed.password[value_length] = '\0';
			parsed.has_password = 1;
		} else {
			errno = EINVAL;
			goto fail;
		}
		offset += line_length + 1;
	}

	if (parsed.has_username != parsed.has_password ||
	    (!parsed.has_pin && !parsed.has_username) ||
	    (parsed.has_pin && parsed.has_username)) {
		errno = EINVAL;
		goto fail;
	}
	*credentials = parsed;
	uqmi_private_credentials_wipe(&parsed, sizeof(parsed));
	return 0;

fail:
	uqmi_private_credentials_wipe(&parsed, sizeof(parsed));
	return -1;
}

int
uqmi_private_credentials_load(const char *path,
			      struct uqmi_private_credentials *credentials)
{
	int fd = -1, dirfd = -1, saved_errno;
	struct stat directory_stat, opened_stat, path_stat;
	char *path_copy = NULL, *directory_path = NULL, *basename = NULL;
	char *slash;
	char buffer[UQMI_PRIVATE_CREDENTIALS_MAX + 1];
	size_t used = 0;
	ssize_t count;
	struct uqmi_private_credentials parsed;

	memset(buffer, 0, sizeof(buffer));
	memset(&parsed, 0, sizeof(parsed));
	if (credentials)
		uqmi_private_credentials_clear(credentials);
	if (!path || !path[0] || !credentials) {
		errno = EINVAL;
		goto fail;
	}

	path_copy = strdup(path);
	if (!path_copy) {
		errno = ENOMEM;
		goto fail;
	}
	slash = strrchr(path_copy, '/');
	if (!slash) {
		directory_path = strdup(".");
		basename = strdup(path_copy);
	} else {
		basename = strdup(slash + 1);
		if (slash == path_copy)
			directory_path = strdup("/");
		else {
			*slash = '\0';
			directory_path = strdup(path_copy);
		}
	}
	if (!directory_path || !basename) {
		errno = ENOMEM;
		goto fail;
	}
	if (!basename[0] || strcmp(basename, ".") == 0 ||
	    strcmp(basename, "..") == 0) {
		errno = EINVAL;
		goto fail;
	}

	dirfd = open(directory_path, O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
	if (dirfd < 0)
		goto fail;
	if (fstat(dirfd, &directory_stat) < 0)
		goto fail;
	if (!S_ISDIR(directory_stat.st_mode) ||
	    directory_stat.st_uid != geteuid() ||
	    (directory_stat.st_mode & 07777) != 0700) {
		errno = EPERM;
		goto fail;
	}

	fd = openat(dirfd, basename,
		    O_RDONLY | O_NOFOLLOW | O_CLOEXEC | O_NONBLOCK);
	if (fd < 0)
		goto fail;
	if (fstat(fd, &opened_stat) < 0)
		goto fail;
	if (!S_ISREG(opened_stat.st_mode) ||
	    opened_stat.st_uid != geteuid() ||
	    (opened_stat.st_mode & 07777) != 0600 ||
	    opened_stat.st_nlink != 1 ||
	    opened_stat.st_size < 1 ||
	    opened_stat.st_size > UQMI_PRIVATE_CREDENTIALS_MAX) {
		errno = EPERM;
		goto fail;
	}

	if (fstatat(dirfd, basename, &path_stat, AT_SYMLINK_NOFOLLOW) < 0)
		goto fail;
	if (!S_ISREG(path_stat.st_mode) ||
	    path_stat.st_dev != opened_stat.st_dev ||
	    path_stat.st_ino != opened_stat.st_ino) {
		errno = EPERM;
		goto fail;
	}
	if (unlinkat(dirfd, basename, 0) < 0)
		goto fail;
	if (close(dirfd) < 0) {
		dirfd = -1;
		goto fail;
	}
	dirfd = -1;

	/* Consume the directory entry before reading any credential bytes. */
	while (used < sizeof(buffer)) {
		count = read(fd, buffer + used, sizeof(buffer) - used);
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

	if (uqmi_private_credentials_parse(buffer, used, &parsed) < 0)
		goto fail;
	*credentials = parsed;
	uqmi_private_credentials_clear(&parsed);
	uqmi_private_credentials_wipe(buffer, sizeof(buffer));
	free(basename);
	free(directory_path);
	free(path_copy);
	return 0;

fail:
	saved_errno = errno ? errno : EINVAL;
	if (fd >= 0)
		close(fd);
	if (dirfd >= 0)
		close(dirfd);
	uqmi_private_credentials_clear(&parsed);
	if (credentials)
		uqmi_private_credentials_clear(credentials);
	uqmi_private_credentials_wipe(buffer, sizeof(buffer));
	free(basename);
	free(directory_path);
	free(path_copy);
	errno = saved_errno;
	return -1;
}
