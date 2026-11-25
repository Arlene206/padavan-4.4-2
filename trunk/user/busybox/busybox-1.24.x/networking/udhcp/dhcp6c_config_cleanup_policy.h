/* SPDX-License-Identifier: GPL-2.0 */
#ifndef DHCP6C_CONFIG_CLEANUP_POLICY_H
#define DHCP6C_CONFIG_CLEANUP_POLICY_H

typedef void (*dhcp6c_config_list_cleanup_fn)(void *context);

/* IA_NA and IA_PD own prefix_list entries; keep their cleanup symmetric. */
static inline void
dhcp6c_config_cleanup_prefix_list(int ia_type, int ia_na_type,
                                  int ia_pd_type, void *prefix_list,
                                  dhcp6c_config_list_cleanup_fn cleanup)
{
	if (!prefix_list || !cleanup)
		return;

	if (ia_type == ia_na_type || ia_type == ia_pd_type)
		cleanup(prefix_list);
}

#endif /* DHCP6C_CONFIG_CLEANUP_POLICY_H */
