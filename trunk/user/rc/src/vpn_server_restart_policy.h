#ifndef VPN_SERVER_RESTART_POLICY_H
#define VPN_SERVER_RESTART_POLICY_H

/* Preserve the first failure while still attempting independent restoration. */
static inline int
vpn_server_restart_first_error(int current_status, int next_status)
{
    return current_status != 0 ? current_status : next_status;
}

#endif /* VPN_SERVER_RESTART_POLICY_H */
