#ifndef PADAVAN_WAN_MTU_SETUP_POLICY_H
#define PADAVAN_WAN_MTU_SETUP_POLICY_H

/*
 * Static IPoE applies a valid custom MTU in its address transaction.
 * Avoid a redundant default-MTU ioctl in that case; all other WAN modes
 * still attempt the common 1500-byte physical-interface setup.
 */
static inline int
wan_default_mtu_should_be_set(int is_static_ipoe, int configured_mtu)
{
	return !(is_static_ipoe && configured_mtu >= 1300 && configured_mtu < 1500);
}

#endif /* PADAVAN_WAN_MTU_SETUP_POLICY_H */
