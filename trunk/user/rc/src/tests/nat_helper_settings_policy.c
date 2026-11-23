#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "nat_helper_settings_policy.h"

struct fake_nvram {
	const char *values[6];
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
	static const char *const names[] = {
		"wan_nat_x", "nf_alg_ftp0", "nf_alg_ftp1",
		"nf_alg_pptp", "nf_alg_h323", "nf_alg_sip"
	};
	struct fake_nvram *fake = (struct fake_nvram *)opaque;
	unsigned int index = fake->read_calls++;

	assert(index < 6);
	assert(strcmp(name, names[index]) == 0);
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
fake_init(struct fake_nvram *fake)
{
	memset(fake, 0, sizeof(*fake));
	fake->values[0] = "1";
	fake->values[1] = "21";
	fake->values[2] = "2121";
	fake->values[3] = "1";
	fake->values[4] = "0";
	fake->values[5] = "1";
	fake->read_fail_at = -1;
	fake->missing_at = -1;
}

static void
test_router_mode_reads_one_coherent_nat_helper_tuple(void)
{
	struct fake_nvram fake;
	struct rc_nat_helper_settings settings;

	fake_init(&fake);
	memset(&settings, 0, sizeof(settings));
	assert(rc_nat_helper_settings_read(0, fake_begin, fake_get, fake_end,
					   &fake, &settings) == 0);
	assert(fake.begin_calls == 1 && fake.read_calls == 6 &&
	       fake.end_calls == 1);
	assert(settings.wan_nat_x == 1);
	assert(settings.ftp0_port == 21 && settings.ftp1_port == 2121);
	assert(settings.pptp_enabled == 1 && settings.h323_enabled == 0 &&
	       settings.sip_enabled == 1);
}

static void
test_missing_keys_keep_legacy_disabled_defaults_and_port_ranges(void)
{
	struct fake_nvram fake;
	struct rc_nat_helper_settings settings;

	fake_init(&fake);
	fake.missing_at = 0;
	fake.values[1] = "20";
	fake.values[2] = "1023";
	memset(&settings, 0xa5, sizeof(settings));
	assert(rc_nat_helper_settings_read(0, fake_begin, fake_get, fake_end,
					   &fake, &settings) == 0);
	assert(fake.begin_calls == 1 && fake.read_calls == 6 &&
	       fake.end_calls == 1);
	assert(settings.wan_nat_x == 0);
	assert(settings.ftp0_port == 0 && settings.ftp1_port == 0);
	assert(settings.pptp_enabled == 1 && settings.h323_enabled == 0 &&
	       settings.sip_enabled == 1);
	/* Missing ALG key is the exact empty string, so it stays disabled. */
	fake_init(&fake);
	fake.missing_at = 5;
	assert(rc_nat_helper_settings_read(0, fake_begin, fake_get, fake_end,
					   &fake, &settings) == 0);
	assert(settings.sip_enabled == 0);
}

static void
test_each_snapshot_read_error_preserves_current_module_decisions(void)
{
	int failure;
	for (failure = 0; failure < 6; ++failure) {
		struct fake_nvram fake;
		struct rc_nat_helper_settings settings;
		struct rc_nat_helper_settings before;

		fake_init(&fake);
		fake.read_fail_at = failure;
		fake.read_error = EACCES;
		settings.wan_nat_x = 1;
		settings.ftp0_port = 2121;
		settings.ftp1_port = 2021;
		settings.pptp_enabled = 1;
		settings.h323_enabled = 1;
		settings.sip_enabled = 1;
		before = settings;
		assert(rc_nat_helper_settings_read(0, fake_begin, fake_get, fake_end,
						   &fake, &settings) == EACCES);
		assert(fake.begin_calls == 1 && fake.end_calls == 1);
		assert(memcmp(&settings, &before, sizeof(settings)) == 0);
	}
}

static void
test_transaction_close_error_preserves_current_module_decisions(void)
{
	struct fake_nvram fake;
	struct rc_nat_helper_settings settings;
	struct rc_nat_helper_settings before;

	fake_init(&fake);
	fake.end_error = EIO;
	memset(&settings, 0x5a, sizeof(settings));
	before = settings;
	assert(rc_nat_helper_settings_read(0, fake_begin, fake_get, fake_end,
					   &fake, &settings) == EIO);
	assert(fake.begin_calls == 1 && fake.read_calls == 6 &&
	       fake.end_calls == 1);
	assert(memcmp(&settings, &before, sizeof(settings)) == 0);
}

static void
test_invalid_integer_and_oversized_value_do_not_publish_settings(void)
{
	struct fake_nvram fake;
	struct rc_nat_helper_settings settings;
	struct rc_nat_helper_settings before;
	char oversized[48];
	int status;

	fake_init(&fake);
	fake.values[1] = "21junk";
	memset(&settings, 0x33, sizeof(settings));
	before = settings;
	assert(rc_nat_helper_settings_read(0, fake_begin, fake_get, fake_end,
					   &fake, &settings) == EINVAL);
	assert(memcmp(&settings, &before, sizeof(settings)) == 0);

	fake_init(&fake);
	memset(oversized, '7', sizeof(oversized) - 1U);
	oversized[sizeof(oversized) - 1U] = '\0';
	fake.values[1] = oversized;
	status = rc_nat_helper_settings_read(0, fake_begin, fake_get, fake_end,
					     &fake, &settings);
	assert(status == ENAMETOOLONG);
	assert(memcmp(&settings, &before, sizeof(settings)) == 0);
}

static void
test_boolean_selectors_reject_unknown_values_without_publishing(void)
{
	static const int selector_indices[] = { 0, 3, 4, 5 };
	static const char *const invalid_values[] = { "2", "-1", "1junk" };
	unsigned int i, j;

	for (i = 0; i < sizeof(selector_indices) / sizeof(selector_indices[0]); ++i) {
		for (j = 0; j < sizeof(invalid_values) / sizeof(invalid_values[0]); ++j) {
			struct fake_nvram fake;
			struct rc_nat_helper_settings settings;
			struct rc_nat_helper_settings before;

			fake_init(&fake);
			fake.values[selector_indices[i]] = invalid_values[j];
			settings.wan_nat_x = 1;
			settings.ftp0_port = 2121;
			settings.ftp1_port = 2021;
			settings.pptp_enabled = 1;
			settings.h323_enabled = 1;
			settings.sip_enabled = 1;
			before = settings;

			assert(rc_nat_helper_settings_read(0, fake_begin, fake_get, fake_end,
						   &fake, &settings) == EINVAL);
			assert(fake.begin_calls == 1 && fake.end_calls == 1);
			assert(memcmp(&settings, &before, sizeof(settings)) == 0);
		}
	}
}

static void
test_ap_mode_reads_only_wan_nat_selector(void)
{
	struct fake_nvram fake;
	struct rc_nat_helper_settings settings;

	fake_init(&fake);
	assert(rc_nat_helper_settings_read(1, fake_begin, fake_get, fake_end,
					   &fake, &settings) == 0);
	assert(fake.begin_calls == 1 && fake.read_calls == 1 &&
	       fake.end_calls == 1);
	assert(settings.wan_nat_x == 1);
	assert(settings.ftp0_port == 0 && settings.ftp1_port == 0 &&
	       settings.pptp_enabled == 0 && settings.h323_enabled == 0 &&
	       settings.sip_enabled == 0);
}

static void
test_invalid_arguments_fail_closed(void)
{
	struct fake_nvram fake;
	struct rc_nat_helper_settings settings;

	fake_init(&fake);
	assert(rc_nat_helper_settings_read(0, NULL, fake_get, fake_end,
					   &fake, &settings) == EINVAL);
	assert(rc_nat_helper_settings_read(0, fake_begin, fake_get, fake_end,
					   &fake, NULL) == EINVAL);
	assert(fake.begin_calls == 0);
}

int
main(void)
{
	test_router_mode_reads_one_coherent_nat_helper_tuple();
	test_missing_keys_keep_legacy_disabled_defaults_and_port_ranges();
	test_each_snapshot_read_error_preserves_current_module_decisions();
	test_transaction_close_error_preserves_current_module_decisions();
	test_invalid_integer_and_oversized_value_do_not_publish_settings();
	test_boolean_selectors_reject_unknown_values_without_publishing();
	test_ap_mode_reads_only_wan_nat_selector();
	test_invalid_arguments_fail_closed();
	puts("NAT helper module setting snapshot: PASS");
	return 0;
}
