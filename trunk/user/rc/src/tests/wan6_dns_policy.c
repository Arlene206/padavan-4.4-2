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

int
main(void)
{
	test_withdrawn_dns_option_clears_previous_lease();
	test_absent_dns_option_is_noop_when_already_empty();
	test_changed_dns_list_is_published();
	test_unchanged_dns_list_is_not_republished();
	test_dns_store_publishes_changed_value();
	test_dns_store_skips_unchanged_value();
	test_dns_store_publishes_withdrawal();
	test_dns_store_propagates_nvram_failure();
	test_dns_store_rejects_missing_writer();
	puts("WAN DHCPv6 DNS withdrawal policy regression: PASS");
	return 0;
}
