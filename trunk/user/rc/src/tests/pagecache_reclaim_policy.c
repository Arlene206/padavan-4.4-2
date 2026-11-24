#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "pagecache_reclaim_policy.h"

#define CHECK(condition) do { \
	if (!(condition)) { \
		fprintf(stderr, "FAIL %s:%d: %s\\n", __FILE__, __LINE__, #condition); \
		return 1; \
	} \
} while (0)

static int
test_missing_setting_keeps_legacy_default(void)
{
	int mode = -1;

	CHECK(rc_pagecache_reclaim_read_status(0, 0, NULL, &mode) == 0);
	CHECK(mode == 0);
	CHECK(rc_pagecache_reclaim_ratio(mode) == 100);
	return 0;
}

static int
test_present_values_keep_legacy_atoi_and_ratio_mapping(void)
{
	static const char *const values[] = {
		"0", "1", "2", "3", "4", "invalid", "3suffix"
	};
	static const int expected_modes[] = { 0, 1, 2, 3, 4, 0, 3 };
	static const int expected_ratios[] = { 100, 30, 50, 70, 85, 100, 70 };
	int i;
	int mode;

	for (i = 0; i < (int)(sizeof(values) / sizeof(values[0])); ++i) {
		mode = -1;
		CHECK(rc_pagecache_reclaim_read_status(1, 0, values[i], &mode) == 0);
		CHECK(mode == expected_modes[i]);
		CHECK(rc_pagecache_reclaim_ratio(mode) == expected_ratios[i]);
	}
	return 0;
}

static int
test_read_failures_never_publish_a_default(void)
{
	int mode = 77;

	CHECK(rc_pagecache_reclaim_read_status(-1, ENODEV, NULL, &mode) == ENODEV);
	CHECK(mode == 0);
	CHECK(rc_pagecache_reclaim_read_status(-1, 0, NULL, &mode) == EIO);
	CHECK(rc_pagecache_reclaim_read_status(1, 0, NULL, &mode) == EIO);
	CHECK(rc_pagecache_reclaim_read_status(2, 0, "1", &mode) == EIO);
	CHECK(rc_pagecache_reclaim_read_status(0, 0, NULL, NULL) == EINVAL);
	return 0;
}

static int
test_write_failures_are_preserved(void)
{
	CHECK(rc_pagecache_reclaim_write_status(0, 0) == 0);
	CHECK(rc_pagecache_reclaim_write_status(ENOSPC, 0) == ENOSPC);
	CHECK(rc_pagecache_reclaim_write_status(-1, EACCES) == EACCES);
	CHECK(rc_pagecache_reclaim_write_status(-1, 0) == EIO);
	return 0;
}

int
main(void)
{
	CHECK(test_missing_setting_keeps_legacy_default() == 0);
	CHECK(test_present_values_keep_legacy_atoi_and_ratio_mapping() == 0);
	CHECK(test_read_failures_never_publish_a_default() == 0);
	CHECK(test_write_failures_are_preserved() == 0);
	puts("RC pagecache reclaim checked NVRAM/write status policy: PASS");
	return 0;
}
