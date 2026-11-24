#ifndef RAETH_CL45_IOCTL_STATUS_POLICY_H
#define RAETH_CL45_IOCTL_STATUS_POLICY_H

#ifdef __KERNEL__
#include <linux/errno.h>
#else
#include <errno.h>
#endif

/* The MDIO manager reports exactly 1 on success and 0 on timeout/error. */
static inline int
raeth_cl45_ioctl_status(unsigned int mdio_status)
{
	return mdio_status == 1U ? 0 : -EIO;
}

#endif /* RAETH_CL45_IOCTL_STATUS_POLICY_H */
