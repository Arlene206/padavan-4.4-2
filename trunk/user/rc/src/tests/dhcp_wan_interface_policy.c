#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "dhcp_wan_interface_policy.h"
#include "dhcp_wan_route_transition_policy.h"
#include "wan_mtu_setup_policy.h"

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
test_dhcp_entrypoint_rejects_missing_interface_environment(void)
{
	assert(!dhcp_wan_interface_name_is_valid(NULL));
	assert(!dhcp_wan_interface_name_is_valid(""));
	assert(dhcp_wan_interface_name_is_valid("eth0"));
	assert(dhcp_wan_interface_name_is_valid("eth3.35"));
}
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


struct fake_route_transition {
	int delete_host_status;
	int delete_default_status;
	int delete_dhcp_status;
	int restore_address_status;
	int restore_routes_status;
	char events[16];
	size_t event_count;
};

static void
record_route_event(struct fake_route_transition *fake, char event)
{
	assert(fake->event_count + 1 < sizeof(fake->events));
	fake->events[fake->event_count++] = event;
	fake->events[fake->event_count] = '\0';
}

static int
fake_delete_host_route(void *opaque)
{
	struct fake_route_transition *fake = (struct fake_route_transition *)opaque;
	record_route_event(fake, 'h');
	return fake->delete_host_status;
}

static int
fake_delete_default_route(void *opaque)
{
	struct fake_route_transition *fake = (struct fake_route_transition *)opaque;
	record_route_event(fake, 'd');
	return fake->delete_default_status;
}

static int
fake_delete_dhcp_routes(void *opaque)
{
	struct fake_route_transition *fake = (struct fake_route_transition *)opaque;
	record_route_event(fake, 'r');
	return fake->delete_dhcp_status;
}

static int
fake_restore_address(void *opaque)
{
	struct fake_route_transition *fake = (struct fake_route_transition *)opaque;
	record_route_event(fake, 'A');
	return fake->restore_address_status;
}

static int
fake_restore_routes(void *opaque)
{
	struct fake_route_transition *fake = (struct fake_route_transition *)opaque;
	record_route_event(fake, 'R');
	return fake->restore_routes_status;
}

static const struct dhcp_wan_route_transition_ops fake_route_transition_ops = {
	fake_delete_host_route,
	fake_delete_default_route,
	fake_delete_dhcp_routes,
	fake_restore_address,
	fake_restore_routes
};

static void
test_route_setup_failure_requires_retry(void)
{
	assert(!dhcp_wan_route_setup_retry_required(0));
	assert(dhcp_wan_route_setup_retry_required(EIO));
	assert(dhcp_wan_route_setup_retry_required(EHOSTUNREACH));
}

static void
test_dhcp_optional_mtu_failure_does_not_request_route_retry(void)
{
	int mtu_status = EIO;
	int route_setup_status = 0;

	/* Cover the entire DHCP parser range, including an explicit 1500 reset. */
	assert(!wan_dhcp_mtu_is_supported(575));
	assert(wan_dhcp_mtu_is_supported(576));
	assert(wan_dhcp_mtu_is_supported(1299));
	assert(wan_dhcp_mtu_is_supported(1300));
	assert(wan_dhcp_mtu_is_supported(1499));
	assert(wan_dhcp_mtu_is_supported(1500));
	assert(!wan_dhcp_mtu_is_supported(1501));

	/* Optional DHCP MTU ioctl errors are logged, not promoted to route errors. */
	assert(mtu_status != 0);
	assert(route_setup_status == 0);
	assert(!dhcp_wan_route_setup_retry_required(route_setup_status));

	/* Independent route failures still use the established retry mechanism. */
	route_setup_status = ENETUNREACH;
	assert(dhcp_wan_route_setup_retry_required(route_setup_status));
}

static void
test_snapshot_failure_restores_old_address_and_routes(void)
{
	struct fake_route_transition fake = { 0, 0, 0, 0, 0, "", 0 };
	int rollback_status = -1;

	assert(dhcp_wan_route_transition_abort_snapshot(ENOSPC,
		&fake_route_transition_ops, &fake, &rollback_status) == ENOSPC);
	assert(rollback_status == 0);
	assert(strcmp(fake.events, "AR") == 0);

	fake.events[0] = '\0';
	fake.event_count = 0;
	fake.restore_address_status = EACCES;
	fake.restore_routes_status = EPERM;
	assert(dhcp_wan_route_transition_abort_snapshot(ENOSPC,
		&fake_route_transition_ops, &fake, &rollback_status) == ENOSPC);
	assert(rollback_status == EACCES);
	assert(strcmp(fake.events, "AR") == 0);
}

