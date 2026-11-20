#ifndef PADAVAN_AUTH_STARTUP_HANDSHAKE_H
#define PADAVAN_AUTH_STARTUP_HANDSHAKE_H

#include <errno.h>
#include <stddef.h>
#include <sys/types.h>
#include <unistd.h>

#define AUTH_STARTUP_IO_MAX_INTERRUPTS 8U

struct auth_startup_io_ops {
	ssize_t (*read_bytes)(void *context, int fd, void *buffer,
			      size_t length);
	ssize_t (*write_bytes)(void *context, int fd, const void *buffer,
			       size_t length);
};

static inline ssize_t
auth_startup_system_read(void *context, int fd, void *buffer, size_t length)
{
	(void)context;
	return read(fd, buffer, length);
}

static inline ssize_t
auth_startup_system_write(void *context, int fd, const void *buffer,
			  size_t length)
{
	(void)context;
	return write(fd, buffer, length);
}

static inline int
auth_startup_write_result_with(int fd, unsigned char result,
			       const struct auth_startup_io_ops *ops,
			       void *context)
{
	unsigned int interruptions = 0;
	ssize_t count;

	if (fd < 0 || !ops || !ops->write_bytes) {
		errno = EINVAL;
		return -1;
	}

	for (;;) {
		errno = 0;
		count = ops->write_bytes(context, fd, &result, sizeof(result));
		if (count == (ssize_t)sizeof(result))
			return 0;
		if (count < 0 && errno == EINTR) {
			if (interruptions >= AUTH_STARTUP_IO_MAX_INTERRUPTS)
				return -1;
			++interruptions;
			continue;
		}
		if (count == 0 || count > (ssize_t)sizeof(result) || errno == 0)
			errno = EIO;
		return -1;
	}
}

static inline int
auth_startup_read_result_with(int fd, unsigned char *result,
			      const struct auth_startup_io_ops *ops,
			      void *context)
{
	unsigned int interruptions = 0;
	ssize_t count;

	if (fd < 0 || !result || !ops || !ops->read_bytes) {
		errno = EINVAL;
		return -1;
	}

	for (;;) {
		errno = 0;
		count = ops->read_bytes(context, fd, result, sizeof(*result));
		if (count == (ssize_t)sizeof(*result))
			return 0;
		if (count < 0 && errno == EINTR) {
			if (interruptions >= AUTH_STARTUP_IO_MAX_INTERRUPTS)
				return -1;
			++interruptions;
			continue;
		}
		if (count == 0)
			errno = EPIPE;
		else if (count > (ssize_t)sizeof(*result) || errno == 0)
			errno = EIO;
		return -1;
	}
}

static inline int
auth_startup_write_result(int fd, unsigned char result)
{
	static const struct auth_startup_io_ops ops = {
		auth_startup_system_read,
		auth_startup_system_write
	};

	return auth_startup_write_result_with(fd, result, &ops, NULL);
}

static inline int
auth_startup_read_result(int fd, unsigned char *result)
{
	static const struct auth_startup_io_ops ops = {
		auth_startup_system_read,
		auth_startup_system_write
	};

	return auth_startup_read_result_with(fd, result, &ops, NULL);
}

#endif /* PADAVAN_AUTH_STARTUP_HANDSHAKE_H */
