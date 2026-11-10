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

	puts("Bridge membership and cleanup policy regression: PASS");
	return 0;
}
