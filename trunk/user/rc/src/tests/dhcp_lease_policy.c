#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "dhcp_lease_policy.h"
#include "dnsmasq_domain_policy.h"


struct dhcp_bound_fake {
	int delete_old_status;
	int delete_new_status;
	int add_old_status;
	int add_new_status;
	int clear_status;
	int set_old_status;
	int set_new_status;
	int publish_status;
	char events[32];
	size_t event_count;
};

static void
dhcp_bound_fake_event(struct dhcp_bound_fake *fake, char event)
{
	if (fake->event_count + 1 < sizeof(fake->events)) {
		fake->events[fake->event_count++] = event;
		fake->events[fake->event_count] = '\0';
	}
}

static int
dhcp_bound_fake_delete(void *context, const char *gateway)
{
	struct dhcp_bound_fake *fake = (struct dhcp_bound_fake *)context;

	if (strcmp(gateway, "192.0.2.1") == 0) {
		dhcp_bound_fake_event(fake, 'd');
		return fake->delete_old_status;
	}
	dhcp_bound_fake_event(fake, 'D');
	return fake->delete_new_status;
}

static int
dhcp_bound_fake_clear(void *context)
{
	struct dhcp_bound_fake *fake = (struct dhcp_bound_fake *)context;

	dhcp_bound_fake_event(fake, 'C');
	return fake->clear_status;
}

static int
dhcp_bound_fake_set(void *context, const char *ipaddr, const char *netmask)
{
	struct dhcp_bound_fake *fake = (struct dhcp_bound_fake *)context;

	(void)netmask;
	if (strcmp(ipaddr, "192.0.2.10") == 0) {
		dhcp_bound_fake_event(fake, 's');
		return fake->set_old_status;
	}
	dhcp_bound_fake_event(fake, 'S');
	return fake->set_new_status;
}

static int
dhcp_bound_fake_add(void *context, const char *gateway)
{
	struct dhcp_bound_fake *fake = (struct dhcp_bound_fake *)context;

	if (strcmp(gateway, "192.0.2.1") == 0) {
		dhcp_bound_fake_event(fake, 'a');
		return fake->add_old_status;
	}
	dhcp_bound_fake_event(fake, 'A');
	return fake->add_new_status;
}

static int
dhcp_bound_fake_publish(void *context)
{
	struct dhcp_bound_fake *fake = (struct dhcp_bound_fake *)context;

	dhcp_bound_fake_event(fake, 'P');
	return fake->publish_status;
}

static const struct dhcp_lease_bound_ops dhcp_bound_fake_ops = {
	dhcp_bound_fake_delete,
	dhcp_bound_fake_clear,
	dhcp_bound_fake_set,
	dhcp_bound_fake_add,
	dhcp_bound_fake_publish
};

