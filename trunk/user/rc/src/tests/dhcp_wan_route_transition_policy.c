#include <assert.h>
#include <errno.h>

#include "dhcp_wan_route_transition_policy.h"


static void
test_bound_callback_status_ignores_auxiliary_service_failures(void)
{
	/* Zero means route and MTU application succeeded; ancillary statuses
	 * are intentionally not arguments to this policy. */
	assert(dhcp_bound_callback_status(0, 0) == 0);
	assert(dhcp_bound_callback_status(EIO, 0) == EIO);
	assert(dhcp_bound_callback_status(0, EBUSY) == EBUSY);
	assert(dhcp_bound_callback_status(EIO, EBUSY) == EIO);
}

static void
test_lease_error_forces_renew_rebind(void)
{
	assert(!dhcp_wan_bound_retry_required(0, 0, 0));
	assert(dhcp_wan_bound_retry_required(EADDRINUSE, 0, 0));
	assert(dhcp_wan_bound_retry_required(0, EIO, 0));
	assert(dhcp_wan_bound_retry_required(EADDRINUSE, EIO, 0));
	assert(dhcp_wan_bound_retry_required(0, 0, EIO));
	assert(dhcp_wan_bound_retry_required(0, EIO, EBUSY));
}

static void
test_retry_decision_fails_closed_on_each_status_read_error(void)
{
	int retry_required = 1;

	assert(dhcp_wan_bound_retry_decide(EACCES, 0, 0, 0, 0, 0,
					   &retry_required) == EACCES);
	assert(retry_required == 0);
	retry_required = 1;
	assert(dhcp_wan_bound_retry_decide(0, EIO, 0, 0, 0, 0,
					   &retry_required) == EIO);
	assert(retry_required == 0);
	retry_required = 1;
	assert(dhcp_wan_bound_retry_decide(0, 0, ENOENT, 0, 0, 0,
					   &retry_required) == ENOENT);
	assert(retry_required == 0);
	assert(dhcp_wan_bound_retry_decide(EACCES, EIO, ENOENT, 0, 0, 0,
					   &retry_required) == EACCES);
	assert(retry_required == 0);
	assert(dhcp_wan_bound_retry_decide(0, 0, 0, 0, 0, 0, NULL) == EINVAL);
}

static void
test_retry_decision_requires_a_complete_checked_tuple(void)
{
	int retry_required = -1;

	assert(dhcp_wan_bound_retry_decide(0, 0, 0, 0, 0, 0,
					   &retry_required) == 0);
	assert(retry_required == 0);
	assert(dhcp_wan_bound_retry_decide(0, 0, 0, EADDRINUSE, 0, 0,
					   &retry_required) == 0);
	assert(retry_required == 1);
	assert(dhcp_wan_bound_retry_decide(0, 0, 0, 0, EIO, 0,
					   &retry_required) == 0);
	assert(retry_required == 1);
	assert(dhcp_wan_bound_retry_decide(0, 0, 0, 0, 0, EBUSY,
					   &retry_required) == 0);
	assert(retry_required == 1);
}


struct dhcp_retry_marker_fixture {
	int values[DHCP_WAN_RETRY_MARKER_COUNT];
	unsigned int writes[DHCP_WAN_RETRY_MARKER_COUNT];
	int fail_marker;
};

static int
dhcp_retry_marker_fixture_write(void *opaque, unsigned int marker, int value)
{
	struct dhcp_retry_marker_fixture *fixture =
		(struct dhcp_retry_marker_fixture *)opaque;

	if (!fixture || marker >= DHCP_WAN_RETRY_MARKER_COUNT)
		return EINVAL;
	++fixture->writes[marker];
	if ((int)marker == fixture->fail_marker)
		return EIO;
	fixture->values[marker] = value;
	return 0;
}

static void
test_retry_marker_write_failure_keeps_followup_retry_latched(void)
{
	unsigned int failed_marker;
	int retry_required, marker;

	for (failed_marker = 0; failed_marker < DHCP_WAN_RETRY_MARKER_COUNT;
	     ++failed_marker) {
		struct dhcp_retry_marker_fixture fixture = {
			{ 0, 0, 0 }, { 0, 0, 0 }, (int)failed_marker
		};

		assert(dhcp_wan_bound_retry_markers_publish(
			0, 0, 0, dhcp_retry_marker_fixture_write, &fixture) == EIO);
		assert(fixture.writes[failed_marker] == 2U);
		for (marker = 0; marker < DHCP_WAN_RETRY_MARKER_COUNT; ++marker)
			assert(fixture.writes[marker] == 2U);

		retry_required = 0;
		assert(dhcp_wan_bound_retry_decide(
			0, 0, 0, fixture.values[DHCP_WAN_RETRY_MARKER_LEASE],
			fixture.values[DHCP_WAN_RETRY_MARKER_ROUTE],
			fixture.values[DHCP_WAN_RETRY_MARKER_MTU],
			&retry_required) == 0);
		assert(retry_required == 1);
	}

	{
		struct dhcp_retry_marker_fixture fixture = {
			{ 0, 0, 0 }, { 0, 0, 0 }, -1
		};

		assert(dhcp_wan_bound_retry_markers_publish(
			0, EHOSTUNREACH, 0, dhcp_retry_marker_fixture_write,
			&fixture) == 0);
		assert(fixture.values[DHCP_WAN_RETRY_MARKER_LEASE] == 0);
		assert(fixture.values[DHCP_WAN_RETRY_MARKER_ROUTE] == EHOSTUNREACH);
		assert(fixture.values[DHCP_WAN_RETRY_MARKER_MTU] == 0);
		retry_required = 0;
		assert(dhcp_wan_bound_retry_decide(
			0, 0, 0, fixture.values[DHCP_WAN_RETRY_MARKER_LEASE],
			fixture.values[DHCP_WAN_RETRY_MARKER_ROUTE],
			fixture.values[DHCP_WAN_RETRY_MARKER_MTU],
			&retry_required) == 0);
		assert(retry_required == 1);
	}
	assert(dhcp_wan_bound_retry_markers_publish(0, 0, 0, NULL, NULL) == EINVAL);
}

int
main(void)
{
	test_bound_callback_status_ignores_auxiliary_service_failures();
	test_lease_error_forces_renew_rebind();
	test_retry_decision_fails_closed_on_each_status_read_error();
	test_retry_decision_requires_a_complete_checked_tuple();
	test_retry_marker_write_failure_keeps_followup_retry_latched();
	return 0;
}
