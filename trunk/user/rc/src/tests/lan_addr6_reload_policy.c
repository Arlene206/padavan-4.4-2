#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "lan_addr6_reload_policy.h"

static int clear_status, add_status, clear_attempts, add_attempts, operation_order;
static const void *expected_context;

static void
reset_fake_operations(int clear_result, int add_result, const void *context)
{
	clear_status = clear_result;
	add_status = add_result;
	clear_attempts = 0;
	add_attempts = 0;
	operation_order = 0;
	expected_context = context;
}

static int
fake_clear_address(void)
{
	assert(operation_order == 0);
	operation_order = 1;
	clear_attempts++;
	return clear_status;
}

static int
fake_add_address(void *context)
{
	assert(operation_order == 1);
	assert(context == expected_context);
	operation_order = 2;
	add_attempts++;
	return add_status;
}


static void
test_static_mode_policy_preserves_service_type_overrides(void)
{
	int is_static = -1;

	assert(rc_lan_addr6_static_mode_status(
		1, 0, 0, EIO, NULL, &is_static) == 0);
	assert(is_static == 0);
	assert(rc_lan_addr6_static_mode_status(
		0, 1, 0, EIO, NULL, &is_static) == 0);
	assert(is_static == 0);
	assert(rc_lan_addr6_static_mode_status(
		0, 0, 1, EIO, NULL, &is_static) == 0);
	assert(is_static == 1);
}

static void
test_dynamic_static_mode_read_failure_is_not_a_default(void)
{
	int is_static = -1;

	assert(rc_lan_addr6_static_mode_status(
		0, 0, 0, EACCES, "0", &is_static) == EACCES);
	assert(is_static == 0);
}

static void
test_dynamic_static_mode_preserves_nvram_match_defaults(void)
{
	int is_static = -1;

	assert(rc_lan_addr6_static_mode_status(
		0, 0, 0, 0, NULL, &is_static) == EIO);
	assert(rc_lan_addr6_static_mode_status(
		0, 0, 0, 0, "", &is_static) == 0);
	assert(is_static == 0);
	assert(rc_lan_addr6_static_mode_status(
		0, 0, 0, 0, "1", &is_static) == 0);
	assert(is_static == 0);
	assert(rc_lan_addr6_static_mode_status(
		0, 0, 0, 0, "0", &is_static) == 0);
	assert(is_static == 1);
}


static void
test_legacy_integer_parser_preserves_nvram_get_int_defaults(void)
{
	int value = -99;

	assert(rc_lan_addr6_legacy_integer_status(0, NULL, &value) == 0);
	assert(value == 0);
	assert(rc_lan_addr6_legacy_integer_status(0, "", &value) == 0);
	assert(value == 0);
	assert(rc_lan_addr6_legacy_integer_status(0, "not-a-number", &value) == 0);
	assert(value == 0);
	assert(rc_lan_addr6_legacy_integer_status(0, "2trailing", &value) == 0);
	assert(value == 2);
	assert(rc_lan_addr6_legacy_integer_status(0, "-1", &value) == 0);
	assert(value == -1);
	assert(rc_lan_addr6_legacy_integer_status(
		0, "999999999999999999999", &value) == 0);
	assert(value == 0);
}

static void
test_legacy_integer_backend_failure_is_not_zero_default(void)
{
	int value = 73;

	assert(rc_lan_addr6_legacy_integer_status(EIO, "1", &value) == EIO);
	assert(value == 73);
}

static void
test_prefix_size_preserves_atoi_legacy_defaults(void)
{
	int prefix_size = -1;

	assert(rc_lan_addr6_legacy_prefix_size(0, NULL, &prefix_size) == 0);
	assert(prefix_size == 64);
	assert(rc_lan_addr6_legacy_prefix_size(0, "", &prefix_size) == 0);
	assert(prefix_size == 64);
	assert(rc_lan_addr6_legacy_prefix_size(0, "not-a-number", &prefix_size) == 0);
	assert(prefix_size == 64);
	assert(rc_lan_addr6_legacy_prefix_size(0, "52trailing", &prefix_size) == 0);
	assert(prefix_size == 52);
	assert(rc_lan_addr6_legacy_prefix_size(0, "48", &prefix_size) == 0);
	assert(prefix_size == 48);
	assert(rc_lan_addr6_legacy_prefix_size(0, "64", &prefix_size) == 0);
	assert(prefix_size == 64);
	assert(rc_lan_addr6_legacy_prefix_size(0, "47", &prefix_size) == 0);
	assert(prefix_size == 64);
	assert(rc_lan_addr6_legacy_prefix_size(0, "65", &prefix_size) == 0);
	assert(prefix_size == 64);
	assert(rc_lan_addr6_legacy_prefix_size(0, "999999999999999999999", &prefix_size) == 0);
	assert(prefix_size == 64);
}

static void
test_prefix_size_backend_error_is_not_a_legacy_default(void)
{
	int prefix_size = 60;

	assert(rc_lan_addr6_legacy_prefix_size(EIO, "48", &prefix_size) == EIO);
	assert(prefix_size == 60);
}


static void
test_radv_read_failure_is_not_enabled_by_default(void)
{
	int enabled = 1;

	assert(rc_lan_addr6_radv_enabled_status(EIO, "0", &enabled) == EIO);
	assert(enabled == 0);
}

static void
test_radv_missing_key_preserves_nvram_invmatch_default(void)
{
	int enabled = 0;

	assert(rc_lan_addr6_radv_enabled_status(0, "", &enabled) == 0);
	assert(enabled == 1);
}

