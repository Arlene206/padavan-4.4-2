#ifndef PADAVAN_WAN_NVRAM_STATUS_POLICY_H
#define PADAVAN_WAN_NVRAM_STATUS_POLICY_H

#include <errno.h>
#include <stddef.h>
#include <string.h>

/* NVRAM APIs use 0/-1 and report the concrete failure through errno. */
static int
rc_wan_nvram_write_status(int api_result, int saved_errno)
{
	if (api_result == 0)
		return 0;
	return saved_errno > 0 ? saved_errno : EIO;
}


/* Checked NVRAM reads distinguish an absent key from a backend failure. */
static int
rc_wan_nvram_read_status(int api_result, int saved_errno)
{
	if (api_result == 0 || api_result == 1)
		return 0;
	if (api_result < 0)
		return saved_errno > 0 ? saved_errno : EIO;
	return EIO;
}

typedef int (*rc_wan_nvram_get_fn)(void *context,
					 const char *param_name,
					 char **value);

/*
 * Copy one checked value before the next cache-backed NVRAM read. A missing
 * key is an empty setting; a backend failure or truncation is never hidden.
 * Callers that need a coherent tuple must hold a transaction around the reads.
 */
static inline int
rc_wan_nvram_copy_value(rc_wan_nvram_get_fn get_value, void *context,
			const char *param_name, char *destination,
			size_t destination_size)
{
	char *source = NULL;
	size_t length;
	int read_result, saved_errno, status;

	if (!get_value || !param_name || !param_name[0] ||
	    !destination || destination_size == 0)
		return EINVAL;
	destination[0] = '\0';

	errno = 0;
	read_result = get_value(context, param_name, &source);
	saved_errno = errno;
	status = rc_wan_nvram_read_status(read_result, saved_errno);
	if (status != 0)
		return status;
	if (read_result == 0)
		source = "";
	else if (!source)
		return EIO;

	length = strlen(source);
	if (length >= destination_size)
		return ENAMETOOLONG;
	memcpy(destination, source, length + 1);
	return 0;
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


/*
 * Clear independent runtime fields even after one write fails. Teardown must
 * still discard every stale route/DNS field while reporting the first error.
 */
static inline int
rc_wan_nvram_set_all_values(const struct rc_wan_nvram_value *values,
			    unsigned int count,
			    rc_wan_nvram_value_set_fn set_value,
			    void *context)
{
	unsigned int i;
	int status, first_status = 0;

	if ((!values && count != 0) || !set_value)
		return EINVAL;
	for (i = 0; i < count; ++i) {
		if (!values[i].param_name || !values[i].param_name[0] ||
		    !values[i].value)
			return EINVAL;
	}
	for (i = 0; i < count; ++i) {
		status = set_value(context, values[i].param_name, values[i].value);
		if (first_status == 0 && status != 0)
			first_status = status;
	}
	return first_status;
}


#endif /* PADAVAN_WAN_NVRAM_STATUS_POLICY_H */
