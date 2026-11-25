#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "wan6_identity_policy.h"

struct fake_identity_backend {
	int set_status[4];
	int set_calls;
	int stop_status;
	int stop_calls;
	int unit[4];
	char names[4][16];
	char values[4][32];
};

static int
fake_set_value(void *opaque, int unit, const char *param_name,
		const char *value)
{
	struct fake_identity_backend *fake = (struct fake_identity_backend *)opaque;
	int call = fake->set_calls++;

	assert(call >= 0 && call < 4);
	fake->unit[call] = unit;
	snprintf(fake->names[call], sizeof(fake->names[call]), "%s", param_name);
	snprintf(fake->values[call], sizeof(fake->values[call]), "%s", value);
	return fake->set_status[call];
}

static int
fake_stop_sit(void *opaque)
{
	struct fake_identity_backend *fake = (struct fake_identity_backend *)opaque;

	++fake->stop_calls;
	return fake->stop_status;
}


struct fake_wan6_addr_lookup {
	int status;
	int found;
	int calls;
	const char *address;
	char ifname[16];
};

static int
fake_wan6_get_addr6(void *opaque, const char *ifname, char *address,
		    int *found)
{
	struct fake_wan6_addr_lookup *fake =
		(struct fake_wan6_addr_lookup *)opaque;

	++fake->calls;
	snprintf(fake->ifname, sizeof(fake->ifname), "%s", ifname);
	if (fake->address)
		snprintf(address, 64, "%s", fake->address);
	*found = fake->found;
	return fake->status;
}

static void
test_wan6_address_host_read_error_fails_closed(void)
{
	struct fake_wan6_addr_lookup fake;
	char address[64] = "stale";
	const char *host = "stale";

	memset(&fake, 0, sizeof(fake));
	fake.found = 1;
	fake.address = "2001:db8::1/64";
	assert(wan6_identity_resolve_addr6_host(
		EACCES, "ppp0", address, &host,
		fake_wan6_get_addr6, &fake) == EACCES);
	assert(host == NULL && address[0] == '\0');
	assert(fake.calls == 0);
}

static void
test_wan6_address_host_missing_interface_keeps_fallback_eligible(void)
{
	struct fake_wan6_addr_lookup fake;
	char address[64] = "stale";
	const char *host = "stale";

	memset(&fake, 0, sizeof(fake));
	assert(wan6_identity_resolve_addr6_host(
		0, "", address, &host,
		fake_wan6_get_addr6, &fake) == 0);
	assert(host == NULL && address[0] == '\0');
	assert(fake.calls == 0);
}

static void
test_wan6_address_host_extracts_prefix_free_public_address(void)
{
	struct fake_wan6_addr_lookup fake;
	char address[64] = "";
	const char *host = NULL;

	memset(&fake, 0, sizeof(fake));
	fake.found = 1;
	fake.address = "2001:db8::1/64";
	assert(wan6_identity_resolve_addr6_host(
		0, "ppp0", address, &host,
		fake_wan6_get_addr6, &fake) == 0);
	assert(fake.calls == 1 && strcmp(fake.ifname, "ppp0") == 0);
	assert(host == address && strcmp(host, "2001:db8::1") == 0);
}

static void
test_wan6_address_host_does_not_hide_address_enumeration_error(void)
{
	struct fake_wan6_addr_lookup fake;
	char address[64] = "stale";
	const char *host = "stale";

	memset(&fake, 0, sizeof(fake));
	fake.status = EIO;
	assert(wan6_identity_resolve_addr6_host(
		0, "ppp0", address, &host,
		fake_wan6_get_addr6, &fake) == EIO);
	assert(fake.calls == 1);
	assert(host == NULL && address[0] == '\0');
}

static void
test_wan6_address_host_no_address_is_a_non_error(void)
{
	struct fake_wan6_addr_lookup fake;
	char address[64] = "stale";
	const char *host = "stale";

	memset(&fake, 0, sizeof(fake));
	assert(wan6_identity_resolve_addr6_host(
		0, "ppp0", address, &host,
		fake_wan6_get_addr6, &fake) == 0);
	assert(fake.calls == 1 && host == NULL);
}

