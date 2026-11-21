#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "dhcp_lease_policy.h"
#include "dnsmasq_domain_policy.h"


struct dhcp_bound_fake {
	int delete_old_status;
	int delete_new_status;
	int add_old_status;
	int add_new_status;
	int clear_status;
	int set_old_status;
	int set_new_status;
	int publish_status;
	char events[32];
	size_t event_count;
};

static void
dhcp_bound_fake_event(struct dhcp_bound_fake *fake, char event)
{
	if (fake->event_count + 1 < sizeof(fake->events)) {
		fake->events[fake->event_count++] = event;
		fake->events[fake->event_count] = '\0';
	}
}

static int
dhcp_bound_fake_delete(void *context, const char *gateway)
{
	struct dhcp_bound_fake *fake = (struct dhcp_bound_fake *)context;

	if (strcmp(gateway, "192.0.2.1") == 0) {
		dhcp_bound_fake_event(fake, 'd');
		return fake->delete_old_status;
	}
	dhcp_bound_fake_event(fake, 'D');
	return fake->delete_new_status;
}

static int
dhcp_bound_fake_clear(void *context)
{
	struct dhcp_bound_fake *fake = (struct dhcp_bound_fake *)context;

	dhcp_bound_fake_event(fake, 'C');
	return fake->clear_status;
}

static int
dhcp_bound_fake_set(void *context, const char *ipaddr, const char *netmask)
{
	struct dhcp_bound_fake *fake = (struct dhcp_bound_fake *)context;

	(void)netmask;
	if (strcmp(ipaddr, "192.0.2.10") == 0) {
		dhcp_bound_fake_event(fake, 's');
		return fake->set_old_status;
	}
	dhcp_bound_fake_event(fake, 'S');
	return fake->set_new_status;
}

static int
dhcp_bound_fake_add(void *context, const char *gateway)
{
	struct dhcp_bound_fake *fake = (struct dhcp_bound_fake *)context;

	if (strcmp(gateway, "192.0.2.1") == 0) {
		dhcp_bound_fake_event(fake, 'a');
		return fake->add_old_status;
	}
	dhcp_bound_fake_event(fake, 'A');
	return fake->add_new_status;
}

static int
dhcp_bound_fake_publish(void *context)
{
	struct dhcp_bound_fake *fake = (struct dhcp_bound_fake *)context;

	dhcp_bound_fake_event(fake, 'P');
	return fake->publish_status;
}

static const struct dhcp_lease_bound_ops dhcp_bound_fake_ops = {
	dhcp_bound_fake_delete,
	dhcp_bound_fake_clear,
	dhcp_bound_fake_set,
	dhcp_bound_fake_add,
	dhcp_bound_fake_publish
};


struct dhcp_nvram_fake {
	const char *names[3];
	const char *old_values[3];
	const char *new_values[3];
	char current[3][32];
	int fail_get_enabled;
	int fail_get_index;
	int fail_get_status;
	int fail_new_index;
	int fail_new_status;
	int fail_new_after_write;
	int fail_restore_index;
	int fail_restore_status;
	int fail_restore_after_write;
	char events[32];
	size_t event_count;
};

static void
dhcp_nvram_fake_event(struct dhcp_nvram_fake *fake, char event)
{
	if (fake->event_count + 1 < sizeof(fake->events)) {
		fake->events[fake->event_count++] = event;
		fake->events[fake->event_count] = '\0';
	}
}

static int
dhcp_nvram_fake_set(void *context, const char *name, const char *value)
{
	struct dhcp_nvram_fake *fake = (struct dhcp_nvram_fake *)context;
	int i;

	for (i = 0; i < 3; ++i) {
		if (strcmp(name, fake->names[i]) != 0)
			continue;
		if (strcmp(value, fake->new_values[i]) == 0) {
			dhcp_nvram_fake_event(fake, (char)('A' + i));
			if (i == fake->fail_new_index) {
				if (fake->fail_new_after_write)
					snprintf(fake->current[i], sizeof(fake->current[i]),
						 "%s", value);
				return fake->fail_new_status;
			}
		} else if (strcmp(value, fake->old_values[i]) == 0) {
			dhcp_nvram_fake_event(fake, (char)('a' + i));
			if (i == fake->fail_restore_index) {
				if (fake->fail_restore_after_write)
					snprintf(fake->current[i], sizeof(fake->current[i]),
						 "%s", value);
				return fake->fail_restore_status;
			}
		} else {
			return EINVAL;
		}
		snprintf(fake->current[i], sizeof(fake->current[i]), "%s", value);
		return 0;
	}

	return ENOENT;
}

static int
dhcp_nvram_fake_get(void *context, const char *name,
                    const char **value)
{
	struct dhcp_nvram_fake *fake = (struct dhcp_nvram_fake *)context;
	int i;

	for (i = 0; i < 3; ++i) {
		if (strcmp(name, fake->names[i]) != 0)
			continue;
		dhcp_nvram_fake_event(fake, 'r');
		if (fake->fail_get_enabled && i == fake->fail_get_index)
			return fake->fail_get_status;
		*value = fake->current[i][0] ? fake->current[i] : NULL;
		return 0;
	}
	return ENOENT;
}


