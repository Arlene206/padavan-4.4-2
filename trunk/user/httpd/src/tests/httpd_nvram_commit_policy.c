#include <stdio.h>

#include "httpd_nvram_commit_policy.h"

static int
test_dirty_state_tracks_durability(void)
{
	if (!httpd_nvram_commit_clears_dirty(0))
		return 1;
	if (httpd_nvram_commit_clears_dirty(-1))
		return 1;
	if (httpd_nvram_commit_clears_dirty(HTTPD_NVRAM_COMMIT_DEFERRED))
		return 1;
	return 0;
}

static int
test_manual_mode_defers_until_commit(void)
{
	if (!httpd_nvram_reboot_requires_commit(1, 0))
		return 1;
	if (!httpd_nvram_reboot_requires_commit(1, 1))
		return 1;
	if (httpd_nvram_reboot_requires_commit(0, 0))
		return 1;
	if (!httpd_nvram_reboot_requires_commit(0, 1))
		return 1;
	return 0;
}

static int
test_failed_commit_blocks_reboot(void)
{
	if (httpd_nvram_reboot_is_safe(1, -1))
		return 1;
	if (httpd_nvram_reboot_is_safe(1, HTTPD_NVRAM_COMMIT_DEFERRED))
		return 1;
	if (!httpd_nvram_reboot_is_safe(1, 0))
		return 1;
	if (!httpd_nvram_reboot_is_safe(0, -1))
		return 1;
	return 0;
}

int
main(void)
{
	if (test_dirty_state_tracks_durability() ||
	    test_manual_mode_defers_until_commit() ||
	    test_failed_commit_blocks_reboot()) {
		fprintf(stderr, "HTTPD NVRAM commit policy: FAIL\n");
		return 1;
	}
	printf("HTTPD NVRAM commit policy: PASS\n");
	return 0;
}
