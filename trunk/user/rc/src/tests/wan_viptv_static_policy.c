#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "wan_viptv_static_policy.h"

struct fake_start {
	int address_status;
	int route_status;
	char events[16];
	int address_published;
	int gateway_published;
	int multicast_started;
};

static void
record_event(struct fake_start *fake, char event)
{
	size_t length = strlen(fake->events);

	assert(length + 1 < sizeof(fake->events));
	fake->events[length] = event;
	fake->events[length + 1] = '\0';
}

static int
fake_configure_address(void *opaque)
{
	struct fake_start *fake = (struct fake_start *)opaque;

	record_event(fake, 'A');
	return fake->address_status;
}

static void
fake_publish_address(void *opaque)
{
	struct fake_start *fake = (struct fake_start *)opaque;

	fake->address_published = 1;
	record_event(fake, 'P');
}

static void
fake_publish_gateway(void *opaque)
{
	struct fake_start *fake = (struct fake_start *)opaque;

	fake->gateway_published = 1;
	record_event(fake, 'G');
}

static int
fake_add_default_route(void *opaque)
{
	struct fake_start *fake = (struct fake_start *)opaque;

	record_event(fake, 'R');
	return fake->route_status;
}

static void
fake_start_multicast(void *opaque)
{
	struct fake_start *fake = (struct fake_start *)opaque;

	fake->multicast_started = 1;
	record_event(fake, 'M');
}

static const struct wan_viptv_static_ops fake_ops = {
	fake_configure_address,
	fake_publish_address,
	fake_publish_gateway,
	fake_add_default_route,
	fake_start_multicast
};

static void
test_address_failure_does_not_publish_or_start_services(void)
{
	struct fake_start fake = { EIO, 0, "", 0, 0, 0 };

	assert(wan_viptv_static_start(1, &fake_ops, &fake) == EIO);
	assert(strcmp(fake.events, "A") == 0);
	assert(!fake.address_published);
	assert(!fake.gateway_published);
	assert(!fake.multicast_started);
}

static void
test_successful_start_orders_address_state_route_and_multicast(void)
{
	struct fake_start fake = { 0, 0, "", 0, 0, 0 };

	assert(wan_viptv_static_start(1, &fake_ops, &fake) == 0);
	assert(strcmp(fake.events, "APGRM") == 0);
	assert(fake.address_published);
	assert(fake.gateway_published);
	assert(fake.multicast_started);
}

static void
test_route_failure_is_reported_after_recovery_state_publication(void)
{
	struct fake_start fake = { 0, EIO, "", 0, 0, 0 };

	assert(wan_viptv_static_start(1, &fake_ops, &fake) == EIO);
	assert(strcmp(fake.events, "APGRM") == 0);
	assert(fake.address_published);
	assert(fake.gateway_published);
	assert(fake.multicast_started);
}

static void
test_missing_gateway_skips_default_route(void)
{
	struct fake_start fake = { 0, 0, "", 0, 0, 0 };

	assert(wan_viptv_static_start(0, &fake_ops, &fake) == 0);
	assert(strcmp(fake.events, "APGM") == 0);
	assert(fake.address_published);
	assert(fake.gateway_published);
	assert(fake.multicast_started);
}

static void
test_invalid_arguments_fail_before_side_effects(void)
{
	struct fake_start fake = { 0, 0, "", 0, 0, 0 };
	struct wan_viptv_static_ops missing_route_ops = {
		fake_configure_address,
		fake_publish_address,
		fake_publish_gateway,
		NULL,
		fake_start_multicast
	};

	assert(wan_viptv_static_start(2, &fake_ops, &fake) == EINVAL);
	assert(fake.events[0] == '\0');
	assert(wan_viptv_static_start(1, &missing_route_ops, &fake) == EINVAL);
	assert(fake.events[0] == '\0');
}

int
main(void)
{
	test_address_failure_does_not_publish_or_start_services();
	test_successful_start_orders_address_state_route_and_multicast();
	test_route_failure_is_reported_after_recovery_state_publication();
	test_missing_gateway_skips_default_route();
	test_invalid_arguments_fail_before_side_effects();
	puts("Static IPTV startup status regression: PASS");
	return 0;
}
