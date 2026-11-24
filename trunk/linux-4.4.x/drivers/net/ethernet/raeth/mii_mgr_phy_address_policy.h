#ifndef RAETH_MII_MGR_PHY_ADDRESS_POLICY_H
#define RAETH_MII_MGR_PHY_ADDRESS_POLICY_H

#ifdef __KERNEL__
#include <linux/errno.h>
#else
#include <errno.h>
#endif

#define RAETH_MII_PHY_ADDRESS_MAX 31U
#define RAETH_MII_PHY_REGISTER_MAX 31U
#define RAETH_MII_PHY_WRITE_DATA_MAX 0xffffU

/* Clause 22/45 MDIO PHY/port addresses are five-bit fields. */
static inline int
mii_mgr_phy_address_status(unsigned int phy_addr)
{
	return phy_addr <= RAETH_MII_PHY_ADDRESS_MAX ? 0 : -EINVAL;
}

static inline int
mii_mgr_phy_register_status(unsigned int phy_register)
{
	return phy_register <= RAETH_MII_PHY_REGISTER_MAX ? 0 : -EINVAL;
}

static inline int
mii_mgr_phy_write_data_status(unsigned int write_data)
{
	return write_data <= RAETH_MII_PHY_WRITE_DATA_MAX ? 0 : -EINVAL;
}

#endif /* RAETH_MII_MGR_PHY_ADDRESS_POLICY_H */
