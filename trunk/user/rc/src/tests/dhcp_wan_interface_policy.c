#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "dhcp_wan_interface_policy.h"

struct fake_interface {
	int clear_status;
	int set_old_status;
	int set_new_status;
	char events[16];
	size_t event_count;
};

static void
record_event(struct fake_interface *fake, char event)
{
	assert(fake->event_count + 1 < sizeof(fake->events));
	fake->events[fake->event_count++] = event;
	fake->events[fake->event_count] = '\0';
}

static int
fake_clear(void *opaque)
{
	struct fake_interface *fake = (struct fake_interface *)opaque;
	record_event(fake, 'C');
	return fake->clear_status;
}

static int
fake_set(void *opaque, const char *ipaddr, const char *netmask)
{
	struct fake_interface *fake = (struct fake_interface *)opaque;
	(void)netmask;
	if (strcmp(ipaddr, "192.0.2.10") == 0 &&
	    strcmp(netmask, "255.255.255.0") == 0) {
		record_event(fake, 'O');
		return fake->set_old_status;
	}
	record_event(fake, 'N');
	return fake->set_new_status;
}

static const struct dhcp_wan_interface_ops fake_ops = {
	fake_clear,
	fake_set
};

static void
test_successful_change(void)
{
	struct fake_interface fake = { 0, 0, 0, "", 0 };
	int rollback_status = -1;
	assert(dhcp_wan_interface_apply("192.0.2.10", "255.255.255.0",
		"198.51.100.20", "255.255.255.128", 1,
		&fake_ops, &fake, &rollback_status) == 0);
	assert(rollback_status == 0);
	assert(strcmp(fake.events, "CN") == 0);
}

static void
test_successful_renew_without_clear(void)
{
	struct fake_interface fake = { 0, 0, 0, "", 0 };
	assert(dhcp_wan_interface_apply("192.0.2.10", "255.255.255.0",
		"192.0.2.10", "255.255.255.128", 0,
		&fake_ops, &fake, NULL) == 0);
	assert(strcmp(fake.events, "N") == 0);
}

static void
test_failed_clear_restores_old_address(void)
{
	struct fake_interface fake = { EIO, 0, 0, "", 0 };
	int rollback_status = -1;
	assert(dhcp_wan_interface_apply("192.0.2.10", "255.255.255.0",
		"198.51.100.20", "255.255.255.128", 1,
		&fake_ops, &fake, &rollback_status) == EIO);
	assert(rollback_status == 0);
	assert(strcmp(fake.events, "CO") == 0);
}

static void
test_failed_set_restores_old_address(void)
{
	struct fake_interface fake = { 0, 0, EIO, "", 0 };
	int rollback_status = -1;
	assert(dhcp_wan_interface_apply("192.0.2.10", "255.255.255.0",
		"198.51.100.20", "255.255.255.128", 1,
		&fake_ops, &fake, &rollback_status) == EIO);
	assert(rollback_status == 0);
	assert(strcmp(fake.events, "CNO") == 0);
}

static void
test_failed_set_reports_rollback_failure(void)
{
	struct fake_interface fake = { 0, EACCES, EIO, "", 0 };
	int rollback_status = 0;
	assert(dhcp_wan_interface_apply("192.0.2.10", "255.255.255.0",
		"198.51.100.20", "255.255.255.128", 1,
		&fake_ops, &fake, &rollback_status) == EIO);
	assert(rollback_status == EACCES);
	assert(strcmp(fake.events, "CNO") == 0);
}

static void
test_zero_conf_first_lease_is_applied_transactionally(void)
{
	struct fake_interface fake = { 0, 0, 0, "", 0 };
	int rollback_status = -1;

	assert(dhcp_wan_interface_apply("", "", "169.254.23.45",
		"255.255.0.0", 1, &fake_ops, &fake,
		&rollback_status) == 0);
	assert(rollback_status == 0);
	assert(strcmp(fake.events, "CN") == 0);
}

static void
test_failed_first_lease_is_cleared(void)
{
	struct fake_interface fake = { 0, 0, EIO, "", 0 };
	int rollback_status = -1;
	assert(dhcp_wan_interface_apply("", "", "198.51.100.20",
		"255.255.255.128", 1, &fake_ops, &fake,
		&rollback_status) == EIO);
	assert(rollback_status == 0);
	assert(strcmp(fake.events, "CNC") == 0);
}

static void
test_failed_clear_without_previous_lease_is_reported(void)
{
	struct fake_interface fake = { EIO, 0, 0, "", 0 };
	int rollback_status = 0;
	assert(dhcp_wan_interface_apply("", "", "198.51.100.20",
		"255.255.255.128", 1, &fake_ops, &fake,
		&rollback_status) == EIO);
	assert(rollback_status == EIO);
	assert(strcmp(fake.events, "CC") == 0);
}

static void
test_invalid_input_has_no_side_effects(void)
{
	struct fake_interface fake = { 0, 0, 0, "", 0 };
	assert(dhcp_wan_interface_apply("192.0.2.10", "255.255.255.0",
		"0.0.0.0", "255.255.255.0", 1,
		&fake_ops, &fake, NULL) == EINVAL);
	assert(fake.event_count == 0);
}

int
main(void)
{
	test_successful_change();
	test_successful_renew_without_clear();
	test_failed_clear_restores_old_address();
	test_failed_set_restores_old_address();
	test_failed_set_reports_rollback_failure();
	test_zero_conf_first_lease_is_applied_transactionally();
	test_failed_first_lease_is_cleared();
	test_failed_clear_without_previous_lease_is_reported();
	test_invalid_input_has_no_side_effects();
	puts("WAN DHCP/ZeroConf interface transaction regression: PASS");
	return 0;
}
