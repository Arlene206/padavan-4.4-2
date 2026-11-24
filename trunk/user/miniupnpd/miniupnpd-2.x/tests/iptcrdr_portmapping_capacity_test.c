#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

#include "../netfilter/iptcrdr_capacity_policy.h"

static void
test_capacity_boundaries(void)
{
	size_t next = 17;

	assert(miniupnpd_portmapping_capacity_next(128, &next) == 0);
	assert(next == 256);

	next = 17;
	errno = 0;
	assert(miniupnpd_portmapping_capacity_next(0, &next) == -1);
	assert(errno == EOVERFLOW);
	assert(next == 17);

	errno = 0;
	assert(miniupnpd_portmapping_capacity_next(UINT_MAX, &next) == -1);
	assert(errno == EOVERFLOW);
	assert(next == 17);

	errno = 0;
	assert(miniupnpd_portmapping_capacity_next(UINT_MAX / 2 + 1, &next) == -1);
	assert(errno == EOVERFLOW);
	assert(next == 17);

	errno = 0;
	assert(miniupnpd_portmapping_capacity_next(128, NULL) == -1);
	assert(errno == EOVERFLOW);
}

static void
test_growth_past_original_128_entries(void)
{
	unsigned short *array;
	unsigned short *grown;
	unsigned int capacity = 128;
	unsigned int count;
	size_t next_capacity;

	array = calloc(capacity, sizeof(*array));
	assert(array != NULL);

	for (count = 0; count < 4096; count++) {
		if (count >= capacity) {
			assert(miniupnpd_portmapping_capacity_next(
				capacity, &next_capacity) == 0);
			grown = realloc(array, sizeof(*array) * next_capacity);
			assert(grown != NULL);
			array = grown;
			capacity = (unsigned int)next_capacity;
		}
		array[count] = (unsigned short)(count + 1);
	}

	assert(capacity >= 4096);
	for (count = 0; count < 4096; count++)
		assert(array[count] == (unsigned short)(count + 1));

	free(array);
}

int
main(void)
{
	test_capacity_boundaries();
	test_growth_past_original_128_entries();
	puts("MiniUPnP port-mapping array capacity policy: PASS");
	return 0;
}
