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
#include "raether.h"
#include "ra_switch.h"
#include "ra_switch_mdio_policy.h"
#include "ra_mac.h"
#include "raeth_reg.h"
#include <linux/spinlock.h>

static DEFINE_SPINLOCK(ra_switch_phy_page_lock);

static int all_ge_ana_cal_wait_checked(unsigned int delay, u8 port_num,
				       u8 *ready);

#define MT7622_CHIP_ID 0x08000008

void reg_bit_zero(void __iomem *addr, unsigned int bit, unsigned int len)
{
	int reg_val;
	int i;

	reg_val = sys_reg_read(addr);
	for (i = 0; i < len; i++)
		reg_val &= ~(1 << (bit + i));
	sys_reg_write(addr, reg_val);
}

void reg_bit_one(void __iomem *addr, unsigned int bit, unsigned int len)
{
	unsigned int reg_val;
	unsigned int i;

	reg_val = sys_reg_read(addr);
	for (i = 0; i < len; i++)
		reg_val |= 1 << (bit + i);
	sys_reg_write(addr, reg_val);
}

u8 fe_cal_flag;
u8 fe_cal_flag_mdix;
u8 fe_cal_tx_offset_flag;
u8 fe_cal_tx_offset_flag_mdix;
u8 fe_cal_r50_flag;
u8 fe_cal_vbg_flag;
u32 iext_cal_result;
u32 r50_p0_cal_result;
u8 ge_cal_r50_flag;
u8 ge_cal_tx_offset_flag;
u8 ge_cal_flag;
int show_time;
static u8 ephy_addr_base;

/* 50ohm_new*/
const u8 ZCAL_TO_R50OHM_TBL_100[64] = {
	127, 121, 116, 115, 111, 109, 108, 104,
	102, 99, 97, 96, 77, 76, 73, 72,
	70, 69, 67, 66, 47, 46, 45, 43,
	42, 41, 40, 38, 37, 36, 35, 34,
	32, 16, 15, 14, 13, 12, 11, 10,
	9, 8, 7, 6, 6, 5, 4, 4,
	3, 2, 2, 1, 1, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0
};

const u8 ZCAL_TO_R50ohm_GE_TBL_100[64] = {
	63, 63, 63, 63, 63, 63, 63, 63,
	63, 63, 63, 63, 63, 63, 63, 60,
	57, 55, 53, 51, 48, 46, 44, 42,
	40, 38, 37, 36, 34, 32, 30, 28,
	27, 26, 25, 23, 22, 21, 19, 18,
	16, 15, 14, 13, 12, 11, 10, 9,
	8, 7, 6, 5, 4, 4, 3, 2,
	1, 0, 0, 0, 0, 0, 0, 0
};

const u8 ZCAL_TO_R50ohm_GE_TBL[64] = {
	63, 63, 63, 63, 63, 63, 63, 63,
	63, 63, 63, 63, 63, 63, 63, 60,
	57, 55, 53, 51, 48, 46, 44, 42,
	40, 38, 37, 36, 34, 32, 30, 28,
	27, 26, 25, 23, 22, 21, 19, 18,
	16, 15, 14, 13, 12, 11, 10, 9,
	8, 7, 6, 5, 4, 4, 3, 2,
	1, 0, 0, 0, 0, 0, 0, 0
};

const u8 ZCAL_TO_REXT_TBL[64] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 1, 1, 1, 1, 1,
	1, 2, 2, 2, 2, 2, 2, 3,
	3, 3, 3, 3, 3, 4, 4, 4,
	4, 4, 4, 4, 5, 5, 5, 5,
	5, 5, 6, 6, 6, 6, 6, 6,
	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7
};

const u8 ZCAL_TO_FILTER_TBL[64] = {
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 1, 1,
	1, 2, 2, 2, 3, 3, 3, 4,
	4, 4, 4, 5, 5, 5, 6, 6,
	7, 7, 7, 8, 8, 8, 9, 9,
	9, 10, 10, 10, 11, 11, 11, 11,
	12, 12, 12, 12, 12, 12, 12, 12
};

static int ra_switch_mdio_read(void *context, unsigned int port,
				 unsigned int reg, unsigned int *value)
{
	(void)context;
	return mii_mgr_read(port, reg, value) ? 0 : -EIO;
}

static int ra_switch_mdio_write(void *context, unsigned int port,
				  unsigned int reg, unsigned int value)
{
	(void)context;
	return mii_mgr_write(port, reg, value) ? 0 : -EIO;
}

static int ra_switch_mdio_read_cl45(void *context, unsigned int port,
				    unsigned int dev, unsigned int reg,
				    unsigned int *value)
{
	(void)context;
	return mii_mgr_read_cl45(port, dev, reg, value) ? 0 : -EIO;
}

static int ra_switch_mdio_write_cl45(unsigned int port,
					   unsigned int dev, unsigned int reg,
					   unsigned int value)
{
	return mii_mgr_write_cl45(port, dev, reg, value) ? 0 : -EIO;
}

static int tc_phy_read_page_reg_checked(u8 port, u8 page, u8 reg,
					int local, u32 *value)
{
	unsigned long flags;
	int status;

	spin_lock_irqsave(&ra_switch_phy_page_lock, flags);
	status = ra_switch_mdio_page_read(local, port, page, reg,
					ra_switch_mdio_read,
					ra_switch_mdio_write, NULL, value);
	spin_unlock_irqrestore(&ra_switch_phy_page_lock, flags);
	return status;
}

static int tc_phy_write_page_reg_checked(u8 port, u8 page, u8 reg,
					 int local, u32 value)
{
	unsigned long flags;
	int status;

	spin_lock_irqsave(&ra_switch_phy_page_lock, flags);
	status = ra_switch_mdio_page_write(local, port, page, reg, value,
					 ra_switch_mdio_write, NULL);
	spin_unlock_irqrestore(&ra_switch_phy_page_lock, flags);
	return status;
}

void tc_phy_write_g_reg(u8 port_num, u8 page_num,
			u8 reg_num, u32 reg_data)
{
	int status = tc_phy_write_page_reg_checked(port_num, page_num, reg_num,
						   0, reg_data);

	if (status != 0)
		pr_err_ratelimited("%s: global PHY page write failed (port=%u page=%u reg=%u status=%d)\n",
				    __func__, port_num, page_num, reg_num, status);
}

void tc_phy_write_l_reg(u8 port_no, u8 page_no,
			u8 reg_num, u32 reg_data)
{
	int status = tc_phy_write_page_reg_checked(port_no, page_no, reg_num,
						   1, reg_data);

	if (status != 0)
		pr_err_ratelimited("%s: local PHY page write failed (port=%u page=%u reg=%u status=%d)\n",
				    __func__, port_no, page_no, reg_num, status);
}

u32 tc_phy_read_g_reg(u8 port_num, u8 page_num, u8 reg_num)
{
	u32 phy_val = 0;
	int status;

	status = tc_phy_read_page_reg_checked(port_num, page_num, reg_num,
					      0, &phy_val);
	if (status != 0) {
		pr_err_ratelimited("%s: global PHY page read failed (port=%u page=%u reg=%u status=%d)\n",
				    __func__, port_num, page_num, reg_num, status);
		return 0;
	}
	return phy_val;
}

u32 tc_phy_read_l_reg(u8 port_no, u8 page_no, u8 reg_num)
{
	u32 phy_val = 0;
	int status;

	status = tc_phy_read_page_reg_checked(port_no, page_no, reg_num,
					      1, &phy_val);
	if (status != 0) {
		pr_err_ratelimited("%s: local PHY page read failed (port=%u page=%u reg=%u status=%d)\n",
				    __func__, port_no, page_no, reg_num, status);
		return 0;
	}
	return phy_val;
}

static int ra_switch_mdio_write_cl45_cb(void *context,
					 unsigned int port, unsigned int dev,
					 unsigned int reg, unsigned int value)
{
	(void)context;
	return ra_switch_mdio_write_cl45(port, dev, reg, value);
}

static int tc_phy_read_dev_reg_checked(u32 port_num, u32 dev_addr,
				       u32 reg_addr, u32 *value)
{
	return ra_switch_mdio_read_cl45_checked(port_num, dev_addr, reg_addr,
						ra_switch_mdio_read_cl45,
						NULL, value);
}

static int tc_phy_write_dev_reg_checked(u32 port_num, u32 dev_addr,
					u32 reg_addr, u32 write_data)
{
	return ra_switch_mdio_write_cl45_checked(port_num, dev_addr, reg_addr,
						  write_data,
						  ra_switch_mdio_write_cl45_cb,
						  NULL);
}

static int tc_mii_read_checked(u32 phy_addr, u32 phy_register, u32 *value)
{
	return ra_switch_mdio_read_checked(phy_addr, phy_register,
					   ra_switch_mdio_read, NULL, value);
}

static int tc_mii_write_checked(u32 phy_addr, u32 phy_register, u32 value)
{
	return ra_switch_mdio_write_checked(phy_addr, phy_register, value,
					    ra_switch_mdio_write, NULL);
}

/* Preserve the legacy value-returning ABI; failed reads now report and return zero. */
u32 tc_phy_read_dev_reg(u32 port_num, u32 dev_addr, u32 reg_addr)
{
	u32 phy_val = 0;
	int status;

	status = tc_phy_read_dev_reg_checked(port_num, dev_addr, reg_addr,
					     &phy_val);
	if (status != 0)
		pr_err_ratelimited("%s: Clause 45 PHY read failed (port=%u dev=%u reg=0x%x status=%d)\n",
				   __func__, port_num, dev_addr, reg_addr, status);
	return phy_val;
}

void tc_phy_write_dev_reg(u32 port_num, u32 dev_addr, u32 reg_addr, u32 write_data)
{
	int status = tc_phy_write_dev_reg_checked(port_num, dev_addr, reg_addr,
						  write_data);

	if (status != 0)
		pr_err_ratelimited("%s: Clause 45 PHY write failed (port=%u dev=%u reg=0x%x status=%d)\n",
				   __func__, port_num, dev_addr, reg_addr, status);
}

u32 tc_mii_read(u32 phy_addr, u32 phy_register)
{
	u32 phy_val = 0;
	int status;

	status = tc_mii_read_checked(phy_addr, phy_register, &phy_val);
	if (status != 0)
		pr_err_ratelimited("%s: Clause 22 PHY read failed (port=%u reg=0x%x status=%d)\n",
				   __func__, phy_addr, phy_register, status);
	return phy_val;
}

void tc_mii_write(u32 phy_addr, u32 phy_register, u32 write_data)
{
	int status = tc_mii_write_checked(phy_addr, phy_register, write_data);

	if (status != 0)
		pr_err_ratelimited("%s: Clause 22 PHY write failed (port=%u reg=0x%x status=%d)\n",
				   __func__, phy_addr, phy_register, status);
}

struct ra_switch_page_update_context {
	unsigned int page;
	int local;
};

static int ra_switch_page_update_read(void *context, unsigned int port,
				      unsigned int reg, unsigned int *value)
{
	struct ra_switch_page_update_context *page = context;

	return tc_phy_read_page_reg_checked(port, (u8)page->page,
					    (u8)reg, page->local, value);
}

static int ra_switch_page_update_write(void *context, unsigned int port,
				       unsigned int reg, unsigned int value)
{
	struct ra_switch_page_update_context *page = context;

	return tc_phy_write_page_reg_checked(port, (u8)page->page,
					     (u8)reg, page->local, value);
}

static int clear_ckinv_ana_txvos_checked(void)
{
	struct ra_switch_page_update_context page;
	unsigned int updated;
	int status = 0;
	int cleanup_status;

	page.page = 7U;
	page.local = 0;

	/* RG_ANA_CALEN_P0, RG_CAL_CKINV_P0, then DA_TXVOS_CALEN_P0. */
	status = ra_switch_mdio_update_bits_checked(
		FE_CAL_COMMON, 24U, 0x2000U, 0U, ra_switch_page_update_read,
		ra_switch_page_update_write, &page, &updated);
	if (status != 0)
		goto cleanup;
	status = ra_switch_mdio_update_bits_checked(
		FE_CAL_COMMON, 24U, 0x4000U, 0U, ra_switch_page_update_read,
		ra_switch_page_update_write, &page, &updated);
	if (status != 0)
		goto cleanup;
	status = ra_switch_mdio_update_bits_checked(
		FE_CAL_COMMON, 24U, 0x1000U, 0U, ra_switch_page_update_read,
		ra_switch_page_update_write, &page, &updated);

cleanup:
	/* Preserve the original final zero write even after an earlier failure. */
	cleanup_status = tc_phy_write_page_reg_checked(
		FE_CAL_COMMON, 7U, 24U, 0, 0U);
	if (status == 0 && cleanup_status != 0)
		status = cleanup_status;
	return status;
}

void clear_ckinv_ana_txvos(void)
{
	int status = clear_ckinv_ana_txvos_checked();

	if (status != 0)
		pr_err_ratelimited("%s: analog calibration cleanup failed (status=%d)\n",
				   __func__, status);
}
static int all_fe_ana_cal_wait_checked(u32 delay, u8 port_num,
				      int reset_port_before, u8 *ready)
{
	struct ra_switch_page_update_context page;
	int status = 0;
	int cleanup_status;

	if (!ready)
		return -EINVAL;
	*ready = 0;
	page.page = 4U;
	page.local = 1;

	/* Preserve the original FE_CAL_COMMON reset before the trigger. */
	status = ra_switch_mdio_write_checked(
		FE_CAL_COMMON, 23U, 0U, ra_switch_page_update_write, &page);
	if (status != 0)
		goto cleanup;

	if (reset_port_before) {
		status = ra_switch_mdio_write_checked(
			port_num, 23U, 0U, ra_switch_page_update_write, &page);
		if (status != 0)
			goto cleanup;
	}

	status = all_ge_ana_cal_wait_checked(delay, port_num, ready);

cleanup:
	/* Attempt both original page-4/register-23 cleanup writes on every exit. */
	cleanup_status = ra_switch_mdio_cleanup_two_writes(
		FE_CAL_COMMON, 23U, 0U, port_num, 23U, 0U,
		ra_switch_page_update_write, &page);
	if (status == 0 && cleanup_status != 0)
		status = cleanup_status;

	if (status != 0)
		*ready = 0;
	return status;
}

u8 all_fe_ana_cal_wait_txamp(u32 delay, u8 port_num)
{
	u8 ready = 0;
	int status;

	/* TX-amp historically pre-clears FE_CAL_COMMON only. */
	status = all_fe_ana_cal_wait_checked(delay, port_num, 0, &ready);
	if (status != 0) {
		pr_err_ratelimited("%s: FE TX-amp calibration wait failed (port=%u status=%d)\n",
				   __func__, port_num, status);
		return ANACAL_ERROR;
	}
	return ready;
}

u8 all_fe_ana_cal_wait(u32 delay, u8 port_num)
{
	u8 ready = 0;
	int status;

	status = all_fe_ana_cal_wait_checked(delay, port_num, 1, &ready);
	if (status != 0) {
		pr_err_ratelimited("%s: FE analog calibration wait failed (port=%u status=%d)\n",
				   __func__, port_num, status);
		return ANACAL_ERROR;
	}
	return ready;
}
void fe_cal_tx_amp(u8 port_num, u32 delay)
{
	unsigned int read_value = 0U, page_value = 0U, updated;
	unsigned int l2r23_temp = 0U, tx_amp_final;
	unsigned int correction;
	int ad_cal_comp_out_init;
	int calibration_polarity;
	int tx_amp_temp = 0x20;
	int all_ana_cal_status = ANACAL_INIT;
	u8 ready = 0, cnt = 0;
	int status = 0, cleanup_status;
	int calibration_completed = 0;
	struct ra_switch_page_update_context page;
	struct END_DEVICE *ei_local = netdev_priv(dev_raether);

	fe_cal_flag = 0;
	if (port_num >= 5U) {
		pr_err_ratelimited("%s: invalid FE port %u\n",
				   __func__, port_num);
		return;
	}

	/* Set device in 100M mode and configure the TXG output path. */
	status = tc_phy_write_page_reg_checked(
		port_num, 0U, 0U, 1, 0x2100U);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_page_reg_checked(
		port_num, 2U, 25U, 0, 0x10c0U);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_page_reg_checked(
		port_num, 1U, 26U, 0, (0x8000U | DAC_IN_2V));
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_page_reg_checked(
		port_num, 4U, 21U, 0, 0x0800U);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_page_reg_checked(
		port_num, 0U, 30U, 1, 0x02c0U);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_page_reg_checked(
		port_num, 4U, 21U, 1, 0U);
	if (status != 0)
		goto cleanup;

	status = tc_phy_write_page_reg_checked(
		FE_CAL_COMMON, 3U, 25U, 1, 0xc800U);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_page_reg_checked(
		port_num, 3U, 25U, 1, 0xc800U);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_page_reg_checked(
		FE_CAL_COMMON, 7U, 24U, 0, 0x7000U);
	if (status != 0)
		goto cleanup;

	/* Enable TXG/ZCAL and disable the opposing calibration bit. */
	page.page = 3U;
	page.local = 1;
	status = ra_switch_mdio_update_bits_checked(
		port_num, 25U, 0U, 0x0400U,
		ra_switch_page_update_read, ra_switch_page_update_write,
		&page, &updated);
	if (status != 0)
		goto cleanup;
	status = ra_switch_mdio_update_bits_checked(
		port_num, 25U, 0x0200U, 0x1000U,
		ra_switch_page_update_read, ra_switch_page_update_write,
		&page, &updated);
	if (status != 0)
		goto cleanup;

	/* Preserve the original fixed PGA/MDIX and RX-to-TX settings. */
	status = tc_phy_write_page_reg_checked(
		port_num, 0U, 26U, 1, 0x5203U);
	if (status != 0)
		goto cleanup;
	status = tc_phy_read_page_reg_checked(
		port_num, 2U, 20U, 1, &read_value);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_page_reg_checked(
		port_num, 2U, 20U, 1, (read_value & ~0x0400U));
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_page_reg_checked(
		port_num, 2U, 23U, 1, (unsigned int)tx_amp_temp);
	if (status != 0)
		goto cleanup;

	/* TX-amp retains its original common-only pre-trigger reset mode. */
	ready = 0;
	status = all_fe_ana_cal_wait_checked(delay, port_num, 0, &ready);
	if (status != 0)
		goto cleanup;
	if (!ready) {
		all_ana_cal_status = ANACAL_ERROR;
		pr_info("FE TX-amplitude calibration did not become ready (init)\n");
	} else {
		status = tc_phy_read_page_reg_checked(
			FE_CAL_COMMON, 7U, 24U, 0, &page_value);
		if (status != 0)
			goto cleanup;
		status = tc_phy_read_page_reg_checked(
			FE_CAL_COMMON, 7U, 24U, 0, &page_value);
		if (status != 0)
			goto cleanup;
		ad_cal_comp_out_init = (int)(page_value & 0x1U);
		calibration_polarity = ad_cal_comp_out_init ? -1 : 1;
		tx_amp_temp = (int)ra_switch_mdio_calibration_code_step(
			(unsigned int)tx_amp_temp, calibration_polarity);

		while ((all_ana_cal_status < ANACAL_ERROR) && (cnt < 254U)) {
			++cnt;
			status = tc_phy_write_page_reg_checked(
				port_num, 2U, 23U, 1,
				(unsigned int)tx_amp_temp);
			if (status != 0)
				goto cleanup;
			status = tc_phy_read_page_reg_checked(
				port_num, 2U, 23U, 1, &l2r23_temp);
			if (status != 0)
				goto cleanup;

			/* Preserve comparator settling reads before each wait. */
			status = tc_phy_read_page_reg_checked(
				FE_CAL_COMMON, 7U, 24U, 0, &page_value);
			if (status != 0)
				goto cleanup;
			status = tc_phy_read_page_reg_checked(
				FE_CAL_COMMON, 7U, 24U, 0, &page_value);
			if (status != 0)
				goto cleanup;

			ready = 0;
			status = all_fe_ana_cal_wait_checked(
				delay, port_num, 0, &ready);
			if (status != 0)
				goto cleanup;
			if (!ready) {
				all_ana_cal_status = ANACAL_ERROR;
				pr_info("FE TX-amplitude calibration timed out (step=%u)\n",
					cnt);
				break;
			}

			status = tc_phy_read_page_reg_checked(
				FE_CAL_COMMON, 7U, 24U, 0, &page_value);
			if (status != 0)
				goto cleanup;
			if ((page_value & 0x1U) !=
			    (unsigned int)ad_cal_comp_out_init) {
				all_ana_cal_status = ANACAL_FINISH;
				/* Keep the original post-crossing settling sequence. */
				status = tc_phy_read_page_reg_checked(
					FE_CAL_COMMON, 7U, 24U, 0, &page_value);
				if (status != 0)
					goto cleanup;
				status = tc_phy_read_page_reg_checked(
					FE_CAL_COMMON, 7U, 24U, 0, &page_value);
				if (status != 0)
					goto cleanup;
				status = tc_phy_read_page_reg_checked(
					FE_CAL_COMMON, 7U, 24U, 0, &page_value);
				if (status != 0)
					goto cleanup;
				ad_cal_comp_out_init = (int)(page_value & 0x1U);
			} else if ((l2r23_temp == 0x3fU) ||
				   (l2r23_temp == 0U)) {
				all_ana_cal_status = ANACAL_SATURATION;
				pr_info("FE TX-amplitude calibration saturated (step=%u)\n",
					cnt);
			} else {
				tx_amp_temp = (int)ra_switch_mdio_calibration_code_step(
					(unsigned int)tx_amp_temp,
					calibration_polarity);
			}
		}
		if ((all_ana_cal_status == ANACAL_INIT) && (cnt >= 254U)) {
			all_ana_cal_status = ANACAL_ERROR;
			pr_info("FE TX-amplitude calibration exhausted its search range\n");
		}
	}

	/* Retain the last bounded code on timeout/saturation, as before. */
	if ((all_ana_cal_status == ANACAL_ERROR) ||
	    (all_ana_cal_status == ANACAL_SATURATION)) {
		status = tc_phy_write_page_reg_checked(
			port_num, 2U, 23U, 1, (unsigned int)tx_amp_temp);
		if (status != 0)
			goto cleanup;
		status = tc_phy_read_page_reg_checked(
			port_num, 2U, 23U, 1, &l2r23_temp);
		if (status != 0)
			goto cleanup;
		pr_info("[%d] %s, ANACAL_SATURATION\n", port_num, __func__);
	} else {
		/* Preserve the existing per-chip and per-port final adjustments. */
		if (ei_local->chip_name == MT7622_FE) {
			if (port_num == 0U)
				l2r23_temp += 10U;
			else if (port_num == 1U)
				l2r23_temp += 11U;
			else if (port_num == 2U)
				l2r23_temp += 10U;
			else if (port_num == 3U)
				l2r23_temp += 9U;
			else if (port_num == 4U)
				l2r23_temp += 10U;
		} else if (ei_local->chip_name == LEOPARD_FE) {
			if (port_num == 1U)
				l2r23_temp += 3U;
			else if (port_num == 2U)
				l2r23_temp += 3U;
			else if (port_num == 3U)
				l2r23_temp += 1U;
			else if (port_num == 4U)
				l2r23_temp += 3U;
		}
		status = tc_phy_write_page_reg_checked(
			port_num, 2U, 23U, 1, l2r23_temp);
		if (status != 0)
			goto cleanup;
		calibration_completed = 1;
	}

	/* Read back the final code and program the corresponding correction. */
	status = tc_phy_read_page_reg_checked(
		port_num, 2U, 23U, 1, &read_value);
	if (status != 0)
		goto cleanup;
	tx_amp_final = read_value & 0x3fU;
	correction = ((tx_amp_final + 15U) << 8) | 0x20U;
	status = tc_phy_write_page_reg_checked(
		port_num, 2U, 24U, 1, correction);
	if (status != 0)
		goto cleanup;

	if (ei_local->chip_name == LEOPARD_FE) {
		if (port_num == 1U)
			correction = ((tx_amp_final + 15U - 4U) << 8) | 0x20U;
		else if (port_num == 2U)
			correction = ((tx_amp_final + 15U + 2U) << 8) | 0x20U;
		else if ((port_num == 3U) || (port_num == 4U))
			correction = ((tx_amp_final + 15U + 4U) << 8) | 0x20U;
		else
			correction = 0U;
		if (correction != 0U) {
			status = tc_phy_write_page_reg_checked(
				port_num, 2U, 24U, 1, correction);
			if (status != 0)
				goto cleanup;
		}
	}
	pr_info("[%d] - tx_amp_final = 0x%x\n", port_num, tx_amp_final);

cleanup:
	/* Attempt every independent analog reset and keep the first error. */
	cleanup_status = clear_ckinv_ana_txvos_checked();
	if (status == 0 && cleanup_status != 0)
		status = cleanup_status;

	cleanup_status = tc_phy_write_page_reg_checked(
		port_num, 3U, 25U, 1, 0U);
	if (status == 0 && cleanup_status != 0)
		status = cleanup_status;

	cleanup_status = tc_phy_write_page_reg_checked(
		FE_CAL_COMMON, 3U, 25U, 1, 0U);
	if (status == 0 && cleanup_status != 0)
		status = cleanup_status;

	cleanup_status = tc_phy_write_page_reg_checked(
		port_num, 1U, 26U, 0, 0U);
	if (status == 0 && cleanup_status != 0)
		status = cleanup_status;

	if (status != 0) {
		pr_err_ratelimited(
			"%s: FE TX-amplitude calibration failed (port=%u status=%d)\n",
			__func__, port_num, status);
		return;
	}
	if (calibration_completed)
		fe_cal_flag = 1;
}

void fe_cal_tx_amp_mdix(u8 port_num, u32 delay)
{
	unsigned int read_value = 0U, page_value = 0U, updated;
	unsigned int l4r26_temp_amp = 0U, tx_amp_mdix_final;
	unsigned int correction;
	int ad_cal_comp_out_init;
	int calibration_polarity;
	int tx_amp_temp = 0x20;
	int all_ana_cal_status = ANACAL_INIT;
	u8 ready = 0, cnt = 0;
	int status = 0, cleanup_status;
	int calibration_completed = 0;
	struct ra_switch_page_update_context page;
	struct END_DEVICE *ei_local = netdev_priv(dev_raether);

	fe_cal_flag_mdix = 0;
	if (port_num >= 5U) {
		pr_err_ratelimited("%s: invalid FE port %u\n",
				   __func__, port_num);
		return;
	}

	status = tc_phy_write_page_reg_checked(
		port_num, 0U, 0U, 1, 0x2100U);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_page_reg_checked(
		port_num, 2U, 25U, 0, 0x10c0U);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_page_reg_checked(
		port_num, 1U, 26U, 0, (0x8000U | DAC_IN_2V));
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_page_reg_checked(
		port_num, 4U, 21U, 0, 0x0800U);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_page_reg_checked(
		port_num, 0U, 30U, 1, 0x02c0U);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_page_reg_checked(
		port_num, 4U, 21U, 1, 0U);
	if (status != 0)
		goto cleanup;

	status = tc_phy_write_page_reg_checked(
		FE_CAL_COMMON, 3U, 25U, 1, 0xc800U);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_page_reg_checked(
		port_num, 3U, 25U, 1, 0xc800U);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_page_reg_checked(
		FE_CAL_COMMON, 7U, 24U, 0, 0x7000U);
	if (status != 0)
		goto cleanup;

	page.page = 3U;
	page.local = 1;
	status = ra_switch_mdio_update_bits_checked(
		port_num, 25U, 0U, 0x0400U,
		ra_switch_page_update_read, ra_switch_page_update_write,
		&page, &updated);
	if (status != 0)
		goto cleanup;
	status = ra_switch_mdio_update_bits_checked(
		port_num, 25U, 0x0200U, 0x1000U,
		ra_switch_page_update_read, ra_switch_page_update_write,
		&page, &updated);
	if (status != 0)
		goto cleanup;

	status = tc_phy_write_page_reg_checked(
		port_num, 0U, 26U, 1, 0x9203U);
	if (status != 0)
		goto cleanup;
	page.page = 2U;
	status = ra_switch_mdio_update_bits_checked(
		port_num, 20U, 0U, 0x0400U,
		ra_switch_page_update_read, ra_switch_page_update_write,
		&page, &updated);
	if (status != 0)
		goto cleanup;

	/* MDIX TX-amplitude code is carried in local page 4/register 26 [5:0]. */
	page.page = 4U;
	status = ra_switch_mdio_update_bits_checked(
		port_num, 26U, 0x003fU, (unsigned int)tx_amp_temp,
		ra_switch_page_update_read, ra_switch_page_update_write,
		&page, &updated);
	if (status != 0)
		goto cleanup;

	/* TX-amp MDIX shares the historical common-only pre-trigger wait. */
	ready = 0;
	status = all_fe_ana_cal_wait_checked(delay, port_num, 0, &ready);
	if (status != 0)
		goto cleanup;
	if (!ready) {
		all_ana_cal_status = ANACAL_ERROR;
		pr_info("FE TX-amplitude MDIX calibration did not become ready (init)\n");
	} else {
		status = tc_phy_read_page_reg_checked(
			FE_CAL_COMMON, 7U, 24U, 0, &page_value);
		if (status != 0)
			goto cleanup;
		status = tc_phy_read_page_reg_checked(
			FE_CAL_COMMON, 7U, 24U, 0, &page_value);
		if (status != 0)
			goto cleanup;
		status = tc_phy_read_page_reg_checked(
			FE_CAL_COMMON, 7U, 24U, 0, &page_value);
		if (status != 0)
			goto cleanup;
		ad_cal_comp_out_init = (int)(page_value & 0x1U);
		calibration_polarity = ad_cal_comp_out_init ? -1 : 1;
		tx_amp_temp = (int)ra_switch_mdio_calibration_code_step(
			(unsigned int)tx_amp_temp, calibration_polarity);

		while ((all_ana_cal_status < ANACAL_ERROR) && (cnt < 254U)) {
			++cnt;
			page.page = 4U;
			status = ra_switch_mdio_update_bits_checked(
				port_num, 26U, 0x003fU,
				(unsigned int)tx_amp_temp,
				ra_switch_page_update_read,
				ra_switch_page_update_write, &page, &updated);
			if (status != 0)
				goto cleanup;
			status = tc_phy_read_page_reg_checked(
				port_num, 4U, 26U, 1, &read_value);
			if (status != 0)
				goto cleanup;
			l4r26_temp_amp = read_value & 0x3fU;

			status = tc_phy_read_page_reg_checked(
				FE_CAL_COMMON, 7U, 24U, 0, &page_value);
			if (status != 0)
				goto cleanup;
			status = tc_phy_read_page_reg_checked(
				FE_CAL_COMMON, 7U, 24U, 0, &page_value);
			if (status != 0)
				goto cleanup;

			ready = 0;
			status = all_fe_ana_cal_wait_checked(
				delay, port_num, 0, &ready);
			if (status != 0)
				goto cleanup;
			if (!ready) {
				all_ana_cal_status = ANACAL_ERROR;
				pr_info("FE TX-amplitude MDIX calibration timed out (step=%u)\n",
					cnt);
				break;
			}

			status = tc_phy_read_page_reg_checked(
				FE_CAL_COMMON, 7U, 24U, 0, &page_value);
			if (status != 0)
				goto cleanup;
			if ((page_value & 0x1U) !=
			    (unsigned int)ad_cal_comp_out_init) {
				all_ana_cal_status = ANACAL_FINISH;
				status = tc_phy_read_page_reg_checked(
					FE_CAL_COMMON, 7U, 24U, 0, &page_value);
				if (status != 0)
					goto cleanup;
				status = tc_phy_read_page_reg_checked(
					FE_CAL_COMMON, 7U, 24U, 0, &page_value);
				if (status != 0)
					goto cleanup;
				status = tc_phy_read_page_reg_checked(
					FE_CAL_COMMON, 7U, 24U, 0, &page_value);
				if (status != 0)
					goto cleanup;
				ad_cal_comp_out_init = (int)(page_value & 0x1U);
			} else if ((l4r26_temp_amp == 0x3fU) ||
				   (l4r26_temp_amp == 0U)) {
				all_ana_cal_status = ANACAL_SATURATION;
				pr_info("FE TX-amplitude MDIX calibration saturated\n");
			} else {
				tx_amp_temp = (int)ra_switch_mdio_calibration_code_step(
					(unsigned int)tx_amp_temp,
					calibration_polarity);
			}
		}
		if ((all_ana_cal_status == ANACAL_INIT) && (cnt >= 254U)) {
			all_ana_cal_status = ANACAL_ERROR;
			pr_info("FE TX-amplitude MDIX calibration exhausted its search range\n");
		}
	}

	if ((all_ana_cal_status == ANACAL_ERROR) ||
	    (all_ana_cal_status == ANACAL_SATURATION)) {
		/* Preserve the last bounded code as the original fallback. */
		page.page = 4U;
		status = ra_switch_mdio_update_bits_checked(
			port_num, 26U, 0x003fU, (unsigned int)tx_amp_temp,
			ra_switch_page_update_read, ra_switch_page_update_write,
			&page, &updated);
		if (status != 0)
			goto cleanup;
		status = tc_phy_read_page_reg_checked(
			port_num, 4U, 26U, 1, &read_value);
		if (status != 0)
			goto cleanup;
		pr_info("FE-%u TX-amplitude MDIX fallback register=0x%x\n",
			port_num, read_value);
	} else {
		/* Preserve the original MT7622/Leopard per-port adjustments. */
		status = tc_phy_read_page_reg_checked(
			port_num, 4U, 26U, 1, &read_value);
		if (status != 0)
			goto cleanup;
		if (ei_local->chip_name == MT7622_FE) {
			if (port_num == 0U)
				read_value += 10U;
			else if (port_num == 1U)
				read_value += 11U;
			else
				read_value += 9U;
		} else if (ei_local->chip_name == LEOPARD_FE) {
			if (port_num == 1U)
				read_value += 2U;
			else if (port_num == 2U)
				read_value += 2U;
			else if (port_num == 3U)
				read_value += 1U;
			else if (port_num == 4U)
				read_value += 3U;
		}
		status = tc_phy_write_page_reg_checked(
			port_num, 4U, 26U, 1, read_value);
		if (status != 0)
			goto cleanup;
		calibration_completed = 1;
	}

	status = tc_phy_read_page_reg_checked(
		port_num, 4U, 26U, 1, &read_value);
	if (status != 0)
		goto cleanup;
	tx_amp_mdix_final = read_value & 0x3fU;
	correction = ((tx_amp_mdix_final + 15U) << 8) | 0x20U;
	status = tc_phy_write_page_reg_checked(
		port_num, 4U, 27U, 1, correction);
	if (status != 0)
		goto cleanup;
	if (ei_local->chip_name == LEOPARD_FE) {
		if (port_num == 2U)
			correction = ((tx_amp_mdix_final + 15U + 1U) << 8) | 0x20U;
		else if ((port_num == 3U) || (port_num == 4U))
			correction = ((tx_amp_mdix_final + 15U + 4U) << 8) | 0x20U;
		else
			correction = 0U;
		if (correction != 0U) {
			status = tc_phy_write_page_reg_checked(
				port_num, 4U, 27U, 1, correction);
			if (status != 0)
				goto cleanup;
		}
	}
	pr_info("[%d] - tx_amp_mdix_final = 0x%x\n",
		port_num, tx_amp_mdix_final);

cleanup:
	cleanup_status = clear_ckinv_ana_txvos_checked();
	if (status == 0 && cleanup_status != 0)
		status = cleanup_status;
	cleanup_status = tc_phy_write_page_reg_checked(
		port_num, 3U, 25U, 1, 0U);
	if (status == 0 && cleanup_status != 0)
		status = cleanup_status;
	cleanup_status = tc_phy_write_page_reg_checked(
		FE_CAL_COMMON, 3U, 25U, 1, 0U);
	if (status == 0 && cleanup_status != 0)
		status = cleanup_status;
	cleanup_status = tc_phy_write_page_reg_checked(
		port_num, 1U, 26U, 0, 0U);
	if (status == 0 && cleanup_status != 0)
		status = cleanup_status;

	if (status != 0) {
		pr_err_ratelimited(
			"%s: FE TX-amplitude MDIX calibration failed (port=%u status=%d)\n",
			__func__, port_num, status);
		return;
	}
	if (calibration_completed)
		fe_cal_flag_mdix = 1;
}

