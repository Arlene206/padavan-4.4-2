#ifndef NETWORKMAP_ARP_POLICY_H
#define NETWORKMAP_ARP_POLICY_H

#include <errno.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <netinet/if_ether.h>

/*
 * The bridge scanner only consumes Ethernet/IPv4 ARP. Reject other protocol
 * shapes and operations before interpreting sender/target fields.
 * Header fields are passed in their on-wire byte order.
 */
static inline int
networkmap_arp_frame_is_supported(unsigned short hardware_type,
				  unsigned short protocol_type,
				  unsigned int hardware_address_length,
				  unsigned int protocol_address_length,
				  unsigned short operation)
{
	unsigned short host_operation = ntohs(operation);

	return ntohs(hardware_type) == ARPHRD_ETHER &&
	       ntohs(protocol_type) == ETH_P_IP &&
	       hardware_address_length == 6U &&
	       protocol_address_length == 4U &&
	       (host_operation == ARPOP_REQUEST ||
		host_operation == ARPOP_REPLY);
}

/* ARP sender addresses must be nonzero, individual (not multicast) MACs. */
static inline int
networkmap_arp_sender_mac_is_valid(const unsigned char *sender_mac)
{
	unsigned int i;
	unsigned char nonzero = 0;

	if (!sender_mac || (sender_mac[0] & 0x01U))
		return 0;

	for (i = 0; i < 6U; i++)
		nonzero |= sender_mac[i];

	return nonzero != 0;
}

/*
 * recvfrom() may be interrupted by the daemon's flag-only signal handlers or
 * time out after the full scan. Those conditions end this drain pass; other
 * errors are propagated so main can clean up and let the supervisor restart.
 */
static inline int
networkmap_arp_receive_error_is_retryable(int error)
{
	return error == EINTR || error == EAGAIN || error == EWOULDBLOCK;
}

/* SO_RCVTIMEO is required to keep the ARP receive loop bounded. */
static inline int
networkmap_arp_receive_timeout_error(int option_status, int saved_errno)
{
	if (option_status == 0)
		return 0;
	return saved_errno > 0 ? saved_errno : EIO;
}

/* Descriptors 0-2 are redirected by daemon(0, 0); keep the socket above them. */
static inline int
networkmap_arp_socket_fd_is_valid(int fd)
{
	return fd >= 0;
}

static inline int
networkmap_arp_socket_fd_needs_duplication(int fd)
{
	return fd >= 0 && fd <= 2;
}

#endif
