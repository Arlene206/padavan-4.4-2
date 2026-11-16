#ifndef RC_WATCHDOG_CPU_RESTART_POLICY_H
#define RC_WATCHDOG_CPU_RESTART_POLICY_H

#include <errno.h>
#include <stddef.h>

typedef struct {
	int (*load)(const char *module_name, const char *module_param);
	int (*unload)(const char *module_name, int recurse_unload);
	int (*is_loaded)(const char *module_name);
} rc_watchdog_cpu_restart_ops_t;

/*
 * module_smart_load() returns zero both when already loaded and when modprobe
 * fails. module_smart_unload() may also leave a referenced module resident.
 * The requested module state, checked after the operation, is authoritative.
 */
static inline int
rc_watchdog_cpu_restart_verified(const char *module_name, int should_load,
				 const rc_watchdog_cpu_restart_ops_t *ops)
{
	if (!module_name || module_name[0] == '\0' || !ops ||
	    !ops->load || !ops->unload || !ops->is_loaded)
		return EINVAL;

	if (should_load)
		(void)ops->load(module_name, NULL);
	else
		(void)ops->unload(module_name, 0);

	if (ops->is_loaded(module_name))
		return should_load ? 0 : EBUSY;
	return should_load ? EIO : 0;
}

#endif /* RC_WATCHDOG_CPU_RESTART_POLICY_H */