static void
test_wan6_address_host_rejects_invalid_arguments(void)
{
	struct fake_wan6_addr_lookup fake;
	char address[64];
	const char *host = NULL;

	memset(&fake, 0, sizeof(fake));
	assert(wan6_identity_resolve_addr6_host(
		0, NULL, address, &host,
		fake_wan6_get_addr6, &fake) == EINVAL);
	assert(wan6_identity_resolve_addr6_host(
		0, "ppp0", NULL, &host,
		fake_wan6_get_addr6, &fake) == EINVAL);
	assert(fake.calls == 0);
}


struct fake_lan_addr6_backend {
	int lookup_status;
	int found;
	int publish_status;
	int lookup_calls;
	int publish_calls;
	const char *address;
	char published[64];
};

static int
fake_lan_addr6_lookup(void *opaque, char *address, size_t address_size,
		      int *found)
{
	struct fake_lan_addr6_backend *fake =
		(struct fake_lan_addr6_backend *)opaque;

	++fake->lookup_calls;
	if (fake->lookup_status != 0)
		return fake->lookup_status;
	*found = fake->found;
	if (fake->found && fake->address) {
		if (strlen(fake->address) >= address_size)
			return ENAMETOOLONG;
		snprintf(address, address_size, "%s", fake->address);
	}
	return 0;
}

static int
fake_lan_addr6_publish(void *opaque, const char *address)
{
	struct fake_lan_addr6_backend *fake =
		(struct fake_lan_addr6_backend *)opaque;

	++fake->publish_calls;
	snprintf(fake->published, sizeof(fake->published), "%s", address);
	return fake->publish_status;
}

static void
test_lan_addr6_read_error_is_not_a_successful_noop(void)
{
	struct fake_lan_addr6_backend fake;
	char address[64] = "stale";

	memset(&fake, 0, sizeof(fake));
	fake.lookup_status = EIO;
	fake.found = 1;
	fake.address = "2001:db8::10/64";
	assert(wan6_identity_publish_lan_addr6(
		fake_lan_addr6_lookup, fake_lan_addr6_publish,
		&fake, address, sizeof(address)) == EIO);
	assert(fake.lookup_calls == 1 && fake.publish_calls == 0);
	assert(address[0] == '\0');
}

static void
test_lan_addr6_clean_miss_preserves_last_published_value(void)
{
	struct fake_lan_addr6_backend fake;
	char address[64] = "stale";

	memset(&fake, 0, sizeof(fake));
	assert(wan6_identity_publish_lan_addr6(
		fake_lan_addr6_lookup, fake_lan_addr6_publish,
		&fake, address, sizeof(address)) == 0);
	assert(fake.lookup_calls == 1 && fake.publish_calls == 0);
	assert(address[0] == '\0');
}

static void
test_lan_addr6_success_publishes_one_complete_address(void)
{
	struct fake_lan_addr6_backend fake;
	char address[64] = "";

	memset(&fake, 0, sizeof(fake));
	fake.found = 1;
	fake.address = "2001:db8::10/64";
	assert(wan6_identity_publish_lan_addr6(
		fake_lan_addr6_lookup, fake_lan_addr6_publish,
		&fake, address, sizeof(address)) == 0);
	assert(fake.lookup_calls == 1 && fake.publish_calls == 1);
	assert(strcmp(fake.published, "2001:db8::10/64") == 0);
}

static void
test_lan_addr6_publish_error_is_propagated(void)
{
	struct fake_lan_addr6_backend fake;
	char address[64] = "";

	memset(&fake, 0, sizeof(fake));
	fake.found = 1;
	fake.address = "2001:db8::10/64";
	fake.publish_status = EACCES;
	assert(wan6_identity_publish_lan_addr6(
		fake_lan_addr6_lookup, fake_lan_addr6_publish,
		&fake, address, sizeof(address)) == EACCES);
	assert(fake.publish_calls == 1);
}

