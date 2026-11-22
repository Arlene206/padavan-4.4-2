#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "dnsmasq_nvram_option_policy.h"

static void
test_present_values_keep_nvram_match_semantics(void)
{
	int enabled = -1;

	assert(dnsmasq_nvram_option_status(1, 0, "1", 0, &enabled) == 0);
	assert(enabled == 1);
	assert(dnsmasq_nvram_option_status(1, 0, "0", 0, &enabled) == 0);
	assert(enabled == 0);
	assert(dnsmasq_nvram_option_status(1, 0, "true", 0, &enabled) == 0);
	assert(enabled == 0);
	assert(dnsmasq_nvram_option_status(1, 0, "", 0, &enabled) == 0);
	assert(enabled == 0);
}

static void
test_invmatch_preserves_nvram_invmatch_semantics(void)
{
	int matches = -1;

	assert(dnsmasq_nvram_invmatch_status(1, 0, "1", "1", 0,
					     &matches) == 0);
	assert(matches == 0);
	assert(dnsmasq_nvram_invmatch_status(1, 0, "0", "1", 0,
					     &matches) == 0);
	assert(matches == 1);
	assert(dnsmasq_nvram_invmatch_status(1, 0, "static", "1", 0,
					     &matches) == 0);
	assert(matches == 1);
	assert(dnsmasq_nvram_invmatch_status(1, 0, "", "1", 0,
					     &matches) == 0);
	assert(matches == 1);

	/* nvram_invmatch() is false for a missing key (NULL nvram_get value). */
	assert(dnsmasq_nvram_invmatch_status(0, 0, NULL, "1", 0,
					     &matches) == 0);
	assert(matches == 0);
	/* The policy helper keeps the missing-key default explicit for other callers. */
	assert(dnsmasq_nvram_invmatch_status(0, 0, NULL, "1", 1,
					     &matches) == 0);
	assert(matches == 1);
}

static void
test_invmatch_backend_errors_preserve_output(void)
{
	int matches = 17;

	assert(dnsmasq_nvram_invmatch_status(-1, EACCES, NULL, "1", 0,
					     &matches) == EACCES);
	assert(matches == 17);
	assert(dnsmasq_nvram_invmatch_status(-1, 0, NULL, "1", 0,
					     &matches) == EIO);
	assert(matches == 17);
	assert(dnsmasq_nvram_invmatch_status(2, 0, "1", "1", 0,
					     &matches) == EIO);
	assert(matches == 17);
	assert(dnsmasq_nvram_invmatch_status(0, 0, "unexpected", "1", 0,
					     &matches) == EIO);
	assert(matches == 17);
	assert(dnsmasq_nvram_invmatch_status(1, 0, NULL, "1", 0,
					     &matches) == EIO);
	assert(matches == 17);
}

static void
test_invmatch_invalid_arguments_are_rejected(void)
{
	int matches = 17;

	assert(dnsmasq_nvram_invmatch_status(1, 0, "1", NULL, 0,
					     &matches) == EINVAL);
	assert(matches == 17);
	assert(dnsmasq_nvram_invmatch_status(1, 0, "1", "1", 0,
					     NULL) == EINVAL);
	assert(dnsmasq_nvram_invmatch_status(0, 0, NULL, "1", 2,
					     &matches) == EINVAL);
	assert(matches == 17);
}

static void
test_integer_boolean_preserves_nonzero_nvram_get_int_semantics(void)
{
	int enabled = 9;

	assert(dnsmasq_nvram_integer_boolean_status(1, 0, "1", 0, &enabled) == 0);
	assert(enabled == 1);
	assert(dnsmasq_nvram_integer_boolean_status(1, 0, "2", 0, &enabled) == 0);
	assert(enabled == 1);
	assert(dnsmasq_nvram_integer_boolean_status(1, 0, "-1", 0, &enabled) == 0);
	assert(enabled == 1);
	assert(dnsmasq_nvram_integer_boolean_status(1, 0, "0", 0, &enabled) == 0);
	assert(enabled == 0);

	enabled = 9;
	assert(dnsmasq_nvram_integer_boolean_status(1, 0, "1junk", 0, &enabled) == EINVAL);
	assert(enabled == 9);
	assert(dnsmasq_nvram_integer_boolean_status(1, 0, "999999999999999999999999",
						    0, &enabled) == ERANGE);
	assert(enabled == 9);
	assert(dnsmasq_nvram_integer_boolean_status(0, 0, NULL, 0, &enabled) == 0);
	assert(enabled == 0);
	assert(dnsmasq_nvram_integer_boolean_status(-1, EACCES, NULL, 0, &enabled) == EACCES);
	assert(enabled == 0);
}

