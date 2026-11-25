#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "nat_offload_settings_policy.h"

struct fake_nvram {
	const char *names[4];
	const char *values[4];
	unsigned int count;
	unsigned int read_calls;
	unsigned int begin_calls;
	unsigned int end_calls;
	int read_fail_at;
	int missing_at;
	int read_error;
	int begin_error;
	int end_error;
	char cache[64];
};

static int
fake_begin(void *opaque)
{
	struct fake_nvram *fake = (struct fake_nvram *)opaque;
	++fake->begin_calls;
	if (fake->begin_error) {
		errno = fake->begin_error;
		return -1;
	}
	return 0;
}

static int
fake_get(void *opaque, const char *name, char **value)
{
	struct fake_nvram *fake = (struct fake_nvram *)opaque;
	unsigned int index = fake->read_calls++;

	assert(index < fake->count);
	assert(strcmp(name, fake->names[index]) == 0);
	if ((int)index == fake->read_fail_at) {
		*value = NULL;
		errno = fake->read_error;
		return -1;
	}
	if ((int)index == fake->missing_at) {
		*value = NULL;
		return 0;
	}
	snprintf(fake->cache, sizeof(fake->cache), "%s", fake->values[index]);
	*value = fake->cache;
	return 1;
}

static int
fake_end(void *opaque)
{
	struct fake_nvram *fake = (struct fake_nvram *)opaque;
	++fake->end_calls;
	if (fake->end_error) {
		errno = fake->end_error;
		return -1;
	}
	return 0;
}

static void
fake_init(struct fake_nvram *fake, int read_hwnat, int read_sfe)
{
	memset(fake, 0, sizeof(*fake));
	fake->read_fail_at = -1;
	fake->missing_at = -1;
	if (read_hwnat) {
		fake->names[fake->count] = "sw_mode";
		fake->values[fake->count++] = "4";
		fake->names[fake->count] = "hw_nat_mode";
		fake->values[fake->count++] = "2";
		fake->names[fake->count] = "ip6_lan_auto";
		fake->values[fake->count++] = "1";
	}
	if (read_sfe) {
		fake->names[fake->count] = "sfe_enable";
		fake->values[fake->count++] = "1";
	}
}

static void
test_hwnat_and_sfe_selectors_share_one_snapshot(void)
{
	struct fake_nvram fake;
	struct rc_nat_offload_settings settings;

	fake_init(&fake, 1, 1);
	memset(&settings, 0, sizeof(settings));
	assert(rc_nat_offload_settings_read(1, 1, fake_begin, fake_get,
					    fake_end, &fake, &settings) == 0);
	assert(fake.begin_calls == 1 && fake.read_calls == 4 &&
	       fake.end_calls == 1);
	assert(settings.hwnat_allowed == 1);
	assert(settings.hw_nat_mode == 2);
	assert(settings.ipv6_nat == 1);
	assert(settings.sfe_enabled == 1);
}

static void
test_hwnat_enable_predicate_preserves_legacy_mode_rules(void)
{
	struct fake_nvram fake;
	struct rc_nat_offload_settings settings;

	fake_init(&fake, 1, 0);
	fake.values[0] = "1";
	fake.values[1] = "2";
	assert(rc_nat_offload_settings_read(1, 0, fake_begin, fake_get,
					    fake_end, &fake, &settings) == 0);
	assert(settings.hwnat_allowed == 1 && settings.hw_nat_mode == 2);
	fake_init(&fake, 1, 0);
	fake.values[0] = "2";
	assert(rc_nat_offload_settings_read(1, 0, fake_begin, fake_get,
					    fake_end, &fake, &settings) == 0);
	assert(settings.hwnat_allowed == 0);
	fake_init(&fake, 1, 0);
	fake.values[1] = "0";
	assert(rc_nat_offload_settings_read(1, 0, fake_begin, fake_get,
					    fake_end, &fake, &settings) == 0);
	assert(settings.hwnat_allowed == 0);
}

