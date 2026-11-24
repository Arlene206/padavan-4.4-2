#ifndef RAETH_ESW_REG_OFFSET_POLICY_H
#define RAETH_ESW_REG_OFFSET_POLICY_H

#ifdef __KERNEL__
#include <linux/errno.h>
#else
#include <errno.h>
#endif

#define RAETH_ESW_REG_OFFSET_ALIGNMENT 4U

/* The ESW ioctl accesses 32-bit MMIO registers through raw_readl/raw_writel. */
static inline int
raeth_esw_reg_offset_status(unsigned int offset, unsigned int max_offset)
{
	if (offset > max_offset ||
	    (offset & (RAETH_ESW_REG_OFFSET_ALIGNMENT - 1U)) != 0U)
		return -EINVAL;

	return 0;
}

#endif /* RAETH_ESW_REG_OFFSET_POLICY_H */
