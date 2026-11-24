#ifndef PADAVAN_SAMBA_CONFIG_MODE_POLICY_H
#define PADAVAN_SAMBA_CONFIG_MODE_POLICY_H

#include <errno.h>

/* Values supported by write_smb_conf_unlocked(); zero means disabled. */
static inline int
samba_config_mode_is_valid(int mode)
{
	return mode == 0 || mode == 1 || mode == 3 || mode == 4;
}

/* st_samba_lmb uses 0=off, 1=local master, 2=domain/preferred master. */
static inline int
samba_local_master_value_status(int read_status, int value, int *mode)
{
	if (!mode)
		return EINVAL;
	*mode = 0;
	if (read_status != 0)
		return read_status > 0 ? read_status : EIO;
	if (value < 0 || value > 2)
		return EINVAL;
	*mode = value;
	return 0;
}

/* A failed read is never interpreted as an explicit false value. */
static inline int
samba_boolean_value_status(int read_status, int value, int *enabled)
{
	if (!enabled)
		return EINVAL;
	*enabled = 0;
	if (read_status != 0)
		return read_status > 0 ? read_status : EIO;
	if (value != 0 && value != 1)
		return EINVAL;
	*enabled = value;
	return 0;
}

/* Disabled global state short-circuits validation of the share mode. */
static inline int
samba_runtime_enabled_status(int enable_read_status, int enable_value,
			     int mode_read_status, int mode_value,
			     int *enabled)
{
	int global_enabled;
	int status;

	if (!enabled)
		return EINVAL;
	*enabled = 0;
	status = samba_boolean_value_status(enable_read_status,
					    enable_value, &global_enabled);
	if (status != 0 || !global_enabled)
		return status;
	if (mode_read_status != 0)
		return mode_read_status > 0 ? mode_read_status : EIO;
	if (!samba_config_mode_is_valid(mode_value))
		return EINVAL;
	*enabled = mode_value != 0;
	return 0;
}

#endif /* PADAVAN_SAMBA_CONFIG_MODE_POLICY_H */
