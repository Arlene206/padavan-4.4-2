#ifndef DETECT_WAN_UDHCPC_POLICY_H
#define DETECT_WAN_UDHCPC_POLICY_H

#include <errno.h>
#include <process_inventory.h>

/* Only a complete RUNNING snapshot may authorize the DHCP renew signal. */
static inline int
detect_wan_udhcpc_should_renew(enum process_presence presence)
{
	switch (presence) {
	case PROCESS_PRESENCE_RUNNING:
		return 1;
	case PROCESS_PRESENCE_ABSENT:
		return 0;
	case PROCESS_PRESENCE_UNKNOWN:
	default:
		errno = EIO;
		return -1;
	}
}


/* Route renewal only to the DHCP client owned by this WAN/AP role. */
static inline const char *
detect_wan_udhcpc_pidfile(int ap_mode)
{
	return ap_mode ? "/var/run/udhcpc_lan.pid" : "/var/run/udhcpc0.pid";
}

#endif /* DETECT_WAN_UDHCPC_POLICY_H */
