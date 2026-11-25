#ifndef RAETH_QDMA_REGISTER_OFFSET_POLICY_H
#define RAETH_QDMA_REGISTER_OFFSET_POLICY_H

#ifdef __KERNEL__
#include <linux/errno.h>
#else
#include <errno.h>
#endif

#define RAETH_QDMA_REGISTER_OFFSET_ALIGNMENT 4U
#define RAETH_QDMA_REGISTER_OFFSET_LEGACY_MAGIC 0x777U

/* QDMA registers are 32-bit MMIO words; 0x777 is a legacy MT7622 selector. */
static inline int
raeth_qdma_register_offset_status(unsigned int offset,
				  unsigned int max_offset,
				  int allow_legacy_magic)
{
	if (offset > max_offset)
		return -EINVAL;

	if ((offset & (RAETH_QDMA_REGISTER_OFFSET_ALIGNMENT - 1U)) != 0U &&
	    !(allow_legacy_magic &&
	      offset == RAETH_QDMA_REGISTER_OFFSET_LEGACY_MAGIC))
		return -EINVAL;

	return 0;
}

#endif /* RAETH_QDMA_REGISTER_OFFSET_POLICY_H */
