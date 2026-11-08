/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/ioctl.h>

#include "httpd.h"
#include "httpd_switch_ioctl_policy.h"

#include <rtl8367_ioctl.h>

////////////////////////////////////////////////////////////////////////////////
// MIB COUNTERS
////////////////////////////////////////////////////////////////////////////////

typedef struct mib_counters_s
{
    uint64_t ifInOctets;
    uint32_t dot3StatsFCSErrors;
    uint32_t dot3StatsSymbolErrors;
    uint32_t dot3InPauseFrames;
    uint32_t dot3ControlInUnknownOpcodes;
    uint32_t etherStatsFragments;
    uint32_t etherStatsJabbers;
    uint32_t ifInUcastPkts;
    uint32_t etherStatsDropEvents;
    uint64_t etherStatsOctets;
    uint32_t etherStatsUndersizePkts;
    uint32_t etherStatsOversizePkts;
    uint32_t etherStatsPkts64Octets;
    uint32_t etherStatsPkts65to127Octets;
    uint32_t etherStatsPkts128to255Octets;
    uint32_t etherStatsPkts256to511Octets;
    uint32_t etherStatsPkts512to1023Octets;
    uint32_t etherStatsPkts1024toMaxOctets;
    uint32_t etherStatsMcastPkts;
    uint32_t etherStatsBcastPkts;
    uint64_t ifOutOctets;
    uint32_t dot3StatsSingleCollisionFrames;
    uint32_t dot3StatsMultipleCollisionFrames;
    uint32_t dot3StatsDeferredTransmissions;
    uint32_t dot3StatsLateCollisions;
    uint32_t etherStatsCollisions;
    uint32_t dot3StatsExcessiveCollisions;
    uint32_t dot3OutPauseFrames;
    uint32_t dot1dBasePortDelayExceededDiscards;
    uint32_t dot1dTpPortInDiscards;
    uint32_t ifOutUcastPkts;
    uint32_t ifOutMulticastPkts;
    uint32_t ifOutBrocastPkts;
    uint32_t outOampduPkts;
    uint32_t inOampduPkts;
    uint32_t pktgenPkts;
} mib_counters_t;

////////////////////////////////////////////////////////////////////////////////
// IOCTL
////////////////////////////////////////////////////////////////////////////////

static int rtl8367_ioctl(unsigned int cmd, unsigned int par, unsigned int *value)
{
	int fd, ioctl_result, ioctl_errno = 0;
	int close_result, close_errno = 0;
	int operation_status, close_status;

	fd = open(RTL8367_DEVPATH, O_RDONLY);
	if (fd < 0) {
		int open_errno = errno;
		perror(RTL8367_DEVPATH);
		return httpd_switch_normalize_result(fd, open_errno);
	}

	cmd &= ((1L << RTL8367_IOCTL_CMD_LENGTH_BITS) - 1);
	cmd |= (par << RTL8367_IOCTL_CMD_LENGTH_BITS);

	ioctl_result = ioctl(fd, cmd, value);
	if (ioctl_result < 0) {
		ioctl_errno = errno;
		perror("ioctl");
	}
	operation_status = httpd_switch_normalize_result(ioctl_result, ioctl_errno);

	close_result = close(fd);
	if (close_result < 0) {
		close_errno = errno;
		perror("close");
	}
	close_status = httpd_switch_normalize_result(close_result, close_errno);

	return httpd_switch_choose_result(operation_status, close_status);
}

