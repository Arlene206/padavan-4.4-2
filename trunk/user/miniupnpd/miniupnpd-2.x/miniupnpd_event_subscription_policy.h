#ifndef MINIUPNPD_EVENT_SUBSCRIPTION_POLICY_H
#define MINIUPNPD_EVENT_SUBSCRIPTION_POLICY_H

#include <stddef.h>

/*
 * Event subscriptions own heap state and a notification socket. Keep an
 * explicit ceiling so repeated LAN SUBSCRIBE requests cannot exhaust the
 * daemon's memory and file descriptors.
 */
#define MINIUPNPD_EVENT_MAX_SUBSCRIBERS 32U

static inline int
miniupnpd_event_subscriber_has_capacity(size_t current_count)
{
	return current_count < MINIUPNPD_EVENT_MAX_SUBSCRIBERS;
}

#endif /* MINIUPNPD_EVENT_SUBSCRIPTION_POLICY_H */