static void
test_lan_dhcp_nvram_publish_compensates_partial_updates(void)
{
	struct dhcp_nvram_fake fake;
	struct dhcp_lease_nvram_value values[3];
	int rollback_status, i;

	memset(&fake, 0, sizeof(fake));
	fake.names[0] = "lan_ipaddr_t";
	fake.names[1] = "lan_netmask_t";
	fake.names[2] = "lan_gateway_t";
	fake.old_values[0] = "old-a";
	fake.old_values[1] = "old-b";
	fake.old_values[2] = "old-c";
	fake.new_values[0] = "new-a";
	fake.new_values[1] = "new-b";
	fake.new_values[2] = "new-c";
	values[0].name = fake.names[0];
	values[1].name = fake.names[1];
	values[2].name = fake.names[2];
	values[0].value = fake.new_values[0];
	values[1].value = fake.new_values[1];
	values[2].value = fake.new_values[2];
	values[0].previous_value = fake.old_values[0];
	values[1].previous_value = fake.old_values[1];
	values[2].previous_value = fake.old_values[2];
	fake.fail_new_index = -1;
	fake.fail_restore_index = -1;
	for (i = 0; i < 3; ++i)
		snprintf(fake.current[i], sizeof(fake.current[i]), "%s",
			 fake.old_values[i]);

	assert(dhcp_lease_nvram_publish(values, 3, dhcp_nvram_fake_set,
					&fake, &rollback_status) == 0);
	assert(rollback_status == 0);
	assert(strcmp(fake.events, "ABC") == 0);
	for (i = 0; i < 3; ++i)
		assert(strcmp(fake.current[i], fake.new_values[i]) == 0);

	memset(fake.events, 0, sizeof(fake.events));
	fake.event_count = 0;
	fake.fail_new_index = 2;
	fake.fail_new_status = ENOSPC;
	fake.fail_new_after_write = 0;
	assert(dhcp_lease_nvram_publish(values, 3, dhcp_nvram_fake_set,
					&fake, &rollback_status) == ENOSPC);
	assert(rollback_status == 0);
	assert(strcmp(fake.events, "ABCcba") == 0);
	for (i = 0; i < 3; ++i)
		assert(strcmp(fake.current[i], fake.old_values[i]) == 0);

	/* Simulate a successful ioctl followed by a close error, then a failed
	 * restore of that just-written key. Earlier keys must still be restored. */
	memset(fake.events, 0, sizeof(fake.events));
	fake.event_count = 0;
	fake.fail_new_after_write = 1;
	fake.fail_restore_index = 2;
	fake.fail_restore_status = EIO;
	assert(dhcp_lease_nvram_publish(values, 3, dhcp_nvram_fake_set,
					&fake, &rollback_status) == ENOSPC);
	assert(rollback_status == EIO);
	assert(strcmp(fake.events, "ABCcba") == 0);
	assert(strcmp(fake.current[0], "old-a") == 0);
	assert(strcmp(fake.current[1], "old-b") == 0);
	assert(strcmp(fake.current[2], "new-c") == 0);

	/*
	 * Recovery after a partial rollback must not stop at the first error or
	 * compensate an applied restore back to the failed candidate lease.
	 */
	for (i = 0; i < 3; ++i)
		snprintf(fake.current[i], sizeof(fake.current[i]), "%s",
			 fake.new_values[i]);
	memset(fake.events, 0, sizeof(fake.events));
	fake.event_count = 0;
	fake.fail_restore_index = 1;
	fake.fail_restore_status = EIO;
	fake.fail_restore_after_write = 0;
	assert(dhcp_lease_nvram_restore(values, 3, dhcp_nvram_fake_set,
					&fake, &rollback_status) == EIO);
	assert(rollback_status == EIO);
	assert(strcmp(fake.events, "cba") == 0);
	assert(strcmp(fake.current[0], "old-a") == 0);
	assert(strcmp(fake.current[1], "new-b") == 0);
	assert(strcmp(fake.current[2], "old-c") == 0);

	/* A close error can follow a successful ioctl. Keep that old value and
	 * continue restoring the rest rather than undoing the applied restore. */
	for (i = 0; i < 3; ++i)
		snprintf(fake.current[i], sizeof(fake.current[i]), "%s",
			 fake.new_values[i]);
	memset(fake.events, 0, sizeof(fake.events));
	fake.event_count = 0;
	fake.fail_restore_index = 2;
	fake.fail_restore_status = EIO;
	fake.fail_restore_after_write = 1;
	assert(dhcp_lease_nvram_restore(values, 3, dhcp_nvram_fake_set,
					&fake, &rollback_status) == EIO);
	assert(rollback_status == EIO);
	assert(strcmp(fake.events, "cba") == 0);
	for (i = 0; i < 3; ++i)
		assert(strcmp(fake.current[i], fake.old_values[i]) == 0);
}

static void
test_dhcp_lease_nvram_restore_if_current_protects_newer_writer(void)
{
	struct dhcp_nvram_fake fake;
	struct dhcp_lease_nvram_value values[3];
	int restore_attempted, restore_status, status, i;

	memset(&fake, 0, sizeof(fake));
	fake.names[0] = "wan0_ipaddr";
	fake.names[1] = "wan0_gateway";
	fake.names[2] = "wan0_dns";
	fake.old_values[0] = "192.0.2.10";
	fake.old_values[1] = "192.0.2.1";
	fake.old_values[2] = "";
	fake.new_values[0] = "198.51.100.10";
	fake.new_values[1] = "198.51.100.1";
	fake.new_values[2] = "203.0.113.53";
	fake.fail_new_index = -1;
	fake.fail_restore_index = -1;
	for (i = 0; i < 3; ++i) {
		values[i].name = fake.names[i];
		values[i].value = fake.new_values[i];
		values[i].previous_value = fake.old_values[i];
		snprintf(fake.current[i], sizeof(fake.current[i]), "%s",
			 fake.new_values[i]);
	}

	/* Recovery of a fully-published candidate restores the old tuple. */
	status = dhcp_lease_nvram_restore_if_current(
		values, 3, dhcp_nvram_fake_get, dhcp_nvram_fake_set, &fake,
		&restore_attempted, &restore_status);
	assert(status == 0);
	assert(restore_attempted == 1 && restore_status == 0);
	assert(strcmp(fake.events, "rrrcba") == 0);
	for (i = 0; i < 3; ++i)
		assert(strcmp(fake.current[i], fake.old_values[i]) == 0);

	/* A previous compensation may have restored only part of the tuple. */
	for (i = 0; i < 3; ++i)
		snprintf(fake.current[i], sizeof(fake.current[i]), "%s",
			 fake.new_values[i]);
	snprintf(fake.current[0], sizeof(fake.current[0]), "%s",
		 fake.old_values[0]);
	memset(fake.events, 0, sizeof(fake.events));
	fake.event_count = 0;
	status = dhcp_lease_nvram_restore_if_current(
		values, 3, dhcp_nvram_fake_get, dhcp_nvram_fake_set, &fake,
		&restore_attempted, &restore_status);
	assert(status == 0);
	assert(restore_attempted == 1 && restore_status == 0);
	assert(strcmp(fake.events, "rrrcba") == 0);
	for (i = 0; i < 3; ++i)
		assert(strcmp(fake.current[i], fake.old_values[i]) == 0);

	/* A distinct newer lease is never overwritten by stale recovery. */
	for (i = 0; i < 3; ++i)
		snprintf(fake.current[i], sizeof(fake.current[i]), "%s",
			 fake.new_values[i]);
	snprintf(fake.current[1], sizeof(fake.current[1]), "%s", "newer-gateway");
	memset(fake.events, 0, sizeof(fake.events));
	fake.event_count = 0;
	status = dhcp_lease_nvram_restore_if_current(
		values, 3, dhcp_nvram_fake_get, dhcp_nvram_fake_set, &fake,
		&restore_attempted, &restore_status);
	assert(status == EAGAIN);
	assert(restore_attempted == 0 && restore_status == 0);
	assert(strcmp(fake.events, "rr") == 0);
	assert(strcmp(fake.current[0], fake.new_values[0]) == 0);
	assert(strcmp(fake.current[1], "newer-gateway") == 0);
	assert(strcmp(fake.current[2], fake.new_values[2]) == 0);

	/* Backend read errors also stop before the first recovery write. */
	for (i = 0; i < 3; ++i)
		snprintf(fake.current[i], sizeof(fake.current[i]), "%s",
			 fake.new_values[i]);
	fake.fail_get_enabled = 1;
	fake.fail_get_index = 2;
	fake.fail_get_status = EIO;
	memset(fake.events, 0, sizeof(fake.events));
	fake.event_count = 0;
	status = dhcp_lease_nvram_restore_if_current(
		values, 3, dhcp_nvram_fake_get, dhcp_nvram_fake_set, &fake,
		&restore_attempted, &restore_status);
	assert(status == EIO);
	assert(restore_attempted == 0 && restore_status == 0);
	assert(strcmp(fake.events, "rrr") == 0);
}


