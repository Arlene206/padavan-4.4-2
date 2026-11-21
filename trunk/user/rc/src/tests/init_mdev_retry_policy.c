#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include "../init_mdev_retry_policy.h"
int
main(void)
{
	init_mdev_retry_state_t state = { 0 };
	assert(!init_mdev_retry_state_is_pending(&state));
	init_mdev_retry_state_begin(&state, 0, 0);
	assert(!init_mdev_retry_state_is_pending(&state));
	init_mdev_retry_state_begin(&state, 0, -EIO);
	assert(init_mdev_retry_state_is_pending(&state));
	assert(init_mdev_retry_state_record_result(&state, -EAGAIN));
	assert(init_mdev_retry_state_is_pending(&state));
	assert(!init_mdev_retry_state_record_result(&state, 0));
	assert(!init_mdev_retry_state_is_pending(&state));
	assert(!init_mdev_retry_state_record_result(&state, -EIO));
	init_mdev_retry_state_begin(&state, 1, -EIO);
	assert(!init_mdev_retry_state_is_pending(&state));
	init_mdev_retry_state_begin(&state, -1, -EIO);
	assert(!init_mdev_retry_state_is_pending(&state));
	init_mdev_retry_state_begin(&state, 0, 1);
	assert(init_mdev_retry_state_is_pending(&state));
	assert(INIT_MDEV_RETRY_DELAY_SECONDS == 5);
	init_mdev_retry_state_begin(NULL, 0, -EIO);
	assert(!init_mdev_retry_state_is_pending(NULL));
	assert(!init_mdev_retry_state_record_result(NULL, -EIO));
	puts("PID 1 deferred mdev hotplug recovery policy: PASS");
	return 0;
}
