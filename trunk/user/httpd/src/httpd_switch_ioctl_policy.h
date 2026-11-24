#ifndef _HTTPD_SWITCH_IOCTL_POLICY_H_
#define _HTTPD_SWITCH_IOCTL_POLICY_H_

#include <errno.h>
#include <stdint.h>

static inline int httpd_switch_normalize_result(int result, int saved_errno)
{
	if (result >= 0)
		return 0;

	return -(saved_errno > 0 ? saved_errno : EIO);
}

static inline int httpd_switch_choose_result(int operation_result, int close_result)
{
	return (operation_result != 0) ? operation_result : close_result;
}

static inline int httpd_switch_publish_port_bytes(int status,
						   uint64_t sample_rx,
						   uint64_t sample_tx,
						   uint64_t *rx,
						   uint64_t *tx)
{
	if (!rx || !tx)
		return -EINVAL;
	if (status != 0)
		return status;

	*rx = sample_rx;
	*tx = sample_tx;
	return 0;
}

#endif
