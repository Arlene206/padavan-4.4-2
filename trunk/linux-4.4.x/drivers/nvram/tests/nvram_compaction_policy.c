#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

static unsigned int sensitive_zero_calls;
static size_t sensitive_zero_bytes;

static void sensitive_test_zero(void *buffer, size_t length)
{
	volatile unsigned char *cursor = (volatile unsigned char *)buffer;
	size_t index;
	++sensitive_zero_calls;
	sensitive_zero_bytes += length;
	for (index = 0; index < length; ++index)
		cursor[index] = 0;
}

#define NVRAM_SENSITIVE_BUFFER_ZERO(buffer, length) \
	sensitive_test_zero((buffer), (length))
#include "nvram_sensitive_buffer_policy.h"
#include "nvram_compaction_policy.h"

static void test_sensitive_value_tail_clear_removes_shortened_secret(void)
{
	char value[] = "wifi-password-material";
	size_t value_capacity = sizeof(value) - 1;
	size_t index;

	/* Simulate strcpy() into a reused, formerly longer arena slot. */
	strcpy(value, "x");
	sensitive_zero_calls = 0;
	sensitive_zero_bytes = 0;
	nvram_sensitive_value_clear_tail(value, value_capacity, 1);
	assert(sensitive_zero_calls == 1);
	assert(sensitive_zero_bytes == value_capacity - 1);
	assert(value[0] == 'x' && value[1] == '\0');
	for (index = 2; index < sizeof(value); ++index)
		assert(value[index] == 0);
}

static void test_sensitive_value_clear_erases_released_slot(void)
{
	char value[] = "vpn-shared-secret";
	size_t value_capacity = sizeof(value) - 1;
	size_t index;

	sensitive_zero_calls = 0;
	sensitive_zero_bytes = 0;
	nvram_sensitive_value_clear(value, value_capacity);
	assert(sensitive_zero_calls == 1);
	assert(sensitive_zero_bytes == sizeof(value));
	for (index = 0; index < sizeof(value); ++index)
		assert(value[index] == 0);
}

static void test_sensitive_value_clear_guards_invalid_ranges(void)
{
	char value[] = "keep";
	sensitive_zero_calls = 0;
	sensitive_zero_bytes = 0;
	nvram_sensitive_value_clear(NULL, 3);
	nvram_sensitive_value_clear(value, (size_t)-1);
	nvram_sensitive_value_clear_tail(NULL, 3, 1);
	nvram_sensitive_value_clear_tail(value, 3, 3);
	nvram_sensitive_value_clear_tail(value, 3, 4);
	assert(sensitive_zero_calls == 0);
	assert(sensitive_zero_bytes == 0);
	assert(strcmp(value, "keep") == 0);
}


static void test_sensitive_buffer_clear_wipes_the_complete_region(void)
{
	unsigned char secret[] = "wireless-key=secret-material";
	size_t index;
	sensitive_zero_calls = 0;
	sensitive_zero_bytes = 0;
	nvram_sensitive_buffer_clear(secret, sizeof(secret));
	assert(sensitive_zero_calls == 1);
	assert(sensitive_zero_bytes == sizeof(secret));
	for (index = 0; index < sizeof(secret); ++index)
		assert(secret[index] == 0);
}

static void test_sensitive_buffer_clear_ignores_null_and_empty_regions(void)
{
	unsigned char secret[] = "unchanged";
	sensitive_zero_calls = 0;
	sensitive_zero_bytes = 0;
	nvram_sensitive_buffer_clear(NULL, sizeof(secret));
	nvram_sensitive_buffer_clear(secret, 0);
	assert(sensitive_zero_calls == 0);
	assert(sensitive_zero_bytes == 0);
	assert(strcmp((const char *)secret, "unchanged") == 0);
}
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


static void test_flash_rehash_is_bounded_by_declared_image_length(void)
{
	size_t end_offset = 77;

	/* The 20-byte header alone cannot contain the required double NUL. */
	assert(!nvram_flash_data_end_offset(20, 20, 256, &end_offset));
	assert(!nvram_flash_data_end_offset(21, 20, 256, &end_offset));

	/* The end offset is relative to the validated image, not full flash. */
	assert(nvram_flash_data_end_offset(22, 20, 256, &end_offset));
	assert(end_offset == 20);
	assert(nvram_flash_data_end_offset(24, 20, 256, &end_offset));
	assert(end_offset == 22);
	assert(nvram_flash_data_end_offset(256, 20, 256, &end_offset));
	assert(end_offset == 254);

	assert(!nvram_flash_data_end_offset(257, 20, 256, &end_offset));
	assert(!nvram_flash_data_end_offset((size_t)-1, 20, 256, &end_offset));
	assert(!nvram_flash_data_end_offset(24, 20, 256, NULL));
}

int main(void)
{
	test_sensitive_buffer_clear_wipes_the_complete_region();
	test_sensitive_buffer_clear_ignores_null_and_empty_regions();
	test_sensitive_value_tail_clear_removes_shortened_secret();
	test_sensitive_value_clear_erases_released_slot();
	test_sensitive_value_clear_guards_invalid_ranges();
	test_compaction_value_spans_are_bounded();
	test_missing_terminator_is_rejected();
	test_compaction_destination_boundaries();
	test_flash_entry_size_fails_closed();
	test_flash_rehash_is_bounded_by_declared_image_length();
	puts("NVRAM sensitive value-slot replacement and release erasure: PASS");
	puts("NVRAM compaction and flash-capacity policy: PASS");
	return 0;
}
