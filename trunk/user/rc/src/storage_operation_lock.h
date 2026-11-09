#ifndef __STORAGE_OPERATION_LOCK_H__
#define __STORAGE_OPERATION_LOCK_H__

/*
 * All storage topology changes use one process-shared lock. Per-device
 * hotplug locks remain useful for duplicate events, but do not serialize
 * sibling partitions or user-initiated safe removal.
 */
#ifndef STORAGE_OPERATION_LOCK_TAG
#define STORAGE_OPERATION_LOCK_TAG "storage"
#endif

typedef int (*storage_operation_lock_fn_t)(char *);

static int
storage_operation_lock_acquire(storage_operation_lock_fn_t lock_fn)
{
	char tag[] = STORAGE_OPERATION_LOCK_TAG;

	if (!lock_fn)
		return -1;

	return lock_fn(tag);
}

#endif
