/* SPDX-License-Identifier: GPL-2.0 */
#ifndef UDHCP_SCRIPT_POLICY_H
#define UDHCP_SCRIPT_POLICY_H

#include <stdint.h>

/*
 * EINTR can consume the full timeout. Saturate before subtracting because
 * timeout is signed while already_waited_sec is unsigned.
 */
static inline unsigned int
udhcpc_wait_remaining_seconds(int timeout, unsigned int already_waited_sec)
{
	if (timeout <= 0 || already_waited_sec >= (unsigned int)timeout)
		return 0U;
	return (unsigned int)timeout - already_waited_sec;
}

/* A missing active receive socket must be retried without expiring the lease. */
#define UDHCP_LISTEN_SOCKET_RETRY_INTERVAL_SECONDS 3U

static inline int
udhcpc_listen_socket_reopen_required(int listen_mode, int listen_none,
				     int socket_fd)
{
	return listen_mode != listen_none && socket_fd < 0;
}

static inline unsigned int
udhcpc_listen_socket_wait_seconds(int timeout, unsigned int already_waited_sec,
				  int listen_mode, int listen_none,
				  int socket_fd)
{
	unsigned int remaining = udhcpc_wait_remaining_seconds(
		timeout, already_waited_sec);

	if (udhcpc_listen_socket_reopen_required(
		    listen_mode, listen_none, socket_fd) &&
	    remaining > UDHCP_LISTEN_SOCKET_RETRY_INTERVAL_SECONDS)
		return UDHCP_LISTEN_SOCKET_RETRY_INTERVAL_SECONDS;
	return remaining;
}

/* The IPv4 subnet mask must be contiguous, in host byte order. */
static inline int
udhcpc_subnet_mask_prefix_length(uint32_t mask, unsigned int *prefix_length)
{
	unsigned int prefix = 0;
	int bit, saw_zero = 0;

	if (!prefix_length)
		return -1;
	*prefix_length = 0;
	for (bit = 31; bit >= 0; --bit) {
		if (mask & (1U << bit)) {
			if (saw_zero)
				return -1;
			++prefix;
		} else {
			saw_zero = 1;
		}
	}
	*prefix_length = prefix;
	return 0;
}

/* Route lists with fixed-width address tuples must not contain a
 * partial final item; the generic formatter otherwise silently drops it. */
static inline int
udhcpc_fixed_route_list_option_is_well_formed(const uint8_t *option,
					      unsigned int length,
					      unsigned int tuple_length)
{
	if (tuple_length == 0U || (!option && length != 0U))
		return 0;
	return length % tuple_length == 0U;
}

/* DHCP classless-route options are a sequence of prefix + router tuples.
 * A truncated or invalid trailing tuple must not be silently discarded by
 * xmalloc_optname_optval(), which otherwise returns the valid prefix. */
static inline int
udhcpc_static_routes_option_is_well_formed(const uint8_t *option,
					   unsigned int length)
{
	unsigned int mask, address_bytes;

	if (!option && length != 0U)
		return 0;

	while (length != 0U) {
		if (length < 5U)
			return 0;

		mask = *option++;
		if (mask > 32U)
			return 0;
		--length;

		address_bytes = (mask + 7U) / 8U;
		if (length < address_bytes + 4U)
			return 0;

		option += address_bytes + 4U;
		length -= address_bytes + 4U;
	}

	return 1;
}

/* RFC 5969 option 0x96/0xd4: two length bytes, a 16-byte prefix,
 * and one or more four-byte border-relay IPv4 addresses. */
static inline int
udhcpc_6rd_option_is_well_formed(const uint8_t *option,
				unsigned int length)
{
	unsigned int ipv4_mask_length, prefix_length;

	if (!option || length < 22U)
		return 0;

	ipv4_mask_length = option[0];
	prefix_length = option[1];
	if (ipv4_mask_length > 32U || prefix_length > 128U ||
	    prefix_length + (32U - ipv4_mask_length) > 128U)
		return 0;

	return ((length - 22U) % 4U) == 0U;
}

/* Use an unsigned left operand: option codes can select the top bitmap bit. */
static inline unsigned int
udhcpc_option_bitmap_mask(unsigned int option_code)
{
	return 1U << (option_code % (sizeof(unsigned int) * 8U));
}

/* Keep the exported DHCP lease value identical to the client's lease timer. */
#define UDHCP_LEASE_TIME_DEFAULT_SECONDS 3600U
#define UDHCP_LEASE_TIME_MIN_SECONDS 0x10U
#define UDHCP_LEASE_TIME_MAX_SECONDS 0x0fffffffU

