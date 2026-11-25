#include <assert.h>
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

	puts("VLAN interface tri-state probe and rollback ownership policy: PASS");
	return 0;
}
