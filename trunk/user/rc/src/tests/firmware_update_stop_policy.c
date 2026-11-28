#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "firmware_update_stop_policy.h"

static void
test_success_does_not_capture_stale_errno(void)
{
	assert(firmware_update_stop_first_errno(0, EACCES, 0) == 0);
}

static void
test_failure_uses_operation_errno(void)
{
	assert(firmware_update_stop_first_errno(7, EBUSY, 0) == EBUSY);
	assert(firmware_update_stop_first_errno(-1, EIO, 0) == EIO);
}

static void
test_failure_without_errno_falls_back_to_eio(void)
{
	assert(firmware_update_stop_first_errno(1, 0, 0) == EIO);
}

static void
test_first_failure_is_sticky(void)
{
	int first_errno = 0;

	first_errno = firmware_update_stop_first_errno(9, EBUSY, first_errno);
	first_errno = firmware_update_stop_first_errno(5, EACCES, first_errno);
	first_errno = firmware_update_stop_first_errno(0, 0, first_errno);
	assert(first_errno == EBUSY);
}

int
main(void)
{
	test_success_does_not_capture_stale_errno();
	test_failure_uses_operation_errno();
	test_failure_without_errno_falls_back_to_eio();
	test_first_failure_is_sticky();
	puts("Firmware-update stop sequence preserves the first errno: PASS");
	return 0;
}
