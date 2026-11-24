/* Copyright  2016 MediaTek Inc.
 * Author: Nelson Chang <nelson.chang@mediatek.com>
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
#include "ra_dbg_proc.h"
#include "ra_dbg_hwioc_policy.h"

#define MCSI_A_PMU_CTL		0x10390100	/* PMU CTRL */
#define MCSI_A_PMU_CYC_CNT	0x10399004	/* Cycle counter */
#define MCSI_A_PMU_CYC_CTL	0x10399008	/* Cycle counter CTRL */

#define MCSI_A_PMU_EVN_SEL0	0x1039A000	/* EVENT SELECT 0 */
#define MCSI_A_PMU_EVN_CNT0	0x1039A004	/* Event Count 0 */
#define MCSI_A_PMU_EVN_CTL0	0x1039A008	/* Event Count control 0 */

#define MCSI_A_PMU_EVN_SEL1	0x1039B000	/* EVENT SELECT 1 */
#define MCSI_A_PMU_EVN_CNT1	0x1039B004	/* Event Count 1 */
#define MCSI_A_PMU_EVN_CTL1	0x1039B008	/* Event Count control 1 */

#define MCSI_A_PMU_EVN_SEL2	0x1039C000	/* EVENT SELECT 2 */
#define MCSI_A_PMU_EVN_CNT2	0x1039C004	/* Event Count 2 */
#define MCSI_A_PMU_EVN_CTL2	0x1039C008	/* Event Count control 2 */

#define MCSI_A_PMU_EVN_SEL3	0x1039D000	/* EVENT SELECT 3 */
#define MCSI_A_PMU_EVN_CNT3	0x1039D004	/* Event Count 3 */
#define MCSI_A_PMU_EVN_CTL3	0x1039D008	/* Event Count control 3 */

#define PMU_EVN_SEL_S0 (0x0 << 5)
#define PMU_EVN_SEL_S1 (0x1 << 5)
#define PMU_EVN_SEL_S2 (0x2 << 5)
#define PMU_EVN_SEL_S3 (0x3 << 5)
#define PMU_EVN_SEL_S4 (0x4 << 5)
#define PMU_EVN_SEL_S5 (0x5 << 5)
#define PMU_EVN_SEL_M0 (0x6 << 5)
#define PMU_EVN_SEL_M1 (0x7 << 5)
#define PMU_EVN_SEL_M2 (0x8 << 5)

#define PMU_EVN_READ_ANY    0x0
#define PMU_EVN_READ_SNOOP  0x3
#define PMU_EVN_READ_HIT    0xA
#define PMU_EVN_WRITE_ANY   0xC
#define PMU_EVN_WU_SNOOP    0x10
#define PMU_EVN_WLU_SNOOP   0x11

#define PMU_0_SEL   (PMU_EVN_SEL_S2 | PMU_EVN_READ_SNOOP)
#define PMU_1_SEL   (PMU_EVN_SEL_S2 | PMU_EVN_READ_HIT)
#define PMU_2_SEL   (PMU_EVN_SEL_S4 | PMU_EVN_READ_SNOOP)
#define PMU_3_SEL   (PMU_EVN_SEL_S4 | PMU_EVN_READ_HIT)

#define MCSI_A_PMU_CTL_BASE	MCSI_A_PMU_CTL
#define MCSI_A_PMU_CNT0_BASE	MCSI_A_PMU_EVN_SEL0
#define MCSI_A_PMU_CNT1_BASE	MCSI_A_PMU_EVN_SEL1
#define MCSI_A_PMU_CNT2_BASE	MCSI_A_PMU_EVN_SEL2
#define MCSI_A_PMU_CNT3_BASE	MCSI_A_PMU_EVN_SEL3

typedef int (*IOC_SET_FUNC) (int par1, int par2, int par3);
static struct proc_dir_entry *proc_hw_io_coherent;

unsigned int reg_pmu_evn_phys[] = {
	MCSI_A_PMU_CNT0_BASE,
	MCSI_A_PMU_CNT1_BASE,
	MCSI_A_PMU_CNT2_BASE,
	MCSI_A_PMU_CNT3_BASE,
};

int ioc_pmu_cnt_config(int pmu_no, int interface, int event)
{
	void __iomem *reg_pmu_cnt;
	unsigned int pmu_sel;

	if (!raeth_hwioc_pmu_counter_args_valid(pmu_no, interface, event))
		return -EINVAL;

	reg_pmu_cnt = ioremap(reg_pmu_evn_phys[pmu_no], 0x10);
	if (!reg_pmu_cnt)
		return -ENOMEM;

	/* Event Select Register: interface in bits [7:5], event in [4:0]. */
	pmu_sel = (interface << 5) | event;
	sys_reg_write(reg_pmu_cnt, pmu_sel);

	/* Enable this counter. */
	sys_reg_write(reg_pmu_cnt + 0x8, 0x1);

	iounmap(reg_pmu_cnt);
	return 0;
}

