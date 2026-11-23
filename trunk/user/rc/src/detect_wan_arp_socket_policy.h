#ifndef DETECT_WAN_ARP_SOCKET_POLICY_H
#define DETECT_WAN_ARP_SOCKET_POLICY_H

#include <errno.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/time.h>

typedef int (*detect_wan_setsockopt_fn)(int, int, int, const void *, socklen_t);

/*
 * SOCK_PACKET transmits through sockaddr.sa_data, which is only 14 bytes;
 * Linux 4.4 forcibly NUL-terminates the last byte before looking up the
 * device. Require a non-empty name that fits both ifreq.ifr_name and this
 * legacy send address, and copy it to both destinations without truncation.
 */
static inline int
detect_wan_arp_socket_device_name_prepare(const char *interface_name,
                                          struct ifreq *interface_request,
                                          struct sockaddr *packet_address)
{
    size_t length = 0;

    if (!interface_name || !interface_request || !packet_address) {
        errno = EINVAL;
        return -1;
    }

    while (length < sizeof(interface_request->ifr_name) &&
           length < sizeof(packet_address->sa_data) &&
           interface_name[length] != '\0')
        ++length;

    if (length == 0) {
        errno = ENODEV;
        return -1;
    }
    if (length >= sizeof(interface_request->ifr_name) ||
        length >= sizeof(packet_address->sa_data)) {
        errno = ENAMETOOLONG;
        return -1;
    }

    memcpy(interface_request->ifr_name, interface_name, length + 1);
    memcpy(packet_address->sa_data, interface_name, length + 1);
    return 0;
}

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
    if (interface_request->ifr_name[0] == '\0') {
        errno = ENODEV;
        return -1;
    }
    if (memchr(interface_request->ifr_name, '\0',
               sizeof(interface_request->ifr_name)) == NULL) {
        errno = ENAMETOOLONG;
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
