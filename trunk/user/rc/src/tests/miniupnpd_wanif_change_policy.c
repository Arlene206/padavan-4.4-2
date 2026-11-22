#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "miniupnpd_wanif_change_policy.h"

static void
test_backend_error_never_requests_stop(void)
{
	int should_stop = 1;

	assert(miniupnpd_wanif_change_decide(EIO, "ppp0", "eth2.2",
					    &should_stop) == EIO);
	assert(should_stop == 0);
}

static void
test_missing_or_invalid_identity_never_requests_stop(void)
{
	int should_stop = 1;

	assert(miniupnpd_wanif_change_decide(0, "ppp0", "", &should_stop) == EINVAL);
	assert(should_stop == 0);
	assert(miniupnpd_wanif_change_decide(0, "", "ppp0", &should_stop) == EINVAL);
	assert(should_stop == 0);
	assert(miniupnpd_wanif_change_decide(0, NULL, "ppp0", &should_stop) == EINVAL);
	assert(should_stop == 0);
}

static void
test_only_verified_difference_requests_stop(void)
{
	int should_stop = 1;

	assert(miniupnpd_wanif_change_decide(0, "ppp0", "ppp0", &should_stop) == 0);
	assert(should_stop == 0);
	assert(miniupnpd_wanif_change_decide(0, "ppp0", "eth2.2", &should_stop) == 0);
	assert(should_stop == 1);
}

static void
test_missing_output_is_rejected(void)
{
	assert(miniupnpd_wanif_change_decide(0, "ppp0", "eth2.2", NULL) == EINVAL);
}

int
main(void)
{
	test_backend_error_never_requests_stop();
	test_missing_or_invalid_identity_never_requests_stop();
	test_only_verified_difference_requests_stop();
	test_missing_output_is_rejected();
	puts("MiniUPnP WAN identity decision fails closed on unavailable inputs: PASS");
	return 0;
}
