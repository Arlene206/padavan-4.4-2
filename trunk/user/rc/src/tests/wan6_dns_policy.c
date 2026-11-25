#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "wan6_dns_policy.h"

static void
test_withdrawn_dns_option_clears_previous_lease(void)
{
	assert(wan6_dns_update_required("2001:db8::53", NULL));
	assert(wan6_dns_update_required("2001:db8::53", ""));
}

static void
test_absent_dns_option_is_noop_when_already_empty(void)
{
	assert(!wan6_dns_update_required(NULL, NULL));
	assert(!wan6_dns_update_required("", NULL));
}

static void
test_changed_dns_list_is_published(void)
{
	assert(wan6_dns_update_required("2001:db8::53", "2001:db8::54"));
}

static void
test_unchanged_dns_list_is_not_republished(void)
{
	assert(!wan6_dns_update_required("2001:db8::53", "2001:db8::53"));
}


struct fake_dns_writer {
	int status;
	int calls;
	char value[256];
};

static int
fake_dns_set_value(void *opaque, const char *value)
{
	struct fake_dns_writer *fake = (struct fake_dns_writer *)opaque;

	++fake->calls;
	snprintf(fake->value, sizeof(fake->value), "%s", value);
	return fake->status;
}

static void
test_dns_store_publishes_changed_value(void)
{
	struct fake_dns_writer fake;

	memset(&fake, 0, sizeof(fake));
	assert(wan6_dns_store_if_changed("2001:db8::1", "2001:db8::2",
		fake_dns_set_value, &fake) == 1);
	assert(fake.calls == 1);
	assert(strcmp(fake.value, "2001:db8::2") == 0);
}

static void
test_dns_store_skips_unchanged_value(void)
{
	struct fake_dns_writer fake;

	memset(&fake, 0, sizeof(fake));
	assert(wan6_dns_store_if_changed("2001:db8::1", "2001:db8::1",
		fake_dns_set_value, &fake) == 0);
	assert(fake.calls == 0);
}

static void
test_dns_store_publishes_withdrawal(void)
{
	struct fake_dns_writer fake;

	memset(&fake, 0, sizeof(fake));
	assert(wan6_dns_store_if_changed("2001:db8::1", "",
		fake_dns_set_value, &fake) == 1);
	assert(fake.calls == 1);
	assert(strcmp(fake.value, "") == 0);
}

static void
test_dns_store_fails_closed_when_current_value_read_fails(void)
{
	struct fake_dns_writer fake;

	memset(&fake, 0, sizeof(fake));
	assert(wan6_dns_store_after_checked_read(EACCES, "", "",
		fake_dns_set_value, &fake) == -EACCES);
	assert(fake.calls == 0);
}

static void
test_dns_store_treats_missing_current_value_as_empty(void)
{
	struct fake_dns_writer fake;

	memset(&fake, 0, sizeof(fake));
	assert(wan6_dns_store_after_checked_read(0, "", "2001:db8::53",
		fake_dns_set_value, &fake) == 1);
	assert(fake.calls == 1);
	assert(strcmp(fake.value, "2001:db8::53") == 0);
}

static void
test_dns_store_propagates_nvram_failure(void)
{
	struct fake_dns_writer fake;

	memset(&fake, 0, sizeof(fake));
	fake.status = EIO;
	assert(wan6_dns_store_if_changed("2001:db8::1", "2001:db8::2",
		fake_dns_set_value, &fake) == -EIO);
	assert(fake.calls == 1);
}

static void
test_dns_store_rejects_missing_writer(void)
{
	assert(wan6_dns_store_if_changed("2001:db8::1", "2001:db8::2",
		NULL, NULL) == -EINVAL);
}


struct fake_dns_nvram {
	const char *names[WAN6_DNS_CONFIG_VALUE_COUNT];
	const char *values[WAN6_DNS_CONFIG_VALUE_COUNT];
	int read_fail_at;
	int missing_at;
	int begin_error;
	int end_error;
	int begin_calls;
	int read_calls;
	int end_calls;
	char cache_slot[64];
};

