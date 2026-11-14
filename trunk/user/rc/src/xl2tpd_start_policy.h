#ifndef XL2TPD_START_POLICY_H
#define XL2TPD_START_POLICY_H
#include <process_inventory.h>
/* 1 = running, 0 = absent, -1 = unknown (fail closed). */
static inline int
xl2tpd_start_policy_result(enum process_presence presence)
{
	if (presence == PROCESS_PRESENCE_RUNNING)
		return 1;
	if (presence == PROCESS_PRESENCE_ABSENT)
		return 0;
	return -1;
}
#endif /* XL2TPD_START_POLICY_H */
