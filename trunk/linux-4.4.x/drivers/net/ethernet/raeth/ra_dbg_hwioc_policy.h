#ifndef _RAETH_DBG_HWIOC_PMU_POLICY_H_
#define _RAETH_DBG_HWIOC_PMU_POLICY_H_

#define RAETH_HWIOC_PMU_OPERATION_COUNT 3
#define RAETH_HWIOC_PMU_COUNTER_COUNT 4
#define RAETH_HWIOC_PMU_INTERFACE_COUNT 9

static inline int raeth_hwioc_pmu_operation_valid(long operation)
{
	return operation >= 0 && operation < RAETH_HWIOC_PMU_OPERATION_COUNT;
}

static inline int raeth_hwioc_pmu_enable_valid(long enable)
{
	return enable == 0 || enable == 1;
}

static inline int
raeth_hwioc_pmu_counter_args_valid(long counter, long interface, long event)
{
	if (counter < 0 || counter >= RAETH_HWIOC_PMU_COUNTER_COUNT)
		return 0;
	if (interface < 0 || interface >= RAETH_HWIOC_PMU_INTERFACE_COUNT)
		return 0;

	switch (event) {
	case 0x0:
	case 0x3:
	case 0xa:
	case 0xc:
	case 0x10:
	case 0x11:
		return 1;
	default:
		return 0;
	}
}

#endif
