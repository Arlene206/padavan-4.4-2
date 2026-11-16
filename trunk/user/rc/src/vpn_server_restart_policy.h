#ifndef VPN_SERVER_RESTART_POLICY_H
#define VPN_SERVER_RESTART_POLICY_H

#include <errno.h>
#include <stddef.h>

/* Preserve the first failure while still attempting independent restoration. */
static inline int
vpn_server_restart_first_error(int current_status, int next_status)
{
    return current_status != 0 ? current_status : next_status;
}

typedef int (*vpn_openvpn_restart_step_fn)(void);

/* Stop, fence callback leases, and only then launch the replacement daemon. */
static inline int
vpn_openvpn_server_restart_run(vpn_openvpn_restart_step_fn stop,
                               vpn_openvpn_restart_step_fn clear_lease,
                               vpn_openvpn_restart_step_fn start)
{
    int status;

    if (stop == NULL || clear_lease == NULL || start == NULL)
        return EINVAL;

    status = stop();
    if (status != 0)
        return status;

    status = clear_lease();
    if (status != 0)
        return status;

    return start();
}

#endif /* VPN_SERVER_RESTART_POLICY_H */
