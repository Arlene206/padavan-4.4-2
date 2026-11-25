#ifndef WAN_VLAN_VID_POLICY_H
#define WAN_VLAN_VID_POLICY_H

/*
 * get_vlan_vid_wan() returns a VLAN ID or a negative errno to its caller.
 * NVRAM checked-read APIs report positive errno values; preserve that
 * established return convention without treating a read failure as a
 * disabled filter and falling back to the physical switch PVID.
 */
static inline int
wan_vlan_vid_policy_read_failure_result(int read_status)
{
	return read_status > 0 ? -read_status : read_status;
}

#endif /* WAN_VLAN_VID_POLICY_H */
