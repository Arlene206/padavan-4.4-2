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

#endif /* MINIUPNPD_REDIRECT_CLEANUP_POLICY_H */
