#ifndef RAETH_LAN_IP_IOCTL_POLICY_H
#define RAETH_LAN_IP_IOCTL_POLICY_H

#ifdef __KERNEL__
#include <linux/errno.h>
#else
#include <errno.h>
#endif

/*
 * The ioctl copies a fixed-width user buffer. Require a non-empty,
 * NUL-terminated address before treating it as a C string.
 */
static inline int
raeth_lan_ip_ioctl_status(const char *value, unsigned int length)
{
	unsigned int i;

	if (!value || length == 0 || value[0] == '\0')
		return -EINVAL;

	for (i = 0; i < length; ++i) {
		if (value[i] == '\0')
			return 0;
	}

	return -EINVAL;
}

#endif /* RAETH_LAN_IP_IOCTL_POLICY_H */