void fe_cal_tx_offset(u8 port_num, u32 delay)
{
	unsigned int read_value = 0U, page_value = 0U, updated;
	unsigned int l2r20_temp, l4r17_temp;
	unsigned int tx_offset_final = 0U;
	int ad_cal_comp_out_init;
	int calibration_polarity, tx_offset_temp;
	int all_ana_cal_status = ANACAL_INIT;
	u8 ready = 0, cnt = 0;
	int status = 0, cleanup_status;
	int calibration_completed = 0;
	struct ra_switch_page_update_context page;

	/* A previous attempt must not make this calibration look successful. */
	fe_cal_tx_offset_flag = 0;
	if (port_num >= 5U) {
		pr_err_ratelimited("%s: invalid FE port %u\n",
				   __func__, port_num);
		return;
	}

	/* Set device in 100M mode. */
	status = tc_phy_write_page_reg_checked(
		port_num, 0U, 0U, 1, 0x2100U);
	if (status != 0)
		goto cleanup;

	/* g4r21[11]: hardware bypass TX offset calibration, firmware calibration. */
	status = tc_phy_read_page_reg_checked(
		port_num, 4U, 21U, 0, &read_value);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_page_reg_checked(
		port_num, 4U, 21U, 0, (read_value | 0x0800U));
	if (status != 0)
		goto cleanup;

	/* l0r30[9], [7], [6], [1]. */
	status = tc_phy_read_page_reg_checked(
		port_num, 0U, 30U, 1, &read_value);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_page_reg_checked(
		port_num, 0U, 30U, 1, (read_value | 0x02c0U));
	if (status != 0)
		goto cleanup;

	/* Enable the TX calibration DAC and common analog calibration. */
	status = tc_phy_write_page_reg_checked(
		port_num, 1U, 26U, 0, (0x8000U | DAC_IN_0V));
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_page_reg_checked(
		FE_CAL_COMMON, 7U, 24U, 0, 0x3000U);
	if (status != 0)
		goto cleanup;

	/* Enable TXG and ZCAL for this port without losing other bits. */
	page.page = 3U;
	page.local = 1;
	status = ra_switch_mdio_update_bits_checked(
		port_num, 25U, 0U, 0x0400U,
		ra_switch_page_update_read, ra_switch_page_update_write,
		&page, &updated);
	if (status != 0)
		goto cleanup;
	status = ra_switch_mdio_update_bits_checked(
		port_num, 25U, 0U, 0x1000U,
		ra_switch_page_update_read, ra_switch_page_update_write,
		&page, &updated);
	if (status != 0)
		goto cleanup;

	/* Preserve the chip-specific fixed PGA/MDIX setup value. */
	status = tc_phy_write_page_reg_checked(
		port_num, 0U, 26U, 1, 0x5203U);
	if (status != 0)
		goto cleanup;

	status = tc_phy_read_page_reg_checked(
		port_num, 2U, 20U, 1, &read_value);
	if (status != 0)
		goto cleanup;
	l2r20_temp = read_value & ~0x0400U;
	status = tc_phy_write_page_reg_checked(
		port_num, 2U, 20U, 1, l2r20_temp);
	if (status != 0)
		goto cleanup;

	/* Initialize the local TX-offset result register. */
	status = tc_phy_write_page_reg_checked(
		port_num, 4U, 17U, 1, 0U);
	if (status != 0)
		goto cleanup;
	status = tc_phy_read_page_reg_checked(
		port_num, 4U, 17U, 1, &read_value);
	if (status != 0)
		goto cleanup;
	l4r17_temp = read_value;
	tx_offset_temp = TX_AMP_OFFSET_0MV;
	status = tc_phy_write_page_reg_checked(
		port_num, 4U, 17U, 1,
		(l4r17_temp | (tx_offset_temp << 8)));
	if (status != 0)
		goto cleanup;

	/* Trigger calibration and distinguish MDIO failure from not-ready. */
	ready = 0;
	status = all_fe_ana_cal_wait_checked(delay, port_num, 1, &ready);
	if (status != 0)
		goto cleanup;
	if (!ready) {
		all_ana_cal_status = ANACAL_ERROR;
		pr_info("FE TX-offset calibration did not become ready (init)\n");
	} else {
		/* Preserve the original comparator settling reads. */
		status = tc_phy_read_page_reg_checked(
			FE_CAL_COMMON, 7U, 24U, 0, &page_value);
		if (status != 0)
			goto cleanup;
		status = tc_phy_read_page_reg_checked(
			FE_CAL_COMMON, 7U, 24U, 0, &page_value);
		if (status != 0)
			goto cleanup;
		status = tc_phy_read_page_reg_checked(
			FE_CAL_COMMON, 7U, 24U, 0, &page_value);
		if (status != 0)
			goto cleanup;
		ad_cal_comp_out_init = (int)(page_value & 0x1U);
		calibration_polarity = ad_cal_comp_out_init ? -1 : 1;
		tx_offset_temp = ra_switch_mdio_calibration_code_step(
			tx_offset_temp, calibration_polarity);

		while ((all_ana_cal_status < ANACAL_ERROR) && (cnt < 254U)) {
			++cnt;
			status = tc_phy_write_page_reg_checked(
				port_num, 4U, 17U, 1,
				((unsigned int)tx_offset_temp << 8));
			if (status != 0)
				goto cleanup;

			status = tc_phy_read_page_reg_checked(
				FE_CAL_COMMON, 7U, 24U, 0, &page_value);
			if (status != 0)
				goto cleanup;
			status = tc_phy_read_page_reg_checked(
				FE_CAL_COMMON, 7U, 24U, 0, &page_value);
			if (status != 0)
				goto cleanup;

			ready = 0;
			status = all_fe_ana_cal_wait_checked(
				delay, port_num, 1, &ready);
			if (status != 0)
				goto cleanup;
			if (!ready) {
				all_ana_cal_status = ANACAL_ERROR;
				pr_info("FE TX-offset calibration timed out (step=%u)\n",
					cnt);
				break;
			}

			status = tc_phy_read_page_reg_checked(
				FE_CAL_COMMON, 7U, 24U, 0, &page_value);
			if (status != 0)
				goto cleanup;
			if ((page_value & 0x1U) !=
			    (unsigned int)ad_cal_comp_out_init) {
				all_ana_cal_status = ANACAL_FINISH;
				/* Retain the original post-crossing settling reads. */
				status = tc_phy_read_page_reg_checked(
					FE_CAL_COMMON, 7U, 24U, 0, &page_value);
				if (status != 0)
					goto cleanup;
				status = tc_phy_read_page_reg_checked(
					FE_CAL_COMMON, 7U, 24U, 0, &page_value);
				if (status != 0)
					goto cleanup;
				status = tc_phy_read_page_reg_checked(
					FE_CAL_COMMON, 7U, 24U, 0, &page_value);
				if (status != 0)
					goto cleanup;
				ad_cal_comp_out_init = (int)(page_value & 0x1U);
			} else {
				status = tc_phy_read_page_reg_checked(
					port_num, 4U, 17U, 1, &l4r17_temp);
				if (status != 0)
					goto cleanup;
				if ((tx_offset_temp == 0x3f) ||
				    (tx_offset_temp == 0x00)) {
					all_ana_cal_status = ANACAL_SATURATION;
					pr_info("FE TX-offset calibration saturated\n");
				} else {
					tx_offset_temp =
						ra_switch_mdio_calibration_code_step(
							tx_offset_temp,
							calibration_polarity);
				}
			}
		}
		if ((all_ana_cal_status == ANACAL_INIT) && (cnt >= 254U)) {
			all_ana_cal_status = ANACAL_ERROR;
			pr_info("FE TX-offset calibration exhausted its search range\n");
		}
	}

	/* Ordinary timeout/saturation keeps the original 0 mV fallback. */
	if ((all_ana_cal_status == ANACAL_ERROR) ||
	    (all_ana_cal_status == ANACAL_SATURATION)) {
		tx_offset_temp = TX_AMP_OFFSET_0MV;
		page.page = 4U;
		page.local = 1;
		status = ra_switch_mdio_update_bits_checked(
			port_num, 17U, 0x3f00U,
			((unsigned int)tx_offset_temp << 8),
			ra_switch_page_update_read, ra_switch_page_update_write,
			&page, &updated);
		if (status != 0)
			goto cleanup;
	} else {
		calibration_completed = 1;
	}

	status = tc_phy_read_page_reg_checked(
		port_num, 4U, 17U, 1, &read_value);
	if (status != 0)
		goto cleanup;
	tx_offset_final = (u16)((read_value & 0x3f00U) >> 8);
	pr_info("[%d] - tx_offset_final = 0x%x\n", port_num, tx_offset_final);

cleanup:
	/* Attempt every independent analog reset and retain the first error. */
	cleanup_status = clear_ckinv_ana_txvos_checked();
	if (status == 0 && cleanup_status != 0)
		status = cleanup_status;

	cleanup_status = tc_phy_write_page_reg_checked(
		port_num, 3U, 25U, 1, 0U);
	if (status == 0 && cleanup_status != 0)
		status = cleanup_status;

	cleanup_status = tc_phy_write_page_reg_checked(
		FE_CAL_COMMON, 3U, 25U, 1, 0U);
	if (status == 0 && cleanup_status != 0)
		status = cleanup_status;

	cleanup_status = tc_phy_write_page_reg_checked(
		port_num, 1U, 26U, 0, 0U);
	if (status == 0 && cleanup_status != 0)
		status = cleanup_status;

	if (status != 0) {
		pr_err_ratelimited(
			"%s: FE TX-offset calibration failed (port=%u status=%d)\n",
			__func__, port_num, status);
		return;
	}
	if (calibration_completed)
		fe_cal_tx_offset_flag = 1;
}

void fe_cal_tx_offset_mdix(u8 port_num, u32 delay)
{
	unsigned int read_value = 0U, page_value = 0U, updated;
	unsigned int l4r26_temp_amp = 0U, tx_offset_final_mdix;
	int ad_cal_comp_out_init;
	int calibration_polarity, tx_offset_temp = TX_AMP_OFFSET_0MV;
	int all_ana_cal_status = ANACAL_INIT;
	u8 ready = 0, cnt = 0;
	int status = 0, cleanup_status;
	int calibration_completed = 0;
	struct ra_switch_page_update_context page;

	fe_cal_tx_offset_flag_mdix = 0;
	if (port_num >= 5U) {
		pr_err_ratelimited("%s: invalid FE port %u\n",
				   __func__, port_num);
		return;
	}

	status = tc_phy_write_page_reg_checked(
		port_num, 0U, 0U, 1, 0x2100U);
	if (status != 0)
		goto cleanup;

	/* Keep the existing hardware bypass and MDIX PGA setup. */
	status = tc_phy_read_page_reg_checked(
		port_num, 4U, 21U, 0, &read_value);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_page_reg_checked(
		port_num, 4U, 21U, 0, (read_value | 0x0800U));
	if (status != 0)
		goto cleanup;
	status = tc_phy_read_page_reg_checked(
		port_num, 0U, 30U, 1, &read_value);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_page_reg_checked(
		port_num, 0U, 30U, 1, (read_value | 0x02c0U));
	if (status != 0)
		goto cleanup;

	status = tc_phy_write_page_reg_checked(
		port_num, 1U, 26U, 0, (0x8000U | DAC_IN_0V));
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_page_reg_checked(
		FE_CAL_COMMON, 7U, 24U, 0, 0x3000U);
	if (status != 0)
		goto cleanup;

	page.page = 3U;
	page.local = 1;
	status = ra_switch_mdio_update_bits_checked(
		port_num, 25U, 0U, 0x0400U,
		ra_switch_page_update_read, ra_switch_page_update_write,
		&page, &updated);
	if (status != 0)
		goto cleanup;
	status = ra_switch_mdio_update_bits_checked(
		port_num, 25U, 0U, 0x1000U,
		ra_switch_page_update_read, ra_switch_page_update_write,
		&page, &updated);
	if (status != 0)
		goto cleanup;

	status = tc_phy_write_page_reg_checked(
		port_num, 0U, 26U, 1, 0x9203U);
	if (status != 0)
		goto cleanup;
	page.page = 2U;
	page.local = 1;
	status = ra_switch_mdio_update_bits_checked(
		port_num, 20U, 0U, 0x0400U,
		ra_switch_page_update_read, ra_switch_page_update_write,
		&page, &updated);
	if (status != 0)
		goto cleanup;

	/* Initialize only the six-bit TX-offset field in local page 4/register 26. */
	page.page = 4U;
	page.local = 1;
	status = ra_switch_mdio_update_bits_checked(
		port_num, 26U, 0x3f00U, 0U,
		ra_switch_page_update_read, ra_switch_page_update_write,
		&page, &updated);
	if (status != 0)
		goto cleanup;

	ready = 0;
	status = all_fe_ana_cal_wait_checked(delay, port_num, 1, &ready);
	if (status != 0)
		goto cleanup;
	if (!ready) {
		all_ana_cal_status = ANACAL_ERROR;
		pr_info("FE TX-offset MDIX calibration did not become ready (init)\n");
	} else {
		status = tc_phy_read_page_reg_checked(
			FE_CAL_COMMON, 7U, 24U, 0, &page_value);
		if (status != 0)
			goto cleanup;
		status = tc_phy_read_page_reg_checked(
			FE_CAL_COMMON, 7U, 24U, 0, &page_value);
		if (status != 0)
			goto cleanup;
		status = tc_phy_read_page_reg_checked(
			FE_CAL_COMMON, 7U, 24U, 0, &page_value);
		if (status != 0)
			goto cleanup;
		ad_cal_comp_out_init = (int)(page_value & 0x1U);
		calibration_polarity = ad_cal_comp_out_init ? -1 : 1;
		tx_offset_temp = (int)ra_switch_mdio_calibration_code_step(
			(unsigned int)tx_offset_temp, calibration_polarity);

		while ((all_ana_cal_status < ANACAL_ERROR) && (cnt < 254U)) {
			++cnt;
			page.page = 4U;
			page.local = 1;
			status = ra_switch_mdio_update_bits_checked(
				port_num, 26U, 0x3f00U,
				((unsigned int)tx_offset_temp << 8),
				ra_switch_page_update_read,
				ra_switch_page_update_write, &page, &updated);
			if (status != 0)
				goto cleanup;
			status = tc_phy_read_page_reg_checked(
				port_num, 4U, 26U, 1, &read_value);
			if (status != 0)
				goto cleanup;
			l4r26_temp_amp = read_value & 0x3fU;

			status = tc_phy_read_page_reg_checked(
				FE_CAL_COMMON, 7U, 24U, 0, &page_value);
			if (status != 0)
				goto cleanup;
			status = tc_phy_read_page_reg_checked(
				FE_CAL_COMMON, 7U, 24U, 0, &page_value);
			if (status != 0)
				goto cleanup;

			ready = 0;
			status = all_fe_ana_cal_wait_checked(
				delay, port_num, 1, &ready);
			if (status != 0)
				goto cleanup;
			if (!ready) {
				all_ana_cal_status = ANACAL_ERROR;
				pr_info("FE TX-offset MDIX calibration timed out (step=%u)\n",
					cnt);
				break;
			}

			status = tc_phy_read_page_reg_checked(
				FE_CAL_COMMON, 7U, 24U, 0, &page_value);
			if (status != 0)
				goto cleanup;
			if ((page_value & 0x1U) !=
			    (unsigned int)ad_cal_comp_out_init) {
				all_ana_cal_status = ANACAL_FINISH;
				status = tc_phy_read_page_reg_checked(
					FE_CAL_COMMON, 7U, 24U, 0, &page_value);
				if (status != 0)
					goto cleanup;
				status = tc_phy_read_page_reg_checked(
					FE_CAL_COMMON, 7U, 24U, 0, &page_value);
				if (status != 0)
					goto cleanup;
				status = tc_phy_read_page_reg_checked(
					FE_CAL_COMMON, 7U, 24U, 0, &page_value);
				if (status != 0)
					goto cleanup;
				ad_cal_comp_out_init = (int)(page_value & 0x1U);
			} else if ((l4r26_temp_amp == 0x3fU) ||
				   (l4r26_temp_amp == 0U)) {
				all_ana_cal_status = ANACAL_SATURATION;
				pr_info("FE TX-offset MDIX calibration saturated\n");
			} else {
				tx_offset_temp = (int)ra_switch_mdio_calibration_code_step(
					(unsigned int)tx_offset_temp,
					calibration_polarity);
			}
		}
		if ((all_ana_cal_status == ANACAL_INIT) && (cnt >= 254U)) {
			all_ana_cal_status = ANACAL_ERROR;
			pr_info("FE TX-offset MDIX calibration exhausted its search range\n");
		}
	}

	/* A failed search must program the explicit 0 mV field, not stale cal_temp. */
	if ((all_ana_cal_status == ANACAL_ERROR) ||
	    (all_ana_cal_status == ANACAL_SATURATION)) {
		tx_offset_temp = TX_AMP_OFFSET_0MV;
		page.page = 4U;
		page.local = 1;
		status = ra_switch_mdio_update_bits_checked(
			port_num, 26U, 0x3f00U,
			((unsigned int)tx_offset_temp << 8),
			ra_switch_page_update_read, ra_switch_page_update_write,
			&page, &updated);
		if (status != 0)
			goto cleanup;
	} else {
		calibration_completed = 1;
	}

	status = tc_phy_read_page_reg_checked(
		port_num, 4U, 26U, 1, &read_value);
	if (status != 0)
		goto cleanup;
	tx_offset_final_mdix = (read_value & 0x3f00U) >> 8;
	pr_info("[%d] - tx_offset_final_mdix = 0x%x\n",
		port_num, tx_offset_final_mdix);

cleanup:
	cleanup_status = clear_ckinv_ana_txvos_checked();
	if (status == 0 && cleanup_status != 0)
		status = cleanup_status;
	cleanup_status = tc_phy_write_page_reg_checked(
		port_num, 3U, 25U, 1, 0U);
	if (status == 0 && cleanup_status != 0)
		status = cleanup_status;
	cleanup_status = tc_phy_write_page_reg_checked(
		FE_CAL_COMMON, 3U, 25U, 1, 0U);
	if (status == 0 && cleanup_status != 0)
		status = cleanup_status;
	cleanup_status = tc_phy_write_page_reg_checked(
		port_num, 1U, 26U, 0, 0U);
	if (status == 0 && cleanup_status != 0)
		status = cleanup_status;

	if (status != 0) {
		pr_err_ratelimited(
			"%s: FE TX-offset MDIX calibration failed (port=%u status=%d)\n",
			__func__, port_num, status);
		return;
	}
	if (calibration_completed)
		fe_cal_tx_offset_flag_mdix = 1;
}


static int set_r50_leopard_checked(u8 port_num, u32 r50_cal_result)
{
	unsigned int l4r22_temp;
	unsigned int tx_value, rx_value;
	int status;

	if (port_num > 4U)
		return -EINVAL;
	if (r50_cal_result >= ARRAY_SIZE(ZCAL_TO_R50OHM_TBL_100))
		return -ERANGE;

	tx_value = ZCAL_TO_R50OHM_TBL_100[r50_cal_result] + 4U;
	rx_value = ZCAL_TO_R50OHM_TBL_100[r50_cal_result] + 6U;
	if (tx_value > 0x7fU)
		tx_value = 0x7fU;
	if (rx_value > 0x7fU)
		rx_value = 0x7fU;

	status = tc_phy_write_page_reg_checked(
		port_num, 4U, 22U, 1, tx_value << 8);
	if (status != 0)
		return status;
	status = tc_phy_read_page_reg_checked(
		port_num, 4U, 22U, 1, &l4r22_temp);
	if (status != 0)
		return status;
	status = tc_phy_write_page_reg_checked(
		port_num, 4U, 22U, 1, l4r22_temp | rx_value);
	if (status != 0)
		return status;
	status = tc_phy_read_page_reg_checked(
		port_num, 4U, 22U, 1, &l4r22_temp);
	if (status != 0)
		return status;
	pr_info("port_num=%u set R50 TX/RX to %u/%u (reg=0x%x)\n",
		port_num, tx_value, rx_value, l4r22_temp);
	return 0;
}

static int set_r50_mt7622_checked(u8 port_num, u32 r50_cal_result)
{
	static const u8 tx_offset[5] = { 5U, 3U, 4U, 4U, 4U };
	static const u8 rx_offset[5] = { 5U, 3U, 5U, 3U, 5U };
	unsigned int tx_index, rx_index;
	unsigned int l4r22_temp;
	unsigned int tx_value, rx_value;
	int status;

	if (port_num >= ARRAY_SIZE(tx_offset))
		return -EINVAL;
	/* Avoid unsigned underflow and out-of-range calibration-table reads. */
	if ((r50_cal_result < tx_offset[port_num]) ||
	    (r50_cal_result < rx_offset[port_num]))
		return -ERANGE;
	tx_index = r50_cal_result - tx_offset[port_num];
	rx_index = r50_cal_result - rx_offset[port_num];
	if ((tx_index >= ARRAY_SIZE(ZCAL_TO_R50OHM_TBL_100)) ||
	    (rx_index >= ARRAY_SIZE(ZCAL_TO_R50OHM_TBL_100)))
		return -ERANGE;

	tx_value = ZCAL_TO_R50OHM_TBL_100[tx_index] + 4U;
	rx_value = ZCAL_TO_R50OHM_TBL_100[rx_index] + 6U;
	if (tx_value > 0x7fU)
		tx_value = 0x7fU;
	if (rx_value > 0x7fU)
		rx_value = 0x7fU;

	status = tc_phy_write_page_reg_checked(
		port_num, 4U, 22U, 1, tx_value << 8);
	if (status != 0)
		return status;
	status = tc_phy_read_page_reg_checked(
		port_num, 4U, 22U, 1, &l4r22_temp);
	if (status != 0)
		return status;
	status = tc_phy_write_page_reg_checked(
		port_num, 4U, 22U, 1, l4r22_temp | rx_value);
	if (status != 0)
		return status;
	status = tc_phy_read_page_reg_checked(
		port_num, 4U, 22U, 1, &l4r22_temp);
	if (status != 0)
		return status;
	pr_info("port_num=%u set R50 TX/RX to %u/%u (reg=0x%x)\n",
		port_num, tx_value, rx_value, l4r22_temp);
	return 0;
}

/* Keep the legacy void entry points for out-of-tree callers. */
void set_r50_leopard(u8 port_num, u32 r50_cal_result)
{
	int status = set_r50_leopard_checked(port_num, r50_cal_result);

	if (status != 0) {
		pr_err_ratelimited("%s: failed (port=%u result=%u status=%d)\n",
				   __func__, port_num, r50_cal_result, status);
		return;
	}
	fe_cal_r50_flag = 1;
}

void set_r50_mt7622(u8 port_num, u32 r50_cal_result)
{
	int status = set_r50_mt7622_checked(port_num, r50_cal_result);

	if (status != 0) {
		pr_err_ratelimited("%s: failed (port=%u result=%u status=%d)\n",
				   __func__, port_num, r50_cal_result, status);
		return;
	}
	fe_cal_r50_flag = 1;
}
void fe_cal_r50(u8 port_num, unsigned int delay)
{
	u8 rg_zcal_ctrl = 0x20U;
	u8 ready = 0;
	u8 calibration_polarity = 1;
	u8 all_ana_cal_status = ANACAL_INIT;
	u8 cnt = 0;
	unsigned int page_value, updated;
	unsigned int r50_cal_result;
	int status = 0;
	int cleanup_status;
	struct ra_switch_page_update_context page;
	struct END_DEVICE *ei_local = netdev_priv(dev_raether);

	fe_cal_r50_flag = 0;

	/* Enable FE calibration mode and prepare the shared R50 control. */
	status = tc_phy_write_page_reg_checked(
		port_num, 0U, 0U, 1, 0x2100U);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_page_reg_checked(
		FE_CAL_COMMON, 2U, 25U, 0, 0xf0c0U);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_page_reg_checked(
		FE_CAL_COMMON, 7U, 24U, 0, 0U);
	if (status != 0)
		goto cleanup;

	page.page = 7U;
	page.local = 0;
	status = ra_switch_mdio_update_bits_checked(
		FE_CAL_COMMON, 24U, 0U, 0x2000U,
		ra_switch_page_update_read, ra_switch_page_update_write,
		&page, &updated);
	if (status != 0)
		goto cleanup;
	status = ra_switch_mdio_update_bits_checked(
		FE_CAL_COMMON, 24U, 0U, 0x4000U,
		ra_switch_page_update_read, ra_switch_page_update_write,
		&page, &updated);
	if (status != 0)
		goto cleanup;
	status = ra_switch_mdio_update_bits_checked(
		FE_CAL_COMMON, 24U, 0x1000U, 0U,
		ra_switch_page_update_read, ra_switch_page_update_write,
		&page, &updated);
	if (status != 0)
		goto cleanup;

	page.page = 4U;
	page.local = 1;
	status = ra_switch_mdio_update_bits_checked(
		port_num, 23U, 0U, 0x0001U,
		ra_switch_page_update_read, ra_switch_page_update_write,
		&page, &updated);
	if (status != 0)
		goto cleanup;

	page.page = 3U;
	status = ra_switch_mdio_update_bits_checked(
		FE_CAL_COMMON, 25U, 0x2000U, 0U,
		ra_switch_page_update_read, ra_switch_page_update_write,
		&page, &updated);
	if (status != 0)
		goto cleanup;
	status = ra_switch_mdio_update_bits_checked(
		port_num, 25U, 0U, 0x1000U,
		ra_switch_page_update_read, ra_switch_page_update_write,
		&page, &updated);
	if (status != 0)
		goto cleanup;

	/* Search all calibration codes with a status-bearing bounded wait. */
	page.page = 7U;
	page.local = 0;
	status = ra_switch_mdio_update_bits_checked(
		FE_CAL_COMMON, 24U, 0x0fc0U,
		((unsigned int)(rg_zcal_ctrl & 0x3fU) << 6),
		ra_switch_page_update_read, ra_switch_page_update_write,
		&page, &updated);
	if (status != 0)
		goto cleanup;

	status = all_fe_ana_cal_wait_checked(delay, port_num, 1, &ready);
	if (status != 0)
		goto cleanup;
	if (!ready) {
		all_ana_cal_status = ANACAL_ERROR;
		pr_info("FE R50 calibration did not become ready (port=%u)\n",
			port_num);
	} else {
		status = tc_phy_read_page_reg_checked(
			FE_CAL_COMMON, 7U, 24U, 0, &page_value);
		if (status != 0)
			goto cleanup;
		if ((page_value & 0x1U) != 0U)
			calibration_polarity = (u8)-1;
		else
			calibration_polarity = 1U;

		while ((all_ana_cal_status < ANACAL_ERROR) && (cnt < 254U)) {
			++cnt;
			rg_zcal_ctrl = (u8)(rg_zcal_ctrl + calibration_polarity);
			status = ra_switch_mdio_update_bits_checked(
				FE_CAL_COMMON, 24U, 0x0fc0U,
				((unsigned int)(rg_zcal_ctrl & 0x3fU) << 6),
				ra_switch_page_update_read,
				ra_switch_page_update_write, &page, &updated);
			if (status != 0)
				goto cleanup;
			ready = 0;
			status = all_fe_ana_cal_wait_checked(
				delay, port_num, 1, &ready);
			if (status != 0)
				goto cleanup;
			if (!ready) {
				all_ana_cal_status = ANACAL_ERROR;
				pr_info("FE R50 calibration timed out at step %u\n",
					cnt);
				break;
			}
			status = tc_phy_read_page_reg_checked(
				FE_CAL_COMMON, 7U, 24U, 0, &page_value);
			if (status != 0)
				goto cleanup;
			if ((page_value & 0x1U) != 0U) {
				/* Comparator has crossed from its initial state. */
				if (calibration_polarity == 1U)
					all_ana_cal_status = ANACAL_FINISH;
			} else if (calibration_polarity == (u8)-1) {
				all_ana_cal_status = ANACAL_FINISH;
			}
			if (all_ana_cal_status == ANACAL_FINISH)
				break;
			if ((rg_zcal_ctrl == 0x3fU) || (rg_zcal_ctrl == 0x00U)) {
				all_ana_cal_status = ANACAL_SATURATION;
				pr_info("FE R50 calibration saturated (port=%u)\n",
					port_num);
				break;
			}
		}
		if ((all_ana_cal_status == ANACAL_INIT) && (cnt >= 254U)) {
			all_ana_cal_status = ANACAL_ERROR;
			pr_info("FE R50 calibration exhausted its search range\n");
		}
	}

	if ((all_ana_cal_status == ANACAL_ERROR) ||
	    (all_ana_cal_status == ANACAL_SATURATION))
		r50_cal_result = 0x20U;
	else
		r50_cal_result = rg_zcal_ctrl;

	if (port_num == 0U)
		r50_p0_cal_result = r50_cal_result;
	if (ei_local->chip_name == MT7622_FE)
		status = set_r50_mt7622_checked(port_num, r50_cal_result);
	else if (ei_local->chip_name == LEOPARD_FE)
		status = set_r50_leopard_checked(port_num, r50_cal_result);
	else
		status = -ENODEV;
	if (status != 0)
		goto cleanup;

cleanup:
	/* Cleanup is best-effort: keep the first error but try every reset. */
	cleanup_status = clear_ckinv_ana_txvos_checked();
	if (status == 0 && cleanup_status != 0)
		status = cleanup_status;
	cleanup_status = tc_phy_write_page_reg_checked(
		port_num, 3U, 25U, 1, 0U);
	if (status == 0 && cleanup_status != 0)
		status = cleanup_status;
	cleanup_status = tc_phy_write_page_reg_checked(
		FE_CAL_COMMON, 3U, 25U, 1, 0U);
	if (status == 0 && cleanup_status != 0)
		status = cleanup_status;

	if (status != 0) {
		pr_err_ratelimited("%s: FE R50 calibration failed (port=%u status=%d cleanup=%d)\n",
				   __func__, port_num, status, cleanup_status);
		return;
	}
	fe_cal_r50_flag = 1;
}
void fe_cal_vbg(u8 port_num, u32 delay)
{
	unsigned int rg_zcal_ctrl = 0U;
	unsigned int page_value = 0U, updated;
	unsigned int rg_bg_rasel = 0U;
	int all_ana_cal_status = ANACAL_INIT;
	int ad_cal_comp_out_init;
	int calibration_polarity;
	u8 cnt = 0, ready = 0, port_no;
	int status = 0, cleanup_status;
	int calibration_completed = 0;
	struct ra_switch_page_update_context page;
	struct END_DEVICE *ei_local = netdev_priv(dev_raether);

	fe_cal_vbg_flag = 0;
	ephy_addr_base = 0;
	if (port_num >= 5U) {
		pr_err_ratelimited("%s: invalid FE port %u\n",
				   __func__, port_num);
		return;
	}

	/* Configure the common VBG calibration path. */
	status = tc_phy_write_page_reg_checked(
		FE_CAL_COMMON, 2U, 25U, 0, 0x30c0U);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_page_reg_checked(
		FE_CAL_COMMON, 0U, 25U, 0, 0x0030U);
	if (status != 0)
		goto cleanup;

	page.page = 7U;
	page.local = 0;
	status = ra_switch_mdio_update_bits_checked(
		FE_CAL_COMMON, 24U, 0U, 0x2000U,
		ra_switch_page_update_read, ra_switch_page_update_write,
		&page, &updated);
	if (status != 0)
		goto cleanup;
	status = ra_switch_mdio_update_bits_checked(
		FE_CAL_COMMON, 24U, 0U, 0x4000U,
		ra_switch_page_update_read, ra_switch_page_update_write,
		&page, &updated);
	if (status != 0)
		goto cleanup;
	status = ra_switch_mdio_update_bits_checked(
		FE_CAL_COMMON, 24U, 0x1000U, 0U,
		ra_switch_page_update_read, ra_switch_page_update_write,
		&page, &updated);
	if (status != 0)
		goto cleanup;

	/* Mark VBG calibration in the common local control. */
	page.page = 3U;
	page.local = 1;
	status = ra_switch_mdio_update_bits_checked(
		FE_CAL_COMMON, 25U, 0U, 0x2000U,
		ra_switch_page_update_read, ra_switch_page_update_write,
		&page, &updated);
	if (status != 0)
		goto cleanup;

	/* Disable RG_ZCALEN on this and all later FE ports. */
	for (port_no = port_num; port_no < 5U; ++port_no) {
		status = ra_switch_mdio_update_bits_checked(
			port_no, 25U, 0x1000U, 0U,
			ra_switch_page_update_read, ra_switch_page_update_write,
			&page, &updated);
		if (status != 0)
			goto cleanup;
	}

	/* Start at code zero and keep the six-bit field bounded. */
	page.page = 7U;
	page.local = 0;
	status = ra_switch_mdio_update_bits_checked(
		FE_CAL_COMMON, 24U, 0x0fc0U, 0U,
		ra_switch_page_update_read, ra_switch_page_update_write,
		&page, &updated);
	if (status != 0)
		goto cleanup;

	ready = 0;
	status = all_fe_ana_cal_wait_checked(delay, port_num, 1, &ready);
	if (status != 0)
		goto cleanup;
	if (!ready) {
		all_ana_cal_status = ANACAL_ERROR;
		pr_info("fe_cal_vbg did not become ready (port=%u)\n", port_num);
	}

	status = tc_phy_read_page_reg_checked(
		FE_CAL_COMMON, 7U, 24U, 0, &page_value);
	if (status != 0)
		goto cleanup;
	ad_cal_comp_out_init = (int)(page_value & 0x1U);
	calibration_polarity = ad_cal_comp_out_init ? -1 : 1;

	while ((all_ana_cal_status < ANACAL_ERROR) && (cnt < 254U)) {
		++cnt;
		rg_zcal_ctrl = ra_switch_mdio_calibration_code_step(
			rg_zcal_ctrl, calibration_polarity);
		status = ra_switch_mdio_update_bits_checked(
			FE_CAL_COMMON, 24U, 0x0fc0U,
			((rg_zcal_ctrl & 0x3fU) << 6),
			ra_switch_page_update_read, ra_switch_page_update_write,
			&page, &updated);
		if (status != 0)
			goto cleanup;

		ready = 0;
		status = all_fe_ana_cal_wait_checked(
			delay, port_num, 1, &ready);
		if (status != 0)
			goto cleanup;
		if (!ready) {
			all_ana_cal_status = ANACAL_ERROR;
			pr_info("VBG calibration timed out at step %u\n", cnt);
			break;
		}

		status = tc_phy_read_page_reg_checked(
			FE_CAL_COMMON, 7U, 24U, 0, &page_value);
		if (status != 0)
			goto cleanup;
		if ((page_value & 0x1U) !=
		    (unsigned int)ad_cal_comp_out_init) {
			all_ana_cal_status = ANACAL_FINISH;
		} else if ((rg_zcal_ctrl == 0x3fU) ||
			   (rg_zcal_ctrl == 0x00U)) {
			all_ana_cal_status = ANACAL_SATURATION;
			pr_info("VBG calibration saturated at step %u\n", cnt);
		}
	}
	if ((all_ana_cal_status == ANACAL_INIT) && (cnt >= 254U)) {
		all_ana_cal_status = ANACAL_ERROR;
		pr_info("VBG calibration exhausted its search range\n");
	}

	if ((all_ana_cal_status == ANACAL_ERROR) ||
	    (all_ana_cal_status == ANACAL_SATURATION)) {
		/* Preserve the original 0 dB fallback instead of publishing the
		 * last partially searched comparator value.
		 */
		rg_zcal_ctrl = 0x20U;
	} else {
		calibration_completed = 1;
	}

	/* Validate the six-bit value before indexing the 64-entry table. */
	rg_zcal_ctrl &= 0x3fU;
	if (rg_zcal_ctrl >= ARRAY_SIZE(ZCAL_TO_REXT_TBL)) {
		status = -ERANGE;
		goto cleanup;
	}
	if (ei_local->chip_name == LEOPARD_FE)
		rg_bg_rasel = ZCAL_TO_REXT_TBL[rg_zcal_ctrl];

	/* Publish the selected code in the local FE calibration register. */
	page.page = 3U;
	page.local = 1;
	status = ra_switch_mdio_update_bits_checked(
		FE_CAL_COMMON, 26U, 0x0fc0U,
		((rg_zcal_ctrl & 0x3fU) << 6),
		ra_switch_page_update_read, ra_switch_page_update_write,
		&page, &updated);
	if (status != 0)
		goto cleanup;

	/* Preserve unrelated global bias bits while programming the chip value. */
	page.page = 2U;
	page.local = 0;
	if (ei_local->chip_name == LEOPARD_FE) {
		rg_bg_rasel &= 0x7U;
		status = ra_switch_mdio_update_bits_checked(
			FE_CAL_COMMON, 22U, 0x0e00U,
			(rg_bg_rasel << 9),
			ra_switch_page_update_read, ra_switch_page_update_write,
			&page, &updated);
		if (status != 0)
			goto cleanup;
	} else if (ei_local->chip_name == MT7622_FE) {
		status = ra_switch_mdio_update_bits_checked(
			FE_CAL_COMMON, 22U, 0x0e00U,
			(((rg_zcal_ctrl & 0x38U) >> 3) << 9),
			ra_switch_page_update_read, ra_switch_page_update_write,
			&page, &updated);
		if (status != 0)
			goto cleanup;
	}

cleanup:
	/* Best-effort shutdown is required even after a setup/search failure. */
	cleanup_status = clear_ckinv_ana_txvos_checked();
	if (status == 0 && cleanup_status != 0)
		status = cleanup_status;

	cleanup_status = tc_phy_write_page_reg_checked(
		port_num, 3U, 25U, 1, 0U);
	if (status == 0 && cleanup_status != 0)
		status = cleanup_status;

	cleanup_status = tc_phy_write_page_reg_checked(
		FE_CAL_COMMON, 3U, 25U, 1, 0U);
	if (status == 0 && cleanup_status != 0)
		status = cleanup_status;

	if (status != 0) {
		pr_err_ratelimited("%s: FE VBG calibration failed (port=%u status=%d)\n",
				   __func__, port_num, status);
		return;
	}
	iext_cal_result = rg_zcal_ctrl;
	if (calibration_completed)
		fe_cal_vbg_flag = 1;
}

