#ifndef PADAVAN_LIFECYCLE_LOCK_RETRY_POLICY_H
#define PADAVAN_LIFECYCLE_LOCK_RETRY_POLICY_H

#include <errno.h>

/* A signal storm must not keep a service lifecycle lock held forever. */
#define RC_LIFECYCLE_LOCK_MAX_INTERRUPTS 8U

static inline int
rc_lifecycle_lock_retry_interrupted(unsigned int *interruptions)
{
	if (!interruptions) {
		errno = EINVAL;
		return -1;
	}
	if (*interruptions >= RC_LIFECYCLE_LOCK_MAX_INTERRUPTS) {
		errno = EINTR;
		return -1;
	}
	++*interruptions;
	return 0;
}

#endif /* PADAVAN_LIFECYCLE_LOCK_RETRY_POLICY_H */
