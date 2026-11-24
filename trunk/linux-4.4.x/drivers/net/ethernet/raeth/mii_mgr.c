/* Copyright  2016 MediaTek Inc.
 * Author: Carlos Huang <carlos.huang@mediatek.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <linux/ktime.h>

#include "mii_mgr.h"
#include "mii_mgr_timeout_policy.h"
#include "mii_mgr_indirect_policy.h"
#include "mii_mgr_phy_address_policy.h"
#include "raeth_mdio_cl45_address_policy.h"

void set_an_polling(u32 an_status)
{
	if (an_status == 1)
		*(unsigned long *)(ESW_PHY_POLLING) |= (1 << 31);
	else
		*(unsigned long *)(ESW_PHY_POLLING) &= ~(1 << 31);
}

static u32 __mii_mgr_read_locked(u32 phy_addr, u32 phy_register,
				 u32 *read_data)
{
	u32 status = 0;
	u32 rc = 0;
	u64 t_start_ns;
	u32 data = 0;

	t_start_ns = ktime_get_ns();
	/* We enable mdio gpio purpose register, and disable it when exit. */
	enable_mdio(1);

	/* make sure previous read operation is complete */
	while (1) {
		/* 0 : Read/write operation complete */
		if (!(sys_reg_read(MDIO_PHY_CONTROL_0) & (0x1 << 31))) {
			break;
		} else if (mii_mgr_busy_timeout_expired(t_start_ns, ktime_get_ns())) {
			enable_mdio(0);
			rc = 0;
			pr_err("\n MDIO Read operation is ongoing !!\n");
			goto out;
		}
	}

	data =
	    (0x01 << 16) | (0x02 << 18) | (phy_addr << 20) | (phy_register <<
							      25);
	sys_reg_write(MDIO_PHY_CONTROL_0, data);
	sys_reg_write(MDIO_PHY_CONTROL_0, (data | (1 << 31)));

	/* make sure read operation is complete */
	t_start_ns = ktime_get_ns();
	while (1) {
		if (!(sys_reg_read(MDIO_PHY_CONTROL_0) & (0x1 << 31))) {
			status = sys_reg_read(MDIO_PHY_CONTROL_0);
			*read_data = (u32)(status & 0x0000FFFF);

			enable_mdio(0);
			rc = 1;
			goto out;
		} else if (mii_mgr_busy_timeout_expired(t_start_ns, ktime_get_ns())) {
			enable_mdio(0);
			rc = 0;
			pr_err("\n MDIO Read operation Time Out!!\n");
			goto out;
		}
	}
out:
	return rc;
}

static u32 __mii_mgr_write_locked(u32 phy_addr, u32 phy_register,
				  u32 write_data)
{
	u64 t_start_ns;
	u32 data;
	u32 rc = 0;

	t_start_ns = ktime_get_ns();
	enable_mdio(1);

	/* make sure previous write operation is complete */
	while (1) {
		if (!(sys_reg_read(MDIO_PHY_CONTROL_0) & (0x1 << 31))) {
			break;
		} else if (mii_mgr_busy_timeout_expired(t_start_ns, ktime_get_ns())) {
			enable_mdio(0);
			rc = 0;
			pr_err("\n MDIO Write operation ongoing\n");
			goto out;
		}
	}

	data =
	    (0x01 << 16) | (1 << 18) | (phy_addr << 20) | (phy_register << 25) |
	    write_data;
	sys_reg_write(MDIO_PHY_CONTROL_0, data);
	sys_reg_write(MDIO_PHY_CONTROL_0, (data | (1 << 31))); /*start*/

	t_start_ns = ktime_get_ns();

	/* make sure write operation is complete */
	while (1) {
		if (!(sys_reg_read(MDIO_PHY_CONTROL_0) & (0x1 << 31))) {
			enable_mdio(0);
			rc = 1;
			goto out;
		} else if (mii_mgr_busy_timeout_expired(t_start_ns, ktime_get_ns())) {
			enable_mdio(0);
			rc = 0;
			pr_err("\n MDIO Write operation Time Out\n");
			goto out;
		}
	}
out:
	return rc;
}


/* Single-register callers still acquire the MDIO bus lock around one command. */
static u32 __mii_mgr_read(u32 phy_addr, u32 phy_register, u32 *read_data)
{
	struct END_DEVICE *ei_local;
	unsigned long flags;
	u32 status;

	if (!dev_raether)
		return 0;
	ei_local = netdev_priv(dev_raether);
	spin_lock_irqsave(&ei_local->mdio_lock, flags);
	status = __mii_mgr_read_locked(phy_addr, phy_register, read_data);
	spin_unlock_irqrestore(&ei_local->mdio_lock, flags);
	return status;
}

