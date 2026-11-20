#ifndef MINIUPNPD_REDIRECT_CLEANUP_POLICY_H
#define MINIUPNPD_REDIRECT_CLEANUP_POLICY_H

/* Preserve the first failure when independent cleanup stages are combined. */
static inline int
miniupnpd_merge_cleanup_status(int primary_status, int secondary_status)
{
	if (primary_status != 0)
		return primary_status;
	return secondary_status;
}

/*
 * Report whether the primary DNAT/filter mapping is gone even when secondary
 * peer SNAT/DSCP cleanup reports an independent failure.
 */
static inline int
miniupnpd_primary_redirect_removed(int nat_scan_completed,
                                   int nat_rule_found,
                                   int nat_delete_status,
                                   int filter_delete_status,
                                   int rollback_attempted,
                                   int rollback_status)
{
	if (!nat_scan_completed)
		return 0;
	if (!nat_rule_found)
		return 1;
	if (nat_delete_status != 0)
		return 0;
	if (filter_delete_status == 0)
		return 1;
	return rollback_attempted && rollback_status != 0;
}

#endif /* MINIUPNPD_REDIRECT_CLEANUP_POLICY_H */
