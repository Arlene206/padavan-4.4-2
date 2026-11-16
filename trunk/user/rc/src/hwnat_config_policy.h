#ifndef RC_HWNAT_CONFIG_POLICY_H
#define RC_HWNAT_CONFIG_POLICY_H

#include <errno.h>
#include <sys/wait.h>

/*
 * An enabled HW NAT module is only configured when the WAN VLAN lookup is
 * valid and the driver accepts the requested VID. Optional offload failures
 * are returned to the caller for status reporting; callers may still proceed
 * with ordinary software forwarding.
 */
static inline int
rc_hwnat_configure_policy(int allowed, int module_loaded, int wan_vid,
			  int (*set_wan_vid)(int))
{
	int status;

	if (!allowed)
		return 0;
	if (wan_vid < 0)
		return EIO;
	if (!module_loaded)
		return 0;
	if (!set_wan_vid)
		return EINVAL;

	status = set_wan_vid(wan_vid);
	if (status == 0)
		return 0;

	return status > 0 ? status : EIO;
}

/* system(3) returns a wait status, not the shell command's plain exit code. */
static inline int
rc_hwnat_command_status_policy(int wait_status, int saved_errno)
{
	if (wait_status == -1)
		return saved_errno > 0 ? saved_errno : EIO;
	if (!WIFEXITED(wait_status) || WEXITSTATUS(wait_status) != 0)
		return EIO;
	return 0;
}

/*
 * Apply independent per-radio and IPv6 debugfs settings only when the
 * optional HW NAT module is actually loaded. All writes are attempted and
 * the first failure is returned so a failed first radio does not leave the
 * second radio or IPv6 setting silently untouched.
 */
static inline int
rc_hwnat_configure_radio_policy(int allowed, int module_loaded,
				int hw_nat_mode, int ipv6_nat,
				const char *ifname_2g, const char *ifname_5g,
				int (*set_radio_registration)(const char *, int),
				int (*set_ipv6_nat)(int))
{
	int first_status = 0;
	int status;

	if (!allowed || !module_loaded)
		return 0;
	if (!ifname_2g || !ifname_2g[0] || !ifname_5g || !ifname_5g[0] ||
	    !set_radio_registration || !set_ipv6_nat)
		return EINVAL;

	status = set_radio_registration(ifname_2g, hw_nat_mode == 1);
	if (status != 0)
		first_status = status > 0 ? status : EIO;

	status = set_radio_registration(ifname_5g, hw_nat_mode == 1);
	if (first_status == 0 && status != 0)
		first_status = status > 0 ? status : EIO;

	status = set_ipv6_nat(ipv6_nat == 1);
	if (first_status == 0 && status != 0)
		first_status = status > 0 ? status : EIO;

	return first_status;
}

#endif /* RC_HWNAT_CONFIG_POLICY_H */
