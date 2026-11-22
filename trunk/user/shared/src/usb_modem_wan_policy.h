#ifndef USB_MODEM_WAN_POLICY_H
#define USB_MODEM_WAN_POLICY_H

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * Preserve nvram_get_int()/atoi() compatibility while reporting backend
 * errors that the legacy getter silently converts into a zero value.
 */
static inline int
usb_modem_wan_parse_devnum_status(int api_result, int saved_errno,
                                  const char *value, int *devnum)
{
	char *end;
	long parsed;

	if (!devnum)
		return EINVAL;
	*devnum = 0;
	if (api_result < 0)
		return saved_errno > 0 ? saved_errno : EIO;
	if (api_result == 0)
		return 0;
	if (api_result != 1 || !value)
		return EIO;

	errno = 0;
	parsed = strtol(value, &end, 10);
	if (end == value)
		return 0;
	if (errno == ERANGE || parsed < INT_MIN || parsed > INT_MAX)
		return ERANGE;

	/* atoi() accepts a numeric prefix; retain that legacy behavior. */
	*devnum = (int)parsed;
	return 0;
}

/*
 * Compatibility callers only have a boolean result. On an identity read
 * failure, choose the conservative modem branch instead of misclassifying
 * an unknown WAN as non-modem. Status-aware callers must use the API result.
 */
static inline int
usb_modem_wan_legacy_boolean(int status, int is_modem)
{
	return status != 0 ? 1 : (is_modem != 0);
}

typedef int (*usb_modem_wan_nvram_set_int_fn)(void *context,
                                              const char *name,
                                              int value);

static inline int
usb_modem_wan_format_nvram_key(int unit, char *key, size_t key_size)
{
	int length;

	if (unit < 0 || !key || key_size == 0)
		return EINVAL;

	key[0] = '\0';
	length = snprintf(key, key_size, "wan%d_modem_dev", unit);
	if (length < 0) {
		key[0] = '\0';
		return errno > 0 ? errno : EIO;
	}
	if ((size_t)length >= key_size) {
		key[0] = '\0';
		return ENAMETOOLONG;
	}
	return 0;
}

/* Return an errno-style status instead of discarding a failed temp write. */
static inline int
usb_modem_wan_publish_nvram(int unit, int devnum,
                            usb_modem_wan_nvram_set_int_fn set_value,
                            void *context)
{
	char key[32];
	int status, api_result, saved_errno;

	if (devnum < 0 || !set_value)
		return EINVAL;

	status = usb_modem_wan_format_nvram_key(unit, key, sizeof(key));
	if (status != 0)
		return status;

	errno = 0;
	api_result = set_value(context, key, devnum);
	saved_errno = errno;
	if (api_result == 0)
		return 0;
	return saved_errno > 0 ? saved_errno : EIO;
}

#endif /* USB_MODEM_WAN_POLICY_H */