static void
test_checked_integer_range_options(void)
{
	int value = 77;

	/* dhcp_verbose exposes the four values 0..3 in the web UI. */
	assert(dnsmasq_nvram_integer_range_status(1, 0, "0", 0, 0, 3, &value) == 0 && value == 0);
	assert(dnsmasq_nvram_integer_range_status(1, 0, "3", 0, 0, 3, &value) == 0 && value == 3);
	assert(dnsmasq_nvram_integer_range_status(1, 0, "4", 0, 0, 3, &value) == ERANGE);
	assert(value == 3);

	/* dhcp_lease UI range is 120..604800 seconds. */
	assert(dnsmasq_nvram_integer_range_status(1, 0, "120", 0, 120, 604800, &value) == 0 && value == 120);
	assert(dnsmasq_nvram_integer_range_status(1, 0, "604800", 0, 120, 604800, &value) == 0 && value == 604800);
	assert(dnsmasq_nvram_integer_range_status(1, 0, "119", 0, 120, 604800, &value) == ERANGE);
	assert(value == 604800);
	assert(dnsmasq_nvram_integer_range_status(1, 0, "604801", 0, 120, 604800, &value) == ERANGE);
	assert(dnsmasq_nvram_integer_range_status(1, 0, "-1", 0, 120, 604800, &value) == ERANGE);
	assert(dnsmasq_nvram_integer_range_status(1, 0, "120junk", 0, 120, 604800, &value) == EINVAL);
	assert(dnsmasq_nvram_integer_range_status(1, 0, "999999999999999999999", 0, 120, 604800, &value) == ERANGE);

	/* Legacy nvram_get_int() returned zero for an absent key. */
	assert(dnsmasq_nvram_integer_range_status(0, 0, NULL, 0, 120, 604800, &value) == 0 && value == 0);
	assert(dnsmasq_nvram_integer_range_status(-1, EACCES, NULL, 0, 0, 3, &value) == EACCES);
	assert(dnsmasq_nvram_integer_range_status(-1, 0, NULL, 0, 0, 3, &value) == EIO);
	assert(dnsmasq_nvram_integer_range_status(2, 0, "1", 0, 0, 3, &value) == EIO);
	assert(dnsmasq_nvram_integer_range_status(0, 0, "0", 0, 0, 3, &value) == EIO);
	assert(dnsmasq_nvram_integer_range_status(1, 0, "1", 0, 3, 0, &value) == EINVAL);
	assert(dnsmasq_nvram_integer_range_status(1, 0, "1", 0, 0, 3, NULL) == EINVAL);
}

static void
test_checked_string_option_and_output_preservation(void)
{
	char value[16] = "unchanged";

	assert(dnsmasq_nvram_string_status(1, 0, "192.0.2.10", "",
					   value, sizeof(value)) == 0);
	assert(strcmp(value, "192.0.2.10") == 0);

	assert(dnsmasq_nvram_string_status(0, 0, NULL, "",
					   value, sizeof(value)) == 0);
	assert(value[0] == '\0');

	strcpy(value, "unchanged");
	assert(dnsmasq_nvram_string_status(-1, EACCES, NULL, "",
					   value, sizeof(value)) == EACCES);
	assert(strcmp(value, "unchanged") == 0);
	assert(dnsmasq_nvram_string_status(-1, 0, NULL, "",
					   value, sizeof(value)) == EIO);
	assert(strcmp(value, "unchanged") == 0);
	assert(dnsmasq_nvram_string_status(2, 0, "x", "",
					   value, sizeof(value)) == EIO);
	assert(strcmp(value, "unchanged") == 0);
	assert(dnsmasq_nvram_string_status(0, 0, "unexpected", "",
					   value, sizeof(value)) == EIO);
	assert(strcmp(value, "unchanged") == 0);
	assert(dnsmasq_nvram_string_status(1, 0, NULL, "",
					   value, sizeof(value)) == EIO);
	assert(strcmp(value, "unchanged") == 0);
	assert(dnsmasq_nvram_string_status(1, 0, "1234567890123456", "",
					   value, sizeof(value)) == ENAMETOOLONG);
	assert(strcmp(value, "unchanged") == 0);
	assert(dnsmasq_nvram_string_status(1, 0, "1", "", NULL, 1) == EINVAL);
	assert(dnsmasq_nvram_string_status(1, 0, "1", "", value, 0) == EINVAL);
}

