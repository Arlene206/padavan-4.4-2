#ifndef RAETH_MDIO_IOCTL_STATUS_POLICY_H
#define RAETH_MDIO_IOCTL_STATUS_POLICY_H

#ifdef __KERNEL__
#include <linux/errno.h>
#else
#include <errno.h>
#endif

/* The RAETH MDIO manager reports exactly 1 on success and 0 on failure. */
static inline int
raeth_mdio_ioctl_status(unsigned int mdio_status)
{
	return mdio_status == 1U ? 0 : -EIO;
}

#endif /* RAETH_MDIO_IOCTL_STATUS_POLICY_H */
