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

int
main(void)
{
	test_ipv6_prefix_format_is_bounded_and_non_overlapping();
	test_clear_failure_prevents_new_address_publication();
	test_add_failure_is_returned_after_successful_clear();
	test_empty_configuration_only_clears_existing_address();
	test_clear_failure_is_reported_even_without_configured_address();
	test_missing_callbacks_fail_closed();
	puts("LAN IPv6 address reload sequencing and result propagation: PASS");
	return 0;
}
