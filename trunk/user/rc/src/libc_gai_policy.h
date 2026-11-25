#ifndef PADAVAN_LIBC_GAI_POLICY_H
#define PADAVAN_LIBC_GAI_POLICY_H

#include <errno.h>

typedef int (*rc_libc_gai_operation_fn)(void *context);

/* Removing an absent override is the already-achieved disabled state. */
static int
rc_libc_gai_apply(int ipv4_first,
		  rc_libc_gai_operation_fn write_operation,
		  rc_libc_gai_operation_fn unlink_operation,
		  void *context)
{
	int status;

	if (ipv4_first) {
		if (!write_operation)
			return EINVAL;
		return write_operation(context);
	}

	if (!unlink_operation)
		return EINVAL;
	status = unlink_operation(context);
	return status == ENOENT ? 0 : status;
}

#endif /* PADAVAN_LIBC_GAI_POLICY_H */
