#ifndef PADAVAN_STORAGE_RWFS_MOUNT_POLICY_H
#define PADAVAN_STORAGE_RWFS_MOUNT_POLICY_H
#include <errno.h>
#include "storage_optware_start_policy.h"
typedef struct {
	void (*load_ubi_module)(void *context);
	int (*ubi_module_status)(void *context);
	int (*ubi_attachment_status)(void *context, int mtd_index, int *presence);
	int (*attach_ubi)(void *context, int mtd_index);
	int (*prepare_mountpoint)(void *context, const char *mountpoint);
	int (*mount_ubifs)(void *context, int mtd_index, const char *mountpoint);
} rc_rwfs_mount_ops_t;
static inline int
rc_rwfs_mount_execute(int enabled, int mtd_index, const char *mountpoint,
		      const rc_rwfs_mount_ops_t *ops, void *context)
{
	int module_status, presence, status, attach_status;
	if (!enabled || mtd_index < 3) return 0;
	if (!mountpoint || !*mountpoint || !ops || !ops->load_ubi_module ||
	    !ops->ubi_module_status || !ops->ubi_attachment_status ||
	    !ops->attach_ubi || !ops->prepare_mountpoint || !ops->mount_ubifs)
		return EINVAL;
	ops->load_ubi_module(context);
	module_status = ops->ubi_module_status(context);
	if (module_status < 0) return -module_status;
	if (module_status == 0) return ENODEV;
	if (module_status != 1) return EIO;
	status = ops->ubi_attachment_status(context, mtd_index, &presence);
	if (status != 0) return status;
	if (presence != STORAGE_OPTWARE_STOP_ABSENT &&
	    presence != STORAGE_OPTWARE_STOP_RUNNING) return EIO;
	if (presence == STORAGE_OPTWARE_STOP_ABSENT) {
		attach_status = ops->attach_ubi(context, mtd_index);
		status = ops->ubi_attachment_status(context, mtd_index, &presence);
		if (status != 0) return status;
		if (presence != STORAGE_OPTWARE_STOP_RUNNING)
			return attach_status != 0 ? attach_status : ENODEV;
	}
	status = ops->prepare_mountpoint(context, mountpoint);
	if (status != 0) return status;
	return ops->mount_ubifs(context, mtd_index, mountpoint);
}
#endif
