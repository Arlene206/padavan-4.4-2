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

static inline int
iapp_rx_buffer_append_fits(size_t capacity, size_t current_size, size_t append_size)
{
	return current_size <= capacity &&
	       append_size <= capacity - current_size;
}

static inline int
iapp_rx_frame_length_matches(size_t received_size, size_t declared_size)
{
	return received_size == declared_size;
}

static inline int
iapp_rx_length_within_capacity(size_t capacity, int returned_length)
{
	return returned_length >= 0 &&
	       (size_t)returned_length <= capacity;
}

#endif /* IAPP_RX_LENGTH_POLICY_H */
