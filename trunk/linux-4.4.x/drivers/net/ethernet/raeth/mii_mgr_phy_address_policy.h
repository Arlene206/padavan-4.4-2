#ifndef RAETH_MII_MGR_PHY_ADDRESS_POLICY_H
#define RAETH_MII_MGR_PHY_ADDRESS_POLICY_H

#ifdef __KERNEL__
#include <linux/errno.h>
#else
#include <errno.h>
#endif

#define RAETH_MII_PHY_ADDRESS_MAX 31U

/* Clause 22/45 MDIO PHY/port addresses are five-bit fields. */
static inline int
mii_mgr_phy_address_status(unsigned int phy_addr)
{
	return phy_addr <= RAETH_MII_PHY_ADDRESS_MAX ? 0 : -EINVAL;
}

#endif /* RAETH_MII_MGR_PHY_ADDRESS_POLICY_H */
