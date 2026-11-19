#ifndef PADAVAN_CREATE_FILE_POLICY_H
#define PADAVAN_CREATE_FILE_POLICY_H
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
typedef int (*create_file_open_fn)(const char *path, int flags, mode_t mode);
typedef int (*create_file_close_fn)(int fd);
/*
 * Treat the operation as successful only after its descriptor closes. This
 * lets callers fail closed on delayed filesystem errors as well as open()
 * failures, while keeping errno meaningful for either failure.
 */
static inline int
create_file_with_ops(const char *path, create_file_open_fn open_fn,
		     create_file_close_fn close_fn)
{
	int fd, saved_errno;
	if (!path || !path[0] || !open_fn || !close_fn) {
		errno = EINVAL;
		return 1;
	}
	errno = 0;
	fd = open_fn(path, O_RDWR | O_CREAT, 0666);
	if (fd < 0) {
		errno = errno ? errno : EIO;
		return 1;
	}
	/* Do not let an unrelated prior errno mask a close() failure. */
	errno = 0;
	if (close_fn(fd) != 0) {
		saved_errno = errno ? errno : EIO;
		errno = saved_errno;
		return 1;
	}
	return 0;
}
#endif /* PADAVAN_CREATE_FILE_POLICY_H */
