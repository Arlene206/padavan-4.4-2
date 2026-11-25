#ifndef __MODEM_SG_MODULE_POLICY_H__
#define __MODEM_SG_MODULE_POLICY_H__

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>

/*
 * A failed NVRAM read must not select the ZeroCD path by default. A missing
 * setting retains the historical zero default; a present value is parsed
 * completely before it is allowed to select the SCSI generic module path.
 */
static inline int
modem_sg_zero_cd_mode_status(int api_result, int saved_errno,
                             const char *value, int *use_sr_mod)
{
	char *end;
	long configured;

	if (!use_sr_mod)
		return EINVAL;
	*use_sr_mod = 0;

	/* nvram_get_checked(): 0=missing, 1=present, -1=backend error. */
	if (api_result == 0)
		return 0;
	if (api_result < 0)
		return saved_errno > 0 ? saved_errno : EIO;
	if (api_result != 1 || !value)
		return EIO;

	end = (char *)value;
	while (*end && isspace((unsigned char)*end))
		++end;
	if (!*end)
		return 0;

	errno = 0;
	configured = strtol(value, &end, 10);
	if (end == value)
		return EINVAL;
	if (errno == ERANGE || configured < INT_MIN || configured > INT_MAX)
		return ERANGE;
	while (*end && isspace((unsigned char)*end))
		++end;
	if (*end)
		return EINVAL;

	*use_sr_mod = configured != 0;
	return 0;
}

/* The module must be positively present after the load/already-loaded path. */
static inline int
modem_sg_module_ready_status(int module_presence)
{
	return (module_presence == 1) ? 0 : -1;
}

/* _eval() returns zero only when the ZeroCD helper exits successfully. */
static inline int
modem_sg_zerocd_command_status(int command_status)
{
	return (command_status == 0) ? 0 : -1;
}

#endif