static void
test_lan_addr6_policy_rejects_invalid_arguments(void)
{
	struct fake_lan_addr6_backend fake;
	char address[64];

	memset(&fake, 0, sizeof(fake));
	assert(wan6_identity_publish_lan_addr6(
		NULL, fake_lan_addr6_publish, &fake, address,
		sizeof(address)) == EINVAL);
	assert(wan6_identity_publish_lan_addr6(
		fake_lan_addr6_lookup, NULL, &fake, address,
		sizeof(address)) == EINVAL);
	assert(wan6_identity_publish_lan_addr6(
		fake_lan_addr6_lookup, fake_lan_addr6_publish,
		&fake, NULL, sizeof(address)) == EINVAL);
	assert(fake.lookup_calls == 0 && fake.publish_calls == 0);
}

static void
test_sit_identity_commit_success(void)
{
	struct fake_identity_backend fake;
	int rollback_status = -1;

	memset(&fake, 0, sizeof(fake));
	assert(wan6_publish_sit_identity(fake_set_value, fake_stop_sit,
		&fake, 2, "sit0", &rollback_status) == 0);
	assert(rollback_status == 0);
	assert(fake.set_calls == 1 && fake.stop_calls == 0);
	assert(fake.unit[0] == 2);
	assert(strcmp(fake.names[0], "ifname6") == 0);
	assert(strcmp(fake.values[0], "sit0") == 0);
}

static void
test_sit_identity_failure_removes_tunnel_and_clears_partial_identity(void)
{
	struct fake_identity_backend fake;
	int rollback_status = 0;

	memset(&fake, 0, sizeof(fake));
	fake.set_status[0] = EIO;
	assert(wan6_publish_sit_identity(fake_set_value, fake_stop_sit,
		&fake, 1, "sit0", &rollback_status) == EIO);
	assert(rollback_status == 0);
	assert(fake.set_calls == 2 && fake.stop_calls == 1);
	assert(strcmp(fake.names[1], "ifname6") == 0);
	assert(strcmp(fake.values[1], "") == 0);
}

static void
test_sit_identity_failure_retains_state_when_tunnel_removal_fails(void)
{
	struct fake_identity_backend fake;
	int rollback_status = 0;

	memset(&fake, 0, sizeof(fake));
	fake.set_status[0] = ENOSPC;
	fake.stop_status = EBUSY;
	assert(wan6_publish_sit_identity(fake_set_value, fake_stop_sit,
		&fake, 1, "sit0", &rollback_status) == ENOSPC);
	assert(rollback_status == EBUSY);
	assert(fake.set_calls == 1 && fake.stop_calls == 1);
}

static void
test_sit_identity_failure_reports_metadata_clear_failure(void)
{
	struct fake_identity_backend fake;
	int rollback_status = 0;

	memset(&fake, 0, sizeof(fake));
	fake.set_status[0] = EIO;
	fake.set_status[1] = EPERM;
	assert(wan6_publish_sit_identity(fake_set_value, fake_stop_sit,
		&fake, 1, "sit0", &rollback_status) == EIO);
	assert(rollback_status == EPERM);
	assert(fake.set_calls == 2 && fake.stop_calls == 1);
}

static void
test_native_identity_is_retained_when_kernel_rollback_incomplete(void)
{
	struct fake_identity_backend fake;

	memset(&fake, 0, sizeof(fake));
	assert(wan6_clear_native_identity_after_rollback(
		fake_set_value, &fake, 2, EBUSY) == EBUSY);
	assert(fake.set_calls == 0);
}

static void
test_native_identity_is_cleared_after_complete_kernel_rollback(void)
{
	struct fake_identity_backend fake;

	memset(&fake, 0, sizeof(fake));
	assert(wan6_clear_native_identity_after_rollback(
		fake_set_value, &fake, 1, 0) == 0);
	assert(fake.set_calls == 1);
	assert(fake.unit[0] == 1);
	assert(strcmp(fake.names[0], "ifname6") == 0);
	assert(strcmp(fake.values[0], "") == 0);
}

