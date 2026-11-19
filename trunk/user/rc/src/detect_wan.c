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

#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>
#include <netinet/if_ether.h>
#include <net/if.h>
#include <net/if_arp.h>

#include "rc.h"
#include "process_pidfile.h"
#include "detect_wan_arp_socket_policy.h"
#include "detect_wan_signal_policy.h"
#include "detect_wan_udhcpc_policy.h"

//#define DEBUG		1
#define MAX_ARP_WAIT	5
#define MAX_ARP_RETRY	3
#define DW_PID_FILE	"/var/run/detect_wan.pid"

static volatile sig_atomic_t dw_stopping;


struct arpMsg {
	/* Ethernet header */
	uint8_t  h_dest[6];			/* destination ether addr */
	uint8_t  h_source[6];			/* source ether addr */
	uint16_t h_proto;			/* packet type ID field */

	/* ARP packet */
	uint16_t htype;				/* hardware type (must be ARPHRD_ETHER) */
	uint16_t ptype;				/* protocol type (must be ETH_P_IP) */
	uint8_t  hlen;				/* hardware address length (must be 6) */
	uint8_t  plen;				/* protocol address length (must be 4) */
	uint16_t operation;			/* ARP opcode */
	uint8_t  sHaddr[6];			/* sender's hardware address */
	uint8_t  sInaddr[4];			/* sender's IP address */
	uint8_t  tHaddr[6];			/* target's hardware address */
	uint8_t  tInaddr[4];			/* target's IP address */
	uint8_t  pad[18];			/* pad for min. Ethernet payload (60 bytes) */
} ATTRIBUTE_PACKED;


static void
detect_wan_wait(unsigned int seconds)
{
	/* One-second slices bound shutdown latency if SIGTERM arrives just
	 * before sleep() is entered rather than during an active sleep. */
	while (seconds > 0 && !dw_stopping) {
		(void)sleep(1);
		seconds--;
	}
}


static int
chk_udhcpc(int ap_mode)
{
	char *gateway_str;
	in_addr_t ip;
	struct process_snapshot snapshot;
	enum process_presence presence;
	int scan_error, renew_decision, renew_status;

	if (dw_stopping)
		return 0;

	if (ap_mode || get_wan_proto(0) == IPV4_WAN_PROTO_IPOE_DHCP)
	{
		if (!ap_mode)
		{
			ip = get_wan_ip4(1);
			gateway_str = get_wan_unit_value(0, "gateway");
		}
		else
		{
			ip = get_lan_ip4();
			gateway_str = nvram_safe_get("lan_gateway_t");
		}

		if (!is_valid_ipv4(gateway_str) || ip == INADDR_ANY)
			return 0;

		presence = process_inventory_scan("udhcpc", &snapshot);
		scan_error = snapshot.error;
		renew_decision = detect_wan_udhcpc_should_renew(presence);
		process_snapshot_release(&snapshot);

		if (renew_decision < 0)
		{
			errno = scan_error ? scan_error : EIO;
			logmessage("detect_wan",
				   "Unable to verify udhcpc state; refusing gateway DHCP renew (error %d)",
				   errno);
			return -1;
		}
		if (renew_decision == 0)
			return 0;

		if (dw_stopping)
			return 0;
		logmessage("detect_wan", "No response from gateway (%s)! Perform DHCP renew...", gateway_str);
		renew_status = process_pidfile_signal(detect_wan_udhcpc_pidfile(ap_mode),
						     "udhcpc", SIGUSR1);
		if (renew_status != 0)
		{
			if (renew_status != -1 || errno == 0)
				errno = EIO;
			logmessage("detect_wan",
				   "Failed to request udhcpc renewal (status %d, error %d)",
				   renew_status, errno);
			return -1;
		}
	}

	return 0;
}


