#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "storage_hotplug_notify_policy.h"

static void
test_success_requires_both_operation_and_notification(void)
{
	assert(storage_hotplug_completion_status(0, 0) == 0);
	assert(storage_hotplug_completion_status(1, 0) == 1);
	assert(storage_hotplug_completion_status(-1, 0) == 1);
	assert(storage_hotplug_completion_status(0, -1) == 1);
	assert(storage_hotplug_completion_status(0, 2) == 1);
	assert(storage_hotplug_completion_status(1, -1) == 1);
}

static void
test_notification_errors_never_become_success(void)
{
	assert(storage_hotplug_completion_status(0, -EIO) == 1);
	assert(storage_hotplug_completion_status(0, -EACCES) == 1);
}

int
main(void)
{
	test_success_requires_both_operation_and_notification();
	test_notification_errors_never_become_success();
	puts("storage hotplug notification status policy: PASS");
	return 0;
}