#define CALDLY 40

/*
 * Run FE analog teardown only after setup has attempted a hardware write.
 * A failed MDIO write may have reached the PHY, so mark setup before issuing it.
 */
struct ra_switch_fe_analog_cleanup_context {
	u8 port_num;
	u16 l0r26_temp;
	u16 l0r30_temp;
	int l0r26_saved;
	int l0r30_saved;
};

static int ra_switch_fe_analog_cal_cleanup(void *context)
{
	struct ra_switch_fe_analog_cleanup_context *cleanup = context;
	struct ra_switch_page_update_context page;
	unsigned int updated = 0U;
	int status = 0;
	int cleanup_status;

	cleanup_status = clear_ckinv_ana_txvos_checked();
	if (status == 0 && cleanup_status != 0)
		status = cleanup_status;
	page.page = 3U;
	page.local = 1;
	cleanup_status = ra_switch_mdio_update_bits_checked(
		cleanup->port_num, 25U, 0x1000U, 0U,
		ra_switch_page_update_read, ra_switch_page_update_write,
		&page, &updated);
	if (status == 0 && cleanup_status != 0)
		status = cleanup_status;
	cleanup_status = tc_phy_write_page_reg_checked(
		cleanup->port_num, 1U, 26U, 0, 0U);
	if (status == 0 && cleanup_status != 0)
		status = cleanup_status;
	if (cleanup->l0r26_saved) {
		cleanup_status = tc_phy_write_page_reg_checked(
			cleanup->port_num, 0U, 26U, 1, cleanup->l0r26_temp);
		if (status == 0 && cleanup_status != 0)
			status = cleanup_status;
	}
	if (cleanup->l0r30_saved) {
		cleanup_status = tc_phy_write_page_reg_checked(
			cleanup->port_num, 0U, 30U, 1, cleanup->l0r30_temp);
		if (status == 0 && cleanup_status != 0)
			status = cleanup_status;
	}
	cleanup_status = tc_phy_write_page_reg_checked(
		cleanup->port_num, 0U, 0U, 1, 0x3100U);
	if (status == 0 && cleanup_status != 0)
		status = cleanup_status;
	cleanup_status = tc_phy_write_page_reg_checked(
		cleanup->port_num, 0U, 4U, 0, 0x05e1U);
	if (status == 0 && cleanup_status != 0)
		status = cleanup_status;
	return status;
}

static int do_fe_phy_all_analog_cal_checked(u8 port_num)
{
	struct ra_switch_fe_analog_cleanup_context cleanup_context;
	struct END_DEVICE *ei_local = netdev_priv(dev_raether);
	unsigned int read_value = 0U;
	u16 l0r26_temp = 0U, l0r30_temp = 0U;
	u8 phyaddr, iext_port = 0xffU, i, attempt;
	unsigned int j;
	unsigned long t_s, t_e;
	int l0r26_saved = 0, l0r30_saved = 0;
	int setup_attempted = 0;
	int status = 0, cleanup_status;
	const char *failed_stage = "setup";
	static const u8 eye_regs[] = { 20U, 23U, 24U, 26U, 27U, 29U, 30U };
	static const u16 eye_values[] = {
		0x0170U, 0x0220U, 0x0206U, 0x0370U,
		0x02f2U, 0x001bU, 0x0002U
	};
	static const u8 leopard_eye_regs[] = {
		19U, 20U, 21U, 22U, 23U, 24U,
		25U, 26U, 27U, 28U, 29U, 30U
	};
	static const u16 leopard_eye_values[] = {
		0x0100U, 0x0161U, 0x00f0U, 0x0046U,
		0x0210U, 0x0206U, 0x0238U, 0x0360U,
		0x02f2U, 0x0240U, 0x0010U, 0x0002U
	};
	static const u16 leopard_zero_dev_regs[] = {
		0x017dU, 0x017eU, 0x017fU, 0x0180U,
		0x0181U, 0x0182U, 0x0183U, 0x0184U,
		0x00dbU, 0x00dcU, 0x003eU, 0x00ddU
	};
	struct fe_default_register {
		u8 page;
		u8 reg;
		u8 local;
		u16 value;
	};
	static const struct fe_default_register fe_defaults[] = {
		{ 3U, 23U, 0U, 0x0000U },
		{ 3U, 23U, 1U, 0x2004U },
		{ 2U, 21U, 1U, 0x8551U },
		{ 4U, 17U, 1U, 0x2000U },
		{ 7U, 20U, 0U, 0x7c62U },
		{ 4U, 20U, 1U, 0x4444U },
		{ 2U, 22U, 1U, 0x1011U },
		{ 4U, 28U, 1U, 0x1011U },
		{ 4U, 19U, 1U, 0x2222U },
		{ 4U, 29U, 1U, 0x2222U },
		{ 2U, 28U, 1U, 0x3444U },
		{ 2U, 29U, 1U, 0x04c6U },
		{ 4U, 30U, 1U, 0x0006U },
		{ 5U, 16U, 1U, 0x04c6U }
	};

	if (port_num >= 5U)
		return -EINVAL;
	ephy_addr_base = 0;
	phyaddr = port_num;
	if (ei_local->chip_name == MT7622_FE)
		iext_port = 0U;
	else if (ei_local->chip_name == LEOPARD_FE)
		iext_port = 1U;
	else
		return -EINVAL;

	fe_cal_vbg_flag = 0;
	fe_cal_r50_flag = 0;
	fe_cal_tx_offset_flag = 0;
	fe_cal_tx_offset_flag_mdix = 0;
	fe_cal_flag = 0;
	fe_cal_flag_mdix = 0;

	/* Save each local register only after a successful read. */
	status = tc_phy_read_page_reg_checked(port_num, 0U, 26U, 1, &read_value);
	if (status != 0)
		goto cleanup;
	l0r26_temp = (u16)read_value;
	l0r26_saved = 1;
	status = tc_phy_read_page_reg_checked(port_num, 0U, 30U, 1, &read_value);
	if (status != 0)
		goto cleanup;
	l0r30_temp = (u16)read_value;
	l0r30_saved = 1;

	/* Set before issuing MDIO: a failed write may still have reached hardware. */
	setup_attempted = 1;
	status = tc_phy_write_page_reg_checked(port_num, 0U, 26U, 1, 0x5600U);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_page_reg_checked(port_num, 4U, 21U, 1, 0x0000U);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_page_reg_checked(port_num, 0U, 0U, 1, 0x2100U);
	if (status != 0)
		goto cleanup;

	for (j = 0; j < ARRAY_SIZE(eye_regs); ++j) {
		status = tc_phy_write_page_reg_checked(
			port_num, 5U, eye_regs[j], 0, eye_values[j]);
		if (status != 0)
			goto cleanup;
	}
	for (i = port_num; i < 5U; ++i) {
		for (j = 0; j < ARRAY_SIZE(fe_defaults); ++j) {
			status = tc_phy_write_page_reg_checked(
				i, fe_defaults[j].page, fe_defaults[j].reg,
				fe_defaults[j].local, fe_defaults[j].value);
			if (status != 0)
				goto cleanup;
		}
	}
	if (ei_local->chip_name == LEOPARD_FE) {
		status = tc_phy_write_page_reg_checked(
			port_num, 0U, 20U, 1, 0x0c0cU);
		if (status != 0)
			goto cleanup;
		for (j = 0; j < ARRAY_SIZE(leopard_zero_dev_regs); ++j) {
			status = tc_phy_write_dev_reg_checked(
				0U, 0x1eU, leopard_zero_dev_regs[j], 0U);
			if (status != 0)
				goto cleanup;
		}
		for (j = 0; j < ARRAY_SIZE(leopard_eye_regs); ++j) {
			status = tc_phy_write_page_reg_checked(
				1U, 5U, leopard_eye_regs[j], 0,
				leopard_eye_values[j]);
			if (status != 0)
				goto cleanup;
		}
	}

	if (port_num == iext_port) {
		failed_stage = "VBG";
		for (attempt = 0; attempt < 3U && !fe_cal_vbg_flag; ++attempt) {
			t_s = jiffies;
			fe_cal_vbg(port_num, 1U);
			t_e = jiffies;
			if (show_time)
				pr_info("port[%u] fe_cal_vbg time = %lu\n",
					port_num, (t_e - t_s) * 4UL);
		}
		if (!fe_cal_vbg_flag) {
			status = -EIO;
			goto cleanup;
		}
		fe_cal_vbg_flag = 0;
	}

	failed_stage = "R50";
	for (attempt = 0; attempt < 3U && !fe_cal_r50_flag; ++attempt) {
		t_s = jiffies;
		fe_cal_r50(port_num, 1U);
		t_e = jiffies;
		if (show_time)
			pr_info("port[%u] fe_cal_r50 time = %lu\n",
				port_num, (t_e - t_s) * 4UL);
	}
	if (!fe_cal_r50_flag) {
		status = -EIO;
		goto cleanup;
	}
	fe_cal_r50_flag = 0;

	failed_stage = "TX offset";
	for (attempt = 0; attempt < 3U && !fe_cal_tx_offset_flag; ++attempt) {
		t_s = jiffies;
		fe_cal_tx_offset(port_num, CALDLY);
		t_e = jiffies;
		if (show_time)
			pr_info("port[%u] fe_cal_tx_offset time = %lu\n",
				port_num, (t_e - t_s) * 4UL);
	}
	if (!fe_cal_tx_offset_flag) {
		status = -EIO;
		goto cleanup;
	}
	fe_cal_tx_offset_flag = 0;

	failed_stage = "TX offset MDIX";
	for (attempt = 0; attempt < 3U && !fe_cal_tx_offset_flag_mdix; ++attempt) {
		t_s = jiffies;
		fe_cal_tx_offset_mdix(port_num, CALDLY);
		t_e = jiffies;
		if (show_time)
			pr_info("port[%u] fe_cal_tx_offset_mdix time = %lu\n",
				port_num, (t_e - t_s) * 4UL);
	}
	if (!fe_cal_tx_offset_flag_mdix) {
		status = -EIO;
		goto cleanup;
	}
	fe_cal_tx_offset_flag_mdix = 0;

	failed_stage = "TX amplitude";
	for (attempt = 0; attempt < 3U && !fe_cal_flag; ++attempt) {
		t_s = jiffies;
		fe_cal_tx_amp(port_num, CALDLY);
		t_e = jiffies;
		if (show_time)
			pr_info("port[%u] fe_cal_tx_amp time = %lu\n",
				port_num, (t_e - t_s) * 4UL);
	}
	if (!fe_cal_flag) {
		status = -EIO;
		goto cleanup;
	}
	fe_cal_flag = 0;

	failed_stage = "TX amplitude MDIX";
	for (attempt = 0; attempt < 3U && !fe_cal_flag_mdix; ++attempt) {
		t_s = jiffies;
		fe_cal_tx_amp_mdix(port_num, CALDLY);
		t_e = jiffies;
		if (show_time)
			pr_info("port[%u] fe_cal_tx_amp_mdix time = %lu\n",
				port_num, (t_e - t_s) * 4UL);
	}
	if (!fe_cal_flag_mdix) {
		status = -EIO;
		goto cleanup;
	}
	fe_cal_flag_mdix = 0;

cleanup:
	cleanup_context.port_num = port_num;
	cleanup_context.l0r26_temp = l0r26_temp;
	cleanup_context.l0r30_temp = l0r30_temp;
	cleanup_context.l0r26_saved = l0r26_saved;
	cleanup_context.l0r30_saved = l0r30_saved;
	cleanup_status = ra_switch_mdio_cleanup_if_started(
		setup_attempted, ra_switch_fe_analog_cal_cleanup,
		&cleanup_context);
	if (status == 0 && cleanup_status != 0)
		status = cleanup_status;

	fe_cal_vbg_flag = 0;
	fe_cal_r50_flag = 0;
	fe_cal_tx_offset_flag = 0;
	fe_cal_tx_offset_flag_mdix = 0;
	fe_cal_flag = 0;
	fe_cal_flag_mdix = 0;
	if (status != 0)
		pr_err_ratelimited(
			"%s: FE all-analog calibration failed (port=%u stage=%s status=%d)\n",
			__func__, port_num, failed_stage, status);
	return status;
}

/* Keep the public void entry point for existing platform initialization callers. */
void do_fe_phy_all_analog_cal(u8 port_num)
{
	int status = do_fe_phy_all_analog_cal_checked(port_num);

	if (status != 0)
		pr_err_ratelimited(
			"%s: calibration request for FE port %u did not complete (status=%d)\n",
			__func__, port_num, status);
}

struct ra_switch_ge_cal_wait_context {
	unsigned int delay;
};

static int ra_switch_ge_cal_wait_read(void *context, unsigned int port,
				      unsigned int reg, unsigned int *value)
{
	(void)context;
	return tc_phy_read_page_reg_checked(port, 7U, (u8)reg, 0, value);
}

static int ra_switch_ge_cal_wait_write(void *context, unsigned int port,
				       unsigned int reg, unsigned int value)
{
	(void)context;
	return tc_phy_write_page_reg_checked(port, 7U, (u8)reg, 0, value);
}

static void ra_switch_ge_cal_wait_delay(void *context)
{
	struct ra_switch_ge_cal_wait_context *wait = context;

	udelay(wait->delay);
}

static int all_ge_ana_cal_wait_checked(unsigned int delay, u8 port_num,
				       u8 *ready)
{
	struct ra_switch_ge_cal_wait_context wait;
	unsigned int sample_ready = 0;
	int status;

	if (!ready)
		return -EINVAL;
	*ready = 0;
	wait.delay = delay;
	status = ra_switch_mdio_calibration_wait(
		FE_CAL_COMMON, 24U, 0x10U, 0x2U, 1000U,
		ra_switch_ge_cal_wait_read, ra_switch_ge_cal_wait_write,
		&wait, ra_switch_ge_cal_wait_delay, &wait, &sample_ready);
	if (status == 0)
		*ready = (u8)sample_ready;
	else
		pr_err_ratelimited("%s: GE analog calibration MDIO wait failed (port=%u status=%d)\n",
				   __func__, port_num, status);
	return status;
}

u8 all_ge_ana_cal_wait(unsigned int delay, u8 port_num) /* for EN7512 */
{
	u8 ready = 0;
	int status;

	status = all_ge_ana_cal_wait_checked(delay, port_num, &ready);
	if (status != 0)
		return ANACAL_ERROR;
	return ready;
}
void ge_cal_rext(u8 phyaddr, unsigned int delay)
{
	u8 rg_zcal_ctrl, all_ana_cal_status;
	u8 ready = 0;
	u32 ad_cal_comp_out_init;
	u32 dev1e_e0_ana_cal_r5 = 0;
	u32 dev1e_17a_tmp, dev1e_e0_tmp;
	int calibration_polarity;
	u8 cnt = 0;
	int status = 0;
	int cleanup_status;
	int snapshot_valid = 0;
	int keep_calibrated = 0;
	unsigned int restore_value;

	/* analog calibration enable, Rext calibration enable */
	/* 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen, [0]:rg_zcalen_a */
	/* 1e_dc[0]:rg_txvos_calen */
	/* 1e_e1[4]:rg_cal_refsel(0:1.2V) */
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x00db, 0x1110);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x00dc, 0x0000);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x00e1, 0x0000);
	if (status != 0)
		goto cleanup;

	/* Save the Rext control immediately before the first write to it. */
	status = tc_phy_read_dev_reg_checked(phyaddr, 0x1e, 0x00e0,
					     &dev1e_e0_ana_cal_r5);
	if (status != 0)
		goto cleanup;
	snapshot_valid = 1;

	rg_zcal_ctrl = 0x20; /* start with 0 dB */
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x00e0,
					       rg_zcal_ctrl);
	if (status != 0)
		goto cleanup;
	status = all_ge_ana_cal_wait_checked(delay, phyaddr, &ready);
	if (status != 0)
		goto cleanup;
	if (!ready) {
		all_ana_cal_status = ANACAL_ERROR;
		pr_info(" GE Rext AnaCal ERROR!   \r\n");
		goto cleanup;
	}
	all_ana_cal_status = ANACAL_INIT;

	/* 1e_17a[8]:ad_cal_comp_out */
	status = tc_phy_read_dev_reg_checked(phyaddr, 0x1e, 0x017a,
					     &ad_cal_comp_out_init);
	if (status != 0)
		goto cleanup;
	ad_cal_comp_out_init = (ad_cal_comp_out_init >> 8) & 0x1;
	if (ad_cal_comp_out_init == 1)
		calibration_polarity = -1;
	else
		calibration_polarity = 1;

	cnt = 0;
	while (all_ana_cal_status < ANACAL_ERROR) {
		cnt++;
		rg_zcal_ctrl += calibration_polarity;
		status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x00e0,
						       rg_zcal_ctrl);
		if (status != 0)
			goto cleanup;
		status = all_ge_ana_cal_wait_checked(delay, phyaddr, &ready);
		if (status != 0)
			goto cleanup;
		if (!ready) {
			all_ana_cal_status = ANACAL_ERROR;
			pr_info("  GE Rext AnaCal ERROR!   \r\n");
			break;
		}
		status = tc_phy_read_dev_reg_checked(phyaddr, 0x1e, 0x017a,
						     &dev1e_17a_tmp);
		if (status != 0)
			goto cleanup;
		if (((dev1e_17a_tmp >> 8) & 0x1) != ad_cal_comp_out_init) {
			all_ana_cal_status = ANACAL_FINISH;
			pr_info("  GE Rext AnaCal Done! (%d)(0x%x)  \r\n",
				cnt, rg_zcal_ctrl);
		} else {
			status = tc_phy_read_dev_reg_checked(phyaddr, 0x1e,
							     0x017a,
							     &dev1e_17a_tmp);
			if (status != 0)
				goto cleanup;
			status = tc_phy_read_dev_reg_checked(phyaddr, 0x1e,
							     0x00e0,
							     &dev1e_e0_tmp);
			if (status != 0)
				goto cleanup;
			if ((rg_zcal_ctrl == 0x3F) || (rg_zcal_ctrl == 0x00)) {
				all_ana_cal_status = ANACAL_SATURATION;
				pr_info(" GE Rext AnaCal Saturation!  \r\n");
				rg_zcal_ctrl = 0x20; /* 0 dB */
			} else {
				pr_info(" GE Rxet cal (%d)(%d)(%d)(0x%x)  \r\n",
					cnt, ad_cal_comp_out_init,
					((dev1e_17a_tmp >> 8) & 0x1),
					dev1e_e0_tmp);
			}
		}
	}

	if (all_ana_cal_status != ANACAL_ERROR) {
		status = tc_phy_write_dev_reg_checked(
			phyaddr, 0x1e, 0x00e0,
			dev1e_e0_ana_cal_r5 | rg_zcal_ctrl);
		if (status != 0)
			goto cleanup;
		status = tc_phy_write_dev_reg_checked(
			phyaddr, 0x1e, 0x00e0,
			(rg_zcal_ctrl << 8) | rg_zcal_ctrl);
		if (status != 0)
			goto cleanup;
		/* **** 1f_115[2:0] = rg_zcal_ctrl[5:3] // Mog review */
		status = tc_phy_write_dev_reg_checked(
			phyaddr, 0x1f, 0x0115,
			((rg_zcal_ctrl & 0x3f) >> 3));
		if (status != 0)
			goto cleanup;
		keep_calibrated = 1;
	}

cleanup:
	/*
	 * On a failed or timed-out calibration, restore the saved Rext control
	 * to the existing 0 dB fallback. A completed calibration keeps its
	 * result. In either case, always attempt to disable the engine.
	 */
	restore_value = dev1e_e0_ana_cal_r5 | 0x20U;
	cleanup_status = ra_switch_mdio_restore_cl45_and_disable(
		phyaddr, 0x1eU, 0x00e0U, restore_value,
		snapshot_valid && !keep_calibrated,
		0x1eU, 0x00dbU,
		ra_switch_mdio_write_cl45_cb, NULL);
	if (status == 0 && cleanup_status != 0)
		status = cleanup_status;
	if (status != 0) {
		pr_err_ratelimited("%s: GE Rext calibration transaction failed (port=%u status=%d cleanup=%d)\n",
				   __func__, phyaddr, status, cleanup_status);
		return;
	}
	if (keep_calibrated) {
		pr_info("  GE Rext AnaCal Done! (%d)(0x%x)  \r\n",
			cnt, rg_zcal_ctrl);
		ge_cal_flag = 1;
	}
}
/*
 * Shared FE/GE R50 setup is used by the GE calibration path too.
 * Keep this checked helper separate from the public legacy wrapper so
 * GE calibration can abort before touching per-pair registers on failure.
 */
static int fe_ge_r50_common_checked(u8 port_num)
{
	struct ra_switch_page_update_context page;
	unsigned int updated;
	int status;

	status = tc_phy_write_page_reg_checked(
		port_num, 0U, 0U, 1, 0x2100U);
	if (status != 0)
		return status;
	status = tc_phy_write_page_reg_checked(
		FE_CAL_COMMON, 2U, 25U, 0, 0xf0c0U);
	if (status != 0)
		return status;
	status = tc_phy_write_page_reg_checked(
		FE_CAL_COMMON, 7U, 24U, 0, 0U);
	if (status != 0)
		return status;

	page.page = 7U;
	page.local = 0;
	status = ra_switch_mdio_update_bits_checked(
		FE_CAL_COMMON, 24U, 0U, 0x2000U,
		ra_switch_page_update_read, ra_switch_page_update_write,
		&page, &updated);
	if (status != 0)
		return status;
	status = ra_switch_mdio_update_bits_checked(
		FE_CAL_COMMON, 24U, 0U, 0x4000U,
		ra_switch_page_update_read, ra_switch_page_update_write,
		&page, &updated);
	if (status != 0)
		return status;
	status = ra_switch_mdio_update_bits_checked(
		FE_CAL_COMMON, 24U, 0x1000U, 0U,
		ra_switch_page_update_read, ra_switch_page_update_write,
		&page, &updated);
	if (status != 0)
		return status;

	page.page = 4U;
	page.local = 1;
	status = ra_switch_mdio_update_bits_checked(
		port_num, 23U, 0x0001U, 0U,
		ra_switch_page_update_read, ra_switch_page_update_write,
		&page, &updated);
	if (status != 0)
		return status;

	page.page = 3U;
	status = ra_switch_mdio_update_bits_checked(
		FE_CAL_COMMON, 25U, 0x2000U, 0U,
		ra_switch_page_update_read, ra_switch_page_update_write,
		&page, &updated);
	if (status != 0)
		return status;

	return ra_switch_mdio_update_bits_checked(
		port_num, 25U, 0U, 0x1000U,
		ra_switch_page_update_read, ra_switch_page_update_write,
		&page, &updated);
}

/* Preserve the pre-existing public void entry point for external callers. */
void fe_ge_r50_common(u8 port_num)
{
	int status = fe_ge_r50_common_checked(port_num);

	if (status != 0)
		pr_err_ratelimited(
			"%s: GE R50 common setup failed (port=%u status=%d)\n",
			__func__, port_num, status);
}