static int
fake_dns_snapshot_begin(void *opaque)
{
	struct fake_dns_nvram *fake = (struct fake_dns_nvram *)opaque;

	++fake->begin_calls;
	if (fake->begin_error) {
		errno = fake->begin_error;
		return -1;
	}
	return 0;
}

static int
fake_dns_snapshot_get(void *opaque, const char *name, char **value)
{
	struct fake_dns_nvram *fake = (struct fake_dns_nvram *)opaque;
	int index = fake->read_calls++;

	assert(index < WAN6_DNS_CONFIG_VALUE_COUNT);
	assert(strcmp(name, fake->names[index]) == 0);
	if (index == fake->read_fail_at) {
		*value = NULL;
		errno = EACCES;
		return -1;
	}
	if (index == fake->missing_at) {
		*value = NULL;
		return 0;
	}
	snprintf(fake->cache_slot, sizeof(fake->cache_slot), "%s",
		 fake->values[index]);
	*value = fake->cache_slot;
	return 1;
}

static int
fake_dns_snapshot_end(void *opaque)
{
	struct fake_dns_nvram *fake = (struct fake_dns_nvram *)opaque;

	++fake->end_calls;
	if (fake->end_error) {
		errno = fake->end_error;
		return -1;
	}
	return 0;
}

static void
init_fake_dns_snapshot(struct fake_dns_nvram *fake)
{
	memset(fake, 0, sizeof(*fake));
	fake->names[0] = "ip6_dns1";
	fake->names[1] = "ip6_dns2";
	fake->names[2] = "ip6_dns3";
	fake->values[0] = "2001:db8::53";
	fake->values[1] = "2001:db8::54";
	fake->values[2] = "2001:db8::55";
	fake->read_fail_at = -1;
	fake->missing_at = -1;
}

static int
read_fake_dns_snapshot(struct fake_dns_nvram *fake, char values[3][64])
{
	static const char *const names[WAN6_DNS_CONFIG_VALUE_COUNT] = {
		"ip6_dns1", "ip6_dns2", "ip6_dns3"
	};
	char *destinations[WAN6_DNS_CONFIG_VALUE_COUNT];
	size_t sizes[WAN6_DNS_CONFIG_VALUE_COUNT];
	unsigned int i;

	for (i = 0; i < WAN6_DNS_CONFIG_VALUE_COUNT; ++i) {
		destinations[i] = values[i];
		sizes[i] = sizeof(values[i]);
	}
	return wan6_dns_copy_config_snapshot(
		names, destinations, sizes, fake_dns_snapshot_begin,
		fake_dns_snapshot_get, fake_dns_snapshot_end, fake);
}

static void
test_dns_config_snapshot_copies_one_coherent_cache_backed_tuple(void)
{
	struct fake_dns_nvram fake;
	char values[3][64] = {{0}};

	init_fake_dns_snapshot(&fake);
	assert(read_fake_dns_snapshot(&fake, values) == 0);
	assert(fake.begin_calls == 1);
	assert(fake.read_calls == 3);
	assert(fake.end_calls == 1);
	assert(strcmp(values[0], "2001:db8::53") == 0);
	assert(strcmp(values[1], "2001:db8::54") == 0);
	assert(strcmp(values[2], "2001:db8::55") == 0);
}

static void
test_dns_config_snapshot_keeps_missing_values_empty(void)
{
	struct fake_dns_nvram fake;
	char values[3][64] = {{0}};

	init_fake_dns_snapshot(&fake);
	fake.missing_at = 1;
	assert(read_fake_dns_snapshot(&fake, values) == 0);
	assert(fake.begin_calls == 1);
	assert(fake.read_calls == 3);
	assert(fake.end_calls == 1);
	assert(strcmp(values[0], "2001:db8::53") == 0);
	assert(values[1][0] == '\0');
	assert(strcmp(values[2], "2001:db8::55") == 0);
}

