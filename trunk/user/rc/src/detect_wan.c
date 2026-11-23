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
#include "detect_wan_arp_result_policy.h"
#include "detect_wan_udhcpc_policy.h"
#include "wan_nvram_status_policy.h"

//#define DEBUG		1
#define MAX_ARP_WAIT	5
#define MAX_ARP_RETRY	3
#define DW_PID_FILE	"/var/run/detect_wan.pid"

static volatile sig_atomic_t dw_stopping;

static int
detect_wan_nvram_get_checked(void *context, const char *param_name,
			     char **value)
{
	(void)context;
	return nvram_get_checked(param_name, value);
}



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
	char gateway_storage[16];
	in_addr_t ip;
	struct process_snapshot snapshot;
	enum process_presence presence;
	int scan_error, renew_decision, renew_status;
	int gateway_status, wan_proto = 0;

	if (dw_stopping)
		return 0;

	if (!ap_mode) {
		gateway_status = get_wan_unit_proto_checked_status(0, &wan_proto);
		if (gateway_status != 0) {
			errno = gateway_status;
			logmessage("detect_wan",
				   "cannot read WAN protocol before DHCP health check (status=%d)",
				   gateway_status);
			return gateway_status;
		}
		if (wan_proto != IPV4_WAN_PROTO_IPOE_DHCP)
			return 0;
	}

	if (!ap_mode)
		ip = get_wan_ip4(1);
	else
		ip = get_lan_ip4();
	if (ip == INADDR_ANY)
		return 0;

	if (ap_mode)
		gateway_status = rc_wan_nvram_copy_value(
			detect_wan_nvram_get_checked, NULL, "lan_gateway_t",
			gateway_storage, sizeof(gateway_storage));
	else
		gateway_status = get_wan_unit_value_copy_status(
			0, "gateway", gateway_storage, sizeof(gateway_storage));
	if (gateway_status != 0) {
		errno = gateway_status;
		logmessage("detect_wan",
			   "cannot read gateway before DHCP health check (status=%d)",
			   gateway_status);
		return gateway_status;
	}
	if (!is_valid_ipv4(gateway_storage))
		return 0;

	presence = process_inventory_scan("udhcpc", &snapshot);
	scan_error = snapshot.error;
	renew_decision = detect_wan_udhcpc_should_renew(presence);
	process_snapshot_release(&snapshot);

	if (renew_decision < 0) {
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
	logmessage("detect_wan", "No response from gateway (%s)! Perform DHCP renew...", gateway_storage);
	renew_status = process_pidfile_signal(detect_wan_udhcpc_pidfile(ap_mode),
					     "udhcpc", SIGUSR1);
	if (renew_status != 0) {
		if (renew_status != -1 || errno == 0)
			errno = EIO;
		logmessage("detect_wan",
			   "Failed to request udhcpc renewal (status %d, error %d)",
			   renew_status, errno);
		return -1;
	}

	return 0;
}


