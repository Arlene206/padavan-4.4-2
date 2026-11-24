#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "infosvr_io_policy.h"

int
main(void)
{
	assert(infosvr_io_error_retryable(EINTR));
	assert(infosvr_io_error_retryable(EAGAIN));
	assert(infosvr_io_error_retryable(EWOULDBLOCK));
	assert(!infosvr_io_error_retryable(EIO));
	assert(!infosvr_io_error_retryable(EBADF));
	assert(!infosvr_io_error_retryable(EINVAL));
	assert(!infosvr_io_error_retryable(0));

	puts("infosvr socket I/O retry policy: PASS");
	return 0;
}
