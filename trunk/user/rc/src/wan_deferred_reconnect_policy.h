#ifndef WAN_DEFERRED_RECONNECT_POLICY_H
#define WAN_DEFERRED_RECONNECT_POLICY_H

#include <errno.h>
#include <limits.h>

typedef int (*wan_deferred_reconnect_publish_fn)(void *context,
                                                  const char *name,
                                                  long value);

static inline int
wan_deferred_reconnect_publish(wan_deferred_reconnect_publish_fn publish,
                               void *context, long deadline)
{
	if (!publish)
		return EINVAL;
	if (deadline < 0)
		return EINVAL;
	if (deadline > INT_MAX)
		return EOVERFLOW;
	return publish(context, "deferred_wanup_t", deadline);
}

static inline int
wan_deferred_reconnect_clear(wan_deferred_reconnect_publish_fn publish,
                             void *context)
{
	return wan_deferred_reconnect_publish(publish, context, 0);
}

/*
 * watchdog.c reads deferred_wanup_t through nvram_get_int(). Keep the
 * published deadline in that same positive signed-int domain and reject
 * overflow instead of invoking signed-overflow UB or silently truncating.
 */
static inline int
wan_deferred_reconnect_schedule(long now, long delay,
                                wan_deferred_reconnect_publish_fn publish,
                                void *context)
{
	if (!publish)
		return EINVAL;
	if (now < 0 || delay <= 0)
		return EINVAL;
	if (delay > INT_MAX || now > (long)INT_MAX - delay)
		return EOVERFLOW;
	return wan_deferred_reconnect_publish(publish, context, now + delay);
}

#endif