static void
test_radv_literal_zero_disables_and_other_values_enable(void)
{
	int enabled = -1;

	assert(rc_lan_addr6_radv_enabled_status(0, "0", &enabled) == 0);
	assert(enabled == 0);
	assert(rc_lan_addr6_radv_enabled_status(0, "1", &enabled) == 0);
	assert(enabled == 1);
	assert(rc_lan_addr6_radv_enabled_status(0, "false", &enabled) == 0);
	assert(enabled == 1);
}

static void
test_radv_policy_rejects_missing_output_or_value(void)
{
	int enabled = 1;

	assert(rc_lan_addr6_radv_enabled_status(0, "0", NULL) == EINVAL);
	assert(rc_lan_addr6_radv_enabled_status(0, NULL, &enabled) == EIO);
	assert(enabled == 0);
}

static void
test_clear_failure_prevents_new_address_publication(void)
{
	const char address[] = "2001:db8::1/64";

	reset_fake_operations(EIO, 0, address);
	assert(rc_lan_addr6_reload_apply(1, fake_clear_address, fake_add_address,
					 (void *)address) == EIO);
	assert(clear_attempts == 1 && add_attempts == 0 && operation_order == 1);
}

static void
test_add_failure_is_returned_after_successful_clear(void)
{
	const char address[] = "2001:db8::2/64";

	reset_fake_operations(0, EADDRINUSE, address);
	assert(rc_lan_addr6_reload_apply(1, fake_clear_address, fake_add_address,
					 (void *)address) == EADDRINUSE);
	assert(clear_attempts == 1 && add_attempts == 1 && operation_order == 2);
}

static void
test_empty_configuration_only_clears_existing_address(void)
{
	reset_fake_operations(0, EIO, NULL);
	assert(rc_lan_addr6_reload_apply(0, fake_clear_address, NULL, NULL) == 0);
	assert(clear_attempts == 1 && add_attempts == 0 && operation_order == 1);
}

static void
test_clear_failure_is_reported_even_without_configured_address(void)
{
	reset_fake_operations(ENODEV, 0, NULL);
	assert(rc_lan_addr6_reload_apply(0, fake_clear_address, NULL, NULL) == ENODEV);
	assert(clear_attempts == 1 && add_attempts == 0);
}

static void
test_missing_callbacks_fail_closed(void)
{
	const char address[] = "2001:db8::3/64";

	reset_fake_operations(0, 0, address);
	assert(rc_lan_addr6_reload_apply(1, NULL, fake_add_address,
					 (void *)address) == EINVAL);
	assert(clear_attempts == 0 && add_attempts == 0);
	assert(rc_lan_addr6_reload_apply(1, fake_clear_address, NULL,
					 (void *)address) == EINVAL);
	assert(clear_attempts == 0 && add_attempts == 0);
}

static void
test_ipv6_prefix_format_is_bounded_and_non_overlapping(void)
{
	char output[46];
	char short_output[8];
	const char longest_literal[] = "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff";

	assert(rc_lan_addr6_format_prefix("2001:db8::1", 64,
					  output, sizeof(output)) == 0);
	assert(strcmp(output, "2001:db8::1/64") == 0);
	assert(rc_lan_addr6_format_prefix(longest_literal, 128,
					  output, sizeof(output)) == 0);
	assert(strcmp(output,
		      "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff/128") == 0);

	errno = 0;
	assert(rc_lan_addr6_format_prefix(longest_literal, 128,
					  short_output, sizeof(short_output)) == -1);
	assert(errno == ENAMETOOLONG && short_output[0] == '\0');
	assert(rc_lan_addr6_format_prefix("2001:db8::1", -1,
					  output, sizeof(output)) == -1);
	assert(errno == EINVAL && output[0] == '\0');
	assert(rc_lan_addr6_format_prefix("2001:db8::1", 129,
					  output, sizeof(output)) == -1);
	assert(rc_lan_addr6_format_prefix(NULL, 64,
					  output, sizeof(output)) == -1);
	assert(rc_lan_addr6_format_prefix("2001:db8::1", 64,
					  output, 0) == -1);
}

static void
test_nvram_write_status_propagates_backend_errors(void)
{
	assert(rc_lan_addr6_nvram_write_status(0, EACCES) == 0);
	assert(rc_lan_addr6_nvram_write_status(-1, ENOSPC) == ENOSPC);
	assert(rc_lan_addr6_nvram_write_status(-1, 0) == EIO);
	assert(rc_lan_addr6_nvram_write_status(1, 0) == EIO);
}

int
main(void)
{


	test_radv_read_failure_is_not_enabled_by_default();
	test_radv_missing_key_preserves_nvram_invmatch_default();
	test_radv_literal_zero_disables_and_other_values_enable();
	test_radv_policy_rejects_missing_output_or_value();
	test_static_mode_policy_preserves_service_type_overrides();
	test_dynamic_static_mode_read_failure_is_not_a_default();
	test_dynamic_static_mode_preserves_nvram_match_defaults();

	test_legacy_integer_parser_preserves_nvram_get_int_defaults();
	test_legacy_integer_backend_failure_is_not_zero_default();
	test_prefix_size_preserves_atoi_legacy_defaults();
	test_prefix_size_backend_error_is_not_a_legacy_default();
	test_ipv6_prefix_format_is_bounded_and_non_overlapping();
	test_clear_failure_prevents_new_address_publication();
	test_add_failure_is_returned_after_successful_clear();
	test_empty_configuration_only_clears_existing_address();
	test_clear_failure_is_reported_even_without_configured_address();
	test_missing_callbacks_fail_closed();
	test_nvram_write_status_propagates_backend_errors();
	puts("LAN IPv6 address reload and NVRAM publication status regression: PASS");
	return 0;
}
