/* SPDX-License-Identifier: GPL-2.0 */
#ifndef UDHCP_SCRIPT_POLICY_H
#define UDHCP_SCRIPT_POLICY_H

#include <stdint.h>

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

/* Shared one-second floor for every DHCP cleanup/retry path. */
static inline int
udhcpc_retry_delay(int retry_timeout)
{
	return retry_timeout > 0 ? retry_timeout : 1;
}

/*
 * Prepare one DECONFIG_PENDING transition before publishing it to the event
 * loop. Keeping the state and timer fields together makes the retry contract
 * host-testable without emulating BusyBox sockets or signals.
 */
struct udhcpc_deconfig_retry_plan {
	int resume_state;
	int release_on_usr2;
	int timeout;
	int packet_num;
	unsigned already_waited_sec;
};

static inline void
udhcpc_deconfig_retry_plan_init(struct udhcpc_deconfig_retry_plan *plan,
				int resume_state, int release_on_usr2,
				int retry_timeout)
{
	plan->resume_state = resume_state;
	plan->release_on_usr2 = release_on_usr2 != 0;
	plan->timeout = udhcpc_retry_delay(retry_timeout);
	plan->packet_num = 0;
	plan->already_waited_sec = 0;
}

/* Keep retries bounded away from a busy loop when -A is zero or negative. */
static inline int
udhcpc_decline_retry_delay(int retry_timeout)
{
	return udhcpc_retry_delay(retry_timeout);
}


/* A DHCPNAK during initial REQUESTING does not invalidate a held lease. */
static inline int
udhcpc_nak_requires_deconfig(int requesting)
{
	return requesting == 0;
}

/*
 * Outside REBINDING, accept a NAK only from the server selected for the
 * request. REBINDING DHCPREQUESTs are broadcast to any server, so a different
 * server may authoritatively NAK an address that is no longer valid here.
 * RFC 2131 also requires a server to identify itself in the server-ID option.
 */
static inline int
udhcpc_nak_server_id_rejected(int rebinding,
			      uint32_t expected_server_id,
			      int server_id_present,
			      uint32_t received_server_id)
{
	if (rebinding)
		return !server_id_present;

	return expected_server_id != 0 &&
	       (!server_id_present || received_server_id != expected_server_id);
}

#endif /* UDHCP_SCRIPT_POLICY_H */

