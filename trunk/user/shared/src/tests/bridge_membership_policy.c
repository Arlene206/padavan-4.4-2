#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include "bridge_membership_policy.h"

static void
expect_port_list_empty(const int *ifindices, unsigned int count, int expected,
		       const char *label)
{
	int actual = bridge_port_list_is_empty(ifindices, count);

	if (actual != expected) {
		fprintf(stderr, "%s: expected %d got %d\n", label, expected, actual);
		exit(1);
	}
}

static void
expect_batch_membership_safe(int attempted, int added, int detached, int expected,
			     const char *label)
{
	int actual = bridge_batch_membership_detached_safe(attempted, added, detached);

	if (actual != expected) {
		fprintf(stderr, "%s: expected %d got %d\n", label, expected, actual);
		exit(1);
	}
}

static void
expect_batch_bridge_removal(int created, int empty, int expected, const char *label)
{
	int actual = bridge_batch_can_remove_created_bridge(created, empty);

	if (actual != expected) {
		fprintf(stderr, "%s: expected %d got %d\n", label, expected, actual);
		exit(1);
	}
}

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
	int empty_list[1] = {0};

	expect_port_list_empty(ifindices, BRIDGE_PORT_LIST_CAPACITY, 1, "empty bridge inventory");
	expect_port_list_empty(NULL, BRIDGE_PORT_LIST_CAPACITY, -1, "unknown bridge inventory");
	expect_batch_membership_safe(0, 0, 0, 1, "attach not attempted");
	expect_batch_membership_safe(1, 1, 1, 1, "owned membership detached");
	expect_batch_membership_safe(1, 1, 0, 0, "owned membership detach failed");
	expect_batch_membership_safe(1, 0, 1, 0, "idempotent/pre-existing membership not owned");
	expect_batch_membership_safe(1, 0, 0, 0, "failed or ambiguous attach not assumed detached");
	expect_batch_bridge_removal(1, 1, 1, "created empty bridge can be removed");
	expect_batch_bridge_removal(0, 1, 0, "pre-existing bridge is preserved");
	expect_batch_bridge_removal(1, 0, 0, "created non-empty bridge is preserved");
	expect_batch_bridge_removal(1, -1, 0, "unknown bridge inventory is preserved");
	expect_port_list_empty(empty_list, 1, 1, "single empty port inventory");

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

	/* Rollback must not delete a VLAN unless this call created it and */
	/* the bridge membership is known to be detached. */
	if (!bridge_rollback_can_remove_created_vlan(1, 1) ||
	    bridge_rollback_can_remove_created_vlan(0, 1) ||
	    bridge_rollback_can_remove_created_vlan(1, 0)) {
		fprintf(stderr, "bridge rollback VLAN ownership gate mismatch\n");
		return 1;
	}
	if (!bridge_rollback_can_restore_interface(1, 1) ||
	    bridge_rollback_can_restore_interface(0, 1) ||
	    bridge_rollback_can_restore_interface(1, 0)) {
		fprintf(stderr, "bridge rollback interface-state gate mismatch\n");
		return 1;
	}

	puts("Bridge membership and teardown policy regression: PASS");
	return 0;
}
