#ifndef _RAETH_MII_MGR_INDIRECT_POLICY_H
#define _RAETH_MII_MGR_INDIRECT_POLICY_H

typedef unsigned int (*mii_mgr_indirect_read_fn)(void *context,
		unsigned int phy_addr, unsigned int phy_register,
		unsigned int *read_data);
typedef unsigned int (*mii_mgr_indirect_write_fn)(void *context,
		unsigned int phy_addr, unsigned int phy_register,
		unsigned int write_data);

/*
 * The caller must hold the shared MDIO bus lock for the complete sequence.
 * Page selection and both data words form one hardware transaction; allowing
 * another Clause-22/Clause-45 request between these operations changes the
 * selected MMD register and can silently corrupt an unrelated PHY access.
 */
static inline unsigned int
mii_mgr_indirect_read(unsigned int phy_addr, unsigned int phy_register,
		      void *context, mii_mgr_indirect_write_fn write_register,
		      mii_mgr_indirect_read_fn read_register,
		      unsigned int *read_data)
{
	unsigned int low_word = 0;
	unsigned int high_word = 0;

	if (!read_data)
		return 0;
	*read_data = 0;
	if (!write_register || !read_register)
		return 0;

	if (!write_register(context, phy_addr, 0x1fU,
			    (phy_register >> 6) & 0x3ffU))
		return 0;
	if (!read_register(context, phy_addr,
			   (phy_register >> 2) & 0x0fU, &low_word))
		return 0;
	if (!read_register(context, phy_addr, 0x10U, &high_word))
		return 0;

	*read_data = (high_word << 16) | (low_word & 0xffffU);
	return 1;
}

static inline unsigned int
mii_mgr_indirect_write(unsigned int phy_addr, unsigned int phy_register,
		       unsigned int write_data, void *context,
		       mii_mgr_indirect_write_fn write_register)
{
	if (!write_register)
		return 0;
	if (!write_register(context, phy_addr, 0x1fU,
			    (phy_register >> 6) & 0x3ffU))
		return 0;
	if (!write_register(context, phy_addr,
			    (phy_register >> 2) & 0x0fU,
			    write_data & 0xffffU))
		return 0;
	if (!write_register(context, phy_addr, 0x10U,
			    (write_data >> 16) & 0xffffU))
		return 0;
	return 1;
}

#endif /* _RAETH_MII_MGR_INDIRECT_POLICY_H */
