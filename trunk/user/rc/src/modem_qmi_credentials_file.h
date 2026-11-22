#ifndef RC_MODEM_QMI_CREDENTIALS_FILE_H
#define RC_MODEM_QMI_CREDENTIALS_FILE_H

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef O_NOFOLLOW
#error "modem QMI credentials require O_NOFOLLOW support"
#endif
#ifndef O_DIRECTORY
#error "modem QMI credentials require O_DIRECTORY support"
#endif
#ifndef AT_SYMLINK_NOFOLLOW
#error "modem QMI credentials require AT_SYMLINK_NOFOLLOW support"
#endif
#ifndef O_CLOEXEC
#define O_CLOEXEC 0
#endif

#define MODEM_QMI_CREDENTIALS_FILE_NAME "credentials"

struct modem_qmi_credentials_file {
	char directory[PATH_MAX];
	char path[PATH_MAX];
};

static inline void
modem_qmi_credentials_wipe(void *memory, size_t length)
{
	volatile unsigned char *cursor = (volatile unsigned char *)memory;

	while (length-- > 0)
		*cursor++ = 0;
}

static inline int
modem_qmi_credentials_value_valid(const char *value, size_t maximum)
{
	size_t length;

	if (!value)
		return 1;
	length = strlen(value);
	if (length > maximum || strchr(value, '\n') || strchr(value, '\r'))
		return 0;
	return 1;
}

static inline int
modem_qmi_credentials_write_all(int fd, const char *buffer, size_t length)
{
	size_t offset = 0;
	ssize_t count;

	while (offset < length) {
		count = write(fd, buffer + offset, length - offset);
		if (count < 0) {
			if (errno == EINTR)
				continue;
			return -1;
		}
		if (count == 0) {
			errno = EIO;
			return -1;
		}
		offset += (size_t)count;
	}
	return 0;
}

/*
 * Create a one-shot QMI credential file in a private, newly-created /tmp
 * directory. The caller passes only file.path to uqmi, never the secret data.
 */
static inline int
modem_qmi_credentials_file_create(const char *pin, const char *username,
				  const char *password,
				  struct modem_qmi_credentials_file *file)
{
	char directory_template[] = "/tmp/uqmi-auth.XXXXXX";
	char contents[768];
	struct stat directory_stat, file_stat;
	int dirfd = -1, fd = -1, saved_errno;
	int length;
	size_t used = 0;

	if (!file || (!pin && !username && !password) ||
	    (!!username != !!password) ||
	    (pin && pin[0] && username) ||
	    !modem_qmi_credentials_value_valid(pin, 64) ||
	    !modem_qmi_credentials_value_valid(username, 256) ||
	    !modem_qmi_credentials_value_valid(password, 256)) {
		errno = EINVAL;
		return -1;
	}
	memset(file, 0, sizeof(*file));
	memset(contents, 0, sizeof(contents));

	if (pin && pin[0]) {
		length = snprintf(contents + used, sizeof(contents) - used,
				  "pin=%s\n", pin);
		if (length < 0 || (size_t)length >= sizeof(contents) - used) {
			errno = EOVERFLOW;
			goto fail;
		}
		used += (size_t)length;
	}
	if (username && password) {
		length = snprintf(contents + used, sizeof(contents) - used,
				  "username=%s\npassword=%s\n",
				  username, password);
		if (length < 0 || (size_t)length >= sizeof(contents) - used) {
			errno = EOVERFLOW;
			goto fail;
		}
		used += (size_t)length;
	}
	if (used == 0) {
		errno = EINVAL;
		goto fail;
	}

	if (!mkdtemp(directory_template))
		goto fail;
	length = snprintf(file->directory, sizeof(file->directory), "%s",
			  directory_template);
	if (length < 0 || (size_t)length >= sizeof(file->directory)) {
		errno = ENAMETOOLONG;
		goto fail;
	}

