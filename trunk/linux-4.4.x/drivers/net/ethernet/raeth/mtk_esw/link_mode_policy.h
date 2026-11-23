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
#define MTK_ESW_JUMBO_MAX_RX_MASK          0x3fU
#define MTK_ESW_JUMBO_MAX_RX_9K            (9U << 2)
#define MTK_ESW_JUMBO_RX_ENABLE            0x3U
#define MTK_ESW_JUMBO_RX_DISABLE           0x1U

/*
 * Preserve unrelated GMACCR bits and never write a fabricated register value
 * when an MDIO read fails. The caller publishes its cached mode only on success.
 */
static inline int
mtk_esw_jumbo_frames_apply(unsigned int reg, int enabled,
			   const struct mtk_esw_link_mode_io *io,
			   void *context)
{
	unsigned int value = 0;
	int status;

	if (!io || !io->read || !io->write ||
	    (enabled != 0 && enabled != 1))
		return -EINVAL;

	status = io->read(context, MTK_ESW_MAC_TABLE_PHY, reg, &value);
	if (status != 0)
		return mtk_esw_link_mode_io_error(status);

	value &= ~MTK_ESW_JUMBO_MAX_RX_MASK;
	value |= MTK_ESW_JUMBO_MAX_RX_9K;
	value |= enabled ? MTK_ESW_JUMBO_RX_ENABLE :
		MTK_ESW_JUMBO_RX_DISABLE;

	status = io->write(context, MTK_ESW_MAC_TABLE_PHY, reg, value);
	if (status != 0)
		return mtk_esw_link_mode_io_error(status);

	return 0;
}
#define MTK_ESW_MAC_TABLE_REG              0x80U
#define MTK_ESW_MAC_TABLE_BUSY_MASK        (1U << 15)
#define MTK_ESW_MAC_TABLE_CLEAR_ALL        0x8002U
#define MTK_ESW_MAC_TABLE_CLEAR_STATIC     0x8602U
#define MTK_ESW_MAC_TABLE_POLL_LIMIT       200U
#define MTK_ESW_MAC_TABLE_POLL_DELAY_US   100U
#define MTK_ESW_VLAN_TABLE_REG             0x90U
#define MTK_ESW_VLAN_TABLE_BUSY_MASK       (1U << 31)
#define MTK_ESW_VLAN_TABLE_POLL_LIMIT      200U
#define MTK_ESW_VLAN_TABLE_POLL_DELAY_US   100U

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


/*
 * A failed VTCR write/read is never treated as an idle table. Keep the
 * caller's VLAN-pool bookkeeping unchanged unless the switch confirms that
 * the command completed.
 */
static inline int
mtk_esw_vlan_table_write(unsigned int vtcr_reg, unsigned int command,
			 unsigned int vid,
			 const struct mtk_esw_link_mode_io *io, void *context)
{
	unsigned int value = 0;
	unsigned int poll;
	int status;

	if (!io || !io->read || !io->write || !io->delay_us ||
	    (command != 1U && command != 2U) || vid > 4095U)
		return -EINVAL;

	status = io->write(context, MTK_ESW_MAC_TABLE_PHY, vtcr_reg,
			   (command << 12) | vid | MTK_ESW_VLAN_TABLE_BUSY_MASK);
	if (status != 0)
		return mtk_esw_link_mode_io_error(status);

	for (poll = 0; poll < MTK_ESW_VLAN_TABLE_POLL_LIMIT; ++poll) {
		io->delay_us(context, MTK_ESW_VLAN_TABLE_POLL_DELAY_US);
		value = 0;
		status = io->read(context, MTK_ESW_MAC_TABLE_PHY, vtcr_reg,
				  &value);
		if (status != 0)
			return mtk_esw_link_mode_io_error(status);
		if (!(value & MTK_ESW_VLAN_TABLE_BUSY_MASK))
			return 0;
	}

	return -ETIMEDOUT;
}


/*
 * A VLAN entry is not published until both data words and the VTCR command
 * complete successfully. A failed stage stops subsequent writes so callers
 * can retain their software pool state and retry the entry.
 */
static inline int
mtk_esw_vlan_entry_apply(unsigned int vawd1_reg, unsigned int vawd2_reg,
			 unsigned int vtcr_reg, unsigned int vid,
			 unsigned int vawd1, unsigned int vawd2,
			 const struct mtk_esw_link_mode_io *io, void *context)
{
	int status;

	if (!io || !io->read || !io->write || !io->delay_us || vid > 4095U)
		return -EINVAL;

	status = io->write(context, MTK_ESW_MAC_TABLE_PHY, vawd1_reg, vawd1);
	if (status != 0)
		return mtk_esw_link_mode_io_error(status);

	status = io->write(context, MTK_ESW_MAC_TABLE_PHY, vawd2_reg, vawd2);
	if (status != 0)
		return mtk_esw_link_mode_io_error(status);

	return mtk_esw_vlan_table_write(vtcr_reg, 1U, vid, io, context);
}