static void
test_native_identity_clear_failure_is_reported(void)
{
	struct fake_identity_backend fake;

	memset(&fake, 0, sizeof(fake));
	fake.set_status[0] = ENOSPC;
	assert(wan6_clear_native_identity_after_rollback(
		fake_set_value, &fake, 1, 0) == ENOSPC);
	assert(fake.set_calls == 1);
	assert(strcmp(fake.names[0], "ifname6") == 0);
}

static void
test_metadata_clear_attempts_both_fields_and_preserves_first_error(void)
{
	struct fake_identity_backend fake;

	memset(&fake, 0, sizeof(fake));
	fake.set_status[0] = EACCES;
	assert(wan6_clear_runtime_metadata(fake_set_value, &fake, 3, 1) == EACCES);
	assert(fake.set_calls == 2);
	assert(strcmp(fake.names[0], "dns6") == 0);
	assert(strcmp(fake.values[0], "") == 0);
	assert(strcmp(fake.names[1], "ifname6") == 0);
	assert(strcmp(fake.values[1], "") == 0);
}

static void
test_metadata_clear_can_preserve_dns_when_requested(void)
{
	struct fake_identity_backend fake;

	memset(&fake, 0, sizeof(fake));
	assert(wan6_clear_runtime_metadata(fake_set_value, &fake, 0, 0) == 0);
	assert(fake.set_calls == 1);
	assert(strcmp(fake.names[0], "ifname6") == 0);
}

static void
test_metadata_clear_rejects_invalid_arguments_before_side_effects(void)
{
	struct fake_identity_backend fake;

	memset(&fake, 0, sizeof(fake));
	assert(wan6_clear_runtime_metadata(NULL, &fake, 0, 1) == EINVAL);
	assert(wan6_clear_runtime_metadata(fake_set_value, &fake, -1, 1) == EINVAL);
	assert(wan6_publish_sit_identity(NULL, fake_stop_sit, &fake,
		0, "sit0", NULL) == EINVAL);
	assert(wan6_publish_sit_identity(fake_set_value, NULL, &fake,
		0, "sit0", NULL) == EINVAL);
	assert(wan6_publish_sit_identity(fake_set_value, fake_stop_sit, &fake,
		0, "", NULL) == EINVAL);
	assert(fake.set_calls == 0 && fake.stop_calls == 0);
}


struct fake_sit_lan_addr6_backend {
	int clear_status;
	int add_status;
	int clear_calls;
	int add_calls;
	char restored_address[64];
};

static int
fake_sit_lan_addr6_clear(void *opaque)
{
	struct fake_sit_lan_addr6_backend *fake =
		(struct fake_sit_lan_addr6_backend *)opaque;

	++fake->clear_calls;
	return fake->clear_status;
}

static int
fake_sit_lan_addr6_add(void *opaque, const char *address)
{
	struct fake_sit_lan_addr6_backend *fake =
		(struct fake_sit_lan_addr6_backend *)opaque;

	++fake->add_calls;
	snprintf(fake->restored_address, sizeof(fake->restored_address),
		 "%s", address);
	return fake->add_status;
}

static void
test_sit_lan_addr6_failure_restores_previous_address(void)
{
	struct fake_sit_lan_addr6_backend fake;
	int rollback_status = -1;

	memset(&fake, 0, sizeof(fake));
	assert(wan6_sit_restore_lan_addr6(
		1, "2001:db8:1::1/64", fake_sit_lan_addr6_clear,
		fake_sit_lan_addr6_add, &fake, &rollback_status) == 0);
	assert(rollback_status == 0);
	assert(fake.clear_calls == 1 && fake.add_calls == 1);
	assert(strcmp(fake.restored_address, "2001:db8:1::1/64") == 0);
}

static void
test_sit_lan_addr6_empty_snapshot_only_clears_partial_address(void)
{
	struct fake_sit_lan_addr6_backend fake;
	int rollback_status = -1;

	memset(&fake, 0, sizeof(fake));
	assert(wan6_sit_restore_lan_addr6(
		1, "", fake_sit_lan_addr6_clear,
		fake_sit_lan_addr6_add, &fake, &rollback_status) == 0);
	assert(rollback_status == 0);
	assert(fake.clear_calls == 1 && fake.add_calls == 0);
}

