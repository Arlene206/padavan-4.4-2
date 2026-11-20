#ifndef MINIUPNPD_PIDFILE_POLICY_H
#define MINIUPNPD_PIDFILE_POLICY_H
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define MINIUPNPD_PIDFILE_MAX_INTERRUPTS 8U
typedef ssize_t (*miniupnpd_pidfile_write_fn)(int fd, const void *buffer,
                                               size_t length, void *context);
typedef ssize_t (*miniupnpd_pidfile_read_fn)(int fd, void *buffer,
                                              size_t length, void *context);
static inline ssize_t
miniupnpd_pidfile_write_real(int fd, const void *buffer, size_t length,
                             void *context)
{
	(void)context;
	return write(fd, buffer, length);
}
static inline ssize_t
miniupnpd_pidfile_read_real(int fd, void *buffer, size_t length, void *context)
{
	(void)context;
	return read(fd, buffer, length);
}
static inline int
miniupnpd_pidfile_write_all_with(int fd, const void *buffer, size_t length,
                                 miniupnpd_pidfile_write_fn write_fn,
                                 void *context)
{
	const char *cursor = (const char *)buffer;
	size_t remaining = length;
	unsigned int interruptions = 0;
	if (fd < 0 || (!buffer && length != 0) || !write_fn) {
		errno = EINVAL;
		return -1;
	}
	while (remaining > 0) {
		ssize_t written = write_fn(fd, cursor, remaining, context);
		if (written < 0) {
			if (errno == EINTR) {
				if (interruptions >= MINIUPNPD_PIDFILE_MAX_INTERRUPTS) {
					errno = EINTR;
					return -1;
				}
				++interruptions;
				continue;
			}
			return -1;
		}
		if (written == 0 || (size_t)written > remaining) {
			errno = EIO;
			return -1;
		}
		cursor += written;
		remaining -= (size_t)written;
		interruptions = 0;
	}
	return 0;
}
static inline int
miniupnpd_pidfile_read_all_with(int fd, char *buffer, size_t capacity,
                                size_t *length,
                                miniupnpd_pidfile_read_fn read_fn,
                                void *context)
{
	size_t used = 0;
	unsigned int interruptions = 0;
	if (fd < 0 || !buffer || capacity < 2 || !length || !read_fn) {
		errno = EINVAL;
		return -1;
	}
	for (;;) {
		ssize_t count;
		if (used == capacity - 1) {
			errno = EOVERFLOW;
			return -1;
		}
		count = read_fn(fd, buffer + used, capacity - used - 1, context);
		if (count < 0) {
			if (errno == EINTR) {
				if (interruptions >= MINIUPNPD_PIDFILE_MAX_INTERRUPTS) {
					errno = EINTR;
					return -1;
				}
				++interruptions;
				continue;
			}
			return -1;
		}
		if (count == 0)
			break;
		if ((size_t)count > capacity - used - 1) {
			errno = EIO;
			return -1;
		}
		used += (size_t)count;
		interruptions = 0;
	}
	buffer[used] = '\0';
	*length = used;
	return 0;
}

static inline int
miniupnpd_pidfile_parse_pid(const char *text, size_t length, pid_t *pid_out)
{
	unsigned long value = 0;
	size_t i;
	pid_t pid;
	if (!text || !pid_out || length < 2 || text[length - 1] != '\n') {
		errno = EINVAL;
		return -1;
	}
	for (i = 0; i + 1 < length; ++i) {
		unsigned int digit;
		if (text[i] < '0' || text[i] > '9') {
			errno = EINVAL;
			return -1;
		}
		digit = (unsigned int)(text[i] - '0');
		if (value > (ULONG_MAX - digit) / 10UL) {
			errno = ERANGE;
			return -1;
		}
		value = value * 10UL + digit;
	}
	if (value == 0) {
		errno = ERANGE;
		return -1;
	}
	pid = (pid_t)value;
	if (pid <= 0 || (unsigned long)pid != value) {
		errno = ERANGE;
		return -1;
	}
	*pid_out = pid;
	return 0;
}

