#include <stddef.h>
#include <errno.h>
#include <assert.h>
#include "../firewall_filter_policy.h"

int
main(void)
{
	assert(firewall_ppp_interface_is_safe("ppp10", 10));
	assert(firewall_ppp_interface_is_safe("ppp59", 10));
	assert(firewall_ppp_interface_is_safe("ppp65535", 10));
	assert(!firewall_ppp_interface_is_safe(NULL, 10));
	assert(!firewall_ppp_interface_is_safe("ppp", 10));
	assert(!firewall_ppp_interface_is_safe("ppp9", 10));
	assert(!firewall_ppp_interface_is_safe("ppp10+", 10));
	assert(!firewall_ppp_interface_is_safe("ppp10x", 10));
	assert(!firewall_ppp_interface_is_safe("ppp010", 10));
	assert(!firewall_ppp_interface_is_safe("ppp00000000000010", 10));
	assert(!firewall_ppp_interface_is_safe("ppp10\n-A INPUT -j ACCEPT", 10));
	assert(!firewall_ppp_interface_is_safe("ppp65536", 10));
	assert(!firewall_ppp_interface_is_safe("ppp999999999999", 10));

	assert(firewall_ipv4_address_is_safe("192.168.1.1"));
	assert(firewall_ipv4_address_is_safe("0.0.0.0"));
	assert(!firewall_ipv4_address_is_safe(NULL));
	assert(!firewall_ipv4_address_is_safe(""));
	assert(!firewall_ipv4_address_is_safe("192.168.1.1\n-A INPUT -j ACCEPT"));
	assert(!firewall_ipv4_address_is_safe("192.168.1.1 -j ACCEPT"));
	assert(!firewall_ipv4_address_is_safe("192.168.1.999"));
	assert(!firewall_ipv4_address_is_safe("192.168.1"));
	assert(!firewall_ipv4_address_is_safe("192.168.1.1\rCOMMIT"));

	/* test_ipv4_netmask_contiguity: reject invalid and non-contiguous subnet masks. */
	assert(ipv4_netmask_is_contiguous("255.255.255.0"));
	assert(ipv4_netmask_is_contiguous("255.255.0.0"));
	assert(ipv4_netmask_is_contiguous("255.0.0.0"));
	assert(ipv4_netmask_is_contiguous("128.0.0.0"));
	assert(ipv4_netmask_is_contiguous("255.255.255.254"));
	assert(ipv4_netmask_is_contiguous("255.255.255.255"));
	assert(!ipv4_netmask_is_contiguous(NULL));
	assert(!ipv4_netmask_is_contiguous(""));
	assert(!ipv4_netmask_is_contiguous("0.0.0.0"));
	assert(!ipv4_netmask_is_contiguous("255.0.255.0"));
	assert(!ipv4_netmask_is_contiguous("255.255.255.1"));
	assert(!ipv4_netmask_is_contiguous("255.255.255.256"));
	assert(!ipv4_netmask_is_contiguous("255.255.255.0\nCOMMIT"));

	/* Interface strings are written as raw iptables-restore tokens. */
	assert(firewall_interface_name_is_safe("br0"));
	assert(firewall_interface_name_is_safe("eth2.2"));
	assert(firewall_interface_name_is_safe("ppp0"));
	assert(firewall_interface_name_is_safe("apcli0"));
	assert(firewall_interface_name_is_safe("wwan0"));
	assert(firewall_interface_name_is_safe("eth0:1"));
	assert(firewall_interface_name_is_safe("ppp10+"));
	assert(!firewall_interface_name_is_safe(NULL));
	assert(!firewall_interface_name_is_safe(""));
	assert(!firewall_interface_name_is_safe("-eth0"));
	assert(!firewall_interface_name_is_safe("+eth0"));
	assert(!firewall_interface_name_is_safe("eth0++"));
	assert(!firewall_interface_name_is_safe("eth0/wan"));
	assert(!firewall_interface_name_is_safe("eth0;COMMIT"));
	assert(!firewall_interface_name_is_safe("eth0 -j ACCEPT"));
	assert(!firewall_interface_name_is_safe("eth0\n-A INPUT -j ACCEPT"));
	assert(!firewall_interface_name_is_safe("1234567890123456"));

	assert(firewall_url_filter_value_is_safe("example.com"));
	assert(firewall_url_filter_value_is_safe("www.example.com/path?q=1&x=two"));
	assert(firewall_url_filter_value_is_safe("video-site.example/channel-1"));
	assert(firewall_url_filter_value_is_safe(""));
	assert(!firewall_url_filter_value_is_safe("evil\"\n-A INPUT -j ACCEPT\n#"));
	assert(!firewall_url_filter_value_is_safe("evil\\\n-A INPUT -j ACCEPT"));
	assert(!firewall_url_filter_value_is_safe("line\nfeed"));
	assert(!firewall_url_filter_value_is_safe("line\rreturn"));
	assert(!firewall_url_filter_value_is_safe("tab\tvalue"));
	assert(!firewall_url_filter_value_is_safe("delete\177char"));
	assert(!firewall_url_filter_value_is_safe(NULL));

	assert(firewall_icmp_type_is_safe("0"));
	assert(firewall_icmp_type_is_safe("8"));
	assert(firewall_icmp_type_is_safe("255"));
	assert(firewall_icmp_type_is_safe("0008"));
	assert(!firewall_icmp_type_is_safe(NULL));
	assert(!firewall_icmp_type_is_safe(""));
	assert(!firewall_icmp_type_is_safe("256"));
	assert(!firewall_icmp_type_is_safe("-1"));
	assert(!firewall_icmp_type_is_safe("8/1"));
	assert(!firewall_icmp_type_is_safe("8\nCOMMIT"));
	assert(!firewall_icmp_type_is_safe("8\r-A INPUT -j ACCEPT"));
	assert(!firewall_icmp_type_is_safe("8\t"));
	assert(!firewall_icmp_type_is_safe("8 -A INPUT -j ACCEPT"));

	assert(firewall_ip_protocol_number_is_safe("0"));
	assert(firewall_ip_protocol_number_is_safe("6"));
	assert(firewall_ip_protocol_number_is_safe("255"));
	assert(firewall_ip_protocol_number_is_safe("0006"));
	assert(!firewall_ip_protocol_number_is_safe(NULL));
	assert(!firewall_ip_protocol_number_is_safe(""));
	assert(!firewall_ip_protocol_number_is_safe("256"));
	assert(!firewall_ip_protocol_number_is_safe("-1"));
	assert(!firewall_ip_protocol_number_is_safe("6\nCOMMIT"));

	assert(firewall_port_number_is_safe("1"));
	assert(firewall_port_number_is_safe("65535"));
	assert(!firewall_port_number_is_safe(NULL));
	assert(!firewall_port_number_is_safe(""));
	assert(!firewall_port_number_is_safe("0"));
	assert(!firewall_port_number_is_safe("65536"));
	assert(!firewall_port_number_is_safe("80\nCOMMIT"));

	/* UDPXY ports must be valid non-privileged TCP/UDP port numbers. */
	assert(!firewall_unprivileged_port_is_safe(0));
	assert(!firewall_unprivileged_port_is_safe(1023));
	assert(firewall_unprivileged_port_is_safe(1024));
	assert(firewall_unprivileged_port_is_safe(65535));
	assert(!firewall_unprivileged_port_is_safe(65536));
	assert(!firewall_unprivileged_port_is_safe(-1));

	/* The virtual-server list is bounded by the WebUI's 64-entry limit. */
	assert(firewall_vts_entry_count_is_safe(0));
	assert(firewall_vts_entry_count_is_safe(64));
	assert(!firewall_vts_entry_count_is_safe(-1));
	assert(!firewall_vts_entry_count_is_safe(65));

	/* Lightweight Filter entries are bounded by the WebUI's 64-entry cap. */
	assert(firewall_lw_entry_count_is_safe(0));
	assert(firewall_lw_entry_count_is_safe(64));
	assert(!firewall_lw_entry_count_is_safe(-1));
	assert(!firewall_lw_entry_count_is_safe(65));

	/* The URL keyword UI caps entries at 128. */
	assert(firewall_url_entry_count_is_safe(0));
	assert(firewall_url_entry_count_is_safe(128));
	assert(!firewall_url_entry_count_is_safe(-1));
	assert(!firewall_url_entry_count_is_safe(129));

	assert(firewall_port_range_is_safe("80", 0));
	assert(firewall_port_range_is_safe("1:65535", 0));
	assert(firewall_port_range_is_safe("", 1));
	assert(!firewall_port_range_is_safe("", 0));
	assert(!firewall_port_range_is_safe(NULL, 1));
	assert(!firewall_port_range_is_safe("80:81:82", 0));
	assert(!firewall_port_range_is_safe("81:80", 0));
	assert(!firewall_port_range_is_safe("80:65536", 0));
	assert(!firewall_port_range_is_safe("80\n-A INPUT -j ACCEPT", 0));
	assert(!firewall_port_range_is_safe("80,81", 0));

	assert(firewall_tcp_flags_are_safe(""));
	assert(firewall_tcp_flags_are_safe("ALL"));
	assert(firewall_tcp_flags_are_safe("SYN"));
	assert(firewall_tcp_flags_are_safe("PSH"));
	assert(!firewall_tcp_flags_are_safe(NULL));
	assert(!firewall_tcp_flags_are_safe("SYN,ACK"));
	assert(!firewall_tcp_flags_are_safe("SYN\n-A INPUT -j ACCEPT"));

	assert(firewall_schedule_date_is_safe("1111111"));
	assert(firewall_schedule_date_is_safe("0101010"));
	assert(firewall_schedule_date_is_safe("0000000"));
	assert(!firewall_schedule_date_is_safe(NULL));
	assert(!firewall_schedule_date_is_safe("111111"));
	assert(!firewall_schedule_date_is_safe("111111x"));
	assert(!firewall_schedule_date_is_safe("111111\n"));

	assert(firewall_schedule_time_is_safe("00002359"));
	assert(firewall_schedule_time_is_safe("23592359"));
	assert(firewall_schedule_time_is_safe("08001730"));
	assert(!firewall_schedule_time_is_safe(NULL));
	assert(!firewall_schedule_time_is_safe("0800173"));
	assert(!firewall_schedule_time_is_safe("0800173x"));
	assert(!firewall_schedule_time_is_safe("24001730"));
	assert(!firewall_schedule_time_is_safe("08601730"));
	assert(!firewall_schedule_time_is_safe("08002400"));
	assert(!firewall_schedule_time_is_safe("08001760"));
	assert(!firewall_schedule_time_is_safe("0800\n-A INPUT"));

	assert(firewall_schedule_is_active("1111111", "00002359"));
	assert(firewall_schedule_is_active("0101010", "08001730"));
	assert(!firewall_schedule_is_active("0000000", "00002359"));
	assert(!firewall_schedule_is_active("0000000", "08001730"));
	assert(!firewall_schedule_is_active("111111x", "00002359"));
	assert(!firewall_schedule_is_active("1111111", "0800173x"));
	assert(!firewall_schedule_is_active(NULL, "00002359"));
	assert(!firewall_schedule_is_active("1111111", NULL));

	/* Missing schedule keys use legacy defaults; present empty keys do not. */
	assert(!strcmp(firewall_schedule_value_or_default(0, NULL, "1111111"), "1111111"));
	assert(!strcmp(firewall_schedule_value_or_default(0, NULL, "00002359"), "00002359"));
	assert(!strcmp(firewall_schedule_value_or_default(1, "", "1111111"), ""));
	assert(!strcmp(firewall_schedule_value_or_default(1, "0101010", "1111111"), "0101010"));

	/* Checked URL/LWF schedule reads preserve defaults but never mask I/O errors. */
	{
		char date_value[8], time_value[9], short_value[4];

		assert(firewall_schedule_copy_checked_value(
			0, 0, NULL, "1111111", date_value, sizeof(date_value)) == 0);
		assert(!strcmp(date_value, "1111111"));
		assert(firewall_schedule_copy_checked_value(
			0, 0, NULL, "00002359", time_value, sizeof(time_value)) == 0);
		assert(!strcmp(time_value, "00002359"));
		assert(firewall_schedule_copy_checked_value(
			0, 1, "", "1111111", date_value, sizeof(date_value)) == 0);
		assert(date_value[0] == '\0');
		assert(firewall_schedule_copy_checked_value(
			EIO, 0, NULL, "1111111", date_value, sizeof(date_value)) == EIO);
		assert(date_value[0] == '\0');
		assert(firewall_schedule_copy_checked_value(
			0, 1, "11111111", "1111111", short_value, sizeof(short_value)) == 0);
		assert(short_value[0] == '\0');
		assert(firewall_schedule_copy_checked_value(
			0, 0, NULL, "1111111", NULL, 0) == EINVAL);
	}

	/* The MAC filter UI exposes only disabled, allowlist and denylist modes. */
	assert(firewall_mac_filter_mode_is_safe(0));
	assert(firewall_mac_filter_mode_is_safe(1));
	assert(firewall_mac_filter_mode_is_safe(2));
	assert(!firewall_mac_filter_mode_is_safe(-1));
	assert(!firewall_mac_filter_mode_is_safe(3));

	/* MAC filter entries are bounded by the WebUI's 64-row limit. */
	assert(firewall_mac_filter_entry_count_is_safe(0));
	assert(firewall_mac_filter_entry_count_is_safe(64));
	assert(!firewall_mac_filter_entry_count_is_safe(-1));
	assert(!firewall_mac_filter_entry_count_is_safe(65));

	/* MAC values are validated before they become bare iptables tokens. */
	{
		char formatted[18];
		assert(firewall_mac_filter_address_format("A1B2C3D4E5F6",
							  formatted, sizeof(formatted)) == 1);
		assert(!strcmp(formatted, "A1:B2:C3:D4:E5:F6"));
		assert(firewall_mac_filter_address_format("a1b2c3d4e5f6",
							  formatted, sizeof(formatted)) == 1);
		assert(!strcmp(formatted, "a1:b2:c3:d4:e5:f6"));
		assert(firewall_mac_filter_address_format("", formatted,
							  sizeof(formatted)) == 0);
		assert(formatted[0] == '\0');
		assert(firewall_mac_filter_address_format("000000000000", formatted,
							  sizeof(formatted)) == 0);
		assert(firewall_mac_filter_address_format("ffffffffffff", formatted,
							  sizeof(formatted)) == 0);
		assert(firewall_mac_filter_address_format("A1B2C3D4E5G6", formatted,
							  sizeof(formatted)) < 0);
		assert(firewall_mac_filter_address_format("A1B2C3D4E5F", formatted,
							  sizeof(formatted)) < 0);
		assert(firewall_mac_filter_address_format("A1B2C3D4E5F6\nCOMMIT",
							  formatted, sizeof(formatted)) < 0);
		assert(firewall_mac_filter_address_format(NULL, formatted,
							  sizeof(formatted)) < 0);
		assert(firewall_mac_filter_address_format("A1B2C3D4E5F6",
							  formatted, sizeof(formatted) - 1) < 0);
	}

	/* Optional URL MAC: only confirmed absence may omit the qualifier. */
	{
		char formatted[24];

		memset(formatted, 'X', sizeof(formatted));
		assert(firewall_url_mac_format_status(0, 0, "A1B2C3D4E5F6",
						      formatted, sizeof(formatted)) == 0);
		assert(formatted[0] == '\0');
		assert(firewall_url_mac_format_status(0, 1, "",
						      formatted, sizeof(formatted)) == 0);
		assert(firewall_url_mac_format_status(0, 1, "000000000000",
						      formatted, sizeof(formatted)) == 0);
		assert(firewall_url_mac_format_status(0, 1, "FFFFFFFFFFFF",
						      formatted, sizeof(formatted)) == 0);
		assert(firewall_url_mac_format_status(0, 1, "A1B2C3D4E5F6",
						      formatted, sizeof(formatted)) == 1);
		assert(!strcmp(formatted, "A1:B2:C3:D4:E5:F6"));

		memset(formatted, 'X', sizeof(formatted));
		assert(firewall_url_mac_format_status(EIO, 1, "A1B2C3D4E5F6",
						      formatted, sizeof(formatted)) < 0);
		assert(formatted[0] == '\0');
		assert(firewall_url_mac_format_status(0, 1, "A1B2C3D4E5G6",
						      formatted, sizeof(formatted)) < 0);
		assert(formatted[0] == '\0');
		assert(firewall_url_mac_format_status(0, 1, NULL,
						      formatted, sizeof(formatted)) < 0);
		assert(firewall_url_mac_format_status(0, 1, "A1B2C3D4E5F6",
						      formatted, 17) < 0);
	}

	return 0;
}
