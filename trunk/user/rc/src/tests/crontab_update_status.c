#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>

#include "crontab_update_status.h"

static void
test_system_wait_status_decode(void)
{
	assert(rc_crontab_update_status_from_wait(0, 0) == 0);
	assert(rc_crontab_update_status_from_wait(1 << 8, 0) == 1);
	assert(rc_crontab_update_status_from_wait(2 << 8, 0) == 2);
	assert(rc_crontab_update_status_from_wait(127 << 8, 0) == 127);
	assert(rc_crontab_update_status_from_wait(-1, EACCES) == EACCES);
	assert(rc_crontab_update_status_from_wait(-1, 0) == -1);
	assert(rc_crontab_update_status_from_wait(-2, 0) == EIO);
	assert(rc_crontab_update_status_from_wait(SIGTERM, 0) == EIO);
}

static void
test_changed_result_and_error_merge(void)
{
	assert(rc_crontab_update_status_merge(0, 0) == 0);
	assert(rc_crontab_update_status_merge(0, 1) == 1);
	assert(rc_crontab_update_status_merge(1, 0) == 1);
	assert(rc_crontab_update_status_merge(1, 1) == 1);
	assert(rc_crontab_update_status_merge(1, 2) == 2);
	assert(rc_crontab_update_status_merge(2, 1) == 2);
	assert(rc_crontab_update_status_merge(-1, 1) == -1);
	assert(rc_crontab_update_status_merge(0, EIO) == EIO);
	assert(rc_crontab_update_status_merge(EBUSY, EIO) == EBUSY);
}

static void
test_only_successful_append_requires_persistence(void)
{
	assert(!rc_crontab_update_requires_persistence(0));
	assert(rc_crontab_update_requires_persistence(1));
	assert(!rc_crontab_update_requires_persistence(2));
	assert(!rc_crontab_update_requires_persistence(EIO));
	assert(!rc_crontab_update_requires_persistence(-1));
}

int
main(void)
{
	test_system_wait_status_decode();
	test_changed_result_and_error_merge();
	test_only_successful_append_requires_persistence();
	puts("RC crontab wait-status decoding and persistence policy: PASS");
	return 0;
}
