#ifndef RAETH_MDIO_CL45_ADDRESS_POLICY_H
#define RAETH_MDIO_CL45_ADDRESS_POLICY_H

#ifdef __KERNEL__
#include <linux/errno.h>
#else
#include <errno.h>
#endif

#define RAETH_MDIO_CL45_PORT_MAX 31U
#define RAETH_MDIO_CL45_DEVICE_MAX 31U
#define RAETH_MDIO_CL45_REGISTER_MAX 0xffffU
#define RAETH_MDIO_CL45_DATA_MAX 0xffffU

/*
 * Clause 45 packs five-bit port/device addresses alongside a 16-bit
 * register address in the MDIO command word. Reject values that would
 * spill into operation/control bits before starting the transaction.
 */
static inline int
raeth_mdio_cl45_write_data_status(unsigned int write_data)
{
	return write_data > RAETH_MDIO_CL45_DATA_MAX ? -EINVAL : 0;
}

static inline int
raeth_mdio_cl45_address_status(unsigned int port_num,
			       unsigned int dev_addr,
			       unsigned int reg_addr)
{
	if (port_num > RAETH_MDIO_CL45_PORT_MAX ||
	    dev_addr > RAETH_MDIO_CL45_DEVICE_MAX ||
	    reg_addr > RAETH_MDIO_CL45_REGISTER_MAX)
		return -EINVAL;

	return 0;
}

#endif /* RAETH_MDIO_CL45_ADDRESS_POLICY_H */
