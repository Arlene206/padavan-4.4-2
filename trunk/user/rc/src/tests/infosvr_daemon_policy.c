#include <assert.h>
#include <stdio.h>

#include "infosvr_daemon_policy.h"

static void test_failed_stop_never_starts_replacement(void)
{
	assert(!infosvr_restart_can_start(-1, 1));
	assert(!infosvr_restart_can_start(-1, 0));
}

static void test_remaining_process_blocks_replacement(void)
{
	assert(!infosvr_restart_can_start(0, 1));
}

static void test_confirmed_stop_allows_start(void)
{
	assert(infosvr_restart_can_start(0, 0));
}

int main(void)
{
	test_failed_stop_never_starts_replacement();
	test_remaining_process_blocks_replacement();
	test_confirmed_stop_allows_start();
	puts("infosvr stop-before-start daemon policy: PASS");
	return 0;
}
