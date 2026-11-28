#ifndef PADAVAN_SMP_AFFINITY_POLICY_H
#define PADAVAN_SMP_AFFINITY_POLICY_H
#include <errno.h>
/*
 * IRQ and queue topology differs between boards and drivers. Missing optional
 * procfs/sysfs nodes are ignored; permission and I/O failures are preserved.
 */
static inline int
smp_affinity_record_status(int first_status, int operation_status)
{
	if (first_status == ENOENT)
		first_status = 0;
	if (operation_status == ENOENT)
		operation_status = 0;
	return first_status != 0 ? first_status : operation_status;
}
#endif /* PADAVAN_SMP_AFFINITY_POLICY_H */
