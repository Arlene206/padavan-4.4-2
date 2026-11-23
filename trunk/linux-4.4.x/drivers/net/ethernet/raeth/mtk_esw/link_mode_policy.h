/*
 * Validate and apply MTK ESW speed/flow-control modes.
 * UAPI: link speed in bits 0..3; flow control in bits 8..9.
 */
#ifndef MTK_ESW_LINK_MODE_POLICY_H
#define MTK_ESW_LINK_MODE_POLICY_H

#ifdef __KERNEL__
#include <linux/errno.h>
#else
#include <errno.h>
#endif

#define MTK_ESW_LINK_MODE_SPEED_MASK 0x0000000fU
#define MTK_ESW_LINK_MODE_FLOW_MASK  0x00000300U
#define MTK_ESW_LINK_MODE_VALID_MASK \
	(MTK_ESW_LINK_MODE_SPEED_MASK | MTK_ESW_LINK_MODE_FLOW_MASK)

#define MTK_ESW_LINK_MODE_REG_CONTROL       0U
#define MTK_ESW_LINK_MODE_REG_ADVERTISEMENT 4U
#define MTK_ESW_LINK_MODE_REG_GIGABIT       9U
#define MTK_ESW_LINK_MODE_GIGABIT_MASK      ((1U << 9) | (1U << 8))
#define MTK_ESW_LINK_MODE_AUTONEG_ENABLE    (1U << 12)
#define MTK_ESW_LINK_MODE_AUTONEG_RESTART   (1U << 9)
#define MTK_ESW_LINK_MODE_POWER_DOWN        (1U << 11)

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

/* All I/O callbacks return zero on success and a negative errno on failure. */
struct mtk_esw_link_mode_io {
	int (*read)(void *context, unsigned int phy_addr,
		    unsigned int reg, unsigned int *value);
	int (*write)(void *context, unsigned int phy_addr,
		     unsigned int reg, unsigned int value);
	void (*delay_ms)(void *context, unsigned int milliseconds);
	void (*delay_us)(void *context, unsigned int microseconds);
};

static inline int
mtk_esw_link_mode_io_error(int status)
{
	return status < 0 ? status : -EIO;
}

/*
 * Program PHY registers in order and stop at the first MDIO failure. The
 * caller must publish its cached link mode only after this function succeeds;
 * leaving the cache unchanged allows a later request to retry partial writes.
 */
static inline int
mtk_esw_link_mode_program(unsigned int phy_addr, unsigned int advertisement,
			 unsigned int control, int gigabit_enabled,
			 int power_enabled,
			 const struct mtk_esw_link_mode_io *io,
			 void *context)
{
	unsigned int gigabit_control = 0;
	int status;

	if (!io || !io->read || !io->write || !io->delay_ms ||
	    (gigabit_enabled != 0 && gigabit_enabled != 1) ||
	    (power_enabled != 0 && power_enabled != 1))
		return -EINVAL;

	status = io->read(context, phy_addr, MTK_ESW_LINK_MODE_REG_GIGABIT,
			  &gigabit_control);
	if (status != 0)
		return mtk_esw_link_mode_io_error(status);

	if (gigabit_enabled)
		gigabit_control |= MTK_ESW_LINK_MODE_GIGABIT_MASK;
	else
		gigabit_control &= ~MTK_ESW_LINK_MODE_GIGABIT_MASK;

	status = io->write(context, phy_addr, MTK_ESW_LINK_MODE_REG_GIGABIT,
			   gigabit_control);
	if (status != 0)
		return mtk_esw_link_mode_io_error(status);

	status = io->write(context, phy_addr,
			   MTK_ESW_LINK_MODE_REG_ADVERTISEMENT,
			   advertisement);
	if (status != 0)
		return mtk_esw_link_mode_io_error(status);

	if (power_enabled) {
		if (!(control & MTK_ESW_LINK_MODE_AUTONEG_ENABLE)) {
			/* Forced modes power-cycle the PHY before applying control. */
			control |= MTK_ESW_LINK_MODE_POWER_DOWN;
			status = io->write(context, phy_addr,
					   MTK_ESW_LINK_MODE_REG_CONTROL, control);
			if (status != 0)
				return mtk_esw_link_mode_io_error(status);
			io->delay_ms(context, 500U);
			control &= ~MTK_ESW_LINK_MODE_POWER_DOWN;
		} else {
			control |= MTK_ESW_LINK_MODE_AUTONEG_RESTART;
		}
	} else {
		control |= MTK_ESW_LINK_MODE_POWER_DOWN;
	}

	status = io->write(context, phy_addr, MTK_ESW_LINK_MODE_REG_CONTROL,
			   control);
	if (status != 0)
		return mtk_esw_link_mode_io_error(status);

	return 0;
}


#define MTK_ESW_MAC_TABLE_PHY              0x1fU
#define MTK_ESW_MAC_TABLE_REG              0x80U
#define MTK_ESW_MAC_TABLE_BUSY_MASK        (1U << 15)
#define MTK_ESW_MAC_TABLE_CLEAR_ALL        0x8002U
#define MTK_ESW_MAC_TABLE_CLEAR_STATIC     0x8602U
#define MTK_ESW_MAC_TABLE_POLL_LIMIT       200U
#define MTK_ESW_MAC_TABLE_POLL_DELAY_US   100U

/* A failed status read is UNKNOWN, never proof that the table is idle. */
static inline int
mtk_esw_mac_table_clear(int static_only,
			const struct mtk_esw_link_mode_io *io,
			void *context)
{
	unsigned int atc_value = 0;
	unsigned int i;
	int status;

	if (!io || !io->read || !io->write || !io->delay_us ||
	    (static_only != 0 && static_only != 1))
		return -EINVAL;

	status = io->write(context, MTK_ESW_MAC_TABLE_PHY,
			   MTK_ESW_MAC_TABLE_REG,
			   static_only ? MTK_ESW_MAC_TABLE_CLEAR_STATIC :
			   MTK_ESW_MAC_TABLE_CLEAR_ALL);
	if (status != 0)
		return mtk_esw_link_mode_io_error(status);

	for (i = 0; i < MTK_ESW_MAC_TABLE_POLL_LIMIT; ++i) {
		io->delay_us(context, MTK_ESW_MAC_TABLE_POLL_DELAY_US);
		atc_value = 0;
		status = io->read(context, MTK_ESW_MAC_TABLE_PHY,
				  MTK_ESW_MAC_TABLE_REG, &atc_value);
		if (status != 0)
			return mtk_esw_link_mode_io_error(status);
		if (!(atc_value & MTK_ESW_MAC_TABLE_BUSY_MASK))
			return 0;
	}

	return -ETIMEDOUT;
}

#endif /* MTK_ESW_LINK_MODE_POLICY_H */