/* Keep a stable inode locked for the process lifetime; never unlink this file. */
static inline int
miniupnpd_pidfile_lock_acquire(const char *pidfile_path)
{
	static const char suffix[] = ".lock";
	char *lock_path;
	size_t path_length;
	int fd;
	int saved_errno;
	struct stat st;
	struct flock lock;
	unsigned int interruptions = 0;
	if (!pidfile_path || pidfile_path[0] == '\0') {
		errno = EINVAL;
		return -1;
	}
	path_length = strlen(pidfile_path);
	if (path_length > (size_t)-1 - sizeof(suffix)) {
		errno = ENAMETOOLONG;
		return -1;
	}
	lock_path = (char *)malloc(path_length + sizeof(suffix));
	if (!lock_path) {
		errno = ENOMEM;
		return -1;
	}
	memcpy(lock_path, pidfile_path, path_length);
	memcpy(lock_path + path_length, suffix, sizeof(suffix));
	fd = open(lock_path, O_RDWR | O_CREAT | O_NOFOLLOW | O_CLOEXEC,
	          S_IRUSR | S_IWUSR);
	saved_errno = errno;
	free(lock_path);
	errno = saved_errno;
	if (fd < 0)
		return -1;
	if (fstat(fd, &st) != 0)
		goto failed;
	if (!S_ISREG(st.st_mode) || st.st_uid != geteuid() || st.st_nlink != 1 ||
	    (st.st_mode & 077) != 0) {
		errno = EPERM;
		goto failed;
	}
	if (fchmod(fd, S_IRUSR | S_IWUSR) != 0)
		goto failed;
	lock.l_type = F_WRLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0;
	lock.l_len = 0;
	for (;;) {
		if (fcntl(fd, F_SETLK, &lock) == 0)
			return fd;
		if (errno == EINTR) {
			if (interruptions >= MINIUPNPD_PIDFILE_MAX_INTERRUPTS) {
				errno = EINTR;
				goto failed;
			}
			++interruptions;
			continue;
		}
		if (errno == EACCES || errno == EAGAIN)
			errno = EBUSY;
		goto failed;
	}
failed:
	saved_errno = errno ? errno : EIO;
	(void)close(fd);
	errno = saved_errno;
	return -1;
}

static inline int
miniupnpd_pidfile_sync(int fd)
{
	unsigned int interruptions = 0;
	while (fsync(fd) != 0) {
		if (errno != EINTR)
			return -1;
		if (interruptions >= MINIUPNPD_PIDFILE_MAX_INTERRUPTS) {
			errno = EINTR;
			return -1;
		}
		++interruptions;
	}
	return 0;
}
/* Publish beside the destination: failed writes never truncate its last
 * complete PID, and rename replaces a destination symlink without following it. */
static inline int
miniupnpd_pidfile_publish_with(const char *path, const void *contents,
                               size_t length,
                               miniupnpd_pidfile_write_fn write_fn,
                               void *context)
{
	static const char suffix[] = ".tmp.XXXXXX";
	char *temporary_path = NULL;
	size_t path_length;
	int fd = -1;
	int saved_errno;
	mode_t old_umask;
	if (!path || path[0] == '\0' || !contents || length == 0 || !write_fn) {
		errno = EINVAL;
		return -1;
	}
	path_length = strlen(path);
	if (path_length > (size_t)-1 - sizeof(suffix)) {
		errno = ENAMETOOLONG;
		return -1;
	}
	temporary_path = (char *)malloc(path_length + sizeof(suffix));
	if (!temporary_path) {
		errno = ENOMEM;
		return -1;
	}
	memcpy(temporary_path, path, path_length);
	memcpy(temporary_path + path_length, suffix, sizeof(suffix));
	fd = mkstemp(temporary_path);
	if (fd < 0)
		goto failed;
	/* Match open(..., 0644)'s umask-filtered mode; mkstemp starts at 0600. */
	old_umask = umask(0);
	(void)umask(old_umask);
	if (fchmod(fd, (mode_t)(0644 & ~old_umask)) != 0)
		goto failed;
	if (miniupnpd_pidfile_write_all_with(fd, contents, length,
	                                     write_fn, context) != 0)
		goto failed;
	if (miniupnpd_pidfile_sync(fd) != 0)
		goto failed;
	if (close(fd) != 0) {
		fd = -1;
		goto failed;
	}
	fd = -1;
	if (rename(temporary_path, path) != 0)
		goto failed;
	free(temporary_path);
	return 0;
failed:
	saved_errno = errno ? errno : EIO;
	if (fd >= 0)
		(void)close(fd);
	if (temporary_path)
		(void)unlink(temporary_path);
	free(temporary_path);
	errno = saved_errno;
	return -1;
}
static inline int
miniupnpd_pidfile_publish(const char *path, const void *contents, size_t length)
{
	return miniupnpd_pidfile_publish_with(path, contents, length,
	                                      miniupnpd_pidfile_write_real, NULL);
}
#endif /* MINIUPNPD_PIDFILE_POLICY_H */
