#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "storage_time_file_policy.h"

static void
test_decimal_epoch_is_parsed_exactly(void)
{
	time_t value = (time_t)0;

	assert(rc_storage_time_parse_status("1790000000", 10U, &value) == 0);
	assert(value == (time_t)1790000000UL);
	assert(rc_storage_time_parse_status("1790000000\n", 11U, &value) == 0);
	assert(value == (time_t)1790000000UL);
}

static void
test_empty_zero_and_non_decimal_values_are_rejected(void)
{
	static const char *const invalid[] = {
		"", "0", "+1790000000", "-1790000000", " 1790000000",
		"1790000000 ", "0x6aa7e900", "1790000000junk", "17\n90000000"
	};
	size_t i;
	time_t value = (time_t)77;

	for (i = 0; i < sizeof(invalid) / sizeof(invalid[0]); i++)
		assert(rc_storage_time_parse_status(
			invalid[i], strlen(invalid[i]), &value) == EINVAL);
	assert(value == (time_t)77);
}

static void
test_overflow_and_invalid_arguments_fail_closed(void)
{
	char too_large[32];
	time_t value = (time_t)77;
	unsigned long overflow = (unsigned long)LONG_MAX + 1UL;

	assert(snprintf(too_large, sizeof(too_large), "%lu", overflow) > 0);
	assert(rc_storage_time_parse_status(
		too_large, strlen(too_large), &value) == EOVERFLOW);
	assert(value == (time_t)77);
	assert(rc_storage_time_parse_status(NULL, 1U, &value) == EINVAL);
	assert(rc_storage_time_parse_status("1", 1U, NULL) == EINVAL);
}

int
main(void)
{
	test_decimal_epoch_is_parsed_exactly();
	test_empty_zero_and_non_decimal_values_are_rejected();
	test_overflow_and_invalid_arguments_fail_closed();
	puts("RC persisted system-time strict decimal parser: PASS");
	return 0;
}
