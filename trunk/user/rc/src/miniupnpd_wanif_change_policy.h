#ifndef MINIUPNPD_WANIF_CHANGE_POLICY_H
#define MINIUPNPD_WANIF_CHANGE_POLICY_H

#include <errno.h>
#include <string.h>

/*
 * An unreadable or empty interface identity must never be interpreted as a
 * WAN transition: callers may use the result to stop a running daemon.
 */
static inline int
miniupnpd_wanif_change_decide(int read_status, const char *requested,
			      const char *active, int *should_stop)
{
	if (should_stop == NULL)
		return EINVAL;

	*should_stop = 0;
	if (read_status != 0)
		return read_status;
	if (requested == NULL || requested[0] == '\0' ||
	    active == NULL || active[0] == '\0')
		return EINVAL;

	*should_stop = (strcmp(requested, active) != 0);
	return 0;
}

#endif /* MINIUPNPD_WANIF_CHANGE_POLICY_H */
