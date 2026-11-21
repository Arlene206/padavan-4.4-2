#include <errno.h>
#include <limits.h>
#include <stdio.h>

#include "../switch_cli_parse_policy.h"

#define CHECK(condition) do { \
	if (!(condition)) { \
		fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
		return 1; \
	} \
} while (0)

static int
test_valid_decimal_and_hex_values(void)
{
	unsigned int value = 99;

	CHECK(switch_cli_parse_uint("0", 0, &value) == 0 && value == 0);
	CHECK(switch_cli_parse_uint("010", 0, &value) == 0 && value == 10);
	CHECK(switch_cli_parse_uint("42", 0, &value) == 0 && value == 42);
	CHECK(switch_cli_parse_uint("0x2a", 1, &value) == 0 && value == 42);
	CHECK(switch_cli_parse_uint("0XFF", 1, &value) == 0 && value == 255);
	CHECK(switch_cli_parse_uint("0xffffffff", 1, &value) == 0 &&
	      value == UINT_MAX);
	CHECK(switch_cli_parse_uint("4294967295", 0, &value) == 0 &&
	      value == UINT_MAX);
	return 0;
}

static int
test_malformed_values_are_rejected_without_output_changes(void)
{
	static const char *const invalid[] = {
		"", " ", "\t1", " 1", "1 ", "+1", "-1",
		"10junk", "0x", "0xg", "0x1z", "1.0"
	};
	unsigned int value;
	unsigned int i;

	for (i = 0; i < sizeof(invalid) / sizeof(invalid[0]); ++i) {
		value = 77;
		CHECK(switch_cli_parse_uint(invalid[i], 1, &value) == EINVAL);
		CHECK(value == 77);
	}
	value = 88;
	CHECK(switch_cli_parse_uint(NULL, 1, &value) == EINVAL);
	CHECK(value == 88);
	CHECK(switch_cli_parse_uint("0x10", 0, &value) == EINVAL);
	CHECK(value == 88);
	CHECK(switch_cli_parse_uint("10", 0, NULL) == EINVAL);
	return 0;
}

static int
test_overflow_is_rejected_without_output_changes(void)
{
	unsigned int value = 123;

	CHECK(switch_cli_parse_uint("4294967296", 0, &value) == ERANGE);
	CHECK(value == 123);
	CHECK(switch_cli_parse_uint("0x100000000", 1, &value) == ERANGE);
	CHECK(value == 123);
	return 0;
}

int
main(void)
{
	CHECK(test_valid_decimal_and_hex_values() == 0);
	CHECK(test_malformed_values_are_rejected_without_output_changes() == 0);
	CHECK(test_overflow_is_rejected_without_output_changes() == 0);
	puts("RC switch CLI strict unsigned parsing: PASS");
	return 0;
}
