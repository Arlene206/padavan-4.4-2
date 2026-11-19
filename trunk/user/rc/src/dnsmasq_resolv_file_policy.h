#ifndef PADAVAN_DNSMASQ_RESOLV_FILE_POLICY_H
#define PADAVAN_DNSMASQ_RESOLV_FILE_POLICY_H

#include <errno.h>

/*
 * create_file() uses a nonzero return for failure but leaves the caller to
 * preserve errno. A zero errno after failure is not a valid success signal;
 * normalize it to EIO so service startup cannot silently continue.
 */
static inline int
dnsmasq_resolv_file_error(int create_status, int create_errno)
{
	if (create_status == 0)
		return 0;
	return create_errno > 0 ? create_errno : EIO;
}

#endif /* PADAVAN_DNSMASQ_RESOLV_FILE_POLICY_H */