void ge_cal_r50(u8 phyaddr, unsigned int delay)
{
	u8 rg_zcal_ctrl, rg_zcal_ctrl_filter, ready;
	unsigned int ad_cal_comp_out_init = 0;
	unsigned int page_value, updated, dev1e_174_tmp, dev1e_175_tmp;
	unsigned int val_tmp, readback;
	unsigned int pair_select;
	unsigned int cal_pair, i;
	int calibration_polarity;
	unsigned int cnt;
	int all_ana_cal_status;
	int status = 0;
	int cleanup_status;
	int disable_status;
	struct ra_switch_page_update_context page;

	/* *** R50 Cal start***************** */
	status = fe_ge_r50_common_checked(phyaddr);
	if (status != 0)
		goto cleanup;

	/* Disable RG_ZCALEN on ports 1..4 before selecting each calibration pair. */
	page.page = 3U;
	page.local = 1;
	for (i = 1U; i <= 4U; ++i) {
		status = ra_switch_mdio_update_bits_checked(
			i, 25U, 0x1000U, 0U, ra_switch_page_update_read,
			ra_switch_page_update_write, &page, &updated);
		if (status != 0)
			goto cleanup;
	}

	page.page = 7U;
	page.local = 0;
	for (cal_pair = ANACAL_PAIR_A; cal_pair <= ANACAL_PAIR_D; ++cal_pair) {
		rg_zcal_ctrl = 0x20; /* start with 0 dB */
		switch (cal_pair) {
		case ANACAL_PAIR_A:
			pair_select = 0x1000U;
			break;
		case ANACAL_PAIR_B:
			pair_select = 0x0100U;
			break;
		case ANACAL_PAIR_C:
			pair_select = 0x0010U;
			break;
		default:
			pair_select = 0x0001U;
			break;
		}
		status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x00dd,
						       pair_select);
		if (status != 0)
			goto cleanup;

		status = ra_switch_mdio_update_bits_checked(
			FE_CAL_COMMON, 24U, 0x0fc0U,
			((unsigned int)(rg_zcal_ctrl & 0x3fU) << 6),
			ra_switch_page_update_read, ra_switch_page_update_write,
			&page, &updated);
		if (status != 0)
			goto cleanup;

		/* Wait for AD_CAL_COMP_OUT and distinguish timeout from MDIO failure. */
		all_ana_cal_status = ANACAL_INIT;
		ready = 0;
		status = all_ge_ana_cal_wait_checked(delay, phyaddr, &ready);
		if (status != 0)
			goto cleanup;
		if (!ready) {
			all_ana_cal_status = ANACAL_ERROR;
			pr_info(" GE R50 AnaCal ERROR! (init)   \r\n");
		} else {
			status = tc_phy_read_page_reg_checked(
				FE_CAL_COMMON, 7U, 24U, 0, &page_value);
			if (status != 0)
				goto cleanup;
			ad_cal_comp_out_init = page_value & 0x1U;
			if (ad_cal_comp_out_init == 1U)
				calibration_polarity = -1;
			else
				calibration_polarity = 1;

			cnt = 0U;
			while ((all_ana_cal_status < ANACAL_ERROR) && (cnt < 254U)) {
				++cnt;
				rg_zcal_ctrl = (u8)(rg_zcal_ctrl +
						    calibration_polarity);
				status = ra_switch_mdio_update_bits_checked(
					FE_CAL_COMMON, 24U, 0x0fc0U,
					((unsigned int)(rg_zcal_ctrl & 0x3fU) << 6),
					ra_switch_page_update_read,
					ra_switch_page_update_write, &page,
					&updated);
				if (status != 0)
					goto cleanup;
				ready = 0;
				status = all_ge_ana_cal_wait_checked(
					delay, phyaddr, &ready);
				if (status != 0)
					goto cleanup;
				if (!ready) {
					all_ana_cal_status = ANACAL_ERROR;
					pr_info(" GE R50 AnaCal ERROR! (%u)  \r\n",
						cnt);
					break;
				}
				status = tc_phy_read_page_reg_checked(
					FE_CAL_COMMON, 7U, 24U, 0, &page_value);
				if (status != 0)
					goto cleanup;
				if ((page_value & 0x1U) != ad_cal_comp_out_init) {
					all_ana_cal_status = ANACAL_FINISH;
				} else if ((rg_zcal_ctrl == 0x3fU) ||
					   (rg_zcal_ctrl == 0x00U)) {
					all_ana_cal_status = ANACAL_SATURATION;
					pr_info(" GE R50 Cal Sat! rg_zcal_ctrl = 0x%x(%u)\n",
						rg_zcal_ctrl, cnt);
				}
			}
		}

		if ((all_ana_cal_status == ANACAL_ERROR) ||
		    (all_ana_cal_status == ANACAL_SATURATION)) {
			rg_zcal_ctrl = 0x20; /* 0 dB */
			rg_zcal_ctrl_filter = 8U; /* default value */
		} else {
			rg_zcal_ctrl_filter = rg_zcal_ctrl;
			rg_zcal_ctrl = ZCAL_TO_R50ohm_GE_TBL[rg_zcal_ctrl];
			rg_zcal_ctrl_filter =
				ZCAL_TO_FILTER_TBL[rg_zcal_ctrl_filter];
			rg_zcal_ctrl_filter &= 0x0fU;
			rg_zcal_ctrl_filter = (u8)((rg_zcal_ctrl_filter << 8) |
						   rg_zcal_ctrl_filter);
		}

		if (all_ana_cal_status == ANACAL_FINISH) {
			if (cal_pair == ANACAL_PAIR_A) {
				status = tc_phy_read_dev_reg_checked(
					phyaddr, 0x1e, 0x0174, &dev1e_174_tmp);
				if (status != 0)
					goto cleanup;
				dev1e_174_tmp &= ~0xff00U;
				if (rg_zcal_ctrl > 4U)
					val_tmp = (((rg_zcal_ctrl - 4U) << 8) &
						   0xff00U) | dev1e_174_tmp;
				else
					val_tmp = dev1e_174_tmp;
				status = tc_phy_write_dev_reg_checked(
					phyaddr, 0x1e, 0x0174, val_tmp);
				if (status != 0)
					goto cleanup;
				status = tc_phy_write_dev_reg_checked(
					phyaddr, 0x1e, 0x03a0,
					rg_zcal_ctrl_filter);
				if (status != 0)
					goto cleanup;
				status = tc_phy_read_dev_reg_checked(
					phyaddr, 0x1e, 0x0174, &readback);
				if (status != 0)
					goto cleanup;
				pr_info("R50_PAIR_A : 1e_174 = 0x%x\n", readback);
				status = tc_phy_read_dev_reg_checked(
					phyaddr, 0x1e, 0x03a0, &readback);
				if (status != 0)
					goto cleanup;
				pr_info("R50_PAIR_A : 1e_3a0 = 0x%x\n", readback);
			} else if (cal_pair == ANACAL_PAIR_B) {
				status = tc_phy_read_dev_reg_checked(
					phyaddr, 0x1e, 0x0174, &dev1e_174_tmp);
				if (status != 0)
					goto cleanup;
				dev1e_174_tmp &= ~0x007fU;
				if (rg_zcal_ctrl > 2U)
					val_tmp = ((rg_zcal_ctrl - 2U) & 0xffU) |
						  dev1e_174_tmp;
				else
					val_tmp = dev1e_174_tmp;
				status = tc_phy_write_dev_reg_checked(
					phyaddr, 0x1e, 0x0174, val_tmp);
				if (status != 0)
					goto cleanup;
				status = tc_phy_write_dev_reg_checked(
					phyaddr, 0x1e, 0x03a1,
					rg_zcal_ctrl_filter);
				if (status != 0)
					goto cleanup;
				status = tc_phy_read_dev_reg_checked(
					phyaddr, 0x1e, 0x0174, &readback);
				if (status != 0)
					goto cleanup;
				pr_info("R50_PAIR_B : 1e_174 = 0x%x\n", readback);
				status = tc_phy_read_dev_reg_checked(
					phyaddr, 0x1e, 0x03a1, &readback);
				if (status != 0)
					goto cleanup;
				pr_info("R50_PAIR_B : 1e_3a1 = 0x%x\n", readback);
			} else if (cal_pair == ANACAL_PAIR_C) {
				status = tc_phy_read_dev_reg_checked(
					phyaddr, 0x1e, 0x0175, &dev1e_175_tmp);
				if (status != 0)
					goto cleanup;
				dev1e_175_tmp &= ~0x7f00U;
				if (rg_zcal_ctrl > 4U)
					val_tmp = dev1e_175_tmp |
						(((rg_zcal_ctrl - 4U) << 8) &
						 0xff00U);
				else
					val_tmp = dev1e_175_tmp;
				status = tc_phy_write_dev_reg_checked(
					phyaddr, 0x1e, 0x0175, val_tmp);
				if (status != 0)
					goto cleanup;
				status = tc_phy_write_dev_reg_checked(
					phyaddr, 0x1e, 0x03a2,
					rg_zcal_ctrl_filter);
				if (status != 0)
					goto cleanup;
				status = tc_phy_read_dev_reg_checked(
					phyaddr, 0x1e, 0x0175, &readback);
				if (status != 0)
					goto cleanup;
				pr_info("R50_PAIR_C : 1e_175 = 0x%x\n", readback);
				status = tc_phy_read_dev_reg_checked(
					phyaddr, 0x1e, 0x03a2, &readback);
				if (status != 0)
					goto cleanup;
				pr_info("R50_PAIR_C : 1e_3a2 = 0x%x\n", readback);
			} else {
				status = tc_phy_read_dev_reg_checked(
					phyaddr, 0x1e, 0x0175, &dev1e_175_tmp);
				if (status != 0)
					goto cleanup;
				dev1e_175_tmp &= ~0x007fU;
				if (rg_zcal_ctrl > 6U)
					val_tmp = dev1e_175_tmp |
						((rg_zcal_ctrl - 6U) & 0xffU);
				else
					val_tmp = dev1e_175_tmp;
				status = tc_phy_write_dev_reg_checked(
					phyaddr, 0x1e, 0x0175, val_tmp);
				if (status != 0)
					goto cleanup;
				status = tc_phy_write_dev_reg_checked(
					phyaddr, 0x1e, 0x03a3,
					rg_zcal_ctrl_filter);
				if (status != 0)
					goto cleanup;
				status = tc_phy_read_dev_reg_checked(
					phyaddr, 0x1e, 0x0175, &readback);
				if (status != 0)
					goto cleanup;
				pr_info("R50_PAIR_D : 1e_175 = 0x%x\n", readback);
				status = tc_phy_read_dev_reg_checked(
					phyaddr, 0x1e, 0x03a3, &readback);
				if (status != 0)
					goto cleanup;
				pr_info("R50_PAIR_D : 1e_3a3 = 0x%x\n", readback);
			}
		}
	}

cleanup:
	/* Always clear common analog bits and the R50 calibration enable. */
	cleanup_status = clear_ckinv_ana_txvos_checked();
	if (status == 0 && cleanup_status != 0)
		status = cleanup_status;
	disable_status = tc_phy_write_dev_reg_checked(
		phyaddr, 0x1e, 0x00db, 0x0000);
	if (status == 0 && disable_status != 0)
		status = disable_status;
	if (status != 0) {
		pr_err_ratelimited("%s: GE R50 calibration transaction failed (port=%u status=%d cleanup=%d disable=%d)\n",
				   __func__, phyaddr, status, cleanup_status,
				   disable_status);
		return;
	}
	ge_cal_r50_flag = 1;
	/* *** R50 Cal end *** */
}
void ge_cal_tx_amp(u8 phyaddr, unsigned int delay)
{
	u8	all_ana_cal_status, ready = 0;
	u16	ad_cal_comp_out_init;
	int	calibration_polarity;
	u16	cal_pair;
	u8	tx_amp_reg_shift;
	u16	reg_temp, val_tmp, l3r25_temp, val_tmp_100;
	u8	tx_amp_temp, tx_amp_reg, cnt = 0, tx_amp_reg_100;

	u16	tx_amp_temp_L, tx_amp_temp_M;
	u16	tx_amp_L_100, tx_amp_M_100;
	unsigned int read_value = 0, page_value = 0;
	int status = 0, cleanup_status;
	static const unsigned int tx_amp_cleanup_dev1e_regs[] = {
		0x017dU, 0x017eU, 0x017fU, 0x0180U,
		0x0181U, 0x0182U, 0x0183U, 0x0184U,
		0x00c9U, 0x00dbU, 0x00dcU, 0x003eU, 0x00ddU
	};
	static const unsigned int tx_amp_cleanup_dev1e_values[] = {
		0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U,
		0x0fffU, 0U, 0U, 0U, 0U
	};
	static const unsigned int tx_amp_cleanup_dev1f_regs[] = { 0x0273U };
	static const unsigned int tx_amp_cleanup_dev1f_values[] = { 0x2000U };
	/* *** Tx Amp Cal start ***/
	status = tc_phy_write_page_reg_checked(0, 0, 0, 1, 0x0140);
	if (status != 0)
		goto cleanup;

	status = tc_phy_write_dev_reg_checked(0, 0x1e, 0x3e, 0xf808);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_dev_reg_checked(0, 0x1e, 0x145, 0x5010);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_dev_reg_checked(0, 0x1e, 0x17d, 0x80f0);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_dev_reg_checked(0, 0x1e, 0x17e, 0x80f0);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_dev_reg_checked(0, 0x1e, 0x17f, 0x80f0);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_dev_reg_checked(0, 0x1e, 0x180, 0x80f0);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_dev_reg_checked(0, 0x1e, 0x181, 0x80f0);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_dev_reg_checked(0, 0x1e, 0x182, 0x80f0);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_dev_reg_checked(0, 0x1e, 0x183, 0x80f0);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_dev_reg_checked(0, 0x1e, 0x184, 0x80f0);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_dev_reg_checked(0, 0x1e, 0x00db, 0x1000);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_dev_reg_checked(0, 0x1e, 0x00dc, 0x0001);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_dev_reg_checked(0, 0x1f, 0x300, 0x4);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_dev_reg_checked(0, 0x1f, 0x27a, 0x33);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_page_reg_checked(1, 2, 25, 0, 0xf020);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_dev_reg_checked(0, 0x1f, 0x300, 0x14);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_page_reg_checked(FE_CAL_COMMON, 7, 24, 0, 0x7000);
	if (status != 0)
		goto cleanup;
	status = tc_phy_read_page_reg_checked(FE_CAL_COMMON, 3, 25, 1, &read_value);
	if (status != 0)
		goto cleanup;
	l3r25_temp = (u16)read_value;
	l3r25_temp = l3r25_temp | 0x200;
	status = tc_phy_write_page_reg_checked(FE_CAL_COMMON, 3, 25, 1, l3r25_temp);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x11, 0xff00);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1f, 0x273, 0);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0xc9, 0xffff);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_page_reg_checked(1, 2, 25, 0, 0xb020);
	if (status != 0)
		goto cleanup;

	for (cal_pair = ANACAL_PAIR_A; cal_pair <= ANACAL_PAIR_D; cal_pair++) {
		tx_amp_temp = 0x20;	/* start with 0 dB */
		status = tc_phy_write_page_reg_checked(FE_CAL_COMMON, 7, 24, 0, 0x7000);
		if (status != 0)
			goto cleanup;
		if (cal_pair == ANACAL_PAIR_A) {
			status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x00dd, 0x1000);
			if (status != 0)
				goto cleanup;
			status = tc_phy_read_dev_reg_checked(phyaddr, 0x1e, 0x012, &read_value);
			if (status != 0)
				goto cleanup;
			reg_temp = (u16)(read_value & (~0xfc00));
			tx_amp_reg_shift = 10;
			tx_amp_reg = 0x12;
			tx_amp_reg_100 = 0x16;
		} else if (cal_pair == ANACAL_PAIR_B) {
			status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x00dd, 0x0100);
			if (status != 0)
				goto cleanup;
			status = tc_phy_read_dev_reg_checked(phyaddr, 0x1e, 0x017, &read_value);
			if (status != 0)
				goto cleanup;
			reg_temp = (u16)(read_value & (~0x3f00));
			tx_amp_reg_shift = 8;
			tx_amp_reg = 0x17;
			tx_amp_reg_100 = 0x18;
		} else if (cal_pair == ANACAL_PAIR_C) {
			status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x00dd, 0x0010);
			if (status != 0)
				goto cleanup;
			status = tc_phy_read_dev_reg_checked(phyaddr, 0x1e, 0x019, &read_value);
			if (status != 0)
				goto cleanup;
			reg_temp = (u16)(read_value & (~0x3f00));
			tx_amp_reg_shift = 8;
			tx_amp_reg = 0x19;
			tx_amp_reg_100 = 0x20;
		} else {/* if(cal_pair == ANACAL_PAIR_D) */
			status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x00dd, 0x0001);
			if (status != 0)
				goto cleanup;
			status = tc_phy_read_dev_reg_checked(phyaddr, 0x1e, 0x021, &read_value);
			if (status != 0)
				goto cleanup;
			reg_temp = (u16)(read_value & (~0x3f00));
			tx_amp_reg_shift = 8;
			tx_amp_reg = 0x21;
			tx_amp_reg_100 = 0x22;
		}
		/* 1e_12, 1e_17, 1e_19, 1e_21 */
		val_tmp = tx_amp_temp | (tx_amp_temp << tx_amp_reg_shift);
		status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, tx_amp_reg, val_tmp);
		if (status != 0)
			goto cleanup;
		status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, tx_amp_reg_100, val_tmp);
		if (status != 0)
			goto cleanup;
		ready = 0;
		status = all_ge_ana_cal_wait_checked(delay, phyaddr, &ready);
		if (status != 0)
			goto cleanup;
		all_ana_cal_status = ready ? ANACAL_INIT : ANACAL_ERROR;
		if (all_ana_cal_status == 0) {
			all_ana_cal_status = ANACAL_ERROR;
			pr_info(" GE Tx amp AnaCal ERROR!   \r\n");
		}
/* 1e_17a[8]:ad_cal_comp_out */
		status = tc_phy_read_page_reg_checked(
			FE_CAL_COMMON, 7U, 24U, 0, &page_value);
		if (status != 0)
			goto cleanup;
		ad_cal_comp_out_init = (u16)(page_value & 0x1U);
		if (ad_cal_comp_out_init == 1)
			calibration_polarity = -1;
		else
			calibration_polarity = 1;

		cnt = 0;
		while ((all_ana_cal_status < ANACAL_ERROR) && (cnt < 254U)) {
			cnt++;
			tx_amp_temp += calibration_polarity;

			val_tmp = (tx_amp_temp | (tx_amp_temp << tx_amp_reg_shift));
			status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, tx_amp_reg, val_tmp);
			if (status != 0)
				goto cleanup;
			status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, tx_amp_reg_100, val_tmp);
			if (status != 0)
				goto cleanup;
			ready = 0;
			status = all_ge_ana_cal_wait_checked(delay, phyaddr, &ready);
			if (status != 0)
				goto cleanup;
			all_ana_cal_status = ready ? ANACAL_INIT : ANACAL_ERROR;
			if (all_ana_cal_status == 0) {
				all_ana_cal_status = ANACAL_ERROR;
				pr_info(" GE Tx amp AnaCal ERROR!\n");
			} else {
				status = tc_phy_read_page_reg_checked(
					FE_CAL_COMMON, 7U, 24U, 0, &page_value);
				if (status != 0)
					goto cleanup;
				if ((page_value & 0x1U) != ad_cal_comp_out_init) {
					all_ana_cal_status = ANACAL_FINISH;
				} else if ((tx_amp_temp == 0x3f) ||
					   (tx_amp_temp == 0x00)) {
					all_ana_cal_status = ANACAL_SATURATION;
					pr_info(" GE Tx amp AnaCal Saturation!  \\r\\n");
				}
			}
		}
		if ((all_ana_cal_status == ANACAL_ERROR) ||
		    (all_ana_cal_status == ANACAL_SATURATION) ||
		    ((all_ana_cal_status == ANACAL_INIT) && (cnt >= 254U))) {
			/* No comparator crossing: retain the known 0 dB TX amplitude. */
			all_ana_cal_status = ANACAL_ERROR;
			pr_info("GE TX amp calibration fallback (pair=%u)\\n",
				(unsigned int)cal_pair);
			tx_amp_temp = 0x20;
			val_tmp = (reg_temp | (tx_amp_temp << tx_amp_reg_shift));
			status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, tx_amp_reg, val_tmp);
			if (status != 0)
				goto cleanup;
		}

		if (all_ana_cal_status == ANACAL_FINISH) {
			if (cal_pair == ANACAL_PAIR_A) {
				tx_amp_temp_M = tx_amp_temp + 9;
				tx_amp_temp_L = tx_amp_temp + 18;
			} else if (cal_pair == ANACAL_PAIR_B) {
				tx_amp_temp_M = tx_amp_temp + 8;
				tx_amp_temp_L = tx_amp_temp + 22;
			} else if (cal_pair == ANACAL_PAIR_C) {
				tx_amp_temp_M = tx_amp_temp + 9;
				tx_amp_temp_L = tx_amp_temp + 9;
			} else if (cal_pair == ANACAL_PAIR_D) {
				tx_amp_temp_M = tx_amp_temp + 9;
				tx_amp_temp_L = tx_amp_temp + 9;
			}
			if (tx_amp_temp_L >= 0x3f)
				tx_amp_temp_L = 0x3f;
			if (tx_amp_temp_M >= 0x3f)
				tx_amp_temp_M = 0x3f;
			val_tmp = ((tx_amp_temp_L) |
				((tx_amp_temp_M) << tx_amp_reg_shift));
			if (cal_pair == ANACAL_PAIR_A) {
				if (tx_amp_temp < 6)
					tx_amp_M_100 = 0;
				else
					tx_amp_M_100 = tx_amp_temp - 6;

				if ((tx_amp_temp + 9) >= 0x3f)
					tx_amp_L_100 = 0x3f;
				else
					tx_amp_L_100 = tx_amp_temp + 9;
				val_tmp_100 = ((tx_amp_L_100) |
					((tx_amp_M_100) << tx_amp_reg_shift));
			} else if (cal_pair == ANACAL_PAIR_B) {
				if (tx_amp_temp < 7)
					tx_amp_M_100 = 0;
				else
					tx_amp_M_100 = tx_amp_temp - 7;

				if ((tx_amp_temp + 8) >= 0x3f)
					tx_amp_L_100 = 0x3f;
				else
					tx_amp_L_100 = tx_amp_temp + 8;
				val_tmp_100 = ((tx_amp_L_100) |
					((tx_amp_M_100) << tx_amp_reg_shift));
			} else if (cal_pair == ANACAL_PAIR_C) {
				if ((tx_amp_temp + 9) >= 0x3f)
					tx_amp_L_100 = 0x3f;
				else
					tx_amp_L_100 = tx_amp_temp + 9;
				tx_amp_M_100 = tx_amp_L_100;
				val_tmp_100 = ((tx_amp_L_100) |
					((tx_amp_M_100) << tx_amp_reg_shift));
			} else if (cal_pair == ANACAL_PAIR_D) {
				if ((tx_amp_temp + 9) >= 0x3f)
					tx_amp_L_100 = 0x3f;
				else
					tx_amp_L_100 = tx_amp_temp + 9;

				tx_amp_M_100 = tx_amp_L_100;
				val_tmp_100 = ((tx_amp_L_100) |
					((tx_amp_M_100) << tx_amp_reg_shift));
			}

			status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, tx_amp_reg, val_tmp);
			if (status != 0)
				goto cleanup;
			status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, tx_amp_reg_100, val_tmp_100);
			if (status != 0)
				goto cleanup;

			if (cal_pair == ANACAL_PAIR_A) {
				pr_info("TX_AMP_PAIR_A : 1e_%x = 0x%x\n",
					tx_amp_reg,
					val_tmp);
				pr_info("TX_AMP_PAIR_A : 1e_%x = 0x%x\n",
					tx_amp_reg_100,
					val_tmp_100);
			} else if (cal_pair == ANACAL_PAIR_B) {
				pr_info("TX_AMP_PAIR_B : 1e_%x = 0x%x\n",
					tx_amp_reg,
					val_tmp);
				pr_info("TX_AMP_PAIR_B : 1e_%x = 0x%x\n",
					tx_amp_reg_100,
					val_tmp_100);
			} else if (cal_pair == ANACAL_PAIR_C) {
				pr_info("TX_AMP_PAIR_C : 1e_%x = 0x%x\n",
					tx_amp_reg,
					val_tmp);
				pr_info("TX_AMP_PAIR_C : 1e_%x = 0x%x\n",
					tx_amp_reg_100,
					val_tmp_100);

			} else {/* if(cal_pair == ANACAL_PAIR_D) */
				pr_info("TX_AMP_PAIR_D : 1e_%x = 0x%x\n",
					tx_amp_reg,
					val_tmp);
				pr_info("TX_AMP_PAIR_D : 1e_%x = 0x%x\n",
					tx_amp_reg_100,
					val_tmp_100);
			}
		}
	}

cleanup:
	/* Attempt all independent shutdown writes and retain the first error. */
	cleanup_status = clear_ckinv_ana_txvos_checked();
	if (status == 0 && cleanup_status != 0)
		status = cleanup_status;

	cleanup_status = ra_switch_mdio_cleanup_cl45_writes(
		phyaddr, 0x1eU, tx_amp_cleanup_dev1e_regs,
		tx_amp_cleanup_dev1e_values,
		sizeof(tx_amp_cleanup_dev1e_regs) /
			sizeof(tx_amp_cleanup_dev1e_regs[0]),
		ra_switch_mdio_write_cl45_cb, NULL);
	if (status == 0 && cleanup_status != 0)
		status = cleanup_status;

	cleanup_status = ra_switch_mdio_cleanup_cl45_writes(
		phyaddr, 0x1fU, tx_amp_cleanup_dev1f_regs,
		tx_amp_cleanup_dev1f_values,
		sizeof(tx_amp_cleanup_dev1f_regs) /
			sizeof(tx_amp_cleanup_dev1f_regs[0]),
		ra_switch_mdio_write_cl45_cb, NULL);
	if (status == 0 && cleanup_status != 0)
		status = cleanup_status;

	cleanup_status = tc_phy_write_page_reg_checked(
		1U, 2U, 25U, 0, 0xb020U);
	if (status == 0 && cleanup_status != 0)
		status = cleanup_status;

	cleanup_status = tc_phy_write_dev_reg_checked(
		0U, 0x1eU, 0x0145U, 0x1000U);
	if (status == 0 && cleanup_status != 0)
		status = cleanup_status;

	if (status != 0) {
		pr_err_ratelimited("%s: GE TX amplitude calibration failed (port=%u status=%d)\\n",
				   __func__, phyaddr, status);
		return;
	}
	ge_cal_flag = 1;
	pr_info("GE_TX_AMP END\\n");
	/* *** Tx Amp Cal end *** */
}

void ge_cal_tx_offset(u8 phyaddr, unsigned int delay)
{
	u8	all_ana_cal_status, ready = 0;
	u16	ad_cal_comp_out_init;
	int	calibration_polarity, tx_offset_temp;
	u16	cal_pair, cal_temp;
	u8	tx_offset_reg_shift;
	u16	tx_offset_reg, reg_temp, val_tmp;
	u8	cnt = 0;
	unsigned int read_value = 0, page_value = 0;
	int status = 0, cleanup_status;
	static const unsigned int tx_offset_cleanup_regs[] = {
		0x017dU, 0x017eU, 0x017fU, 0x0180U,
		0x0181U, 0x0182U, 0x0183U, 0x0184U,
		0x00dbU, 0x00dcU, 0x003eU, 0x00ddU
	};
	static const unsigned int tx_offset_cleanup_values[] = {
		0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U,
		0U, 0U, 0U, 0U
	};

	status = tc_phy_write_page_reg_checked(0, 0, 0, 1, 0x2100);
	if (status != 0)
		goto cleanup;

	/* 1e_db[12]:rg_cal_ckinv, [8]:rg_ana_calen, [4]:rg_rext_calen, [0]:rg_zcalen_a */
	/* 1e_dc[0]:rg_txvos_calen */
	/* 1e_96[15]:bypass_tx_offset_cal, Hw bypass, Fw cal */
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x00db, 0x0100);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x00dc, 0x0001);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x0096, 0x8000);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x003e, 0xf808);/* 1e_3e */
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_page_reg_checked(FE_CAL_COMMON, 7, 24, 0, 0x3000);
	if (status != 0)
		goto cleanup;

	for (cal_pair = ANACAL_PAIR_A; cal_pair <= ANACAL_PAIR_D; cal_pair++) {
		tx_offset_temp = 0x20;

		if (cal_pair == ANACAL_PAIR_A) {
			status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x145, 0x5010);
			if (status != 0)
				goto cleanup;
			status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x00dd, 0x1000);
			if (status != 0)
				goto cleanup;
			status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x017d, (0x8000 | DAC_IN_0V));
			if (status != 0)
				goto cleanup;
			status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x0181, (0x8000 | DAC_IN_0V));
			if (status != 0)
				goto cleanup;
			status = tc_phy_read_dev_reg_checked(phyaddr, 0x1e, 0x0172, &read_value);
			if (status != 0)
				goto cleanup;
			reg_temp = (u16)(read_value & (~0x3f00));
			tx_offset_reg_shift = 8;/* 1e_172[13:8] */
			tx_offset_reg = 0x0172;

		} else if (cal_pair == ANACAL_PAIR_B) {
			status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x145, 0x5018);
			if (status != 0)
				goto cleanup;
			status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x00dd, 0x0100);
			if (status != 0)
				goto cleanup;
			status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x017e, (0x8000 | DAC_IN_0V));
			if (status != 0)
				goto cleanup;
			status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x0182, (0x8000 | DAC_IN_0V));
			if (status != 0)
				goto cleanup;
			status = tc_phy_read_dev_reg_checked(phyaddr, 0x1e, 0x0172, &read_value);
			if (status != 0)
				goto cleanup;
			reg_temp = (u16)(read_value & (~0x003f));
			tx_offset_reg_shift = 0;
			tx_offset_reg = 0x0172;/* 1e_172[5:0] */
		} else if (cal_pair == ANACAL_PAIR_C) {
			status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x00dd, 0x0010);
			if (status != 0)
				goto cleanup;
			status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x017f, (0x8000 | DAC_IN_0V));
			if (status != 0)
				goto cleanup;
			status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x0183, (0x8000 | DAC_IN_0V));
			if (status != 0)
				goto cleanup;
			status = tc_phy_read_dev_reg_checked(phyaddr, 0x1e, 0x0173, &read_value);
			if (status != 0)
				goto cleanup;
			reg_temp = (u16)(read_value & (~0x3f00));
			tx_offset_reg_shift = 8;
			tx_offset_reg = 0x0173;/* 1e_173[13:8] */
		} else {/* if(cal_pair == ANACAL_PAIR_D) */
			status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x00dd, 0x0001);
			if (status != 0)
				goto cleanup;
			status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x0180, (0x8000 | DAC_IN_0V));
			if (status != 0)
				goto cleanup;
			status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x0184, (0x8000 | DAC_IN_0V));
			if (status != 0)
				goto cleanup;
			status = tc_phy_read_dev_reg_checked(phyaddr, 0x1e, 0x0173, &read_value);
			if (status != 0)
				goto cleanup;
			reg_temp = (u16)(read_value & (~0x003f));
			tx_offset_reg_shift = 0;
			tx_offset_reg = 0x0173;/* 1e_173[5:0] */
		}
		/* 1e_172, 1e_173 */
		val_tmp =  (reg_temp | (tx_offset_temp << tx_offset_reg_shift));
		status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, tx_offset_reg, val_tmp);
		if (status != 0)
			goto cleanup;

		ready = 0;

		status = all_ge_ana_cal_wait_checked(delay, phyaddr, &ready);

		if (status != 0)

			goto cleanup;

		all_ana_cal_status = ready ? ANACAL_INIT : ANACAL_ERROR;
		if (all_ana_cal_status == 0) {
			all_ana_cal_status = ANACAL_ERROR;
			pr_info(" GE Tx offset AnaCal ERROR!   \r\n");
		}
		calibration_polarity = 1;
		if (all_ana_cal_status != ANACAL_ERROR) {
			status = tc_phy_read_page_reg_checked(
				FE_CAL_COMMON, 7U, 24U, 0, &page_value);
			if (status != 0)
				goto cleanup;
			ad_cal_comp_out_init = (u16)(page_value & 0x1U);
			if (ad_cal_comp_out_init == 1U)
				calibration_polarity = -1;
		}

		cnt = 0;
		tx_offset_temp += calibration_polarity;
		while ((all_ana_cal_status < ANACAL_ERROR) && (cnt < 254U)) {
			cnt++;
			cal_temp = tx_offset_temp;
			val_tmp = (reg_temp | (cal_temp << tx_offset_reg_shift));
			status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, tx_offset_reg, val_tmp);
			if (status != 0)
				goto cleanup;

			ready = 0;

			status = all_ge_ana_cal_wait_checked(delay, phyaddr, &ready);

			if (status != 0)

				goto cleanup;

			all_ana_cal_status = ready ? ANACAL_INIT : ANACAL_ERROR;
			if (all_ana_cal_status == 0) {
				all_ana_cal_status = ANACAL_ERROR;
				pr_info(" GE Tx offset AnaCal ERROR!   \r\n");
			} else {
				status = tc_phy_read_page_reg_checked(
					FE_CAL_COMMON, 7U, 24U, 0, &page_value);
				if (status != 0)
					goto cleanup;
				if ((page_value & 0x1U) != ad_cal_comp_out_init) {
					all_ana_cal_status = ANACAL_FINISH;
				} else if ((tx_offset_temp == 0x3f) ||
					   (tx_offset_temp == 0x00)) {
					all_ana_cal_status = ANACAL_SATURATION;
					pr_info("GE tx offset ANACAL_SATURATION\\n");
				} else {
					tx_offset_temp += calibration_polarity;
				}
			}
		}
		if ((all_ana_cal_status == ANACAL_ERROR) ||
		    (all_ana_cal_status == ANACAL_SATURATION) ||
		    ((all_ana_cal_status == ANACAL_INIT) && (cnt >= 254U))) {
			/* No comparator crossing: leave this pair at the known 0 dB code. */
			all_ana_cal_status = ANACAL_ERROR;
			tx_offset_temp = 0x20;
			val_tmp = (reg_temp | (tx_offset_temp << tx_offset_reg_shift));
			status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, tx_offset_reg, val_tmp);
			if (status != 0)
				goto cleanup;
		}

		if (all_ana_cal_status == ANACAL_FINISH) {
			if (cal_pair == ANACAL_PAIR_A) {
				pr_info("TX_OFFSET_PAIR_A : 1e_%x = 0x%x\n",
					tx_offset_reg,
				val_tmp);
			} else if (cal_pair == ANACAL_PAIR_B) {
				pr_info("TX_OFFSET_PAIR_B : 1e_%x = 0x%x\n",
					tx_offset_reg,
				val_tmp);
			} else if (cal_pair == ANACAL_PAIR_C) {
				pr_info("TX_OFFSET_PAIR_C : 1e_%x = 0x%x\n",
					tx_offset_reg,
				val_tmp);

			} else {/* if(cal_pair == ANACAL_PAIR_D) */
				pr_info("TX_OFFSET_PAIR_D : 1e_%x = 0x%x\n",
					tx_offset_reg,
				val_tmp);
			}
		}
	}
cleanup:
	/* Clear shared analog state and attempt every force-mode reset. */
	cleanup_status = clear_ckinv_ana_txvos_checked();
	if (status == 0 && cleanup_status != 0)
		status = cleanup_status;

	cleanup_status = ra_switch_mdio_cleanup_cl45_writes(
		phyaddr, 0x1eU, tx_offset_cleanup_regs,
		tx_offset_cleanup_values,
		sizeof(tx_offset_cleanup_regs) /
			sizeof(tx_offset_cleanup_regs[0]),
		ra_switch_mdio_write_cl45_cb, NULL);
	if (status == 0 && cleanup_status != 0)
		status = cleanup_status;

	if (status != 0) {
		pr_err_ratelimited("%s: GE TX offset calibration failed (port=%u status=%d)\\n",
				   __func__, phyaddr, status);
		return;
	}
	ge_cal_tx_offset_flag = 1;
}