static void
test_missing_keys_use_explicit_legacy_defaults(void)
{
	int enabled = -1;

	assert(dnsmasq_nvram_option_status(0, 0, NULL, 0, &enabled) == 0);
	assert(enabled == 0);
	assert(dnsmasq_nvram_option_status(0, 0, NULL, 1, &enabled) == 0);
	assert(enabled == 1);
}

static void
test_backend_errors_preserve_errno_and_output(void)
{
	int enabled = 19;

	assert(dnsmasq_nvram_option_status(-1, EACCES, NULL, 0, &enabled) == EACCES);
	assert(enabled == 19);
	assert(dnsmasq_nvram_option_status(-1, 0, NULL, 0, &enabled) == EIO);
	assert(enabled == 19);
}

static void
test_inconsistent_results_fail_closed(void)
{
	int enabled = 19;

	assert(dnsmasq_nvram_option_status(2, 0, "1", 0, &enabled) == EIO);
	assert(enabled == 19);
	assert(dnsmasq_nvram_option_status(1, 0, NULL, 0, &enabled) == EIO);
	assert(enabled == 19);
	assert(dnsmasq_nvram_option_status(0, 0, "1", 0, &enabled) == EIO);
	assert(enabled == 19);
}

static void
test_invalid_arguments_are_rejected(void)
{
	assert(dnsmasq_nvram_option_status(1, 0, "1", 0, NULL) == EINVAL);
}


#define FAKE_DHCP_NVRAM_KEYS 14

struct fake_dhcp_nvram_item {
	const char *name;
	const char *value;
};

struct fake_dhcp_nvram {
	struct fake_dhcp_nvram_item items[FAKE_DHCP_NVRAM_KEYS];
	unsigned int item_count;
	unsigned int read_calls;
	unsigned int begin_calls;
	unsigned int end_calls;
	int fail_read_at;
	int read_error;
	int begin_error;
	int end_error;
	int active;
	char rotating_cache[DNSMASQ_SPLIT_DNS_LIST_VALUE_SIZE + 1];
};

static void
fake_dhcp_nvram_add(struct fake_dhcp_nvram *fake, const char *name,
		    const char *value)
{
	assert(fake->item_count < FAKE_DHCP_NVRAM_KEYS);
	fake->items[fake->item_count].name = name;
	fake->items[fake->item_count].value = value;
	fake->item_count++;
}

static void
fake_dhcp_nvram_init(struct fake_dhcp_nvram *fake)
{
	memset(fake, 0, sizeof(*fake));
	fake->fail_read_at = -1;
}

static int
fake_dhcp_nvram_begin(void *opaque)
{
	struct fake_dhcp_nvram *fake = (struct fake_dhcp_nvram *)opaque;

	fake->begin_calls++;
	if (fake->begin_error) {
		errno = fake->begin_error;
		return -1;
	}
	fake->active = 1;
	return 0;
}

static int
fake_dhcp_nvram_get(void *opaque, const char *name, char **value)
{
	struct fake_dhcp_nvram *fake = (struct fake_dhcp_nvram *)opaque;
	unsigned int i, call = fake->read_calls++;

	assert(fake->active);
	if ((int)call == fake->fail_read_at) {
		errno = fake->read_error ? fake->read_error : EIO;
		return -1;
	}
	for (i = 0; i < fake->item_count; ++i) {
		if (strcmp(fake->items[i].name, name) == 0) {
			assert(strlen(fake->items[i].value) < sizeof(fake->rotating_cache));
			snprintf(fake->rotating_cache, sizeof(fake->rotating_cache),
				 "%s", fake->items[i].value);
			*value = fake->rotating_cache;
			return 1;
		}
	}
	*value = NULL;
	return 0;
}

static int
fake_dhcp_nvram_end(void *opaque)
{
	struct fake_dhcp_nvram *fake = (struct fake_dhcp_nvram *)opaque;

	fake->end_calls++;
	assert(fake->active);
	fake->active = 0;
	if (fake->end_error) {
		errno = fake->end_error;
		return -1;
	}
	return 0;
}

static int
fake_dhcp_nvram_snapshot(struct dnsmasq_dhcp_config_nvram_snapshot *snapshot,
			 int dhcp_enabled, struct fake_dhcp_nvram *fake)
{
	return dnsmasq_dhcp_config_nvram_snapshot_read(
		snapshot, dhcp_enabled, fake_dhcp_nvram_begin,
		fake_dhcp_nvram_get, fake_dhcp_nvram_end, fake);
}

