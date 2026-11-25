#ifndef MINIUPNPD_IPTCRDR_ALLOCATION_POLICY_H
#define MINIUPNPD_IPTCRDR_ALLOCATION_POLICY_H

#include <errno.h>
#include <stdlib.h>
#include <syslog.h>

/* Release every allocated component of an unfinished iptables rule. */
static inline int
miniupnpd_iptcrdr_allocation_failure(void *entry, void *match, void *target,
                                     const char *rule_name,
                                     const char *component)
{
	int saved_errno = errno ? errno : ENOMEM;

	syslog(LOG_ERR, "%s: unable to allocate %s for iptables rule",
	       rule_name ? rule_name : "iptcrdr",
	       component ? component : "rule component");
	free(target);
	free(match);
	free(entry);
	errno = saved_errno;
	return -1;
}

#endif
