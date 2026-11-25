#include <assert.h>
#include <errno.h>
#include <string.h>

#include "iptv_proxy_nvram_policy.h"

struct fake_nvram {
	const char *viptv_ifname;
	const char *mr_enable;
	const char *mr_altnet;
	const char *mr_qleave;
	const char *fail_name;
	int failure_status;
	int altnet_reads;
	int quickleave_reads;
};

static int
fake_lookup(struct fake_nvram *fake, const char *name, const char **value)
{
	if (!fake || !name || !value)
		return EINVAL;
	*value = NULL;
	if (fake->fail_name && strcmp(fake->fail_name, name) == 0)
		return fake->failure_status > 0 ? fake->failure_status : EIO;
	if (strcmp(name, "viptv_ifname") == 0)
		*value = fake->viptv_ifname;
	else if (strcmp(name, "mr_enable_x") == 0)
		*value = fake->mr_enable;
	else if (strcmp(name, "mr_altnet_x") == 0) {
		++fake->altnet_reads;
		*value = fake->mr_altnet;
	} else if (strcmp(name, "mr_qleave_x") == 0) {
		++fake->quickleave_reads;
		*value = fake->mr_qleave;
	} else
		return EINVAL;
	return 0;
}

static int
fake_copy(void *context, const char *name, char *destination, size_t size)
{
	struct fake_nvram *fake = (struct fake_nvram *)context;
	const char *value;
	size_t length;
	int status;

	if (!destination || size == 0)
		return EINVAL;
	destination[0] = '\0';
	status = fake_lookup(fake, name, &value);
	if (status != 0)
		return status;
	if (!value)
		value = "";
	length = strlen(value);
	if (length >= size)
		return ENAMETOOLONG;
	memcpy(destination, value, length + 1);
	return 0;
}

static int
fake_match(void *context, const char *name, const char *expected,
	   const char *missing_value, int *matches)
{
	struct fake_nvram *fake = (struct fake_nvram *)context;
	const char *value;
	int status;

	if (!expected || !missing_value || !matches)
		return EINVAL;
	*matches = 0;
	status = fake_lookup(fake, name, &value);
	if (status != 0)
		return status;
	if (!value)
		value = missing_value;
	*matches = strcmp(value, expected) == 0;
	return 0;
}

static int
take_snapshot(struct fake_nvram *fake,
	      struct iptv_proxy_nvram_settings *settings)
{
	return iptv_proxy_nvram_settings_read(settings, fake_copy, fake_match,
					      fake);
}

static void
test_missing_keys_preserve_legacy_defaults(void)
{
	struct fake_nvram fake;
	struct iptv_proxy_nvram_settings settings;

	memset(&fake, 0, sizeof(fake));
	assert(take_snapshot(&fake, &settings) == 0);
	assert(settings.viptv_ifname[0] == '\0');
	assert(settings.enabled == 1);
	assert(strcmp(settings.altnet, IPTV_PROXY_ALTN_DEFAULT) == 0);
	assert(settings.quickleave == 0);
	assert(fake.altnet_reads == 1 && fake.quickleave_reads == 2);

	/* Explicitly empty values normalize to the same legacy default. */
	fake.mr_enable = "1";
	fake.mr_altnet = "";
	assert(take_snapshot(&fake, &settings) == 0);
	assert(strcmp(settings.altnet, IPTV_PROXY_ALTN_DEFAULT) == 0);
}

static void
test_configured_values_are_copied_to_one_snapshot(void)
{
	struct fake_nvram fake;
	struct iptv_proxy_nvram_settings settings;

	memset(&fake, 0, sizeof(fake));
	fake.viptv_ifname = "br0.2";
	fake.mr_enable = "1";
	fake.mr_altnet = "239.0.0.0/8";
	fake.mr_qleave = "1";
	assert(take_snapshot(&fake, &settings) == 0);
	assert(strcmp(settings.viptv_ifname, "br0.2") == 0);
	assert(settings.enabled == 1);
	assert(strcmp(settings.altnet, "239.0.0.0/8") == 0);
	assert(settings.quickleave == 1);
}

static void
test_disabled_proxy_skips_dependent_reads(void)
{
	struct fake_nvram fake;
	struct iptv_proxy_nvram_settings settings;

	memset(&fake, 0, sizeof(fake));
	fake.mr_enable = "0";
	fake.fail_name = "mr_altnet_x";
	fake.failure_status = EIO;
	assert(take_snapshot(&fake, &settings) == 0);
	assert(settings.enabled == 0);
	assert(settings.altnet[0] == '\0' && settings.quickleave == 0);
	assert(fake.altnet_reads == 0 && fake.quickleave_reads == 0);
}