static void
test_missing_values_keep_legacy_zero_defaults(void)
{
	struct fake_nvram fake;
	struct rc_nat_offload_settings settings;

	fake_init(&fake, 1, 1);
	fake.missing_at = 0;
	assert(rc_nat_offload_settings_read(1, 1, fake_begin, fake_get,
					    fake_end, &fake, &settings) == 0);
	assert(settings.hwnat_allowed == 0);
	assert(settings.hw_nat_mode == 2);
	assert(settings.ipv6_nat == 1);
	assert(settings.sfe_enabled == 1);
	fake_init(&fake, 1, 1);
	fake.missing_at = 3;
	assert(rc_nat_offload_settings_read(1, 1, fake_begin, fake_get,
					    fake_end, &fake, &settings) == 0);
	assert(settings.sfe_enabled == 0);
}

static void
test_every_snapshot_read_error_preserves_previous_settings(void)
{
	int failure;
	for (failure = 0; failure < 4; ++failure) {
		struct fake_nvram fake;
		struct rc_nat_offload_settings settings;
		struct rc_nat_offload_settings before;

		fake_init(&fake, 1, 1);
		fake.read_fail_at = failure;
		fake.read_error = EACCES;
		settings.hwnat_allowed = 1;
		settings.hw_nat_mode = 1;
		settings.ipv6_nat = 1;
		settings.sfe_enabled = 1;
		before = settings;
		assert(rc_nat_offload_settings_read(1, 1, fake_begin, fake_get,
					    fake_end, &fake, &settings) == EACCES);
		assert(fake.begin_calls == 1 && fake.end_calls == 1);
		assert(memcmp(&settings, &before, sizeof(settings)) == 0);
	}
}

static void
test_transaction_failure_and_invalid_values_do_not_publish_settings(void)
{
	struct fake_nvram fake;
	struct rc_nat_offload_settings settings;
	struct rc_nat_offload_settings before;
	char oversized[64];
	int status;

	fake_init(&fake, 1, 1);
	fake.end_error = EIO;
	memset(&settings, 0x5a, sizeof(settings));
	before = settings;
	assert(rc_nat_offload_settings_read(1, 1, fake_begin, fake_get,
					    fake_end, &fake, &settings) == EIO);
	assert(memcmp(&settings, &before, sizeof(settings)) == 0);

	fake_init(&fake, 1, 1);
	fake.values[1] = "1junk";
	assert(rc_nat_offload_settings_read(1, 1, fake_begin, fake_get,
					    fake_end, &fake, &settings) == EINVAL);
	assert(memcmp(&settings, &before, sizeof(settings)) == 0);

	fake_init(&fake, 1, 1);
	memset(oversized, '9', sizeof(oversized) - 1U);
	oversized[sizeof(oversized) - 1U] = '\0';
	fake.values[1] = oversized;
	status = rc_nat_offload_settings_read(1, 1, fake_begin, fake_get,
					     fake_end, &fake, &settings);
	assert(status == ENAMETOOLONG);
	assert(memcmp(&settings, &before, sizeof(settings)) == 0);
}

static void
test_invalid_ipv6_lan_auto_values_preserve_previous_settings(void)
{
	static const char *const invalid_values[] = { "2", "-1", "1junk" };
	unsigned int i;

	for (i = 0; i < sizeof(invalid_values) / sizeof(invalid_values[0]); ++i) {
		struct fake_nvram fake;
		struct rc_nat_offload_settings settings;
		struct rc_nat_offload_settings before;

		fake_init(&fake, 1, 1);
		fake.values[2] = invalid_values[i];
		settings.hwnat_allowed = 1;
		settings.hw_nat_mode = 2;
		settings.ipv6_nat = 1;
		settings.sfe_enabled = 2;
		before = settings;

		assert(rc_nat_offload_settings_read(1, 1, fake_begin, fake_get,
					    fake_end, &fake, &settings) == EINVAL);
		assert(fake.begin_calls == 1 && fake.read_calls == 4 &&
		       fake.end_calls == 1);
		assert(memcmp(&settings, &before, sizeof(settings)) == 0);
	}
}

