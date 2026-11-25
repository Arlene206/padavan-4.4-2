#ifndef RC_WIRELESS_MODULE_LOAD_POLICY_H
#define RC_WIRELESS_MODULE_LOAD_POLICY_H

#include <errno.h>
#include <stddef.h>

typedef struct {
	int (*load)(const char *module_name, const char *module_param);
	/* 1 = loaded, 0 = absent, negative errno = probe failed. */
	int (*is_loaded)(const char *module_name);
} rc_module_load_ops_t;

typedef rc_module_load_ops_t rc_wireless_module_load_ops_t;

/*
 * Only a confirmed ABSENT (0) sysfs state permits a new modprobe request.
 * A loaded module (1) or an unreadable/unknown state (negative errno) must
 * not be collapsed into the legacy boolean "not loaded" result.
 */
static inline int
rc_module_load_probe_allows_attempt(int module_status)
{
	return module_status == 0;
}

/*
 * module_smart_load() is not a reliable success indicator: it returns zero
 * both when a module is already present and when modprobe fails. The
 * /sys/module post-load state is authoritative; preserve module parameters.
 * A failed state probe is not proof that the module is absent.
 */
static inline int
rc_module_load_verified(const char *module_name, const char *module_param,
			const rc_module_load_ops_t *ops)
{
	int loaded;

	if (!module_name || module_name[0] == '\0' ||
	    !ops || !ops->load || !ops->is_loaded)
		return EINVAL;

	(void)ops->load(module_name, module_param);
	loaded = ops->is_loaded(module_name);
	if (loaded < 0)
		return -loaded;
	return loaded == 1 ? 0 : EIO;
}


typedef struct {
	const char *module_name;
	const char *module_param;
	int required;
} rc_module_load_requirement_t;

/* Try every configured requirement and retain the first load/probe failure. */
static inline int
rc_module_load_requirements_apply(const rc_module_load_requirement_t *requirements,
				  size_t requirement_count,
				  const rc_module_load_ops_t *ops)
{
	size_t i;
	int first_status = 0;

	if ((!requirements && requirement_count != 0) ||
	    !ops || !ops->load || !ops->is_loaded)
		return EINVAL;

	for (i = 0; i < requirement_count; ++i) {
		int status;
		if (!requirements[i].required)
			continue;
		status = rc_module_load_verified(requirements[i].module_name,
						 requirements[i].module_param, ops);
		if (first_status == 0 && status != 0)
			first_status = status;
	}
	return first_status;
}

typedef struct {
	int (*unload)(const char *module_name, int recurse_unload);
	/* 1 = loaded, 0 = absent, negative errno = probe failed. */
	int (*is_loaded)(const char *module_name);
} rc_module_unload_ops_t;

/*
 * module_smart_unload() can report that a module is busy without removing it.
 * Verify the postcondition instead of treating the command result as proof.
 */
static inline int
rc_module_unload_verified(const char *module_name, int recurse_unload,
			  const rc_module_unload_ops_t *ops)
{
	int loaded;

	if (!module_name || module_name[0] == '\0' ||
	    !ops || !ops->unload || !ops->is_loaded)
		return EINVAL;

	loaded = ops->is_loaded(module_name);
	if (loaded < 0)
		return -loaded;
	if (loaded == 0)
		return 0;

	(void)ops->unload(module_name, recurse_unload);
	loaded = ops->is_loaded(module_name);
	if (loaded < 0)
		return -loaded;
	return loaded ? EBUSY : 0;
}

/* Preserve the existing call shape for wireless-driver users. */
static inline int
rc_wireless_module_load_verified(const char *module_name,
				 const rc_wireless_module_load_ops_t *ops)
{
	return rc_module_load_verified(module_name, NULL, ops);
}

#endif
