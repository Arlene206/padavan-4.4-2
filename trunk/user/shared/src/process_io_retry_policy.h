#ifndef PADAVAN_PROCESS_IO_RETRY_POLICY_H
#define PADAVAN_PROCESS_IO_RETRY_POLICY_H

#include <errno.h>
#include <stddef.h>
#include <sys/types.h>
#include <unistd.h>

/* Bound repeated signal interruptions while reading procfs snapshots. */
#define PROCESS_IO_MAX_INTERRUPTS 8U

static inline int
process_io_retry_interrupted(unsigned int *interruptions)
{
	if (!interruptions) {
		errno = EINVAL;
		return -1;
	}
	if (*interruptions >= PROCESS_IO_MAX_INTERRUPTS) {
		errno = EINTR;
		return -1;
	}
	++*interruptions;
	return 0;
}

typedef ssize_t (*process_io_read_callback)(int fd, void *buffer,
                                                size_t length, void *context);
typedef ssize_t (*process_io_write_callback)(int fd, const void *buffer,
                                             size_t length, void *context);

static inline ssize_t
process_io_read_with_retry(int fd, void *buffer, size_t length,
                           process_io_read_callback callback, void *context)
{
	unsigned int interruptions = 0;

	if (!callback || (!buffer && length != 0)) {
		errno = EINVAL;
		return -1;
	}
	for (;;) {
		ssize_t count = callback(fd, buffer, length, context);

		if (count >= 0)
			return count;
		if (errno != EINTR ||
		    process_io_retry_interrupted(&interruptions) != 0)
			return -1;
	}
}

static inline ssize_t
process_io_write_with_retry(int fd, const void *buffer, size_t length,
                            process_io_write_callback callback, void *context)
{
	unsigned int interruptions = 0;

	if (!callback || (!buffer && length != 0)) {
		errno = EINVAL;
		return -1;
	}
	for (;;) {
		ssize_t count = callback(fd, buffer, length, context);

		if (count >= 0)
			return count;
		if (errno != EINTR ||
		    process_io_retry_interrupted(&interruptions) != 0)
			return -1;
	}
}

static inline ssize_t
process_io_system_read(int fd, void *buffer, size_t length, void *context)
{
	(void)context;
	return read(fd, buffer, length);
}

static inline ssize_t
process_io_system_write(int fd, const void *buffer, size_t length,
                        void *context)
{
	(void)context;
	return write(fd, buffer, length);
}

static inline ssize_t
process_io_read(int fd, void *buffer, size_t length)
{
	return process_io_read_with_retry(fd, buffer, length,
					  process_io_system_read, NULL);
}

static inline ssize_t
process_io_write(int fd, const void *buffer, size_t length)
{
	return process_io_write_with_retry(fd, buffer, length,
					   process_io_system_write, NULL);
}

#endif /* PADAVAN_PROCESS_IO_RETRY_POLICY_H */