int get_eth_port_bytes(int get_eth_port_bytes(int port_id_uapi, uint64_t *rx, uint64_t *tx)
{
	port_bytes_t pb;
	int ioctl_result;

	if (!rx || !tx)
		return -EINVAL;

	memset(&pb, 0, sizeof(pb));
	ioctl_result = rtl8367_ioctl(RTL8367_IOCTL_STATUS_BYTES_PORT, port_id_uapi, (unsigned int *)&pb);
	return httpd_switch_publish_port_bytes(ioctl_result, pb.RX, pb.TX, rx, tx);
}

int fill_eth_port_status(int fill_eth_port_status(int port_id_uapi, char linkstate[40])
{
	unsigned int link_value = 0;
	int has_link = 0;

	if (rtl8367_ioctl(RTL8367_IOCTL_STATUS_SPEED_PORT, port_id_uapi, &link_value) != 0) {
		strcpy(linkstate, "I/O Error");
		return 0;
	}

	has_link = (link_value >> 16) & 0x01;

	if (has_link) {
		int lspeed;
		const char *text_fc = "";
		const char *text_dup = "Half Duplex";
		const char *text_eee = "";
		
		switch (link_value & 0x03)
		{
		case 2:
			lspeed = 1000;
			break;
		case 1:
			lspeed = 100;
			break;
		default:
			lspeed = 10;
			break;
		}
		
		if ((link_value >> 8) & 0x01) {
			unsigned int link_fc = (link_value >> 9) & 0x03;
			if (link_fc == 0x03)
				text_fc = "TX/RX";
			else if (link_fc == 0x01)
				text_fc = "TX Asy";
			else if (link_fc == 0x02)
				text_fc = "RX Asy";
			else
				text_fc = "OFF";
			text_dup = "Full Duplex, FC ";
		}
		
		if ((link_value >> 11) & 0x03)
			text_eee = ", EEE";
		
		/* 1000 Mbps, Full Duplex, FC TX/RX */
		sprintf(linkstate, "%d Mbps, %s%s%s", lspeed, text_dup, text_fc, text_eee);
	} else
		strcpy(linkstate, "No Link");

	return has_link;
}

int fill_eth_status(int port_id_uapi, webs_t wp)
{
	int ret = 0;
	char etherlink[40] = {0};
	mib_counters_t mibc;

	fill_eth_port_status(port_id_uapi, etherlink);

	ret += websWrite(wp, "Port Link			: %s\n", etherlink);

	memset(&mibc, 0, sizeof(mib_counters_t));
	if (rtl8367_ioctl(RTL8367_IOCTL_STATUS_MIB_PORT, port_id_uapi, (unsigned int *)&mibc) == 0) {
		ret += websWrite(wp, "\nMIB Counters\n");
		ret += websWrite(wp, "----------------------------------------\n");
		ret += websWrite(wp, "ifInOctets			: %llu\n", mibc.ifInOctets);
		ret += websWrite(wp, "ifOutOctets			: %llu\n", mibc.ifOutOctets);
		ret += websWrite(wp, "etherStatsMcastPkts		: %u\n", mibc.etherStatsMcastPkts);
		ret += websWrite(wp, "etherStatsBcastPkts		: %u\n", mibc.etherStatsBcastPkts);
		ret += websWrite(wp, "etherStatsFragments		: %u\n", mibc.etherStatsFragments);
		ret += websWrite(wp, "etherStatsDropEvents		: %u\n", mibc.etherStatsDropEvents);
		ret += websWrite(wp, "etherStatsUndersizePkts		: %u\n", mibc.etherStatsUndersizePkts);
		ret += websWrite(wp, "etherStatsOversizePkts		: %u\n", mibc.etherStatsOversizePkts);
		ret += websWrite(wp, "etherStatsCollisions		: %u\n", mibc.etherStatsCollisions);
		ret += websWrite(wp, "dot3StatsFCSErrors		: %u\n", mibc.dot3StatsFCSErrors);
		ret += websWrite(wp, "dot3StatsSymbolErrors		: %u\n", mibc.dot3StatsSymbolErrors);
		ret += websWrite(wp, "dot3ControlInUnknownOpcodes	: %u\n", mibc.dot3ControlInUnknownOpcodes);
		ret += websWrite(wp, "dot3InPauseFrames		: %u\n", mibc.dot3InPauseFrames);
		ret += websWrite(wp, "dot3OutPauseFrames		: %u", mibc.dot3OutPauseFrames);
	}

	return ret;
}