/*
 * Change PHY power only after a successful control-register read. A failed
 * write is returned to the caller and must not be recorded as applied state.
 */
static inline int
mtk_esw_phy_power_set(unsigned int phy_addr, int power_on, int force,
		      int enable_autoneg, const struct mtk_esw_link_mode_io *io,
		      void *context, int *power_changed)
{
	unsigned int control = 0;
	int was_powered_on;
	int status;

	if (!power_changed || !io || !io->read || !io->write ||
	    (power_on != 0 && power_on != 1) ||
	    (force != 0 && force != 1) ||
	    (enable_autoneg != 0 && enable_autoneg != 1))
		return -EINVAL;

	*power_changed = 0;
	status = io->read(context, phy_addr, MTK_ESW_LINK_MODE_REG_CONTROL,
			  &control);
	if (status != 0)
		return mtk_esw_link_mode_io_error(status);

	was_powered_on = !(control & MTK_ESW_LINK_MODE_POWER_DOWN);
	control &= ~(MTK_ESW_LINK_MODE_POWER_DOWN |
		     MTK_ESW_LINK_MODE_AUTONEG_RESTART);
	if (enable_autoneg)
		control |= MTK_ESW_LINK_MODE_AUTONEG_ENABLE;

	if (power_on)
		control |= MTK_ESW_LINK_MODE_AUTONEG_RESTART;
	else
		control |= MTK_ESW_LINK_MODE_POWER_DOWN;

	if (force || was_powered_on != power_on) {
		status = io->write(context, phy_addr,
				   MTK_ESW_LINK_MODE_REG_CONTROL, control);
		if (status != 0)
			return mtk_esw_link_mode_io_error(status);
	}

	*power_changed = (was_powered_on != power_on);
	return 0;
}

/* Preserve the first error while still allowing independent PHYs to run. */
static inline int
mtk_esw_first_error(int first_status, int next_status)
{
	if (first_status != 0)
		return first_status;
	return next_status == 0 ? 0 : mtk_esw_link_mode_io_error(next_status);
}

/*
 * Perform a fail-closed masked register update. The caller may publish its
 * software cache only after both the read and write complete successfully.
 */
static inline int
mtk_esw_register_update_bits(unsigned int phy_addr, unsigned int reg,
			     unsigned int clear_mask, unsigned int set_mask,
			     const struct mtk_esw_link_mode_io *io,
			     void *context)
{
	unsigned int value = 0;
	int status;

	if (!io || !io->read || !io->write)
		return -EINVAL;

	status = io->read(context, phy_addr, reg, &value);
	if (status != 0)
		return mtk_esw_link_mode_io_error(status);

	value &= ~clear_mask;
	value |= set_mask;
	status = io->write(context, phy_addr, reg, value);
	if (status != 0)
		return mtk_esw_link_mode_io_error(status);

	return 0;
}

/*
 * Issue one checked register write and normalize callback failures.
 */
static inline int
mtk_esw_register_write(unsigned int phy_addr, unsigned int reg,
                       unsigned int value,
                       const struct mtk_esw_link_mode_io *io, void *context)
{
	int status;

	if (!io || !io->write)
		return -EINVAL;
	status = io->write(context, phy_addr, reg, value);
	if (status != 0)
		return mtk_esw_link_mode_io_error(status);
	return 0;
}


/*
 * Write a per-port register value to every selected switch port. A failed write
 * must not prevent independent ports from being attempted, but any failure
 * keeps the higher-level cached configuration dirty for a later retry.
 */
static inline int
mtk_esw_register_write_ports(unsigned int phy_addr, unsigned int reg_base,
			     unsigned int reg_stride, unsigned int port_count,
			     unsigned int port_mask, unsigned int value,
			     const struct mtk_esw_link_mode_io *io,
			     void *context)
{
	unsigned int port;
	int status, first_error = 0;

	if (!io || !io->write || !reg_stride || !port_count ||
	    port_count > 32U ||
	    (port_count > 1U &&
	     reg_stride > (~0U - reg_base) / (port_count - 1U)))
		return -EINVAL;

	for (port = 0; port < port_count; ++port) {
		if (!(port_mask & (1U << port)))
			continue;

		status = io->write(context, phy_addr,
				   reg_base + reg_stride * port, value);
		if (status != 0)
			first_error = mtk_esw_first_error(first_error, status);
	}

	return first_error;
}

/*
 * Publish requested VLAN rule state only after the caller's complete hardware
 * transition succeeds (including the MAC-table flush). On failure the old
 * snapshot stays dirty so a repeated request retries the programming sequence.
 */
static inline int
mtk_esw_vlan_rules_commit(int status, unsigned int rule_count,
			  const unsigned int *requested,
			  unsigned int *applied)
{
	unsigned int i;

	if (!rule_count || !requested || !applied)
		return -EINVAL;
	if (status != 0)
		return mtk_esw_link_mode_io_error(status);

	for (i = 0; i < rule_count; ++i)
		applied[i] = requested[i];
	return 0;
}

#endif /* MTK_ESW_LINK_MODE_POLICY_H */
