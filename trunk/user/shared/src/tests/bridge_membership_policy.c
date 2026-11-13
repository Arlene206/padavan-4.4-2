#include <errno.h>
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
expect_membership_request(int is_add, int membership, int expected, const char *label)
{
	int actual = bridge_membership_request_satisfied(is_add, membership);

	if (actual != expected) {
		fprintf(stderr, "%s: expected %d got %d\n", label, expected, actual);
		exit(1);
	}
}

static void
expect_membership_change(int is_add, int membership_before, int operation_status,
			 int expected, const char *label)
{
	int actual = bridge_membership_operation_changed(is_add, membership_before,
						 operation_status);

	if (actual != expected) {
		fprintf(stderr, "%s: expected %d got %d\n", label, expected, actual);
		exit(1);
	}
}

static void
expect_membership_race(int is_add, int error_number, int expected, const char *label)
{
	int actual = bridge_membership_race_is_idempotent(is_add, error_number);

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

	/* The requested membership state is idempotent in both directions. */
	expect_membership_request(1, 1, 1, "add already present");
	expect_membership_request(1, 0, 0, "add missing");
	expect_membership_request(1, -1, 0, "add unknown inventory");
	expect_membership_request(0, 0, 1, "delete already absent");
	expect_membership_request(0, 1, 0, "delete still present");
	expect_membership_request(0, -1, 0, "delete unknown inventory");
	expect_membership_change(1, 0, 0, 1, "new add owns membership change");
	expect_membership_change(1, 1, 0, 0, "already-attached add owns no change");
	expect_membership_change(1, 0, -1, 0, "failed add owns no change");
	expect_membership_change(0, 1, 0, 1, "new delete owns membership change");
	expect_membership_change(0, 0, 0, 0, "already-absent delete owns no change");
	expect_membership_change(0, 1, -1, 0, "failed delete owns no change");
	expect_membership_change(1, -1, 0, 0, "unknown add inventory owns no change");
	expect_membership_change(0, 2, 0, 0, "invalid membership inventory owns no change");
	expect_membership_race(1, EBUSY, 1, "duplicate add race");
	expect_membership_race(0, EINVAL, 1, "duplicate delete race");
	expect_membership_race(1, EINVAL, 0, "add unexpected EINVAL");
	expect_membership_race(0, EBUSY, 0, "delete unexpected EBUSY");

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
