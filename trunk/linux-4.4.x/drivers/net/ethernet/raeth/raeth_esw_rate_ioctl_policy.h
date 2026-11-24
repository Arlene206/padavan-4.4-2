#ifndef RAETH_ESW_RATE_IOCTL_POLICY_H
#define RAETH_ESW_RATE_IOCTL_POLICY_H

#ifdef __KERNEL__
#include <linux/errno.h>
#else
#include <errno.h>
#endif

#define RAETH_ESW_RATE_MAX_PORT 4U
#define RAETH_ESW_RATE_MAX_BANDWIDTH_MBPS (255U * 10000U)

/* The rate field has an eight-bit magnitude and a separate scale selector. */
static inline int
raeth_esw_rate_ioctl_status(unsigned int on_off,
			    unsigned int port,
			    unsigned int bandwidth_mbps)
{
	if (on_off > 1U || port > RAETH_ESW_RATE_MAX_PORT ||
	    bandwidth_mbps > RAETH_ESW_RATE_MAX_BANDWIDTH_MBPS)
		return -EINVAL;

	return 0;
}

#endif /* RAETH_ESW_RATE_IOCTL_POLICY_H */
