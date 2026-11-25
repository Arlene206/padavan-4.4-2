#ifndef INFOSVR_IO_POLICY_H
#define INFOSVR_IO_POLICY_H

#include <errno.h>

/* Socket timeouts and interrupted syscalls are transient; other I/O errors are fatal. */
static inline int
infosvr_io_error_retryable(int error_number)
{
	return error_number == EINTR ||
	       error_number == EAGAIN ||
	       error_number == EWOULDBLOCK;
}

#endif /* INFOSVR_IO_POLICY_H */