int do_ge_phy_all_analog_cal(u8 phyaddr)
{
	u16	reg0_temp, dev1e_145_temp, reg_temp;
	u16	reg_tmp;
	u32 read_value = 0;
	int status = 0, cleanup_status;
	int reg0_saved = 0, dev145_saved = 0;

	status = tc_mii_write_checked(phyaddr, 0x1f, 0x0000);/* g0 */
	if (status != 0)
		goto cleanup;
	status = tc_mii_read_checked(phyaddr, 0x0, &read_value);/* keep the default value */
	if (status != 0)
		goto cleanup;
	reg0_temp = (u16)read_value;
	reg0_saved = 1;
/* set [12]AN disable, [8]full duplex, [13/6]1000Mbps */
	status = tc_mii_write_checked(phyaddr, 0x0, 0x0140);
	if (status != 0)
		goto cleanup;

	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1f, 0x0100, 0xc000);/* BG voltage output */
	if (status != 0)
		goto cleanup;
	status = tc_phy_read_dev_reg_checked(phyaddr, 0x1e, 0x0145, &read_value);
	if (status != 0)
		goto cleanup;
	dev1e_145_temp = (u16)read_value;
	dev145_saved = 1;
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x0145, 0x1010);/* fix mdi */
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x0185, 0x0000);/* disable tx slew control */
	if (status != 0)
		goto cleanup;

	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1f, 0x27c, 0x1f1f);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1f, 0x27c, 0x3300);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1f, 0x273, 0);
	if (status != 0)
		goto cleanup;

	status = tc_phy_read_dev_reg_checked(phyaddr, 0x1e, 0x11, &read_value);

	if (status != 0)

		goto cleanup;

	reg_tmp = (u16)read_value;
	reg_tmp = reg_tmp | (0xf << 12);
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x11, reg_tmp);
	if (status != 0)
		goto cleanup;

	/* calibration start ============ */
	ge_cal_flag = 1; /*GE calibration not calibration*/
	while (ge_cal_flag == 0)
		ge_cal_rext(phyaddr, 100);

	/* *** R50 Cal start ***************************** */
	/*phyaddress = 0*/
	ge_cal_r50_flag = 0;
	ge_cal_r50(phyaddr, CALDLY);
	if (!ge_cal_r50_flag) {
		status = -EIO;
		goto cleanup;
	}
	/* *** R50 Cal end *** */

	/* *** Tx offset Cal start *********************** */
	ge_cal_tx_offset_flag = 0;
	ge_cal_tx_offset(phyaddr, CALDLY);
	if (!ge_cal_tx_offset_flag) {
		status = -EIO;
		goto cleanup;
	}
	/* *** Tx offset Cal end *** */

	/* *** Tx Amp Cal start *** */
	ge_cal_flag = 0;
	ge_cal_tx_amp(phyaddr, CALDLY);
	if (!ge_cal_flag) {
		status = -EIO;
		goto cleanup;
	}
	/* *** Tx Amp Cal end *** */

	/* *** Rx offset Cal start *************** */
	/* 1e_96[15]:bypass_tx_offset_cal, Hw bypass, Fw cal */
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x0096, 0x8000);
	if (status != 0)
		goto cleanup;
	/* tx/rx_cal_criteria_value */
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x0037, 0x0033);
	if (status != 0)
		goto cleanup;
	/* [14]: bypass all calibration, [11]: bypass adc offset cal analog */
	status = tc_phy_read_dev_reg_checked(phyaddr, 0x1e, 0x0039, &read_value);
	if (status != 0)
		goto cleanup;
	reg_temp = (u16)(read_value & (~0x4800));
	/* rx offset cal by Hw setup */
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x0039, reg_temp);
	if (status != 0)
		goto cleanup;
	/* [12]: enable rtune calibration */
	status = tc_phy_read_dev_reg_checked(phyaddr, 0x1f, 0x0107, &read_value);
	if (status != 0)
		goto cleanup;
	reg_temp = (u16)(read_value & (~0x1000));
	/* disable rtune calibration */
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1f, 0x0107, reg_temp);
	if (status != 0)
		goto cleanup;
	/* 1e_171[8:7]: bypass tx/rx dc offset cancellation process */
	status = tc_phy_read_dev_reg_checked(phyaddr, 0x1e, 0x0171, &read_value);
	if (status != 0)
		goto cleanup;
	reg_temp = (u16)(read_value & (~0x0180));
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x0171, (reg_temp | 0x0180));
	if (status != 0)
		goto cleanup;
	status = tc_phy_read_dev_reg_checked(phyaddr, 0x1e, 0x0039, &read_value);
	/* rx offset calibration start */
	if (status != 0)
		goto cleanup;
	reg_temp = (u16)read_value;
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x0039, (reg_temp | 0x2000));
	if (status != 0)
		goto cleanup;
	/* rx offset calibration end */
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x0039, (reg_temp & (~0x2000)));
	if (status != 0)
		goto cleanup;
	mdelay(10);	/* mdelay for Hw calibration finish */
	status = tc_phy_read_dev_reg_checked(phyaddr, 0x1e, 0x0171, &read_value);
	if (status != 0)
		goto cleanup;
	reg_temp = (u16)(read_value & (~0x0180));
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x0171, reg_temp);
	if (status != 0)
		goto cleanup;

	status = tc_mii_write_checked(phyaddr, 0x0, reg0_temp);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1f, 0x0100, 0x0000);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x0145, dev1e_145_temp);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1f, 0x273, 0x2000);
	if (status != 0)
		goto cleanup;
	/* *** Rx offset Cal end *** */
	/*eye pic*/
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x0, 0x018d);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x1, 0x01c7);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x2, 0x01c0);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x3, 0x003a);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x4, 0x0206);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x5, 0x0000);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x6, 0x038a);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x7, 0x03c8);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x8, 0x03c0);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x9, 0x0235);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0xa, 0x0008);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0xb, 0x0000);
	if (status != 0)
		goto cleanup;

	/*tmp maybe changed*/
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1f, 0x27c, 0x1111);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1f, 0x27b, 0x47);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1f, 0x273, 0x2200);
	if (status != 0)
		goto cleanup;

	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x3a8, 0x0810);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x3aa, 0x0008);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x3ab, 0x0810);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x3ad, 0x0008);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x3ae, 0x0106);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x3b0, 0x0001);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x3b1, 0x0106);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x3b3, 0x0001);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x18c, 0x0001);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x18d, 0x0001);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x18e, 0x0001);
	if (status != 0)
		goto cleanup;
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x18f, 0x0001);
	if (status != 0)
		goto cleanup;

	/*da_tx_bias1_b_tx_standby = 5'b10 (dev1eh_reg3aah[12:8])*/
	status = tc_phy_read_dev_reg_checked(phyaddr, 0x1e, 0x3aa, &read_value);
	if (status != 0)
		goto cleanup;
	reg_tmp = (u16)read_value;
	reg_tmp = reg_tmp & ~(0x1f00);
	reg_tmp = reg_tmp | 0x2 << 8;
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x3aa, reg_tmp);
	if (status != 0)
		goto cleanup;

	/*da_tx_bias1_a_tx_standby = 5'b10 (dev1eh_reg3a9h[4:0])*/
	status = tc_phy_read_dev_reg_checked(phyaddr, 0x1e, 0x3a9, &read_value);
	if (status != 0)
		goto cleanup;
	reg_tmp = (u16)read_value;
	reg_tmp = reg_tmp & ~(0x1f);
	reg_tmp = reg_tmp | 0x2;
	status = tc_phy_write_dev_reg_checked(phyaddr, 0x1e, 0x3a9, reg_tmp);
	if (status != 0)
		goto cleanup;
cleanup:
	if (reg0_saved) {
		cleanup_status = tc_mii_write_checked(phyaddr, 0U, reg0_temp);
		if (status == 0 && cleanup_status != 0)
			status = cleanup_status;
	}
	cleanup_status = tc_phy_write_dev_reg_checked(
		phyaddr, 0x1fU, 0x0100U, 0U);
	if (status == 0 && cleanup_status != 0)
		status = cleanup_status;
	if (dev145_saved) {
		cleanup_status = tc_phy_write_dev_reg_checked(
			phyaddr, 0x1eU, 0x0145U, dev1e_145_temp);
		if (status == 0 && cleanup_status != 0)
			status = cleanup_status;
	}
	cleanup_status = tc_phy_write_dev_reg_checked(
		phyaddr, 0x1fU, 0x0273U, 0x2000U);
	if (status == 0 && cleanup_status != 0)
		status = cleanup_status;
	if (status != 0)
		pr_err_ratelimited("%s: GE all-analog calibration failed (port=%u status=%d)\\n",
				   __func__, phyaddr, status);
	return status;
}

static int mt7622_ephy_cal(void)
{
	int i, status;
	unsigned long t_s, t_e;

	t_s = jiffies;
	for (i = 0; i < 5; i++) {
		status = do_fe_phy_all_analog_cal_checked((u8)i);
		if (status != 0)
			return status;
	}
	t_e = jiffies;
	if (show_time)
		pr_info("cal time = %lu\n", (t_e - t_s) * 4);
	return 0;
}

static int leopard_ephy_cal(void)
{
	int i, status;
	unsigned long t_s, t_e;

	t_s = jiffies;
	for (i = 1; i < 5; i++) {
		status = do_fe_phy_all_analog_cal_checked((u8)i);
		if (status != 0)
			return status;
	}

	status = do_ge_phy_all_analog_cal(0U);
	if (status != 0)
		return status;

	t_e = jiffies;
	if (show_time)
		pr_info("cal time = %lu\n", (t_e - t_s) * 4);
	return 0;
}

static void wait_loop(void)
{
	int i;
	int read_data;

	for (i = 0; i < 320; i = i + 1)
		read_data = sys_reg_read(RALINK_ETH_SW_BASE + 0x108);
}

static int trgmii_calibration_7623(void)
{
	/* minimum delay for all correct */
	unsigned int tap_a[5] = {
		0, 0, 0, 0, 0
	};
	/* maximum delay for all correct */
	unsigned int tap_b[5] = {
		0, 0, 0, 0, 0
	};
	unsigned int final_tap[5];
	int status;
	unsigned int rxc_step_size;
	unsigned int rxd_step_size;
	unsigned int read_data;
	unsigned int tmp;
	unsigned int rd_wd;
	int i;
	unsigned int err_cnt[5];
	unsigned int init_toggle_data;
	unsigned int err_flag[5];
	unsigned int err_total_flag;
	unsigned int training_word;
	unsigned int rd_tap;

	void __iomem *TRGMII_7623_base;
	void __iomem *TRGMII_7623_RD_0;
	void __iomem *temp_addr;

	TRGMII_7623_base = ETHDMASYS_ETH_SW_BASE + 0x0300;
	TRGMII_7623_RD_0 = TRGMII_7623_base + 0x10;
	rxd_step_size = 0x1;
	rxc_step_size = 0x4;
	init_toggle_data = 0x00000055;
	training_word = 0x000000AC;

	/* RX clock gating in MT7623 */
	reg_bit_zero(TRGMII_7623_base + 0x04, 30, 2);
	/* Assert RX  reset in MT7623 */
	reg_bit_one(TRGMII_7623_base + 0x00, 31, 1);
	/* Set TX OE edge in  MT7623 */
	reg_bit_one(TRGMII_7623_base + 0x78, 13, 1);
	/* Disable RX clock gating in MT7623 */
	reg_bit_one(TRGMII_7623_base + 0x04, 30, 2);
	/* Release RX reset in MT7623 */
	reg_bit_zero(TRGMII_7623_base, 31, 1);

	for (i = 0; i < 5; i++)
		/* Set bslip_en = 1 */
		reg_bit_one(TRGMII_7623_RD_0 + i * 8, 31, 1);

	/* Enable Training Mode in MT7530 */
	status = ra_switch_mdio_read(NULL, 0x1F, 0x7A40, &read_data);
	if (status != 0)
		goto mdio_error;
	read_data |= 0xc0000000;
	status = ra_switch_mdio_write(NULL, 0x1F, 0x7A40, read_data);
	if (status != 0)
		goto mdio_error;

	err_total_flag = 0;
	read_data = 0x0;
	while (err_total_flag == 0 && read_data != 0x68) {
		/* Enable EDGE CHK in MT7623 */
		for (i = 0; i < 5; i++) {
			reg_bit_zero(TRGMII_7623_RD_0 + i * 8, 28, 4);
			reg_bit_one(TRGMII_7623_RD_0 + i * 8, 31, 1);
		}
		wait_loop();
		err_total_flag = 1;
		for (i = 0; i < 5; i++) {
			tmp = sys_reg_read(TRGMII_7623_RD_0 + i * 8);
			err_cnt[i] = (tmp >> 8) & 0x0000000f;

			tmp = sys_reg_read(TRGMII_7623_RD_0 + i * 8);
			rd_wd = (tmp >> 16) & 0x000000ff;

			if (err_cnt[i] != 0)
				err_flag[i] = 1;
			else if (rd_wd != 0x55)
				err_flag[i] = 1;
			else
				err_flag[i] = 0;
			err_total_flag = err_flag[i] & err_total_flag;
		}

		/* Disable EDGE CHK in MT7623 */
		for (i = 0; i < 5; i++) {
			reg_bit_one(TRGMII_7623_RD_0 + i * 8, 30, 1);
			reg_bit_zero(TRGMII_7623_RD_0 + i * 8, 28, 2);
			reg_bit_zero(TRGMII_7623_RD_0 + i * 8, 31, 1);
		}
		wait_loop();
		/* Adjust RXC delay */
		/* RX clock gating in MT7623 */
		reg_bit_zero(TRGMII_7623_base + 0x04, 30, 2);
		read_data = sys_reg_read(TRGMII_7623_base);
		if (err_total_flag == 0) {
			tmp = (read_data & 0x0000007f) + rxc_step_size;
			read_data >>= 8;
			read_data &= 0xffffff80;
			read_data |= tmp;
			read_data <<= 8;
			read_data &= 0xffffff80;
			read_data |= tmp;
			sys_reg_write(TRGMII_7623_base, read_data);
		} else {
			tmp = (read_data & 0x0000007f) + 16;
			read_data >>= 8;
			read_data &= 0xffffff80;
			read_data |= tmp;
			read_data <<= 8;
			read_data &= 0xffffff80;
			read_data |= tmp;
			sys_reg_write(TRGMII_7623_base, read_data);
		}
		read_data &= 0x000000ff;
		/* Disable RX clock gating in MT7623 */
		reg_bit_one(TRGMII_7623_base + 0x04, 30, 2);
		for (i = 0; i < 5; i++)
			reg_bit_one(TRGMII_7623_RD_0 + i * 8, 31, 1);
	}
	/* Read RD_WD MT7623 */
	for (i = 0; i < 5; i++) {
		temp_addr = TRGMII_7623_RD_0 + i * 8;
		rd_tap = 0;
		while (err_flag[i] != 0 && rd_tap != 128) {
			/* Enable EDGE CHK in MT7623 */
			tmp = sys_reg_read(temp_addr);
			tmp |= 0x40000000;
			reg_bit_zero(temp_addr, 28, 4);
			reg_bit_one(temp_addr, 30, 1);
			wait_loop();
			read_data = sys_reg_read(temp_addr);
			/* Read MT7623 Errcnt */
			err_cnt[i] = (read_data >> 8) & 0x0000000f;
			rd_wd = (read_data >> 16) & 0x000000ff;
			if (err_cnt[i] != 0 || rd_wd != 0x55)
				err_flag[i] = 1;
			else
				err_flag[i] = 0;
			/* Disable EDGE CHK in MT7623 */
			reg_bit_zero(temp_addr, 28, 2);
			reg_bit_zero(temp_addr, 31, 1);
			tmp |= 0x40000000;
			sys_reg_write(temp_addr, tmp & 0x4fffffff);
			wait_loop();
			if (err_flag[i] != 0) {
				/* Add RXD delay in MT7623 */
				rd_tap = (read_data & 0x7f) + rxd_step_size;

				read_data = (read_data & 0xffffff80) | rd_tap;
				sys_reg_write(temp_addr, read_data);
				tap_a[i] = rd_tap;
			} else {
				rd_tap = (read_data & 0x0000007f) + 48;
				read_data = (read_data & 0xffffff80) | rd_tap;
				sys_reg_write(temp_addr, read_data);
			}
		}
		pr_info("MT7623 %dth bit  Tap_a = %d\n", i, tap_a[i]);
	}
	for (i = 0; i < 5; i++) {
		while ((err_flag[i] == 0) && (rd_tap != 128)) {
			read_data = sys_reg_read(TRGMII_7623_RD_0 + i * 8);
			/* Add RXD delay in MT7623 */
			rd_tap = (read_data & 0x7f) + rxd_step_size;

			read_data = (read_data & 0xffffff80) | rd_tap;
			sys_reg_write(TRGMII_7623_RD_0 + i * 8, read_data);

			/* Enable EDGE CHK in MT7623 */
			tmp = sys_reg_read(TRGMII_7623_RD_0 + i * 8);
			tmp |= 0x40000000;
			sys_reg_write(TRGMII_7623_RD_0 + i * 8,
				      (tmp & 0x4fffffff));
			wait_loop();
			read_data = sys_reg_read(TRGMII_7623_RD_0 + i * 8);
			/* Read MT7623 Errcnt */
			err_cnt[i] = (read_data >> 8) & 0xf;
			rd_wd = (read_data >> 16) & 0x000000ff;
			if (err_cnt[i] != 0 || rd_wd != 0x55)
				err_flag[i] = 1;
			else
				err_flag[i] = 0;

			/* Disable EDGE CHK in MT7623 */
			tmp = sys_reg_read(TRGMII_7623_RD_0 + i * 8);
			tmp |= 0x40000000;
			sys_reg_write(TRGMII_7623_RD_0 + i * 8,
				      (tmp & 0x4fffffff));
			wait_loop();
		}
		tap_b[i] = rd_tap;	/* -rxd_step_size; */
		pr_info("MT7623 %dth bit  Tap_b = %d\n", i, tap_b[i]);
		/* Calculate RXD delay = (TAP_A + TAP_B)/2 */
		final_tap[i] = (tap_a[i] + tap_b[i]) / 2;
		read_data = (read_data & 0xffffff80) | final_tap[i];
		sys_reg_write(TRGMII_7623_RD_0 + i * 8, read_data);
	}

	status = ra_switch_mdio_read(NULL, 0x1F, 0x7A40, &read_data);
	if (status != 0)
		goto mdio_error;
	read_data &= 0x3fffffff;
	status = ra_switch_mdio_write(NULL, 0x1F, 0x7A40, read_data);
	if (status != 0)
		goto mdio_error;
	return 0;

mdio_error:
	pr_err_ratelimited("%s: trgmii_calibration_7623 MDIO operation failed: %d\n",
			   __func__, status);
	return status;
}

static int trgmii_calibration_7530(void)
{
	struct END_DEVICE *ei_local = netdev_priv(dev_raether);
	int status;
	unsigned int tap_a[5] = {
		0, 0, 0, 0, 0
	};
	unsigned int tap_b[5] = {
		0, 0, 0, 0, 0
	};
	unsigned int final_tap[5];
	unsigned int rxc_step_size;
	unsigned int rxd_step_size;
	unsigned int read_data;
	unsigned int tmp = 0;
	int i;
	unsigned int err_cnt[5];
	unsigned int rd_wd;
	unsigned int init_toggle_data;
	unsigned int err_flag[5];
	unsigned int err_total_flag;
	unsigned int training_word;
	unsigned int rd_tap;

	void __iomem *TRGMII_7623_base;
	u32 TRGMII_7530_RD_0;
	u32 TRGMII_7530_base;
	u32 TRGMII_7530_TX_base;

	TRGMII_7623_base = ETHDMASYS_ETH_SW_BASE + 0x0300;
	TRGMII_7530_base = 0x7A00;
	TRGMII_7530_RD_0 = TRGMII_7530_base + 0x10;
	rxd_step_size = 0x1;
	rxc_step_size = 0x8;
	init_toggle_data = 0x00000055;
	training_word = 0x000000AC;

	TRGMII_7530_TX_base = TRGMII_7530_base + 0x50;

	reg_bit_one(TRGMII_7623_base + 0x40, 31, 1);
	status = ra_switch_mdio_read(NULL, 0x1F, 0x7a10, &read_data);
	if (status != 0)
		goto mdio_error;

	/* RX clock gating in MT7530 */
	status = ra_switch_mdio_read(NULL, 0x1F, TRGMII_7530_base + 0x04, &read_data);
	if (status != 0)
		goto mdio_error;
	read_data &= 0x3fffffff;
	status = ra_switch_mdio_write(NULL, 0x1F, TRGMII_7530_base + 0x04, read_data);
	if (status != 0)
		goto mdio_error;

	/* Set TX OE edge in  MT7530 */
	status = ra_switch_mdio_read(NULL, 0x1F, TRGMII_7530_base + 0x78, &read_data);
	if (status != 0)
		goto mdio_error;
	read_data |= 0x00002000;
	status = ra_switch_mdio_write(NULL, 0x1F, TRGMII_7530_base + 0x78, read_data);
	if (status != 0)
		goto mdio_error;

	/* Assert RX  reset in MT7530 */
	status = ra_switch_mdio_read(NULL, 0x1F, TRGMII_7530_base, &read_data);
	if (status != 0)
		goto mdio_error;
	read_data |= 0x80000000;
	status = ra_switch_mdio_write(NULL, 0x1F, TRGMII_7530_base, read_data);
	if (status != 0)
		goto mdio_error;

	/* Release RX reset in MT7530 */
	status = ra_switch_mdio_read(NULL, 0x1F, TRGMII_7530_base, &read_data);
	if (status != 0)
		goto mdio_error;
	read_data &= 0x7fffffff;
	status = ra_switch_mdio_write(NULL, 0x1F, TRGMII_7530_base, read_data);
	if (status != 0)
		goto mdio_error;

	/* Disable RX clock gating in MT7530 */
	status = ra_switch_mdio_read(NULL, 0x1F, TRGMII_7530_base + 0x04, &read_data);
	if (status != 0)
		goto mdio_error;
	read_data |= 0xC0000000;
	status = ra_switch_mdio_write(NULL, 0x1F, TRGMII_7530_base + 0x04, read_data);
	if (status != 0)
		goto mdio_error;

	/*Enable Training Mode in MT7623 */
	reg_bit_zero(TRGMII_7623_base + 0x40, 30, 1);
	if (ei_local->architecture & GE1_TRGMII_FORCE_2000)
		reg_bit_one(TRGMII_7623_base + 0x40, 30, 2);
	else
		reg_bit_one(TRGMII_7623_base + 0x40, 31, 1);
	reg_bit_zero(TRGMII_7623_base + 0x78, 8, 4);
	reg_bit_zero(TRGMII_7623_base + 0x50, 8, 4);
	reg_bit_zero(TRGMII_7623_base + 0x58, 8, 4);
	reg_bit_zero(TRGMII_7623_base + 0x60, 8, 4);
	reg_bit_zero(TRGMII_7623_base + 0x68, 8, 4);
	reg_bit_zero(TRGMII_7623_base + 0x70, 8, 4);
	reg_bit_one(TRGMII_7623_base + 0x78, 11, 1);

	err_total_flag = 0;
	read_data = 0x0;
	while (err_total_flag == 0 && (read_data != 0x68)) {
		/* Enable EDGE CHK in MT7530 */
		for (i = 0; i < 5; i++) {
			status = ra_switch_mdio_read(NULL, 0x1F, TRGMII_7530_RD_0 + i * 8, &read_data);
			if (status != 0)
				goto mdio_error;
			read_data |= 0x40000000;
			read_data &= 0x4fffffff;
			status = ra_switch_mdio_write(NULL, 0x1F, TRGMII_7530_RD_0 + i * 8, read_data);
			if (status != 0)
				goto mdio_error;
			wait_loop();
			status = ra_switch_mdio_read(NULL, 0x1F, TRGMII_7530_RD_0 + i * 8, &err_cnt[i]);
			if (status != 0)
				goto mdio_error;
			err_cnt[i] >>= 8;
			err_cnt[i] &= 0x0000ff0f;
			rd_wd = err_cnt[i] >> 8;
			rd_wd &= 0x000000ff;
			err_cnt[i] &= 0x0000000f;
			if (err_cnt[i] != 0)
				err_flag[i] = 1;
			else if (rd_wd != 0x55)
				err_flag[i] = 1;
			else
				err_flag[i] = 0;

			if (i == 0)
				err_total_flag = err_flag[i];
			else
				err_total_flag = err_flag[i] & err_total_flag;
			/* Disable EDGE CHK in MT7530 */
			status = ra_switch_mdio_read(NULL, 0x1F, TRGMII_7530_RD_0 + i * 8, &read_data);
			if (status != 0)
				goto mdio_error;
			read_data |= 0x40000000;
			read_data &= 0x4fffffff;
			status = ra_switch_mdio_write(NULL, 0x1F, TRGMII_7530_RD_0 + i * 8, read_data);
			if (status != 0)
				goto mdio_error;
			wait_loop();
		}
		/*Adjust RXC delay */
		if (err_total_flag == 0) {
			/* Assert RX  reset in MT7530 */
			status = ra_switch_mdio_read(NULL, 0x1F, TRGMII_7530_base, &read_data);
			if (status != 0)
				goto mdio_error;
			read_data |= 0x80000000;
			status = ra_switch_mdio_write(NULL, 0x1F, TRGMII_7530_base, read_data);
			if (status != 0)
				goto mdio_error;

			/* RX clock gating in MT7530 */
			status = ra_switch_mdio_read(NULL, 0x1F, TRGMII_7530_base + 0x04, &read_data);
			if (status != 0)
				goto mdio_error;
			read_data &= 0x3fffffff;
			status = ra_switch_mdio_write(NULL, 0x1F, TRGMII_7530_base + 0x04, read_data);
			if (status != 0)
				goto mdio_error;

			status = ra_switch_mdio_read(NULL, 0x1F, TRGMII_7530_base, &read_data);
			if (status != 0)
				goto mdio_error;
			tmp = read_data;
			tmp &= 0x0000007f;
			tmp += rxc_step_size;
			read_data &= 0xffffff80;
			read_data |= tmp;
			status = ra_switch_mdio_write(NULL, 0x1F, TRGMII_7530_base, read_data);
			if (status != 0)
				goto mdio_error;
			status = ra_switch_mdio_read(NULL, 0x1F, TRGMII_7530_base, &read_data);
			if (status != 0)
				goto mdio_error;

			/* Release RX reset in MT7530 */
			status = ra_switch_mdio_read(NULL, 0x1F, TRGMII_7530_base, &read_data);
			if (status != 0)
				goto mdio_error;
			read_data &= 0x7fffffff;
			status = ra_switch_mdio_write(NULL, 0x1F, TRGMII_7530_base, read_data);
			if (status != 0)
				goto mdio_error;

			/* Disable RX clock gating in MT7530 */
			status = ra_switch_mdio_read(NULL, 0x1F, TRGMII_7530_base + 0x04, &read_data);
			if (status != 0)
				goto mdio_error;
			read_data |= 0xc0000000;
			status = ra_switch_mdio_write(NULL, 0x1F, TRGMII_7530_base + 0x04, read_data);
			if (status != 0)
				goto mdio_error;
		}
		read_data = tmp;
	}
	/* Read RD_WD MT7530 */
	for (i = 0; i < 5; i++) {
		rd_tap = 0;
		while (err_flag[i] != 0 && rd_tap != 128) {
			/* Enable EDGE CHK in MT7530 */
			status = ra_switch_mdio_read(NULL, 0x1F, TRGMII_7530_RD_0 + i * 8, &read_data);
			if (status != 0)
				goto mdio_error;
			read_data |= 0x40000000;
			read_data &= 0x4fffffff;
			status = ra_switch_mdio_write(NULL, 0x1F, TRGMII_7530_RD_0 + i * 8, read_data);
			if (status != 0)
				goto mdio_error;
			wait_loop();
			err_cnt[i] = (read_data >> 8) & 0x0000000f;
			rd_wd = (read_data >> 16) & 0x000000ff;
			if (err_cnt[i] != 0 || rd_wd != 0x55)
				err_flag[i] = 1;
			else
				err_flag[i] = 0;

			if (err_flag[i] != 0) {
				/* Add RXD delay in MT7530 */
				rd_tap = (read_data & 0x7f) + rxd_step_size;
				read_data = (read_data & 0xffffff80) | rd_tap;
				status = ra_switch_mdio_write(NULL, 0x1F, TRGMII_7530_RD_0 + i * 8, read_data);
				if (status != 0)
					goto mdio_error;
				tap_a[i] = rd_tap;
			} else {
				/* Record the min delay TAP_A */
				tap_a[i] = (read_data & 0x0000007f);
				rd_tap = tap_a[i] + 0x4;
				read_data = (read_data & 0xffffff80) | rd_tap;
				status = ra_switch_mdio_write(NULL, 0x1F, TRGMII_7530_RD_0 + i * 8, read_data);
				if (status != 0)
					goto mdio_error;
			}

			/* Disable EDGE CHK in MT7530 */
			status = ra_switch_mdio_read(NULL, 0x1F, TRGMII_7530_RD_0 + i * 8, &read_data);
			if (status != 0)
				goto mdio_error;
			read_data |= 0x40000000;
			read_data &= 0x4fffffff;
			status = ra_switch_mdio_write(NULL, 0x1F, TRGMII_7530_RD_0 + i * 8, read_data);
			if (status != 0)
				goto mdio_error;
			wait_loop();
		}
		pr_info("MT7530 %dth bit  Tap_a = %d\n", i, tap_a[i]);
	}
	for (i = 0; i < 5; i++) {
		rd_tap = 0;
		while (err_flag[i] == 0 && (rd_tap != 128)) {
			/* Enable EDGE CHK in MT7530 */
			status = ra_switch_mdio_read(NULL, 0x1F, TRGMII_7530_RD_0 + i * 8, &read_data);
			if (status != 0)
				goto mdio_error;
			read_data |= 0x40000000;
			read_data &= 0x4fffffff;
			status = ra_switch_mdio_write(NULL, 0x1F, TRGMII_7530_RD_0 + i * 8, read_data);
			if (status != 0)
				goto mdio_error;
			wait_loop();
			err_cnt[i] = (read_data >> 8) & 0x0000000f;
			rd_wd = (read_data >> 16) & 0x000000ff;
			if (err_cnt[i] != 0 || rd_wd != 0x55)
				err_flag[i] = 1;
			else
				err_flag[i] = 0;

			if (err_flag[i] == 0 && (rd_tap != 128)) {
				/* Add RXD delay in MT7530 */
				rd_tap = (read_data & 0x7f) + rxd_step_size;
				read_data = (read_data & 0xffffff80) | rd_tap;
				status = ra_switch_mdio_write(NULL, 0x1F, TRGMII_7530_RD_0 + i * 8, read_data);
				if (status != 0)
					goto mdio_error;
			}
			/* Disable EDGE CHK in MT7530 */
			status = ra_switch_mdio_read(NULL, 0x1F, TRGMII_7530_RD_0 + i * 8, &read_data);
			if (status != 0)
				goto mdio_error;
			read_data |= 0x40000000;
			read_data &= 0x4fffffff;
			status = ra_switch_mdio_write(NULL, 0x1F, TRGMII_7530_RD_0 + i * 8, read_data);
			if (status != 0)
				goto mdio_error;
			wait_loop();
		}
		tap_b[i] = rd_tap;	/* - rxd_step_size; */
		pr_info("MT7530 %dth bit  Tap_b = %d\n", i, tap_b[i]);
		/* Calculate RXD delay = (TAP_A + TAP_B)/2 */
		final_tap[i] = (tap_a[i] + tap_b[i]) / 2;
		read_data = (read_data & 0xffffff80) | final_tap[i];
		status = ra_switch_mdio_write(NULL, 0x1F, TRGMII_7530_RD_0 + i * 8, read_data);
		if (status != 0)
			goto mdio_error;
	}
	if (ei_local->architecture & GE1_TRGMII_FORCE_2000)
		reg_bit_zero(TRGMII_7623_base + 0x40, 31, 1);
	else
		reg_bit_zero(TRGMII_7623_base + 0x40, 30, 2);
	return 0;

mdio_error:
	pr_err_ratelimited("%s: trgmii_calibration_7530 MDIO operation failed: %d\n",
			   __func__, status);
	return status;
}

static int mt7530_trgmii_clock_setting(u32 xtal_mode)
{
	u32 reg_value;
	int status;
	/* TRGMII Clock */
	status = ra_switch_mdio_write_cl45(0, 0x1f, 0x410, 0x1);
	if (status != 0)
		goto mdio_error;
	if (xtal_mode == 1) {	/* 25MHz */
		status = ra_switch_mdio_write_cl45(0, 0x1f, 0x404, MT7530_TRGMII_PLL_25M);
		if (status != 0)
			goto mdio_error;
	} else if (xtal_mode == 2) {	/* 40MHz */
		status = ra_switch_mdio_write_cl45(0, 0x1f, 0x404, MT7530_TRGMII_PLL_40M);
		if (status != 0)
			goto mdio_error;
	}
	status = ra_switch_mdio_write_cl45(0, 0x1f, 0x405, 0);
	if (status != 0)
		goto mdio_error;
	if (xtal_mode == 1) { /* 25MHz */
		status = ra_switch_mdio_write_cl45(0, 0x1f, 0x409, 0x57);
		if (status != 0)
			goto mdio_error;
	} else {
		status = ra_switch_mdio_write_cl45(0, 0x1f, 0x409, 0x87);
		if (status != 0)
			goto mdio_error;
	}

	if (xtal_mode == 1) { /* 25MHz */
		status = ra_switch_mdio_write_cl45(0, 0x1f, 0x40a, 0x57);
		if (status != 0)
			goto mdio_error;
	} else {
		status = ra_switch_mdio_write_cl45(0, 0x1f, 0x40a, 0x87);
		if (status != 0)
			goto mdio_error;
	}

	status = ra_switch_mdio_write_cl45(0, 0x1f, 0x403, 0x1800);
	if (status != 0)
		goto mdio_error;
	status = ra_switch_mdio_write_cl45(0, 0x1f, 0x403, 0x1c00);
	if (status != 0)
		goto mdio_error;
	status = ra_switch_mdio_write_cl45(0, 0x1f, 0x401, 0xc020);
	if (status != 0)
		goto mdio_error;
	status = ra_switch_mdio_write_cl45(0, 0x1f, 0x406, 0xa030);
	if (status != 0)
		goto mdio_error;
	status = ra_switch_mdio_write_cl45(0, 0x1f, 0x406, 0xa038);
	if (status != 0)
		goto mdio_error;
	usleep_range(120, 130);	/* for MT7623 bring up test */
	status = ra_switch_mdio_write_cl45(0, 0x1f, 0x410, 0x3);
	if (status != 0)
		goto mdio_error;

	status = ra_switch_mdio_read(NULL, 31, 0x7830, &reg_value);
	if (status != 0)
		goto mdio_error;
	reg_value &= 0xFFFFFFFC;
	reg_value |= 0x00000001;
	status = ra_switch_mdio_write(NULL, 31, 0x7830, reg_value);
	if (status != 0)
		goto mdio_error;

	status = ra_switch_mdio_read(NULL, 31, 0x7a40, &reg_value);
	if (status != 0)
		goto mdio_error;
	reg_value &= ~(0x1 << 30);
	reg_value &= ~(0x1 << 28);
	status = ra_switch_mdio_write(NULL, 31, 0x7a40, reg_value);
	if (status != 0)
		goto mdio_error;

	status = ra_switch_mdio_write(NULL, 31, 0x7a78, 0x55);
	if (status != 0)
		goto mdio_error;
	usleep_range(100, 110);	/* for mt7623 bring up test */

	/* Release MT7623 RXC reset */
	reg_bit_zero(ETHDMASYS_ETH_SW_BASE + 0x0300, 31, 1);

	status = trgmii_calibration_7623();
	if (status != 0)
		goto mdio_error;
	status = trgmii_calibration_7530();
	if (status != 0)
		goto mdio_error;
	/* Assert RX  reset in MT7623 */
	reg_bit_one(ETHDMASYS_ETH_SW_BASE + 0x0300, 31, 1);
	/* Release RX reset in MT7623 */
	reg_bit_zero(ETHDMASYS_ETH_SW_BASE + 0x0300, 31, 1);
	status = ra_switch_mdio_read(NULL, 31, 0x7a00, &reg_value);
	if (status != 0)
		goto mdio_error;
	reg_value |= (0x1 << 31);
	status = ra_switch_mdio_write(NULL, 31, 0x7a00, reg_value);
	if (status != 0)
		goto mdio_error;
	mdelay(1);
	reg_value &= ~(0x1 << 31);
	status = ra_switch_mdio_write(NULL, 31, 0x7a00, reg_value);
	if (status != 0)
		goto mdio_error;
	mdelay(100);
	return 0;

mdio_error:
	pr_err_ratelimited("%s: mt7530_trgmii_clock_setting MDIO operation failed: %d\n",
			   __func__, status);
	return status;
}

