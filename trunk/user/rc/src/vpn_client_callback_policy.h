#ifndef PADAVAN_VPN_CLIENT_CALLBACK_POLICY_H
#define PADAVAN_VPN_CLIENT_CALLBACK_POLICY_H

#include <sys/types.h>

#include "ppp_callback_identity_policy.h"
#include "vpn_server_callback_policy.h"

/* pppd's MAXUNIT is 255; callback interface tokens must be exact pppN names. */
#define VPN_CLIENT_CALLBACK_MAX_PPP_UNITS 256U

static inline int
vpn_client_callback_interface_is_valid(const char *ifname)
{
	int unit;

	if (ppp_callback_parse_index(
		ifname, "ppp", VPN_CLIENT_CALLBACK_MAX_PPP_UNITS, &unit) != 0)
		return 0;
	return unit >= 0;
}

/*
 * The callback's PPPD_PID, direct parent and live per-interface pidfile must
 * all name one process. This rejects re-parented callbacks and a reused pppN.
 */
static inline int
vpn_client_callback_session_matches(const char *ifname, const char *pid_text,
				    pid_t parent_pid, pid_t pidfile_pid)
{
	pid_t callback_pid;

	if (!vpn_client_callback_interface_is_valid(ifname) ||
	    vpn_server_callback_pid_parse(pid_text, &callback_pid) != 0)
		return 0;

	return vpn_server_callback_pids_match(
		callback_pid, parent_pid, pidfile_pid);
}

#endif /* PADAVAN_VPN_CLIENT_CALLBACK_POLICY_H */
