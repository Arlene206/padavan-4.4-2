#ifndef XL2TPD_START_POLICY_H
#define XL2TPD_START_POLICY_H
#include <process_inventory.h>
#include <errno.h>
#include <string.h>

/* NVRAM setters use zero for success; retain close/ioctl errors on failure. */
static inline int
xl2tpd_start_policy_write_status(int api_result, int saved_errno)
{
	if (api_result == 0)
		return 0;
	return saved_errno > 0 ? saved_errno : EIO;
}

struct xl2tpd_config_flags {
	int use_vpn_server;
	int use_wan_lac;
	int use_vpnc;
	int vpnc_default_route;
};

/*
 * Preserve nvram_match()/nvram_invmatch() legacy semantics, but never
 * interpret a failed transaction as a disabled VPN or optional LAC.
 */
static inline int
xl2tpd_start_policy_config_flags(int read_status, int wan_proto,
				 int l2tp_proto, int rpl2tp_build,
				 const char *vpns_enable,
				 const char *vpns_type,
				 const char *vpnc_enable,
				 const char *vpnc_type,
				 const char *vpnc_dgw,
				 const char *wan_l2tpd,
				 struct xl2tpd_config_flags *flags)
{
	if (!flags)
		return EINVAL;
	memset(flags, 0, sizeof(*flags));
	if (read_status != 0)
		return read_status;
	if (!vpns_enable || !vpns_type || !vpnc_enable ||
	    !vpnc_type || !vpnc_dgw || !wan_l2tpd)
		return EINVAL;

	flags->use_vpn_server =
		(strcmp(vpns_enable, "1") == 0 && strcmp(vpns_type, "1") == 0);
	flags->use_wan_lac =
		(wan_proto == l2tp_proto &&
		 (!rpl2tp_build || strcmp(wan_l2tpd, "1") != 0));
	flags->use_vpnc =
		(strcmp(vpnc_enable, "1") == 0 && strcmp(vpnc_type, "1") == 0);
	flags->vpnc_default_route =
		(strcmp(vpnc_dgw, "1") == 0) ? 2 : 0;
	return 0;
}

/* 1 = running, 0 = absent, -1 = unknown (fail closed). */
static inline int
xl2tpd_start_policy_result(enum process_presence presence)
{
	if (presence == PROCESS_PRESENCE_RUNNING)
		return 1;
	if (presence == PROCESS_PRESENCE_ABSENT)
		return 0;
	return -1;
}

/*
 * Convert the launch result without confusing a positive child exit status
 * with the "already running" sentinel returned by xl2tpd_start_policy_result.
 */
static inline int
xl2tpd_start_policy_completion_result(int presence_status, int command_status)
{
	if (command_status != 0)
		return -1;
	if (presence_status == 1 || presence_status == 0)
		return presence_status;
	return -1;
}

/* A failed RPL2TP session must roll back the daemon launched for it. */
static inline int
xl2tpd_start_policy_rpl2tp_failure_requires_cleanup(int session_status)
{
	return session_status != 0;
}

/* Preserve the primary session failure while recording daemon rollback failure. */
static inline int
xl2tpd_start_policy_rpl2tp_failure_status(int session_status, int stop_status)
{
	return session_status != 0 ? session_status : stop_status;
}

/* WAN PPP treats an already-running shared daemon as a successful start. */
static inline int
xl2tpd_start_policy_wan_launch_result(int start_status)
{
	if (start_status == 0 || start_status == 1)
		return 0;
	return -1;
}
#endif /* XL2TPD_START_POLICY_H */
