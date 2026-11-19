#include <assert.h>
#include <stdio.h>

#include "storage_hotplug_name_policy.h"

static void
test_fitting_names_are_accepted(void)
{
	assert(storage_hotplug_name_format_status(0, 32) == 0);
	assert(storage_hotplug_name_format_status(31, 32) == 0);
}

static void
test_truncation_is_rejected(void)
{
	assert(storage_hotplug_name_format_status(32, 32) == -1);
	assert(storage_hotplug_name_format_status(33, 32) == -1);
}

static void
test_snprintf_errors_are_rejected(void)
{
	assert(storage_hotplug_name_format_status(-1, 32) == -1);
}

static void
test_zero_capacity_is_rejected(void)
{
	assert(storage_hotplug_name_format_status(0, 0) == -1);
}

static void
test_decimal_tokens_require_complete_bounded_digits(void)
{
	unsigned int value = 77;

	assert(storage_hotplug_parse_decimal_token("0", 0, 2, &value) == 0);
	assert(value == 0);
	assert(storage_hotplug_parse_decimal_token("02", 0, 2, &value) == 0);
	assert(value == 2);
	assert(storage_hotplug_parse_decimal_token("1", 1, 2, &value) == 0);
	assert(value == 1);

	value = 77;
	assert(storage_hotplug_parse_decimal_token(NULL, 0, 2, &value) == -1);
	assert(value == 77);
	assert(storage_hotplug_parse_decimal_token("", 0, 2, &value) == -1);
	assert(storage_hotplug_parse_decimal_token("3", 0, 2, &value) == -1);
	assert(storage_hotplug_parse_decimal_token("-1", 0, 2, &value) == -1);
	assert(storage_hotplug_parse_decimal_token("+1", 0, 2, &value) == -1);
	assert(storage_hotplug_parse_decimal_token(" 1", 0, 2, &value) == -1);
	assert(storage_hotplug_parse_decimal_token("1 ", 0, 2, &value) == -1);
	assert(storage_hotplug_parse_decimal_token("1x", 0, 2, &value) == -1);
	assert(storage_hotplug_parse_decimal_token("4294967296", 0, 4294967295U, &value) == -1);
	assert(value == 77);
}

int
main(void)
{
	test_fitting_names_are_accepted();
	test_truncation_is_rejected();
	test_snprintf_errors_are_rejected();
	test_zero_capacity_is_rejected();
	test_decimal_tokens_require_complete_bounded_digits();

	puts("storage hotplug share-name formatting policy: PASS");
	return 0;
}
