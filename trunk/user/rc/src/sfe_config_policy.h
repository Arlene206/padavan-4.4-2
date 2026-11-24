#ifndef RC_SFE_CONFIG_POLICY_H
#define RC_SFE_CONFIG_POLICY_H

#include <errno.h>

/* Map the configured SFE mode to the sysfs control and report write errors. */
static inline int
rc_sfe_configure_policy(int sfe_enable, int (*set_skip_to_bridge_ingress)(int))
{
	int skip_to_bridge_ingress;
	int status;

	if (sfe_enable == 0)
		return 0;
	if (sfe_enable == 1)
		skip_to_bridge_ingress = 0;
	else if (sfe_enable == 2)
		skip_to_bridge_ingress = 1;
	else
		return EINVAL;

	if (!set_skip_to_bridge_ingress)
		return EINVAL;

	status = set_skip_to_bridge_ingress(skip_to_bridge_ingress);
	if (status == 0)
		return 0;

	return status > 0 ? status : EIO;
}

#endif /* RC_SFE_CONFIG_POLICY_H */
