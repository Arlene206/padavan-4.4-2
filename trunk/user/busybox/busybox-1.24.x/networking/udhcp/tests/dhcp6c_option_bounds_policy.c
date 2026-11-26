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
	/* RFC 5908 FQDN is one uncompressed ASCII hostname, not a domain list. */
	{
		static const unsigned char valid_fqdn[] = {
			4, 'n', 't', 'p', '1', 7, 'e', 'x', 'a', 'm', 'p', 'l', 'e',
			3, 'c', 'o', 'm', 0
		};
		static const unsigned char valid_single_label[] = { 3, 'n', 't', 'p', 0 };
		static const unsigned char multiple_names[] = { 3, 'n', 't', 'p', 0, 3, 'n', 't', 'p', 0 };
		static const unsigned char compressed[] = { 3, 'n', 't', 'p', 0xc0, 0x0c };
		static const unsigned char unterminated[] = { 3, 'n', 't', 'p' };
		static const unsigned char non_ascii[] = { 3, 'n', 0x80, 't', 0 };
		static const unsigned char idna_alabel[] = { 6, 'x', 'n', '-', '-', 'a', 'b', 0 };
		static const unsigned char bad_hyphen[] = { 3, '-', 't', 'p', 0 };
		static const unsigned char overlong_label[] = { 64, 'a', 'b', 'c' };
		static const unsigned char too_long[256] = { 1, 'a', 0 };

		CHECK(dhcp6c_ntp_fqdn_wire_valid(valid_fqdn, sizeof(valid_fqdn)));
		CHECK(dhcp6c_ntp_fqdn_wire_valid(valid_single_label, sizeof(valid_single_label)));
		CHECK(!dhcp6c_ntp_fqdn_wire_valid(multiple_names, sizeof(multiple_names)));
		CHECK(!dhcp6c_ntp_fqdn_wire_valid(compressed, sizeof(compressed)));
		CHECK(!dhcp6c_ntp_fqdn_wire_valid(unterminated, sizeof(unterminated)));
		CHECK(!dhcp6c_ntp_fqdn_wire_valid(non_ascii, sizeof(non_ascii)));
		CHECK(!dhcp6c_ntp_fqdn_wire_valid(idna_alabel, sizeof(idna_alabel)));
		CHECK(!dhcp6c_ntp_fqdn_wire_valid(bad_hyphen, sizeof(bad_hyphen)));
		CHECK(!dhcp6c_ntp_fqdn_wire_valid(overlong_label, sizeof(overlong_label)));
		CHECK(!dhcp6c_ntp_fqdn_wire_valid(too_long, sizeof(too_long)));
		CHECK(!dhcp6c_ntp_fqdn_wire_valid(NULL, 0));
	}

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

		CHECK(!dhcp6c_ntp_time_source_present(source_seen));
		CHECK(dhcp6c_ntp_time_source_claim(99U, &source_seen));
		CHECK(dhcp6c_ntp_time_source_present(source_seen));
		CHECK(!dhcp6c_ntp_time_source_claim(1U, &source_seen));
	}
	{
		int source_seen = 0;

		CHECK(dhcp6c_ntp_time_source_claim(1U, &source_seen));
		CHECK(dhcp6c_ntp_time_source_present(source_seen));
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
