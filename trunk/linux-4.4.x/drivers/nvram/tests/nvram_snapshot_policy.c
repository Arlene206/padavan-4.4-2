#include <assert.h>
#include <stddef.h>
#include <stdio.h>

#include "nvram_snapshot_policy.h"

static void test_ioctl_getall_requires_an_empty_pointer_pair(void)
{
	assert(nvram_ioctl_param_request_valid(0, 64, 0, 0));
	assert(!nvram_ioctl_param_request_valid(0, 64, 1, 0));
	assert(!nvram_ioctl_param_request_valid(1, 64, 0, 0));
	assert(nvram_ioctl_param_request_valid(1, 64, 1, 0));
	assert(nvram_ioctl_param_request_valid(63, 64, 1, 0));
	assert(!nvram_ioctl_param_request_valid(64, 64, 1, 0));
	assert(!nvram_ioctl_param_request_valid(-1, 64, 0, 0));
}

static void test_ioctl_parameter_embedded_nul_is_rejected(void)
{
	assert(nvram_ioctl_param_request_valid(5, 64, 1, 0));
	assert(!nvram_ioctl_param_request_valid(5, 64, 1, 1));
	assert(!nvram_ioctl_param_request_valid(0, 64, 0, 1));
}

static void test_value_that_fits_is_copied(void)
{
	assert(nvram_snapshot_copy_decision(32, 8, 7) ==
	       NVRAM_SNAPSHOT_COPY_READY);
	assert(nvram_snapshot_copy_decision(8, 8, 7) ==
	       NVRAM_SNAPSHOT_COPY_READY);
}

static void test_growth_between_sizing_and_copy_retries(void)
{
	size_t original_capacity = 6;
	size_t grown_length = 6;

	assert(nvram_snapshot_copy_decision(64, original_capacity, grown_length) ==
	       NVRAM_SNAPSHOT_COPY_RETRY);
	assert(nvram_snapshot_copy_decision(64, grown_length + 1, grown_length) ==
	       NVRAM_SNAPSHOT_COPY_READY);
}

static void test_shrinkage_between_sizing_and_copy_is_safe(void)
{
	assert(nvram_snapshot_copy_decision(32, 16, 5) ==
	       NVRAM_SNAPSHOT_COPY_READY);
}

static void test_missing_terminator_at_pool_boundary_is_rejected(void)
{
	assert(nvram_snapshot_copy_decision(8, 16, 8) ==
	       NVRAM_SNAPSHOT_COPY_INVALID);
	assert(nvram_snapshot_copy_decision(8, 8, 8) ==
	       NVRAM_SNAPSHOT_COPY_INVALID);
}

int main(void)
{
	assert(NVRAM_GET_SNAPSHOT_RETRIES == 3);
	test_ioctl_getall_requires_an_empty_pointer_pair();
	test_ioctl_parameter_embedded_nul_is_rejected();
	test_value_that_fits_is_copied();
	test_growth_between_sizing_and_copy_retries();
	test_shrinkage_between_sizing_and_copy_is_safe();
	test_missing_terminator_at_pool_boundary_is_rejected();
	puts("NVRAM single-value snapshot sizing policy: PASS");
	return 0;
}