static void
test_dhcp_snapshot_copies_rotating_cache_as_one_tuple(void)
{
	struct fake_dhcp_nvram fake;
	struct dnsmasq_dhcp_config_nvram_snapshot snapshot;

	fake_dhcp_nvram_init(&fake);
	fake_dhcp_nvram_add(&fake, "lan_ipaddr", "192.168.1.1");
	fake_dhcp_nvram_add(&fake, "lan_netmask", "255.255.255.0");
	fake_dhcp_nvram_add(&fake, "lan_domain", "router.example");
	fake_dhcp_nvram_add(&fake, "dhcp_start", "192.168.1.100");
	fake_dhcp_nvram_add(&fake, "dhcp_end", "192.168.1.200");
	fake_dhcp_nvram_add(&fake, "dhcp_gateway_x", "192.168.1.254");
	fake_dhcp_nvram_add(&fake, "dhcp_dns1_x", "192.168.1.53");
	fake_dhcp_nvram_add(&fake, "dhcp_dns2_x", "192.168.1.54");
	fake_dhcp_nvram_add(&fake, "dhcp_dns3_x", "192.168.1.55");
	fake_dhcp_nvram_add(&fake, "dhcp_wins_x", "192.168.1.56");

	assert(fake_dhcp_nvram_snapshot(&snapshot, 1, &fake) == 0);
	assert(fake.begin_calls == 1 && fake.end_calls == 1 && !fake.active);
	assert(fake.read_calls == 10);
	assert(strcmp(snapshot.lan_ipaddr, "192.168.1.1") == 0);
	assert(strcmp(snapshot.lan_netmask, "255.255.255.0") == 0);
	assert(strcmp(snapshot.lan_domain, "router.example") == 0);
	assert(strcmp(snapshot.dhcp_start, "192.168.1.100") == 0);
	assert(strcmp(snapshot.dhcp_end, "192.168.1.200") == 0);
	assert(strcmp(snapshot.dhcp_gateway, "192.168.1.254") == 0);
	assert(strcmp(snapshot.dhcp_dns1, "192.168.1.53") == 0);
	assert(strcmp(snapshot.dhcp_dns2, "192.168.1.54") == 0);
	assert(strcmp(snapshot.dhcp_dns3, "192.168.1.55") == 0);
	assert(strcmp(snapshot.dhcp_wins, "192.168.1.56") == 0);
}

static void
test_dhcp_snapshot_skips_optional_keys_when_dhcp_disabled(void)
{
	struct fake_dhcp_nvram fake;
	struct dnsmasq_dhcp_config_nvram_snapshot snapshot;

	fake_dhcp_nvram_init(&fake);
	fake_dhcp_nvram_add(&fake, "lan_ipaddr", "192.168.1.1");
	fake_dhcp_nvram_add(&fake, "lan_netmask", "255.255.255.0");
	fake_dhcp_nvram_add(&fake, "lan_domain", "router.example");

	assert(fake_dhcp_nvram_snapshot(&snapshot, 0, &fake) == 0);
	assert(fake.read_calls == 3 && fake.begin_calls == 1 && fake.end_calls == 1);
	assert(strcmp(snapshot.lan_ipaddr, "192.168.1.1") == 0);
	assert(snapshot.dhcp_start[0] == '\0');
	assert(snapshot.dhcp_dns1[0] == '\0');
	assert(snapshot.dhcp_wins[0] == '\0');
}

static void
test_dhcp_snapshot_read_and_close_errors_invalidate_all_values(void)
{
	struct fake_dhcp_nvram fake;
	struct dnsmasq_dhcp_config_nvram_snapshot snapshot;
	struct dnsmasq_dhcp_config_nvram_snapshot zero;

	memset(&zero, 0, sizeof(zero));
	fake_dhcp_nvram_init(&fake);
	fake.fail_read_at = 7;
	fake.read_error = EACCES;
	memset(&snapshot, 0xa5, sizeof(snapshot));
	assert(fake_dhcp_nvram_snapshot(&snapshot, 1, &fake) == EACCES);
	assert(fake.end_calls == 1 && !fake.active);
	assert(memcmp(&snapshot, &zero, sizeof(snapshot)) == 0);

	fake_dhcp_nvram_init(&fake);
	fake.end_error = EBUSY;
	fake_dhcp_nvram_add(&fake, "lan_ipaddr", "192.168.1.1");
	fake_dhcp_nvram_add(&fake, "lan_netmask", "255.255.255.0");
	fake_dhcp_nvram_add(&fake, "lan_domain", "router.example");
	memset(&snapshot, 0xa5, sizeof(snapshot));
	assert(fake_dhcp_nvram_snapshot(&snapshot, 0, &fake) == EBUSY);
	assert(fake.end_calls == 1 && !fake.active);
	assert(memcmp(&snapshot, &zero, sizeof(snapshot)) == 0);
}

