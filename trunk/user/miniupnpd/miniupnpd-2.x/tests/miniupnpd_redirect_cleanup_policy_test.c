#include <assert.h>
#include <stdio.h>

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

int
main(void)
{
	test_cleanup_status_preserves_each_failure();
	puts("MiniUPnP redirect cleanup status policy: PASS");
	return 0;
}