static int
arpping_gateway(int ap_mode)
{
	in_addr_t gateway_ip;
	in_addr_t ip;
	uint8_t mac[6];
	char wanmac[18];
	char tmp[3];
	char *ifname;
	int s, i, rv, rcv_bytes;
	struct sockaddr addr;
	struct ifreq ifr;
	struct arpMsg arp;
	struct timeval arp_timeout;
	long timeStart;
	const int one = 1;

	if (dw_stopping)
		return 0;

	if (!ap_mode)
	{
		ip = get_wan_ip4(1);
		gateway_ip = inet_addr_safe(get_wan_unit_value(0, "gateway"));
		snprintf(wanmac, sizeof(wanmac), "%s", get_wan_unit_value(0, "hwaddr"));	// WAN MAC address
	}
	else
	{
		ip = get_lan_ip4();
		gateway_ip = inet_addr_safe(nvram_safe_get("lan_gateway_t"));
		snprintf(wanmac, sizeof(wanmac), "%s", nvram_safe_get("lan_hwaddr"));	// br0 MAC address
	}

	if (gateway_ip == INADDR_ANY || ip == INADDR_ANY || gateway_ip == ip)
	{
		return 1;
	}

	wanmac[17]=0;
	for(i=0; i<6; i++)
	{
		strncpy(tmp, wanmac+i*3, 2);
		tmp[2]=0;
		mac[i]=strtol(tmp, NULL, 16);
	}

	s = socket(PF_PACKET, SOCK_PACKET, htons(ETH_P_ARP));
	if (s < 0) {
		perror("socket:");
		return 0;
	}
	
	if (setsockopt(s, SOL_SOCKET, SO_BROADCAST, &one, sizeof(one)) < 0) {
		perror("setsockopt:");
		close(s);
		return 0;
	}

	/* send arp request */
	memset(&arp, 0, sizeof(arp));
	memset(arp.h_dest, 0xFF, 6);			/* MAC DA (Broadcast) */
	memcpy(arp.h_source, mac, 6);			/* MAC SA */
	arp.h_proto = htons(ETH_P_ARP);			/* protocol type (Ethernet) */
	arp.htype = htons(ARPHRD_ETHER);		/* hardware type */
	arp.ptype = htons(ETH_P_IP);			/* protocol type (ARP message) */
	arp.hlen = 6;					/* hardware address length */
	arp.plen = 4;					/* protocol address length */
	arp.operation = htons(ARPOP_REQUEST);		/* ARP op code */
	memcpy(arp.sInaddr, &ip, sizeof(ip));		/* source IP address */
	memcpy(arp.sHaddr, mac, 6);			/* source hardware address */
	memcpy(arp.tInaddr, &gateway_ip, sizeof(gateway_ip));	/* target IP address */

	memset(&addr, 0, sizeof(addr));
	memset(&ifr, 0, sizeof(ifr));
	if (!ap_mode)
		ifname = get_man_ifname(0);
	else
		ifname = IFNAME_BR;

	strncpy(addr.sa_data, ifname, sizeof(addr.sa_data));
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	arp_timeout.tv_sec = 1;
	arp_timeout.tv_usec = 0;
	if (detect_wan_arp_socket_prepare(s, &ifr, &arp_timeout, setsockopt) < 0) {
		perror("setsockopt:");
		close(s);
		return 0;
	}

	if (sendto(s, &arp, sizeof(arp), 0, &addr, sizeof(addr)) < 0)
	{
		close(s);
		
		perror("sendto:");
		
		detect_wan_wait(1);
		return 0;
	}

	/* wait arp reply; socket preparation installed a finite timeout above. */
	rv = 0;
	timeStart = uptime();
	for(;;)
	{
		if (dw_stopping)
			break;
		rcv_bytes = recvfrom(s, &arp, sizeof(arp), 0, NULL, NULL);
		if (dw_stopping)
			break;
		if (rcv_bytes < 0 && errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK)
			break;
		
		if (rcv_bytes >= 42 &&
		    arp.h_proto == htons(ETH_P_ARP) &&
		    arp.operation == htons(ARPOP_REPLY) &&
		    memcmp(arp.tHaddr, mac, 6) == 0 &&
		    *((uint32_t *) arp.sInaddr) == gateway_ip) 
		{
#ifdef DEBUG
				fprintf(stderr, "Valid arp reply from [%02X:%02X:%02X:%02X:%02X:%02X]\n",
					(unsigned char)arp.sHaddr[0],
					(unsigned char)arp.sHaddr[1],
					(unsigned char)arp.sHaddr[2],
					(unsigned char)arp.sHaddr[3],
					(unsigned char)arp.sHaddr[4],
					(unsigned char)arp.sHaddr[5]);
#endif
			rv = 1;
			break;
		}
		
		if (uptime() - timeStart > MAX_ARP_WAIT)
			break;
	}
	
	close(s);

	return rv;
}

