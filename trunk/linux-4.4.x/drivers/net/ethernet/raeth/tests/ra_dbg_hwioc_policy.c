#include <assert.h>
#include <stdio.h>

#include "../ra_dbg_hwioc_policy.h"

static void test_operation_bounds(void)
{
	assert(!raeth_hwioc_pmu_operation_valid(-1));
	assert(raeth_hwioc_pmu_operation_valid(0));
	assert(raeth_hwioc_pmu_operation_valid(1));
	assert(raeth_hwioc_pmu_operation_valid(2));
	assert(!raeth_hwioc_pmu_operation_valid(3));
	assert(!raeth_hwioc_pmu_operation_valid(0x7fffffffL));
}

static void test_enable_values(void)
{
	assert(raeth_hwioc_pmu_enable_valid(0));
	assert(raeth_hwioc_pmu_enable_valid(1));
	assert(!raeth_hwioc_pmu_enable_valid(-1));
	assert(!raeth_hwioc_pmu_enable_valid(2));
}

static void test_counter_arguments(void)
{
	static const long events[] = { 0x0, 0x3, 0xa, 0xc, 0x10, 0x11 };
	unsigned int i;

	assert(!raeth_hwioc_pmu_counter_args_valid(-1, 0, 0));
	assert(!raeth_hwioc_pmu_counter_args_valid(4, 0, 0));
	assert(!raeth_hwioc_pmu_counter_args_valid(0, -1, 0));
	assert(!raeth_hwioc_pmu_counter_args_valid(0, 9, 0));
	for (i = 0; i < sizeof(events) / sizeof(events[0]); ++i)
		assert(raeth_hwioc_pmu_counter_args_valid(3, 8, events[i]));
	assert(!raeth_hwioc_pmu_counter_args_valid(0, 0, -1));
	assert(!raeth_hwioc_pmu_counter_args_valid(0, 0, 1));
	assert(!raeth_hwioc_pmu_counter_args_valid(0, 0, 0x1f));
}

int main(void)
{
	test_operation_bounds();
	test_enable_values();
	test_counter_arguments();
	puts("RAETH HW-IOC PMU command bounds and event validation: PASS");
	return 0;
}