static inline uint32_t
udhcpc_lease_time_effective(uint32_t lease_seconds, int option_valid)
{
	if (!option_valid)
		return UDHCP_LEASE_TIME_DEFAULT_SECONDS;
	if (lease_seconds < UDHCP_LEASE_TIME_MIN_SECONDS)
		return UDHCP_LEASE_TIME_MIN_SECONDS;
	if (lease_seconds >= 0x10000000U)
		return UDHCP_LEASE_TIME_MAX_SECONDS;
	return lease_seconds;
}

/*
 * The bound/renew script runs after a DHCPACK but consumes lease time. Before
 * T1, charge its runtime against the initial wait. At/after T1, begin renewal
 * immediately but retain only the actual lease time left for retries. Never
 * treat "script took longer than T1" as proof that the full lease expired.
 *
 * Return 1 when the lease has expired before the callback completed, 0 when
 * it remains valid, and -1 for invalid output pointers.
 */
static inline int
udhcpc_lease_script_plan(uint32_t lease_seconds, unsigned script_elapsed,
			 int *timeout, unsigned *already_waited_sec)
{
	unsigned int renew_at, remaining;

	if (!timeout || !already_waited_sec)
		return -1;
	*timeout = 0;
	*already_waited_sec = 0;
	if (lease_seconds == 0U || script_elapsed >= lease_seconds)
		return 1;

	renew_at = lease_seconds / 2U;
	if (renew_at == 0U)
		renew_at = 1U;
	if (script_elapsed < renew_at) {
		*timeout = (int)renew_at;
		*already_waited_sec = script_elapsed;
	} else {
		remaining = lease_seconds - script_elapsed;
		*timeout = (int)remaining;
		*already_waited_sec = remaining;
	}
	return 0;
}

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

/*
 * A failed initial bound callback may have partially configured the interface.
 * Renew/rebind callback failure must retain the currently active lease.
 */
static inline int
udhcpc_bound_script_failure_requires_deconfig(int acquiring)
{
	return acquiring != 0;
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
 * Outside REBINDING, accept an ACK or NAK only from the server selected for
 * the request. REBINDING DHCPREQUESTs are broadcast to any server, so another
 * server may legitimately renew or invalidate the current address. RFC 2131
 * requires the server to identify itself in the server-ID option. Preserve
 * the legacy address-less mode when no server address was selected: an
 * explicit 0.0.0.0 then behaves like a missing option. Once a server is pinned
 * or while REBINDING, zero is invalid. Limited-broadcast
 * (255.255.255.255) is never a usable server endpoint.
 */
static inline int
udhcpc_server_id_rejected(int rebinding,
			  uint32_t expected_server_id,
			  int server_id_present,
			  uint32_t received_server_id)
{
	if (server_id_present &&
	    (received_server_id == 0xffffffffU ||
	     (received_server_id == 0U &&
	      (rebinding || expected_server_id != 0U))))
		return 1;
	if (rebinding)
		return !server_id_present;

	return expected_server_id != 0 &&
	       (!server_id_present || received_server_id != expected_server_id);
}

/*
 * A server identifier is exactly one IPv4 address. A true absence remains
 * compatible with address-less DHCP servers; a present empty/short/long
 * value, or an option-parser error, must not be treated as absence.
 * Lookup status: -1 malformed, 0 absent, 1 present.
 */
static inline int
udhcpc_server_id_option_status_valid(int lookup_status,
                                     unsigned int option_length)
{
	return lookup_status == 0 ? option_length == 0U :
		lookup_status == 1 && option_length == sizeof(uint32_t);
}

/* Keep the existing NAK-specific callsite name while sharing the policy. */
static inline int
udhcpc_nak_server_id_rejected(int rebinding,
			      uint32_t expected_server_id,
			      int server_id_present,
			      uint32_t received_server_id)
{
	return udhcpc_server_id_rejected(rebinding, expected_server_id,
					 server_id_present, received_server_id);
}

/*
 * A REBINDING ACK may be issued by any server. Once the event callback has
 * committed that lease, use its server identifier for future unicast renewals
 * and releases. Also learn an identifier when an address-less offer had none.
 */
static inline uint32_t
udhcpc_server_addr_after_ack(uint32_t current_server_addr, int rebinding,
			     int server_id_present, uint32_t received_server_id)
{
	if (server_id_present && (rebinding || current_server_addr == 0U))
		return received_server_id;
	return current_server_addr;
}

#endif /* UDHCP_SCRIPT_POLICY_H */

