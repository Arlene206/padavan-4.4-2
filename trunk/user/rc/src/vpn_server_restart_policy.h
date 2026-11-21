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

/* Restore a shared L2TP transport when teardown may have stopped it and a
 * WAN L2TP or VPNC client still depends on the daemon. */
static inline int
vpn_server_restart_should_restore_shared_xl2tpd(int stop_attempted,
						int wan_l2tp_enabled,
						int vpnc_l2tp_enabled)
{
    return stop_attempted && (wan_l2tp_enabled || vpnc_l2tp_enabled);
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
