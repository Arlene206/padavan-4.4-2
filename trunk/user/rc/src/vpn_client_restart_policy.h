#ifndef VPN_CLIENT_RESTART_POLICY_H
#define VPN_CLIENT_RESTART_POLICY_H

/* A forced VPN-client stop can remove the xl2tpd instance shared with WAN
 * L2TP or the L2TP VPN server. Restore it after a later teardown failure. */
static inline int
vpn_client_restart_should_restore_shared_xl2tpd(int xl2tpd_was_stopped,
                                                int wan_l2tp_active,
                                                int vpn_server_l2tp_active)
{
	return xl2tpd_was_stopped &&
		(wan_l2tp_active || vpn_server_l2tp_active);
}

/* An unreadable dependency snapshot is not evidence that xl2tpd is unused. */
static inline int
vpn_client_restart_should_attempt_restore_shared_xl2tpd(
    int xl2tpd_was_stopped, int dependency_read_status,
    int wan_l2tp_active, int vpn_server_l2tp_active)
{
    return xl2tpd_was_stopped &&
        (dependency_read_status != 0 || wan_l2tp_active ||
         vpn_server_l2tp_active);
}

#endif /* VPN_CLIENT_RESTART_POLICY_H */

