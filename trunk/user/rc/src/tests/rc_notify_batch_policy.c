#include <assert.h>

#include "rc_notify_batch_policy.h"

int
main(void)
{
	assert(!rc_notify_batch_should_requeue(0, 1, 0));
	assert(!rc_notify_batch_should_requeue(9, 1, 0));
	assert(!rc_notify_batch_should_requeue(10, 0, 0));
	assert(rc_notify_batch_should_requeue(10, 1, 0));
	assert(rc_notify_batch_should_requeue(11, 1, 0));
	assert(!rc_notify_batch_should_requeue(10, 1, 1));
	assert(!rc_notify_batch_should_requeue(100, 1, 1));
	return 0;
}