static void
test_dhcp_lease_nvram_publish_rejects_stale_snapshot(void)
{
	struct dhcp_nvram_fake fake;
	struct dhcp_lease_nvram_value values[3], restore_values[3];
	int publish_attempted, rollback_status, status, i;

	memset(&fake, 0, sizeof(fake));
	fake.names[0] = "wan0_ipaddr";
	fake.names[1] = "wan0_gateway";
	fake.names[2] = "wan0_dns";
	fake.old_values[0] = "192.0.2.10";
	fake.old_values[1] = "192.0.2.1";
	fake.old_values[2] = "";
	fake.new_values[0] = "198.51.100.10";
	fake.new_values[1] = "198.51.100.1";
	fake.new_values[2] = "203.0.113.53";
	fake.fail_new_index = -1;
	fake.fail_restore_index = -1;
	for (i = 0; i < 3; ++i) {
		values[i].name = fake.names[i];
		values[i].value = fake.new_values[i];
		values[i].previous_value = fake.old_values[i];
		snprintf(fake.current[i], sizeof(fake.current[i]), "%s",
			 fake.old_values[i]);
	}

	/* The expected tuple is current: verify all fields before any write. */
	status = dhcp_lease_nvram_publish_if_current(
		values, 3, dhcp_nvram_fake_get, dhcp_nvram_fake_set, &fake,
		&publish_attempted, &rollback_status);
	assert(status == 0);
	assert(publish_attempted == 1 && rollback_status == 0);
	assert(strcmp(fake.events, "rrrABC") == 0);
	for (i = 0; i < 3; ++i)
		assert(strcmp(fake.current[i], fake.new_values[i]) == 0);

	/* Reverse publication restores this tuple only while it remains current. */
	for (i = 0; i < 3; ++i) {
		restore_values[i].name = values[i].name;
		restore_values[i].value = values[i].previous_value;
		restore_values[i].previous_value = values[i].value;
	}
	memset(fake.events, 0, sizeof(fake.events));
	fake.event_count = 0;
	status = dhcp_lease_nvram_publish_if_current(
		restore_values, 3, dhcp_nvram_fake_get, dhcp_nvram_fake_set, &fake,
		&publish_attempted, &rollback_status);
	assert(status == 0);
	assert(publish_attempted == 1 && rollback_status == 0);
	assert(strcmp(fake.events, "rrrabc") == 0);
	for (i = 0; i < 3; ++i)
		assert(strcmp(fake.current[i], fake.old_values[i]) == 0);

	/* A newer callback's different tuple blocks this stale restoration. */
	for (i = 0; i < 3; ++i)
		snprintf(fake.current[i], sizeof(fake.current[i]), "%s",
			 fake.new_values[i]);
	snprintf(fake.current[1], sizeof(fake.current[1]), "%s", "newer-gateway");
	memset(fake.events, 0, sizeof(fake.events));
	fake.event_count = 0;
	status = dhcp_lease_nvram_publish_if_current(
		restore_values, 3, dhcp_nvram_fake_get, dhcp_nvram_fake_set, &fake,
		&publish_attempted, &rollback_status);
	assert(status == EAGAIN);
	assert(publish_attempted == 0 && rollback_status == 0);
	assert(strcmp(fake.events, "rr") == 0);
	assert(strcmp(fake.current[0], fake.new_values[0]) == 0);
	assert(strcmp(fake.current[1], "newer-gateway") == 0);
	assert(strcmp(fake.current[2], fake.new_values[2]) == 0);

	/* A newer callback committed between snapshot and publish. Refuse the
	 * stale lease and never compensate over the newer value. */
	memset(fake.events, 0, sizeof(fake.events));
	fake.event_count = 0;
	for (i = 0; i < 3; ++i)
		snprintf(fake.current[i], sizeof(fake.current[i]), "%s",
			 fake.old_values[i]);
	snprintf(fake.current[1], sizeof(fake.current[1]), "%s", "newer-gateway");
	status = dhcp_lease_nvram_publish_if_current(
		values, 3, dhcp_nvram_fake_get, dhcp_nvram_fake_set, &fake,
		&publish_attempted, &rollback_status);
	assert(status == EAGAIN);
	assert(publish_attempted == 0 && rollback_status == 0);
	assert(strcmp(fake.events, "rr") == 0);
	assert(strcmp(fake.current[0], fake.old_values[0]) == 0);
	assert(strcmp(fake.current[1], "newer-gateway") == 0);
	assert(strcmp(fake.current[2], fake.old_values[2]) == 0);

	/* A read failure also precedes all writes and must not trigger rollback. */
	memset(fake.events, 0, sizeof(fake.events));
	fake.event_count = 0;
	for (i = 0; i < 3; ++i)
		snprintf(fake.current[i], sizeof(fake.current[i]), "%s",
			 fake.old_values[i]);
	fake.fail_get_enabled = 1;
	fake.fail_get_index = 2;
	fake.fail_get_status = EIO;
	status = dhcp_lease_nvram_publish_if_current(
		values, 3, dhcp_nvram_fake_get, dhcp_nvram_fake_set, &fake,
		&publish_attempted, &rollback_status);
	assert(status == EIO);
	assert(publish_attempted == 0 && rollback_status == 0);
	assert(strcmp(fake.events, "rrr") == 0);
	for (i = 0; i < 3; ++i)
		assert(strcmp(fake.current[i], fake.old_values[i]) == 0);

	/* A missing NVRAM key is equivalent to the captured empty default. */
	fake.fail_get_enabled = 0;
	memset(fake.events, 0, sizeof(fake.events));
	fake.event_count = 0;
	status = dhcp_lease_nvram_publish_if_current(
		values, 3, dhcp_nvram_fake_get, dhcp_nvram_fake_set, &fake,
		&publish_attempted, &rollback_status);
	assert(status == 0);
	assert(publish_attempted == 1);
	assert(strcmp(fake.events, "rrrABC") == 0);
	assert(strcmp(fake.current[2], fake.new_values[2]) == 0);
}


