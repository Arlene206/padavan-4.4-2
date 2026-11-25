#ifndef NETWORKMAP_ARP_POLICY_H
#define NETWORKMAP_ARP_POLICY_H

#include <errno.h>

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

#endif
