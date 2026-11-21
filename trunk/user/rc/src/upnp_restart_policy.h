#ifndef RC_UPNP_RESTART_POLICY_H
#define RC_UPNP_RESTART_POLICY_H
#include <errno.h>
#include <stddef.h>
typedef int (*rc_upnp_stop_fn)(void);
typedef int (*rc_upnp_firewall_restart_fn)(void);
static inline int
rc_upnp_restart_run(rc_upnp_stop_fn stop,
                    rc_upnp_firewall_restart_fn restart_firewall)
{
    int stop_status;
    if (stop == NULL || restart_firewall == NULL)
        return EINVAL;
    stop_status = stop();
    if (stop_status != 0)
        return stop_status;
    return restart_firewall();
}
#endif /* RC_UPNP_RESTART_POLICY_H */
