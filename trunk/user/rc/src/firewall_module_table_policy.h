#ifndef PADAVAN_FIREWALL_MODULE_TABLE_POLICY_H
#define PADAVAN_FIREWALL_MODULE_TABLE_POLICY_H
#include <errno.h>
#include <limits.h>

/* Probe contract: 1=loaded, 0=absent, negative errno=UNKNOWN. */
static inline int
rc_firewall_module_table_restore_decide(int probe_status, int force_restore,
					int *should_restore)
{
	if (!should_restore)
		return EINVAL;
	*should_restore = 0;
	if (probe_status == INT_MIN)
		return EIO;
	if (probe_status < 0)
		return -probe_status;
	if (probe_status > 1)
		return EIO;
	*should_restore = force_restore || probe_status == 1;
	return 0;
}
#endif /* PADAVAN_FIREWALL_MODULE_TABLE_POLICY_H */
