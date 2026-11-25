/* SPDX-License-Identifier: GPL-2.0 */
#ifndef DHCP6C_IA_POLICY_H
#define DHCP6C_IA_POLICY_H
static inline int
dhcp6c_ia_pd_target_needs_apply(int is_dhcp_interface, int already_installed)
{
	return !is_dhcp_interface && !already_installed;
}
static inline int
dhcp6c_ia_first_error(int first_error, int next_error)
{
	return first_error != 0 ? first_error : next_error;
}
static inline int
dhcp6c_ia_retry_required(int status)
{
	return status != 0;
}
#endif /* DHCP6C_IA_POLICY_H */
