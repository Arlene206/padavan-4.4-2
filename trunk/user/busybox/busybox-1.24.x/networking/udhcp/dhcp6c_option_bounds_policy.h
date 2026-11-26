/* SPDX-License-Identifier: GPL-2.0 */
#ifndef DHCP6C_OPTION_BOUNDS_POLICY_H
#define DHCP6C_OPTION_BOUNDS_POLICY_H

#include <stddef.h>

/*
 * Validate a wire-format option using lengths, before computing a pointer to
 * its payload end. Subtraction avoids overflow in header + payload checks.
 */
static inline int
dhcp6c_option_wire_length_valid(size_t remaining, size_t header_size,
			       size_t payload_size)
{
	return header_size <= remaining &&
	       payload_size <= remaining - header_size;
}

/*
 * recvmsg(MSG_TRUNC) reports the original UDP datagram size on Linux. Reject
 * datagrams that cannot fit in the receive buffer before parsing any fields.
 */
static inline int
dhcp6c_datagram_length_fits(size_t received_size, size_t buffer_size)
{
	return received_size <= buffer_size;
}

/*
 * DHCP6 Reconfigure Accept carries no payload. Reject malformed non-empty
 * instances even though this client does not otherwise implement the feature.
 */
static inline int
dhcp6c_option_zero_payload_length_valid(size_t payload_size)
{
	return payload_size == 0;
}

/*
 * Client/server IDs, relay messages and interface IDs each have one vbuf slot
 * in dhcp6_optinfo. Reject duplicates instead of overwriting an owned buffer.
 */
static inline int
dhcp6c_option_vbuf_is_unset(const void *buffer, size_t length)
{
	return buffer == NULL && length == 0;
}

#endif /* DHCP6C_OPTION_BOUNDS_POLICY_H */
