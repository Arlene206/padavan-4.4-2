#include <assert.h>
#include <stddef.h>

#include "miniupnpd_event_subscription_policy.h"

static void
test_subscriber_capacity_boundaries(void)
{
	assert(miniupnpd_event_subscriber_has_capacity(0));
	assert(miniupnpd_event_subscriber_has_capacity(1));
	assert(miniupnpd_event_subscriber_has_capacity(
		MINIUPNPD_EVENT_MAX_SUBSCRIBERS - 1U));
	assert(!miniupnpd_event_subscriber_has_capacity(
		MINIUPNPD_EVENT_MAX_SUBSCRIBERS));
	assert(!miniupnpd_event_subscriber_has_capacity(
		MINIUPNPD_EVENT_MAX_SUBSCRIBERS + 1U));
}

int
main(void)
{
	test_subscriber_capacity_boundaries();
	return 0;
}
