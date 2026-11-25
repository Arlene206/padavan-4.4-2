#ifndef __MODEM_SR_STATUS_POLICY_H__
#define __MODEM_SR_STATUS_POLICY_H__

/*
 * module_smart_unload() returns one if the module was unloaded or remains
 * loaded because it is in use. A zero result can also mean that the refcount
 * file was unavailable because the module is already absent; a tri-state
 * module-presence probe distinguishes that safe case from an error.
 * Presence: 0 absent, 1 present, negative unknown.
 */
static inline int
modem_sr_completion_status(int eject_status, int unload_status,
			   int module_presence)
{
	return (eject_status == 0 &&
		(unload_status != 0 || module_presence == 0)) ? 0 : -1;
}

#endif
