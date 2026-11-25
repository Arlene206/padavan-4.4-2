/* SPDX-License-Identifier: GPL-2.0 */
#ifndef PADAVAN_PIDS_PRESENCE_POLICY_H
#define PADAVAN_PIDS_PRESENCE_POLICY_H

#include "process_inventory.h"

/*
 * Legacy pids() has a Boolean contract. Only a confirmed ABSENT result may
 * become false; UNKNOWN is conservatively true so procfs/allocation failures
 * cannot be mistaken for proof that a service is stopped. New callers that
 * need to distinguish UNKNOWN should use pids_presence().
 */
static inline int
pids_presence_legacy_boolean(enum process_presence presence)
{
	return presence != PROCESS_PRESENCE_ABSENT;
}

#endif /* PADAVAN_PIDS_PRESENCE_POLICY_H */