static void
test_dhcp_snapshot_begin_and_truncation_errors_fail_closed(void)
{
	struct fake_dhcp_nvram fake;
	struct dnsmasq_dhcp_config_nvram_snapshot snapshot;
	struct dnsmasq_dhcp_config_nvram_snapshot zero;
	char oversized[DNSMASQ_DHCP_DOMAIN_VALUE_SIZE + 1];

	memset(&zero, 0, sizeof(zero));
	fake_dhcp_nvram_init(&fake);
	fake.begin_error = EIO;
	memset(&snapshot, 0xa5, sizeof(snapshot));
	assert(fake_dhcp_nvram_snapshot(&snapshot, 0, &fake) == EIO);
	assert(fake.read_calls == 0 && fake.end_calls == 0);
	assert(memcmp(&snapshot, &zero, sizeof(snapshot)) == 0);

	fake_dhcp_nvram_init(&fake);
	memset(oversized, 'x', sizeof(oversized) - 1);
	oversized[sizeof(oversized) - 1] = '\0';
	fake_dhcp_nvram_add(&fake, "lan_ipaddr", "192.168.1.1");
	fake_dhcp_nvram_add(&fake, "lan_netmask", "255.255.255.0");
	fake_dhcp_nvram_add(&fake, "lan_domain", oversized);
	memset(&snapshot, 0xa5, sizeof(snapshot));
	assert(fake_dhcp_nvram_snapshot(&snapshot, 0, &fake) == ENAMETOOLONG);
	assert(fake.end_calls == 1 && !fake.active);
	assert(memcmp(&snapshot, &zero, sizeof(snapshot)) == 0);
}



static int
fake_dhcp6_nvram_snapshot(struct dnsmasq_dhcp6_nvram_snapshot *snapshot,
			  int dhcp6_mode, struct fake_dhcp_nvram *fake)
{
	return dnsmasq_dhcp6_nvram_snapshot_read(
		snapshot, dhcp6_mode, fake_dhcp_nvram_begin,
		fake_dhcp_nvram_get, fake_dhcp_nvram_end, fake);
}

static void
test_dhcp6_snapshot_copies_stateful_values_and_parses_bounds(void)
{
	struct fake_dhcp_nvram fake;
	struct dnsmasq_dhcp6_nvram_snapshot snapshot;

	fake_dhcp_nvram_init(&fake);
	fake_dhcp_nvram_add(&fake, "dhcp_dnsv6_x", "2001:db8::53");
	fake_dhcp_nvram_add(&fake, "ip6_lan_sflt", "3600");
	fake_dhcp_nvram_add(&fake, "ip6_lan_sfps", "8192");
	fake_dhcp_nvram_add(&fake, "ip6_lan_sfpe", "12288");
	assert(fake_dhcp6_nvram_snapshot(&snapshot, 2, &fake) == 0);
	assert(fake.begin_calls == 1 && fake.end_calls == 1 && !fake.active);
	assert(fake.read_calls == 4);
	assert(strcmp(snapshot.dns_server, "2001:db8::53") == 0);
	assert(snapshot.valid_lifetime == 3600);
	assert(snapshot.start_port == 8192);
	assert(snapshot.end_port == 12288);
}

static void
test_dhcp6_snapshot_preserves_mode_defaults_and_skips_unused_keys(void)
{
	struct fake_dhcp_nvram fake;
	struct dnsmasq_dhcp6_nvram_snapshot snapshot;

	fake_dhcp_nvram_init(&fake);
	fake_dhcp_nvram_add(&fake, "dhcp_dnsv6_x", "2001:db8::1");
	assert(fake_dhcp6_nvram_snapshot(&snapshot, 1, &fake) == 0);
	assert(fake.read_calls == 1 && fake.begin_calls == 1 && fake.end_calls == 1);
	assert(strcmp(snapshot.dns_server, "2001:db8::1") == 0);
	assert(snapshot.valid_lifetime == 0 && snapshot.start_port == 0);

	fake_dhcp_nvram_init(&fake);
	assert(fake_dhcp6_nvram_snapshot(&snapshot, 0, &fake) == 0);
	assert(fake.begin_calls == 0 && fake.read_calls == 0 && fake.end_calls == 0);
	assert(snapshot.dns_server[0] == '\0');
}