static void
test_sit_lan_addr6_restore_attempts_add_after_clear_failure(void)
{
	struct fake_sit_lan_addr6_backend fake;
	int rollback_status = 0;

	memset(&fake, 0, sizeof(fake));
	fake.clear_status = EBUSY;
	fake.add_status = EIO;
	assert(wan6_sit_restore_lan_addr6(
		1, "2001:db8:1::1/64", fake_sit_lan_addr6_clear,
		fake_sit_lan_addr6_add, &fake, &rollback_status) == EBUSY);
	assert(rollback_status == EBUSY);
	assert(fake.clear_calls == 1 && fake.add_calls == 1);
	assert(strcmp(fake.restored_address, "2001:db8:1::1/64") == 0);
}

static void
test_sit_lan_addr6_no_mutation_skips_compensation(void)
{
	struct fake_sit_lan_addr6_backend fake;
	int rollback_status = -1;

	memset(&fake, 0, sizeof(fake));
	assert(wan6_sit_restore_lan_addr6(
		0, NULL, NULL, NULL, &fake, &rollback_status) == 0);
	assert(rollback_status == 0);
	assert(fake.clear_calls == 0 && fake.add_calls == 0);
}

static void
test_sit_lan_addr6_restore_rejects_missing_callbacks(void)
{
	struct fake_sit_lan_addr6_backend fake;

	memset(&fake, 0, sizeof(fake));
	assert(wan6_sit_restore_lan_addr6(
		1, "2001:db8:1::1/64", NULL,
		fake_sit_lan_addr6_add, &fake, NULL) == EINVAL);
	assert(fake.clear_calls == 0 && fake.add_calls == 0);
}


static void
test_sit_endpoint_route_new_add_is_owned(void)
{
	int route_added = -1;

	assert(wan6_sit_endpoint_route_add_status(0, &route_added) == 0);
	assert(route_added == 1);
}

static void
test_sit_endpoint_route_duplicate_is_accepted_without_ownership(void)
{
	int route_added = -1;

	assert(wan6_sit_endpoint_route_add_status(EEXIST, &route_added) == 0);
	assert(route_added == 0);
}

static void
test_sit_endpoint_route_failure_is_not_hidden(void)
{
	int route_added = -1;

	assert(wan6_sit_endpoint_route_add_status(ENETUNREACH,
						  &route_added) == ENETUNREACH);
	assert(route_added == 0);
}

static void
test_sit_endpoint_route_requires_ownership_output(void)
{
	assert(wan6_sit_endpoint_route_add_status(0, NULL) == EINVAL);
}


struct fake_sit_route4_state {
	int delete_status;
	int clear_status;
	int delete_calls;
	int clear_calls;
	int clear_unit;
	char ifname[WAN6_SIT_ROUTE4_IFNAME_SIZE];
	char endpoint[WAN6_SIT_ROUTE4_IPV4_SIZE];
	char gateway[WAN6_SIT_ROUTE4_IPV4_SIZE];
};

static int
fake_sit_route4_delete(void *context, const char *ifname,
		       const char *endpoint, const char *gateway)
{
	struct fake_sit_route4_state *state =
		(struct fake_sit_route4_state *)context;

	state->delete_calls++;
	snprintf(state->ifname, sizeof(state->ifname), "%s", ifname);
	snprintf(state->endpoint, sizeof(state->endpoint), "%s", endpoint);
	snprintf(state->gateway, sizeof(state->gateway), "%s", gateway);
	return state->delete_status;
}

static int
fake_sit_route4_clear(void *context, int unit)
{
	struct fake_sit_route4_state *state =
		(struct fake_sit_route4_state *)context;

	state->clear_calls++;
	state->clear_unit = unit;
	return state->clear_status;
}

