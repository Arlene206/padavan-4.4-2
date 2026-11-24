#ifndef WAN_SRC_PHY_POLICY_H
#define WAN_SRC_PHY_POLICY_H

#include <errno.h>

/* WAN=0 and external LAN ports 1..5 are selectable PHYs. CPU ports are not.
 * Keep the historical negative-value fallback to WAN, but reject unknown
 * positive IDs: MTK's status ioctl maps unmapped ports to success/link-down.
 */
#define WAN_SRC_PHY_DEFAULT 0
#define WAN_SRC_PHY_MAX_EXTERNAL 5

static inline int
wan_src_phy_resolve(int configured_port, int *resolved_port)
{
	if (!resolved_port)
		return EINVAL;
	if (configured_port < 0)
		configured_port = WAN_SRC_PHY_DEFAULT;
	if (configured_port > WAN_SRC_PHY_MAX_EXTERNAL)
		return ERANGE;
	*resolved_port = configured_port;
	return 0;
}

#endif /* WAN_SRC_PHY_POLICY_H */
