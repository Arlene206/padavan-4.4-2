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

static void
test_mac_parser(void)
{
    unsigned int high = 0, low = 0;
    assert(switch_parse_mac_address("001122aAbBcC", &high, &low) == 0);
    assert(high == 0x001122aaU && low == 0xbbccU);
    assert(switch_parse_mac_address("FFFFFFFFFFFF", &high, &low) == 0);
    assert(high == 0xffffffffU && low == 0xffffU);

    high = 0x12345678U;
    low = 0x87654321U;
    assert(switch_parse_mac_address(0, &high, &low) != 0);
    assert(switch_parse_mac_address("", &high, &low) != 0);
    assert(switch_parse_mac_address("0011223344556", &high, &low) != 0);
    assert(switch_parse_mac_address("00112233445", &high, &low) != 0);
    assert(switch_parse_mac_address("00112233445g", &high, &low) != 0);
    assert(switch_parse_mac_address("001122-34455", &high, &low) != 0);
    assert(high == 0x12345678U && low == 0x87654321U);
    assert(switch_parse_mac_address("001122334455", 0, 0) != 0);
}

static void
test_uint_range_parser(void)
{
    unsigned int value = 0x12345678U;
    assert(switch_parse_uint_range("0", 0U, 4095U, &value) == 0 && value == 0U);
    assert(switch_parse_uint_range("4095", 0U, 4095U, &value) == 0 && value == 4095U);
    assert(switch_parse_uint_range("0xfff", 0U, 4095U, &value) == 0 && value == 4095U);
    assert(switch_parse_uint_range("0377", 1U, 255U, &value) == 0 && value == 255U);
    assert(switch_parse_uint_range("1", 1U, 255U, &value) == 0 && value == 1U);
    assert(switch_parse_uint_range(0, 0U, 4095U, &value) != 0);
    assert(switch_parse_uint_range("", 0U, 4095U, &value) != 0);
    assert(switch_parse_uint_range("-1", 0U, 4095U, &value) != 0);
    assert(switch_parse_uint_range("+1", 0U, 4095U, &value) != 0);
    assert(switch_parse_uint_range("4096", 0U, 4095U, &value) != 0);
    assert(switch_parse_uint_range("256", 1U, 255U, &value) != 0);
    assert(switch_parse_uint_range("8", 0U, 7U, &value) != 0);
    assert(switch_parse_uint_range("09", 0U, 255U, &value) != 0);
    assert(switch_parse_uint_range("1junk", 0U, 255U, &value) != 0);
    assert(switch_parse_uint_range(" 1", 0U, 255U, &value) != 0);
    assert(switch_parse_uint_range("999999999999999999999999999999", 0U, 4095U, &value) != 0);
}

static void
test_table_portmap_parser(void)
{
    unsigned int map = 0x12345678U;
    assert(switch_parse_table_portmap("1111111", &map) == 0 && map == 0x7fU);
    assert(switch_parse_table_portmap("0000000", &map) == 0 && map == 0U);
    assert(switch_parse_table_portmap("11111110", &map) == 0 && map == 0x7fU);
    assert(switch_parse_table_portmap("11111111", &map) == 0 && map == 0x7fU);
    assert(switch_parse_table_portmap("1111111x", &map) != 0 && map == 0x7fU);
    assert(switch_parse_table_portmap("111111", &map) != 0 && map == 0x7fU);
    assert(switch_parse_table_portmap("111111111", &map) != 0 && map == 0x7fU);
}

int
main(void)
{
	test_valid_ipv4_addresses();
	test_invalid_ipv4_addresses();
	test_null_output_is_rejected();
	test_valid_portmaps();
	test_invalid_portmaps();
    test_mac_parser();
    test_uint_range_parser();
    test_table_portmap_parser();
    puts("switch IPv4/MAC/numeric/port-map parser policy: PASS");
	return 0;
}