static void
test_dhcp6_snapshot_defaults_and_rejects_invalid_ranges(void)
{
	struct fake_dhcp_nvram fake;
	struct dnsmasq_dhcp6_nvram_snapshot snapshot;
	struct dnsmasq_dhcp6_nvram_snapshot zero;

	memset(&zero, 0, sizeof(zero));
	fake_dhcp_nvram_init(&fake);
	fake_dhcp_nvram_add(&fake, "dhcp_dnsv6_x", "");
	assert(fake_dhcp6_nvram_snapshot(&snapshot, 3, &fake) == 0);
	assert(snapshot.valid_lifetime == 1800);
	assert(snapshot.start_port == 4096);
	assert(snapshot.end_port == 4352);

	fake_dhcp_nvram_init(&fake);
	fake_dhcp_nvram_add(&fake, "dhcp_dnsv6_x", "2001:db8::53");
	fake_dhcp_nvram_add(&fake, "ip6_lan_sflt", "604801");
	fake_dhcp_nvram_add(&fake, "ip6_lan_sfps", "4096");
	fake_dhcp_nvram_add(&fake, "ip6_lan_sfpe", "4352");
	memset(&snapshot, 0xa5, sizeof(snapshot));
	assert(fake_dhcp6_nvram_snapshot(&snapshot, 2, &fake) == ERANGE);
	assert(fake.end_calls == 1 && !fake.active);
	assert(memcmp(&snapshot, &zero, sizeof(snapshot)) == 0);

	fake_dhcp_nvram_init(&fake);
	fake_dhcp_nvram_add(&fake, "dhcp_dnsv6_x", "2001:db8::53");
	fake_dhcp_nvram_add(&fake, "ip6_lan_sflt", "bad");
	fake_dhcp_nvram_add(&fake, "ip6_lan_sfps", "4096");
	fake_dhcp_nvram_add(&fake, "ip6_lan_sfpe", "4352");
	assert(fake_dhcp6_nvram_snapshot(&snapshot, 2, &fake) == EINVAL);
	assert(memcmp(&snapshot, &zero, sizeof(snapshot)) == 0);
}

static void
test_dhcp6_snapshot_read_and_close_failures_clear_all_fields(void)
{
	struct fake_dhcp_nvram fake;
	struct dnsmasq_dhcp6_nvram_snapshot snapshot;
	struct dnsmasq_dhcp6_nvram_snapshot zero;

	memset(&zero, 0, sizeof(zero));
	fake_dhcp_nvram_init(&fake);
	fake.fail_read_at = 2;
	fake.read_error = EACCES;
	memset(&snapshot, 0xa5, sizeof(snapshot));
	assert(fake_dhcp6_nvram_snapshot(&snapshot, 2, &fake) == EACCES);
	assert(fake.end_calls == 1 && !fake.active);
	assert(memcmp(&snapshot, &zero, sizeof(snapshot)) == 0);

	fake_dhcp_nvram_init(&fake);
	fake.end_error = EBUSY;
	fake_dhcp_nvram_add(&fake, "dhcp_dnsv6_x", "2001:db8::53");
	fake_dhcp_nvram_add(&fake, "ip6_lan_sflt", "1800");
	fake_dhcp_nvram_add(&fake, "ip6_lan_sfps", "4096");
	fake_dhcp_nvram_add(&fake, "ip6_lan_sfpe", "4352");
	memset(&snapshot, 0xa5, sizeof(snapshot));
	assert(fake_dhcp6_nvram_snapshot(&snapshot, 2, &fake) == EBUSY);
	assert(fake.end_calls == 1 && !fake.active);
	assert(memcmp(&snapshot, &zero, sizeof(snapshot)) == 0);
}

static void
test_dhcp6_snapshot_rejects_invalid_mode_and_arguments(void)
{
	struct fake_dhcp_nvram fake;
	struct dnsmasq_dhcp6_nvram_snapshot snapshot;

	fake_dhcp_nvram_init(&fake);
	assert(fake_dhcp6_nvram_snapshot(&snapshot, 4, &fake) == EINVAL);
	assert(fake_dhcp6_nvram_snapshot(NULL, 2, &fake) == EINVAL);
	assert(dnsmasq_dhcp6_nvram_snapshot_read(
		&snapshot, 2, NULL, fake_dhcp_nvram_get,
		fake_dhcp_nvram_end, &fake) == EINVAL);
}



