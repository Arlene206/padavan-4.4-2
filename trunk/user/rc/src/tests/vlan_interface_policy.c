#include <assert.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>

#include "vlan_interface_policy.h"

static void
check_probe(int ifflags, int probe_errno, int expected_status, int expected_exists)
{
	int exists = -1;
	int status;

	status = vlan_interface_probe_result(ifflags, probe_errno, &exists);
	assert(status == expected_status);
	assert(exists == expected_exists);
}


static const char *restore_calls[3];
static int restore_values[4];
static int restore_results[4];
static int restore_call_count;

static int
record_restore(const char *name, int value)
{
	int index = restore_call_count++;
	assert(index >= 0 && index < 5);
	restore_calls[index] = name;
	restore_values[index] = value;
	return restore_results[index];
}

static int restore_admin_up(const char *ifname, int up)
{
	assert(strcmp(ifname, "eth2.35") == 0);
	return record_restore(up ? "up" : "down", up);
}
static int restore_mtu(const char *ifname, int mtu)
{
	assert(strcmp(ifname, "eth2.35") == 0);
	return record_restore("mtu", mtu);
}
static int restore_hwaddr(const char *ifname, const char *mac)
{
	assert(strcmp(ifname, "eth2.35") == 0);
	assert(strcmp(mac, "00:11:22:33:44:55") == 0);
	return record_restore("hwaddr", 1);
}
static const vlan_interface_restore_ops_t restore_ops = {
	restore_admin_up, restore_mtu, restore_hwaddr
};

static void
reset_restore_mocks(void)
{
	int i;
	restore_call_count = 0;
	for (i = 0; i < 4; ++i) {
		restore_calls[i] = NULL;
		restore_values[i] = -1;
		restore_results[i] = 0;
	}
}

static void
test_reused_vlan_restore_attempts_all_original_attributes_in_order(void)
{
	int failures = -1;
	reset_restore_mocks();
	assert(vlan_interface_restore_existing(&restore_ops, "eth2.35", 1,
		1, 1480, 1, "00:11:22:33:44:55", &failures) == 0);
	assert(failures == 0 && restore_call_count == 4);
	assert(strcmp(restore_calls[0], "down") == 0);
	assert(strcmp(restore_calls[1], "mtu") == 0 && restore_values[1] == 1480);
	assert(strcmp(restore_calls[2], "hwaddr") == 0);
	assert(strcmp(restore_calls[3], "up") == 0);
}

static void
test_reused_vlan_restore_attempts_all_operations_after_failure(void)
{
	int failures = 0;
	reset_restore_mocks();
	restore_results[0] = EBUSY;
	restore_results[2] = EIO;
	restore_results[3] = ENODEV;
	assert(vlan_interface_restore_existing(&restore_ops, "eth2.35", 0,
		1, 1500, 1, "00:11:22:33:44:55", &failures) == EBUSY);
	assert(failures == 3 && restore_call_count == 4);
	assert(strcmp(restore_calls[4], "down") == 0);
}

static void
test_reused_vlan_restore_skips_attributes_not_changed(void)
{
	int failures = -1;
	reset_restore_mocks();
	assert(vlan_interface_restore_existing(&restore_ops, "eth2.35", 0,
		0, 0, 0, NULL, &failures) == 0);
	assert(failures == 0 && restore_call_count == 2);
	assert(strcmp(restore_calls[0], "down") == 0);
	assert(strcmp(restore_calls[1], "down") == 0);
}

int
main(void)
{
	assert(vlan_interface_probe_result(0, 0, NULL) == EINVAL);
	check_probe(0, 0, 0, 1);
	check_probe(1, 0, 0, 1);
	check_probe(-1, ENODEV, 0, 0);
	check_probe(-1, ENXIO, 0, 0);
	check_probe(-1, EPERM, EPERM, 0);
	check_probe(-1, EIO, EIO, 0);
	check_probe(-1, 0, EIO, 0);

	assert(vlan_interface_cleanup_status(0, 0) == 0);
	assert(vlan_interface_cleanup_status(EIO, 0) == EIO);
	assert(vlan_interface_cleanup_status(0, EBUSY) == EBUSY);
	assert(vlan_interface_cleanup_status(EIO, EBUSY) == EIO);

	assert(vlan_interface_retain_ownership(1, 0) == 0);
	assert(vlan_interface_retain_ownership(1, EIO) == 1);
	assert(vlan_interface_retain_ownership(0, EIO) == 0);

	test_reused_vlan_restore_attempts_all_original_attributes_in_order();
	test_reused_vlan_restore_attempts_all_operations_after_failure();
	test_reused_vlan_restore_skips_attributes_not_changed();

	puts("VLAN interface tri-state probe and rollback ownership policy: PASS");
	return 0;
}
