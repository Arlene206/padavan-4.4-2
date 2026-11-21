#ifndef MINIUPNPD_REDIRECT_CLEANUP_POLICY_H
#define MINIUPNPD_REDIRECT_CLEANUP_POLICY_H

#include <errno.h>

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


/* Keep in-memory redirect metadata aligned with primary mapping state while
 * preserving the aggregate firewall status from independent cleanup stages. */
typedef void (*miniupnpd_redirect_metadata_cleanup_fn)(unsigned short eport,
                                                        int proto);

static inline int
miniupnpd_finalize_redirect_cleanup(
	int aggregate_status,
	int primary_mapping_removed,
	miniupnpd_redirect_metadata_cleanup_fn cleanup,
	unsigned short eport,
	int proto)
{
	if (primary_mapping_removed && cleanup)
		cleanup(eport, proto);
	return aggregate_status;
}


/* Never tear down peer SNAT/DSCP while the primary DNAT/filter mapping
 * is still present or its deletion state could not be established. */
static inline int
miniupnpd_peer_redirect_cleanup_allowed(int primary_mapping_removed)
{
	return primary_mapping_removed != 0;
}


/* Cleanup helpers may alter errno while releasing a failed IPTC handle.
 * Restore the operation's original error before returning to the caller. */
static inline int
miniupnpd_restore_errno_on_failure(int status, int saved_errno)
{
	if (status != 0)
		errno = saved_errno != 0 ? saved_errno : EIO;
	return status;
}


/* Primary failure wins; otherwise return the secondary cleanup error. */
static inline int
miniupnpd_select_cleanup_failure_errno(int primary_status,
                                       int primary_errno,
                                       int secondary_errno)
{
	if (primary_status != 0)
		return primary_errno != 0 ? primary_errno : EIO;
	return secondary_errno != 0 ? secondary_errno : EIO;
}


/* Retain the first failure errno observed within one cleanup phase. */
static inline int
miniupnpd_record_first_failure_errno(int current_errno, int candidate_errno)
{
	if (current_errno != 0)
		return current_errno;
	return candidate_errno != 0 ? candidate_errno : EIO;
}

#endif /* MINIUPNPD_REDIRECT_CLEANUP_POLICY_H */
