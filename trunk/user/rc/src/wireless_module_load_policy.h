#ifndef RC_WIRELESS_MODULE_LOAD_POLICY_H
#define RC_WIRELESS_MODULE_LOAD_POLICY_H

#include <errno.h>
#include <stddef.h>

typedef struct {
	int (*load)(const char *module_name, const char *module_param);
	int (*is_loaded)(const char *module_name);
} rc_module_load_ops_t;

typedef rc_module_load_ops_t rc_wireless_module_load_ops_t;

/*
 * module_smart_load() is not a reliable success indicator: it returns zero
 * both when a module is already present and when modprobe fails. The
 * /sys/module post-load state is authoritative; preserve module parameters.
 */
static inline int
rc_module_load_verified(const char *module_name, const char *module_param,
			const rc_module_load_ops_t *ops)
{
	if (!module_name || module_name[0] == '\0' ||
	    !ops || !ops->load || !ops->is_loaded)
		return EINVAL;

	(void)ops->load(module_name, module_param);
	return ops->is_loaded(module_name) ? 0 : EIO;
}

/* Preserve the existing call shape for wireless-driver users. */
static inline int
rc_wireless_module_load_verified(const char *module_name,
				 const rc_wireless_module_load_ops_t *ops)
{
	return rc_module_load_verified(module_name, NULL, ops);
}

#endif
