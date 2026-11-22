#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "dnsmasq_static_host_nvram_policy.h"

#define FAKE_NVRAM_KEYS 16

struct fake_nvram_item { const char *name; const char *value; };
struct fake_nvram {
	struct fake_nvram_item items[FAKE_NVRAM_KEYS];
	unsigned int item_count;
	char cache[DNSMASQ_STATIC_HOST_NAME_SIZE + 1];
	const char *fail_name;
	int fail_status, begin_status, end_status;
	unsigned int get_calls, begin_calls, end_calls;
	int active;
};

static int
fake_begin(void *opaque)
{
	struct fake_nvram *fake = (struct fake_nvram *)opaque;
	fake->begin_calls++;
	if (fake->begin_status) { errno = fake->begin_status; return -1; }
	fake->active = 1;
	return 0;
}

static int
fake_get(void *opaque, const char *name, char **value)
{
	struct fake_nvram *fake = (struct fake_nvram *)opaque;
	unsigned int i;
	fake->get_calls++;
	assert(fake->active);
	if (fake->fail_name && strcmp(fake->fail_name, name) == 0) {
		errno = fake->fail_status;
		return -1;
	}
	for (i = 0; i < fake->item_count; ++i) {
		if (strcmp(fake->items[i].name, name) == 0) {
			snprintf(fake->cache, sizeof(fake->cache), "%s", fake->items[i].value);
			*value = fake->cache;
			return 1;
		}
	}
	*value = NULL;
	return 0;
}

static int
fake_end(void *opaque)
{
	struct fake_nvram *fake = (struct fake_nvram *)opaque;
	fake->end_calls++;
	assert(fake->active);
	fake->active = 0;
	if (fake->end_status) { errno = fake->end_status; return -1; }
	return 0;
}

static void
fake_add(struct fake_nvram *fake, const char *name, const char *value)
{
	assert(fake->item_count < FAKE_NVRAM_KEYS);
	fake->items[fake->item_count].name = name;
	fake->items[fake->item_count].value = value;
	fake->item_count++;
}

static void
assert_clear(const struct dnsmasq_static_host_nvram_snapshot *snapshot)
{
	struct dnsmasq_static_host_nvram_snapshot zero;
	memset(&zero, 0, sizeof(zero));
	assert(memcmp(snapshot, &zero, sizeof(zero)) == 0);
}

static void
test_success_copies_rotating_cache_values_atomically(void)
{
	struct fake_nvram fake;
	struct dnsmasq_static_host_nvram_snapshot snapshot;
	memset(&fake, 0, sizeof(fake));
	fake_add(&fake, "dhcp_static_x", "1");
	fake_add(&fake, "dhcp_static_arp", "1");
	fake_add(&fake, "dhcp_staticnum_x", "2");
	fake_add(&fake, "dhcp_staticmac_x0", "001122334455");
	fake_add(&fake, "dhcp_staticip_x0", "192.168.1.10");
	fake_add(&fake, "dhcp_staticname_x0", "desktop");
	fake_add(&fake, "dhcp_staticmac_x1", "021122334466");
	fake_add(&fake, "dhcp_staticip_x1", "192.168.1.11");
	fake_add(&fake, "dhcp_staticname_x1", "laptop");
	assert(dnsmasq_static_host_nvram_snapshot_read(
		&snapshot, fake_begin, fake_get, fake_end, &fake) == 0);
	assert(fake.begin_calls == 1 && fake.end_calls == 1 && !fake.active);
	assert(fake.get_calls == 9);
	assert(snapshot.enabled == 1 && snapshot.arp_enabled == 1);
	assert(snapshot.item_count == 2);
	assert(strcmp(snapshot.entries[0].mac, "001122334455") == 0);
	assert(strcmp(snapshot.entries[0].ip, "192.168.1.10") == 0);
	assert(strcmp(snapshot.entries[0].hostname, "desktop") == 0);
	assert(strcmp(snapshot.entries[1].mac, "021122334466") == 0);
	assert(strcmp(snapshot.entries[1].ip, "192.168.1.11") == 0);
	assert(strcmp(snapshot.entries[1].hostname, "laptop") == 0);
}

static void
test_missing_keys_keep_disabled_and_empty_defaults(void)
{
	struct fake_nvram fake;
	struct dnsmasq_static_host_nvram_snapshot snapshot;
	memset(&fake, 0, sizeof(fake));
	assert(dnsmasq_static_host_nvram_snapshot_read(
		&snapshot, fake_begin, fake_get, fake_end, &fake) == 0);
	assert(snapshot.enabled == 0 && snapshot.arp_enabled == 0 && snapshot.item_count == 0);
	assert(fake.get_calls == 1 && fake.end_calls == 1);
	memset(&fake, 0, sizeof(fake));
	fake_add(&fake, "dhcp_static_x", "1");
	fake_add(&fake, "dhcp_static_arp", "0");
	fake_add(&fake, "dhcp_staticnum_x", "1");
	assert(dnsmasq_static_host_nvram_snapshot_read(
		&snapshot, fake_begin, fake_get, fake_end, &fake) == 0);
	assert(snapshot.item_count == 1);
	assert(snapshot.entries[0].mac[0] == '\0');
	assert(snapshot.entries[0].ip[0] == '\0');
	assert(snapshot.entries[0].hostname[0] == '\0');
}

