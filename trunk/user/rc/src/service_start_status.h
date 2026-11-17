#ifndef RC_SERVICE_START_STATUS_H
#define RC_SERVICE_START_STATUS_H

/* Keep the first nonzero boot-service status without skipping later starts. */
static inline int
rc_first_service_start_status(int first_status, int next_status)
{
	return first_status != 0 ? first_status : next_status;
}

/* A queued storage event is acknowledged only after required starts pass. */
static inline int
rc_service_start_event_is_complete(int start_status)
{
	return start_status == 0;
}

/*
 * A WAN protocol client must not expose a path until the required firewall
 * rebuild, including configured extension hooks and snapshot cleanup, succeeds.
 */
static inline int
rc_wan_firewall_start_allows_protocol(int firewall_status)
{
	return firewall_status == 0;
}

/*
 * A static IPoE address that could not be cleared after EAPOL startup failure
 * must not remain on an administratively up physical WAN interface.
 */
static inline int
rc_wan_static_ipoe_clear_failure_requires_ifdown(int is_static_ipoe, int clear_status)
{
	return is_static_ipoe && clear_status != 0;
}

#endif