static void
test_dhcp_lease_nvram_publish_attempts_all_and_restores_all(void)
{
	struct dhcp_nvram_fake fake;
	struct dhcp_lease_nvram_value values[3];
	int rollback_status, i;

	memset(&fake, 0, sizeof(fake));
	fake.names[0] = "wan0_dns";
	fake.names[1] = "wan0_domain";
	fake.names[2] = "wan0_wins";
	fake.old_values[0] = "old-dns";
	fake.old_values[1] = "old-domain";
	fake.old_values[2] = "old-wins";
	fake.new_values[0] = "new-dns";
	fake.new_values[1] = "new-domain";
	fake.new_values[2] = "new-wins";
	fake.fail_new_index = -1;
	fake.fail_restore_index = -1;
	for (i = 0; i < 3; ++i) {
		values[i].name = fake.names[i];
		values[i].value = fake.new_values[i];
		values[i].previous_value = fake.old_values[i];
		snprintf(fake.current[i], sizeof(fake.current[i]), "%s",
			 fake.old_values[i]);
	}

	/* Reject a malformed tail entry before any setter is called. */
	values[2].name = NULL;
	assert(dhcp_lease_nvram_publish(values, 3, dhcp_nvram_fake_set,
					&fake, &rollback_status) == EINVAL);
	assert(fake.event_count == 0);
	values[2].name = fake.names[2];

	/* A first-field error after write must still attempt later fields and
	 * restore the entire previous tuple in reverse order. */
	fake.fail_new_index = 0;
	fake.fail_new_status = ENOSPC;
	fake.fail_new_after_write = 1;
	assert(dhcp_lease_nvram_publish(values, 3, dhcp_nvram_fake_set,
					&fake, &rollback_status) == ENOSPC);
	assert(rollback_status == 0);
	assert(strcmp(fake.events, "ABCcba") == 0);
	for (i = 0; i < 3; ++i)
		assert(strcmp(fake.current[i], fake.old_values[i]) == 0);
}

/* A withdrawn IPTV lease stays deconfigured only when every required
 * operation succeeds. */
struct dhcp_viptv_deconfig_fake {
	int clear_status;
	int route_status;
	int published;
	int publish_status;
	char events[16];
	size_t event_count;
};

static void
test_dhcp_lease_6rd_snapshot_failure_restores_entire_tuple(void)
{
	struct dhcp_nvram_fake fake;
	struct dhcp_lease_nvram_value values[3];
	int rollback_status, i;

	memset(&fake, 0, sizeof(fake));
	fake.names[0] = "wan0_addr6";
	fake.names[1] = "wan0_6rd_size";
	fake.names[2] = "wan0_6rd_relay";
	fake.old_values[0] = "2001:db8:1::/64";
	fake.old_values[1] = "24";
	fake.old_values[2] = "192.0.2.1";
	fake.new_values[0] = "2001:db8:2::/64";
	fake.new_values[1] = "32";
	fake.new_values[2] = "198.51.100.1";
	fake.fail_new_index = 1;
	fake.fail_new_status = ENOSPC;
	fake.fail_new_after_write = 1;
	fake.fail_restore_index = -1;

	for (i = 0; i < 3; ++i) {
		values[i].name = fake.names[i];
		values[i].value = fake.new_values[i];
		values[i].previous_value = fake.old_values[i];
		snprintf(fake.current[i], sizeof(fake.current[i]), "%s",
			 fake.old_values[i]);
	}

	/* A failed close may still have applied; compensate all fields in reverse. */
	assert(dhcp_lease_nvram_publish(values, 3, dhcp_nvram_fake_set,
					&fake, &rollback_status) == ENOSPC);
	assert(rollback_status == 0);
	assert(strcmp(fake.events, "ABCcba") == 0);
	for (i = 0; i < 3; ++i)
		assert(strcmp(fake.current[i], fake.old_values[i]) == 0);
}


static void
dhcp_viptv_deconfig_event(struct dhcp_viptv_deconfig_fake *fake, char event)
{
	if (fake->event_count + 1 < sizeof(fake->events)) {
		fake->events[fake->event_count++] = event;
		fake->events[fake->event_count] = 0;
	}
}

static int
dhcp_viptv_deconfig_clear(void *context)
{
	struct dhcp_viptv_deconfig_fake *fake = (struct dhcp_viptv_deconfig_fake *)context;
	dhcp_viptv_deconfig_event(fake, 'C');
	return fake->clear_status;
}

static int
dhcp_viptv_deconfig_routes(void *context)
{
	struct dhcp_viptv_deconfig_fake *fake = (struct dhcp_viptv_deconfig_fake *)context;
	dhcp_viptv_deconfig_event(fake, 'R');
	return fake->route_status;
}

static int
dhcp_viptv_deconfig_publish(void *context)
{
	struct dhcp_viptv_deconfig_fake *fake = (struct dhcp_viptv_deconfig_fake *)context;
	fake->published++;
	dhcp_viptv_deconfig_event(fake, 'P');
	return fake->publish_status;
}

static const struct dhcp_lease_viptv_deconfig_ops dhcp_viptv_deconfig_fake_ops = {
	dhcp_viptv_deconfig_clear,
	dhcp_viptv_deconfig_routes,
	dhcp_viptv_deconfig_publish
};


static void
test_dhcp_lease_text_fits_buffer(void)
{
	char old_address[16];

	assert(dhcp_lease_text_fits_buffer("", sizeof(old_address)));
	assert(dhcp_lease_text_fits_buffer("255.255.255.255", sizeof(old_address)));
	assert(!dhcp_lease_text_fits_buffer("255.255.255.255x", sizeof(old_address)));
	assert(!dhcp_lease_text_fits_buffer(NULL, sizeof(old_address)));
	assert(!dhcp_lease_text_fits_buffer("1.2.3.4", 0));
}


struct dhcp_viptv_nvram_fake {
	const char *names[5];
	const char *old_values[5];
	const char *new_values[5];
	char current[5][48];
	int fail_new_index;
	int fail_restore_index;
	int fail_restore_status;
	int fail_new_after_write;
	char events[32];
	size_t event_count;
};

static void
dhcp_viptv_nvram_event(struct dhcp_viptv_nvram_fake *fake, char event)
{
	if (fake->event_count + 1 < sizeof(fake->events)) {
		fake->events[fake->event_count++] = event;
		fake->events[fake->event_count] = '\0';
	}
}

