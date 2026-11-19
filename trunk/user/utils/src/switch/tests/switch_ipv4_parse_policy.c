#include <assert.h>
#include <stdio.h>

#include "../switch_ipv4_parse_policy.h"

static void
assert_valid(const char *text, unsigned int expected)
{
	unsigned int parsed = 0;

	assert(switch_parse_ipv4_address(text, &parsed) == 0);
	assert(parsed == expected);
}

static void
assert_invalid(const char *text)
{
	unsigned int parsed = 0x12345678U;

	assert(switch_parse_ipv4_address(text, &parsed) != 0);
	assert(parsed == 0x12345678U);
}

static void
test_valid_ipv4_addresses(void)
{
	assert_valid("192.168.1.1", 0xc0a80101U);
	assert_valid("0.0.0.0", 0x00000000U);
	assert_valid("255.255.255.255", 0xffffffffU);
	/* Preserve the legacy decimal interpretation of leading zeroes. */
	assert_valid("001.002.003.004", 0x01020304U);
}

static void
test_invalid_ipv4_addresses(void)
{
	assert_invalid(0);
	assert_invalid("");
	assert_invalid("192.168.1");
	assert_invalid("192.168.1.1.5");
	assert_invalid(".1.2.3");
	assert_invalid("1..2.3");
	assert_invalid("1.2.3.");
	assert_invalid("256.1.1.1");
	assert_invalid("1.256.1.1");
	assert_invalid("1.1.256.1");
	assert_invalid("1.1.1.256");
	assert_invalid("-1.2.3.4");
	assert_invalid("+1.2.3.4");
	assert_invalid("1.2.3.4junk");
	assert_invalid(" 1.2.3.4");
	assert_invalid("1.2.3.4 ");
	assert_invalid("999999999999999999999999999999.1.2.3");
}

static void
test_null_output_is_rejected(void)
{
	assert(switch_parse_ipv4_address("1.2.3.4", 0) != 0);
}

static void
test_valid_portmaps(void)
{
	unsigned int portmap = 0xffffffffU;

	assert(switch_parse_portmap("00000000", &portmap) == 0);
	assert(portmap == 0U);
	assert(switch_parse_portmap("11111110", &portmap) == 0);
	assert(portmap == 0x7fU);
	/* The eighth bit is reserved by the current seven-port table encoding. */
	assert(switch_parse_portmap("11111111", &portmap) == 0);
	assert(portmap == 0x7fU);
}

static void
assert_invalid_portmap(const char *text)
{
	unsigned int portmap = 0x12345678U;

	assert(switch_parse_portmap(text, &portmap) != 0);
	assert(portmap == 0x12345678U);
}

static void
test_invalid_portmaps(void)
{
	assert_invalid_portmap(0);
	assert_invalid_portmap("");
	assert_invalid_portmap("1111111");
	assert_invalid_portmap("111111111");
	assert_invalid_portmap("1111111x");
	assert_invalid_portmap("x1111110");
	assert_invalid_portmap("00010 00");
	assert(switch_parse_portmap("00000000", 0) != 0);
}

int
main(void)
{
	test_valid_ipv4_addresses();
	test_invalid_ipv4_addresses();
	test_null_output_is_rejected();
	test_valid_portmaps();
	test_invalid_portmaps();
	puts("switch IPv4 and port-map parser policy: PASS");
	return 0;
}
