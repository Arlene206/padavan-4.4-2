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

static void
test_reserve_keeps_the_current_lap_when_space_is_sufficient(void)
{
	char cache[128];
	int next_index = 64;
	int offset;
	size_t i;

	memset(cache, 0xa5, sizeof(cache));
	offset = nvram_value_cache_reserve(cache, sizeof(cache), 32, 24,
					   &next_index);
	assert(offset == 64);
	assert(next_index == 64);
	for (i = 0; i < sizeof(cache); ++i)
		assert((unsigned char)cache[i] == 0xa5);
}

static void
test_reserve_erases_the_previous_lap_at_the_minimum_wrap_boundary(void)
{
	char cache[128];
	int next_index = 100;
	int offset;
	size_t i;

	memset(cache, 0xa5, sizeof(cache));
	memcpy(cache + 8, "vpn-secret", sizeof("vpn-secret"));
	offset = nvram_value_cache_reserve(cache, sizeof(cache), 32, 0,
					   &next_index);
	assert(offset == 0);
	assert(next_index == 0);
	for (i = 0; i < sizeof(cache); ++i)
		assert((unsigned char)cache[i] == 0);
}

static void
test_reserve_erases_the_previous_lap_when_a_value_will_not_fit(void)
{
	char cache[128];
	int next_index = 80;
	int offset;
	size_t i;

	memset(cache, 0xa5, sizeof(cache));
	memcpy(cache + 4, "wifi-secret", sizeof("wifi-secret"));
	offset = nvram_value_cache_reserve(cache, sizeof(cache), 32, 64,
					   &next_index);
	assert(offset == 0);
	assert(next_index == 0);
	for (i = 0; i < sizeof(cache); ++i)
		assert((unsigned char)cache[i] == 0);
}

static void
test_reserve_rejects_invalid_cursor_without_erasing(void)
{
	char cache[128];
	int next_index = -1;
	size_t i;

	memset(cache, 0xa5, sizeof(cache));
	assert(nvram_value_cache_reserve(cache, sizeof(cache), 32, 0,
					 &next_index) == -1);
	assert(next_index == -1);
	for (i = 0; i < sizeof(cache); ++i)
		assert((unsigned char)cache[i] == 0xa5);

	next_index = 129;
	assert(nvram_value_cache_reserve(cache, sizeof(cache), 32, 0,
					 &next_index) == -1);
	assert(next_index == 129);
}
int
main(void)
{
	test_full_cache_is_erased_and_cursor_reset();
	test_null_cache_still_resets_cursor();
	test_reserve_keeps_the_current_lap_when_space_is_sufficient();
	test_reserve_erases_the_previous_lap_at_the_minimum_wrap_boundary();
	test_reserve_erases_the_previous_lap_when_a_value_will_not_fit();
	test_reserve_rejects_invalid_cursor_without_erasing();
	return 0;
}