static int
dhcp_viptv_nvram_set(void *context, const char *name, const char *value)
{
	struct dhcp_viptv_nvram_fake *fake = (struct dhcp_viptv_nvram_fake *)context;
	size_t i;

	for (i = 0; i < 5; ++i) {
		if (strcmp(name, fake->names[i]) != 0)
			continue;
		if (strcmp(value, fake->new_values[i]) == 0) {
			dhcp_viptv_nvram_event(fake, (char)('A' + i));
			if ((int)i == fake->fail_new_index) {
				if (fake->fail_new_after_write)
					snprintf(fake->current[i], sizeof(fake->current[i]), "%s", value);
				return ENOSPC;
			}
		} else if (strcmp(value, fake->old_values[i]) == 0) {
			dhcp_viptv_nvram_event(fake, (char)('a' + i));
			if ((int)i == fake->fail_restore_index)
				return fake->fail_restore_status;
		} else {
			return EINVAL;
		}
		snprintf(fake->current[i], sizeof(fake->current[i]), "%s", value);
		return 0;
	}
	return ENOENT;
}

static void
test_dhcp_viptv_deconfig_metadata_clear_rolls_back_as_one_tuple(void)
{
	static const char *const names[5] = {
		"manv_ipaddr", "manv_gateway", "manv_routes",
		"manv_routes_ms", "manv_routes_rfc"
	};
	static const char *const old_values[5] = {
		"192.0.2.20", "192.0.2.1", "192.0.2.0/24 198.51.100.1",
		"198.51.100.0/24 192.0.2.1", "203.0.113.0/24 192.0.2.1"
	};
	static const char *const clear_values[5] = {
		"0.0.0.0", "", "", "", ""
	};
	struct dhcp_viptv_nvram_fake fake;
	struct dhcp_lease_nvram_value values[5];
	int rollback_status, i;

	memset(&fake, 0, sizeof(fake));
	fake.fail_new_index = -1;
	fake.fail_restore_index = -1;
	for (i = 0; i < 5; ++i) {
		fake.names[i] = names[i];
		fake.old_values[i] = old_values[i];
		fake.new_values[i] = clear_values[i];
		values[i].name = names[i];
		values[i].value = clear_values[i];
		values[i].previous_value = old_values[i];
		snprintf(fake.current[i], sizeof(fake.current[i]), "%s", old_values[i]);
	}

	/* All five lease fields become visible as one successful publication. */
	assert(dhcp_lease_nvram_publish(values, 5, dhcp_viptv_nvram_set,
					&fake, &rollback_status) == 0);
	assert(rollback_status == 0);
	assert(strcmp(fake.events, "ABCDE") == 0);
	for (i = 0; i < 5; ++i)
		assert(strcmp(fake.current[i], clear_values[i]) == 0);

	/* A partial write must restore the entire old tuple in reverse order. */
	memset(fake.events, 0, sizeof(fake.events));
	fake.event_count = 0;
	fake.fail_new_index = 2;
	fake.fail_new_after_write = 1;
	for (i = 0; i < 5; ++i)
		snprintf(fake.current[i], sizeof(fake.current[i]), "%s", old_values[i]);
	assert(dhcp_lease_nvram_publish(values, 5, dhcp_viptv_nvram_set,
					&fake, &rollback_status) == ENOSPC);
	assert(rollback_status == 0);
	assert(strcmp(fake.events, "ABCDEedcba") == 0);
	for (i = 0; i < 5; ++i)
		assert(strcmp(fake.current[i], old_values[i]) == 0);

	/* A failed compensating write is reported without skipping later fields. */
	memset(fake.events, 0, sizeof(fake.events));
	fake.event_count = 0;
	fake.fail_restore_index = 1;
	fake.fail_restore_status = EIO;
	for (i = 0; i < 5; ++i)
		snprintf(fake.current[i], sizeof(fake.current[i]), "%s", old_values[i]);
	assert(dhcp_lease_nvram_publish(values, 5, dhcp_viptv_nvram_set,
					&fake, &rollback_status) == ENOSPC);
	assert(rollback_status == EIO);
	assert(strcmp(fake.events, "ABCDEedcba") == 0);
	assert(strcmp(fake.current[0], old_values[0]) == 0);
	assert(strcmp(fake.current[1], clear_values[1]) == 0);
	assert(strcmp(fake.current[2], old_values[2]) == 0);
	assert(strcmp(fake.current[3], old_values[3]) == 0);
	assert(strcmp(fake.current[4], old_values[4]) == 0);
}

struct dhcp_nvram_tuple_fake {
	const char *names[12];
	const char *old_values[12];
	const char *new_values[12];
	char current[12][16];
	int fail_new_index;
	char events[32];
	size_t event_count;
};

static void
dhcp_nvram_tuple_event(struct dhcp_nvram_tuple_fake *fake, char event)
{
	if (fake->event_count + 1 < sizeof(fake->events)) {
		fake->events[fake->event_count++] = event;
		fake->events[fake->event_count] = '\0';
	}
}

static int
dhcp_nvram_tuple_set(void *context, const char *name, const char *value)
{
	struct dhcp_nvram_tuple_fake *fake = (struct dhcp_nvram_tuple_fake *)context;
	size_t i;

	for (i = 0; i < 12; ++i) {
		if (!fake->names[i])
			continue;
		if (strcmp(name, fake->names[i]) != 0)
			continue;
		if (strcmp(value, fake->new_values[i]) == 0) {
			dhcp_nvram_tuple_event(fake, (char)('A' + i));
			snprintf(fake->current[i], sizeof(fake->current[i]), "%s", value);
			if ((int)i == fake->fail_new_index)
				return ENOSPC;
			return 0;
		}
		if (strcmp(value, fake->old_values[i]) == 0) {
			dhcp_nvram_tuple_event(fake, (char)('a' + i));
			snprintf(fake->current[i], sizeof(fake->current[i]), "%s", value);
			return 0;
		}
		return EINVAL;
	}
	return ENOENT;
}

