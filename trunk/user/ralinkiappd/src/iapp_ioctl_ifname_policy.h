#ifndef IAPP_IOCTL_IFNAME_POLICY_H
#define IAPP_IOCTL_IFNAME_POLICY_H

#include <stddef.h>
#include <string.h>

/*
 * Validate argv -wi values before storing them in the fixed interface array.
 * The active count is also the next write index, so both bounds must hold.
 */
static inline int
iapp_ifname_arg_valid(size_t name_length, size_t slot_size)
{
	return name_length > 0 && name_length < slot_size;
}

static inline int
iapp_ioctl_ifname_arg_valid(int configured_count, size_t capacity,
                           size_t name_length, size_t slot_size)
{
	return configured_count >= 0 &&
	       (size_t)configured_count < capacity &&
	       iapp_ifname_arg_valid(name_length, slot_size);
}

/*
 * Return a validated configured name, or the safe fallback. The index and
 * configured-count checks deliberately precede pointer arithmetic and any
 * read from the fixed-width slot. memchr bounds the NUL search to one slot.
 */
static inline const char *
iapp_ioctl_ifname_select(const char *slots, size_t capacity, size_t slot_size,
                         int configured_count, int requested_index,
                         size_t output_size, const char *fallback)
{
	const char *candidate;
	const char *terminator;

	if (!slots || !fallback || capacity == 0 || slot_size == 0 ||
	    output_size == 0 || configured_count <= 0 || requested_index < 0 ||
	    requested_index >= configured_count ||
	    (size_t)requested_index >= capacity)
		return fallback;

	candidate = slots + (size_t)requested_index * slot_size;
	terminator = (const char *)memchr(candidate, '\0', slot_size);
	if (!terminator || terminator == candidate ||
	    (size_t)(terminator - candidate) >= output_size)
		return fallback;

	return candidate;
}

#endif /* IAPP_IOCTL_IFNAME_POLICY_H */
