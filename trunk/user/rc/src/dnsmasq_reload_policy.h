#ifndef PADAVAN_DNSMASQ_RELOAD_POLICY_H
#define PADAVAN_DNSMASQ_RELOAD_POLICY_H

#include <errno.h>
#include <signal.h>
#include "process_inventory.h"

typedef enum process_presence (*dnsmasq_reload_validate_fn)(
	void *context, const struct process_identity *identity);
typedef int (*dnsmasq_reload_signal_fn)(void *context, pid_t pid,
					int signal_number);

/* Resolver publication may skip cache reload only on a complete ABSENT
 * snapshot. UNKNOWN is an error, never a false "not running" result. */
static inline int
dnsmasq_reload_notification_decision(enum process_presence presence)
{
	if (presence == PROCESS_PRESENCE_RUNNING)
		return 1;
	if (presence == PROCESS_PRESENCE_ABSENT)
		return 0;
	return -1;
}

/* If every target disappears before a reload is delivered, the requested
 * cache state is already absent. A signal race remains an error while any
 * DNSMasq process is still running or its state cannot be verified. */
static inline int
dnsmasq_reload_post_signal_status(int signal_status,
				  enum process_presence post_presence,
				  int scan_error)
{
	if (signal_status != ESRCH)
		return signal_status;
	if (post_presence == PROCESS_PRESENCE_ABSENT)
		return 0;
	if (post_presence == PROCESS_PRESENCE_UNKNOWN)
		return scan_error > 0 ? scan_error : EIO;
	if (post_presence == PROCESS_PRESENCE_RUNNING)
		return ESRCH;
	return EIO;
}

/* Validate all snapshot identities before the first signal, then validate
 * again immediately before each targeted SIGHUP. UNKNOWN always fails closed;
 * identities that have disappeared are skipped. */
static inline int
dnsmasq_reload_signal_snapshot(const struct process_snapshot *snapshot,
			       dnsmasq_reload_validate_fn validate,
			       dnsmasq_reload_signal_fn send_signal,
			       void *context)
{
	size_t i;
	int signalled = 0;
	enum process_presence presence;

	if (!snapshot || !validate || !send_signal)
		return EINVAL;
	if (snapshot->state == PROCESS_PRESENCE_ABSENT)
		return ESRCH;
	if (snapshot->state != PROCESS_PRESENCE_RUNNING ||
	    !snapshot->items || snapshot->count == 0)
		return EIO;

	/* A failed validation anywhere in the initial snapshot means no signals. */
	for (i = 0; i < snapshot->count; i++) {
		errno = 0;
		presence = validate(context, &snapshot->items[i]);
		if (presence == PROCESS_PRESENCE_UNKNOWN)
			return errno ? errno : EIO;
		if (presence != PROCESS_PRESENCE_RUNNING &&
		    presence != PROCESS_PRESENCE_ABSENT)
			return EIO;
	}

	for (i = 0; i < snapshot->count; i++) {
		errno = 0;
		presence = validate(context, &snapshot->items[i]);
		if (presence == PROCESS_PRESENCE_UNKNOWN)
			return errno ? errno : EIO;
		if (presence == PROCESS_PRESENCE_ABSENT)
			continue;
		if (presence != PROCESS_PRESENCE_RUNNING)
			return EIO;
		errno = 0;
		if (send_signal(context, snapshot->items[i].pid, SIGHUP) != 0)
			return errno ? errno : EIO;
		signalled++;
	}

	return signalled ? 0 : ESRCH;
}

#endif /* PADAVAN_DNSMASQ_RELOAD_POLICY_H */