static void
test_sit_route4_record_format_and_parse_round_trip(void)
{
	char record[WAN6_SIT_ROUTE4_RECORD_SIZE];
	char ifname[WAN6_SIT_ROUTE4_IFNAME_SIZE];
	char endpoint[WAN6_SIT_ROUTE4_IPV4_SIZE];
	char gateway[WAN6_SIT_ROUTE4_IPV4_SIZE];

	assert(wan6_sit_route4_record_format(
		"eth2", "192.0.2.1", "192.0.2.254",
		record, sizeof(record)) == 0);
	assert(strcmp(record, "eth2|192.0.2.1|192.0.2.254") == 0);
	assert(wan6_sit_route4_record_parse(
		record, ifname, sizeof(ifname), endpoint, sizeof(endpoint),
		gateway, sizeof(gateway)) == 0);
	assert(strcmp(ifname, "eth2") == 0);
	assert(strcmp(endpoint, "192.0.2.1") == 0);
	assert(strcmp(gateway, "192.0.2.254") == 0);
}

static void
test_sit_route4_record_allows_direct_endpoint_route(void)
{
	char ifname[WAN6_SIT_ROUTE4_IFNAME_SIZE];
	char endpoint[WAN6_SIT_ROUTE4_IPV4_SIZE];
	char gateway[WAN6_SIT_ROUTE4_IPV4_SIZE];

	assert(wan6_sit_route4_record_parse(
		"eth2|192.0.2.1|", ifname, sizeof(ifname),
		endpoint, sizeof(endpoint), gateway, sizeof(gateway)) == 0);
	assert(strcmp(gateway, "") == 0);
}

static void
test_sit_route4_record_rejects_ambiguous_delimiters(void)
{
	char record[WAN6_SIT_ROUTE4_RECORD_SIZE];
	char ifname[WAN6_SIT_ROUTE4_IFNAME_SIZE];
	char endpoint[WAN6_SIT_ROUTE4_IPV4_SIZE];
	char gateway[WAN6_SIT_ROUTE4_IPV4_SIZE];

	assert(wan6_sit_route4_record_format(
		"eth|2", "192.0.2.1", "192.0.2.254",
		record, sizeof(record)) == EINVAL);
	assert(wan6_sit_route4_record_parse(
		"eth2|192.0.2.1|192.0.2.254|extra",
		ifname, sizeof(ifname), endpoint, sizeof(endpoint),
		gateway, sizeof(gateway)) == EINVAL);
}

static void
test_sit_route4_cleanup_deletes_exact_owned_route_then_clears_record(void)
{
	struct fake_sit_route4_state state = {0};

	assert(wan6_sit_route4_cleanup_record(
		3, "eth2|192.0.2.1|192.0.2.254",
		fake_sit_route4_delete, fake_sit_route4_clear, &state) == 0);
	assert(state.delete_calls == 1);
	assert(state.clear_calls == 1);
	assert(state.clear_unit == 3);
	assert(strcmp(state.ifname, "eth2") == 0);
	assert(strcmp(state.endpoint, "192.0.2.1") == 0);
	assert(strcmp(state.gateway, "192.0.2.254") == 0);
}

static void
test_sit_route4_cleanup_preserves_record_when_route_delete_fails(void)
{
	struct fake_sit_route4_state state = {0};

	state.delete_status = EIO;
	assert(wan6_sit_route4_cleanup_record(
		0, "eth2|192.0.2.1|192.0.2.254",
		fake_sit_route4_delete, fake_sit_route4_clear, &state) == EIO);
	assert(state.delete_calls == 1);
	assert(state.clear_calls == 0);
}

static void
test_sit_route4_cleanup_clears_record_when_route_is_already_absent(void)
{
	struct fake_sit_route4_state state = {0};

	state.delete_status = ESRCH;
	assert(wan6_sit_route4_cleanup_record(
		0, "eth2|192.0.2.1|192.0.2.254",
		fake_sit_route4_delete, fake_sit_route4_clear, &state) == 0);
	assert(state.delete_calls == 1);
	assert(state.clear_calls == 1);
}

static void
test_sit_route4_cleanup_clears_record_when_interface_is_gone(void)
{
	struct fake_sit_route4_state state = {0};

	state.delete_status = ENODEV;
	assert(wan6_sit_route4_cleanup_record(
		0, "eth2|192.0.2.1|192.0.2.254",
		fake_sit_route4_delete, fake_sit_route4_clear, &state) == 0);
	assert(state.delete_calls == 1);
	assert(state.clear_calls == 1);
}

