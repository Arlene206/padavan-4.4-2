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

/* A committed NAT deletion is compensated when the filter deletion fails. */
static inline int
miniupnpd_nat_delete_rollback_required(int nat_status, int filter_status)
{
	return nat_status == 0 && filter_status != 0;
}

/* Keep the filter-side failure if restoring NAT also fails. */
static inline int
miniupnpd_nat_delete_failure_result(int filter_status, int rollback_status)
{
	return filter_status != 0 ? filter_status : rollback_status;
}

/* Restore peer SNAT when a later mangle/DSCP deletion fails. */
static inline int
miniupnpd_peer_delete_rollback_required(int peer_status, int dscp_status)
{
	return peer_status == 0 && dscp_status != 0;
}

static inline int
miniupnpd_peer_delete_failure_result(int dscp_status, int rollback_status)
{
	return dscp_status != 0 ? dscp_status : rollback_status;
}

#endif
