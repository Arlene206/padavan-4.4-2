#ifndef RC_PAGECACHE_RECLAIM_POLICY_H
#define RC_PAGECACHE_RECLAIM_POLICY_H

#include <errno.h>
#include <stdlib.h>

/*
 * nvram_get_checked(): 1 = present, 0 = absent, -1 = read failure.
 * Missing settings retain the historical default of mode 0 (ratio 100).
 */
static inline int
rc_pagecache_reclaim_read_status(int api_result, int saved_errno,
                                 const char *value, int *reclaim_mode)
{
	if (!reclaim_mode)
		return EINVAL;

	*reclaim_mode = 0;
	if (api_result == 0)
		return 0;
	if (api_result == 1) {
		if (!value)
			return EIO;
		/* Keep nvram_get_int()'s legacy atoi() interpretation. */
		*reclaim_mode = atoi(value);
		return 0;
	}
	if (api_result == -1)
		return saved_errno > 0 ? saved_errno : EIO;

	return EIO;
}

static inline int
rc_pagecache_reclaim_ratio(int reclaim_mode)
{
	switch (reclaim_mode) {
	case 1:
		return 30;
	case 2:
		return 50;
	case 3:
		return 70;
	case 4:
		return 85;
	default:
		return 100;
	}
}

/* fput_int() returns an errno-style status and also preserves errno. */
static inline int
rc_pagecache_reclaim_write_status(int operation_status, int saved_errno)
{
	if (operation_status == 0)
		return 0;
	if (operation_status > 0)
		return operation_status;
	return saved_errno > 0 ? saved_errno : EIO;
}

#endif /* RC_PAGECACHE_RECLAIM_POLICY_H */
