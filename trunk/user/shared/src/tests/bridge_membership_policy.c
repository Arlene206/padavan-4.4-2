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


static int m2u_fast_leave;
static int m2u_to_unicast;
static const char *m2u_calls[8];
static int m2u_call_values[8];
static int m2u_call_status[8];
static int m2u_call_errno[8];
static int m2u_call_count;

static int
m2u_record(const char *operation, int value)
{
	int index = m2u_call_count++;
	assert(index >= 0 && index < 8);
	m2u_calls[index] = operation;
	m2u_call_values[index] = value;
	if (m2u_call_status[index] != 0) {
		errno = m2u_call_errno[index] ? m2u_call_errno[index] : EIO;
		return -1;
	}
	return 0;
}

static int
m2u_get_mock(const char *ifname, const char *param, int *value)
{
	const char *operation;
	assert(strcmp(ifname, "eth2.35") == 0);
	assert(value != NULL);
	if (strcmp(param, "multicast_fast_leave") == 0) {
		operation = "get-fast";
		if (m2u_record(operation, 0) != 0)
			return -1;
		*value = m2u_fast_leave;
		return 0;
	}
	assert(strcmp(param, "multicast_to_unicast") == 0);
	operation = "get-m2u";
	if (m2u_record(operation, 0) != 0)
		return -1;
	*value = m2u_to_unicast;
	return 0;
}

static int
m2u_set_mock(const char *ifname, const char *param, int value)
{
	const char *operation;
	int ret;
	assert(strcmp(ifname, "eth2.35") == 0);
	if (strcmp(param, "multicast_fast_leave") == 0)
		operation = "set-fast";
	else {
		assert(strcmp(param, "multicast_to_unicast") == 0);
		operation = "set-m2u";
	}
	ret = m2u_record(operation, value);
	if (ret != 0)
		return ret;
	if (strcmp(param, "multicast_fast_leave") == 0)
		m2u_fast_leave = value;
	else
		m2u_to_unicast = value;
	return 0;
}

static const bridge_port_m2u_ops_t m2u_ops = {
	m2u_get_mock, m2u_set_mock
};

static void
m2u_reset(int fast_leave, int to_unicast)
{
	int i;
	m2u_fast_leave = fast_leave;
	m2u_to_unicast = to_unicast;
	m2u_call_count = 0;
	for (i = 0; i < 8; ++i) {
		m2u_calls[i] = NULL;
		m2u_call_values[i] = 0;
		m2u_call_status[i] = 0;
		m2u_call_errno[i] = 0;
	}
}

static int
m2u_apply(int enabled, int *rollback_status, int *rollback_errno)
{
	return bridge_port_m2u_apply_with_ops(
		"eth2.35", enabled, &m2u_ops, rollback_status, rollback_errno);
}

static void
test_m2u_update_snapshots_both_flags_before_mutation(void)
{
	int rollback_status, rollback_errno;
	m2u_reset(0, 0);
	assert(m2u_apply(1, &rollback_status, &rollback_errno) == 0);
	assert(m2u_call_count == 4);
	assert(strcmp(m2u_calls[0], "get-fast") == 0);
	assert(strcmp(m2u_calls[1], "get-m2u") == 0);
	assert(strcmp(m2u_calls[2], "set-fast") == 0 && m2u_call_values[2] == 1);
	assert(strcmp(m2u_calls[3], "set-m2u") == 0 && m2u_call_values[3] == 1);
	assert(m2u_fast_leave == 1 && m2u_to_unicast == 1);
	assert(rollback_status == 0 && rollback_errno == 0);
}

static void
test_m2u_second_write_failure_restores_first_flag(void)
{
	int rollback_status, rollback_errno;
	m2u_reset(0, 0);
	m2u_call_status[3] = -1;
	m2u_call_errno[3] = EIO;
	errno = 0;
	assert(m2u_apply(1, &rollback_status, &rollback_errno) == -1);
	assert(errno == EIO);
	assert(m2u_call_count == 6);
	assert(strcmp(m2u_calls[4], "set-m2u") == 0 && m2u_call_values[4] == 0);
	assert(strcmp(m2u_calls[5], "set-fast") == 0 && m2u_call_values[5] == 0);
	assert(m2u_fast_leave == 0 && m2u_to_unicast == 0);
	assert(rollback_status == 0 && rollback_errno == 0);
}

static void
test_m2u_rollback_failures_are_reported_without_losing_primary_errno(void)
{
	int rollback_status, rollback_errno;
	m2u_reset(0, 0);
	m2u_call_status[3] = -1;
	m2u_call_errno[3] = EIO;
	m2u_call_status[4] = -1;
	m2u_call_errno[4] = EBUSY;
	m2u_call_status[5] = -1;
	m2u_call_errno[5] = ENODEV;
	errno = 0;
	assert(m2u_apply(1, &rollback_status, &rollback_errno) == -1);
	assert(errno == EIO);
	assert(rollback_status == -1 && rollback_errno == EBUSY);
	assert(m2u_fast_leave == 1 && m2u_to_unicast == 0);
	assert(m2u_call_count == 6);
}

static void
test_m2u_snapshot_failure_does_not_mutate_either_flag(void)
{
	int rollback_status, rollback_errno;
	m2u_reset(0, 0);
	m2u_call_status[1] = -1;
	m2u_call_errno[1] = EACCES;
	errno = 0;
	assert(m2u_apply(1, &rollback_status, &rollback_errno) == -1);
	assert(errno == EACCES);
	assert(m2u_call_count == 2);
	assert(m2u_fast_leave == 0 && m2u_to_unicast == 0);
}

static void
test_m2u_repeated_target_state_is_idempotent(void)
{
	int rollback_status, rollback_errno;
	m2u_reset(1, 1);
	assert(m2u_apply(1, &rollback_status, &rollback_errno) == 0);
	assert(m2u_call_count == 2);
	assert(m2u_fast_leave == 1 && m2u_to_unicast == 1);
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

	test_m2u_update_snapshots_both_flags_before_mutation();
	test_m2u_second_write_failure_restores_first_flag();
	test_m2u_rollback_failures_are_reported_without_losing_primary_errno();
	test_m2u_snapshot_failure_does_not_mutate_either_flag();
	test_m2u_repeated_target_state_is_idempotent();

	puts("Bridge membership and teardown policy regression: PASS");
	return 0;
}
