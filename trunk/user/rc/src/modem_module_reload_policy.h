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


/* Publish a requested module setting only after the new driver group is ready.
 * A failed reload must leave the old cache value intact so the retained
 * notification can retry the transition instead of silently skipping it.
 */
static inline int
rc_modem_module_reload_cache_publish(int reload_status, int *cached_value,
				     int requested_value)
{
	if (!cached_value)
		return EINVAL;
	if (reload_status != 0)
		return reload_status > 0 ? reload_status : EIO;
	*cached_value = requested_value;
	return 0;
}

typedef int (*rc_modem_module_unload_fn)(const char *module_name,
					 void *context);

/* A confirmed-absent module needs no unload. UNKNOWN presence or a failed
 * unload must block a driver-family transition; preserve errno when available. */
static inline int
rc_modem_module_unload_attempt_status(int loaded_status, int unload_status,
				      int unload_errno)
{
	if (loaded_status == 0)
		return 0;
	if (loaded_status < 0)
		return -loaded_status;
	if (loaded_status != 1)
		return EIO;
	if (unload_status == 1)
		return 0;
	if (unload_status == 0 && unload_errno > 0)
		return unload_errno;
	return EIO;
}

/* Validate the entire module set before causing unload side effects. */
static inline int
rc_modem_module_unload_group_status(const char *const *module_names,
				    size_t module_count,
				    rc_modem_module_unload_fn unload,
				    void *context,
				    size_t *successful_count)
{
	size_t i, successful = 0;
	int first_error = 0;

	if (successful_count)
		*successful_count = 0;
	if (!module_names || module_count == 0 || !unload)
		return EINVAL;
	for (i = 0; i < module_count; i++) {
		if (!module_names[i] || module_names[i][0] == '\0')
			return EINVAL;
	}

	for (i = 0; i < module_count; i++) {
		int result, saved_errno;

		errno = 0;
		result = unload(module_names[i], context);
		saved_errno = errno;
		if (result == 1)
			++successful;
		else if (first_error == 0)
			first_error = result == 0 && saved_errno > 0 ?
				saved_errno : EIO;
	}

	if (successful_count)
		*successful_count = successful;
	return first_error;
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
