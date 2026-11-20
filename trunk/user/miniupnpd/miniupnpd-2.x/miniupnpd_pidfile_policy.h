#ifndef MINIUPNPD_PIDFILE_POLICY_H
#define MINIUPNPD_PIDFILE_POLICY_H
#include <errno.h>
#include <fcntl.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define MINIUPNPD_PIDFILE_MAX_INTERRUPTS 8U
typedef ssize_t (*miniupnpd_pidfile_write_fn)(int fd, const void *buffer,
                                               size_t length, void *context);
static inline ssize_t
miniupnpd_pidfile_write_real(int fd, const void *buffer, size_t length,
                             void *context)
{
	(void)context;
	return write(fd, buffer, length);
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
	if (fchmod(fd, 0644) != 0)
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
