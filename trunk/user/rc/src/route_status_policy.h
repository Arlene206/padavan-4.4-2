/*
 * Aggregate route operation errors without short-circuiting later cleanup.
 */
#ifndef ROUTE_STATUS_POLICY_H
#define ROUTE_STATUS_POLICY_H
static inline int
route_status_accumulate(int current_status, int operation_status)
{
	return current_status ? current_status : operation_status;
}
#endif