int ioc_pmu_ctl_config(int enable, int ignore1, int ignore2)
{
	void __iomem *reg_pmu_ctl;

	if (!raeth_hwioc_pmu_enable_valid(enable))
		return -EINVAL;

	reg_pmu_ctl = ioremap(MCSI_A_PMU_CTL_BASE, 0x10);
	if (!reg_pmu_ctl)
		return -ENOMEM;

	if (enable) {
		sys_reg_write(reg_pmu_ctl, BIT(1));
		sys_reg_write(reg_pmu_ctl, BIT(0));
	} else {
		sys_reg_write(reg_pmu_ctl, 0x0);
	}

	iounmap(reg_pmu_ctl);
	return 0;
}

int ioc_set_usage(int ignore1, int ignore2, int ignore3)
{
	pr_info("<Usage> echo \"[OP Mode] [Arg1] [Arg2 | Arg3]\" > /proc/%s\n\r",
		PROCREG_HW_IO_COHERENT);
	pr_info("\tControl PMU counter: echo \"1 [Enable]\" > /proc/%s\n\r",
		PROCREG_HW_IO_COHERENT);
	pr_info("\t\t[Enable]:\n\r\t\t\t1: enable\n\r\t\t\t0: disable\n\r");
	pr_info("\tConfigure PMU counter: echo \"2 [CNT No.] [IF] [EVN]\" > /proc/%s\n\r",
		PROCREG_HW_IO_COHERENT);
	pr_info("\t\t[CNT No.]: 0/1/2/3 PMU Counter\n\r");
	pr_info("\t\t[IF]:\n\r");
	pr_info("\t\t\t0: PMU_EVN_SEL_S0\n\r");
	pr_info("\t\t\t1: PMU_EVN_SEL_S1\n\r");
	pr_info("\t\t\t2: PMU_EVN_SEL_S2\n\r");
	pr_info("\t\t\t3: PMU_EVN_SEL_S3\n\r");
	pr_info("\t\t\t4: PMU_EVN_SEL_S4\n\r");
	pr_info("\t\t\t5: PMU_EVN_SEL_S5\n\r");
	pr_info("\t\t\t6: PMU_EVN_SEL_M0\n\r");
	pr_info("\t\t\t7: PMU_EVN_SEL_M1\n\r");
	pr_info("\t\t\t8: PMU_EVN_SEL_M2\n\r");
	pr_info("\t\t[EVN]:\n\r");
	pr_info("\t\t\t0: PMU_EVN_READ_ANY\n\r");
	pr_info("\t\t\t3: PMU_EVN_READ_SNOOP\n\r");
	pr_info("\t\t\tA: PMU_EVN_READ_HIT\n\r");
	pr_info("\t\t\tC: PMU_EVN_WRITE_ANY\n\r");
	pr_info("\t\t\t10: PMU_EVN_WU_SNOOP\n\r");
	pr_info("\t\t\t11: PMU_EVN_WLU_SNOOP\n\r");

	return 0;
}

static const IOC_SET_FUNC iocoherent_set_func[] = {
	[0] = ioc_set_usage,
	[1] = ioc_pmu_ctl_config,
	[2] = ioc_pmu_cnt_config,
};

static int ioc_next_hex_arg(char **cursor, long *value)
{
	char *token;
	int ret;

	while ((token = strsep(cursor, " \t\r\n")) != NULL) {
		if (!*token)
			continue;
		ret = kstrtol(token, 16, value);
		if (ret != 0)
			return ret;
		return 0;
	}

	return -EINVAL;
}

static int ioc_no_more_hex_args(char **cursor)
{
	char *token;

	while ((token = strsep(cursor, " \t\r\n")) != NULL) {
		if (*token)
			return -EINVAL;
	}
	return 0;
}

ssize_t ioc_pmu_write(struct file *file, const char __user *buffer,
		      size_t count, loff_t *data)
{
	char buf[32];
	char *cursor;
	long args[4] = { 0, 0, 0, 0 };
	int ret;
	size_t len = count;

	if (count == 0)
		return -EINVAL;
	if (count >= sizeof(buf))
		return -E2BIG;
	if (copy_from_user(buf, buffer, count))
		return -EFAULT;

	buf[count] = '\0';
	pr_info("write parameter data = %s\n\r", buf);
	cursor = buf;

	ret = ioc_next_hex_arg(&cursor, &args[0]);
	if (ret != 0)
		return ret;
	if (!raeth_hwioc_pmu_operation_valid(args[0]) ||
	    (unsigned long)args[0] >= ARRAY_SIZE(iocoherent_set_func))
		return -EINVAL;

	switch (args[0]) {
	case 0:
		break;
	case 1:
		ret = ioc_next_hex_arg(&cursor, &args[1]);
		if (ret != 0)
			return ret;
		if (!raeth_hwioc_pmu_enable_valid(args[1]))
			return -EINVAL;
		break;
	case 2:
		ret = ioc_next_hex_arg(&cursor, &args[1]);
		if (ret != 0)
			return ret;
		ret = ioc_next_hex_arg(&cursor, &args[2]);
		if (ret != 0)
			return ret;
		ret = ioc_next_hex_arg(&cursor, &args[3]);
		if (ret != 0)
			return ret;
		if (!raeth_hwioc_pmu_counter_args_valid(args[1], args[2], args[3]))
			return -EINVAL;
		break;
	default:
		return -EINVAL;
	}

	ret = ioc_no_more_hex_args(&cursor);
	if (ret != 0)
		return ret;

	ret = iocoherent_set_func[args[0]]((int)args[1],
					    (int)args[2], (int)args[3]);
	if (ret != 0)
		return ret;

	return len;
}