static void
test_dns_config_snapshot_discards_partial_values_on_read_failure(void)
{
	struct fake_dns_nvram fake;
	char values[3][64];
	unsigned int i;

	init_fake_dns_snapshot(&fake);
	fake.read_fail_at = 1;
	for (i = 0; i < 3; ++i)
		strcpy(values[i], "stale");
	assert(read_fake_dns_snapshot(&fake, values) == EACCES);
	assert(fake.begin_calls == 1);
	assert(fake.read_calls == 2);
	assert(fake.end_calls == 1);
	for (i = 0; i < 3; ++i)
		assert(values[i][0] == '\0');
}

static void
test_dns_config_snapshot_discards_values_on_transaction_close_failure(void)
{
	struct fake_dns_nvram fake;
	char values[3][64];
	unsigned int i;

	init_fake_dns_snapshot(&fake);
	fake.end_error = EIO;
	for (i = 0; i < 3; ++i)
		strcpy(values[i], "stale");
	assert(read_fake_dns_snapshot(&fake, values) == EIO);
	assert(fake.begin_calls == 1);
	assert(fake.read_calls == 3);
	assert(fake.end_calls == 1);
	for (i = 0; i < 3; ++i)
		assert(values[i][0] == '\0');
}

static void
test_dns_config_snapshot_begin_failure_does_not_read_or_close(void)
{
	struct fake_dns_nvram fake;
	char values[3][64] = {{"stale"}, {"stale"}, {"stale"}};
	unsigned int i;

	init_fake_dns_snapshot(&fake);
	fake.begin_error = EBUSY;
	assert(read_fake_dns_snapshot(&fake, values) == EBUSY);
	assert(fake.begin_calls == 1);
	assert(fake.read_calls == 0);
	assert(fake.end_calls == 0);
	for (i = 0; i < 3; ++i)
		assert(values[i][0] == '\0');
}

static void
test_static_dns_server_validation_accepts_ipv6_and_empty_slots(void)
{
	assert(wan6_dns_validate_static_server("2001:db8::53") == 0);
	assert(wan6_dns_validate_static_server("::1") == 0);
	assert(wan6_dns_validate_static_server("fe80::1") == 0);
	assert(wan6_dns_validate_static_server("") == 0);
}

static void
test_static_dns_server_validation_rejects_non_literals_and_injection(void)
{
	assert(wan6_dns_validate_static_server(NULL) == EINVAL);
	assert(wan6_dns_validate_static_server("dns.example") == EINVAL);
	assert(wan6_dns_validate_static_server("192.0.2.53") == EINVAL);
	assert(wan6_dns_validate_static_server("2001:db8::53/64") == EINVAL);
	assert(wan6_dns_validate_static_server("2001:db8::53\nnameserver 192.0.2.1") == EINVAL);
	assert(wan6_dns_validate_static_server("2001:db8::xyz") == EINVAL);
}

int
main(void)
{
	test_static_dns_server_validation_accepts_ipv6_and_empty_slots();
	test_static_dns_server_validation_rejects_non_literals_and_injection();
	test_withdrawn_dns_option_clears_previous_lease();
	test_absent_dns_option_is_noop_when_already_empty();
	test_changed_dns_list_is_published();
	test_unchanged_dns_list_is_not_republished();
	test_dns_store_publishes_changed_value();
	test_dns_store_skips_unchanged_value();
	test_dns_store_publishes_withdrawal();
	test_dns_store_fails_closed_when_current_value_read_fails();
	test_dns_store_treats_missing_current_value_as_empty();
	test_dns_store_propagates_nvram_failure();
	test_dns_store_rejects_missing_writer();
	test_dns_config_snapshot_copies_one_coherent_cache_backed_tuple();
	test_dns_config_snapshot_keeps_missing_values_empty();
	test_dns_config_snapshot_discards_partial_values_on_read_failure();
	test_dns_config_snapshot_discards_values_on_transaction_close_failure();
	test_dns_config_snapshot_begin_failure_does_not_read_or_close();
	puts("WAN DHCPv6 DNS withdrawal policy regression: PASS");
	return 0;
}
