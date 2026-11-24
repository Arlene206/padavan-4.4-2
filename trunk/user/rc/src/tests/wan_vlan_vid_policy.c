#include <assert.h>
#include <errno.h>
#include "../wan_vlan_vid_policy.h"

int
main(void)
{
	/* The WAN VLAN lookup contract uses negative errno values for failures. */
	assert(wan_vlan_vid_policy_read_failure_result(0) == 0);
	assert(wan_vlan_vid_policy_read_failure_result(EIO) == -EIO);
	assert(wan_vlan_vid_policy_read_failure_result(ENODEV) == -ENODEV);
	assert(wan_vlan_vid_policy_read_failure_result(EACCES) == -EACCES);

	/* Already-negative failures stay negative and are not sign-flipped. */
	assert(wan_vlan_vid_policy_read_failure_result(-EIO) == -EIO);
	assert(wan_vlan_vid_policy_read_failure_result(-EINVAL) == -EINVAL);
	return 0;
}
