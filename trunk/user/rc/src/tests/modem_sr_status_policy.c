#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "modem_sr_status_policy.h"

static void
test_successful_unload(void)
{
	assert(modem_sr_completion_status(0, 1, 1) == 0);
}

static void
test_in_use_module_is_a_safe_noop(void)
{
	assert(modem_sr_completion_status(0, 1, 1) == 0);
}

static void
test_already_absent_module_is_a_safe_noop(void)
{
	assert(modem_sr_completion_status(0, 0, 0) == 0);
}

static void
test_present_module_after_unload_failure_is_rejected(void)
{
	assert(modem_sr_completion_status(0, 0, 1) == -1);
}

static void
test_unknown_module_presence_fails_closed(void)
{
	assert(modem_sr_completion_status(0, 0, -EIO) == -1);
}

static void
test_eject_failure_is_reported_even_when_unload_is_safe(void)
{
	assert(modem_sr_completion_status(256, 1, 1) == -1);
	assert(modem_sr_completion_status(-1, 0, 0) == -1);
}

int
main(void)
{
	test_successful_unload();
	test_in_use_module_is_a_safe_noop();
	test_already_absent_module_is_a_safe_noop();
	test_present_module_after_unload_failure_is_rejected();
	test_unknown_module_presence_fails_closed();
	test_eject_failure_is_reported_even_when_unload_is_safe();

	puts("USB SCSI CD eject/unload result policy: PASS");
	return 0;
}
