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
		/* A zero -A value must not turn repeated callback failures into a
		 * tight DHCPREQUEST loop. Match deconfig retry's one-second floor. */
		*timeout = retry_timeout > 0 ? retry_timeout : 1;
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

/* arpping(): zero means conflict, one means clear, negative means I/O failure. */
static inline int udhcpc_arp_probe_failed(int probe_status)
{
	return probe_status < 0;
}

/* A nonzero event-script status means that lease cleanup is incomplete. */
static inline int udhcpc_deconfig_retry_required(int script_status)
{
	return script_status != 0;
}

/* A pending deconfig must still honor SIGUSR2 for a previously held lease. */
static inline int
udhcpc_deconfig_pending_release_required(int lease_was_active,
					 int release_requested)
{
	return lease_was_active != 0 && release_requested != 0;
}


/* sendto()/safe_write() return a byte count on success and -1 on failure. */
static inline int
udhcpc_packet_send_failed(int send_status)
{
	return send_status < 0;
}

/* Keep retries bounded away from a busy loop when -A is zero or negative. */
static inline int
udhcpc_decline_retry_delay(int retry_timeout)
{
	return retry_timeout > 0 ? retry_timeout : 1;
}


/* A DHCPNAK during initial REQUESTING does not invalidate a held lease. */
static inline int
udhcpc_nak_requires_deconfig(int requesting)
{
	return requesting == 0;
}

#endif /* UDHCP_SCRIPT_POLICY_H */

