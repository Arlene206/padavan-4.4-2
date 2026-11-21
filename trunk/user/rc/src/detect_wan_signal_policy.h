#ifndef DETECT_WAN_SIGNAL_POLICY_H
#define DETECT_WAN_SIGNAL_POLICY_H

#include <signal.h>

/* SIGTERM only records a stop request; cleanup belongs to normal context. */
static inline void
detect_wan_record_signal(volatile sig_atomic_t *stopping, int signal_number)
{
	if (!stopping || signal_number != SIGTERM)
		return;
	*stopping = 1;
}

#endif /* DETECT_WAN_SIGNAL_POLICY_H */
