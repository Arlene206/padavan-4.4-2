#ifndef IAPP_RX_LENGTH_POLICY_H
#define IAPP_RX_LENGTH_POLICY_H

#include <stddef.h>

/* Validate a declared payload against received bytes without addition wrap. */
static inline int
iapp_rx_payload_fits(size_t received_size, size_t fixed_size, size_t payload_size)
{
	return received_size >= fixed_size &&
	       payload_size <= received_size - fixed_size;
}

#endif /* IAPP_RX_LENGTH_POLICY_H */
