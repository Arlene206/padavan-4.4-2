#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>

#include "../uqmi-client-id-policy.h"

static void
expect_valid(const char *text, int expected)
{
	int value = -1;
	assert(uqmi_parse_client_id(text, &value) == 0);
	assert(value == expected);
}

static void
expect_invalid(const char *text)
{
	int value = 77;
	assert(uqmi_parse_client_id(text, &value) == EINVAL);
	assert(value == 77);
}

static void
test_client_id_parser_accepts_valid_8_bit_values(void)
{
	expect_valid("0", 0);
	expect_valid("255", UCHAR_MAX);
	expect_valid("0377", UCHAR_MAX);
	expect_valid("0xff", UCHAR_MAX);
	expect_valid("  +42", 42);
}

static void
test_client_id_parser_rejects_invalid_numbers(void)
{
	expect_invalid(NULL);
	expect_invalid("");
	expect_invalid("   ");
	expect_invalid("-1");
	expect_invalid(" -0");
	expect_invalid("256");
	expect_invalid("0x100");
	expect_invalid("0x");
	expect_invalid("08");
	expect_invalid("7tail");
	expect_invalid("7 ");
	expect_invalid("18446744073709551616");
	expect_invalid("999999999999999999999999999999999999999999999999");
	assert(uqmi_parse_client_id("7", NULL) == EINVAL);
}

int
main(void)
{
	test_client_id_parser_accepts_valid_8_bit_values();
	test_client_id_parser_rejects_invalid_numbers();
	puts("uqmi strict 8-bit client-ID parser: PASS");
	return 0;
}