int
main(void)
{
	char first[INET_ADDRSTRLEN];
	unsigned int number = 0;
	struct dhcp_bound_fake fake;
	int rollback_status, transaction_status;

	assert(dhcp_lease_ipv4_is_valid("192.0.2.1"));
	assert(!dhcp_lease_ipv4_is_valid("0.0.0.0"));
	assert(!dhcp_lease_ipv4_list_is_valid("0.0.0.0 192.0.2.1", 255, 16, 1));
	assert(dhcp_lease_route_destination_is_valid("0.0.0.0", 0));
	assert(!dhcp_lease_route_destination_is_valid("192.0.2.1", 0));
	assert(!dhcp_lease_route_destination_is_valid("0.0.0.0", 24));
	assert(dhcp_lease_route_destination_is_valid("192.0.2.0", 24));
	assert(dhcp_lease_route_delete_is_ok(0));
	assert(dhcp_lease_route_delete_is_ok(ESRCH));
	assert(!dhcp_lease_route_delete_is_ok(EIO));
	assert(dhcp_lease_deconfig_status(0, 0, 0) == 0);
	assert(dhcp_lease_deconfig_status(ESRCH, 0, 0) == 0);
	assert(dhcp_lease_deconfig_status(EIO, 0, 0) == EIO);
	assert(dhcp_lease_deconfig_status(0, EIO, 0) == EIO);
	assert(dhcp_lease_deconfig_status(0, -1, 0) == -1);
	assert(dhcp_lease_deconfig_status(0, 0, ENOSPC) == ENOSPC);
	assert(dhcp_lease_deconfig_status(EIO, EIO, ENOSPC) == EIO);
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


	assert(dhcp_lease_route_add_is_ok(0));
	assert(dhcp_lease_route_add_is_ok(EEXIST));
	assert(!dhcp_lease_route_add_is_ok(EIO));

	memset(&fake, 0, sizeof(fake));
	transaction_status = dhcp_lease_bound_apply(
		"192.0.2.10", "255.255.255.0", "192.0.2.1",
		"198.51.100.20", "255.255.255.128", "198.51.100.1",
		1, 1, 1, &dhcp_bound_fake_ops, &fake, &rollback_status);
	assert(transaction_status == 0 && rollback_status == 0);
	assert(strcmp(fake.events, "dCSAP") == 0);

	memset(&fake, 0, sizeof(fake));
	fake.delete_old_status = EIO;
	transaction_status = dhcp_lease_bound_apply(
		"192.0.2.10", "255.255.255.0", "192.0.2.1",
		"198.51.100.20", "255.255.255.128", "198.51.100.1",
		1, 1, 1, &dhcp_bound_fake_ops, &fake, &rollback_status);
	assert(transaction_status == EIO && rollback_status == 0);
	assert(strcmp(fake.events, "d") == 0);

	memset(&fake, 0, sizeof(fake));
	fake.clear_status = EIO;
	transaction_status = dhcp_lease_bound_apply(
		"192.0.2.10", "255.255.255.0", "192.0.2.1",
		"198.51.100.20", "255.255.255.128", "198.51.100.1",
		1, 1, 1, &dhcp_bound_fake_ops, &fake, &rollback_status);
	assert(transaction_status == EIO && rollback_status == 0);
	assert(strcmp(fake.events, "dCsa") == 0);

	memset(&fake, 0, sizeof(fake));
	fake.set_new_status = EIO;
	transaction_status = dhcp_lease_bound_apply(
		"192.0.2.10", "255.255.255.0", "192.0.2.1",
		"198.51.100.20", "255.255.255.128", "198.51.100.1",
		1, 1, 1, &dhcp_bound_fake_ops, &fake, &rollback_status);
	assert(transaction_status == EIO && rollback_status == 0);
	assert(strcmp(fake.events, "dCSsa") == 0);

	memset(&fake, 0, sizeof(fake));
	fake.add_new_status = EIO;
	transaction_status = dhcp_lease_bound_apply(
		"192.0.2.10", "255.255.255.0", "192.0.2.1",
		"198.51.100.20", "255.255.255.128", "198.51.100.1",
		1, 1, 1, &dhcp_bound_fake_ops, &fake, &rollback_status);
	assert(transaction_status == EIO && rollback_status == 0);
	assert(strcmp(fake.events, "dCSAsa") == 0);

	memset(&fake, 0, sizeof(fake));
	fake.publish_status = ENOSPC;
	transaction_status = dhcp_lease_bound_apply(
		"192.0.2.10", "255.255.255.0", "192.0.2.1",
		"198.51.100.20", "255.255.255.128", "198.51.100.1",
		1, 1, 1, &dhcp_bound_fake_ops, &fake, &rollback_status);
	assert(transaction_status == ENOSPC && rollback_status == 0);
	assert(strcmp(fake.events, "dCSAPDsa") == 0);

	memset(&fake, 0, sizeof(fake));
	fake.add_new_status = EEXIST;
	fake.publish_status = ENOSPC;
	transaction_status = dhcp_lease_bound_apply(
		"192.0.2.10", "255.255.255.0", "192.0.2.1",
		"198.51.100.20", "255.255.255.128", "198.51.100.1",
		1, 1, 1, &dhcp_bound_fake_ops, &fake, &rollback_status);
	assert(transaction_status == ENOSPC && rollback_status == 0);
	assert(strcmp(fake.events, "dCSAPsa") == 0);

	memset(&fake, 0, sizeof(fake));
	fake.publish_status = ENOSPC;
	fake.delete_new_status = EIO;
	fake.set_old_status = EIO;
	fake.add_old_status = EIO;
	transaction_status = dhcp_lease_bound_apply(
		"192.0.2.10", "255.255.255.0", "192.0.2.1",
		"198.51.100.20", "255.255.255.128", "198.51.100.1",
		1, 1, 1, &dhcp_bound_fake_ops, &fake, &rollback_status);
	assert(transaction_status == ENOSPC && rollback_status == EIO);
	assert(strcmp(fake.events, "dCSAPDsa") == 0);

	memset(&fake, 0, sizeof(fake));
	fake.delete_old_status = ESRCH;
	transaction_status = dhcp_lease_bound_apply(
		"192.0.2.10", "255.255.255.0", "192.0.2.1",
		"192.0.2.10", "255.255.255.0", "198.51.100.1",
		0, 0, 1, &dhcp_bound_fake_ops, &fake, &rollback_status);
	assert(transaction_status == 0 && rollback_status == 0);
	assert(strcmp(fake.events, "dAP") == 0);

	memset(&fake, 0, sizeof(fake));
	transaction_status = dhcp_lease_bound_apply(
		"192.0.2.10", "255.255.255.0", "192.0.2.1",
		"192.0.2.10", "255.255.255.0", "",
		0, 0, 1, &dhcp_bound_fake_ops, &fake, &rollback_status);
	assert(transaction_status == 0 && rollback_status == 0);
	assert(strcmp(fake.events, "dP") == 0);

	memset(&fake, 0, sizeof(fake));
	fake.add_old_status = EEXIST;
	transaction_status = dhcp_lease_bound_apply(
		"192.0.2.10", "255.255.255.0", "192.0.2.1",
		"192.0.2.10", "255.255.255.0", "192.0.2.1",
		0, 0, 1, &dhcp_bound_fake_ops, &fake, &rollback_status);
	assert(transaction_status == 0 && rollback_status == 0);
	assert(strcmp(fake.events, "aP") == 0);

	puts("DHCP lease input policy regression: OK");
	return 0;
}
