#ifndef PADAVAN_MODEM_NODE_IDENTITY_POLICY_H
#define PADAVAN_MODEM_NODE_IDENTITY_POLICY_H

#include <string.h>

/* USB devnum is meaningful only when bound to its physical port identity. */
static inline int
modem_node_usb_identity_is_valid(const char *port_id, int devnum)
{
	return port_id && *port_id && devnum >= 1 && devnum <= 127;
}

static inline int
modem_node_usb_identity_matches(const char *expected_port_id,
                                int expected_devnum,
                                const char *candidate_port_id,
                                int candidate_devnum)
{
	if (!modem_node_usb_identity_is_valid(expected_port_id, expected_devnum) ||
	    !modem_node_usb_identity_is_valid(candidate_port_id, candidate_devnum))
		return 0;

	return expected_devnum == candidate_devnum &&
	       strcmp(expected_port_id, candidate_port_id) == 0;
}

#endif /* PADAVAN_MODEM_NODE_IDENTITY_POLICY_H */