static u32 __mii_mgr_write(u32 phy_addr, u32 phy_register, u32 write_data)
{
	struct END_DEVICE *ei_local;
	unsigned long flags;
	u32 status;

	if (!dev_raether)
		return 0;
	ei_local = netdev_priv(dev_raether);
	spin_lock_irqsave(&ei_local->mdio_lock, flags);
	status = __mii_mgr_write_locked(phy_addr, phy_register, write_data);
	spin_unlock_irqrestore(&ei_local->mdio_lock, flags);
	return status;
}

static unsigned int
mii_mgr_indirect_locked_read(void *context, unsigned int phy_addr,
			     unsigned int phy_register, unsigned int *read_data)
{
	(void)context;
	return __mii_mgr_read_locked(phy_addr, phy_register, read_data);
}

static unsigned int
mii_mgr_indirect_locked_write(void *context, unsigned int phy_addr,
			      unsigned int phy_register, unsigned int write_data)
{
	(void)context;
	return __mii_mgr_write_locked(phy_addr, phy_register, write_data);
}

u32 mii_mgr_read(u32 phy_addr, u32 phy_register, u32 *read_data)
{
	struct END_DEVICE *ei_local;
	unsigned long flags;
	u32 an_status = 0;
	u32 status;

	if (!read_data)
		return 0;
	*read_data = 0;
	if (!dev_raether)
		return 0;
	if (mii_mgr_phy_address_status(phy_addr) != 0)
		return 0;
	ei_local = netdev_priv(dev_raether);

	if ((ei_local->architecture &
	     (GE1_RGMII_FORCE_1000 | GE1_TRGMII_FORCE_2000 |
	      GE1_TRGMII_FORCE_2600)) && (phy_addr == 31)) {
		/* The indirect register is a 16-bit logical address, not a 5-bit C22 reg. */
		if (mii_mgr_indirect_register_status(phy_register) != 0)
			return 0;
		/* Keep page-select and both data words indivisible to every MDIO user. */
		spin_lock_irqsave(&ei_local->mdio_lock, flags);
		an_status = (*(unsigned long *)(ESW_PHY_POLLING) & (1 << 31));
		if (an_status)
			set_an_polling(0);
		status = mii_mgr_indirect_read(
			phy_addr, phy_register, ei_local,
			mii_mgr_indirect_locked_write,
			mii_mgr_indirect_locked_read, read_data);
		if (an_status)
			set_an_polling(1);
		spin_unlock_irqrestore(&ei_local->mdio_lock, flags);
		return status;
	}

	if (mii_mgr_phy_register_status(phy_register) != 0)
		return 0;
	return __mii_mgr_read(phy_addr, phy_register, read_data);
}
EXPORT_SYMBOL(mii_mgr_read);

u32 mii_mgr_write(u32 phy_addr, u32 phy_register, u32 write_data)
{
	struct END_DEVICE *ei_local;
	unsigned long flags;
	u32 an_status = 0;
	u32 status;

	if (!dev_raether)
		return 0;
	if (mii_mgr_phy_address_status(phy_addr) != 0)
		return 0;
	ei_local = netdev_priv(dev_raether);

	if ((ei_local->architecture &
	     (GE1_RGMII_FORCE_1000 | GE1_TRGMII_FORCE_2000 |
	      GE1_TRGMII_FORCE_2600)) && (phy_addr == 31)) {
		/* This logical transaction writes two 16-bit words from one u32. */
		if (mii_mgr_indirect_register_status(phy_register) != 0)
			return 0;
		/* Page-select and both data words share one bus-lock critical section. */
		spin_lock_irqsave(&ei_local->mdio_lock, flags);
		an_status = (*(unsigned long *)(ESW_PHY_POLLING) & (1 << 31));
		if (an_status)
			set_an_polling(0);
		status = mii_mgr_indirect_write(
			phy_addr, phy_register, write_data, ei_local,
			mii_mgr_indirect_locked_write);
		if (an_status)
			set_an_polling(1);
		spin_unlock_irqrestore(&ei_local->mdio_lock, flags);
		return status;
	}

	if (mii_mgr_phy_register_status(phy_register) != 0 ||
	    mii_mgr_phy_write_data_status(write_data) != 0)
		return 0;
	return __mii_mgr_write(phy_addr, phy_register, write_data);
}
EXPORT_SYMBOL(mii_mgr_write);

u32 mii_mgr_cl45_set_address(u32 port_num, u32 dev_addr, u32 reg_addr)
{
	u32 rc = 0;
	u64 t_start_ns;
	u32 data = 0;

	/* Validate even internal callers before composing the MDIO command word. */
	if (raeth_mdio_cl45_address_status(port_num, dev_addr, reg_addr) != 0)
		return rc;

	t_start_ns = ktime_get_ns();
	enable_mdio(1);

	while (1) {
		if (!(sys_reg_read(MDIO_PHY_CONTROL_0) & (0x1 << 31))) {
			break;
		} else if (mii_mgr_busy_timeout_expired(t_start_ns, ktime_get_ns())) {
			enable_mdio(0);
			pr_err("\n MDIO Read operation is ongoing !!\n");
			return rc;
		}
	}
	data =
	    (dev_addr << 25) | (port_num << 20) | (0x00 << 18) | (0x00 << 16) |
	    reg_addr;
	sys_reg_write(MDIO_PHY_CONTROL_0, data);
	sys_reg_write(MDIO_PHY_CONTROL_0, (data | (1 << 31)));

	t_start_ns = ktime_get_ns();
	while (1) {
		if (!(sys_reg_read(MDIO_PHY_CONTROL_0) & (0x1 << 31))) {
			enable_mdio(0);
			return 1;
		} else if (mii_mgr_busy_timeout_expired(t_start_ns, ktime_get_ns())) {
			enable_mdio(0);
			pr_err("\n MDIO Write operation Time Out\n");
			return 0;
		}
	}
}

