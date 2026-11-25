#ifndef ROUTER_RUNTIME_NVRAM_RESET_POLICY_H
#define ROUTER_RUNTIME_NVRAM_RESET_POLICY_H

#include <errno.h>
#include <stddef.h>

struct router_runtime_nvram_entry {
	const char *name;
	const char *string_value;
	int integer_value;
	int is_integer;
};

typedef int (*router_runtime_nvram_set_fn)(void *context,
					   const char *name,
					   const char *string_value,
					   int integer_value,
					   int is_integer);

/*
 * Clear all per-boot runtime markers even when one write fails, then return
 * the first failure and restore its errno. Startup must not proceed with a
 * partly reset event/reconnect state table.
 */
static inline int
router_runtime_nvram_reset_apply(
	const struct router_runtime_nvram_entry *entries, size_t count,
	router_runtime_nvram_set_fn set_value, void *context)
{
	size_t i;
	int status, saved_errno, first_status = 0, first_errno = 0;
	int failed = 0;

	if (!set_value)
		return EINVAL;
	if (count == 0)
		return 0;
	if (!entries)
		return EINVAL;

	/* Validate the whole batch before issuing any side effects. */
	for (i = 0; i < count; ++i) {
		if (!entries[i].name || !entries[i].name[0] ||
		    (entries[i].is_integer != 0 && entries[i].is_integer != 1) ||
		    (!entries[i].is_integer && !entries[i].string_value))
			return EINVAL;
	}

	for (i = 0; i < count; ++i) {
		errno = 0;
		status = set_value(context, entries[i].name,
				   entries[i].string_value,
				   entries[i].integer_value,
				   entries[i].is_integer);
		saved_errno = errno;
		if (status != 0 && !failed) {
			failed = 1;
			first_status = status;
			first_errno = saved_errno;
		}
	}

	if (failed)
		errno = first_errno;
	return first_status;
}

#endif /* ROUTER_RUNTIME_NVRAM_RESET_POLICY_H */
