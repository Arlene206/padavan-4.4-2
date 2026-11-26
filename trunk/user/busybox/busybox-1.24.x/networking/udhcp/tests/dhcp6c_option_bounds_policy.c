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
	/* DHCP6 Reconfigure Accept has a mandatory zero-length payload. */
	CHECK(dhcp6c_option_zero_payload_length_valid(0U));
	CHECK(!dhcp6c_option_zero_payload_length_valid(1U));
	CHECK(!dhcp6c_option_zero_payload_length_valid((size_t)-1));
	/* RFC 5908 NTP Server option suboption framing and time-source uniqueness. */
	CHECK(dhcp6c_ntp_suboption_wire_length_valid(20U, 16U));
	CHECK(dhcp6c_ntp_suboption_wire_length_valid(5U, 1U));
	CHECK(!dhcp6c_ntp_suboption_wire_length_valid(3U, 0U));
	CHECK(!dhcp6c_ntp_suboption_wire_length_valid(19U, 16U));
	CHECK(!dhcp6c_ntp_suboption_wire_length_valid(4U, 1U));
	CHECK(dhcp6c_ntp_suboption_payload_length_valid(1U, 16U));
	CHECK(dhcp6c_ntp_suboption_payload_length_valid(2U, 16U));
	CHECK(!dhcp6c_ntp_suboption_payload_length_valid(1U, 15U));
	CHECK(!dhcp6c_ntp_suboption_payload_length_valid(2U, 17U));
	CHECK(dhcp6c_ntp_address_kind_valid(1U, 0));
	CHECK(!dhcp6c_ntp_address_kind_valid(1U, 1));
	CHECK(dhcp6c_ntp_address_kind_valid(2U, 1));
	CHECK(!dhcp6c_ntp_address_kind_valid(2U, 0));
	CHECK(dhcp6c_ntp_address_kind_valid(99U, 1));
	CHECK(dhcp6c_ntp_suboption_payload_length_valid(3U, 1U));
	CHECK(!dhcp6c_ntp_suboption_payload_length_valid(3U, 0U));
	CHECK(dhcp6c_ntp_suboption_payload_length_valid(99U, 0U));
	{
		int source_seen = 0;

		CHECK(dhcp6c_ntp_time_source_claim(99U, &source_seen));
		CHECK(!source_seen);
		CHECK(dhcp6c_ntp_time_source_claim(1U, &source_seen));
		CHECK(source_seen);
		CHECK(!dhcp6c_ntp_time_source_claim(3U, &source_seen));
	}
	/* Singleton option buffer policy: reject a second owned vbuf. */
	{
		char existing[1] = {0};

		CHECK(dhcp6c_option_vbuf_is_unset(NULL, 0U));
		CHECK(!dhcp6c_option_vbuf_is_unset(existing, 0U));
		CHECK(!dhcp6c_option_vbuf_is_unset(existing, sizeof(existing)));
		CHECK(!dhcp6c_option_vbuf_is_unset(NULL, sizeof(existing)));
	}
	puts("DHCPv6 option bounds and NTP suboption policy: PASS");
	return 0;
}
