#ifndef WSD_NETLINK_RECV_POLICY_H
#define WSD_NETLINK_RECV_POLICY_H

#include <sys/socket.h>

static inline int
wsd_netlink_datagram_truncated(int message_flags)
{
	return (message_flags & MSG_TRUNC) != 0;
}

#endif /* WSD_NETLINK_RECV_POLICY_H */
