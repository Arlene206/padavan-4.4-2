/* SPDX-License-Identifier: GPL-2.0 */
#ifndef DHCP6C_SCRIPT_RETRY_POLICY_H
#define DHCP6C_SCRIPT_RETRY_POLICY_H

/* Avoid tight callback-failure loops while retaining the DHCP event. */
#define DHCP6C_SCRIPT_RETRY_DELAY_MS 5000ULL

static inline int dhcp6c_script_retry_required(int script_status)
{
	return script_status != 0;
}

#endif /* DHCP6C_SCRIPT_RETRY_POLICY_H */