	dirfd = open(file->directory,
		     O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
	if (dirfd < 0)
		goto fail;
	if (fstat(dirfd, &directory_stat) < 0)
		goto fail;
	if (directory_stat.st_uid != geteuid() ||
	    !S_ISDIR(directory_stat.st_mode)) {
		errno = EPERM;
		goto fail;
	}
	if (fchmod(dirfd, 0700) < 0)
		goto fail;
	if (fstat(dirfd, &directory_stat) < 0)
		goto fail;
	if ((directory_stat.st_mode & 07777) != 0700) {
		errno = EPERM;
		goto fail;
	}

	fd = openat(dirfd, MODEM_QMI_CREDENTIALS_FILE_NAME,
		    O_WRONLY | O_CREAT | O_EXCL | O_NOFOLLOW | O_CLOEXEC, 0600);
	if (fd < 0)
		goto fail;
	if (fchmod(fd, 0600) < 0 || fstat(fd, &file_stat) < 0)
		goto fail;
	if (!S_ISREG(file_stat.st_mode) ||
	    file_stat.st_uid != geteuid() ||
	    (file_stat.st_mode & 07777) != 0600 ||
	    file_stat.st_nlink != 1) {
		errno = EPERM;
		goto fail;
	}
	if (modem_qmi_credentials_write_all(fd, contents, used) < 0)
		goto fail;
	if (fsync(fd) < 0)
		goto fail;
	if (close(fd) < 0) {
		fd = -1;
		goto fail;
	}
	fd = -1;
	if (close(dirfd) < 0) {
		dirfd = -1;
		goto fail;
	}
	dirfd = -1;

	length = snprintf(file->path, sizeof(file->path), "%s/%s",
			  file->directory, MODEM_QMI_CREDENTIALS_FILE_NAME);
	if (length < 0 || (size_t)length >= sizeof(file->path)) {
		errno = ENAMETOOLONG;
		goto fail;
	}
	modem_qmi_credentials_wipe(contents, sizeof(contents));
	return 0;

fail:
	saved_errno = errno ? errno : EIO;
	if (fd >= 0)
		close(fd);
	if (dirfd >= 0) {
		unlinkat(dirfd, MODEM_QMI_CREDENTIALS_FILE_NAME, 0);
		close(dirfd);
	}
	if (file->directory[0])
		rmdir(file->directory);
	memset(file, 0, sizeof(*file));
	modem_qmi_credentials_wipe(contents, sizeof(contents));
	errno = saved_errno;
	return -1;
}

/*
 * Remove any unconsumed credential file and then its private directory.
 * Refuse to follow or remove an unexpected replacement inode.
 */
static inline int
modem_qmi_credentials_file_cleanup(struct modem_qmi_credentials_file *file)
{
	struct stat directory_stat, file_stat;
	int dirfd, status = 0;

	if (!file || !file->directory[0]) {
		errno = EINVAL;
		return EINVAL;
	}
	dirfd = open(file->directory,
		     O_RDONLY | O_DIRECTORY | O_NOFOLLOW | O_CLOEXEC);
	if (dirfd < 0) {
		status = errno == ENOENT ? 0 : (errno ? errno : EIO);
		goto done;
	}
	if (fstat(dirfd, &directory_stat) < 0)
		status = errno ? errno : EIO;
	else if (!S_ISDIR(directory_stat.st_mode) ||
		 directory_stat.st_uid != geteuid() ||
		 (directory_stat.st_mode & 07777) != 0700)
		status = EPERM;

	if (status == 0) {
		if (fstatat(dirfd, MODEM_QMI_CREDENTIALS_FILE_NAME,
			    &file_stat, AT_SYMLINK_NOFOLLOW) == 0) {
			if (!S_ISREG(file_stat.st_mode) ||
			    file_stat.st_uid != geteuid() ||
			    (file_stat.st_mode & 07777) != 0600 ||
			    file_stat.st_nlink != 1)
				status = EPERM;
			else if (unlinkat(dirfd, MODEM_QMI_CREDENTIALS_FILE_NAME, 0) < 0)
				status = errno ? errno : EIO;
		} else if (errno != ENOENT) {
			status = errno ? errno : EIO;
		}
	}
	if (close(dirfd) < 0 && status == 0)
		status = errno ? errno : EIO;
	if (status == 0 && rmdir(file->directory) < 0 && errno != ENOENT)
		status = errno ? errno : EIO;

done:
	if (status == 0)
		memset(file, 0, sizeof(*file));
	else
		errno = status;
	return status;
}

#endif