static void
test_dhcp_lease_nvram_publish_restores_full_wan_bound_tuple(void)
{
	static const char *const names[12] = {
		"wan0_ipaddr", "wan0_netmask", "wan0_dns", "wan0_wins",
		"wan0_mtu", "wan0_routes", "wan0_routes_ms", "wan0_routes_rfc",
		"wan0_domain", "wan0_lease", "wan0_gateway", "wan0_dltime"
	};
	static const char *const old_values[12] = {
		"old-ip", "old-mask", "old-dns", "old-wins", "old-mtu",
		"old-routes", "old-routes-ms", "old-routes-rfc", "old-domain",
		"old-lease", "old-gateway", "old-dltime"
	};
	static const char *const new_values[12] = {
		"new-ip", "new-mask", "new-dns", "new-wins", "new-mtu",
		"new-routes", "new-routes-ms", "new-routes-rfc", "new-domain",
		"new-lease", "new-gateway", "new-dltime"
	};
	struct dhcp_nvram_tuple_fake fake;
	struct dhcp_lease_nvram_value values[12];
	int rollback_status, i;

	memset(&fake, 0, sizeof(fake));
	fake.fail_new_index = 4;
	for (i = 0; i < 12; ++i) {
		fake.names[i] = names[i];
		fake.old_values[i] = old_values[i];
		fake.new_values[i] = new_values[i];
		values[i].name = names[i];
		values[i].value = new_values[i];
		values[i].previous_value = old_values[i];
		snprintf(fake.current[i], sizeof(fake.current[i]), "%s", old_values[i]);
	}
	assert(dhcp_lease_nvram_publish(values, 12, dhcp_nvram_tuple_set,
					&fake, &rollback_status) == ENOSPC);
	assert(rollback_status == 0);
	assert(strcmp(fake.events, "ABCDEFGHIJKLlkjihgfedcba") == 0);
	for (i = 0; i < 12; ++i)
		assert(strcmp(fake.current[i], old_values[i]) == 0);
}


static void
test_dhcp_lease_fast_renew_publishes_dltime_as_one_checked_value(void)
{
	struct dhcp_nvram_tuple_fake fake;
	struct dhcp_lease_nvram_value value;
	int rollback_status;

	memset(&fake, 0, sizeof(fake));
	fake.names[0] = "wan0_dltime";
	fake.old_values[0] = "123";
	fake.new_values[0] = "456";
	fake.fail_new_index = -1;
	snprintf(fake.current[0], sizeof(fake.current[0]), "%s",
		 fake.old_values[0]);

	value.name = fake.names[0];
	value.value = fake.new_values[0];
	value.previous_value = fake.old_values[0];
	assert(dhcp_lease_nvram_publish(&value, 1, dhcp_nvram_tuple_set,
					&fake, &rollback_status) == 0);
	assert(rollback_status == 0);
	assert(strcmp(fake.events, "A") == 0);
	assert(strcmp(fake.current[0], "456") == 0);
}

static void
test_dhcp_lease_nvram_reverse_snapshot_validates_complete_tuple(void)
{
	struct dhcp_lease_nvram_value values[3], reversed[3];
	const char *const names[3] = {
		"wan0_ipaddr", "wan0_gateway", "wan0_dns"
	};
	const char *const candidates[3] = {
		"198.51.100.10", "198.51.100.1", "203.0.113.53"
	};
	const char *const previous[3] = {
		"192.0.2.10", "192.0.2.1", ""
	};
	size_t i;

	memset(reversed, 0, sizeof(reversed));
	for (i = 0; i < 3; ++i) {
		values[i].name = names[i];
		values[i].value = candidates[i];
		values[i].previous_value = previous[i];
	}
	assert(dhcp_lease_nvram_reverse_snapshot(values, 3, reversed) == 0);
	for (i = 0; i < 3; ++i) {
		assert(reversed[i].name == values[i].name);
		assert(reversed[i].value == values[i].previous_value);
		assert(reversed[i].previous_value == values[i].value);
	}

	/* Validate the entire input before modifying any destination entry. */
	memset(reversed, 0, sizeof(reversed));
	values[2].previous_value = NULL;
	assert(dhcp_lease_nvram_reverse_snapshot(values, 3, reversed) == EINVAL);
	for (i = 0; i < 3; ++i) {
		assert(reversed[i].name == NULL);
		assert(reversed[i].value == NULL);
		assert(reversed[i].previous_value == NULL);
	}
	values[2].previous_value = previous[2];
	assert(dhcp_lease_nvram_reverse_snapshot(values, 0, reversed) == EINVAL);
	assert(dhcp_lease_nvram_reverse_snapshot(values, 3, NULL) == EINVAL);
	assert(dhcp_lease_nvram_reverse_snapshot(values, 3, values) == EINVAL);
}


