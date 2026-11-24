#ifndef RAETH_QDMA_QUEUE_MAPPING_POLICY_H
#define RAETH_QDMA_QUEUE_MAPPING_POLICY_H

#ifdef __KERNEL__
#include <linux/errno.h>
#else
#include <errno.h>
#endif

#define RAETH_QDMA_QUEUE_MAPPING_TABLE_SIZE 64U
#define RAETH_QDMA_QUEUE_MAPPING_INDEX_MASK \
	(RAETH_QDMA_QUEUE_MAPPING_TABLE_SIZE - 1U)
#define RAETH_QDMA_QUEUE_MAPPING_SEPARATE_MASK 0x100U
#define RAETH_QDMA_QUEUE_ID_MAX 63U

static inline int
raeth_qdma_queue_id_status(unsigned int queue_id)
{
	return queue_id <= RAETH_QDMA_QUEUE_ID_MAX ? 0 : -EINVAL;
}

/*
 * The selector carries a six-bit skb mark and, optionally, bit 8 to enable
 * LAN/WAN separation. Reject reserved bits rather than silently masking them
 * into an unchecked table index.
 */
static inline int
raeth_qdma_queue_mapping_decode(unsigned int selector,
				unsigned int *index,
				unsigned int *separate)
{
	const unsigned int valid_mask =
		RAETH_QDMA_QUEUE_MAPPING_SEPARATE_MASK |
		RAETH_QDMA_QUEUE_MAPPING_INDEX_MASK;

	if (!index || !separate || (selector & ~valid_mask) != 0)
		return -EINVAL;

	*index = selector & RAETH_QDMA_QUEUE_MAPPING_INDEX_MASK;
	*separate = (selector & RAETH_QDMA_QUEUE_MAPPING_SEPARATE_MASK) != 0;
	return 0;
}

#endif /* RAETH_QDMA_QUEUE_MAPPING_POLICY_H */
