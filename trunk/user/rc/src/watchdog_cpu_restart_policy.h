#ifndef RC_WATCHDOG_CPU_RESTART_POLICY_H
#define RC_WATCHDOG_CPU_RESTART_POLICY_H

#include <errno.h>
#include <stddef.h>

/*
 * A failed watchdog_cpu read must not become configuration value zero:
 * zero explicitly disables the CPU watchdog module.
 */
static inline int
rc_watchdog_cpu_restart_config_status(int read_status, int configured_value,
				      int *should_load)
{
	if (!should_load)
		return EINVAL;
	if (read_status != 0)
		return read_status;
	*should_load = configured_value != 0;
	return 0;
}

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
