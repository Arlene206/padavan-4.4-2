/*
 * Validate the MTK ESW speed/flow-control mode before touching PHY registers.
 * UAPI: link speed in bits 0..3; flow control in bits 8..9.
 */
#ifndef MTK_ESW_LINK_MODE_POLICY_H
#define MTK_ESW_LINK_MODE_POLICY_H

#define MTK_ESW_LINK_MODE_SPEED_MASK 0x0000000fU
#define MTK_ESW_LINK_MODE_FLOW_MASK  0x00000300U
#define MTK_ESW_LINK_MODE_VALID_MASK \
	(MTK_ESW_LINK_MODE_SPEED_MASK | MTK_ESW_LINK_MODE_FLOW_MASK)

static inline int
mtk_esw_link_mode_is_valid(unsigned int link_mode)
{
	unsigned int speed = link_mode & MTK_ESW_LINK_MODE_SPEED_MASK;
	unsigned int flow = (link_mode & MTK_ESW_LINK_MODE_FLOW_MASK) >> 8;

	if (link_mode & ~MTK_ESW_LINK_MODE_VALID_MASK)
		return 0;
	if (speed > 9U && speed != 15U)
		return 0;
	return flow <= 2U;
}

#endif /* MTK_ESW_LINK_MODE_POLICY_H */