void trgmii_set_7621(void)
{
	u32 val = 0;
	u32 val_0 = 0;

	val = sys_reg_read(RSTCTRL);
	/* MT7621 need to reset GMAC and FE first */
	val = val | RALINK_FE_RST | RALINK_ETH_RST;
	sys_reg_write(RSTCTRL, val);

	/* set TRGMII clock */
	val_0 = sys_reg_read(CLK_CFG_0);
	val_0 &= 0xffffff9f;
	val_0 |= (0x1 << 5);
	sys_reg_write(CLK_CFG_0, val_0);
	mdelay(1);
	val_0 = sys_reg_read(CLK_CFG_0);
	pr_info("set CLK_CFG_0 = 0x%x!!!!!!!!!!!!!!!!!!1\n", val_0);
	val = val & ~(RALINK_FE_RST | RALINK_ETH_RST);
	sys_reg_write(RSTCTRL, val);
	pr_info("trgmii_set_7621 Completed!!\n");
}

int trgmii_set_7530(void)
{
	u32 regValue;
	int status;

	status = ra_switch_mdio_write(NULL, 0, 13, 0x1f);
	if (status != 0)
		goto mdio_error;
	status = ra_switch_mdio_write(NULL, 0, 14, 0x404);
	if (status != 0)
		goto mdio_error;
	status = ra_switch_mdio_write(NULL, 0, 13, 0x401f);
	if (status != 0)
		goto mdio_error;
	status = ra_switch_mdio_read(NULL, 31, 0x7800, &regValue);
	if (status != 0)
		goto mdio_error;
	regValue = (regValue >> 9) & 0x3;
	if (regValue == 0x3) {
		status = ra_switch_mdio_write(NULL, 0, 14, 0x0C00);/*25Mhz XTAL for 150Mhz CLK */
		if (status != 0)
			goto mdio_error;
	} else if (regValue == 0x2) {
		status = ra_switch_mdio_write(NULL, 0, 14, 0x0780);/*40Mhz XTAL for 150Mhz CLK */
		if (status != 0)
			goto mdio_error;
	}

	mdelay(1);

	status = ra_switch_mdio_write(NULL, 0, 13, 0x1f);
	if (status != 0)
		goto mdio_error;
	status = ra_switch_mdio_write(NULL, 0, 14, 0x409);
	if (status != 0)
		goto mdio_error;
	status = ra_switch_mdio_write(NULL, 0, 13, 0x401f);
	if (status != 0)
		goto mdio_error;
	if (regValue == 0x3) { /* 25MHz */
		status = ra_switch_mdio_write(NULL, 0, 14, 0x57);
		if (status != 0)
			goto mdio_error;
	} else {
		status = ra_switch_mdio_write(NULL, 0, 14, 0x87);
		if (status != 0)
			goto mdio_error;
	}
	mdelay(1);

	status = ra_switch_mdio_write(NULL, 0, 13, 0x1f);
	if (status != 0)
		goto mdio_error;
	status = ra_switch_mdio_write(NULL, 0, 14, 0x40a);
	if (status != 0)
		goto mdio_error;
	status = ra_switch_mdio_write(NULL, 0, 13, 0x401f);
	if (status != 0)
		goto mdio_error;
	if (regValue == 0x3) { /* 25MHz */
		status = ra_switch_mdio_write(NULL, 0, 14, 0x57);
		if (status != 0)
			goto mdio_error;
	} else {
		status = ra_switch_mdio_write(NULL, 0, 14, 0x87);
		if (status != 0)
			goto mdio_error;
	}

/* PLL BIAS en */
	status = ra_switch_mdio_write(NULL, 0, 13, 0x1f);
	if (status != 0)
		goto mdio_error;
	status = ra_switch_mdio_write(NULL, 0, 14, 0x403);
	if (status != 0)
		goto mdio_error;
	status = ra_switch_mdio_write(NULL, 0, 13, 0x401f);
	if (status != 0)
		goto mdio_error;
	status = ra_switch_mdio_write(NULL, 0, 14, 0x1800);
	if (status != 0)
		goto mdio_error;
	mdelay(1);

/* BIAS LPF en */
	status = ra_switch_mdio_write(NULL, 0, 13, 0x1f);
	if (status != 0)
		goto mdio_error;
	status = ra_switch_mdio_write(NULL, 0, 14, 0x403);
	if (status != 0)
		goto mdio_error;
	status = ra_switch_mdio_write(NULL, 0, 13, 0x401f);
	if (status != 0)
		goto mdio_error;
	status = ra_switch_mdio_write(NULL, 0, 14, 0x1c00);
	if (status != 0)
		goto mdio_error;

/* sys PLL en */
	status = ra_switch_mdio_write(NULL, 0, 13, 0x1f);
	if (status != 0)
		goto mdio_error;
	status = ra_switch_mdio_write(NULL, 0, 14, 0x401);
	if (status != 0)
		goto mdio_error;
	status = ra_switch_mdio_write(NULL, 0, 13, 0x401f);
	if (status != 0)
		goto mdio_error;
	status = ra_switch_mdio_write(NULL, 0, 14, 0xc020);
	if (status != 0)
		goto mdio_error;

/* LCDDDS PWDS */
	status = ra_switch_mdio_write(NULL, 0, 13, 0x1f);
	if (status != 0)
		goto mdio_error;
	status = ra_switch_mdio_write(NULL, 0, 14, 0x406);
	if (status != 0)
		goto mdio_error;
	status = ra_switch_mdio_write(NULL, 0, 13, 0x401f);
	if (status != 0)
		goto mdio_error;
	status = ra_switch_mdio_write(NULL, 0, 14, 0xa030);
	if (status != 0)
		goto mdio_error;
	mdelay(1);

/* GSW_2X_CLK */
	status = ra_switch_mdio_write(NULL, 0, 13, 0x1f);
	if (status != 0)
		goto mdio_error;
	status = ra_switch_mdio_write(NULL, 0, 14, 0x410);
	if (status != 0)
		goto mdio_error;
	status = ra_switch_mdio_write(NULL, 0, 13, 0x401f);
	if (status != 0)
		goto mdio_error;
	status = ra_switch_mdio_write(NULL, 0, 14, 0x0003);
	if (status != 0)
		goto mdio_error;
	status = ra_switch_mdio_write_cl45(0, 0x1f, 0x410, 0x0003);
	if (status != 0)
		goto mdio_error;

/* enable P6 */
	status = ra_switch_mdio_write(NULL, 31, 0x3600, 0x5e33b);
	if (status != 0)
		goto mdio_error;

/* enable TRGMII */
	status = ra_switch_mdio_write(NULL, 31, 0x7830, 0x1);
	if (status != 0)
		goto mdio_error;

	pr_info("trgmii_set_7530 Completed!!\n");
	return 0;

mdio_error:
	pr_err_ratelimited("%s: trgmii_set_7530 MDIO operation failed: %d\n",
			   __func__, status);
	return status;
}

#if !defined (CONFIG_RAETH_ESW_CONTROL)
static int is_switch_vlan_table_busy(void)
{
	int j = 0;
	unsigned int value = 0;
	int status;

	for (j = 0; j < 20; j++) {
		status = ra_switch_mdio_read(NULL, 31, 0x90, &value);
		if (status != 0)
			return status;
		if ((value & 0x80000000) == 0) {	/* table busy */
			break;
		}
		mdelay(70);
	}
	if (j == 20) {
		pr_err_ratelimited("%s: VLAN table remained busy (VTCR=0x%x)\n",
				   __func__, value);
		return -ETIMEDOUT;
	}
	return 0;
}

static int lan_wan_partition(void)
{
	struct END_DEVICE *ei_local = netdev_priv(dev_raether);
	int status;
	/*Set  MT7530 */
	if (ei_local->architecture & WAN_AT_P0) {
		pr_info("set LAN/WAN WLLLL\n");
		/*WLLLL, wan at P0 */
		/*LAN/WAN ports as security mode */
		status = ra_switch_mdio_write(NULL, 31, 0x2004, 0xff0003);	/* port0 */
		if (status != 0)
			goto mdio_error;
		status = ra_switch_mdio_write(NULL, 31, 0x2104, 0xff0003);	/* port1 */
		if (status != 0)
			goto mdio_error;
		status = ra_switch_mdio_write(NULL, 31, 0x2204, 0xff0003);	/* port2 */
		if (status != 0)
			goto mdio_error;
		status = ra_switch_mdio_write(NULL, 31, 0x2304, 0xff0003);	/* port3 */
		if (status != 0)
			goto mdio_error;
		status = ra_switch_mdio_write(NULL, 31, 0x2404, 0xff0003);	/* port4 */
		if (status != 0)
			goto mdio_error;
		status = ra_switch_mdio_write(NULL, 31, 0x2504, 0xff0003);	/* port5 */
		if (status != 0)
			goto mdio_error;
		status = ra_switch_mdio_write(NULL, 31, 0x2604, 0xff0003);	/* port6 */
		if (status != 0)
			goto mdio_error;

		/*set PVID */
		status = ra_switch_mdio_write(NULL, 31, 0x2014, 0x10002);	/* port0 */
		if (status != 0)
			goto mdio_error;
		status = ra_switch_mdio_write(NULL, 31, 0x2114, 0x10001);	/* port1 */
		if (status != 0)
			goto mdio_error;
		status = ra_switch_mdio_write(NULL, 31, 0x2214, 0x10001);	/* port2 */
		if (status != 0)
			goto mdio_error;
		status = ra_switch_mdio_write(NULL, 31, 0x2314, 0x10001);	/* port3 */
		if (status != 0)
			goto mdio_error;
		status = ra_switch_mdio_write(NULL, 31, 0x2414, 0x10001);	/* port4 */
		if (status != 0)
			goto mdio_error;
		status = ra_switch_mdio_write(NULL, 31, 0x2514, 0x10002);	/* port5 */
		if (status != 0)
			goto mdio_error;
		status = ra_switch_mdio_write(NULL, 31, 0x2614, 0x10001);	/* port6 */
		if (status != 0)
			goto mdio_error;
		/*port6 */
		/*VLAN member */
		status = is_switch_vlan_table_busy();
		if (status != 0)
			goto mdio_error;
		status = ra_switch_mdio_write(NULL, 31, 0x94, 0x405e0001);	/* VAWD1 */
		if (status != 0)
			goto mdio_error;
		status = ra_switch_mdio_write(NULL, 31, 0x90, 0x80001001);	/* VTCR, VID=1 */
		if (status != 0)
			goto mdio_error;
		status = is_switch_vlan_table_busy();
		if (status != 0)
			goto mdio_error;

		status = ra_switch_mdio_write(NULL, 31, 0x94, 0x40210001);	/* VAWD1 */
		if (status != 0)
			goto mdio_error;
		status = ra_switch_mdio_write(NULL, 31, 0x90, 0x80001002);	/* VTCR, VID=2 */
		if (status != 0)
			goto mdio_error;
		status = is_switch_vlan_table_busy();
		if (status != 0)
			goto mdio_error;
	}
	if (ei_local->architecture & WAN_AT_P4) {
		pr_info("set LAN/WAN LLLLW\n");
		/*LLLLW, wan at P4 */
		/*LAN/WAN ports as security mode */
		status = ra_switch_mdio_write(NULL, 31, 0x2004, 0xff0003);	/* port0 */
		if (status != 0)
			goto mdio_error;
		status = ra_switch_mdio_write(NULL, 31, 0x2104, 0xff0003);	/* port1 */
		if (status != 0)
			goto mdio_error;
		status = ra_switch_mdio_write(NULL, 31, 0x2204, 0xff0003);	/* port2 */
		if (status != 0)
			goto mdio_error;
		status = ra_switch_mdio_write(NULL, 31, 0x2304, 0xff0003);	/* port3 */
		if (status != 0)
			goto mdio_error;
		status = ra_switch_mdio_write(NULL, 31, 0x2404, 0xff0003);	/* port4 */
		if (status != 0)
			goto mdio_error;
		status = ra_switch_mdio_write(NULL, 31, 0x2504, 0xff0003);	/* port5 */
		if (status != 0)
			goto mdio_error;
		status = ra_switch_mdio_write(NULL, 31, 0x2604, 0xff0003);	/* port6 */
		if (status != 0)
			goto mdio_error;

		/*set PVID */
		status = ra_switch_mdio_write(NULL, 31, 0x2014, 0x10001);	/* port0 */
		if (status != 0)
			goto mdio_error;
		status = ra_switch_mdio_write(NULL, 31, 0x2114, 0x10001);	/* port1 */
		if (status != 0)
			goto mdio_error;
		status = ra_switch_mdio_write(NULL, 31, 0x2214, 0x10001);	/* port2 */
		if (status != 0)
			goto mdio_error;
		status = ra_switch_mdio_write(NULL, 31, 0x2314, 0x10001);	/* port3 */
		if (status != 0)
			goto mdio_error;
		status = ra_switch_mdio_write(NULL, 31, 0x2414, 0x10002);	/* port4 */
		if (status != 0)
			goto mdio_error;
		status = ra_switch_mdio_write(NULL, 31, 0x2514, 0x10002);	/* port5 */
		if (status != 0)
			goto mdio_error;
		status = ra_switch_mdio_write(NULL, 31, 0x2614, 0x10001);	/* port6 */
		if (status != 0)
			goto mdio_error;

		/*VLAN member */
		status = is_switch_vlan_table_busy();
		if (status != 0)
			goto mdio_error;
		status = ra_switch_mdio_write(NULL, 31, 0x94, 0x404f0001);	/* VAWD1 */
		if (status != 0)
			goto mdio_error;
		status = ra_switch_mdio_write(NULL, 31, 0x90, 0x80001001);	/* VTCR, VID=1 */
		if (status != 0)
			goto mdio_error;
		status = is_switch_vlan_table_busy();
		if (status != 0)
			goto mdio_error;
		status = ra_switch_mdio_write(NULL, 31, 0x94, 0x40300001);	/* VAWD1 */
		if (status != 0)
			goto mdio_error;
		status = ra_switch_mdio_write(NULL, 31, 0x90, 0x80001002);	/* VTCR, VID=2 */
		if (status != 0)
			goto mdio_error;
		status = is_switch_vlan_table_busy();
		if (status != 0)
			goto mdio_error;
	}
	return 0;

mdio_error:
	pr_err_ratelimited("%s: LAN/WAN VLAN partition failed: %d\n",
			   __func__, status);
	return status;
}
#endif

static int mt7530_phy_setting(void)
{
	u32 i;
	u32 reg_value;
	int status;

	for (i = 0; i < 5; i++) {
		/* Disable EEE */
		status = ra_switch_mdio_write_cl45(i, 0x7, 0x3c, 0);
		if (status != 0)
			goto mdio_error;
		/* Enable HW auto downshift */
		status = ra_switch_mdio_write(NULL, i, 31, 0x1);
		if (status != 0)
			goto mdio_error;
		status = ra_switch_mdio_read(NULL, i, 0x14, &reg_value);
		if (status != 0)
			goto mdio_error;
		reg_value |= (1 << 4);
		status = ra_switch_mdio_write(NULL, i, 0x14, reg_value);
		if (status != 0)
			goto mdio_error;
		/* Increase SlvDPSready time */
		status = ra_switch_mdio_write(NULL, i, 31, 0x52b5);
		if (status != 0)
			goto mdio_error;
		status = ra_switch_mdio_write(NULL, i, 16, 0xafae);
		if (status != 0)
			goto mdio_error;
		status = ra_switch_mdio_write(NULL, i, 18, 0x2f);
		if (status != 0)
			goto mdio_error;
		status = ra_switch_mdio_write(NULL, i, 16, 0x8fae);
		if (status != 0)
			goto mdio_error;
		/* Incease post_update_timer */
		status = ra_switch_mdio_write(NULL, i, 31, 0x3);
		if (status != 0)
			goto mdio_error;
		status = ra_switch_mdio_write(NULL, i, 17, 0x4b);
		if (status != 0)
			goto mdio_error;
		/* Adjust 100_mse_threshold */
		status = ra_switch_mdio_write_cl45(i, 0x1e, 0x123, 0xffff);
		if (status != 0)
			goto mdio_error;
		/* Disable mcc */
		status = ra_switch_mdio_write_cl45(i, 0x1e, 0xa6, 0x300);
		if (status != 0)
			goto mdio_error;
	}
	return 0;

mdio_error:
	pr_err_ratelimited("%s: MT7530 PHY initialization failed: %d\n",
			   __func__, status);
	return status;
}

static void ra_switch_mdio_reset_delay(void *context)
{
	(void)context;
	mdelay(10);
}

static int setup_internal_gsw(void)
{
	void __iomem *gpio_base_virt;
	struct END_DEVICE *ei_local;
	u32 reg_value;
	u32 xtal_mode;
	u32 i;
	int status;

	gpio_base_virt = ioremap(ETH_GPIO_BASE, 0x1000);
	if (!gpio_base_virt) {
		pr_err("%s: failed to map GPIO registers\n", __func__);
		return -ENOMEM;
	}
	ei_local = netdev_priv(dev_raether);

	if (ei_local->architecture &
	    (GE1_TRGMII_FORCE_2000 | GE1_TRGMII_FORCE_2600))
		reg_bit_one(RALINK_SYSCTL_BASE + 0x2c, 11, 1);
	else
		reg_bit_zero(RALINK_SYSCTL_BASE + 0x2c, 11, 1);
	reg_bit_one(ETHDMASYS_ETH_SW_BASE + 0x0390, 1, 1);	/* TRGMII mode */

	#if defined(CONFIG_GE1_RGMII_FORCE_1200)

	if (ei_local->chip_name == MT7621_FE)
		trgmii_set_7621();

	#endif

	/*Hardware reset Switch */

	reg_bit_zero((void __iomem *)gpio_base_virt + 0x520, 1, 1);
	mdelay(1);
	reg_bit_one((void __iomem *)gpio_base_virt + 0x520, 1, 1);
	mdelay(100);

	/* Assert MT7623 RXC reset */
	reg_bit_one(ETHDMASYS_ETH_SW_BASE + 0x0300, 31, 1);
	/*For MT7623 reset MT7530 */
	reg_bit_one(RALINK_SYSCTL_BASE + 0x34, 2, 1);
	mdelay(1);
	reg_bit_zero(RALINK_SYSCTL_BASE + 0x34, 2, 1);
	mdelay(100);

	/* Do not continue switch setup with an unreadable or unready switch. */
	status = ra_switch_mdio_wait_nonzero(ra_switch_mdio_read,
					     ra_switch_mdio_reset_delay, NULL,
					     31U, 0x7800U, 100U, &reg_value);
	if (status != 0) {
		pr_err("MT7530 reset-ready check failed: %d\n", status);
		iounmap(gpio_base_virt);
		return status;
	}
	pr_info("MT7530 Reset Completed!!\n");

	for (i = 0; i <= 4; i++) {
		/*turn off PHY */
		status = ra_switch_mdio_read(NULL, i, 0x0, &reg_value);
		if (status != 0)
			goto mdio_error;
		reg_value |= (0x1 << 11);
		status = ra_switch_mdio_write(NULL, i, 0x0, reg_value);
		if (status != 0)
			goto mdio_error;
	}
	status = ra_switch_mdio_write(NULL, 31, 0x7000, 0x3);	/* reset switch */
	if (status != 0)
		goto mdio_error;
	usleep_range(100, 110);

	#if defined(CONFIG_GE1_RGMII_FORCE_1200)

	if (ei_local->chip_name == MT7621_FE) {
	status = trgmii_set_7530();
	if (status != 0)
		goto mdio_error;
	/* enable MDIO to control MT7530 */
	reg_value = sys_reg_read(RALINK_SYSCTL_BASE + 0x60);
	reg_value &= ~(0x3 << 12);
	sys_reg_write(RALINK_SYSCTL_BASE + 0x60, reg_value);
	}

	#endif

	/* (GE1, Force 1000M/FD, FC ON) */
	sys_reg_write(RALINK_ETH_SW_BASE + 0x100, 0x2105e33b);
	status = ra_switch_mdio_write(NULL, 31, 0x3600, 0x5e33b);
	if (status != 0)
		goto mdio_error;
	status = ra_switch_mdio_read(NULL, 31, 0x3600, &reg_value);
	if (status != 0)
		goto mdio_error;
	/* (GE2, Link down) */
	sys_reg_write(RALINK_ETH_SW_BASE + 0x200, 0x00008000);

	status = ra_switch_mdio_read(NULL, 31, 0x7804, &reg_value);
	if (status != 0)
		goto mdio_error;
	reg_value &= ~(1 << 8);	/* Enable Port 6 */
	reg_value |= (1 << 6);	/* Disable Port 5 */
	reg_value |= (1 << 13);	/* Port 5 as GMAC, no Internal PHY */

	if (ei_local->architecture & GMAC2) {
		/*RGMII2=Normal mode */
		reg_bit_zero(RALINK_SYSCTL_BASE + 0x60, 15, 1);

		/*GMAC2= RGMII mode */
		reg_bit_zero(SYSCFG1, 14, 2);
		if (ei_local->architecture & GE2_RGMII_AN) {
			status = ra_switch_mdio_write(NULL, 31, 0x3500, 0x56300);
			if (status != 0)
				goto mdio_error;
			/* (GE2, auto-polling) */
			sys_reg_write(RALINK_ETH_SW_BASE + 0x200, 0x21056300);
			reg_value |= (1 << 6);	/* disable MT7530 P5 */
			enable_auto_negotiate(ei_local);

		} else {
			/* MT7530 P5 Force 1000 */
			status = ra_switch_mdio_write(NULL, 31, 0x3500, 0x5e33b);
			if (status != 0)
				goto mdio_error;
			/* (GE2, Force 1000) */
			sys_reg_write(RALINK_ETH_SW_BASE + 0x200, 0x2105e33b);
			reg_value &= ~(1 << 6);	/* enable MT7530 P5 */
			reg_value |= ((1 << 7) | (1 << 13) | (1 << 16));
			if (ei_local->architecture & WAN_AT_P0)
				reg_value |= (1 << 20);
			else
				reg_value &= ~(1 << 20);
		}
	}
	reg_value &= ~(1 << 5);
	reg_value |= (1 << 16);	/* change HW-TRAP */
	pr_info("change HW-TRAP to 0x%x\n", reg_value);
	status = ra_switch_mdio_write(NULL, 31, 0x7804, reg_value);
	if (status != 0)
		goto mdio_error;
	status = ra_switch_mdio_read(NULL, 31, 0x7800, &reg_value);
	if (status != 0)
		goto mdio_error;
	reg_value = (reg_value >> 9) & 0x3;
	if (reg_value == 0x3) {	/* 25Mhz Xtal */
		xtal_mode = 1;
		/*Do Nothing */
	} else if (reg_value == 0x2) {	/* 40Mhz */
		xtal_mode = 2;
		/* disable MT7530 core clock */
		status = ra_switch_mdio_write_cl45(0, 0x1f, 0x410, 0x0);
		if (status != 0)
			goto mdio_error;

		status = ra_switch_mdio_write_cl45(0, 0x1f, 0x40d, 0x2020);
		if (status != 0)
			goto mdio_error;
		status = ra_switch_mdio_write_cl45(0, 0x1f, 0x40e, 0x119);
		if (status != 0)
			goto mdio_error;
		status = ra_switch_mdio_write_cl45(0, 0x1f, 0x40d, 0x2820);
		if (status != 0)
			goto mdio_error;
		usleep_range(20, 30);	/* suggest by CD */
	#if defined(CONFIG_GE1_RGMII_FORCE_1200)
		status = ra_switch_mdio_write_cl45(0, 0x1f, 0x410, 0x3);
		if (status != 0)
			goto mdio_error;
	#else
		status = ra_switch_mdio_write_cl45(0, 0x1f, 0x410, 0x1);
		if (status != 0)
			goto mdio_error;
	#endif

	} else {
		xtal_mode = 3;
	 /* TODO */}

	/* set MT7530 central align */
	#if !defined(CONFIG_GE1_RGMII_FORCE_1200)  /* for RGMII 1000HZ */
	status = ra_switch_mdio_read(NULL, 31, 0x7830, &reg_value);
	if (status != 0)
		goto mdio_error;
	reg_value &= ~1;
	reg_value |= 1 << 1;
	status = ra_switch_mdio_write(NULL, 31, 0x7830, reg_value);
	if (status != 0)
		goto mdio_error;

	status = ra_switch_mdio_read(NULL, 31, 0x7a40, &reg_value);
	if (status != 0)
		goto mdio_error;
	reg_value &= ~(1 << 30);
	status = ra_switch_mdio_write(NULL, 31, 0x7a40, reg_value);
	if (status != 0)
		goto mdio_error;

	reg_value = 0x855;
	status = ra_switch_mdio_write(NULL, 31, 0x7a78, reg_value);
	if (status != 0)
		goto mdio_error;
	#endif

	status = ra_switch_mdio_write(NULL, 31, 0x7b00, 0x104);	/* delay setting for 10/1000M */
	if (status != 0)
		goto mdio_error;
	status = ra_switch_mdio_write(NULL, 31, 0x7b04, 0x10);	/* delay setting for 10/1000M */
	if (status != 0)
		goto mdio_error;

	/*Tx Driving */
	status = ra_switch_mdio_write(NULL, 31, 0x7a54, 0x88);	/* lower GE1 driving */
	if (status != 0)
		goto mdio_error;
	status = ra_switch_mdio_write(NULL, 31, 0x7a5c, 0x88);	/* lower GE1 driving */
	if (status != 0)
		goto mdio_error;
	status = ra_switch_mdio_write(NULL, 31, 0x7a64, 0x88);	/* lower GE1 driving */
	if (status != 0)
		goto mdio_error;
	status = ra_switch_mdio_write(NULL, 31, 0x7a6c, 0x88);	/* lower GE1 driving */
	if (status != 0)
		goto mdio_error;
	status = ra_switch_mdio_write(NULL, 31, 0x7a74, 0x88);	/* lower GE1 driving */
	if (status != 0)
		goto mdio_error;
	status = ra_switch_mdio_write(NULL, 31, 0x7a7c, 0x88);	/* lower GE1 driving */
	if (status != 0)
		goto mdio_error;
	status = ra_switch_mdio_write(NULL, 31, 0x7810, 0x11);	/* lower GE2 driving */
	if (status != 0)
		goto mdio_error;
	/*Set MT7623 TX Driving */
	sys_reg_write(ETHDMASYS_ETH_SW_BASE + 0x0354, 0x88);
	sys_reg_write(ETHDMASYS_ETH_SW_BASE + 0x035c, 0x88);
	sys_reg_write(ETHDMASYS_ETH_SW_BASE + 0x0364, 0x88);
	sys_reg_write(ETHDMASYS_ETH_SW_BASE + 0x036c, 0x88);
	sys_reg_write(ETHDMASYS_ETH_SW_BASE + 0x0374, 0x88);
	sys_reg_write(ETHDMASYS_ETH_SW_BASE + 0x037c, 0x88);

	/* Set GE2 driving and slew rate */
	if (ei_local->architecture & GE2_RGMII_AN)
		sys_reg_write((void __iomem *)gpio_base_virt + 0xf00, 0xe00);
	else
		sys_reg_write((void __iomem *)gpio_base_virt + 0xf00, 0xa00);
	/* set GE2 TDSEL */
	sys_reg_write((void __iomem *)gpio_base_virt + 0x4c0, 0x5);
	/* set GE2 TUNE */
	sys_reg_write((void __iomem *)gpio_base_virt + 0xed0, 0);

	if (ei_local->chip_name == MT7623_FE) {
		status = mt7530_trgmii_clock_setting(xtal_mode);
		if (status != 0)
			goto mdio_error;
	}
	if (ei_local->architecture & GE1_RGMII_FORCE_1000) {
		sys_reg_write(ETHDMASYS_ETH_SW_BASE + 0x0350, 0x55);
		sys_reg_write(ETHDMASYS_ETH_SW_BASE + 0x0358, 0x55);
		sys_reg_write(ETHDMASYS_ETH_SW_BASE + 0x0360, 0x55);
		sys_reg_write(ETHDMASYS_ETH_SW_BASE + 0x0368, 0x55);
		sys_reg_write(ETHDMASYS_ETH_SW_BASE + 0x0370, 0x55);
		sys_reg_write(ETHDMASYS_ETH_SW_BASE + 0x0378, 0x855);
	}

#if !defined (CONFIG_RAETH_ESW_CONTROL)
	status = lan_wan_partition();
	if (status != 0)
		goto mdio_error;
#endif
	status = mt7530_phy_setting();
	if (status != 0)
		goto mdio_error;
	for (i = 0; i <= 4; i++) {
		/*turn on PHY */
		status = ra_switch_mdio_read(NULL, i, 0x0, &reg_value);
		if (status != 0)
			goto mdio_error;
		reg_value &= ~(0x1 << 11);
		status = ra_switch_mdio_write(NULL, i, 0x0, reg_value);
		if (status != 0)
			goto mdio_error;
	}

	status = ra_switch_mdio_read(NULL, 31, 0x7808, &reg_value);
	if (status != 0)
		goto mdio_error;
	reg_value |= (3 << 16);	/* Enable INTR */
	status = ra_switch_mdio_write(NULL, 31, 0x7808, reg_value);
	if (status != 0)
		goto mdio_error;

	iounmap(gpio_base_virt);
	return 0;

mdio_error:
	pr_err_ratelimited("%s: internal switch MDIO setup failed: %d\n",
			   __func__, status);
	iounmap(gpio_base_virt);
	return status;
}

void setup_external_gsw(void)
{
	/* reduce RGMII2 PAD driving strength */
	reg_bit_zero(PAD_RGMII2_MDIO_CFG, 4, 2);
	/*enable MDIO */
	reg_bit_zero(RALINK_SYSCTL_BASE + 0x60, 12, 2);

	/*RGMII1=Normal mode */
	reg_bit_zero(RALINK_SYSCTL_BASE + 0x60, 14, 1);
	/*GMAC1= RGMII mode */
	reg_bit_zero(SYSCFG1, 12, 2);

	/* (GE1, Link down) */
	sys_reg_write(RALINK_ETH_SW_BASE + 0x100, 0x00008000);

	/*RGMII2=Normal mode */
	reg_bit_zero(RALINK_SYSCTL_BASE + 0x60, 15, 1);
	/*GMAC2= RGMII mode */
	reg_bit_zero(SYSCFG1, 14, 2);

	/* (GE2, Force 1000M/FD, FC ON) */
	sys_reg_write(RALINK_ETH_SW_BASE + 0x200, 0x2105e33b);

} int is_marvell_gigaphy(int ge)
{
	struct END_DEVICE *ei_local = netdev_priv(dev_raether);
	u32 phy_id0 = 0, phy_id1 = 0, phy_address;

	if (ei_local->architecture & GE1_RGMII_AN)
		phy_address = mac_to_gigaphy_mode_addr;
	else
		phy_address = mac_to_gigaphy_mode_addr2;

	if (!ra_switch_mdio_read(NULL, phy_address, 2, &phy_id0))
		return -EIO;
	if (!ra_switch_mdio_read(NULL, phy_address, 3, &phy_id1))
		return -EIO;

	if ((phy_id0 == EV_MARVELL_PHY_ID0) && (phy_id1 == EV_MARVELL_PHY_ID1))
		return 1;
	return 0;
}

