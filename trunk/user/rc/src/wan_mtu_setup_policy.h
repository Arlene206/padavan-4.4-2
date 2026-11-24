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

/*
 * DHCP option 26 specifies the interface MTU. The DHCP parser in this
 * firmware accepts 576..1500; explicit 1500 restores Ethernet default after
 * a smaller lease MTU. This is separate from the narrower static/UI policy.
 */
static inline int
wan_dhcp_mtu_is_supported(int dhcp_mtu)
{
	return dhcp_mtu >= 576 && dhcp_mtu <= 1500;
}

#endif /* PADAVAN_WAN_MTU_SETUP_POLICY_H */
