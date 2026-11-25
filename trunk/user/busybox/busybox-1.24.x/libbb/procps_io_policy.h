#ifndef BB_PROCPS_IO_POLICY_H
#define BB_PROCPS_IO_POLICY_H

#include <errno.h>
#include <stddef.h>
#include <sys/types.h>
#include <unistd.h>

/* Avoid dropping a process snapshot on a transient signal interruption. */
#define PROCPS_READ_MAX_INTERRUPTS 8U

typedef ssize_t (*procps_read_callback_t)(int fd, void *buffer,
                                          size_t length, void *context);

static inline ssize_t
procps_read_with_retry_cb(int fd, void *buffer, size_t length,
                          procps_read_callback_t callback, void *context)
{
	unsigned int interruptions = 0;

	if (!callback || (!buffer && length != 0)) {
		errno = EINVAL;
		return -1;
	}

	for (;;) {
		ssize_t result = callback(fd, buffer, length, context);

		if (result >= 0)
			return result;
		if (errno != EINTR ||
		    interruptions >= PROCPS_READ_MAX_INTERRUPTS)
			return -1;
		++interruptions;
	}
}

static inline ssize_t
procps_system_read(int fd, void *buffer, size_t length, void *context)
{
	(void)context;
	return read(fd, buffer, length);
}

static inline ssize_t
procps_read_with_retry(int fd, void *buffer, size_t length)
{
	return procps_read_with_retry_cb(fd, buffer, length,
	                                 procps_system_read, NULL);
}

#endif /* BB_PROCPS_IO_POLICY_H */