u32 mii_mgr_read_cl45(u32 port_num, u32 dev_addr, u32 reg_addr, u32 *read_data)
{
	u32 rc = 0;
	u64 t_start_ns;
	u32 data = 0;
	struct END_DEVICE *ei_local;
	unsigned long flags;

	if (!read_data)
		return 0;
	*read_data = 0;
	if (!dev_raether)
		return 0;
	ei_local = netdev_priv(dev_raether);

	spin_lock_irqsave(&ei_local->mdio_lock, flags);
	/* Do not issue a data phase when address setup timed out. */
	if (!mii_mgr_cl45_set_address(port_num, dev_addr, reg_addr))
		goto out;
	t_start_ns = ktime_get_ns();

	enable_mdio(1);

	while (1) {
		if (!(sys_reg_read(MDIO_PHY_CONTROL_0) & (0x1 << 31))) {
			break;
		} else if (mii_mgr_busy_timeout_expired(t_start_ns, ktime_get_ns())) {
			enable_mdio(0);
			rc = 0;
			pr_err("\n MDIO Read operation is ongoing !!\n");
			goto out;
		}
	}
	data =
	    (dev_addr << 25) | (port_num << 20) | (0x03 << 18) | (0x00 << 16) |
	    reg_addr;
	sys_reg_write(MDIO_PHY_CONTROL_0, data);
	sys_reg_write(MDIO_PHY_CONTROL_0, (data | (1 << 31)));
	t_start_ns = ktime_get_ns();
	while (1) {
		if (!(sys_reg_read(MDIO_PHY_CONTROL_0) & (0x1 << 31))) {
			*read_data =
			    (sys_reg_read(MDIO_PHY_CONTROL_0) & 0x0000FFFF);
			enable_mdio(0);
			rc = 1;
			goto out;
		} else if (mii_mgr_busy_timeout_expired(t_start_ns, ktime_get_ns())) {
			enable_mdio(0);
			rc = 0;
			pr_err
			    ("\n MDIO Read operation Time Out!!\n");
			goto out;
		}
	}
out:
	spin_unlock_irqrestore(&ei_local->mdio_lock, flags);
	return rc;
}

u32 mii_mgr_write_cl45(u32 port_num, u32 dev_addr, u32 reg_addr, u32 write_data)
{
	u32 rc = 0;
	u64 t_start_ns;
	u32 data = 0;
	struct END_DEVICE *ei_local;
	unsigned long flags;

	/* The Clause 45 data phase accepts only a 16-bit payload. */
	if (raeth_mdio_cl45_write_data_status(write_data) != 0)
		return rc;
	if (!dev_raether)
		return 0;
	ei_local = netdev_priv(dev_raether);

	spin_lock_irqsave(&ei_local->mdio_lock, flags);
	/* Do not issue a data phase when address setup timed out. */
	if (!mii_mgr_cl45_set_address(port_num, dev_addr, reg_addr))
		goto out;
	t_start_ns = ktime_get_ns();

	enable_mdio(1);
	while (1) {
		if (!(sys_reg_read(MDIO_PHY_CONTROL_0) & (0x1 << 31))) {
			break;
		} else if (mii_mgr_busy_timeout_expired(t_start_ns, ktime_get_ns())) {
			enable_mdio(0);
			rc = 0;
			pr_err("\n MDIO Read operation is ongoing !!\n");
			goto out;
		}
	}

	data =
	    (dev_addr << 25) | (port_num << 20) | (0x01 << 18) | (0x00 << 16) |
	    write_data;
	sys_reg_write(MDIO_PHY_CONTROL_0, data);
	sys_reg_write(MDIO_PHY_CONTROL_0, (data | (1 << 31)));

	t_start_ns = ktime_get_ns();

	while (1) {
		if (!(sys_reg_read(MDIO_PHY_CONTROL_0) & (0x1 << 31))) {
			enable_mdio(0);
			rc = 1;
			goto out;
		} else if (mii_mgr_busy_timeout_expired(t_start_ns, ktime_get_ns())) {
			enable_mdio(0);
			rc = 0;
			pr_err("\n MDIO Write operation Time Out\n");
			goto out;
		}
	}
out:
	spin_unlock_irqrestore(&ei_local->mdio_lock, flags);
	return rc;
}
