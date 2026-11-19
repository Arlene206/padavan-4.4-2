#ifndef PADAVAN_PROCESS_IO_RETRY_POLICY_H
#define PADAVAN_PROCESS_IO_RETRY_POLICY_H

#include <errno.h>

/* Bound repeated signal interruptions while reading procfs snapshots. */
#define PROCESS_IO_MAX_INTERRUPTS 8U

static inline int
process_io_retry_interrupted(unsigned int *interruptions)
{
	if (!interruptions) {
		errno = EINVAL;
		return -1;
	}
	if (*interruptions >= PROCESS_IO_MAX_INTERRUPTS) {
		errno = EINTR;
		return -1;
	}
	++*interruptions;
	return 0;
}

#endif /* PADAVAN_PROCESS_IO_RETRY_POLICY_H */
