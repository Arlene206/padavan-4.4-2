#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "ppp_wan_ipup_policy.h"

struct fake_ipup {
	int configure_status;
	char events[8];
	char ipaddr[64];
	char netmask[64];
};

static void
record_event(struct fake_ipup *fake, char event)
{
	size_t used = strlen(fake->events);

	assert(used + 1 < sizeof(fake->events));
	fake->events[used] = event;
	fake->events[used + 1] = '\0';
}

static int
fake_set_interface_address(void *opaque, const char *ipaddr,
			   const char *netmask)
{
	struct fake_ipup *fake = (struct fake_ipup *)opaque;

	(void)ipaddr;
	(void)netmask;
	record_event(fake, 'C');
	return fake->configure_status;
}

static void
fake_publish_address(void *opaque, const char *ipaddr, const char *netmask)
{
	struct fake_ipup *fake = (struct fake_ipup *)opaque;

	record_event(fake, 'P');
	snprintf(fake->ipaddr, sizeof(fake->ipaddr), "%s", ipaddr);
	snprintf(fake->netmask, sizeof(fake->netmask), "%s", netmask);
}

static const struct ppp_wan_ipup_ops fake_ops = {
	fake_set_interface_address,
	fake_publish_address
};

static void
test_success_applies_before_publishing(void)
{
	struct fake_ipup fake = { 0, "", "", "" };

	assert(ppp_wan_ipup_apply_address("198.51.100.2", "255.255.255.255",
					  &fake_ops, &fake) == 0);
	assert(strcmp(fake.events, "CP") == 0);
	assert(strcmp(fake.ipaddr, "198.51.100.2") == 0);
	assert(strcmp(fake.netmask, "255.255.255.255") == 0);
}

static void
test_interface_failure_preserves_published_state(void)
{
	struct fake_ipup fake = { EIO, "", "192.0.2.9", "255.255.255.255" };

	assert(ppp_wan_ipup_apply_address("198.51.100.2", "255.255.255.255",
					  &fake_ops, &fake) == EIO);
	assert(strcmp(fake.events, "C") == 0);
	assert(strcmp(fake.ipaddr, "192.0.2.9") == 0);
	assert(strcmp(fake.netmask, "255.255.255.255") == 0);
}

static void
test_invalid_input_has_no_side_effects(void)
{
	struct fake_ipup fake = { 0, "", "", "" };

	assert(ppp_wan_ipup_apply_address("", "255.255.255.255",
					  &fake_ops, &fake) == EINVAL);
	assert(fake.events[0] == '\0');
	assert(fake.ipaddr[0] == '\0');
	assert(fake.netmask[0] == '\0');
}

int
main(void)
{
	test_success_applies_before_publishing();
	test_interface_failure_preserves_published_state();
	test_invalid_input_has_no_side_effects();
	puts("PPP ip-up address publication regression: PASS");
	return 0;
}