int is_vtss_gigaphy(int ge)
{
	struct END_DEVICE *ei_local = netdev_priv(dev_raether);
	u32 phy_id0 = 0, phy_id1 = 0, phy_address;

	if (ei_local->architecture & GE1_RGMII_AN)
		phy_address = mac_to_gigaphy_mode_addr;
	else
		phy_address = mac_to_gigaphy_mode_addr2;

	if (!ra_switch_mdio_read(NULL, phy_address, 2, &phy_id0))
		return -EIO;
	if (!ra_switch_mdio_read(NULL, phy_address, 3, &phy_id1))
		return -EIO;

	if ((phy_id0 == EV_VTSS_PHY_ID0) && (phy_id1 == EV_VTSS_PHY_ID1))
		return 1;
	return 0;
}

void fe_sw_preinit(struct END_DEVICE *ei_local)
{
	struct device_node *np = ei_local->switch_np;
	struct platform_device *pdev = of_find_device_by_node(np);
	struct mtk_gsw *gsw;
	int ret;

	gsw = platform_get_drvdata(pdev);
	if (!gsw) {
		pr_info("Failed to get gsw\n");
		return;
	}

	regulator_set_voltage(gsw->supply, 1000000, 1000000);
	ret = regulator_enable(gsw->supply);
	if (ret)
		pr_info("Failed to enable mt7530 power: %d\n", ret);

	if (gsw->mcm) {
		regulator_set_voltage(gsw->b3v, 3300000, 3300000);
		ret = regulator_enable(gsw->b3v);
		if (ret)
			dev_err(&pdev->dev, "Failed to enable b3v: %d\n", ret);
	} else {
		ret = devm_gpio_request(&pdev->dev, gsw->reset_pin,
					"mediatek,reset-pin");
		if (ret)
			pr_info("fail to devm_gpio_request\n");

		gpio_direction_output(gsw->reset_pin, 0);
		usleep_range(1000, 1100);
		gpio_set_value(gsw->reset_pin, 1);
		mdelay(100);
		devm_gpio_free(&pdev->dev, gsw->reset_pin);
	}
}

int init_rtl8367s(void)
{
	struct END_DEVICE *ei_local = netdev_priv(dev_raether);
	static int switch_init;
	unsigned int reg_value;
	void __iomem *virt_addr;
	rtk_vlan_cfg_t vlan1, vlan2;

	if (switch_init)
		return 0;
#define GPIO_MODE2 0x10211330
#define GPIO_DIR1 0x10211010
#define GPIO_DOUT1 0x10211110

	if (ei_local->chip_name == MT7622_FE) {
		/* release switch reset */
		/* set GPIO54 as GPIO mode */
		virt_addr = ioremap(GPIO_MODE2, 0x10);
		if (!virt_addr) {
			pr_err_ratelimited("%s: failed to map GPIO mode registers\n", __func__);
			return -ENOMEM;
		}
		reg_value = sys_reg_read(virt_addr);
		reg_value &= 0xfff0ffff;
		reg_value |= (1 << 16);
		sys_reg_write(virt_addr, reg_value);
		iounmap(virt_addr);

		/* set GPIO54 as output mode */
		virt_addr = ioremap(GPIO_DIR1, 0x10);
		if (!virt_addr) {
			pr_err_ratelimited("%s: failed to map GPIO direction registers\n", __func__);
			return -ENOMEM;
		}
		reg_value = sys_reg_read(virt_addr);
		reg_value |= (1 << 22);
		sys_reg_write(virt_addr, reg_value);
		iounmap(virt_addr);

		/* set GPIO54 as output 1 */
		virt_addr = ioremap(GPIO_DOUT1, 0x10);
		if (!virt_addr) {
			pr_err_ratelimited("%s: failed to map GPIO output registers\n", __func__);
			return -ENOMEM;
		}
		reg_value = sys_reg_read(virt_addr);
		reg_value |= (1 << 22);
		sys_reg_write(virt_addr, reg_value);
		iounmap(virt_addr);

		mdelay(500);
	}
	rtk_hal_switch_init();
	switch_init = 1;

	/* Set LAN/WAN VLAN partition */
	memset(&vlan1, 0x00, sizeof(rtk_vlan_cfg_t));
	RTK_PORTMASK_PORT_SET(vlan1.mbr, EXT_PORT0);
	RTK_PORTMASK_PORT_SET(vlan1.mbr, UTP_PORT1);
	RTK_PORTMASK_PORT_SET(vlan1.mbr, UTP_PORT2);
	RTK_PORTMASK_PORT_SET(vlan1.mbr, UTP_PORT3);
	RTK_PORTMASK_PORT_SET(vlan1.untag, EXT_PORT0);
	RTK_PORTMASK_PORT_SET(vlan1.untag, UTP_PORT1);
	RTK_PORTMASK_PORT_SET(vlan1.untag, UTP_PORT2);
	RTK_PORTMASK_PORT_SET(vlan1.untag, UTP_PORT3);
	if (ei_local->architecture & WAN_AT_P0) {
		RTK_PORTMASK_PORT_SET(vlan1.mbr, UTP_PORT4);
		RTK_PORTMASK_PORT_SET(vlan1.untag, UTP_PORT4);
	} else {
		RTK_PORTMASK_PORT_SET(vlan1.mbr, UTP_PORT0);
		RTK_PORTMASK_PORT_SET(vlan1.untag, UTP_PORT0);
	}
	vlan1.ivl_en = 1;
	rtk_vlan_set(1, &vlan1);

	memset(&vlan2, 0x00, sizeof(rtk_vlan_cfg_t));
	RTK_PORTMASK_PORT_SET(vlan2.mbr, EXT_PORT1);
	RTK_PORTMASK_PORT_SET(vlan2.untag, EXT_PORT1);
	if (ei_local->architecture & WAN_AT_P0) {
		RTK_PORTMASK_PORT_SET(vlan2.mbr, UTP_PORT0);
		RTK_PORTMASK_PORT_SET(vlan2.untag, UTP_PORT0);
	} else {
		RTK_PORTMASK_PORT_SET(vlan2.mbr, UTP_PORT4);
		RTK_PORTMASK_PORT_SET(vlan2.untag, UTP_PORT4);
	}
	vlan2.ivl_en = 1;
	rtk_vlan_set(2, &vlan2);

	rtk_hal_vlan_portpvid_set(EXT_PORT0, 1, 0);
	rtk_hal_vlan_portpvid_set(UTP_PORT1, 1, 0);
	rtk_hal_vlan_portpvid_set(UTP_PORT2, 1, 0);
	rtk_hal_vlan_portpvid_set(UTP_PORT3, 1, 0);
	rtk_hal_vlan_portpvid_set(EXT_PORT1, 2, 0);
	if (ei_local->architecture & WAN_AT_P0) {
		rtk_hal_vlan_portpvid_set(UTP_PORT0, 2, 0);
		rtk_hal_vlan_portpvid_set(UTP_PORT4, 1, 0);
	} else {
		rtk_hal_vlan_portpvid_set(UTP_PORT0, 1, 0);
		rtk_hal_vlan_portpvid_set(UTP_PORT4, 2, 0);
	}

	return 0;
}

int set_rtl8367s_sgmii(void)
{
	rtk_port_mac_ability_t mac_cfg;
	rtk_mode_ext_t mode;
	int status;

	status = init_rtl8367s();
	if (status != 0)
		return status;

	mode = MODE_EXT_HSGMII;
	mac_cfg.forcemode = MAC_FORCE;
	mac_cfg.speed = PORT_SPEED_2500M;
	mac_cfg.duplex = PORT_FULL_DUPLEX;
	mac_cfg.link = PORT_LINKUP;
	mac_cfg.nway = DISABLED;
	mac_cfg.txpause = ENABLED;
	mac_cfg.rxpause = ENABLED;
	rtk_port_macForceLinkExt_set(EXT_PORT0, mode, &mac_cfg);
	rtk_port_sgmiiNway_set(EXT_PORT0, DISABLED);
	rtk_port_phyEnableAll_set(ENABLED);
	return 0;
}

int set_rtl8367s_rgmii(void)
{
	rtk_port_mac_ability_t mac_cfg;
	rtk_mode_ext_t mode;
	int status;

	status = init_rtl8367s();
	if (status != 0)
		return status;

	mode = MODE_EXT_RGMII;
	mac_cfg.forcemode = MAC_FORCE;
	mac_cfg.speed = PORT_SPEED_1000M;
	mac_cfg.duplex = PORT_FULL_DUPLEX;
	mac_cfg.link = PORT_LINKUP;
	mac_cfg.nway = DISABLED;
	mac_cfg.txpause = ENABLED;
	mac_cfg.rxpause = ENABLED;
	rtk_port_macForceLinkExt_set(EXT_PORT1, mode, &mac_cfg);
	rtk_port_rgmiiDelayExt_set(EXT_PORT1, 1, 3);
	rtk_port_phyEnableAll_set(ENABLED);
	return 0;
}

int set_sgmii_force_link(int port_num, int speed)
{
	void __iomem *ethsys_base;
	void __iomem *phya_base;
	void __iomem *sgmii_base;
	unsigned int reg_value;
	unsigned int sgmii_reg_phya, sgmii_reg;

	if (port_num == 1) {
		sgmii_reg_phya = SGMII_REG_PHYA_BASE0;
		sgmii_reg = SGMII_REG_BASE0;
	} else if (port_num == 2) {
		sgmii_reg_phya = SGMII_REG_PHYA_BASE1;
		sgmii_reg = SGMII_REG_BASE1;
	} else {
		return -EINVAL;
	}

	/* Acquire every register window before changing SGMII state. */
	ethsys_base = ioremap(ETHSYS_BASE, 0x20);
	if (!ethsys_base)
		return -ENOMEM;
	phya_base = ioremap(sgmii_reg_phya, 0x100);
	if (!phya_base) {
		iounmap(ethsys_base);
		return -ENOMEM;
	}
	sgmii_base = ioremap(sgmii_reg, 0x100);
	if (!sgmii_base) {
		iounmap(phya_base);
		iounmap(ethsys_base);
		return -ENOMEM;
	}

	reg_value = sys_reg_read(ethsys_base + 0x14);
	if (port_num == 1) {
		reg_value |= SGMII_CONFIG_0;
		set_ge1_force_1000();
	} else {
		reg_value |= SGMII_CONFIG_1;
		set_ge2_force_1000();
	}
	sys_reg_write(ethsys_base + 0x14, reg_value);
	reg_value = sys_reg_read(ethsys_base + 0x14);

	/* Set SGMII GEN2 speed(2.5G). */
	reg_value = sys_reg_read(phya_base + 0x28);
	reg_value |= speed << 2;
	sys_reg_write(phya_base + 0x28, reg_value);

	/* Disable SGMII AN and force the selected link mode. */
	reg_value = sys_reg_read(sgmii_base);
	reg_value &= ~(1 << 12);
	sys_reg_write(sgmii_base, reg_value);
	reg_value = sys_reg_read(sgmii_base + 0x20);
	sys_reg_write(sgmii_base + 0x20, 0x31120019);
	reg_value = sys_reg_read(sgmii_base + 0x20);
	reg_value = sys_reg_read(sgmii_base + 0xe8);
	reg_value &= ~(1 << 4);
	sys_reg_write(sgmii_base + 0xe8, reg_value);

	iounmap(sgmii_base);
	iounmap(phya_base);
	iounmap(ethsys_base);
	return 0;
}

int set_sgmii_an(int port_num)
{
	void __iomem *ethsys_base;
	void __iomem *mac_base;
	void __iomem *sgmii_base;
	unsigned int reg_value;
	unsigned int sgmii_reg;

	if (port_num == 1)
		sgmii_reg = SGMII_REG_BASE0;
	else if (port_num == 2)
		sgmii_reg = SGMII_REG_BASE1;
	else
		return -EINVAL;

	/* Map all windows first so an allocation failure cannot leave a
	 * half-programmed SGMII auto-negotiation path.
	 */
	ethsys_base = ioremap(ETHSYS_BASE, 0x20);
	if (!ethsys_base)
		return -ENOMEM;
	mac_base = ioremap(ETHSYS_MAC_BASE, 0x300);
	if (!mac_base) {
		iounmap(ethsys_base);
		return -ENOMEM;
	}
	sgmii_base = ioremap(sgmii_reg, 0x100);
	if (!sgmii_base) {
		iounmap(mac_base);
		iounmap(ethsys_base);
		return -ENOMEM;
	}

	reg_value = sys_reg_read(ethsys_base + 0x14);
	if (port_num == 1)
		reg_value |= SGMII_CONFIG_0;
	else
		reg_value |= SGMII_CONFIG_1;
	sys_reg_write(ethsys_base + 0x14, reg_value);

	/* Set auto polling. */
	sys_reg_write(mac_base + (0x100 * port_num), 0x21056300);

	/* Configure link timer, remote fault handling and restart AN. */
	sys_reg_write(sgmii_base + 0x18, 0x186a0);
	reg_value = sys_reg_read(sgmii_base + 0x20);
	reg_value |= 1 << 8;
	sys_reg_write(sgmii_base + 0x20, reg_value);
	reg_value = sys_reg_read(sgmii_base);
	reg_value |= 1 << 9;
	sys_reg_write(sgmii_base, reg_value);
	reg_value = sys_reg_read(sgmii_base + 0xe8);
	reg_value &= ~(1 << 4);
	sys_reg_write(sgmii_base + 0xe8, reg_value);

	iounmap(sgmii_base);
	iounmap(mac_base);
	iounmap(ethsys_base);
	return 0;
}

static int mt7622_esw_5port_gpio(void)
{
	u32 ret, value, i;
	int status;

	status = ra_switch_mdio_write(NULL, 0, 31, 0x2000); /* change G2 page */
	if (status != 0)
		goto mdio_error;

	status = ra_switch_mdio_read(NULL, 0U, 31U, &value);
	if (status != 0)
		goto mdio_error;
	ret = 1;
	pr_debug("(%d) R31: %x!\n", ret, value);

	status = ra_switch_mdio_read(NULL, 0, 25, &value);
	if (status != 0)
		goto mdio_error;
	value = 0xf020;
	status = ra_switch_mdio_write(NULL, 0, 25, value);
	if (status != 0)
		goto mdio_error;
	status = ra_switch_mdio_read(NULL, 0, 25, &value);
	if (status != 0)
		goto mdio_error;
	pr_debug("G2_R25: %x!\n", value);

	status = ra_switch_mdio_write(NULL, 0, 31, 0x7000); /* change G7 page */
	if (status != 0)
		goto mdio_error;
	status = ra_switch_mdio_read(NULL, 0, 22, &value);
	if (status != 0)
		goto mdio_error;

	if (value & 0x8000) {
		pr_debug("G7_R22[15]: 1\n");
	} else {
		status = ra_switch_mdio_write(NULL, 0, 22, (value | (1 << 15)));
		if (status != 0)
			goto mdio_error;
		pr_debug("G7_R22[15]: set to 1\n");
	}

	status = ra_switch_mdio_write(NULL, 0, 31, 0x3000); /* change G3 page */
	if (status != 0)
		goto mdio_error;
	status = ra_switch_mdio_read(NULL, 0, 16, &value);
	if (status != 0)
		goto mdio_error;
	value |= (1 << 3);
	status = ra_switch_mdio_write(NULL, 0, 16, value);
	if (status != 0)
		goto mdio_error;

	status = ra_switch_mdio_read(NULL, 0, 16, &value);
	if (status != 0)
		goto mdio_error;
	pr_debug("G3_R16: %x!\n", value);

	status = ra_switch_mdio_write(NULL, 0, 31, 0x7000); /* change G7 page */
	if (status != 0)
		goto mdio_error;
	status = ra_switch_mdio_read(NULL, 0, 22, &value);
	if (status != 0)
		goto mdio_error;
	value |= (1 << 5);
	status = ra_switch_mdio_write(NULL, 0, 22, value);
	if (status != 0)
		goto mdio_error;

	status = ra_switch_mdio_read(NULL, 0, 24, &value);
	if (status != 0)
		goto mdio_error;
	value &= 0xDFFF;
	status = ra_switch_mdio_write(NULL, 0, 24, value);
	if (status != 0)
		goto mdio_error;

	status = ra_switch_mdio_read(NULL, 0, 24, &value);
	if (status != 0)
		goto mdio_error;
	value |= (1 << 14);
	status = ra_switch_mdio_write(NULL, 0, 24, value);
	if (status != 0)
		goto mdio_error;

	status = ra_switch_mdio_read(NULL, 0, 22, &value);
	if (status != 0)
		goto mdio_error;
	pr_debug("G7_R22: %x!\n", value);

	status = ra_switch_mdio_read(NULL, 0, 24, &value);
	if (status != 0)
		goto mdio_error;
	pr_debug("G7_R24: %x!\n", value);

	for (i = 0; i <= 4; i++) {
		status = ra_switch_mdio_write(NULL, i, 31, 0x8000); /* change L0 page */
		if (status != 0)
			goto mdio_error;

		status = ra_switch_mdio_read(NULL, i, 30, &value);
		if (status != 0)
			goto mdio_error;
		value |= 0x3FFF;
		status = ra_switch_mdio_write(NULL, i, 30, value);
		if (status != 0)
			goto mdio_error;
		status = ra_switch_mdio_read(NULL, i, 30, &value);
		if (status != 0)
			goto mdio_error;
		pr_debug("port %d L0_R30: %x!\n", i, value);

		status = ra_switch_mdio_write(NULL, i, 31, 0xB000); /* change L3 page */
		if (status != 0)
			goto mdio_error;

		status = ra_switch_mdio_read(NULL, i, 26, &value);
		if (status != 0)
			goto mdio_error;
		value |= (1 << 12);
		status = ra_switch_mdio_write(NULL, i, 26, value);
		if (status != 0)
			goto mdio_error;

		status = ra_switch_mdio_read(NULL, i, 26, &value);
		if (status != 0)
			goto mdio_error;
		pr_debug("port %d L3_R26: %x!\n", i, value);

		status = ra_switch_mdio_read(NULL, i, 25, &value);
		if (status != 0)
			goto mdio_error;
		value |= (1 << 8);
		value |= (1 << 12);
		status = ra_switch_mdio_write(NULL, i, 25, value);
		if (status != 0)
			goto mdio_error;

		status = ra_switch_mdio_read(NULL, i, 25, &value);
		if (status != 0)
			goto mdio_error;
		pr_debug("port %d L3_R25: %x!\n", i, value);
	}

	status = ra_switch_mdio_write(NULL, 0, 31, 0x2000); /* change G2 page */
	if (status != 0)
		goto mdio_error;

	status = ra_switch_mdio_read(NULL, 0, 25, &value);
	if (status != 0)
		goto mdio_error;

	pr_debug("G2_R25 before: %x!\n", value);
	/* value &= 0xFFFF3FFF; */
	/* G2_R25: 1020!-->0020 */
	/* value &= 0xFFFF2FFF; */
	value = 0x20;
	status = ra_switch_mdio_write(NULL, 0, 25, value);
	if (status != 0)
		goto mdio_error;

	status = ra_switch_mdio_read(NULL, 0, 25, &value);
	if (status != 0)
		goto mdio_error;
	pr_debug("G2_R25: %x!\n", value);

	/* LDO */
	status = ra_switch_mdio_write(NULL, 0, 31, 0x7000); /* change G7 page */
	if (status != 0)
		goto mdio_error;

	status = ra_switch_mdio_read(NULL, 0, 16, &value);
	if (status != 0)
		goto mdio_error;
	value |= (1 << 2);
	status = ra_switch_mdio_write(NULL, 0, 16, value);
	if (status != 0)
		goto mdio_error;

	status = ra_switch_mdio_read(NULL, 0, 16, &value);
	if (status != 0)
		goto mdio_error;
	pr_debug("G7_R16: %x!\n", value);

	/* BG */
	status = ra_switch_mdio_write(NULL, 0, 31, 0x2000); /* change G2 page */
	if (status != 0)
		goto mdio_error;

	status = ra_switch_mdio_read(NULL, 0, 22, &value);
	if (status != 0)
		goto mdio_error;
	value |= (1 << 12);
	value |= (1 << 13);
	value |= (1 << 14);
	status = ra_switch_mdio_write(NULL, 0, 22, value);
	if (status != 0)
		goto mdio_error;

	status = ra_switch_mdio_read(NULL, 0, 22, &value);
	if (status != 0)
		goto mdio_error;
	pr_debug("G2_R22: %x!\n", value);

	status = ra_switch_mdio_read(NULL, 0, 22, &value);
	if (status != 0)
		goto mdio_error;
	value &= 0x7FFF;
	status = ra_switch_mdio_write(NULL, 0, 22, value);
	if (status != 0)
		goto mdio_error;

	status = ra_switch_mdio_read(NULL, 0, 22, &value);
	if (status != 0)
		goto mdio_error;
	pr_debug("G2_R22: %x!\n", value);
	return 0;

mdio_error:
	pr_err_ratelimited("%s: embedded-switch GPIO PHY setup failed: %d\n",
			   __func__, status);
	return status;
}

int leopard_gmii_config(u8 enable)
{
	unsigned int reg_value = 0;
	void __iomem *gpio_base_virt, *infra_base_virt;
	struct END_DEVICE *ei_local = netdev_priv(dev_raether);

	/* Acquire both register windows before changing the PHY mux. */
	infra_base_virt = ioremap(INFRA_BASE, 0x10);
	if (!infra_base_virt) {
		return -ENOMEM;
	}
	gpio_base_virt = ioremap(GPIO_GO_BASE, 0x10);
	if (!gpio_base_virt) {
		iounmap(infra_base_virt);
		return -ENOMEM;
	}

	/*bit[1]: gphy connect GMAC0 or GMAC2 1:GMAC0. 0:GMAC2*/
	/*bit[0]: Co-QPHY path selection 0:U3path, 1:SGMII*/
	reg_value = sys_reg_read(infra_base_virt);
	if (enable) {
		reg_value |= 0x02;
		sys_reg_write(infra_base_virt, reg_value);

		mac_to_gigaphy_mode_addr = 0;
		enable_auto_negotiate(ei_local);

		/*port5 enable*/
		sys_reg_write(ETHDMASYS_ETH_SW_BASE + 0x90, 0x00007f7f);
		/*port5 an mode, port6 fix*/
		sys_reg_write(ETHDMASYS_ETH_SW_BASE + 0xc8, 0x20503bfa);
	} else {
		reg_value &= ~0x2;
		sys_reg_write(infra_base_virt, reg_value);
		sys_reg_write(ETHDMASYS_ETH_SW_BASE + 0x84, 0);
		sys_reg_write(ETHDMASYS_ETH_SW_BASE + 0x90, 0x10007f7f);
		sys_reg_write(ETHDMASYS_ETH_SW_BASE + 0xc8, 0x05503f38);
	}

	/*10000710 GEPHY_CTRL0[9:6] = 0 */
	reg_value = 0x10000820;
	sys_reg_write(gpio_base_virt, reg_value);
	iounmap(gpio_base_virt);
	iounmap(infra_base_virt);
	return 0;
}

int fe_sw_init(void)
{
	struct END_DEVICE *ei_local = netdev_priv(dev_raether);
	unsigned int reg_value = 0;
	void __iomem *gpio_base_virt, *infra_base_virt, *ethsys_base_virt;
	int i;
	int status;
	u16 r0_tmp;

	/* Case1: MT7623/MT7622 GE1 + GigaPhy */
	if (ei_local->architecture & GE1_RGMII_AN) {
		enable_auto_negotiate(ei_local);
		status = is_marvell_gigaphy(1);
		if (status < 0)
			goto mdio_error;
		if (status > 0) {
			if (ei_local->features & FE_FPGA_MODE) {
				status = ra_switch_mdio_read(NULL, mac_to_gigaphy_mode_addr, 9, &reg_value);
				if (status != 0)
					goto mdio_error;
				/* turn off 1000Base-T Advertisement
				 * (9.9=1000Full, 9.8=1000Half)
				 */
				reg_value &= ~(3 << 8);
				status = ra_switch_mdio_write(NULL, mac_to_gigaphy_mode_addr, 9, reg_value);
				if (status != 0)
					goto mdio_error;

				/*10Mbps, debug */
				status = ra_switch_mdio_write(NULL, mac_to_gigaphy_mode_addr, 4, 0x461);
				if (status != 0)
					goto mdio_error;

				status = ra_switch_mdio_read(NULL, mac_to_gigaphy_mode_addr, 0, &reg_value);
				if (status != 0)
					goto mdio_error;
				reg_value |= 1 << 9;	/* restart AN */
				status = ra_switch_mdio_write(NULL, mac_to_gigaphy_mode_addr, 0, reg_value);
				if (status != 0)
					goto mdio_error;
			}
		}
		status = is_vtss_gigaphy(1);
		if (status < 0)
			goto mdio_error;
		if (status > 0) {
			status = ra_switch_mdio_write(NULL, mac_to_gigaphy_mode_addr, 31, 1);
			if (status != 0)
				goto mdio_error;
			status = ra_switch_mdio_read(NULL, mac_to_gigaphy_mode_addr, 28, &reg_value);
			if (status != 0)
				goto mdio_error;
			pr_info("Vitesse phy skew: %x --> ", reg_value);
			reg_value |= (0x3 << 12);
			reg_value &= ~(0x3 << 14);
			pr_info("%x\n", reg_value);
			status = ra_switch_mdio_write(NULL, mac_to_gigaphy_mode_addr, 28, reg_value);
			if (status != 0)
				goto mdio_error;
			status = ra_switch_mdio_write(NULL, mac_to_gigaphy_mode_addr, 31, 0);
			if (status != 0)
				goto mdio_error;
		}
	}

	/* Case2: RT3883/MT7621 GE2 + GigaPhy */
	if (ei_local->architecture & GE2_RGMII_AN) {
		status = leopard_gmii_config(0);
		if (status != 0)
			goto mdio_error;
		enable_auto_negotiate(ei_local);
		set_ge2_an();
		status = set_ge2_gmii();
		if (status != 0)
			goto mdio_error;
		if (ei_local->chip_name == LEOPARD_FE) {
			for (i = 1; i < 5; i++) {
				status = do_fe_phy_all_analog_cal_checked((u8)i);
				if (status != 0)
					goto mdio_error;
			}

			status = do_ge_phy_all_analog_cal(0U);
			if (status != 0)
				goto mdio_error;
		}

		status = is_marvell_gigaphy(2);
		if (status < 0)
			goto mdio_error;
		if (status > 0) {
			status = ra_switch_mdio_read(NULL, mac_to_gigaphy_mode_addr2, 9, &reg_value);
			if (status != 0)
				goto mdio_error;
			/* turn off 1000Base-T Advertisement
			 * (9.9=1000Full, 9.8=1000Half)
			 */
			reg_value &= ~(3 << 8);
			status = ra_switch_mdio_write(NULL, mac_to_gigaphy_mode_addr2, 9, reg_value);
			if (status != 0)
				goto mdio_error;

			status = ra_switch_mdio_read(NULL, mac_to_gigaphy_mode_addr2, 20, &reg_value);
			if (status != 0)
				goto mdio_error;
			/* Add delay to RX_CLK for RXD Outputs */
			reg_value |= 1 << 7;
			status = ra_switch_mdio_write(NULL, mac_to_gigaphy_mode_addr2, 20, reg_value);
			if (status != 0)
				goto mdio_error;

			status = ra_switch_mdio_read(NULL, mac_to_gigaphy_mode_addr2, 0, &reg_value);
			if (status != 0)
				goto mdio_error;
			reg_value |= 1 << 15;	/* PHY Software Reset */
			status = ra_switch_mdio_write(NULL, mac_to_gigaphy_mode_addr2, 0, reg_value);
			if (status != 0)
				goto mdio_error;
			if (ei_local->features & FE_FPGA_MODE) {
				status = ra_switch_mdio_read(NULL, mac_to_gigaphy_mode_addr2, 9, &reg_value);
				if (status != 0)
					goto mdio_error;
				/* turn off 1000Base-T Advertisement
				 * (9.9=1000Full, 9.8=1000Half)
				 */
				reg_value &= ~(3 << 8);
				status = ra_switch_mdio_write(NULL, mac_to_gigaphy_mode_addr2, 9, reg_value);
				if (status != 0)
					goto mdio_error;

				/*10Mbps, debug */
				status = ra_switch_mdio_write(NULL, mac_to_gigaphy_mode_addr2, 4, 0x461);
				if (status != 0)
					goto mdio_error;

				status = ra_switch_mdio_read(NULL, mac_to_gigaphy_mode_addr2, 0, &reg_value);
				if (status != 0)
					goto mdio_error;
				reg_value |= 1 << 9;	/* restart AN */
				status = ra_switch_mdio_write(NULL, mac_to_gigaphy_mode_addr2, 0, reg_value);
				if (status != 0)
					goto mdio_error;
			}
		}
		status = is_vtss_gigaphy(2);
		if (status < 0)
			goto mdio_error;
		if (status > 0) {
			status = ra_switch_mdio_write(NULL, mac_to_gigaphy_mode_addr2, 31, 1);
			if (status != 0)
				goto mdio_error;
			status = ra_switch_mdio_read(NULL, mac_to_gigaphy_mode_addr2, 28, &reg_value);
			if (status != 0)
				goto mdio_error;
			pr_info("Vitesse phy skew: %x --> ", reg_value);
			reg_value |= (0x3 << 12);
			reg_value &= ~(0x3 << 14);
			pr_info("%x\n", reg_value);
			status = ra_switch_mdio_write(NULL, mac_to_gigaphy_mode_addr2, 28, reg_value);
			if (status != 0)
				goto mdio_error;
			status = ra_switch_mdio_write(NULL, mac_to_gigaphy_mode_addr2, 31, 0);
			if (status != 0)
				goto mdio_error;
		}
	}

	/* Case3:  MT7623 GE1 + Internal GigaSW */
	if (ei_local->architecture &
	    (GE1_RGMII_FORCE_1000 | GE1_TRGMII_FORCE_2000 |
	     GE1_TRGMII_FORCE_2600)) {
		if ((ei_local->chip_name == MT7623_FE) ||
		    (ei_local->chip_name == MT7621_FE)) {
			status = setup_internal_gsw();
			if (status != 0)
				return status;
		}
		/* TODO
		 * else if (ei_local->features & FE_FPGA_MODE)
		 * setup_fpga_gsw();
		 * else
		 * sys_reg_write(MDIO_CFG, INIT_VALUE_OF_FORCE_1000_FD);
		 */
	}

	/* Case4: MT7623 GE2 + GigaSW */
	if (ei_local->architecture & GE2_RGMII_FORCE_1000) {
		set_ge2_force_1000();
		if (ei_local->chip_name == MT7623_FE)
			setup_external_gsw();
		if (ei_local->chip_name == MT7622_FE) {
			status = set_rtl8367s_rgmii();
			if (status != 0)
				goto mdio_error;
		}
	}
	/*TODO
	 * else
	 * sys_reg_write(MDIO_CFG2, INIT_VALUE_OF_FORCE_1000_FD);
	 */

	/* Case5: MT7622 embedded switch */
	if (ei_local->architecture & RAETH_ESW) {
		reg_value = sys_reg_read(ETHDMASYS_ETH_MAC_BASE + 0xC);
		reg_value = reg_value | 0x1;
		sys_reg_write(ETHDMASYS_ETH_MAC_BASE + 0xC, reg_value);

		if (ei_local->architecture & MT7622_EPHY) {
			gpio_base_virt = ioremap(GPIO_GO_BASE, 0x100);
			if (!gpio_base_virt) {
				status = -ENOMEM;
				goto mdio_error;
			}
			sys_reg_write(gpio_base_virt + 0xF0, 0xE0FFFFFF);
			iounmap(gpio_base_virt);
			gpio_base_virt = ioremap(GPIO_MODE_BASE, 0x100);
			if (!gpio_base_virt) {
				status = -ENOMEM;
				goto mdio_error;
			}
			reg_value = sys_reg_read(gpio_base_virt + 0x90);
			reg_value &= 0x0000ffff;
			reg_value |= 0x22220000;
			sys_reg_write(gpio_base_virt + 0x90, reg_value);
			iounmap(gpio_base_virt);
			sys_reg_write(ETHDMASYS_ETH_SW_BASE + 0x84, 0);
			sys_reg_write(ETHDMASYS_ETH_SW_BASE + 0x90, 0x10007f7f);
			sys_reg_write(ETHDMASYS_ETH_SW_BASE + 0xc8, 0x05503f38);
		} else if (ei_local->architecture & LEOPARD_EPHY) {
			set_ge1_an();
			/*port0 force link down*/
			sys_reg_write(ETHDMASYS_ETH_SW_BASE + 0x84, 0x8000000);
			sys_reg_write(ETHDMASYS_ETH_SW_BASE + 0x8c, 0x02404040);
			sys_reg_write(ETHDMASYS_ETH_SW_BASE + 0x98, 0x00007f7f);
			sys_reg_write(ETHDMASYS_ETH_SW_BASE + 0x04, 0xfbffffff);
			sys_reg_write(ETHDMASYS_ETH_SW_BASE + 0x9c, 0x0008a041);
			if (ei_local->architecture & LEOPARD_EPHY_GMII) {
				status = leopard_gmii_config(1);
				if (status != 0)
					goto mdio_error;
				status = set_ge0_gmii();
				if (status != 0)
					goto mdio_error;
			} else {
				status = leopard_gmii_config(0);
				if (status != 0)
					goto mdio_error;
			}
		}
	}

	/* clear SGMII setting */
	if ((ei_local->chip_name == LEOPARD_FE) || (ei_local->chip_name == MT7622_FE)) {
		ethsys_base_virt = ioremap(ETHSYS_BASE, 0x20);
		if (!ethsys_base_virt) {
			status = -ENOMEM;
			goto mdio_error;
		}
		reg_value = sys_reg_read(ethsys_base_virt + 0x14);
		reg_value &= ~(3 << 8);
		sys_reg_write(ethsys_base_virt + 0x14, reg_value);
		iounmap(ethsys_base_virt);
	}

	if (ei_local->architecture & GE1_SGMII_FORCE_2500) {
		status = set_rtl8367s_sgmii();
		if (status != 0)
			goto mdio_error;
		status = set_sgmii_force_link(1, 1);
		if (status != 0)
			goto mdio_error;
	} else if (ei_local->architecture & GE1_SGMII_AN) {
		enable_auto_negotiate(ei_local);
		status = set_sgmii_an(1);
		if (status != 0)
			goto mdio_error;
	}
	if (ei_local->chip_name == LEOPARD_FE) {
		if (ei_local->architecture & GE2_RAETH_SGMII) {
			/*bit[1]: gphy connect GMAC0 or GMAC2 1:GMAC0. 0:GMAC2*/
			/*bit[0]: Co-QPHY path selection 0:U3path, 1:SGMII*/
			infra_base_virt = ioremap(INFRA_BASE, 0x10);
			if (!infra_base_virt) {
				status = -ENOMEM;
				goto mdio_error;
			}
			reg_value = sys_reg_read(infra_base_virt);
			reg_value = reg_value | 0x01;
			sys_reg_write(infra_base_virt, reg_value);
			iounmap(infra_base_virt);
		}
	}

	if (ei_local->architecture & GE2_SGMII_FORCE_2500) {
		if (ei_local->architecture & SGMII_SWITCH) {
			status = set_rtl8367s_sgmii();
			if (status != 0)
				goto mdio_error;
		}
		status = set_sgmii_force_link(2, 1);
		if (status != 0)
			goto mdio_error;
	} else if (ei_local->architecture & GE2_SGMII_AN) {
		enable_auto_negotiate(ei_local);
		status = set_sgmii_an(2);
		if (status != 0)
			goto mdio_error;
	}

	if (ei_local->architecture & MT7622_EPHY) {
		status = mt7622_ephy_cal();
		if (status != 0)
			goto mdio_error;
	} else if (ei_local->architecture & LEOPARD_EPHY) {
		status = leopard_ephy_cal();
		if (status != 0)
			goto mdio_error;

		/* Stop initialization if any Leopard EPHY MDIO operation fails. */
		status = tc_phy_write_page_reg_checked(2U, 1U, 18U, 1, 0x021fU);
		if (status != 0)
			goto mdio_error;
		status = tc_phy_write_page_reg_checked(2U, 1U, 18U, 1, 0x022fU);
		if (status != 0)
			goto mdio_error;
		status = tc_phy_write_page_reg_checked(2U, 1U, 18U, 1, 0x023fU);
		if (status != 0)
			goto mdio_error;
		status = tc_phy_write_page_reg_checked(2U, 1U, 18U, 1, 0x024fU);
		if (status != 0)
			goto mdio_error;
		status = tc_phy_write_page_reg_checked(2U, 1U, 18U, 1, 0x004fU);
		if (status != 0)
			goto mdio_error;
		status = tc_phy_write_page_reg_checked(4U, 1U, 18U, 1, 0x021fU);
		if (status != 0)
			goto mdio_error;
		status = tc_phy_write_page_reg_checked(4U, 1U, 18U, 1, 0x022fU);
		if (status != 0)
			goto mdio_error;
		status = tc_phy_write_page_reg_checked(4U, 1U, 18U, 1, 0x002fU);
		if (status != 0)
			goto mdio_error;

		status = tc_phy_read_page_reg_checked(3U, 0U, 0U, 1, &reg_value);
		if (status != 0)
			goto mdio_error;
		r0_tmp = (u16)(reg_value | 0x0200U);
		status = tc_phy_write_page_reg_checked(3U, 0U, 0U, 1, r0_tmp);
		if (status != 0)
			goto mdio_error;
	}

	if (ei_local->chip_name == MT7621_FE) {
		clk_prepare_enable(ei_local->clks[MTK_CLK_GP0]);

		/* switch to esw */
		sys_reg_write(ETHDMASYS_ETH_MAC_BASE + 0xC, 0x1);

		/* set agpio to 5-port ephy */
		gpio_base_virt = ioremap(GPIO_GO_BASE, 0x100);
		if (!gpio_base_virt) {
			status = -ENOMEM;
			clk_disable_unprepare(ei_local->clks[MTK_CLK_GP0]);
			goto mdio_error;
		}
		reg_value = sys_reg_read(gpio_base_virt + 0xF0);
		reg_value &= 0xE0FFFFFF;
		sys_reg_write(gpio_base_virt + 0xF0, reg_value);
		iounmap(gpio_base_virt);

		/* set ephy to 5-port gpio mode */
		status = mt7622_esw_5port_gpio();
		if (status != 0) {
			clk_disable_unprepare(ei_local->clks[MTK_CLK_GP0]);
			goto mdio_error;
		}

		/* set agpio to 0-port ephy */
		gpio_base_virt = ioremap(GPIO_GO_BASE, 0x100);
		if (!gpio_base_virt) {
			status = -ENOMEM;
			clk_disable_unprepare(ei_local->clks[MTK_CLK_GP0]);
			goto mdio_error;
		}
		reg_value = sys_reg_read(gpio_base_virt + 0xF0);
		reg_value |= BITS(24, 28);
		sys_reg_write(gpio_base_virt + 0xF0, reg_value);
		iounmap(gpio_base_virt);

		/* switch back to gmac1 */
		sys_reg_write(ETHDMASYS_ETH_MAC_BASE + 0xC, 0x0);

		clk_disable_unprepare(ei_local->clks[MTK_CLK_GP0]);
	}

	if (ei_local->chip_name == MT7622_FE) {
		if (ei_local->features & FE_GE2_SUPPORT) {
			gpio_base_virt = ioremap(GPIO_GO_BASE + 0x100, 0x100);
			if (!gpio_base_virt) {
				status = -ENOMEM;
				goto mdio_error;
			}
			reg_value = sys_reg_read(gpio_base_virt + 0x70);
			reg_value = reg_value | (1 << 30);
			sys_reg_write(gpio_base_virt + 0x70, reg_value);
			reg_value = sys_reg_read(gpio_base_virt + 0x8c);
			reg_value = reg_value | (1 << 24);
			sys_reg_write(gpio_base_virt + 0x8c, reg_value);
			iounmap(gpio_base_virt);
		}
	}
	return 0;

mdio_error:
	pr_err_ratelimited("%s: switch/PHY initialization failed: %d\n",
			   __func__, status);
	return status;
}

