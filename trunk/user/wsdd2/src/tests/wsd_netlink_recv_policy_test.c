#include <assert.h>
#include <sys/socket.h>

#include "wsd_netlink_recv_policy.h"

int
main(void)
{
	assert(!wsd_netlink_datagram_truncated(0));
	assert(!wsd_netlink_datagram_truncated(MSG_DONTWAIT));
	assert(wsd_netlink_datagram_truncated(MSG_TRUNC));
	assert(wsd_netlink_datagram_truncated(MSG_TRUNC | MSG_DONTWAIT));
	return 0;
}
