/* SPDX-License-Identifier: GPL-2.0 */
#ifndef DHCP6C_SCRIPT_RETRY_POLICY_H
#define DHCP6C_SCRIPT_RETRY_POLICY_H

#include "dhcp_script_env_policy.h"

typedef dhcp_script_env_put_fn dhcp6c_script_env_put_fn;
typedef dhcp_script_env_unset_fn dhcp6c_script_env_unset_fn;

static inline int
dhcp6c_script_env_publish(char **envp,
			  dhcp6c_script_env_put_fn publish,
			  dhcp6c_script_env_unset_fn unset_value)
{
	return dhcp_script_env_publish(envp, publish, unset_value);
}

/* Return a non-wrapping remaining interval for an absolute monotonic deadline. */
static inline unsigned long long
dhcp6_timer_remaining_ms(unsigned long long deadline, unsigned long long now)
{
	return deadline > now ? deadline - now : 0ULL;
}

/* Avoid tight callback-failure loops while retaining the DHCP event. */
#define DHCP6C_SCRIPT_RETRY_DELAY_MS 5000ULL

static inline int dhcp6c_script_retry_required(int script_status)
{
	return script_status != 0;
}

#endif /* DHCP6C_SCRIPT_RETRY_POLICY_H */
