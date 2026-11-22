#ifndef RC_MODEM_MODULE_RELOAD_POLICY_H
#define RC_MODEM_MODULE_RELOAD_POLICY_H

#include <errno.h>
#include <stddef.h>


typedef int (*rc_modem_nvram_int_read_fn)(const char *name,
					  int missing_value, int *value);

/* Treat modem_rule and modem_type as one configuration snapshot. Do not
 * publish a partial pair when either checked NVRAM read fails. */
static inline int
rc_modem_read_nvram_settings_status(rc_modem_nvram_int_read_fn read_int,
				     int *modem_rule, int *modem_type)
{
	int rule_value = 0, type_value = 0, status;

	if (!read_int || !modem_rule || !modem_type) {
		errno = EINVAL;
		return EINVAL;
	}

	*modem_rule = 0;
	*modem_type = 0;
	status = read_int("modem_rule", 0, &rule_value);
	if (status != 0)
		return status;
	status = read_int("modem_type", 0, &type_value);
	if (status != 0)
		return status;

	*modem_rule = rule_value;
	*modem_type = type_value;
	return 0;
}

typedef struct {
	int (*load)(const char *module_name, const char *module_param);
	int (*is_loaded)(const char *module_name);
} rc_modem_module_load_ops_t;

/*
 * A modem family is a set of alternative drivers, not a list of required
 * modules. Attempt each candidate, then verify observable /sys/module state.
 * This distinguishes an already-loaded module from a failed
 * module_smart_load(), whose legacy return value is zero in both cases.
 * The state callback is tri-state: 1=loaded, 0=absent, negative errno=UNKNOWN.
 */
static inline int
rc_modem_module_load_group_verified(const char *const *module_names,
				    const char *const *module_params,
				    size_t module_count,
				    const rc_modem_module_load_ops_t *ops,
				    size_t *available_count)
{
	size_t i, available = 0;
	int first_probe_error = 0;

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
		int probe_status;

		(void)ops->load(module_names[i],
				module_params ? module_params[i] : NULL);
		probe_status = ops->is_loaded(module_names[i]);
		if (probe_status == 1)
			available++;
		else if (probe_status < 0 && first_probe_error == 0)
			first_probe_error = -probe_status;
		else if (probe_status > 1 && first_probe_error == 0)
			first_probe_error = EIO;
	}

	if (available_count)
		*available_count = available;
	if (available > 0)
		return 0;
	if (first_probe_error != 0)
		return first_probe_error;
	return ENODEV;
}

#endif /* RC_MODEM_MODULE_RELOAD_POLICY_H */
