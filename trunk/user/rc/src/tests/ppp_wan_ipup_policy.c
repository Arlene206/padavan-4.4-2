#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "ppp_wan_ipup_policy.h"

struct fake_ipup {
	int configure_status;
	int publish_status;
	int rollback_status;
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

static int
fake_publish_address(void *opaque, const char *ipaddr, const char *netmask)
{
	struct fake_ipup *fake = (struct fake_ipup *)opaque;

	record_event(fake, 'P');
	if (fake->publish_status != 0)
		return fake->publish_status;
	snprintf(fake->ipaddr, sizeof(fake->ipaddr), "%s", ipaddr);
	snprintf(fake->netmask, sizeof(fake->netmask), "%s", netmask);
	return 0;
}

static int
fake_clear_interface_address(void *opaque)
{
	struct fake_ipup *fake = (struct fake_ipup *)opaque;

	record_event(fake, 'R');
	return fake->rollback_status;
}

static const struct ppp_wan_ipup_ops fake_ops = {
	fake_set_interface_address,
	fake_publish_address,
	fake_clear_interface_address
};

static void
test_success_applies_before_publishing(void)
{
	struct fake_ipup fake = { 0, 0, 0, "", "", "" };

	assert(ppp_wan_ipup_apply_address("198.51.100.2", "198.51.100.1",
					  "255.255.255.255", &fake_ops, &fake) == 0);
	assert(strcmp(fake.events, "CP") == 0);
	assert(strcmp(fake.ipaddr, "198.51.100.2") == 0);
	assert(strcmp(fake.netmask, "255.255.255.255") == 0);
}

static void
test_interface_failure_preserves_published_state(void)
{
	struct fake_ipup fake = { EIO, 0, 0, "", "192.0.2.9", "255.255.255.255" };

	assert(ppp_wan_ipup_apply_address("198.51.100.2", "198.51.100.1",
					  "255.255.255.255", &fake_ops, &fake) == EIO);
	assert(strcmp(fake.events, "C") == 0);
	assert(strcmp(fake.ipaddr, "192.0.2.9") == 0);
	assert(strcmp(fake.netmask, "255.255.255.255") == 0);
}


static void
test_publication_failure_rolls_back_interface_and_preserves_primary_error(void)
{
	struct fake_ipup fake = {
		0, EACCES, 0, "", "192.0.2.9", "255.255.255.255"
	};

	assert(ppp_wan_ipup_apply_address("198.51.100.2", "198.51.100.1",
					  "255.255.255.255", &fake_ops, &fake) == EACCES);
	assert(strcmp(fake.events, "CPR") == 0);
	assert(strcmp(fake.ipaddr, "192.0.2.9") == 0);
	assert(strcmp(fake.netmask, "255.255.255.255") == 0);
}

static void
test_rollback_failure_does_not_hide_publication_error(void)
{
	struct fake_ipup fake = {
		0, ENOSPC, EIO, "", "192.0.2.9", "255.255.255.255"
	};

	assert(ppp_wan_ipup_apply_address("198.51.100.2", "198.51.100.1",
					  "255.255.255.255", &fake_ops, &fake) == ENOSPC);
	assert(strcmp(fake.events, "CPR") == 0);
}

static void
test_missing_ipcp_addresses_are_rejected(void)
{
	struct fake_ipup fake = { 0, 0, 0, "", "", "" };

	assert(ppp_wan_ipup_apply_address(NULL, "198.51.100.1",
					  "255.255.255.255", &fake_ops, &fake) == EINVAL);
	assert(ppp_wan_ipup_apply_address("198.51.100.2", NULL,
					  "255.255.255.255", &fake_ops, &fake) == EINVAL);
	assert(ppp_wan_ipup_apply_address("198.51.100.2", "",
					  "255.255.255.255", &fake_ops, &fake) == EINVAL);
	assert(ppp_wan_ipup_apply_address("", "198.51.100.1",
					  "255.255.255.255", &fake_ops, &fake) == EINVAL);
	assert(ppp_wan_ipup_apply_address("198.51.100.2", "198.51.100.1",
					  NULL, &fake_ops, &fake) == EINVAL);
	assert(fake.events[0] == '\0');
	assert(fake.ipaddr[0] == '\0');
	assert(fake.netmask[0] == '\0');
}

static void
test_invalid_ipcp_values_have_no_side_effects(void)
{
	struct fake_ipup fake = { 0, 0, 0, "", "", "" };

	assert(ppp_wan_ipup_apply_address("not-an-ip", "198.51.100.1",
					  "255.255.255.255", &fake_ops, &fake) == EINVAL);
	assert(ppp_wan_ipup_apply_address("0.0.0.0", "198.51.100.1",
					  "255.255.255.255", &fake_ops, &fake) == EINVAL);
	assert(ppp_wan_ipup_apply_address("198.51.100.2", "not-an-ip",
					  "255.255.255.255", &fake_ops, &fake) == EINVAL);
	assert(ppp_wan_ipup_apply_address("198.51.100.2", "0.0.0.0",
					  "255.255.255.255", &fake_ops, &fake) == EINVAL);
	assert(ppp_wan_ipup_apply_address("198.51.100.2", "198.51.100.1",
					  "255.0.255.0", &fake_ops, &fake) == EINVAL);
	assert(ppp_wan_ipup_apply_address("198.051.100.2", "198.51.100.1",
					  "255.255.255.255", &fake_ops, &fake) == EINVAL);
	assert(fake.events[0] == '\0');
	assert(fake.ipaddr[0] == '\0');
	assert(fake.netmask[0] == '\0');
}

static void
test_route_setup_error_is_returned_after_unlock(void)
{
	assert(ppp_wan_ipup_finish_status(0, 0) == 0);
	assert(ppp_wan_ipup_finish_status(EIO, 0) == EIO);
	assert(ppp_wan_ipup_finish_status(0, -1) == -1);
	assert(ppp_wan_ipup_finish_status(EIO, -1) == -1);
}

int
main(void)
{
	test_success_applies_before_publishing();
	test_interface_failure_preserves_published_state();
	test_publication_failure_rolls_back_interface_and_preserves_primary_error();
	test_rollback_failure_does_not_hide_publication_error();
	test_missing_ipcp_addresses_are_rejected();
	test_invalid_ipcp_values_have_no_side_effects();
	test_route_setup_error_is_returned_after_unlock();
	puts("PPP ip-up address and route status regression: PASS");
	return 0;
}
