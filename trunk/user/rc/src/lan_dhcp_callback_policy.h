#ifndef PADAVAN_LAN_DHCP_CALLBACK_POLICY_H
#define PADAVAN_LAN_DHCP_CALLBACK_POLICY_H

/*
 * BusyBox commits its internal DHCP lease only when the bound/renew script
 * exits successfully. The address, route, resolver and NVRAM transaction
 * has already committed before ancillary host, IAPP and networkmap refreshes
 * run. Failures of those optional follow-up operations are logged by the caller,
 * but must not make BusyBox retain an old lease while the router uses the new one.
 */
/* Retry ancillary host publication after a same-value RENEW only when an
 * earlier publication failed; do not repeat it for every healthy renewal. */
static inline int
lan_dhcp_hosts_refresh_required(int lease_changed, int is_renew,
				int retry_pending)
{
	return lease_changed || !is_renew || retry_pending;
}


/*
 * Host-file publication is post-commit: either a persistent same-value
 * renewal marker or an immediate PID 1 notification is sufficient to retry.
 * Return an error only when both independent recovery paths failed.
 */
static inline int
lan_dhcp_retry_schedule_status(int marker_status, int notification_status)
{
	if (marker_status == 0 || notification_status == 0)
		return 0;
	return marker_status != 0 ? marker_status : notification_status;
}

static inline int
lan_dhcp_bound_callback_status(int transaction_status, int ancillary_status)
{
	(void)ancillary_status;
	return transaction_status;
}

#endif /* PADAVAN_LAN_DHCP_CALLBACK_POLICY_H */
