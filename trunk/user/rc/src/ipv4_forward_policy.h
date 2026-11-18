#ifndef RC_IPV4_FORWARD_POLICY_H
#define RC_IPV4_FORWARD_POLICY_H
#include <errno.h>
typedef int (*rc_ipv4_forward_write_fn)(void *context, int enabled);
/* Preserve errno-style sysctl failures; normalize negative results to EIO. */
static inline int
rc_ipv4_forward_apply(int enabled, rc_ipv4_forward_write_fn write_fn,
		      void *context)
{
	int status;
	if (!write_fn)
		return EINVAL;
	status = write_fn(context, enabled ? 1 : 0);
	if (status == 0)
		return 0;
	return status > 0 ? status : EIO;
}
#endif