static int
arpping_gateway(int ap_mode)
{
    in_addr_t gateway_ip;
    in_addr_t ip;
    uint8_t mac[6];
    char gateway_storage[16];
    char hardware_address_storage[18];
    char interface_storage[IFNAMSIZ];
    int s, rv, rcv_bytes, probe_errno = 0, config_status;
    struct sockaddr addr;
    struct ifreq ifr;
    struct arpMsg arp;
    struct detect_wan_arp_reply arp_reply;
    struct timeval arp_timeout;
    long timeStart;
    const int one = 1;

    if (dw_stopping)
        return DETECT_WAN_ARP_PROBE_NO_REPLY;

    if (!ap_mode)
    {
        ip = get_wan_ip4(1);
        config_status = get_wan_unit_value_copy_status(
            0, "ifname", interface_storage, sizeof(interface_storage));
        if (config_status == 0)
            config_status = get_wan_unit_value_copy_status(
                0, "gateway", gateway_storage, sizeof(gateway_storage));
        if (config_status == 0)
            config_status = get_wan_unit_value_copy_status(
                0, "hwaddr", hardware_address_storage,
                sizeof(hardware_address_storage));
    }
    else
    {
        ip = get_lan_ip4();
        config_status = rc_wan_nvram_copy_text(
            IFNAME_BR, interface_storage, sizeof(interface_storage));
        if (config_status == 0)
            config_status = rc_wan_nvram_copy_value(
                detect_wan_nvram_get_checked, NULL, "lan_gateway_t",
            gateway_storage, sizeof(gateway_storage));
        if (config_status == 0)
            config_status = rc_wan_nvram_copy_value(
                detect_wan_nvram_get_checked, NULL, "lan_hwaddr",
                hardware_address_storage, sizeof(hardware_address_storage));
    }
    if (config_status != 0) {
        errno = config_status;
        logmessage("detect_wan",
                   "cannot read gateway ARP identity (status=%d)",
                   config_status);
        return detect_wan_arp_probe_result(0, config_status);
    }
    gateway_ip = inet_addr_safe(gateway_storage);

    if (gateway_ip == INADDR_ANY || ip == INADDR_ANY || gateway_ip == ip)
        return DETECT_WAN_ARP_PROBE_REPLY;

    config_status = detect_wan_arp_parse_hardware_address(
        hardware_address_storage, mac);
    if (config_status != 0) {
        errno = config_status;
        logmessage("detect_wan",
                   "invalid local hardware address for gateway ARP probe (status=%d)",
                   config_status);
        return detect_wan_arp_probe_result(0, config_status);
    }

    memset(&addr, 0, sizeof(addr));
    memset(&ifr, 0, sizeof(ifr));
    config_status = detect_wan_arp_socket_device_name_prepare(
        interface_storage, &ifr, &addr);
    if (config_status < 0) {
        probe_errno = errno ? errno : EIO;
        logmessage("detect_wan",
                   "invalid or unavailable ARP egress interface (status=%d)",
                   probe_errno);
        return detect_wan_arp_probe_result(0, probe_errno);
    }

    s = socket(PF_PACKET, SOCK_PACKET, htons(ETH_P_ARP));
    if (s < 0)
        return detect_wan_arp_probe_result(0, errno ? errno : EIO);

    if (setsockopt(s, SOL_SOCKET, SO_BROADCAST, &one, sizeof(one)) < 0) {
        probe_errno = errno ? errno : EIO;
        (void)close(s);
        return detect_wan_arp_probe_result(0, probe_errno);
    }

    memset(&arp, 0, sizeof(arp));
    memset(arp.h_dest, 0xFF, 6);
    memcpy(arp.h_source, mac, 6);
    arp.h_proto = htons(ETH_P_ARP);
    arp.htype = htons(ARPHRD_ETHER);
    arp.ptype = htons(ETH_P_IP);
    arp.hlen = 6;
    arp.plen = 4;
    arp.operation = htons(ARPOP_REQUEST);
    memcpy(arp.sInaddr, &ip, sizeof(ip));
    memcpy(arp.sHaddr, mac, 6);
    memcpy(arp.tInaddr, &gateway_ip, sizeof(gateway_ip));

    arp_timeout.tv_sec = 1;
    arp_timeout.tv_usec = 0;
    if (detect_wan_arp_socket_prepare(s, &ifr, &arp_timeout, setsockopt) < 0) {
        probe_errno = errno ? errno : EIO;
        (void)close(s);
        return detect_wan_arp_probe_result(0, probe_errno);
    }

    if (sendto(s, &arp, sizeof(arp), 0, &addr, sizeof(addr)) < 0) {
        probe_errno = errno ? errno : EIO;
        (void)close(s);
        return detect_wan_arp_probe_result(0, probe_errno);
    }

    /* A finite receive timeout separates silence from hard socket errors. */
    rv = 0;
    timeStart = uptime();
    for (;;)
    {
        if (dw_stopping)
            break;
        rcv_bytes = recvfrom(s, &arp, sizeof(arp), 0, NULL, NULL);
        if (dw_stopping)
            break;
        if (rcv_bytes < 0) {
            if (errno == EINTR)
                continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                if (uptime() - timeStart > MAX_ARP_WAIT)
                    break;
                continue;
            }
            probe_errno = errno ? errno : EIO;
            break;
        }

        if (rcv_bytes >= 42) {
            arp_reply.ethernet_protocol = arp.h_proto;
            arp_reply.hardware_type = arp.htype;
            arp_reply.protocol_type = arp.ptype;
            arp_reply.hardware_length = arp.hlen;
            arp_reply.protocol_length = arp.plen;
            arp_reply.operation = arp.operation;
            arp_reply.ethernet_source = arp.h_source;
            arp_reply.ethernet_destination = arp.h_dest;
            arp_reply.sender_hardware = arp.sHaddr;
            arp_reply.sender_protocol = arp.sInaddr;
            arp_reply.target_hardware = arp.tHaddr;
            arp_reply.target_protocol = arp.tInaddr;
            if (detect_wan_arp_reply_matches(&arp_reply, mac, gateway_ip, ip)) {
#ifdef DEBUG
                fprintf(stderr, "Valid arp reply from [%02X:%02X:%02X:%02X:%02X:%02X]\n",
                        (unsigned char)arp.sHaddr[0], (unsigned char)arp.sHaddr[1],
                        (unsigned char)arp.sHaddr[2], (unsigned char)arp.sHaddr[3],
                        (unsigned char)arp.sHaddr[4], (unsigned char)arp.sHaddr[5]);
#endif
                rv = 1;
                break;
            }
        }

        if (uptime() - timeStart > MAX_ARP_WAIT)
            break;
    }

    if (close(s) < 0 && probe_errno == 0)
        probe_errno = errno ? errno : EIO;
    return detect_wan_arp_probe_result(rv, probe_errno);
}

static int
poll_gateway(void)
{
    int count, probe_count, probe_status, probe_error, link_up, renew_status;
    int ap_mode = get_ap_mode();

    while (!dw_stopping)
    {
        count = 0;
        probe_count = 0;
        probe_error = 0;

        while (count < MAX_ARP_RETRY && !dw_stopping)
        {
            if (!ap_mode && !get_wan_ether_link_cached())
            {
                count++;
                sleep(2);
            }
            else
            {
                probe_status = arpping_gateway(ap_mode);
                probe_count++;
                if (probe_status == DETECT_WAN_ARP_PROBE_REPLY)
                    break;
                if (probe_status == DETECT_WAN_ARP_PROBE_ERROR) {
                    if (probe_error == 0) {
                        probe_error = errno > 0 ? errno : EIO;
                        logmessage("detect_wan",
                                   "gateway ARP probe failed locally (errno=%d); suppressing DHCP renew",
                                   probe_error);
                    }
                }
                else
                {
#ifdef DEBUG
                    fprintf(stderr, "[detect_wan] no response from gateway\n");
#endif
                }
                count++;
            }
        }

        if (dw_stopping)
            break;

        link_up = ap_mode ? 1 : get_wan_ether_link_cached();
        if (detect_wan_arp_should_renew(count >= MAX_ARP_RETRY,
                                        probe_count, MAX_ARP_RETRY,
                                        ap_mode, link_up, probe_error))
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
