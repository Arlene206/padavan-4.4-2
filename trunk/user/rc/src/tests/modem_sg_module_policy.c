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
test_zerocd_success_is_accepted(void)
{
	assert(modem_sg_zerocd_command_status(0) == 0);
}

static void
test_zerocd_failure_is_reported(void)
{
	assert(modem_sg_zerocd_command_status(1) == -1);
	assert(modem_sg_zerocd_command_status(127) == -1);
	assert(modem_sg_zerocd_command_status(-1) == -1);
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
	test_zerocd_success_is_accepted();
	test_zerocd_failure_is_reported();

	puts("USB SCSI generic/ZeroCD command status policy: PASS");
	return 0;
}
