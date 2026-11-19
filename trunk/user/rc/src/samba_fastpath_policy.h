#ifndef PADAVAN_SAMBA_FASTPATH_POLICY_H
#define PADAVAN_SAMBA_FASTPATH_POLICY_H

#include "process_inventory.h"

/* Hardware SMB acceleration is valid only while smbd is verified RUNNING. */
static inline int
samba_fastpath_should_enable(int configured,
			    enum process_presence smbd_presence)
{
	return configured == 1 && smbd_presence == PROCESS_PRESENCE_RUNNING;
}

#endif /* PADAVAN_SAMBA_FASTPATH_POLICY_H */
