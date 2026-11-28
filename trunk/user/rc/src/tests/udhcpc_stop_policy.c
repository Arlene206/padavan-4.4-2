#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "udhcpc_stop_policy.h"

static void
test_successful_release_waits_for_deconfig(void)
{
	assert(udhcpc_stop_release_wait_required(0));
	assert(udhcpc_stop_release_status(0, 0, 0) == 0);
	assert(udhcpc_stop_release_status(0, 0, EIO) == EIO);
}

static void
test_missing_or_exited_client_does_not_block_stop(void)
{
	assert(!udhcpc_stop_release_wait_required(-1));
	assert(udhcpc_stop_release_status(-1, ENOENT, 0) == 0);
	assert(udhcpc_stop_release_status(-1, ESRCH, 0) == 0);
}

static void
test_unknown_pid_identity_fails_closed(void)
{
	assert(udhcpc_stop_release_status(-1, EACCES, 0) == -1);
	assert(udhcpc_stop_release_status(-1, EIO, 0) == -1);
	assert(udhcpc_stop_release_status(-1, 0, 0) == -1);
}

int
main(void)
{
	test_successful_release_waits_for_deconfig();
	test_missing_or_exited_client_does_not_block_stop();
	test_unknown_pid_identity_fails_closed();
	puts("WAN/VIPTV udhcpc release-before-stop policy: PASS");
	return 0;
}
