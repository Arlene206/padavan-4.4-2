#ifndef RAETH_DESC_POLICY_H
#define RAETH_DESC_POLICY_H

#ifdef __KERNEL__
#include <linux/errno.h>
#include <linux/types.h>
#else
#include <errno.h>
#include <stdint.h>
typedef uint32_t u32;
#endif

static inline int raeth_desc_policy_calculate(
	u32 gmac1_txq_num, u32 gmac1_txq_txd_num,
	u32 gmac2_txq_num, u32 gmac2_txq_txd_num,
	u32 num_rx_desc, int gmac2_enabled,
	u32 *gmac1_txd_num, u32 *gmac2_txd_num,
	u32 *num_tx_desc, u32 *total_txq_num)
{
	u32 max_value = ~(u32)0;
	u32 txd1;
	u32 txd2;
	u32 tx_descs;
	u32 tx_queues;

	if (!gmac1_txd_num || !gmac2_txd_num ||
	    !num_tx_desc || !total_txq_num)
		return -EINVAL;
	if (gmac2_enabled != 0 && gmac2_enabled != 1)
		return -EINVAL;
	if (!gmac1_txq_num || !gmac1_txq_txd_num || !num_rx_desc)
		return -EINVAL;
	if ((!gmac2_txq_num) != (!gmac2_txq_txd_num))
		return -EINVAL;
	if (gmac2_enabled && (!gmac2_txq_num || !gmac2_txq_txd_num))
		return -EINVAL;

	if (gmac1_txq_txd_num > max_value / gmac1_txq_num)
		return -EOVERFLOW;
	txd1 = gmac1_txq_num * gmac1_txq_txd_num;
	if (gmac2_txq_num &&
	    gmac2_txq_txd_num > max_value / gmac2_txq_num)
		return -EOVERFLOW;
	txd2 = gmac2_txq_num * gmac2_txq_txd_num;
	if (txd1 > max_value - txd2)
		return -EOVERFLOW;
	tx_descs = txd1 + txd2;
	if (gmac1_txq_num > max_value - gmac2_txq_num)
		return -EOVERFLOW;
	tx_queues = gmac1_txq_num + gmac2_txq_num;
	if (!tx_descs || !tx_queues)
		return -EINVAL;

	*gmac1_txd_num = txd1;
	*gmac2_txd_num = txd2;
	*num_tx_desc = tx_descs;
	*total_txq_num = tx_queues;
	return 0;
}

#endif