static void
test_invalid_sfe_modes_preserve_previous_settings(void)
{
	static const char *const invalid_values[] = { "3", "-1", "1junk" };
	struct fake_nvram fake;
	struct rc_nat_offload_settings settings;
	struct rc_nat_offload_settings before;
	unsigned int i;

	for (i = 0; i < sizeof(invalid_values) / sizeof(invalid_values[0]); ++i) {
		fake_init(&fake, 0, 1);
		fake.values[0] = invalid_values[i];
		settings.hwnat_allowed = 1;
		settings.hw_nat_mode = 2;
		settings.ipv6_nat = 1;
		settings.sfe_enabled = 2;
		before = settings;

		assert(rc_nat_offload_settings_read(0, 1, fake_begin, fake_get,
						    fake_end, &fake, &settings) == EINVAL);
		assert(fake.begin_calls == 1 && fake.read_calls == 1 &&
		       fake.end_calls == 1);
		assert(memcmp(&settings, &before, sizeof(settings)) == 0);
	}
}

static void
test_selective_reads_and_no_feature_defaults(void)
{
	struct fake_nvram fake;
	struct rc_nat_offload_settings settings;

	fake_init(&fake, 1, 0);
	assert(rc_nat_offload_settings_read(1, 0, fake_begin, fake_get,
					    fake_end, &fake, &settings) == 0);
	assert(fake.read_calls == 3 && fake.begin_calls == 1 &&
	       fake.end_calls == 1);
	fake_init(&fake, 0, 1);
	assert(strcmp(fake.names[0], "sfe_enable") == 0);
	assert(rc_nat_offload_settings_read(0, 1, fake_begin, fake_get,
					    fake_end, &fake, &settings) == 0);
	assert(fake.read_calls == 1 && settings.sfe_enabled == 1);

	fake_init(&fake, 0, 1);
	fake.values[0] = "2";
	assert(rc_nat_offload_settings_read(0, 1, fake_begin, fake_get,
					    fake_end, &fake, &settings) == 0);
	assert(fake.read_calls == 1 && settings.sfe_enabled == 2);
	fake_init(&fake, 0, 0);
	memset(&settings, 0x7f, sizeof(settings));
	assert(rc_nat_offload_settings_read(0, 0, NULL, NULL, NULL,
					    NULL, &settings) == 0);
	assert(settings.hwnat_allowed == 0 && settings.hw_nat_mode == 0 &&
	       settings.ipv6_nat == 0 && settings.sfe_enabled == 0);
}

static void
test_invalid_arguments_fail_closed(void)
{
	struct fake_nvram fake;
	struct rc_nat_offload_settings settings;

	fake_init(&fake, 1, 1);
	assert(rc_nat_offload_settings_read(2, 1, fake_begin, fake_get,
					    fake_end, &fake, &settings) == EINVAL);
	assert(rc_nat_offload_settings_read(1, 0, NULL, fake_get,
					    fake_end, &fake, &settings) == EINVAL);
	assert(fake.begin_calls == 0);
}

int
main(void)
{
	test_hwnat_and_sfe_selectors_share_one_snapshot();
	test_hwnat_enable_predicate_preserves_legacy_mode_rules();
	test_missing_values_keep_legacy_zero_defaults();
	test_every_snapshot_read_error_preserves_previous_settings();
	test_transaction_failure_and_invalid_values_do_not_publish_settings();
	test_invalid_ipv6_lan_auto_values_preserve_previous_settings();
	test_invalid_sfe_modes_preserve_previous_settings();
	test_selective_reads_and_no_feature_defaults();
	test_invalid_arguments_fail_closed();
	puts("NAT offload setting snapshot: PASS");
	return 0;
}
