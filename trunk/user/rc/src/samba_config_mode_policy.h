#ifndef PADAVAN_SAMBA_CONFIG_MODE_POLICY_H
#define PADAVAN_SAMBA_CONFIG_MODE_POLICY_H

/* Values supported by write_smb_conf_unlocked(); zero means disabled. */
static int
samba_config_mode_is_valid(int mode)
{
	return mode == 0 || mode == 1 || mode == 3 || mode == 4;
}

#endif /* PADAVAN_SAMBA_CONFIG_MODE_POLICY_H */