static int
fake_split_dns_nvram_snapshot(struct dnsmasq_split_dns_nvram_snapshot *snapshot,
			      struct fake_dhcp_nvram *fake)
{
	return dnsmasq_split_dns_nvram_snapshot_read(
		snapshot, fake_dhcp_nvram_begin, fake_dhcp_nvram_get,
		fake_dhcp_nvram_end, fake);
}

static void
fake_add_split_dns_defaults(struct fake_dhcp_nvram *fake)
{
	fake_dhcp_nvram_add(fake, "vpnc_pdns", "1");
	fake_dhcp_nvram_add(fake, "vpnc_dom_t", "vpn.example");
	fake_dhcp_nvram_add(fake, "vpnc_dns_t", "10.8.0.1 10.8.0.2");
	fake_dhcp_nvram_add(fake, "wanx_domain", "man.example");
	fake_dhcp_nvram_add(fake, "wanx_dns", "192.0.2.53 192.0.2.54");
}

static void
test_split_dns_snapshot_copies_one_generation(void)
{
	struct fake_dhcp_nvram fake;
	struct dnsmasq_split_dns_nvram_snapshot snapshot;

	fake_dhcp_nvram_init(&fake);
	fake_add_split_dns_defaults(&fake);
	assert(fake_split_dns_nvram_snapshot(&snapshot, &fake) == 0);
	assert(fake.begin_calls == 1 && fake.end_calls == 1 && !fake.active);
	assert(fake.read_calls == 5);
	assert(snapshot.vpn_private_dns_enabled == 1);
	assert(strcmp(snapshot.vpn_domain, "vpn.example") == 0);
	assert(strcmp(snapshot.vpn_dns_servers, "10.8.0.1 10.8.0.2") == 0);
	assert(strcmp(snapshot.man_domain, "man.example") == 0);
	assert(strcmp(snapshot.man_dns_servers, "192.0.2.53 192.0.2.54") == 0);
}

static void
test_split_dns_snapshot_preserves_missing_and_nonpositive_defaults(void)
{
	struct fake_dhcp_nvram fake;
	struct dnsmasq_split_dns_nvram_snapshot snapshot;

	fake_dhcp_nvram_init(&fake);
	assert(fake_split_dns_nvram_snapshot(&snapshot, &fake) == 0);
	assert(fake.begin_calls == 1 && fake.end_calls == 1 && fake.read_calls == 5);
	assert(snapshot.vpn_private_dns_enabled == 0);
	assert(snapshot.vpn_domain[0] == '\0');
	assert(snapshot.vpn_dns_servers[0] == '\0');
	assert(snapshot.man_domain[0] == '\0');
	assert(snapshot.man_dns_servers[0] == '\0');

	fake_dhcp_nvram_init(&fake);
	fake_dhcp_nvram_add(&fake, "vpnc_pdns", "-1");
	assert(fake_split_dns_nvram_snapshot(&snapshot, &fake) == 0);
	assert(snapshot.vpn_private_dns_enabled == 0);
	assert(fake.begin_calls == 1 && fake.end_calls == 1);
}

static void
test_split_dns_snapshot_read_failure_at_every_key_invalidates_tuple(void)
{
	struct fake_dhcp_nvram fake;
	struct dnsmasq_split_dns_nvram_snapshot snapshot, zero;
	int failure_index;

	memset(&zero, 0, sizeof(zero));
	for (failure_index = 0; failure_index < 5; ++failure_index) {
		fake_dhcp_nvram_init(&fake);
		fake_add_split_dns_defaults(&fake);
		fake.fail_read_at = failure_index;
		fake.read_error = EACCES;
		memset(&snapshot, 0xa5, sizeof(snapshot));
		assert(fake_split_dns_nvram_snapshot(&snapshot, &fake) == EACCES);
		assert(fake.end_calls == 1 && !fake.active);
		assert(memcmp(&snapshot, &zero, sizeof(snapshot)) == 0);
	}
}