static void
test_sit_route4_cleanup_retains_record_when_clear_fails(void)
{
	struct fake_sit_route4_state state = {0};

	state.clear_status = EIO;
	assert(wan6_sit_route4_cleanup_record(
		1, "eth2|192.0.2.1|192.0.2.254",
		fake_sit_route4_delete, fake_sit_route4_clear, &state) == EIO);
	assert(state.delete_calls == 1);
	assert(state.clear_calls == 1);
}

static void
test_sit_route4_cleanup_rejects_malformed_record_without_mutation(void)
{
	struct fake_sit_route4_state state = {0};

	assert(wan6_sit_route4_cleanup_record(
		0, "eth2|bad",
		fake_sit_route4_delete, fake_sit_route4_clear, &state) == EINVAL);
	assert(state.delete_calls == 0);
	assert(state.clear_calls == 0);
}

static void
test_sit_route4_cleanup_empty_record_is_noop(void)
{
	struct fake_sit_route4_state state = {0};

	assert(wan6_sit_route4_cleanup_record(
		0, "", fake_sit_route4_delete, fake_sit_route4_clear,
		&state) == 0);
	assert(state.delete_calls == 0);
	assert(state.clear_calls == 0);
}

int
main(void)
{

	test_sit_endpoint_route_new_add_is_owned();
	test_sit_endpoint_route_duplicate_is_accepted_without_ownership();
	test_sit_endpoint_route_failure_is_not_hidden();
	test_sit_endpoint_route_requires_ownership_output();
	test_sit_route4_record_format_and_parse_round_trip();
	test_sit_route4_record_allows_direct_endpoint_route();
	test_sit_route4_record_rejects_ambiguous_delimiters();
	test_sit_route4_cleanup_deletes_exact_owned_route_then_clears_record();
	test_sit_route4_cleanup_preserves_record_when_route_delete_fails();
	test_sit_route4_cleanup_clears_record_when_route_is_already_absent();
	test_sit_route4_cleanup_clears_record_when_interface_is_gone();
	test_sit_route4_cleanup_retains_record_when_clear_fails();
	test_sit_route4_cleanup_rejects_malformed_record_without_mutation();
	test_sit_route4_cleanup_empty_record_is_noop();
	test_wan6_address_host_read_error_fails_closed();
	test_wan6_address_host_missing_interface_keeps_fallback_eligible();
	test_wan6_address_host_extracts_prefix_free_public_address();
	test_wan6_address_host_does_not_hide_address_enumeration_error();
	test_wan6_address_host_no_address_is_a_non_error();
	test_wan6_address_host_rejects_invalid_arguments();

	test_lan_addr6_read_error_is_not_a_successful_noop();
	test_lan_addr6_clean_miss_preserves_last_published_value();
	test_lan_addr6_success_publishes_one_complete_address();
	test_lan_addr6_publish_error_is_propagated();
	test_lan_addr6_policy_rejects_invalid_arguments();
	test_sit_identity_commit_success();
	test_sit_identity_failure_removes_tunnel_and_clears_partial_identity();
	test_sit_identity_failure_retains_state_when_tunnel_removal_fails();
	test_sit_identity_failure_reports_metadata_clear_failure();
	test_native_identity_is_retained_when_kernel_rollback_incomplete();
	test_native_identity_is_cleared_after_complete_kernel_rollback();
	test_native_identity_clear_failure_is_reported();
	test_metadata_clear_attempts_both_fields_and_preserves_first_error();
	test_metadata_clear_can_preserve_dns_when_requested();
	test_metadata_clear_rejects_invalid_arguments_before_side_effects();
	test_sit_lan_addr6_failure_restores_previous_address();
	test_sit_lan_addr6_empty_snapshot_only_clears_partial_address();
	test_sit_lan_addr6_restore_attempts_add_after_clear_failure();
	test_sit_lan_addr6_no_mutation_skips_compensation();
	test_sit_lan_addr6_restore_rejects_missing_callbacks();
	puts("WAN IPv6 identity publication policy: PASS");
	return 0;
}
