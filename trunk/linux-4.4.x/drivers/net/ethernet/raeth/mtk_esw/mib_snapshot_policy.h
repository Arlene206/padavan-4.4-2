#ifndef _MTK_ESW_MIB_SNAPSHOT_POLICY_H
#define _MTK_ESW_MIB_SNAPSHOT_POLICY_H

#ifdef __KERNEL__
#include <linux/errno.h>
#else
#include <errno.h>
#endif

/*
 * Read a complete MT7530 per-port MIB snapshot into a temporary object.
 * The caller's snapshot is published only when every register read succeeds,
 * so a failed MDIO transaction can never expose a mixed old/new counter set.
 */
struct mtk_esw_mib_snapshot {
	unsigned long long tx_good_octets;
	unsigned int tx_unicast_frames;
	unsigned int tx_multicast_frames;
	unsigned int tx_broadcast_frames;
	unsigned int tx_drop_frames;
	unsigned int tx_pause_frames;
	unsigned int tx_collision;
	unsigned int tx_crc_error;
	unsigned long long rx_good_octets;
	unsigned int rx_unicast_frames;
	unsigned int rx_multicast_frames;
	unsigned int rx_broadcast_frames;
	unsigned int rx_drop_frames;
	unsigned int rx_pause_frames;
	unsigned int rx_filter_frames;
	unsigned int rx_crc_error;
	unsigned int rx_alignment_error;
};

typedef int (*mtk_esw_mib_read_fn)(void *context, unsigned int reg,
				   unsigned int *value);

static inline int
mtk_esw_mib_read_octets(unsigned int base, unsigned int low_offset,
			unsigned int high_offset, mtk_esw_mib_read_fn read,
			void *context, unsigned long long *octets)
{
	unsigned int low, high;
	int status;

	status = read(context, base + low_offset, &low);
	if (status != 0)
		return status;
	status = read(context, base + high_offset, &high);
	if (status != 0)
		return status;

	*octets = ((unsigned long long)high << 32) | low;
	return 0;
}

static inline int
mtk_esw_mib_snapshot_read(unsigned int port_base, mtk_esw_mib_read_fn read,
			  void *context, struct mtk_esw_mib_snapshot *snapshot)
{
	static const unsigned int counter_offsets[] = {
		0x4000U, 0x4004U, 0x4008U, 0x400cU, 0x4010U,
		0x4014U, 0x402cU,
		0x4060U, 0x4064U, 0x4068U, 0x406cU, 0x4070U,
		0x4074U, 0x4078U, 0x408cU
	};
	struct mtk_esw_mib_snapshot result = { 0 };
	unsigned int counters[sizeof(counter_offsets) / sizeof(counter_offsets[0])];
	unsigned int i;
	int status;

	if (!read || !snapshot || port_base > 0xffffffffU - 0x40acU)
		return -EINVAL;

	status = mtk_esw_mib_read_octets(port_base, 0x40a8U, 0x40acU,
					 read, context, &result.rx_good_octets);
	if (status != 0)
		return status;
	status = mtk_esw_mib_read_octets(port_base, 0x4048U, 0x404cU,
					 read, context, &result.tx_good_octets);
	if (status != 0)
		return status;

	for (i = 0U; i < sizeof(counter_offsets) / sizeof(counter_offsets[0]); ++i) {
		status = read(context, port_base + counter_offsets[i], &counters[i]);
		if (status != 0)
			return status;
	}

	result.tx_drop_frames = counters[0];
	result.tx_crc_error = counters[1];
	result.tx_unicast_frames = counters[2];
	result.tx_multicast_frames = counters[3];
	result.tx_broadcast_frames = counters[4];
	result.tx_collision = counters[5];
	result.tx_pause_frames = counters[6];
	result.rx_drop_frames = counters[7];
	result.rx_filter_frames = counters[8];
	result.rx_unicast_frames = counters[9];
	result.rx_multicast_frames = counters[10];
	result.rx_broadcast_frames = counters[11];
	result.rx_alignment_error = counters[12];
	result.rx_crc_error = counters[13];
	result.rx_pause_frames = counters[14];

	*snapshot = result;
	return 0;
}

#endif