static void
test_split_dns_snapshot_begin_close_and_parse_errors_fail_closed(void)
{
	struct fake_dhcp_nvram fake;
	struct dnsmasq_split_dns_nvram_snapshot snapshot, zero;

	memset(&zero, 0, sizeof(zero));
	fake_dhcp_nvram_init(&fake);
	fake.begin_error = EBUSY;
	memset(&snapshot, 0xa5, sizeof(snapshot));
	assert(fake_split_dns_nvram_snapshot(&snapshot, &fake) == EBUSY);
	assert(fake.end_calls == 0);
	assert(memcmp(&snapshot, &zero, sizeof(snapshot)) == 0);

	fake_dhcp_nvram_init(&fake);
	fake_add_split_dns_defaults(&fake);
	fake.end_error = EIO;
	memset(&snapshot, 0xa5, sizeof(snapshot));
	assert(fake_split_dns_nvram_snapshot(&snapshot, &fake) == EIO);
	assert(fake.end_calls == 1 && !fake.active);
	assert(memcmp(&snapshot, &zero, sizeof(snapshot)) == 0);

	fake_dhcp_nvram_init(&fake);
	fake_dhcp_nvram_add(&fake, "vpnc_pdns", "1junk");
	memset(&snapshot, 0xa5, sizeof(snapshot));
	assert(fake_split_dns_nvram_snapshot(&snapshot, &fake) == EINVAL);
	assert(fake.end_calls == 1 && !fake.active);
	assert(memcmp(&snapshot, &zero, sizeof(snapshot)) == 0);
}

static void
test_split_dns_snapshot_rejects_oversized_domain_and_server_lists(void)
{
	struct fake_dhcp_nvram fake;
	struct dnsmasq_split_dns_nvram_snapshot snapshot, zero;
	char oversized_domain[DNSMASQ_SPLIT_DNS_DOMAIN_VALUE_SIZE + 1];
	char oversized_dns[DNSMASQ_SPLIT_DNS_LIST_VALUE_SIZE + 1];

	memset(&zero, 0, sizeof(zero));
	memset(oversized_domain, 'd', sizeof(oversized_domain) - 1);
	oversized_domain[sizeof(oversized_domain) - 1] = '\0';
	fake_dhcp_nvram_init(&fake);
	fake_dhcp_nvram_add(&fake, "vpnc_dom_t", oversized_domain);
	memset(&snapshot, 0xa5, sizeof(snapshot));
	assert(fake_split_dns_nvram_snapshot(&snapshot, &fake) == ENAMETOOLONG);
	assert(fake.end_calls == 1 && !fake.active);
	assert(memcmp(&snapshot, &zero, sizeof(snapshot)) == 0);

	memset(oversized_dns, '1', sizeof(oversized_dns) - 1);
	oversized_dns[sizeof(oversized_dns) - 1] = '\0';
	fake_dhcp_nvram_init(&fake);
	fake_dhcp_nvram_add(&fake, "wanx_dns", oversized_dns);
	memset(&snapshot, 0xa5, sizeof(snapshot));
	assert(fake_split_dns_nvram_snapshot(&snapshot, &fake) == ENAMETOOLONG);
	assert(fake.end_calls == 1 && !fake.active);
	assert(memcmp(&snapshot, &zero, sizeof(snapshot)) == 0);
}


int
main(void)
{
	test_present_values_keep_nvram_match_semantics();
	test_invmatch_preserves_nvram_invmatch_semantics();
	test_invmatch_backend_errors_preserve_output();
	test_invmatch_invalid_arguments_are_rejected();
	test_integer_boolean_preserves_nonzero_nvram_get_int_semantics();
	test_checked_integer_range_options();
	test_checked_string_option_and_output_preservation();
	test_missing_keys_use_explicit_legacy_defaults();
	test_backend_errors_preserve_errno_and_output();
	test_inconsistent_results_fail_closed();
	test_invalid_arguments_are_rejected();
	test_dhcp_snapshot_copies_rotating_cache_as_one_tuple();
	test_dhcp_snapshot_skips_optional_keys_when_dhcp_disabled();
	test_dhcp_snapshot_read_and_close_errors_invalidate_all_values();
	test_dhcp_snapshot_begin_and_truncation_errors_fail_closed();
	test_dhcp6_snapshot_copies_stateful_values_and_parses_bounds();
	test_dhcp6_snapshot_preserves_mode_defaults_and_skips_unused_keys();
	test_dhcp6_snapshot_defaults_and_rejects_invalid_ranges();
	test_dhcp6_snapshot_read_and_close_failures_clear_all_fields();
	test_dhcp6_snapshot_rejects_invalid_mode_and_arguments();
	test_split_dns_snapshot_copies_one_generation();
	test_split_dns_snapshot_preserves_missing_and_nonpositive_defaults();
	test_split_dns_snapshot_read_failure_at_every_key_invalidates_tuple();
	test_split_dns_snapshot_begin_close_and_parse_errors_fail_closed();
	test_split_dns_snapshot_rejects_oversized_domain_and_server_lists();
	return 0;
}
