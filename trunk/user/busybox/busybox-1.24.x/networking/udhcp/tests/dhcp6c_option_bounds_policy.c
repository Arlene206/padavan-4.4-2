#include <stdio.h>
#include "dhcp6c_option_bounds_policy.h"

#define CHECK(condition) do { \
	if (!(condition)) { \
		fprintf(stderr, "%s:%d: check failed: %s\n", __FILE__, __LINE__, #condition); \
		return 1; \
	} \
} while (0)

int main(void)
{
	CHECK(dhcp6c_option_wire_length_valid(4U, 4U, 0U));
	CHECK(dhcp6c_option_wire_length_valid(9U, 4U, 5U));
	CHECK(dhcp6c_option_wire_length_valid(65539U, 4U, 65535U));
	CHECK(!dhcp6c_option_wire_length_valid(3U, 4U, 0U));
	CHECK(!dhcp6c_option_wire_length_valid(8U, 4U, 5U));
	CHECK(!dhcp6c_option_wire_length_valid(4U, 4U, 1U));
	CHECK(!dhcp6c_option_wire_length_valid(0U, 0U, 1U));
	CHECK(!dhcp6c_option_wire_length_valid((size_t)-1,
		(size_t)-1, 1U));
	CHECK(dhcp6c_option_wire_length_valid((size_t)-1, 0U,
		(size_t)-1));
	CHECK(dhcp6c_datagram_length_fits(8192U, 8192U));
	CHECK(!dhcp6c_datagram_length_fits(8193U, 8192U));
	CHECK(!dhcp6c_datagram_length_fits((size_t)-1, 8192U));
	puts("DHCPv6 option and UDP datagram bounds policy: PASS");
	return 0;
}
