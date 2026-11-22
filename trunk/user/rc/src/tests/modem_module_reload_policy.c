#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "../modem_module_reload_policy.h"

#define TEST_MODULES 3

static const char *const module_names[TEST_MODULES] = {
	"cdc_ether", "rndis_host", "cdc_ncm"
};
static const char *expected_params[TEST_MODULES];
static int loaded[TEST_MODULES];
static int become_loaded[TEST_MODULES];
static int load_results[TEST_MODULES];
static int load_calls[TEST_MODULES];
static int probe_calls[TEST_MODULES];

static int
find_module_index(const char *name)
{
	int i;

	for (i = 0; i < TEST_MODULES; i++) {
		if (strcmp(name, module_names[i]) == 0)
			return i;
	}
	return -1;
}

static int
mock_load(const char *name, const char *param)
{
	int index = find_module_index(name);

	assert(index >= 0);
	if (expected_params[index] == NULL)
		assert(param == NULL);
	else
		assert(param != NULL && strcmp(param, expected_params[index]) == 0);
	load_calls[index]++;
	if (become_loaded[index])
		loaded[index] = 1;
	return load_results[index];
}

static int
mock_is_loaded(const char *name)
{
	int index = find_module_index(name);

	assert(index >= 0);
	probe_calls[index]++;
	return loaded[index];
}

static const rc_modem_module_load_ops_t mock_ops = {
	mock_load,
	mock_is_loaded
};

static void
reset_case(void)
{
	int i;

	for (i = 0; i < TEST_MODULES; i++) {
		expected_params[i] = NULL;
		loaded[i] = 0;
		become_loaded[i] = 0;
		load_results[i] = 0;
		load_calls[i] = 0;
		probe_calls[i] = 0;
	}
}

static void
assert_all_attempted(void)
{
	int i;

	for (i = 0; i < TEST_MODULES; i++) {
		assert(load_calls[i] == 1);
		assert(probe_calls[i] == 1);
	}
}


struct modem_settings_fixture {
	int modem_rule;
	int modem_type;
	int fail_call;
	int fail_status;
	int calls;
};

static struct modem_settings_fixture *active_settings_fixture;

static int
mock_read_modem_setting(const char *name, int missing_value, int *value)
{
	struct modem_settings_fixture *fixture = active_settings_fixture;

	assert(fixture != NULL);
	assert(value != NULL);
	assert(missing_value == 0);
	fixture->calls++;
	if (fixture->calls == fixture->fail_call)
		return fixture->fail_status;
	if (strcmp(name, "modem_rule") == 0)
		*value = fixture->modem_rule;
	else if (strcmp(name, "modem_type") == 0)
		*value = fixture->modem_type;
	else
		return EINVAL;
	return 0;
}

static void
test_modem_settings_snapshot_reads_complete_pair(void)
{
	struct modem_settings_fixture fixture;
	int modem_rule = 77, modem_type = 88;

	memset(&fixture, 0, sizeof(fixture));
	fixture.modem_rule = 3;
	fixture.modem_type = 2;
	active_settings_fixture = &fixture;
	assert(rc_modem_read_nvram_settings_status(
		mock_read_modem_setting, &modem_rule, &modem_type) == 0);
	assert(fixture.calls == 2);
	assert(modem_rule == 3 && modem_type == 2);
}

static void
test_modem_settings_first_read_error_publishes_no_values(void)
{
	struct modem_settings_fixture fixture;
	int modem_rule = 77, modem_type = 88;

	memset(&fixture, 0, sizeof(fixture));
	fixture.fail_call = 1;
	fixture.fail_status = EIO;
	active_settings_fixture = &fixture;
	assert(rc_modem_read_nvram_settings_status(
		mock_read_modem_setting, &modem_rule, &modem_type) == EIO);
	assert(fixture.calls == 1);
	assert(modem_rule == 0 && modem_type == 0);
}

static void
test_modem_settings_second_read_error_discards_partial_pair(void)
{
	struct modem_settings_fixture fixture;
	int modem_rule = 77, modem_type = 88;

	memset(&fixture, 0, sizeof(fixture));
	fixture.modem_rule = 3;
	fixture.fail_call = 2;
	fixture.fail_status = EACCES;
	active_settings_fixture = &fixture;
	assert(rc_modem_read_nvram_settings_status(
		mock_read_modem_setting, &modem_rule, &modem_type) == EACCES);
	assert(fixture.calls == 2);
	assert(modem_rule == 0 && modem_type == 0);
}

static void
test_modem_settings_absent_keys_keep_legacy_defaults(void)
{
	struct modem_settings_fixture fixture;
	int modem_rule = 77, modem_type = 88;

	memset(&fixture, 0, sizeof(fixture));
	/* A checked reader returns success and missing_value for absent keys. */
	active_settings_fixture = &fixture;
	assert(rc_modem_read_nvram_settings_status(
		mock_read_modem_setting, &modem_rule, &modem_type) == 0);
	assert(fixture.calls == 2);
	assert(modem_rule == 0 && modem_type == 0);
}

