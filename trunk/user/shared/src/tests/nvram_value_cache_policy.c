#include <assert.h>
#include <string.h>

#include "nvram_value_cache_policy.h"

static void
test_full_cache_is_erased_and_cursor_reset(void)
{
	char cache[128];
	int next_index = 97;
	size_t i;

	memset(cache, 0xa5, sizeof(cache));
	strcpy(cache + 8, "vpn-secret");
	nvram_value_cache_secure_clear(cache, sizeof(cache), &next_index);

	assert(next_index == 0);
	for (i = 0; i < sizeof(cache); ++i)
		assert((unsigned char)cache[i] == 0);
}

static void
test_null_cache_still_resets_cursor(void)
{
	int next_index = 41;

	nvram_value_cache_secure_clear(NULL, 128, &next_index);
	assert(next_index == 0);
	nvram_value_cache_secure_clear(NULL, 0, NULL);
}

int
main(void)
{
	test_full_cache_is_erased_and_cursor_reset();
	test_null_cache_still_resets_cursor();
	return 0;
}
