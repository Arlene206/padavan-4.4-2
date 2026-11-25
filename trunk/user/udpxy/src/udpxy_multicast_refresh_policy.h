/* Testable multicast renewal and retry-deadline policy. */
#ifndef UDPXY_MULTICAST_REFRESH_POLICY_H
#define UDPXY_MULTICAST_REFRESH_POLICY_H

#include <stddef.h>
#include <time.h>

struct in_addr;

typedef int (*udpxy_multicast_update_fn)(
    int, const struct in_addr *, const struct in_addr *,
    const char *, void *);

/*
 * A DROP failure must not prevent ADD. The membership may already be absent
 * (for example after a previous DROP succeeded but ADD failed); only the ADD
 * result determines whether the renewed subscription is now established.
 */
static inline int
udpxy_renew_multicast_with(udpxy_multicast_update_fn update, void *context,
                           int msockfd, const struct in_addr *mifaddr,
                           const struct in_addr *s_in_addr)
{
    if (NULL == update || NULL == mifaddr || NULL == s_in_addr)
        return -1;

    (void)update(msockfd, mifaddr, s_in_addr, "DROP", context);
    return update(msockfd, mifaddr, s_in_addr, "ADD", context);
}

/*
 * On failure, make the next attempt due in one second rather than silently
 * waiting a full refresh interval or retrying on every relay-loop iteration.
 */
static inline time_t
udpxy_mcast_refresh_anchor(time_t now, time_t interval, int refresh_status)
{
    if (0 == refresh_status || interval <= (time_t)1)
        return now;

    return now - interval + (time_t)1;
}

#endif /* UDPXY_MULTICAST_REFRESH_POLICY_H */
