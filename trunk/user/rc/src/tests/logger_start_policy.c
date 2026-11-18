#include <assert.h>
#include <stdio.h>

#include "logger_start_policy.h"

int
main(void)
{
	assert(rc_logger_start_status(0, 0) == 0);
	assert(rc_logger_start_status(17, 0) == 17);
	assert(rc_logger_start_status(0, 23) == 23);
	assert(rc_logger_start_status(17, 23) == 17);
	assert(rc_logger_start_status(-1, 0) == -1);
	assert(rc_logger_start_status(0, -1) == -1);
	assert(rc_logger_start_status_with_wait(0, 0, 0) == 0);
	assert(rc_logger_start_status_with_wait(17, 23, 29) == 17);
	assert(rc_logger_start_status_with_wait(0, 23, 29) == 23);
	assert(rc_logger_start_status_with_wait(0, 0, 29) == 29);
	assert(rc_logger_start_status_with_wait(-1, 23, 29) == -1);
	assert(rc_logger_start_status_with_wait(0, -1, 29) == -1);
	puts("RC logger startup and EINTR-safe settling wait first-failure policy: PASS");
	return 0;
}
