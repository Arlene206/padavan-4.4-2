#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "../nvram_ioctl_close_policy.h"

static void test_success_requires_successful_close(void)
{
	int saved_errno = -1;

	assert(nvram_ioctl_close_status(0, 0, 0, 0, &saved_errno) == 0);
	assert(saved_errno == 0);
	assert(nvram_ioctl_close_status(3, 0, 0, 0, &saved_errno) == 3);
	assert(saved_errno == 0);
	assert(nvram_ioctl_close_status(0, 0, -1, EIO, &saved_errno) == -1);
	assert(saved_errno == EIO);
}

static void test_ioctl_error_takes_precedence_over_close_error(void)
{
	int saved_errno = -1;

	assert(nvram_ioctl_close_status(-1, EFAULT, -1, EIO,
					&saved_errno) == -1);
	assert(saved_errno == EFAULT);
}

static void test_missing_error_values_fail_closed(void)
{
	int saved_errno = -1;

	assert(nvram_ioctl_close_status(-1, 0, 0, 0, &saved_errno) == -1);
	assert(saved_errno == EIO);
	assert(nvram_ioctl_close_status(0, 0, -1, 0, &saved_errno) == -1);
	assert(saved_errno == EIO);
	errno = 0;
	assert(nvram_ioctl_close_status(0, 0, 0, 0, NULL) == -1);
	assert(errno == EINVAL);
}

int main(void)
{
	test_success_requires_successful_close();
	test_ioctl_error_takes_precedence_over_close_error();
	test_missing_error_values_fail_closed();
	puts("NVRAM ioctl/close error precedence regression: PASS");
	return 0;
}
