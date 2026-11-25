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

#endif /* DHCP6C_OPTION_BOUNDS_POLICY_H */
