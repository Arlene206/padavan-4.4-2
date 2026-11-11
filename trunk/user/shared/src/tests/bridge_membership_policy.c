#include <stdio.h>
#include <stdlib.h>
#include "bridge_membership_policy.h"

static void
expect_membership(const int *ifindices, unsigned int count, int ifindex,
		  int expected, const char *label)
{
	int actual = bridge_port_list_contains(ifindices, count, ifindex);

	if (actual != expected) {
		fprintf(stderr, "%s: expected %d got %d\n", label, expected, actual);
		exit(1);
	}
}

static void
expect_remove_vlan(int do_if_rem, const char *ifname, const char *lan_ifname,
		   int expected, const char *label)
{
	int actual = bridge_cleanup_should_remove_vlan(do_if_rem, ifname, lan_ifname);

	if (actual != expected) {
		fprintf(stderr, "%s: expected %d got %d\\n", label, expected, actual);
		exit(1);
	}
}

int
main(void)
{
	int ifindices[BRIDGE_PORT_LIST_CAPACITY] = {0};

	ifindices[0] = 3;
	ifindices[17] = 22;
	ifindices[BRIDGE_PORT_LIST_CAPACITY - 1] = 1024;

	expect_membership(ifindices, BRIDGE_PORT_LIST_CAPACITY, 3, 1, "first bridge port");
	expect_membership(ifindices, BRIDGE_PORT_LIST_CAPACITY, 22, 1, "middle bridge port");
	expect_membership(ifindices, BRIDGE_PORT_LIST_CAPACITY, 1024, 1, "last bridge port");
	expect_membership(ifindices, BRIDGE_PORT_LIST_CAPACITY, 99, 0, "different interface");
	expect_membership(ifindices, 0, 3, 0, "empty port list");
	expect_membership(NULL, BRIDGE_PORT_LIST_CAPACITY, 3, 0, "missing port list");
	expect_membership(ifindices, BRIDGE_PORT_LIST_CAPACITY, 0, 0, "zero interface index");
	expect_membership(ifindices, BRIDGE_PORT_LIST_CAPACITY, -1, 0, "negative interface index");

	expect_remove_vlan(0, "eth2.35", "br0", 0, "removal disabled");
	expect_remove_vlan(1, "br0", "br0", 0, "LAN bridge preserved");
	expect_remove_vlan(1, "eth2", "br0", 0, "physical port preserved");
	expect_remove_vlan(1, "eth2.35", "br0", 1, "VLAN port selected");
	expect_remove_vlan(1, NULL, "br0", 0, "missing interface name");
	expect_remove_vlan(1, "", "br0", 0, "empty interface name");

	if (bridge_cleanup_should_set_interface_down(0, 1) != 0 ||
	    bridge_cleanup_should_set_interface_down(1, 0) != 0 ||
	    bridge_cleanup_should_set_interface_down(1, 1) != 1) {
		fprintf(stderr, "bridge cleanup down-flag policy mismatch\n");
		return 1;
	}
	if (bridge_cleanup_status_update(0, 0) != 0 ||
	    bridge_cleanup_status_update(0, -1) != -1 ||
	    bridge_cleanup_status_update(-1, 0) != -1) {
		fprintf(stderr, "bridge cleanup error aggregation mismatch\n");
		return 1;
	}

	puts("Bridge membership and teardown policy regression: PASS");
	return 0;
}
