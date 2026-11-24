#include <assert.h>
#include <stdio.h>

#include "factory_reset_policy.h"

static void
test_storage_failure_blocks_nvram_reset(void)
{
	assert(factory_reset_storage_allows_nvram(0));
	assert(!factory_reset_storage_allows_nvram(1));
	assert(!factory_reset_storage_allows_nvram(-1));
	assert(!factory_reset_storage_allows_nvram(256));
}

static void
test_both_persistent_steps_gate_reboot(void)
{
	assert(factory_reset_results_allow_reboot(0, 0));
	assert(!factory_reset_results_allow_reboot(1, 0));
	assert(!factory_reset_results_allow_reboot(-1, 0));
	assert(!factory_reset_results_allow_reboot(0, 1));
	assert(!factory_reset_results_allow_reboot(0, -1));
	assert(!factory_reset_results_allow_reboot(256, 256));
}

int
main(void)
{
	test_storage_failure_blocks_nvram_reset();
	test_both_persistent_steps_gate_reboot();
	puts("Factory reset fail-closed sequencing policy: PASS");
	return 0;
}
