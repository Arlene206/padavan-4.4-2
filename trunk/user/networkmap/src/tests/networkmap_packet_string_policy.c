#include <stdio.h>
#include <string.h>

#include "networkmap_packet_string_policy.h"

#define CHECK(condition) do { \
	if (!(condition)) { \
		fprintf(stderr, "%s:%d: check failed: %s\n", __FILE__, __LINE__, #condition); \
		return 1; \
	} \
} while (0)

int
main(void)
{
	unsigned char product_id[32];
	unsigned char adjacent_field[33];

	memset(product_id, 'A', sizeof(product_id));
	CHECK(!networkmap_fixed_field_is_nul_terminated(product_id,
							 sizeof(product_id)));

	product_id[sizeof(product_id) - 1U] = '\0';
	CHECK(networkmap_fixed_field_is_nul_terminated(product_id,
						       sizeof(product_id)));

	product_id[0] = '\0';
	CHECK(networkmap_fixed_field_is_nul_terminated(product_id,
						       sizeof(product_id)));

	memset(adjacent_field, 'B', sizeof(adjacent_field));
	adjacent_field[sizeof(product_id)] = '\0';
	CHECK(!networkmap_fixed_field_is_nul_terminated(adjacent_field,
							 sizeof(product_id)));
	CHECK(networkmap_fixed_field_is_nul_terminated(adjacent_field,
						       sizeof(adjacent_field)));

	CHECK(!networkmap_fixed_field_is_nul_terminated(NULL, sizeof(product_id)));
	CHECK(!networkmap_fixed_field_is_nul_terminated(product_id, 0U));

	puts("networkmap fixed-width packet string bounds: PASS");
	return 0;
}
