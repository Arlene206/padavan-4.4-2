#include <assert.h>
#include <stdio.h>
#include "../miniupnpd_redirect_cleanup_policy.h"

#include "miniupnpd_redirect_cleanup_policy.h"

static void
test_cleanup_status_preserves_each_failure(void)
{
	assert(miniupnpd_merge_cleanup_status(0, 0) == 0);
	assert(miniupnpd_merge_cleanup_status(-1, 0) == -1);
	assert(miniupnpd_merge_cleanup_status(0, -1) == -1);
	assert(miniupnpd_merge_cleanup_status(-1, -1) == -1);
	assert(miniupnpd_merge_cleanup_status(7, 0) == 7);
}

static void
test_primary_redirect_removal_is_independent_of_peer_cleanup(void)
{
	assert(miniupnpd_primary_redirect_removed(0, 0, -1, -1, 0, 0) == 0);
	assert(miniupnpd_primary_redirect_removed(1, 0, -1, -1, 0, 0) == 1);
	assert(miniupnpd_primary_redirect_removed(1, 1, -1, -1, 0, 0) == 0);
	assert(miniupnpd_primary_redirect_removed(1, 1, 0, 0, 0, 0) == 1);
	assert(miniupnpd_primary_redirect_removed(1, 1, 0, -1, 1, 0) == 0);
	assert(miniupnpd_primary_redirect_removed(1, 1, 0, -1, 1, -1) == 1);
	assert(miniupnpd_primary_redirect_removed(1, 1, 0, -1, 0, 0) == 0);
}


int
main(void)
{
	test_cleanup_status_preserves_each_failure();
	test_primary_redirect_removal_is_independent_of_peer_cleanup();
	puts("MiniUPnP redirect cleanup status policy: PASS");
	return 0;
}
