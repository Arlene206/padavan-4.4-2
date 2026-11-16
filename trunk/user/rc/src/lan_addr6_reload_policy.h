#ifndef __LAN_ADDR6_RELOAD_POLICY_H__
#define __LAN_ADDR6_RELOAD_POLICY_H__

#include <errno.h>

typedef int (*rc_lan_addr6_clear_fn)(void);
typedef int (*rc_lan_addr6_add_fn)(void *context);

/* Clear stale state before publishing a replacement; preserve either error. */
static inline int
rc_lan_addr6_reload_apply(int has_address,
                          rc_lan_addr6_clear_fn clear_address,
                          rc_lan_addr6_add_fn add_address,
                          void *context)
{
	int status;

	if (!clear_address || (has_address && !add_address))
		return EINVAL;

	status = clear_address();
	if (status != 0 || !has_address)
		return status;

	return add_address(context);
}

#endif
