#ifndef PADAVAN_NVRAM_OPEN_POLICY_H
#define PADAVAN_NVRAM_OPEN_POLICY_H

#include <fcntl.h>

/* NVRAM descriptors must not escape the locked adapter across exec(). */
static inline int
nvram_device_open_flags(void)
{
	return O_RDWR | O_CLOEXEC;
}

#endif /* PADAVAN_NVRAM_OPEN_POLICY_H */
