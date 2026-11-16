#ifndef RC_MODEM_MODULE_RELOAD_POLICY_H
#define RC_MODEM_MODULE_RELOAD_POLICY_H

#include <errno.h>
#include <stddef.h>

typedef struct {
	int (*load)(const char *module_name, const char *module_param);
	int (*is_loaded)(const char *module_name);
} rc_modem_module_load_ops_t;

/*
 * A modem family is a set of alternative drivers, not a list of required
 * modules. Attempt each candidate, then verify observable /sys/module state.
 * This distinguishes an already-loaded module from a failed
 * module_smart_load(), whose legacy return value is zero in both cases.
 */
static inline int
rc_modem_module_load_group_verified(const char *const *module_names,
				    const char *const *module_params,
				    size_t module_count,
				    const rc_modem_module_load_ops_t *ops,
				    size_t *available_count)
{
	size_t i, available = 0;

	if (available_count)
		*available_count = 0;
	if (!module_names || module_count == 0 || !ops ||
	    !ops->load || !ops->is_loaded)
		return EINVAL;

	/* Validate the entire group before invoking any loader. */
	for (i = 0; i < module_count; i++) {
		if (!module_names[i] || module_names[i][0] == '\0')
			return EINVAL;
	}

	for (i = 0; i < module_count; i++) {
		(void)ops->load(module_names[i],
				module_params ? module_params[i] : NULL);
		if (ops->is_loaded(module_names[i]))
			available++;
	}

	if (available_count)
		*available_count = available;
	return available > 0 ? 0 : ENODEV;
}

#endif /* RC_MODEM_MODULE_RELOAD_POLICY_H */