static void
test_modem_settings_reject_invalid_arguments(void)
{
	int modem_rule = 9, modem_type = 8;

	assert(rc_modem_read_nvram_settings_status(
		NULL, &modem_rule, &modem_type) == EINVAL);
	assert(rc_modem_read_nvram_settings_status(
		mock_read_modem_setting, NULL, &modem_type) == EINVAL);
	assert(rc_modem_read_nvram_settings_status(
		mock_read_modem_setting, &modem_rule, NULL) == EINVAL);
	assert(modem_rule == 9 && modem_type == 8);
}

static void
test_one_available_alternative_is_sufficient(void)
{
	size_t available = 0;

	reset_case();
	become_loaded[1] = 1;
	load_results[1] = 1;
	assert(rc_modem_module_load_group_verified(module_names, NULL,
						  TEST_MODULES, &mock_ops,
						  &available) == 0);
	assert(available == 1);
	assert_all_attempted();
}

static void
test_already_loaded_alternative_counts_as_ready(void)
{
	size_t available = 0;

	reset_case();
	loaded[2] = 1;
	/* The legacy loader returns zero when the module is already present. */
	assert(rc_modem_module_load_group_verified(module_names, NULL,
						  TEST_MODULES, &mock_ops,
						  &available) == 0);
	assert(available == 1);
	assert_all_attempted();
}

static void
test_all_missing_alternatives_fail_the_group(void)
{
	size_t available = 99;

	reset_case();
	load_results[0] = -1;
	load_results[1] = 0;
	load_results[2] = -1;
	assert(rc_modem_module_load_group_verified(module_names, NULL,
						  TEST_MODULES, &mock_ops,
						  &available) == ENODEV);
	assert(available == 0);
	assert_all_attempted();
}

static void
test_driver_specific_parameter_is_forwarded(void)
{
	const char *params[TEST_MODULES] = { NULL, NULL, "prefer_mbim=0" };
	size_t available = 0;

	reset_case();
	expected_params[2] = "prefer_mbim=0";
	become_loaded[0] = 1;
	assert(rc_modem_module_load_group_verified(module_names, params,
						  TEST_MODULES, &mock_ops,
						  &available) == 0);
	assert(available == 1);
	assert_all_attempted();
}

static void
test_unknown_probe_is_not_counted_as_available(void)
{
	size_t available = 99;

	reset_case();
	loaded[0] = -EACCES;
	loaded[1] = -EPERM;
	assert(rc_modem_module_load_group_verified(module_names, NULL,
						  TEST_MODULES, &mock_ops,
						  &available) == EACCES);
	assert(available == 0);
	assert_all_attempted();

	/* One confirmed loaded alternative is sufficient despite another UNKNOWN. */
	reset_case();
	loaded[0] = 1;
	loaded[1] = -EACCES;
	assert(rc_modem_module_load_group_verified(module_names, NULL,
						  TEST_MODULES, &mock_ops,
						  &available) == 0);
	assert(available == 1);
	assert_all_attempted();
}

static void
test_invalid_probe_value_fails_closed(void)
{
	size_t available = 99;

	reset_case();
	loaded[0] = 2;
	assert(rc_modem_module_load_group_verified(module_names, NULL,
						  TEST_MODULES, &mock_ops,
						  &available) == EIO);
	assert(available == 0);
	assert_all_attempted();
}

static void
test_invalid_group_fails_before_side_effects(void)
{
	const char *const invalid_names[TEST_MODULES] = {
		"cdc_ether", NULL, "cdc_ncm"
	};
	rc_modem_module_load_ops_t no_loader = { NULL, mock_is_loaded };
	rc_modem_module_load_ops_t no_probe = { mock_load, NULL };
	size_t available = 99;
	int i;

	reset_case();
	assert(rc_modem_module_load_group_verified(NULL, NULL, TEST_MODULES,
						  &mock_ops, &available) == EINVAL);
	assert(available == 0);
	assert(rc_modem_module_load_group_verified(module_names, NULL, 0,
						  &mock_ops, &available) == EINVAL);
	assert(rc_modem_module_load_group_verified(invalid_names, NULL,
						  TEST_MODULES, &mock_ops,
						  &available) == EINVAL);
	assert(rc_modem_module_load_group_verified(module_names, NULL,
						  TEST_MODULES, &no_loader,
						  &available) == EINVAL);
	assert(rc_modem_module_load_group_verified(module_names, NULL,
						  TEST_MODULES, &no_probe,
						  &available) == EINVAL);
	for (i = 0; i < TEST_MODULES; i++)
		assert(load_calls[i] == 0 && probe_calls[i] == 0);
}

int
main(void)
{
	test_one_available_alternative_is_sufficient();
	test_already_loaded_alternative_counts_as_ready();
	test_all_missing_alternatives_fail_the_group();
	test_unknown_probe_is_not_counted_as_available();
	test_invalid_probe_value_fails_closed();
	test_driver_specific_parameter_is_forwarded();
	test_invalid_group_fails_before_side_effects();
	test_modem_settings_snapshot_reads_complete_pair();
	test_modem_settings_first_read_error_publishes_no_values();
	test_modem_settings_second_read_error_discards_partial_pair();
	test_modem_settings_absent_keys_keep_legacy_defaults();
	test_modem_settings_reject_invalid_arguments();
	puts("USB modem module alternative readiness policy: PASS");
	return 0;
}