int ioc_pmu_read(struct seq_file *seq, void *v)
{
	void __iomem *reg_virt_0, *reg_virt_1, *reg_virt_2, *reg_virt_3;

	/* Do not emit a partial PMU snapshot if any counter window is missing. */
	reg_virt_0 = ioremap(MCSI_A_PMU_EVN_SEL0, 0x10);
	if (!reg_virt_0)
		return -ENOMEM;
	reg_virt_1 = ioremap(MCSI_A_PMU_EVN_SEL1, 0x10);
	if (!reg_virt_1) {
		iounmap(reg_virt_0);
		return -ENOMEM;
	}
	reg_virt_2 = ioremap(MCSI_A_PMU_EVN_SEL2, 0x10);
	if (!reg_virt_2) {
		iounmap(reg_virt_1);
		iounmap(reg_virt_0);
		return -ENOMEM;
	}
	reg_virt_3 = ioremap(MCSI_A_PMU_EVN_SEL3, 0x10);
	if (!reg_virt_3) {
		iounmap(reg_virt_2);
		iounmap(reg_virt_1);
		iounmap(reg_virt_0);
		return -ENOMEM;
	}

	seq_printf(seq, "MCSI_A_PMU_EVN_SEL0 = 0x%x\n",
		   sys_reg_read(reg_virt_0));
	seq_printf(seq, "MCSI_A_PMU_EVN_CNT0 = 0x%x\n",
		   sys_reg_read(reg_virt_0 + 0x4));
	seq_printf(seq, "MCSI_A_PMU_EVN_CTL0 = 0x%x\n",
		   sys_reg_read(reg_virt_0 + 0x8));
	seq_printf(seq, "MCSI_A_PMU_EVN_SEL1 = 0x%x\n",
		   sys_reg_read(reg_virt_1));
	seq_printf(seq, "MCSI_A_PMU_EVN_CNT1 = 0x%x\n",
		   sys_reg_read(reg_virt_1 + 0x4));
	seq_printf(seq, "MCSI_A_PMU_EVN_CTL1 = 0x%x\n",
		   sys_reg_read(reg_virt_1 + 0x8));

	seq_printf(seq, "MCSI_A_PMU_EVN_SEL2 = 0x%x\n",
		   sys_reg_read(reg_virt_2));
	seq_printf(seq, "MCSI_A_PMU_EVN_CNT2 = 0x%x\n",
		   sys_reg_read(reg_virt_2 + 0x4));
	seq_printf(seq, "MCSI_A_PMU_EVN_CTL2 = 0x%x\n",
		   sys_reg_read(reg_virt_2 + 0x8));

	seq_printf(seq, "MCSI_A_PMU_EVN_SEL3 = 0x%x\n",
		   sys_reg_read(reg_virt_3));
	seq_printf(seq, "MCSI_A_PMU_EVN_CNT3 = 0x%x\n",
		   sys_reg_read(reg_virt_3 + 0x4));
	seq_printf(seq, "MCSI_A_PMU_EVN_CTL3 = 0x%x\n",
		   sys_reg_read(reg_virt_3 + 0x8));

	iounmap(reg_virt_3);
	iounmap(reg_virt_2);
	iounmap(reg_virt_1);
	iounmap(reg_virt_0);
	return 0;
}

static int ioc_pmu_open(struct inode *inode, struct file *file)
{
	return single_open(file, ioc_pmu_read, NULL);
}

static const struct file_operations ioc_pmu_fops = {
	.owner = THIS_MODULE,
	.open = ioc_pmu_open,
	.read = seq_read,
	.llseek = seq_lseek,
	.write = ioc_pmu_write,
	.release = single_release
};

void hwioc_debug_proc_init(struct proc_dir_entry *proc_reg_dir)
{
	proc_hw_io_coherent =
	     proc_create(PROCREG_HW_IO_COHERENT, 0, proc_reg_dir,
			 &ioc_pmu_fops);
	if (!proc_hw_io_coherent)
		pr_err("FAIL to create %s PROC!\n", PROCREG_HW_IO_COHERENT);
}
EXPORT_SYMBOL(hwioc_debug_proc_init);

void hwioc_debug_proc_exit(struct proc_dir_entry *proc_reg_dir)
{
	if (proc_hw_io_coherent)
		remove_proc_entry(PROCREG_HW_IO_COHERENT, proc_reg_dir);
}
EXPORT_SYMBOL(hwioc_debug_proc_exit);
