#ifndef PADAVAN_MODEM_NODE_IDENTITY_POLICY_H
#define PADAVAN_MODEM_NODE_IDENTITY_POLICY_H

#include <string.h>

/* A configured manual node is exact; never substitute another index. */
static inline int
modem_node_manual_index_matches(int manual_index, int candidate_index)
{
	return manual_index < 0 || manual_index == candidate_index;
}

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

/*
 * Select only nodes belonging to the target USB device. Manual selection is
 * a hard index pin; automatic selection may require a preferred port first.
 */
static inline int
modem_node_usb_identity_node_is_eligible(const char *expected_port_id,
                                         int expected_devnum,
                                         const char *candidate_port_id,
                                         int candidate_devnum,
                                         int candidate_index,
                                         int candidate_pref,
                                         int manual_index,
                                         int require_preferred)
{
	if (!modem_node_usb_identity_matches(expected_port_id, expected_devnum,
					     candidate_port_id, candidate_devnum))
		return 0;

	if (manual_index >= 0)
		return candidate_index == manual_index;

	return !require_preferred || candidate_pref > 0;
}

#endif /* PADAVAN_MODEM_NODE_IDENTITY_POLICY_H */