static void
test_any_required_read_failure_invalidates_snapshot(void)
{
	static const char *const keys[] = {
		"viptv_ifname", "mr_enable_x", "mr_altnet_x", "mr_qleave_x"
	};
	unsigned int i;

	for (i = 0; i < sizeof(keys) / sizeof(keys[0]); ++i) {
		struct fake_nvram fake;
		struct iptv_proxy_nvram_settings settings;

		memset(&fake, 0, sizeof(fake));
		fake.mr_enable = "1";
		fake.mr_altnet = "239.0.0.0/8";
		fake.mr_qleave = "1";
		fake.fail_name = keys[i];
		fake.failure_status = EACCES;
		memset(&settings, 0x5a, sizeof(settings));
		assert(take_snapshot(&fake, &settings) == EACCES);
		assert(settings.viptv_ifname[0] == '\0');
		assert(settings.altnet[0] == '\0');
		assert(settings.enabled == 0 && settings.quickleave == 0);
	}
}

static void
test_invalid_altnet_values_fail_closed(void)
{
	static const char *const invalid[] = {
		"bad", "192.0.2.256/24", "256.0.2.1/24",
		"192.0.2/24", "192.0.2.1", "192.0.2.1/33",
		"192.0.2.1/", "192.0.2.1/24/8",
		"010.0.0.0/8", "192.168.001.1/24",
		"255.255.255.255/32", "192.0.2.1/24 extra"
	};
	unsigned int i;

	for (i = 0; i < sizeof(invalid) / sizeof(invalid[0]); ++i) {
		struct fake_nvram fake;
		struct iptv_proxy_nvram_settings settings;

		memset(&fake, 0, sizeof(fake));
		fake.mr_enable = "1";
		fake.mr_altnet = invalid[i];
		fake.mr_qleave = "1";
		memset(&settings, 0x5a, sizeof(settings));
		assert(take_snapshot(&fake, &settings) == EINVAL);
		assert(settings.altnet[0] == '\0');
		assert(settings.viptv_ifname[0] == '\0');
		assert(settings.enabled == 0 && settings.quickleave == 0);
	}
}

static void
test_invalid_boolean_selectors_fail_closed(void)
{
	static const char *const invalid[] = { "", "2", "true", "-1", "01" };
	unsigned int i;
	for (i = 0; i < sizeof(invalid) / sizeof(invalid[0]); ++i) {
		struct fake_nvram fake;
		struct iptv_proxy_nvram_settings settings;
		memset(&fake, 0, sizeof(fake));
		fake.mr_enable = invalid[i];
		fake.mr_altnet = "239.0.0.0/8";
		fake.mr_qleave = "1";
		memset(&settings, 0x5a, sizeof(settings));
		assert(take_snapshot(&fake, &settings) == EINVAL);
		assert(settings.enabled == 0 && settings.quickleave == 0);
		assert(settings.viptv_ifname[0] == '\0' && settings.altnet[0] == '\0');
		assert(fake.altnet_reads == 0 && fake.quickleave_reads == 0);
	}
	for (i = 0; i < sizeof(invalid) / sizeof(invalid[0]); ++i) {
		struct fake_nvram fake;
		struct iptv_proxy_nvram_settings settings;
		memset(&fake, 0, sizeof(fake));
		fake.mr_enable = "1";
		fake.mr_altnet = "239.0.0.0/8";
		fake.mr_qleave = invalid[i];
		memset(&settings, 0x5a, sizeof(settings));
		assert(take_snapshot(&fake, &settings) == EINVAL);
		assert(settings.enabled == 0 && settings.quickleave == 0);
		assert(settings.viptv_ifname[0] == '\0' && settings.altnet[0] == '\0');
	}
	/* Missing selectors retain their documented legacy defaults. */
	{
		struct fake_nvram fake;
		struct iptv_proxy_nvram_settings settings;
		memset(&fake, 0, sizeof(fake));
		assert(take_snapshot(&fake, &settings) == 0);
		assert(settings.enabled == 1 && settings.quickleave == 0);
	}
}

static void
test_oversized_values_fail_closed(void)
{
	struct fake_nvram fake;
	struct iptv_proxy_nvram_settings settings;

	memset(&fake, 0, sizeof(fake));
	fake.viptv_ifname = "interface-name-that-is-too-long";
	fake.mr_enable = "1";
	assert(take_snapshot(&fake, &settings) == ENAMETOOLONG);
	assert(settings.viptv_ifname[0] == '\0' && settings.enabled == 0);

	memset(&fake, 0, sizeof(fake));
	fake.mr_enable = "1";
	fake.mr_altnet = "239.1.1.1/32-and-extra-data-that-overflows-the-buffer-and-is-definitely-longer-than-sixty-four-bytes";
	assert(take_snapshot(&fake, &settings) == ENAMETOOLONG);
	assert(settings.altnet[0] == '\0' && settings.enabled == 0);
}

int
main(void)
{
	test_missing_keys_preserve_legacy_defaults();
	test_configured_values_are_copied_to_one_snapshot();
	test_disabled_proxy_skips_dependent_reads();
	test_any_required_read_failure_invalidates_snapshot();
	test_invalid_altnet_values_fail_closed();
	test_invalid_boolean_selectors_fail_closed();
	test_oversized_values_fail_closed();
	return 0;
}