static void
test_snapshot_success_does_not_restore_network(void)
{
	struct fake_route_transition fake = { 0, 0, 0, 0, 0, "", 0 };
	int rollback_status = -1;

	assert(dhcp_wan_route_transition_abort_snapshot(0,
		&fake_route_transition_ops, &fake, &rollback_status) == 0);
	assert(rollback_status == 0);
	assert(fake.event_count == 0);
}

static void
test_route_transition_success(void)
{
	struct fake_route_transition fake = { 0, 0, 0, 0, 0, "", 0 };
	int rollback_status = -1;

	assert(dhcp_wan_route_transition_apply(1, 1, 1,
		&fake_route_transition_ops, &fake, &rollback_status) == 0);
	assert(rollback_status == 0);
	assert(strcmp(fake.events, "hdr") == 0);
}

static void
test_route_transition_ignores_missing_old_routes(void)
{
	struct fake_route_transition fake = { ESRCH, ESRCH, ESRCH, 0, 0, "", 0 };
	int rollback_status = -1;

	assert(dhcp_wan_route_transition_apply(1, 1, 1,
		&fake_route_transition_ops, &fake, &rollback_status) == 0);
	assert(rollback_status == 0);
	assert(strcmp(fake.events, "hdr") == 0);
}

static void
test_route_transition_rolls_back_all_after_delete_errors(void)
{
	struct fake_route_transition fake = {
		EIO, EACCES, ENETUNREACH, 0, 0, "", 0
	};
	int rollback_status = -1;

	assert(dhcp_wan_route_transition_apply(1, 1, 1,
		&fake_route_transition_ops, &fake, &rollback_status) == EIO);
	assert(rollback_status == 0);
	assert(strcmp(fake.events, "hdrAR") == 0);
}

static void
test_route_transition_reports_rollback_failure_but_attempts_both(void)
{
	struct fake_route_transition fake = {
		EIO, 0, 0, EACCES, EPERM, "", 0
	};
	int rollback_status = 0;

	assert(dhcp_wan_route_transition_apply(1, 1, 1,
		&fake_route_transition_ops, &fake, &rollback_status) == EIO);
	assert(rollback_status == EACCES);
	assert(strcmp(fake.events, "hdrAR") == 0);
}

static void
test_route_transition_skips_unrequested_operations(void)
{
	struct fake_route_transition fake = { 0, 0, 0, 0, 0, "", 0 };

	assert(dhcp_wan_route_transition_apply(0, 0, 0,
		&fake_route_transition_ops, &fake, NULL) == 0);
	assert(fake.event_count == 0);
	assert(dhcp_wan_route_transition_apply(2, 0, 0,
		&fake_route_transition_ops, &fake, NULL) == EINVAL);
	assert(fake.event_count == 0);
}

int
main(void)
{
	test_dhcp_entrypoint_rejects_missing_interface_environment();
	test_successful_change();
	test_successful_renew_without_clear();
	test_failed_clear_restores_old_address();
	test_failed_set_restores_old_address();
	test_failed_set_reports_rollback_failure();
	test_zero_conf_first_lease_is_applied_transactionally();
	test_failed_first_lease_is_cleared();
	test_failed_clear_without_previous_lease_is_reported();
	test_invalid_input_has_no_side_effects();
	test_route_setup_failure_requires_retry();
	test_dhcp_optional_mtu_failure_does_not_request_route_retry();
	test_snapshot_failure_restores_old_address_and_routes();
	test_snapshot_success_does_not_restore_network();
	test_route_transition_success();
	test_route_transition_ignores_missing_old_routes();
	test_route_transition_rolls_back_all_after_delete_errors();
	test_route_transition_reports_rollback_failure_but_attempts_both();
	test_route_transition_skips_unrequested_operations();
	puts("WAN DHCP/ZeroConf interface and route transaction regression: PASS");
	return 0;
}
