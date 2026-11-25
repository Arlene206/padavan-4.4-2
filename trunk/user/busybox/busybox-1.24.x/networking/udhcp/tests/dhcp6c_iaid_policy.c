#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>

#include "dhcp6c_iaid_policy.h"

static void
expect_valid(const char *text, uint32_t expected)
{
	uint32_t actual = 0xdeadbeefU;

	errno = 0;
	assert(dhcp6c_iaid_parse(text, &actual) == 0);
	assert(actual == expected);
}

static void
expect_invalid(const char *text, int expected_errno)
{
	uint32_t actual = 0x12345678U;

	errno = 0;
	assert(dhcp6c_iaid_parse(text, &actual) == -1);
	assert(errno == expected_errno);
	assert(actual == 0x12345678U);
}

static void
test_decimal_iaid_boundaries(void)
{
	expect_valid("0", 0U);
	expect_valid("1", 1U);
	expect_valid("00042", 42U);
	expect_valid("4294967295", 0xffffffffU);
}

static void
test_malformed_iaid_is_rejected_without_partial_output(void)
{
	expect_invalid(NULL, EINVAL);
	expect_invalid("", EINVAL);
	expect_invalid("-1", EINVAL);
	expect_invalid("+1", EINVAL);
	expect_invalid(" 1", EINVAL);
	expect_invalid("1 ", EINVAL);
	expect_invalid("1x", EINVAL);
	expect_invalid("x1", EINVAL);
	expect_invalid("1.0", EINVAL);
	expect_invalid("4294967296", ERANGE);
	expect_invalid("999999999999999999999999999999999999", ERANGE);
}

static void
test_output_pointer_is_required(void)
{
	errno = 0;
	assert(dhcp6c_iaid_parse("1", NULL) == -1);
	assert(errno == EINVAL);
}

int
main(void)
{
	test_decimal_iaid_boundaries();
	test_malformed_iaid_is_rejected_without_partial_output();
	test_output_pointer_is_required();
	puts("DHCPv6 IAID strict decimal parsing policy: PASS");
	return 0;
}
