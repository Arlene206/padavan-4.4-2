#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "wan_viptv_static_policy.h"

struct fake_start {
	int address_status;
	int publish_status;
	int clear_address_status;
	int route_status;
	int multicast_status;
	char events[16];
	int snapshot_published;
	int address_cleared;
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

static void
init_fake(struct fake_start *fake)
{
	memset(fake, 0, sizeof(*fake));
}

static int
fake_configure_address(void *opaque)
{
	struct fake_start *fake = (struct fake_start *)opaque;

	record_event(fake, 'A');
	return fake->address_status;
}

static int
fake_publish_snapshot(void *opaque)
{
	struct fake_start *fake = (struct fake_start *)opaque;

	record_event(fake, 'P');
	if (fake->publish_status == 0)
		fake->snapshot_published = 1;
	return fake->publish_status;
}

static int
fake_clear_address(void *opaque)
{
	struct fake_start *fake = (struct fake_start *)opaque;

	fake->address_cleared = 1;
	record_event(fake, 'C');
	return fake->clear_address_status;
}

static int
fake_add_default_route(void *opaque)
{
	struct fake_start *fake = (struct fake_start *)opaque;

	record_event(fake, 'R');
	return fake->route_status;
}

static int
fake_start_multicast(void *opaque)
{
	struct fake_start *fake = (struct fake_start *)opaque;

	fake->multicast_started = 1;
	record_event(fake, 'M');
	return fake->multicast_status;
}

static const struct wan_viptv_static_ops fake_ops = {
	fake_configure_address,
	fake_publish_snapshot,
	fake_clear_address,
	fake_add_default_route,
	fake_start_multicast
};

static void
test_address_failure_does_not_publish_or_start_services(void)
{
	struct fake_start fake;

	init_fake(&fake);
	fake.address_status = EIO;
	assert(wan_viptv_static_start(1, &fake_ops, &fake) == EIO);
	assert(strcmp(fake.events, "A") == 0);
	assert(!fake.snapshot_published);
	assert(!fake.address_cleared);
	assert(!fake.multicast_started);
}

static void
test_snapshot_failure_clears_address_and_skips_routes_and_multicast(void)
{
	struct fake_start fake;

	init_fake(&fake);
	fake.publish_status = ENOSPC;
	assert(wan_viptv_static_start(1, &fake_ops, &fake) == ENOSPC);
	assert(strcmp(fake.events, "APC") == 0);
	assert(fake.address_cleared);
	assert(!fake.snapshot_published);
	assert(!fake.multicast_started);
}

static void
test_snapshot_failure_preserves_primary_error_when_address_clear_fails(void)
{
	struct fake_start fake;

	init_fake(&fake);
	fake.publish_status = ENOSPC;
	fake.clear_address_status = EIO;
	assert(wan_viptv_static_start(1, &fake_ops, &fake) == ENOSPC);
	assert(strcmp(fake.events, "APC") == 0);
	assert(fake.address_cleared);
	assert(!fake.multicast_started);
}

static void
test_successful_start_orders_snapshot_before_route_and_multicast(void)
{
	struct fake_start fake;

	init_fake(&fake);
	assert(wan_viptv_static_start(1, &fake_ops, &fake) == 0);
	assert(strcmp(fake.events, "APRM") == 0);
	assert(fake.snapshot_published);
	assert(!fake.address_cleared);
	assert(fake.multicast_started);
}

static void
test_route_failure_is_reported_after_recovery_state_publication(void)
{
	struct fake_start fake;

	init_fake(&fake);
	fake.route_status = EIO;
	assert(wan_viptv_static_start(1, &fake_ops, &fake) == EIO);
	assert(strcmp(fake.events, "APRM") == 0);
	assert(fake.snapshot_published);
	assert(fake.multicast_started);
}

static void
test_multicast_failure_is_returned_without_gateway(void)
{
	struct fake_start fake;

	init_fake(&fake);
	fake.multicast_status = EBUSY;
	assert(wan_viptv_static_start(0, &fake_ops, &fake) == EBUSY);
	assert(strcmp(fake.events, "APM") == 0);
	assert(fake.multicast_started);
}

static void
test_first_route_error_is_preserved_when_multicast_also_fails(void)
{
	struct fake_start fake;

	init_fake(&fake);
	fake.route_status = EIO;
	fake.multicast_status = EPERM;
	assert(wan_viptv_static_start(1, &fake_ops, &fake) == EIO);
	assert(strcmp(fake.events, "APRM") == 0);
	assert(fake.multicast_started);
}

static void
test_missing_gateway_skips_default_route(void)
{
	struct fake_start fake;

	init_fake(&fake);
	assert(wan_viptv_static_start(0, &fake_ops, &fake) == 0);
	assert(strcmp(fake.events, "APM") == 0);
	assert(fake.snapshot_published);
	assert(fake.multicast_started);
}

static void
test_invalid_arguments_fail_before_side_effects(void)
{
	struct fake_start fake;
	struct wan_viptv_static_ops missing_route_ops = {
		fake_configure_address,
		fake_publish_snapshot,
		fake_clear_address,
		NULL,
		fake_start_multicast
	};
	struct wan_viptv_static_ops missing_clear_ops = {
		fake_configure_address,
		fake_publish_snapshot,
		NULL,
		fake_add_default_route,
		fake_start_multicast
	};

	init_fake(&fake);
	assert(wan_viptv_static_start(2, &fake_ops, &fake) == EINVAL);
	assert(fake.events[0] == '\0');
	assert(wan_viptv_static_start(1, &missing_route_ops, &fake) == EINVAL);
	assert(fake.events[0] == '\0');
	assert(wan_viptv_static_start(0, &missing_clear_ops, &fake) == EINVAL);
	assert(fake.events[0] == '\0');
}

struct fake_ifname_publish {
	int publish_status;
	int rollback_status;
	char events[8];
};

static void
record_ifname_event(struct fake_ifname_publish *fake, char event)
{
	size_t length = strlen(fake->events);

	assert(length + 1 < sizeof(fake->events));
	fake->events[length] = event;
	fake->events[length + 1] = '\0';
}

static int
fake_ifname_publish(void *opaque)
{
	struct fake_ifname_publish *fake =
		(struct fake_ifname_publish *)opaque;

	record_ifname_event(fake, 'P');
	return fake->publish_status;
}

static int
fake_ifname_rollback(void *opaque)
{
	struct fake_ifname_publish *fake =
		(struct fake_ifname_publish *)opaque;

	record_ifname_event(fake, 'R');
	return fake->rollback_status;
}

static const struct wan_viptv_ifname_ops fake_ifname_ops = {
	fake_ifname_publish,
	fake_ifname_rollback
};

static void
test_ifname_publish_success_keeps_vlan_and_skips_rollback(void)
{
	struct fake_ifname_publish fake;

	memset(&fake, 0, sizeof(fake));
	assert(wan_viptv_ifname_publish_or_rollback(&fake_ifname_ops, &fake) == 0);
	assert(strcmp(fake.events, "P") == 0);
}

static void
test_ifname_publish_failure_rolls_back_vlan_and_preserves_primary_error(void)
{
	struct fake_ifname_publish fake;

	memset(&fake, 0, sizeof(fake));
	fake.publish_status = ENOSPC;
	assert(wan_viptv_ifname_publish_or_rollback(&fake_ifname_ops, &fake) == ENOSPC);
	assert(strcmp(fake.events, "PR") == 0);
}

static void
test_ifname_rollback_failure_does_not_mask_publish_error(void)
{
	struct fake_ifname_publish fake;

	memset(&fake, 0, sizeof(fake));
	fake.publish_status = ENOSPC;
	fake.rollback_status = EIO;
	assert(wan_viptv_ifname_publish_or_rollback(&fake_ifname_ops, &fake) == ENOSPC);
	assert(strcmp(fake.events, "PR") == 0);
}

static void
test_ifname_publish_policy_rejects_incomplete_callbacks(void)
{
	struct fake_ifname_publish fake;
	struct wan_viptv_ifname_ops missing_rollback = {
		fake_ifname_publish, NULL
	};

	memset(&fake, 0, sizeof(fake));
	assert(wan_viptv_ifname_publish_or_rollback(NULL, &fake) == EINVAL);
	assert(wan_viptv_ifname_publish_or_rollback(&missing_rollback, &fake) == EINVAL);
	assert(fake.events[0] == '\0');
}


struct fake_old_vlan_replace {
	int clear_status;
	int remove_status;
	int restore_status;
	char events[8];
};

static void
record_old_vlan_event(struct fake_old_vlan_replace *fake, char event)
{
	size_t length = strlen(fake->events);

	assert(length + 1 < sizeof(fake->events));
	fake->events[length] = event;
	fake->events[length + 1] = '\0';
}

static int
fake_old_vlan_clear(void *opaque)
{
	struct fake_old_vlan_replace *fake =
		(struct fake_old_vlan_replace *)opaque;

	record_old_vlan_event(fake, 'C');
	return fake->clear_status;
}

static int
fake_old_vlan_remove(void *opaque)
{
	struct fake_old_vlan_replace *fake =
		(struct fake_old_vlan_replace *)opaque;

	record_old_vlan_event(fake, 'R');
	return fake->remove_status;
}

static int
fake_old_vlan_restore(void *opaque)
{
	struct fake_old_vlan_replace *fake =
		(struct fake_old_vlan_replace *)opaque;

	record_old_vlan_event(fake, 'S');
	return fake->restore_status;
}

static const struct wan_viptv_old_vlan_ops fake_old_vlan_ops = {
	fake_old_vlan_clear,
	fake_old_vlan_remove,
	fake_old_vlan_restore
};

static void
test_old_vlan_metadata_is_cleared_before_vlan_removal(void)
{
	struct fake_old_vlan_replace fake;

	memset(&fake, 0, sizeof(fake));
	assert(wan_viptv_ifname_remove_old_vlan(&fake_old_vlan_ops, &fake) == 0);
	assert(strcmp(fake.events, "CR") == 0);
}

static void
test_old_vlan_clear_failure_preserves_vlan_and_skips_removal(void)
{
	struct fake_old_vlan_replace fake;

	memset(&fake, 0, sizeof(fake));
	fake.clear_status = ENOSPC;
	assert(wan_viptv_ifname_remove_old_vlan(&fake_old_vlan_ops, &fake) == ENOSPC);
	assert(strcmp(fake.events, "C") == 0);
}

static void
test_old_vlan_remove_failure_restores_recovery_identity(void)
{
	struct fake_old_vlan_replace fake;

	memset(&fake, 0, sizeof(fake));
	fake.remove_status = EBUSY;
	assert(wan_viptv_ifname_remove_old_vlan(&fake_old_vlan_ops, &fake) == EBUSY);
	assert(strcmp(fake.events, "CRS") == 0);
}

static void
test_old_vlan_restore_failure_preserves_remove_error(void)
{
	struct fake_old_vlan_replace fake;

	memset(&fake, 0, sizeof(fake));
	fake.remove_status = EBUSY;
	fake.restore_status = ENOSPC;
	assert(wan_viptv_ifname_remove_old_vlan(&fake_old_vlan_ops, &fake) == EBUSY);
	assert(strcmp(fake.events, "CRS") == 0);
}

static void
test_old_vlan_transition_rejects_incomplete_callbacks(void)
{
	struct fake_old_vlan_replace fake;
	struct wan_viptv_old_vlan_ops missing_restore = {
		fake_old_vlan_clear, fake_old_vlan_remove, NULL
	};

	memset(&fake, 0, sizeof(fake));
	assert(wan_viptv_ifname_remove_old_vlan(NULL, &fake) == EINVAL);
	assert(wan_viptv_ifname_remove_old_vlan(&missing_restore, &fake) == EINVAL);
	assert(fake.events[0] == '\0');
}


int
main(void)
{
	test_address_failure_does_not_publish_or_start_services();
	test_snapshot_failure_clears_address_and_skips_routes_and_multicast();
	test_snapshot_failure_preserves_primary_error_when_address_clear_fails();
	test_successful_start_orders_snapshot_before_route_and_multicast();
	test_route_failure_is_reported_after_recovery_state_publication();
	test_multicast_failure_is_returned_without_gateway();
	test_first_route_error_is_preserved_when_multicast_also_fails();
	test_missing_gateway_skips_default_route();
	test_invalid_arguments_fail_before_side_effects();
	test_ifname_publish_success_keeps_vlan_and_skips_rollback();
	test_ifname_publish_failure_rolls_back_vlan_and_preserves_primary_error();
	test_ifname_rollback_failure_does_not_mask_publish_error();
	test_ifname_publish_policy_rejects_incomplete_callbacks();
	test_old_vlan_metadata_is_cleared_before_vlan_removal();
	test_old_vlan_clear_failure_preserves_vlan_and_skips_removal();
	test_old_vlan_remove_failure_restores_recovery_identity();
	test_old_vlan_restore_failure_preserves_remove_error();
	test_old_vlan_transition_rejects_incomplete_callbacks();
	puts("Static IPTV startup, identity and replacement regression: PASS");
	return 0;
}
