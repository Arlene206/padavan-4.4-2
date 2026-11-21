#ifndef DETECT_WAN_ARP_SOCKET_POLICY_H
#define DETECT_WAN_ARP_SOCKET_POLICY_H

#include <errno.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/time.h>

typedef int (*detect_wan_setsockopt_fn)(int, int, int, const void *, socklen_t);

/*
 * A gateway ARP probe is only trustworthy when its receive socket is bound
 * to the intended interface and has a finite receive timeout. Fail closed
 * if either prerequisite cannot be installed.
 */
static inline int
detect_wan_arp_socket_prepare(int fd, const struct ifreq *interface_request,
                              const struct timeval *receive_timeout,
                              detect_wan_setsockopt_fn set_option)
{
    if (fd < 0 || !interface_request || !receive_timeout || !set_option) {
        errno = EINVAL;
        return -1;
    }

    if (set_option(fd, SOL_SOCKET, SO_BINDTODEVICE, interface_request,
                   sizeof(*interface_request)) < 0)
        return -1;

    if (set_option(fd, SOL_SOCKET, SO_RCVTIMEO, receive_timeout,
                   sizeof(*receive_timeout)) < 0)
        return -1;

    return 0;
}

#endif /* DETECT_WAN_ARP_SOCKET_POLICY_H */