void fe_sw_deinit(struct END_DEVICE *ei_local)
{
	struct device_node *np = ei_local->switch_np;
	struct platform_device *pdev = of_find_device_by_node(np);
	void __iomem *gpio_base_virt;
	unsigned int reg_value;
	struct mtk_gsw *gsw;
	int ret;

	gsw = platform_get_drvdata(pdev);
	if (!gsw)
		return;

	ret = regulator_disable(gsw->supply);
	if (ret)
		dev_err(&pdev->dev, "Failed to disable mt7530 power: %d\n", ret);

	if (gsw->mcm) {
		ret = regulator_disable(gsw->b3v);
		if (ret)
			dev_err(&pdev->dev, "Failed to disable b3v: %d\n", ret);
	}

	if (ei_local->architecture & MT7622_EPHY) {
		mt7622_esw_5port_gpio();
		gpio_base_virt = ioremap(GPIO_GO_BASE, 0x100);
		if (!gpio_base_virt) {
			pr_err_ratelimited("%s: failed to map MT7622 shutdown GPIO registers\n", __func__);
			return;
		}
		reg_value = sys_reg_read(gpio_base_virt + 0xF0);
		reg_value |= BITS(24, 28);
		sys_reg_write(gpio_base_virt + 0xF0, reg_value);
		iounmap(gpio_base_virt);
	} else if (ei_local->architecture & LEOPARD_EPHY) {
		mt7622_esw_5port_gpio();
		gpio_base_virt = ioremap(GPIO_GO_BASE, 0x10);
		if (!gpio_base_virt) {
			pr_err_ratelimited("%s: failed to map Leopard shutdown GPIO registers\n", __func__);
			return;
		}
		reg_value = sys_reg_read(gpio_base_virt);
		reg_value |= 0x3c0;
		sys_reg_write(gpio_base_virt, reg_value);
		iounmap(gpio_base_virt);

		gpio_base_virt = ioremap(GPIO_MODE_BASE, 0x100);
		if (!gpio_base_virt) {
			pr_err_ratelimited("%s: failed to map Leopard GPIO mode registers\n", __func__);
			return;
		}
		reg_value = sys_reg_read(gpio_base_virt + 0x10);
		reg_value &= 0x0000ffff;
		reg_value &= ~0xffff0000;
		sys_reg_write(gpio_base_virt + 0x10, reg_value);

		reg_value = sys_reg_read(gpio_base_virt + 0x20);
		reg_value &= ~0xfff00fff;
		sys_reg_write(gpio_base_virt + 0x20, reg_value);
		iounmap(gpio_base_virt);
	}
}

void (*esw_link_status_hook)(u32 port_id, int port_link) = NULL;
EXPORT_SYMBOL(esw_link_status_hook);

static int esw_link_status_changed(int port_no, void *dev_id)
{
	int port_link;
	int status;

	status = ra_switch_mdio_read_link_status(31U,
						 0x3008U + (port_no * 0x100U),
						 1U, ra_switch_mdio_read, NULL,
						 &port_link);
	if (status != 0)
		return status;
	if (esw_link_status_hook)
		esw_link_status_hook(port_no, port_link);
	return 0;
}

irqreturn_t gsw_interrupt(int irq, void *resv)
{
	unsigned long flags;
	unsigned int reg_int_val = 0;
	unsigned int ack_mask = 0;
	struct net_device *dev = dev_raether;
	struct END_DEVICE *ei_local = netdev_priv(dev);
	void *dev_id = NULL;
	int status;

	spin_lock_irqsave(&ei_local->page_lock, flags);
	status = ra_switch_mdio_read_checked(31U, 0x700cU,
					     ra_switch_mdio_read, NULL,
					     &reg_int_val);
	if (status != 0) {
		pr_err_ratelimited("%s: switch link interrupt status read failed (status=%d)\n",
				   __func__, status);
		goto out;
	}
	if (reg_int_val & P4_LINK_CH) {
		status = esw_link_status_changed(4, dev_id);
		if (status == 0)
			ack_mask |= P4_LINK_CH;
		else
			pr_err_ratelimited("%s: port 4 link status read failed (status=%d)\n", __func__, status);
	}
	if (reg_int_val & P3_LINK_CH) {
		status = esw_link_status_changed(3, dev_id);
		if (status == 0)
			ack_mask |= P3_LINK_CH;
		else
			pr_err_ratelimited("%s: port 3 link status read failed (status=%d)\n", __func__, status);
	}
	if (reg_int_val & P2_LINK_CH) {
		status = esw_link_status_changed(2, dev_id);
		if (status == 0)
			ack_mask |= P2_LINK_CH;
		else
			pr_err_ratelimited("%s: port 2 link status read failed (status=%d)\n", __func__, status);
	}
	if (reg_int_val & P1_LINK_CH) {
		status = esw_link_status_changed(1, dev_id);
		if (status == 0)
			ack_mask |= P1_LINK_CH;
		else
			pr_err_ratelimited("%s: port 1 link status read failed (status=%d)\n", __func__, status);
	}
	if (reg_int_val & P0_LINK_CH) {
		status = esw_link_status_changed(0, dev_id);
		if (status == 0)
			ack_mask |= P0_LINK_CH;
		else
			pr_err_ratelimited("%s: port 0 link status read failed (status=%d)\n", __func__, status);
	}

	/* Ack only sampled ports; failed reads remain pending for a later retry. */
	if (ack_mask && !mii_mgr_write(31U, 0x700cU, ack_mask))
		pr_err_ratelimited("%s: switch link interrupt ack failed (mask=0x%x)\n",
				   __func__, ack_mask);
out:
	spin_unlock_irqrestore(&ei_local->page_lock, flags);
	return IRQ_HANDLED;
}

static int phy_tr_dbg_checked(u8 phyaddr, const char *type,
			      u32 data_addr, u8 ch_num, u32 *result)
{
	u16 page_reg = 31;
	u32 token_ring_debug_reg = 0x52B5;
	u32 token_ring_control_reg = 0x10;
	u32 token_ring_low_data_reg = 0x11;
	u32 token_ring_high_data_reg = 0x12;
	u16 ch_addr = 0;
	u32 node_addr = 0;
	u32 value;
	u32 value_high = 0;
	u32 value_low = 0;
	unsigned long flags;
	int status;
	int restore_status;

	if (!type || !result)
		return -EINVAL;
	*result = 0;
	if (strncmp(type, "DSPF", 4) == 0) {
		ch_addr = 0x02;
		node_addr = 0x0D;
	} else if (strncmp(type, "PMA", 3) == 0) {
		ch_addr = 0x01;
		node_addr = 0x0F;
	} else if (strncmp(type, "TR", 2) == 0) {
		ch_addr = 0x01;
		node_addr = 0x0D;
	} else if (strncmp(type, "PCS", 3) == 0) {
		ch_addr = 0x02;
		node_addr = 0x0F;
	} else if (strncmp(type, "FFE", 3) == 0) {
		ch_addr = ch_num;
		node_addr = 0x04;
	} else if (strncmp(type, "EC", 2) == 0) {
		ch_addr = ch_num;
		node_addr = 0x00;
	} else if (strncmp(type, "ECT", 3) == 0) {
		ch_addr = ch_num;
		node_addr = 0x01;
	} else if (strncmp(type, "NC", 2) == 0) {
		ch_addr = ch_num;
		node_addr = 0x01;
	} else if (strncmp(type, "DFEDC", 5) == 0) {
		ch_addr = ch_num;
		node_addr = 0x05;
	} else if (strncmp(type, "DEC", 3) == 0) {
		ch_addr = 0x00;
		node_addr = 0x07;
	} else if (strncmp(type, "CRC", 3) == 0) {
		ch_addr = ch_num;
		node_addr = 0x06;
	} else if (strncmp(type, "AN", 2) == 0) {
		ch_addr = 0x00;
		node_addr = 0x0F;
	} else if (strncmp(type, "CMI", 3) == 0) {
		ch_addr = 0x03;
		node_addr = 0x0F;
	} else if (strncmp(type, "SUPV", 4) == 0) {
		ch_addr = 0x00;
		node_addr = 0x0D;
	} else {
		return -EINVAL;
	}
	data_addr &= 0x3F;

	/* Serialize the token-ring page/control/data sequence against paged PHY I/O. */
	spin_lock_irqsave(&ra_switch_phy_page_lock, flags);
	status = ra_switch_mdio_write(NULL, phyaddr, page_reg,
				      token_ring_debug_reg);
	if (status == 0)
		status = ra_switch_mdio_write(NULL, phyaddr,
					      token_ring_control_reg,
					      (1U << 15) | (1U << 13) |
					      (ch_addr << 11) |
					      (node_addr << 7) |
					      (data_addr << 1));
	if (status == 0)
		status = ra_switch_mdio_read_checked(phyaddr,
						     token_ring_low_data_reg,
						     ra_switch_mdio_read,
						     NULL, &value_low);
	if (status == 0)
		status = ra_switch_mdio_read_checked(phyaddr,
						     token_ring_high_data_reg,
						     ra_switch_mdio_read,
						     NULL, &value_high);

	/* Restore page 0 after success or failure; retain the first error. */
	restore_status = ra_switch_mdio_write(NULL, phyaddr, page_reg, 0);
	if (status == 0)
		status = restore_status;
	spin_unlock_irqrestore(&ra_switch_phy_page_lock, flags);
	if (status != 0)
		return status;

	value = value_low + ((value_high & 0x00FF) << 16);
	*result = value;
	pr_info("*%s => Phyaddr=%d, ch_addr=0x%X, node_addr=0x%X, data_addr=0x%X , value=0x%X\r\n",
		type, phyaddr, ch_addr, node_addr, data_addr, value);
	return 0;
}

/* Keep the legacy value-returning entry point for existing in-tree callers. */
u32 phy_tr_dbg(u8 phyaddr, char *type, u32 data_addr, u8 ch_num)
{
	u32 value = 0;
	int status;

	status = phy_tr_dbg_checked(phyaddr, type, data_addr, ch_num, &value);
	if (status != 0) {
		pr_err_ratelimited("%s: token-ring PHY debug read failed (phy=%u status=%d)\n",
				   __func__, phyaddr, status);
		return 0xFFFF;
	}
	return value;
}

static int esw_debug_read_mmd(u32 phy_addr, u32 dev_addr,
			      u32 reg_addr, u32 *value)
{
	int status;

	status = ra_switch_mdio_read_cl45_checked(phy_addr, dev_addr, reg_addr,
						  ra_switch_mdio_read_cl45,
						  NULL, value);
	if (status != 0)
		pr_err_ratelimited("%s: PHY debug CL45 read failed (phy=%u dev=%u reg=0x%x status=%d)\n",
				   __func__, phy_addr, dev_addr, reg_addr, status);
	return status;
}

void esw_show_debug_log(u32 phy_addr)
{
	u32 val;
	int status;

	status = phy_tr_dbg_checked(phy_addr, "PMA", 0x38, 0, &val);
	if (status != 0) {
		pr_err_ratelimited("%s: token-ring PHY debug unavailable (phy=%u status=%d)\n",
				   __func__, phy_addr, status);
		return;
	}
	pr_info("VgaStateA =0x%x\n", ((val >> 4) & 0x1F));
	pr_info("VgaStateB =0x%x\n", ((val >> 9) & 0x1F));
	pr_info("VgaStateC =0x%x\n", ((val >> 14) & 0x1F));
	pr_info("VgaStateD =0x%x\n", ((val >> 19) & 0x1F));

	/* Pair A */
	if (esw_debug_read_mmd(phy_addr, 0x1E, 0x9B, &val))
		return;
	pr_info("XX0 0x1E,0x9B =0x%x\n", val);
	pr_info("AA0 lch_mse_mdcA =0x%x\r\n", (val >> 8) & 0xFF);

	/* Pair B */
	if (esw_debug_read_mmd(phy_addr, 0x1E, 0x9B, &val))
		return;
	pr_info("XX1 0x1E,0x9B =0x%x\n", val);
	pr_info("AA1 lch_mse_mdcB =0x%x\r\n", val & 0xFF);

	/* Pair C */
	if (esw_debug_read_mmd(phy_addr, 0x1E, 0x9C, &val))
		return;
	pr_info("XX2 0x1E,0x9C =0x%x\n", val);
	pr_info("AA2 lch_mse_mdcC =0x%x\r\n", (val >> 8) & 0xFF);

	/* Pair D */
	if (esw_debug_read_mmd(phy_addr, 0x1E, 0x9C, &val))
		return;
	pr_info("XX3 0x1E,0x9C =0x%x\n", val);
	pr_info("AA3 lch_mse_mdcD =0x%x\r\n", val & 0xFF);
}

irqreturn_t esw_interrupt(int irq, void *resv)
{
	unsigned long flags;
	int link_up;
	int status;
	int i;
	static unsigned int port_status[5] = {0, 0, 0, 0, 0};
	struct net_device *dev = dev_raether;
	struct END_DEVICE *ei_local = netdev_priv(dev);

	spin_lock_irqsave(&ei_local->page_lock, flags);
	/* disable irq mask and ack irq status */
	sys_reg_write(ETHDMASYS_ETH_SW_BASE + 0x4, 0xffffffff);
	sys_reg_write(ETHDMASYS_ETH_SW_BASE, 0x04000000);
	spin_unlock_irqrestore(&ei_local->page_lock, flags);
	for (i = 0; i < 5; i++) {
		status = ra_switch_mdio_read_link_status(i, 1U, 0x4U,
							 ra_switch_mdio_read, NULL,
							 &link_up);
		if (status != 0) {
			pr_err_ratelimited("%s: PHY link status read failed (port=%d status=%d)\n",
					   __func__, i, status);
			continue;
		}
		if (port_status[i] != (unsigned int)link_up) {
			port_status[i] = (unsigned int)link_up;
			if (link_up) {
				pr_info("ESW: Link Status Changed - Port%d Link Up\n", i);
			} else {
				pr_info("ESW: Link Status Changed - Port%d Link Down\n", i);
			}
			if (ei_local->architecture & LEOPARD_EPHY) {
				if (i == 0)
					esw_show_debug_log(i);/*port0 giga port*/
			}
		}
	}
	/* enable irq mask */
	sys_reg_write(ETHDMASYS_ETH_SW_BASE + 0x4, 0xfbffffff);
	return IRQ_HANDLED;
}

void sw_ioctl(struct ra_switch_ioctl_data *ioctl_data)
{
	unsigned int cmd;

	cmd = ioctl_data->cmd;
	switch (cmd) {
	case SW_IOCTL_DUMP_MIB:
		rtk_hal_dump_full_mib();
		break;

	case SW_IOCTL_SET_EGRESS_RATE:
		rtk_hal_set_egress_rate(ioctl_data);
		break;

	case SW_IOCTL_SET_INGRESS_RATE:
		rtk_hal_set_ingress_rate(ioctl_data);
		break;

	case SW_IOCTL_DUMP_VLAN:
		rtk_hal_dump_vlan();
		break;

	case SW_IOCTL_ADD_L2_ADDR:
		rtk_hal_add_table(ioctl_data);
		break;

	case SW_IOCTL_DEL_L2_ADDR:
		rtk_hal_del_table(ioctl_data);
		break;

	case SW_IOCTL_CLEAR_VLAN:
		rtk_hal_clear_vlan();
		break;

	case SW_IOCTL_SET_VLAN:
		rtk_hal_set_vlan(ioctl_data);
		break;

	case SW_IOCTL_DUMP_TABLE:
		rtk_hal_dump_table();
		break;

	case SW_IOCTL_CLEAR_TABLE:
		rtk_hal_clear_table();
		break;

	case SW_IOCTL_GET_PHY_STATUS:
		rtk_hal_get_phy_status(ioctl_data);
		break;

	case SW_IOCTL_SET_PORT_MIRROR:
		rtk_hal_set_port_mirror(ioctl_data);
		break;

	case SW_IOCTL_READ_REG:
		rtk_hal_read_reg(ioctl_data);
		break;

	case SW_IOCTL_WRITE_REG:
		rtk_hal_write_reg(ioctl_data);
		break;

	case SW_IOCTL_QOS_EN:
		rtk_hal_qos_en(ioctl_data);
		break;

	case SW_IOCTL_QOS_SET_TABLE2TYPE:
		rtk_hal_qos_set_table2type(ioctl_data);
		break;

	case SW_IOCTL_QOS_GET_TABLE2TYPE:
		rtk_hal_qos_get_table2type(ioctl_data);
		break;

	case SW_IOCTL_QOS_SET_PORT2TABLE:
		rtk_hal_qos_set_port2table(ioctl_data);
		break;

	case SW_IOCTL_QOS_GET_PORT2TABLE:
		rtk_hal_qos_get_port2table(ioctl_data);
		break;

	case SW_IOCTL_QOS_SET_PORT2PRI:
		rtk_hal_qos_set_port2pri(ioctl_data);
		break;

	case SW_IOCTL_QOS_GET_PORT2PRI:
		rtk_hal_qos_get_port2pri(ioctl_data);
		break;

	case SW_IOCTL_QOS_SET_DSCP2PRI:
		rtk_hal_qos_set_dscp2pri(ioctl_data);
		break;

	case SW_IOCTL_QOS_GET_DSCP2PRI:
		rtk_hal_qos_get_dscp2pri(ioctl_data);
		break;

	case SW_IOCTL_QOS_SET_PRI2QUEUE:
		rtk_hal_qos_set_pri2queue(ioctl_data);
		break;

	case SW_IOCTL_QOS_GET_PRI2QUEUE:
		rtk_hal_qos_get_pri2queue(ioctl_data);
		break;

	case SW_IOCTL_QOS_SET_QUEUE_WEIGHT:
		rtk_hal_qos_set_queue_weight(ioctl_data);
		break;

	case SW_IOCTL_QOS_GET_QUEUE_WEIGHT:
		rtk_hal_qos_get_queue_weight(ioctl_data);
		break;

	case SW_IOCTL_ENABLE_IGMPSNOOP:
		rtk_hal_enable_igmpsnoop(ioctl_data);
		break;

	case SW_IOCTL_DISABLE_IGMPSNOOP:
		rtk_hal_disable_igmpsnoop();
		break;

	case SW_IOCTL_SET_PHY_TEST_MODE:
		rtk_hal_set_phy_test_mode(ioctl_data);
		break;

	case SW_IOCTL_GET_PHY_REG:
		rtk_hal_get_phy_reg(ioctl_data);
		break;

	case SW_IOCTL_SET_PHY_REG:
		rtk_hal_set_phy_reg(ioctl_data);
		break;

	case SW_IOCTL_VLAN_TAG:
		rtk_hal_vlan_tag(ioctl_data);
		break;

	case SW_IOCTL_SET_VLAN_MODE:
		rtk_hal_vlan_mode(ioctl_data);
		break;

	case SW_IOCTL_SET_PORT_TRUNK:
		rtk_hal_set_port_trunk(ioctl_data);

	default:
		break;
	}
}

int ephy_ioctl(struct net_device *dev, struct ifreq *ifr,
	       struct ephy_ioctl_data *ioctl_data)
{
	int ret = 0;
	unsigned int cmd;
	u8 cnt = 0;
	u8 port_num = 0;

	cmd = ioctl_data->cmd;
	pr_info("%s : cmd =%x\n", __func__, cmd);
	switch (cmd) {
	case RAETH_VBG_IEXT_CALIBRATION:
		for (port_num = 0; port_num < 5; port_num++) {
			/* Reset both retry and completion state for every port. */
			cnt = 0;
			fe_cal_vbg_flag = 0;
			while ((fe_cal_vbg_flag == 0) && (cnt < 0x3)) {
				fe_cal_vbg(port_num, 1);
				cnt++;
				if (fe_cal_vbg_flag == 0)
					pr_info(" VBG wait! (%d)\n", cnt);
			}
			if (!fe_cal_vbg_flag) {
				ret = -EIO;
				break;
			}
		}
		break;

	case RAETH_TXG_R50_CALIBRATION:
		for (port_num = 0; port_num < 5; port_num++) {
			cnt = 0;
			fe_cal_r50_flag = 0;
			while ((fe_cal_r50_flag == 0) && (cnt < 0x3)) {
				fe_cal_r50(port_num, 1);
				cnt++;
				if (fe_cal_r50_flag == 0)
					pr_info(" FE R50 wait! (%d)\n", cnt);
			}
			if (!fe_cal_r50_flag) {
				ret = -EIO;
				break;
			}
		}
		break;

	case RAETH_TXG_OFFSET_CALIBRATION:
		for (port_num = 0; port_num < 5; port_num++) {
			cnt = 0;
			fe_cal_tx_offset_flag = 0;
			while ((fe_cal_tx_offset_flag == 0) && (cnt < 0x3)) {
				fe_cal_tx_offset(port_num, 100);
				cnt++;
				if (fe_cal_tx_offset_flag == 0)
					pr_info("FeTxOffsetAnaCal wait!(%d)\n",
						cnt);
			}
			if (!fe_cal_tx_offset_flag) {
				ret = -EIO;
				break;
			}

			cnt = 0;
			fe_cal_tx_offset_flag_mdix = 0;
			while ((fe_cal_tx_offset_flag_mdix == 0) && (cnt < 0x3)) {
				fe_cal_tx_offset_mdix(port_num, 100);
				cnt++;
				if (fe_cal_tx_offset_flag_mdix == 0)
					pr_info("FeTxOffsetAnaCal mdix wait!(%d)\n",
						cnt);
			}
			if (!fe_cal_tx_offset_flag_mdix) {
				ret = -EIO;
				break;
			}
		}
		break;

	case RAETH_TXG_AMP_CALIBRATION:
		for (port_num = 0; port_num < 5; port_num++) {
			cnt = 0;
			fe_cal_flag = 0;
			while ((fe_cal_flag == 0) && (cnt < 0x3)) {
				fe_cal_tx_amp(port_num, 300);
				cnt++;
				if (fe_cal_flag == 0)
					pr_info("FETxAmpAnaCal wait!(%d)\n",
						cnt);
			}
			if (!fe_cal_flag) {
				ret = -EIO;
				break;
			}

			cnt = 0;
			fe_cal_flag_mdix = 0;
			while ((fe_cal_flag_mdix == 0) && (cnt < 0x3)) {
				fe_cal_tx_amp_mdix(port_num, 300);
				cnt++;
				if (fe_cal_flag_mdix == 0)
					pr_info("FETxAmpAnaCal mdix wait!(%d)\n",
						cnt);
			}
			if (!fe_cal_flag_mdix) {
				ret = -EIO;
				break;
			}
		}
		break;

	case GE_TXG_R50_CALIBRATION:
		cnt = 0;
		ge_cal_r50_flag = 0;
		while ((ge_cal_r50_flag == 0) && (cnt < 0x3)) {
			ge_cal_r50(0, 20);
			cnt++;
			if (ge_cal_r50_flag == 0)
				pr_info(" GE R50 wait! (%d)\n", cnt);
		}
		if (!ge_cal_r50_flag)
			ret = -EIO;
		break;

	case GE_TXG_OFFSET_CALIBRATION:
		cnt = 0;
		ge_cal_tx_offset_flag = 0;
		while ((ge_cal_tx_offset_flag == 0) && (cnt < 0x3)) {
			ge_cal_tx_offset(0, 20);
			cnt++;
			if (ge_cal_tx_offset_flag == 0)
				pr_info("GeTxOffsetAnaCal wait!(%d)\n", cnt);
		}
		if (!ge_cal_tx_offset_flag)
			ret = -EIO;
		break;

	case GE_TXG_AMP_CALIBRATION:
		cnt = 0;
		ge_cal_flag = 0;
		while ((ge_cal_flag == 0) && (cnt < 0x3)) {
			ge_cal_tx_amp(0, 20);
			cnt++;
			if (ge_cal_flag == 0)
				pr_info("GETxAmpAnaCal wait!(%d)\n", cnt);
		}
		if (!ge_cal_flag)
			ret = -EIO;
		break;
	default:
		ret = 1;
		break;
	}

	return ret;
}

static const struct of_device_id mediatek_gsw_match[] = {
	{.compatible = "mediatek,mt7623-gsw"},
	{.compatible = "mediatek,mt7621-gsw"},
	{},
};

MODULE_DEVICE_TABLE(of, mediatek_gsw_match);

static int mtk_gsw_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct device_node *pctl;
	struct mtk_gsw *gsw;
	int err;
	const char *pm;

	gsw = devm_kzalloc(&pdev->dev, sizeof(struct mtk_gsw), GFP_KERNEL);
	if (!gsw)
		return -ENOMEM;

	gsw->dev = &pdev->dev;
	gsw->trgmii_force = 2000;
	gsw->irq = irq_of_parse_and_map(np, 0);
	if (gsw->irq < 0)
		return -EINVAL;

	err = of_property_read_string(pdev->dev.of_node, "mcm", &pm);
	if (!err && !strcasecmp(pm, "enable")) {
		gsw->mcm = true;
		pr_info("== MT7530 MCM ==\n");
	}

	gsw->ethsys = syscon_regmap_lookup_by_phandle(np, "mediatek,ethsys");
	if (IS_ERR(gsw->ethsys)) {
		pr_err("fail at %s %d\n", __func__, __LINE__);
		return PTR_ERR(gsw->ethsys);
	}

	if (!gsw->mcm) {
		gsw->reset_pin = of_get_named_gpio(np, "mediatek,reset-pin", 0);
		if (gsw->reset_pin < 0) {
			pr_err("fail at %s %d\n", __func__, __LINE__);
			return -1;
		}
		pr_debug("reset_pin_port= %d\n", gsw->reset_pin);

		pctl = of_parse_phandle(np, "mediatek,pctl-regmap", 0);
		if (IS_ERR(pctl)) {
			pr_err("fail at %s %d\n", __func__, __LINE__);
			return PTR_ERR(pctl);
		}

		gsw->pctl = syscon_node_to_regmap(pctl);
		if (IS_ERR(pctl)) {
			pr_err("fail at %s %d\n", __func__, __LINE__);
			return PTR_ERR(pctl);
		}

		gsw->pins = pinctrl_get(&pdev->dev);
		if (gsw->pins) {
			gsw->ps_reset =
			    pinctrl_lookup_state(gsw->pins, "reset");

			if (IS_ERR(gsw->ps_reset)) {
				dev_err(&pdev->dev,
					"failed to lookup the gsw_reset state\n");
				return PTR_ERR(gsw->ps_reset);
			}
		} else {
			dev_err(&pdev->dev, "gsw get pinctrl fail\n");
			return PTR_ERR(gsw->pins);
		}
	}

	gsw->supply = devm_regulator_get(&pdev->dev, "mt7530");
	if (IS_ERR(gsw->supply)) {
		pr_info("fail at %s %d\n", __func__, __LINE__);
		return PTR_ERR(gsw->supply);
	}

	if (gsw->mcm) {
		gsw->b3v = devm_regulator_get(&pdev->dev, "b3v");
		if (IS_ERR(gsw->b3v))
			return PTR_ERR(gsw->b3v);
	}

	gsw->wllll = of_property_read_bool(np, "mediatek,wllll");

	platform_set_drvdata(pdev, gsw);

	return 0;
}

static int mtk_gsw_remove(struct platform_device *pdev)
{
	platform_set_drvdata(pdev, NULL);

	return 0;
}

static struct platform_driver gsw_driver = {
	.probe = mtk_gsw_probe,
	.remove = mtk_gsw_remove,
	.driver = {
		   .name = "mtk-gsw",
		   .owner = THIS_MODULE,
		   .of_match_table = mediatek_gsw_match,
		   },
};

module_platform_driver(gsw_driver);