static int
poll_gateway(void)
{
	int count, renew_status;
	int ap_mode = get_ap_mode();

	while (!dw_stopping)
	{
		count = 0;
		
		while (count < MAX_ARP_RETRY && !dw_stopping)
		{
			if (!ap_mode && !get_wan_ether_link_cached())
			{
				count++;
				sleep(2);
			}
			else if (arpping_gateway(ap_mode))
			{
				break;
			}
			else
			{
#ifdef DEBUG
				fprintf(stderr, "[detect_wan] no response from gateway\n");
#endif
				count++;
			}
		}
		
		if (dw_stopping)
			break;
		if ((count >= MAX_ARP_RETRY) && (ap_mode || get_wan_ether_link_cached()))
		{
			renew_status = chk_udhcpc(ap_mode);
			if (dw_stopping)
				break;
			if (renew_status != 0)
				return -1;
		}
		
		detect_wan_wait(20);
	}

	return 0;
}

static void
catch_sig_detect_wan(int sig)
{
	detect_wan_record_signal(&dw_stopping, sig);
}

int
detect_wan_main(int argc, char *argv[])
{
	struct sigaction sa;
	int result, saved_errno, cleanup_errno;

	(void)argc;
	(void)argv;

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = catch_sig_detect_wan;
	sigemptyset(&sa.sa_mask);
	if (sigaction(SIGTERM, &sa, NULL) < 0) {
		saved_errno = errno;
		errno = saved_errno;
		perror("sigaction SIGTERM");
		return saved_errno;
	}

	memset(&sa, 0, sizeof(sa));
	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	if (sigaction(SIGPIPE, &sa, NULL) < 0 ||
	    sigaction(SIGHUP, &sa, NULL) < 0 ||
	    sigaction(SIGUSR1, &sa, NULL) < 0 ||
	    sigaction(SIGUSR2, &sa, NULL) < 0) {
		saved_errno = errno;
		errno = saved_errno;
		perror("sigaction ignored signal");
		return saved_errno;
	}

	if (daemon(0, 0) < 0) {
		saved_errno = errno;
		errno = saved_errno;
		perror("daemon");
		return saved_errno;
	}

	/* A stop received during daemonization must not publish a stale PID. */
	if (dw_stopping)
		return 0;

	/* Publish a PID/start-time identity before the monitor begins polling. */
	if (process_pidfile_write(DW_PID_FILE, "detect_wan") < 0) {
		saved_errno = errno ? errno : EIO;
		errno = saved_errno;
		logmessage("detect_wan", "cannot publish pid file (errno=%d)", saved_errno);
		return saved_errno;
	}

	result = poll_gateway();
	saved_errno = errno;
	/* PID cleanup and logging run in normal context, never in the handler. */
	if (remove(DW_PID_FILE) != 0 && errno != ENOENT) {
		cleanup_errno = errno ? errno : EIO;
		logmessage("detect_wan", "cannot remove pid file (errno=%d)", cleanup_errno);
		if (result == 0) {
			result = -1;
			saved_errno = cleanup_errno;
		}
	}
	if (result != 0 && saved_errno != 0)
		errno = saved_errno;
	return result;
}
