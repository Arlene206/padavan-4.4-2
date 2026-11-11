/* SPDX-License-Identifier: GPL-2.0 */
#ifndef UDHCP_SCRIPT_POLICY_H
#define UDHCP_SCRIPT_POLICY_H

/*
 * A failed lease event must not be published as a successful DHCP bind.
 * During initial acquisition, retry the same selected request after the
 * configured retry interval. During renewal, retain the existing lease
 * deadline and charge the script runtime against it.
 */
static inline void udhcpc_script_failure_prepare_retry(
	int acquiring,
	int retry_timeout,
	unsigned script_elapsed,
	int *timeout,
	unsigned *already_waited_sec,
	int *packet_num)
{
	unsigned remaining;

	if (acquiring) {
		*timeout = retry_timeout;
		*already_waited_sec = 0;
		*packet_num = 0;
		return;
	}

	if (*timeout <= 0 || *already_waited_sec >= (unsigned)*timeout) {
		*timeout = 0;
		*already_waited_sec = 0;
		return;
	}

	remaining = (unsigned)*timeout - *already_waited_sec;
	if (script_elapsed >= remaining) {
		*timeout = 0;
		*already_waited_sec = 0;
		return;
	}

	*already_waited_sec += script_elapsed;
}

#endif /* UDHCP_SCRIPT_POLICY_H */
