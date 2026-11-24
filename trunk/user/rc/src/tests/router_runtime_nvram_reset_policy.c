#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "router_runtime_nvram_reset_policy.h"

struct fake_nvram_writer {
	int fail_at;
	int fail_status;
	int fail_errno;
	int second_fail_at;
	int second_fail_status;
	int second_fail_errno;
	unsigned int calls;
	char names[8][32];
	char string_values[8][32];
	int integer_values[8];
	int is_integer[8];
};

static int
fake_nvram_set(void *opaque, const char *name, const char *string_value,
	       int integer_value, int is_integer)
{
	struct fake_nvram_writer *fake = (struct fake_nvram_writer *)opaque;
	unsigned int index = fake->calls++;

	assert(index < 8);
	snprintf(fake->names[index], sizeof(fake->names[index]), "%s", name);
	if (string_value)
		snprintf(fake->string_values[index], sizeof(fake->string_values[index]),
			 "%s", string_value);
	fake->integer_values[index] = integer_value;
	fake->is_integer[index] = is_integer;

	if ((int)index == fake->fail_at) {
		errno = fake->fail_errno;
		return fake->fail_status;
	}
	if ((int)index == fake->second_fail_at) {
		errno = fake->second_fail_errno;
		return fake->second_fail_status;
	}
	return 0;
}

static void
test_success_preserves_key_order_and_value_types(void)
{
	const struct router_runtime_nvram_entry entries[] = {
		{ "login_timestamp", "0000000000", 0, 0 },
		{ "link_internet", NULL, 2, 1 },
		{ "vpnc_dns_t", "", 0, 0 },
		{ "usb_hotplug_lp", NULL, 0, 1 }
	};
	struct fake_nvram_writer fake;

	memset(&fake, 0, sizeof(fake));
	fake.fail_at = -1;
	fake.second_fail_at = -1;

	assert(router_runtime_nvram_reset_apply(entries,
		sizeof(entries) / sizeof(entries[0]), fake_nvram_set, &fake) == 0);
	assert(fake.calls == 4);
	assert(strcmp(fake.names[0], "login_timestamp") == 0);
	assert(strcmp(fake.string_values[0], "0000000000") == 0);
	assert(fake.is_integer[0] == 0);
	assert(strcmp(fake.names[1], "link_internet") == 0);
	assert(fake.integer_values[1] == 2);
	assert(fake.is_integer[1] == 1);
	assert(strcmp(fake.names[2], "vpnc_dns_t") == 0);
	assert(strcmp(fake.string_values[2], "") == 0);
	assert(fake.is_integer[2] == 0);
	assert(strcmp(fake.names[3], "usb_hotplug_lp") == 0);
	assert(fake.integer_values[3] == 0);
	assert(fake.is_integer[3] == 1);
}

static void
test_all_resets_are_attempted_and_first_error_wins(void)
{
	const struct router_runtime_nvram_entry entries[] = {
		{ "first", NULL, 0, 1 },
		{ "second", NULL, 0, 1 },
		{ "third", "", 0, 0 },
		{ "fourth", NULL, 1, 1 }
	};
	struct fake_nvram_writer fake;

	memset(&fake, 0, sizeof(fake));
	fake.fail_at = 1;
	fake.fail_status = -1;
	fake.fail_errno = ENOSPC;
	fake.second_fail_at = 3;
	fake.second_fail_status = EACCES;
	fake.second_fail_errno = EPERM;

	assert(router_runtime_nvram_reset_apply(entries,
		sizeof(entries) / sizeof(entries[0]), fake_nvram_set, &fake) == -1);
	assert(fake.calls == 4);
	assert(errno == ENOSPC);
}

static void
test_first_error_with_zero_errno_does_not_leak_later_errno(void)
{
	const struct router_runtime_nvram_entry entries[] = {
		{ "first", NULL, 0, 1 },
		{ "second", NULL, 0, 1 }
	};
	struct fake_nvram_writer fake;

	memset(&fake, 0, sizeof(fake));
	fake.fail_at = 0;
	fake.fail_status = EIO;
	fake.fail_errno = 0;
	fake.second_fail_at = 1;
	fake.second_fail_status = -1;
	fake.second_fail_errno = EPERM;

	assert(router_runtime_nvram_reset_apply(entries, 2, fake_nvram_set, &fake) == EIO);
	assert(fake.calls == 2);
	assert(errno == 0);
}

static void
test_invalid_batch_has_no_side_effects(void)
{
	const struct router_runtime_nvram_entry invalid_entries[] = {
		{ "first", NULL, 0, 1 },
		{ "", "invalid", 0, 0 }
	};
	struct fake_nvram_writer fake;

	memset(&fake, 0, sizeof(fake));
	fake.fail_at = -1;
	fake.second_fail_at = -1;
	assert(router_runtime_nvram_reset_apply(invalid_entries, 2,
						fake_nvram_set, &fake) == EINVAL);
	assert(fake.calls == 0);
	assert(router_runtime_nvram_reset_apply(NULL, 1,
						fake_nvram_set, &fake) == EINVAL);
	assert(fake.calls == 0);
	assert(router_runtime_nvram_reset_apply(NULL, 0,
						fake_nvram_set, &fake) == 0);
	assert(fake.calls == 0);
	assert(router_runtime_nvram_reset_apply(invalid_entries, 1,
						NULL, &fake) == EINVAL);
	assert(fake.calls == 0);
}

int
main(void)
{
	test_success_preserves_key_order_and_value_types();
	test_all_resets_are_attempted_and_first_error_wins();
	test_first_error_with_zero_errno_does_not_leak_later_errno();
	test_invalid_batch_has_no_side_effects();
	puts("Router runtime NVRAM reset policy regression: PASS");
	return 0;
}
