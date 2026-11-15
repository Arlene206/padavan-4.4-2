#include <assert.h>
#include <stdio.h>

#include "service_start_status.h"

int
main(void)
{
	assert(rc_first_service_start_status(0, 0) == 0);
	assert(rc_first_service_start_status(0, -1) == -1);
	assert(rc_first_service_start_status(0, 17) == 17);
	assert(rc_first_service_start_status(-1, 0) == -1);
	assert(rc_first_service_start_status(17, -1) == 17);
	assert(rc_first_service_start_status(-5, 9) == -5);
	assert(rc_first_service_start_status(4, 0) == 4);
	puts("RC boot service first-failure aggregation policy: PASS");
	return 0;
}
