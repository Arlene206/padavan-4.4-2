#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "dhcp_lease_policy.h"
#include "dnsmasq_domain_policy.h"

int
main(void)
{
	char first[INET_ADDRSTRLEN];
	unsigned int number = 0;

	assert(dhcp_lease_ipv4_is_valid("192.0.2.1"));
	assert(!dhcp_lease_ipv4_is_valid("0.0.0.0"));
	assert(!dhcp_lease_ipv4_list_is_valid("0.0.0.0 192.0.2.1", 255, 16, 1));
	assert(dhcp_lease_route_destination_is_valid("0.0.0.0", 0));
	assert(!dhcp_lease_route_destination_is_valid("192.0.2.1", 0));
	assert(!dhcp_lease_route_destination_is_valid("0.0.0.0", 24));
	assert(dhcp_lease_route_destination_is_valid("192.0.2.0", 24));
	assert(!dhcp_lease_route_destination_is_valid("192.0.2.0", 33));
	assert(!dhcp_lease_ipv4_is_valid("192.0.2.1;reboot"));
	assert(!dhcp_lease_ipv4_is_valid("192.0.2.1\n"));
	assert(!dhcp_lease_ipv4_is_valid("999.0.2.1"));
	assert(dhcp_lease_netmask_is_valid("255.255.255.0"));
	assert(dhcp_lease_netmask_is_valid("255.255.255.255"));
	assert(!dhcp_lease_netmask_is_valid("255.0.255.0"));
	assert(!dhcp_lease_netmask_is_valid("0.0.0.0"));

	assert(dnsmasq_valid_server_domain("example.com"));
	assert(dnsmasq_valid_server_domain("router.lan."));
	assert(!dnsmasq_valid_server_domain(""));
	assert(!dnsmasq_valid_server_domain("example.com\n127.0.0.1 injected"));
	assert(!dnsmasq_valid_server_domain("example.com/8.8.8.8"));
	assert(!dnsmasq_valid_server_domain("-bad.example"));
	assert(!dnsmasq_valid_server_domain("bad-.example"));

	assert(dhcp_lease_ipv4_list_is_valid("192.0.2.1 198.51.100.2", 255, 16, 0));
	assert(dhcp_lease_ipv4_list_is_valid("", 255, 16, 1));
	assert(dhcp_lease_ipv4_list_is_valid(NULL, 255, 16, 1));
	assert(!dhcp_lease_ipv4_list_is_valid("192.0.2.1 198.51.100.2;reboot", 255, 16, 1));
	assert(!dhcp_lease_ipv4_list_is_valid("192.0.2.1\r198.51.100.2", 255, 16, 1));
	assert(!dhcp_lease_ipv4_list_is_valid("192.0.2.1 999.1.1.1", 255, 16, 1));
	assert(!dhcp_lease_ipv4_list_is_valid("192.0.2.1", 3, 16, 1));
	assert(dhcp_lease_ipv4_list_first(" 192.0.2.1 198.51.100.2", first, sizeof(first)) == 1);
	assert(strcmp(first, "192.0.2.1") == 0);
	assert(dhcp_lease_ipv4_list_first("", first, sizeof(first)) == 0);
	assert(dhcp_lease_ipv4_list_first("192.0.2.1,198.51.100.2", first, sizeof(first)) == -1);

	assert(!dhcp_lease_router_is_changed("192.0.2.1",
		"192.0.2.1 198.51.100.2", first, sizeof(first)));
	assert(dhcp_lease_router_is_changed("192.0.2.1", NULL,
		first, sizeof(first)));
	assert(first[0] == '\0');
	assert(!dhcp_lease_router_is_changed("", NULL, first, sizeof(first)));
	assert(dhcp_lease_router_is_changed("192.0.2.1", "",
		first, sizeof(first)));
	assert(first[0] == '\0');
	assert(dhcp_lease_router_is_changed("192.0.2.1", "198.51.100.1",
		first, sizeof(first)));
	assert(strcmp(first, "198.51.100.1") == 0);
	assert(dhcp_lease_router_is_changed("192.0.2.1", "invalid",
		first, sizeof(first)));
	assert(first[0] == '\0');

	assert(dhcp_lease_decimal_is_valid("1500", 576, 1500, &number) && number == 1500);
	assert(dhcp_lease_decimal_is_valid("0", 0, INT_MAX, &number) && number == 0);
	assert(!dhcp_lease_decimal_is_valid("-1", 0, INT_MAX, &number));
	assert(!dhcp_lease_decimal_is_valid("1501", 576, 1500, &number));
	assert(!dhcp_lease_decimal_is_valid("12x", 0, INT_MAX, &number));
	assert(!dhcp_lease_decimal_is_valid("999999999999999999999", 0, INT_MAX, &number));

	assert(dhcp_lease_route_text_is_safe(""));
	assert(dhcp_lease_route_text_is_safe("192.0.2.0/24 198.51.100.1 0.0.0.0/0 198.51.100.1"));
	assert(!dhcp_lease_route_text_is_safe("192.0.2.0/24;reboot"));
	assert(!dhcp_lease_route_text_is_safe("192.0.2.0/24\nCOMMIT"));
	assert(!dhcp_lease_route_text_is_safe("192.0.2.0/24 198.51.100.1 extra"));

	puts("DHCP lease input policy regression: OK");
	return 0;
}
