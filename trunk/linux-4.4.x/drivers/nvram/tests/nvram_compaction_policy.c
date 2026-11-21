#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "nvram_compaction_policy.h"

static void test_compaction_value_spans_are_bounded(void)
{
	char pool[] = { 'x', 'a', 'b', '\0', '\0', 'z', '\0' };
	size_t length = 99;

	assert(nvram_compaction_value_length(pool, sizeof(pool), 1, &length) == 0);
	assert(length == 2);
	assert(nvram_compaction_value_length(pool, sizeof(pool), 4, &length) == 0);
	assert(length == 0);
	assert(nvram_compaction_value_length(pool, sizeof(pool), 5, &length) == 0);
	assert(length == 1);
	assert(nvram_compaction_value_length(pool, sizeof(pool), sizeof(pool),
					     &length) != 0);
}

static void test_missing_terminator_is_rejected(void)
{
	char unterminated[] = { 'n', 'o', 'n', 'u', 'l' };
	size_t length = 0;

	assert(nvram_compaction_value_length(unterminated,
					     sizeof(unterminated), 0,
					     &length) != 0);
}

static void test_compaction_destination_boundaries(void)
{
	assert(nvram_compaction_fits(16, 12, 3));
	assert(!nvram_compaction_fits(16, 13, 3));
	assert(!nvram_compaction_fits(16, 17, 0));
	assert(!nvram_compaction_fits(16, 0, (size_t)-1));
}

static void test_flash_entry_size_fails_closed(void)
{
	size_t entry_length = 77;

	assert(nvram_generate_entry_size(9, 3, 4, &entry_length));
	assert(entry_length == 9);
	assert(!nvram_generate_entry_size(8, 3, 4, &entry_length));
	assert(entry_length == 9);
	assert(!nvram_generate_entry_size(64, (size_t)-2, 0, &entry_length));
	assert(!nvram_generate_entry_size(64, 1, (size_t)-1, &entry_length));
	assert(!nvram_generate_entry_size(64, 1, 1, NULL));
}

int main(void)
{
	test_compaction_value_spans_are_bounded();
	test_missing_terminator_is_rejected();
	test_compaction_destination_boundaries();
	test_flash_entry_size_fails_closed();
	puts("NVRAM compaction and flash-capacity policy: PASS");
	return 0;
}
