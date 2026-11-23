#ifndef PADAVAN_SAMBA_FASTPATH_POLICY_H
#define PADAVAN_SAMBA_FASTPATH_POLICY_H

#include <errno.h>

#include "process_inventory.h"

/* Hardware SMB acceleration is valid only while smbd is verified RUNNING. */
static inline int
samba_fastpath_should_enable(int configured,
			    enum process_presence smbd_presence)
{
	return configured == 1 && smbd_presence == PROCESS_PRESENCE_RUNNING;
}

/* A daemon stop is not complete while stale SMB acceleration remains set. */
static inline int
samba_fastpath_stop_result(int daemon_stop_status, int fastpath_write_status)
{
	if (daemon_stop_status != 0)
		return daemon_stop_status;
	/* ENOENT means this target has no optional SMB fastpath proc hook. */
	return fastpath_write_status == 0 || fastpath_write_status == ENOENT ?
		0 : -1;
}

#endif /* PADAVAN_SAMBA_FASTPATH_POLICY_H */
