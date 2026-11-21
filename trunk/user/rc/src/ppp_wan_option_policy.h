#ifndef PADAVAN_PPP_WAN_OPTION_POLICY_H
#define PADAVAN_PPP_WAN_OPTION_POLICY_H

#include <errno.h>
#include <string.h>

struct ppp_wan_option_values {
	int auth_type;
	int mtu;
	int mru;
	int pppoe_idletime;
	int pppoe_demand;
	int pppoe_txonly_x;
	int dnsenable_x;
	int mppe_mode;
	int lcp_echo;
	int adaptive_lcp;
};

typedef int (*ppp_wan_option_read_int_fn)(void *context,
					  const char *param_name,
					  int *value);

/*
 * Read every integer that affects one PPP option file before stopping a
 * daemon or opening the shared file. Missing NVRAM keys retain the legacy
 * zero default; a backend/parse error aborts the whole snapshot.
 */
static inline int
ppp_wan_option_values_read(int is_pppoe, int is_pptp, int is_l2tp,
			   ppp_wan_option_read_int_fn read_int,
			   void *context,
			   struct ppp_wan_option_values *values)
{
	int status;

	if (!values || !read_int ||
	    (is_pppoe != 0 && is_pppoe != 1) ||
	    (is_pptp != 0 && is_pptp != 1) ||
	    (is_l2tp != 0 && is_l2tp != 1) ||
	    is_pppoe + is_pptp + is_l2tp != 1)
		return EINVAL;

	memset(values, 0, sizeof(*values));
#define PPP_WAN_READ_OPTION(name, member) \
	do { \
		status = read_int(context, name, &values->member); \
		if (status != 0) \
			goto failed; \
	} while (0)

	PPP_WAN_READ_OPTION("ppp_auth", auth_type);
	if (is_pppoe) {
		PPP_WAN_READ_OPTION("pppoe_mtu", mtu);
		PPP_WAN_READ_OPTION("pppoe_mru", mru);
		PPP_WAN_READ_OPTION("pppoe_idletime", pppoe_idletime);
		PPP_WAN_READ_OPTION("pppoe_demand", pppoe_demand);
		PPP_WAN_READ_OPTION("pppoe_txonly_x", pppoe_txonly_x);
	} else if (is_pptp) {
		PPP_WAN_READ_OPTION("pptp_mtu", mtu);
		PPP_WAN_READ_OPTION("pptp_mru", mru);
	} else {
		PPP_WAN_READ_OPTION("l2tp_mtu", mtu);
		PPP_WAN_READ_OPTION("l2tp_mru", mru);
	}

	PPP_WAN_READ_OPTION("dnsenable_x", dnsenable_x);
	if (!is_pppoe)
		PPP_WAN_READ_OPTION("ppp_mppe", mppe_mode);
	PPP_WAN_READ_OPTION("ppp_lcp", lcp_echo);
	PPP_WAN_READ_OPTION("ppp_alcp", adaptive_lcp);

#undef PPP_WAN_READ_OPTION
	return 0;

failed:
	memset(values, 0, sizeof(*values));
	return status > 0 ? status : EIO;
}

#endif /* PADAVAN_PPP_WAN_OPTION_POLICY_H */
