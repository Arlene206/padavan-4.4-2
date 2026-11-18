#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "modem_sg_module_policy.h"

static void
test_loaded_module_is_ready(void)
{
	assert(modem_sg_module_ready_status(1) == 0);
}

static void
test_absent_module_is_not_ready(void)
{
	assert(modem_sg_module_ready_status(0) == -1);
}

static void
test_unknown_module_state_fails_closed(void)
{
	assert(modem_sg_module_ready_status(-EIO) == -1);
}

int
main(void)
{
	test_loaded_module_is_ready();
	test_absent_module_is_not_ready();
	test_unknown_module_state_fails_closed();

	puts("USB SCSI generic module readiness policy: PASS");
	return 0;
}
