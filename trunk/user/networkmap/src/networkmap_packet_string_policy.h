#ifndef NETWORKMAP_PACKET_STRING_POLICY_H
#define NETWORKMAP_PACKET_STRING_POLICY_H

#include <stddef.h>

/* Fixed-width packet strings must terminate inside their own field. */
static inline int
networkmap_fixed_field_is_nul_terminated(const unsigned char *field,
					 size_t field_size)
{
	size_t i;

	if (!field || field_size == 0U)
		return 0;

	for (i = 0; i < field_size; ++i) {
		if (field[i] == '\0')
			return 1;
	}

	return 0;
}

#endif /* NETWORKMAP_PACKET_STRING_POLICY_H */