int
main(void)
{
	char first[INET_ADDRSTRLEN];
	unsigned int number = 0;
	struct dhcp_bound_fake fake;
	struct dhcp_viptv_deconfig_fake viptv_fake;
	int rollback_status, transaction_status;

	assert(dhcp_lease_ipv4_is_valid("192.0.2.1"));
	assert(!dhcp_lease_ipv4_is_valid("0.0.0.0"));
	assert(!dhcp_lease_ipv4_list_is_valid("0.0.0.0 192.0.2.1", 255, 16, 1));
	assert(dhcp_lease_route_destination_is_valid("0.0.0.0", 0));
	assert(!dhcp_lease_route_destination_is_valid("192.0.2.1", 0));
	assert(!dhcp_lease_route_destination_is_valid("0.0.0.0", 24));
	assert(dhcp_lease_route_destination_is_valid("192.0.2.0", 24));
	assert(dhcp_lease_route_delete_is_ok(0));
	assert(dhcp_lease_route_delete_is_ok(ESRCH));
	assert(!dhcp_lease_route_delete_is_ok(EIO));
	assert(dhcp_lease_deconfig_address_ready(0, 0));
	assert(dhcp_lease_deconfig_address_ready(ESRCH, 0));
	assert(!dhcp_lease_deconfig_address_ready(EIO, 0));
	assert(!dhcp_lease_deconfig_address_ready(0, EIO));
	assert(dhcp_lease_deconfig_status(0, 0, 0) == 0);
	assert(dhcp_lease_deconfig_status(ESRCH, 0, 0) == 0);
	assert(dhcp_lease_deconfig_status(EIO, 0, 0) == EIO);
	assert(dhcp_lease_deconfig_status(0, EIO, 0) == EIO);
	assert(dhcp_lease_deconfig_status(0, -1, 0) == -1);
	assert(dhcp_lease_deconfig_status(0, 0, ENOSPC) == ENOSPC);
	assert(dhcp_lease_deconfig_status(EIO, EIO, ENOSPC) == EIO);
	assert(!dhcp_lease_route_destination_is_valid("192.0.2.0", 33));
	assert(!dhcp_lease_ipv4_is_valid("192.0.2.1;reboot"));
	assert(!dhcp_lease_ipv4_is_valid("192.0.2.1\n"));
	assert(!dhcp_lease_ipv4_is_valid("999.0.2.1"));
	assert(dhcp_lease_netmask_is_valid("255.255.255.0"));
	assert(dhcp_lease_netmask_is_valid("255.255.255.255"));
	assert(!dhcp_lease_netmask_is_valid("255.0.255.0"));
	assert(!dhcp_lease_netmask_is_valid("0.0.0.0"));

	assert(dnsmasq_valid_server_domain("example.com"));
	assert(dnsmasq_valid_server_domain("router.lan."));
	assert(!dnsmasq_valid_server_domain(""));
	assert(!dnsmasq_valid_server_domain("example.com\n127.0.0.1 injected"));
	assert(!dnsmasq_valid_server_domain("example.com/8.8.8.8"));
	assert(!dnsmasq_valid_server_domain("-bad.example"));
	assert(!dnsmasq_valid_server_domain("bad-.example"));

	assert(dhcp_lease_ipv4_list_is_valid("192.0.2.1 198.51.100.2", 255, 16, 0));
	assert(dhcp_lease_ipv4_list_is_valid("", 255, 16, 1));
	assert(dhcp_lease_ipv4_list_is_valid(NULL, 255, 16, 1));
	assert(!dhcp_lease_ipv4_list_is_valid("192.0.2.1 198.51.100.2;reboot", 255, 16, 1));
	assert(!dhcp_lease_ipv4_list_is_valid("192.0.2.1\r198.51.100.2", 255, 16, 1));
	assert(!dhcp_lease_ipv4_list_is_valid("192.0.2.1 999.1.1.1", 255, 16, 1));
	assert(!dhcp_lease_ipv4_list_is_valid("192.0.2.1", 3, 16, 1));
	assert(dhcp_lease_ipv4_list_first(" 192.0.2.1 198.51.100.2", first, sizeof(first)) == 1);
	assert(strcmp(first, "192.0.2.1") == 0);
	assert(dhcp_lease_ipv4_list_first("", first, sizeof(first)) == 0);
	assert(dhcp_lease_ipv4_list_first("192.0.2.1,198.51.100.2", first, sizeof(first)) == -1);

	assert(!dhcp_lease_router_is_changed("192.0.2.1",
		"192.0.2.1 198.51.100.2", first, sizeof(first)));
	assert(dhcp_lease_router_is_changed("192.0.2.1", NULL,
		first, sizeof(first)));
	assert(first[0] == '\0');
	assert(!dhcp_lease_router_is_changed("", NULL, first, sizeof(first)));
	assert(dhcp_lease_router_is_changed("192.0.2.1", "",
		first, sizeof(first)));
	assert(first[0] == '\0');
	assert(dhcp_lease_router_is_changed("192.0.2.1", "198.51.100.1",
		first, sizeof(first)));
	assert(strcmp(first, "198.51.100.1") == 0);
	assert(dhcp_lease_router_is_changed("192.0.2.1", "invalid",
		first, sizeof(first)));
	assert(first[0] == '\0');

	assert(dhcp_lease_decimal_is_valid("1500", 576, 1500, &number) && number == 1500);
	assert(dhcp_lease_decimal_is_valid("0", 0, INT_MAX, &number) && number == 0);
	assert(!dhcp_lease_decimal_is_valid("-1", 0, INT_MAX, &number));
	assert(!dhcp_lease_decimal_is_valid("1501", 576, 1500, &number));
	assert(!dhcp_lease_decimal_is_valid("12x", 0, INT_MAX, &number));
	assert(!dhcp_lease_decimal_is_valid("999999999999999999999", 0, INT_MAX, &number));

	assert(dhcp_lease_route_text_is_safe(""));
	assert(dhcp_lease_route_text_is_safe("192.0.2.0/24 198.51.100.1 0.0.0.0/0 198.51.100.1"));
	assert(!dhcp_lease_route_text_is_safe("192.0.2.0/24;reboot"));
	assert(!dhcp_lease_route_text_is_safe("192.0.2.0/24\nCOMMIT"));
	assert(!dhcp_lease_route_text_is_safe("192.0.2.0/24 198.51.100.1 extra"));


	assert(dhcp_lease_route_add_is_ok(0));
	assert(dhcp_lease_route_add_is_ok(EEXIST));
	assert(!dhcp_lease_route_add_is_ok(EIO));

	memset(&fake, 0, sizeof(fake));
	transaction_status = dhcp_lease_bound_apply(
		"192.0.2.10", "255.255.255.0", "192.0.2.1",
		"198.51.100.20", "255.255.255.128", "198.51.100.1",
		1, 1, 1, &dhcp_bound_fake_ops, &fake, &rollback_status);
	assert(transaction_status == 0 && rollback_status == 0);
	assert(strcmp(fake.events, "dCSAP") == 0);

	memset(&fake, 0, sizeof(fake));
	fake.delete_old_status = EIO;
	transaction_status = dhcp_lease_bound_apply(
		"192.0.2.10", "255.255.255.0", "192.0.2.1",
		"198.51.100.20", "255.255.255.128", "198.51.100.1",
		1, 1, 1, &dhcp_bound_fake_ops, &fake, &rollback_status);
	assert(transaction_status == EIO && rollback_status == 0);
	assert(strcmp(fake.events, "d") == 0);

	memset(&fake, 0, sizeof(fake));
	fake.clear_status = EIO;
	transaction_status = dhcp_lease_bound_apply(
		"192.0.2.10", "255.255.255.0", "192.0.2.1",
		"198.51.100.20", "255.255.255.128", "198.51.100.1",
		1, 1, 1, &dhcp_bound_fake_ops, &fake, &rollback_status);
	assert(transaction_status == EIO && rollback_status == 0);
	assert(strcmp(fake.events, "dCsa") == 0);

	memset(&fake, 0, sizeof(fake));
	fake.set_new_status = EIO;
	transaction_status = dhcp_lease_bound_apply(
		"192.0.2.10", "255.255.255.0", "192.0.2.1",
		"198.51.100.20", "255.255.255.128", "198.51.100.1",
		1, 1, 1, &dhcp_bound_fake_ops, &fake, &rollback_status);
	assert(transaction_status == EIO && rollback_status == 0);
	assert(strcmp(fake.events, "dCSsa") == 0);

	memset(&fake, 0, sizeof(fake));
	fake.add_new_status = EIO;
	transaction_status = dhcp_lease_bound_apply(
		"192.0.2.10", "255.255.255.0", "192.0.2.1",
		"198.51.100.20", "255.255.255.128", "198.51.100.1",
		1, 1, 1, &dhcp_bound_fake_ops, &fake, &rollback_status);
	assert(transaction_status == EIO && rollback_status == 0);
	assert(strcmp(fake.events, "dCSAsa") == 0);

	memset(&fake, 0, sizeof(fake));
	fake.publish_status = ENOSPC;
	transaction_status = dhcp_lease_bound_apply(
		"192.0.2.10", "255.255.255.0", "192.0.2.1",
		"198.51.100.20", "255.255.255.128", "198.51.100.1",
		1, 1, 1, &dhcp_bound_fake_ops, &fake, &rollback_status);
	assert(transaction_status == ENOSPC && rollback_status == 0);
	assert(strcmp(fake.events, "dCSAPDsa") == 0);

	memset(&fake, 0, sizeof(fake));
	fake.add_new_status = EEXIST;
	fake.publish_status = ENOSPC;
	transaction_status = dhcp_lease_bound_apply(
		"192.0.2.10", "255.255.255.0", "192.0.2.1",
		"198.51.100.20", "255.255.255.128", "198.51.100.1",
		1, 1, 1, &dhcp_bound_fake_ops, &fake, &rollback_status);
	assert(transaction_status == ENOSPC && rollback_status == 0);
	assert(strcmp(fake.events, "dCSAPsa") == 0);

	memset(&fake, 0, sizeof(fake));
	fake.publish_status = ENOSPC;
	fake.delete_new_status = EIO;
	fake.set_old_status = EIO;
	fake.add_old_status = EIO;
	transaction_status = dhcp_lease_bound_apply(
		"192.0.2.10", "255.255.255.0", "192.0.2.1",
		"198.51.100.20", "255.255.255.128", "198.51.100.1",
		1, 1, 1, &dhcp_bound_fake_ops, &fake, &rollback_status);
	assert(transaction_status == ENOSPC && rollback_status == EIO);
	assert(strcmp(fake.events, "dCSAPDsa") == 0);

	memset(&fake, 0, sizeof(fake));
	fake.delete_old_status = ESRCH;
	transaction_status = dhcp_lease_bound_apply(
		"192.0.2.10", "255.255.255.0", "192.0.2.1",
		"192.0.2.10", "255.255.255.0", "198.51.100.1",
		0, 0, 1, &dhcp_bound_fake_ops, &fake, &rollback_status);
	assert(transaction_status == 0 && rollback_status == 0);
	assert(strcmp(fake.events, "dAP") == 0);

	memset(&fake, 0, sizeof(fake));
	transaction_status = dhcp_lease_bound_apply(
		"192.0.2.10", "255.255.255.0", "192.0.2.1",
		"192.0.2.10", "255.255.255.0", "",
		0, 0, 1, &dhcp_bound_fake_ops, &fake, &rollback_status);
	assert(transaction_status == 0 && rollback_status == 0);
	assert(strcmp(fake.events, "dP") == 0);

	memset(&fake, 0, sizeof(fake));
	fake.add_old_status = EEXIST;
	transaction_status = dhcp_lease_bound_apply(
		"192.0.2.10", "255.255.255.0", "192.0.2.1",
		"192.0.2.10", "255.255.255.0", "192.0.2.1",
		0, 0, 1, &dhcp_bound_fake_ops, &fake, &rollback_status);
	assert(transaction_status == 0 && rollback_status == 0);
	assert(strcmp(fake.events, "aP") == 0);


	/* A withdrawn IPTV lease stays deconfigured when route cleanup fails.
	 * The route inventory remains available for BusyBox's callback retry. */
	memset(&viptv_fake, 0, sizeof(viptv_fake));
	transaction_status = dhcp_lease_viptv_deconfig_apply(
		&dhcp_viptv_deconfig_fake_ops, &viptv_fake);
	assert(transaction_status == 0);
	assert(strcmp(viptv_fake.events, "CRP") == 0);
	assert(viptv_fake.published == 1);

	memset(&viptv_fake, 0, sizeof(viptv_fake));
	viptv_fake.clear_status = EIO;
	transaction_status = dhcp_lease_viptv_deconfig_apply(
		&dhcp_viptv_deconfig_fake_ops, &viptv_fake);
	assert(transaction_status == EIO);
	assert(strcmp(viptv_fake.events, "CR") == 0);
	assert(viptv_fake.published == 0);

	memset(&viptv_fake, 0, sizeof(viptv_fake));
	viptv_fake.clear_status = EBUSY;
	viptv_fake.route_status = ENOSPC;
	transaction_status = dhcp_lease_viptv_deconfig_apply(
		&dhcp_viptv_deconfig_fake_ops, &viptv_fake);
	assert(transaction_status == ENOSPC);
	assert(strcmp(viptv_fake.events, "CR") == 0);
	assert(viptv_fake.published == 0);

	memset(&viptv_fake, 0, sizeof(viptv_fake));
	viptv_fake.route_status = EIO;
	transaction_status = dhcp_lease_viptv_deconfig_apply(
		&dhcp_viptv_deconfig_fake_ops, &viptv_fake);
	assert(transaction_status == EIO);
	assert(strcmp(viptv_fake.events, "CR") == 0);
	assert(viptv_fake.published == 0);
	/* DECONFIG_PENDING retry: clear is idempotent, then route cleanup can
	 * complete and only then is the old lease identity withdrawn. */
	viptv_fake.route_status = 0;
	transaction_status = dhcp_lease_viptv_deconfig_apply(
		&dhcp_viptv_deconfig_fake_ops, &viptv_fake);
	assert(transaction_status == 0);
	assert(strcmp(viptv_fake.events, "CRCRP") == 0);
	assert(viptv_fake.published == 1);

	/* A failed publication is propagated; the deconfig retry may complete it. */
	memset(&viptv_fake, 0, sizeof(viptv_fake));
	viptv_fake.publish_status = ENOSPC;
	transaction_status = dhcp_lease_viptv_deconfig_apply(
		&dhcp_viptv_deconfig_fake_ops, &viptv_fake);
	assert(transaction_status == ENOSPC);
	assert(strcmp(viptv_fake.events, "CRP") == 0);
	viptv_fake.publish_status = 0;
	transaction_status = dhcp_lease_viptv_deconfig_apply(
		&dhcp_viptv_deconfig_fake_ops, &viptv_fake);
	assert(transaction_status == 0);
	assert(strcmp(viptv_fake.events, "CRPCRP") == 0);
	assert(viptv_fake.published == 2);

	test_dhcp_viptv_deconfig_metadata_clear_rolls_back_as_one_tuple();
	test_lan_dhcp_nvram_publish_compensates_partial_updates();
	test_dhcp_lease_nvram_publish_rejects_stale_snapshot();
	test_dhcp_lease_nvram_restore_if_current_protects_newer_writer();
	test_dhcp_lease_nvram_publish_attempts_all_and_restores_all();
	test_dhcp_lease_6rd_snapshot_failure_restores_entire_tuple();
	test_dhcp_lease_nvram_reverse_snapshot_validates_complete_tuple();
	test_dhcp_lease_text_fits_buffer();
	test_dhcp_lease_nvram_publish_restores_full_wan_bound_tuple();
	test_dhcp_lease_fast_renew_publishes_dltime_as_one_checked_value();
	puts("DHCP lease input policy regression: OK");
	return 0;
}