static void
test_read_error_invalidates_snapshot_and_closes_transaction(void)
{
	struct fake_nvram fake;
	struct dnsmasq_static_host_nvram_snapshot snapshot;
	memset(&fake, 0, sizeof(fake));
	fake_add(&fake, "dhcp_static_x", "1");
	fake_add(&fake, "dhcp_static_arp", "1");
	fake_add(&fake, "dhcp_staticnum_x", "1");
	fake.fail_name = "dhcp_staticip_x0";
	fake.fail_status = EACCES;
	memset(&snapshot, 0xa5, sizeof(snapshot));
	assert(dnsmasq_static_host_nvram_snapshot_read(
		&snapshot, fake_begin, fake_get, fake_end, &fake) == EACCES);
	assert(fake.end_calls == 1 && !fake.active);
	assert_clear(&snapshot);
}

static void
test_end_and_begin_failures_are_fail_closed(void)
{
	struct fake_nvram fake;
	struct dnsmasq_static_host_nvram_snapshot snapshot;
	memset(&fake, 0, sizeof(fake));
	fake_add(&fake, "dhcp_static_x", "1");
	fake.end_status = EBUSY;
	assert(dnsmasq_static_host_nvram_snapshot_read(
		&snapshot, fake_begin, fake_get, fake_end, &fake) == EBUSY);
	assert(fake.end_calls == 1);
	assert_clear(&snapshot);
	memset(&fake, 0, sizeof(fake));
	fake.begin_status = EIO;
	memset(&snapshot, 0xa5, sizeof(snapshot));
	assert(dnsmasq_static_host_nvram_snapshot_read(
		&snapshot, fake_begin, fake_get, fake_end, &fake) == EIO);
	assert(fake.begin_calls == 1 && fake.get_calls == 0 && fake.end_calls == 0);
	assert_clear(&snapshot);
}

static void
test_malformed_and_oversized_values_fail_closed(void)
{
	struct fake_nvram fake;
	struct dnsmasq_static_host_nvram_snapshot snapshot;
	char oversized[DNSMASQ_STATIC_HOST_NAME_SIZE + 2];
	memset(&fake, 0, sizeof(fake));
	fake_add(&fake, "dhcp_static_x", "1");
	fake_add(&fake, "dhcp_static_arp", "0");
	fake_add(&fake, "dhcp_staticnum_x", "1x");
	assert(dnsmasq_static_host_nvram_snapshot_read(
		&snapshot, fake_begin, fake_get, fake_end, &fake) == EINVAL);
	assert(fake.end_calls == 1);
	assert_clear(&snapshot);
	memset(&fake, 0, sizeof(fake));
	fake_add(&fake, "dhcp_static_x", "1");
	fake_add(&fake, "dhcp_static_arp", "0");
	fake_add(&fake, "dhcp_staticnum_x", "1");
	memset(oversized, 'x', sizeof(oversized) - 1);
	oversized[sizeof(oversized) - 1] = '\0';
	fake_add(&fake, "dhcp_staticmac_x0", "001122334455");
	fake_add(&fake, "dhcp_staticip_x0", "192.168.1.10");
	fake_add(&fake, "dhcp_staticname_x0", oversized);
	assert(dnsmasq_static_host_nvram_snapshot_read(
		&snapshot, fake_begin, fake_get, fake_end, &fake) == ENAMETOOLONG);
	assert(fake.end_calls == 1);
	assert_clear(&snapshot);
}

static void
test_item_count_is_bounded(void)
{
	struct fake_nvram fake;
	struct dnsmasq_static_host_nvram_snapshot snapshot;
	memset(&fake, 0, sizeof(fake));
	fake_add(&fake, "dhcp_static_x", "1");
	fake_add(&fake, "dhcp_static_arp", "0");
	fake_add(&fake, "dhcp_staticnum_x", "99");
	assert(dnsmasq_static_host_nvram_snapshot_read(
		&snapshot, fake_begin, fake_get, fake_end, &fake) == 0);
	assert(snapshot.item_count == DNSMASQ_STATIC_HOST_MAX);
	assert(fake.get_calls == 3 + DNSMASQ_STATIC_HOST_MAX * 3);
}

int
main(void)
{
	test_success_copies_rotating_cache_values_atomically();
	test_missing_keys_keep_disabled_and_empty_defaults();
	test_read_error_invalidates_snapshot_and_closes_transaction();
	test_end_and_begin_failures_are_fail_closed();
	test_malformed_and_oversized_values_fail_closed();
	test_item_count_is_bounded();
	puts("DNSMasq static-host NVRAM snapshot fail-closed regression: PASS");
	return 0;
}
