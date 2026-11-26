#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "networkmap_arp_policy.h"

int
main(void)
{
	static const unsigned char valid_mac[6] = {
		0x02, 0x11, 0x22, 0x33, 0x44, 0x55
	};
	static const unsigned char zero_mac[6] = { 0, 0, 0, 0, 0, 0 };
	static const unsigned char multicast_mac[6] = {
		0x01, 0x00, 0x5e, 0x00, 0x00, 0x01
	};
	static const unsigned char broadcast_mac[6] = {
		0xff, 0xff, 0xff, 0xff, 0xff, 0xff
	};

	assert(networkmap_arp_frame_is_supported(
		htons(ARPHRD_ETHER), htons(ETH_P_IP), 6, 4,
		htons(ARPOP_REQUEST)));
	assert(networkmap_arp_frame_is_supported(
		htons(ARPHRD_ETHER), htons(ETH_P_IP), 6, 4,
		htons(ARPOP_REPLY)));

	assert(!networkmap_arp_frame_is_supported(
		htons(ARPHRD_IEEE802), htons(ETH_P_IP), 6, 4,
		htons(ARPOP_REPLY)));
	assert(!networkmap_arp_frame_is_supported(
		htons(ARPHRD_ETHER), htons(ETH_P_IPV6), 6, 4,
		htons(ARPOP_REPLY)));
	assert(!networkmap_arp_frame_is_supported(
		htons(ARPHRD_ETHER), htons(ETH_P_IP), 8, 4,
		htons(ARPOP_REPLY)));
	assert(!networkmap_arp_frame_is_supported(
		htons(ARPHRD_ETHER), htons(ETH_P_IP), 6, 16,
		htons(ARPOP_REPLY)));
	assert(!networkmap_arp_frame_is_supported(
		htons(ARPHRD_ETHER), htons(ETH_P_IP), 6, 4, htons(3)));

	assert(networkmap_arp_sender_mac_is_valid(valid_mac));
	assert(!networkmap_arp_sender_mac_is_valid(NULL));
	assert(!networkmap_arp_sender_mac_is_valid(zero_mac));
	assert(!networkmap_arp_sender_mac_is_valid(multicast_mac));
	assert(!networkmap_arp_sender_mac_is_valid(broadcast_mac));

	assert(networkmap_arp_receive_error_is_retryable(EINTR));
	assert(networkmap_arp_receive_error_is_retryable(EAGAIN));
	assert(networkmap_arp_receive_error_is_retryable(EWOULDBLOCK));

	assert(!networkmap_arp_receive_error_is_retryable(0));
	assert(!networkmap_arp_receive_error_is_retryable(EIO));
	assert(!networkmap_arp_receive_error_is_retryable(EBADF));
	assert(!networkmap_arp_receive_error_is_retryable(ENODEV));
	assert(!networkmap_arp_receive_error_is_retryable(ENETDOWN));

	puts("networkmap ARP frame validation and receive error policy: PASS");
	return 0;
}
