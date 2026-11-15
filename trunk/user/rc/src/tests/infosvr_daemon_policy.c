#include <assert.h>
#include <stdio.h>

#include "infosvr_daemon_policy.h"

static void test_failed_stop_never_starts_replacement(void)
{
	assert(!infosvr_restart_can_start(-1, PROCESS_PRESENCE_RUNNING));
	assert(!infosvr_restart_can_start(-1, PROCESS_PRESENCE_ABSENT));
	assert(!infosvr_restart_can_start(-1, PROCESS_PRESENCE_UNKNOWN));
}

static void test_remaining_process_blocks_replacement(void)
{
	assert(!infosvr_restart_can_start(0, PROCESS_PRESENCE_RUNNING));
}

static void test_unknown_inventory_blocks_replacement(void)
{
	assert(!infosvr_restart_can_start(0, PROCESS_PRESENCE_UNKNOWN));
	assert(!infosvr_restart_can_start(0, (enum process_presence)9));
}

static void test_confirmed_absence_allows_start(void)
{
	assert(infosvr_restart_can_start(0, PROCESS_PRESENCE_ABSENT));
}

int main(void)
{
	test_failed_stop_never_starts_replacement();
	test_remaining_process_blocks_replacement();
	test_unknown_inventory_blocks_replacement();
	test_confirmed_absence_allows_start();
	puts("infosvr tri-state stop-before-start daemon policy: PASS");
	return 0;
}
