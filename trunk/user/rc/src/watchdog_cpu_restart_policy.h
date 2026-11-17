#ifndef RC_WATCHDOG_CPU_RESTART_POLICY_H
#define RC_WATCHDOG_CPU_RESTART_POLICY_H

#include <errno.h>
#include <stddef.h>

typedef struct {
	int (*load)(const char *module_name, const char *module_param);
	int (*unload)(const char *module_name, int recurse_unload);
	/* 1=loaded, 0=absent, negative errno=UNKNOWN/probe failure. */
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
	int probe_status;

	if (!module_name || module_name[0] == '\0' || !ops ||
	    !ops->load || !ops->unload || !ops->is_loaded)
		return EINVAL;

	if (should_load)
		(void)ops->load(module_name, NULL);
	else
		(void)ops->unload(module_name, 0);

	/* An unreadable sysfs state is not proof that the module is absent. */
	probe_status = ops->is_loaded(module_name);
	if (probe_status < 0)
		return -probe_status;
	if (probe_status > 1)
		return EIO;
	if (probe_status == 1)
		return should_load ? 0 : EBUSY;
	return should_load ? EIO : 0;
}

#endif /* RC_WATCHDOG_CPU_RESTART_POLICY_H */
