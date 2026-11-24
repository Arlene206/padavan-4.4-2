#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "networkmap_arp_policy.h"

int
main(void)
{
	assert(networkmap_arp_receive_error_is_retryable(EINTR));
	assert(networkmap_arp_receive_error_is_retryable(EAGAIN));
	assert(networkmap_arp_receive_error_is_retryable(EWOULDBLOCK));

	assert(!networkmap_arp_receive_error_is_retryable(0));
	assert(!networkmap_arp_receive_error_is_retryable(EIO));
	assert(!networkmap_arp_receive_error_is_retryable(EBADF));
	assert(!networkmap_arp_receive_error_is_retryable(ENODEV));
	assert(!networkmap_arp_receive_error_is_retryable(ENETDOWN));

	puts("networkmap ARP receive error policy: PASS");
	return 0;
}
