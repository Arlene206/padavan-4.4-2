#ifndef PADAVAN_WAN_NVRAM_STATUS_POLICY_H
#define PADAVAN_WAN_NVRAM_STATUS_POLICY_H

#include <errno.h>

/* NVRAM APIs use 0/-1 and report the concrete failure through errno. */
static int
rc_wan_nvram_write_status(int api_result, int saved_errno)
{
	if (api_result == 0)
		return 0;
	return saved_errno > 0 ? saved_errno : EIO;
}

typedef int (*rc_wan_nvram_param_set_fn)(void *context,
					 const char *param_name);

/*
 * Copy a required configuration tuple in order. Stop at the first failure so
 * callers can prevent activation with a partially refreshed WAN snapshot.
 */
static inline int
rc_wan_nvram_set_required_params(const char *const *param_names,
				 unsigned int count,
				 rc_wan_nvram_param_set_fn set_param,
				 void *context)
{
	unsigned int i;
	int status;

	if ((!param_names && count != 0) || !set_param)
		return EINVAL;

	for (i = 0; i < count; ++i) {
		if (!param_names[i] || !param_names[i][0])
			return EINVAL;
		status = set_param(context, param_names[i]);
		if (status != 0)
			return status;
	}

	return 0;
}


struct rc_wan_nvram_value {
	const char *param_name;
	const char *value;
};

typedef int (*rc_wan_nvram_value_set_fn)(void *context,
					 const char *param_name,
					 const char *value);

/* Clear or seed a required runtime snapshot in order, stopping at first error. */
static inline int
rc_wan_nvram_set_required_values(const struct rc_wan_nvram_value *values,
				 unsigned int count,
				 rc_wan_nvram_value_set_fn set_value,
				 void *context)
{
	unsigned int i;
	int status;

	if ((!values && count != 0) || !set_value)
		return EINVAL;

	for (i = 0; i < count; ++i) {
		if (!values[i].param_name || !values[i].param_name[0] ||
		    !values[i].value)
			return EINVAL;
		status = set_value(context, values[i].param_name, values[i].value);
		if (status != 0)
			return status;
	}

	return 0;
}

#endif /* PADAVAN_WAN_NVRAM_STATUS_POLICY_H */
