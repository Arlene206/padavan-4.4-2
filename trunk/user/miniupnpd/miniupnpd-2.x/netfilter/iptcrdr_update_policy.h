#ifndef MINIUPNPD_IPTCRDR_UPDATE_POLICY_H
#define MINIUPNPD_IPTCRDR_UPDATE_POLICY_H

/* Undo a completed first-table write when the second-table write fails. */
static inline int
miniupnpd_portmapping_rollback_required(int first_status, int second_status)
{
	return first_status == 0 && second_status != 0;
}

/* Keep the primary update error if its compensating rollback also fails. */
static inline int
miniupnpd_portmapping_failure_result(int update_status, int rollback_status)
{
	return update_status != 0 ? update_status : rollback_status;
}

#endif
