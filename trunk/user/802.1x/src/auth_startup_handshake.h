#ifndef PADAVAN_AUTH_STARTUP_HANDSHAKE_H
#define PADAVAN_AUTH_STARTUP_HANDSHAKE_H

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

#define AUTH_STARTUP_IO_MAX_INTERRUPTS 8U

/*
 * wireless extensions store interface names in IFNAMSIZ-byte fields.
 * Callers supply a prefix and an AP index; never write a truncated or
 * unterminated name into struct iwreq.
 */
static inline int
auth_startup_format_interface_name(char *name, size_t name_size,
                                   const char *prefix, unsigned int apidx)
{
	int length;

	if (!name || name_size == 0 || !prefix || !*prefix) {
		errno = EINVAL;
		return -1;
	}

	length = snprintf(name, name_size, "%s%u", prefix, apidx);
	if (length < 0) {
		name[0] = '\0';
		errno = EIO;
		return -1;
	}
	if ((size_t)length >= name_size) {
		name[0] = '\0';
		errno = ENAMETOOLONG;
		return -1;
	}

	return 0;
}


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
