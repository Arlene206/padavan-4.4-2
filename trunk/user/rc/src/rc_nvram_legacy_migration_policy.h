#ifndef RC_NVRAM_LEGACY_MIGRATION_POLICY_H
#define RC_NVRAM_LEGACY_MIGRATION_POLICY_H
#include <errno.h>

struct rc_nvram_legacy_migration_write {
	const char *name;
	const char *value;
};
struct rc_nvram_legacy_migration_ops {
	int (*set)(void *context, const char *name, const char *value);
	int (*unset)(void *context, const char *name);
};

/* Publish every replacement before deleting the only remaining source key. */
static inline int
rc_nvram_legacy_publish_then_unset(
	const struct rc_nvram_legacy_migration_ops *ops,
	void *context, const char *source_key,
	const struct rc_nvram_legacy_migration_write *writes,
	unsigned int write_count, int *changed)
{
	unsigned int i;
	int status;

	if (!ops || !ops->unset || !source_key || !source_key[0] || !changed)
		return EINVAL;
	if (write_count && (!ops->set || !writes))
		return EINVAL;
	for (i = 0; i < write_count; ++i) {
		if (!writes[i].name || !writes[i].name[0] || !writes[i].value)
			return EINVAL;
		status = ops->set(context, writes[i].name, writes[i].value);
		if (status != 0)
			return status;
	}
	status = ops->unset(context, source_key);
	if (status != 0)
		return status;
	*changed = 1;
	return 0;
}
#endif
