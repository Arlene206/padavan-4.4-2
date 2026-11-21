#ifndef STORAGE_PERSISTENCE_POLICY_H
#define STORAGE_PERSISTENCE_POLICY_H
/* Do not snapshot storage until the optional system-time file was saved. */
static inline int
storage_persistence_mtd_may_run(int system_time_status)
{
	return system_time_status == 0;
}
/* Firmware may be flashed only after both persistence steps succeeded. */
static inline int
storage_persistence_complete(int system_time_status, int mtd_status)
{
	return storage_persistence_mtd_may_run(system_time_status) && mtd_status == 0;
}
#endif
