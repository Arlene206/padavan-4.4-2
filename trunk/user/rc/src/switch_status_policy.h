#ifndef SWITCH_STATUS_POLICY_H
#define SWITCH_STATUS_POLICY_H

#include <errno.h>
#include <stdint.h>

/*
 * Switch ioctl wrappers use zero for success and positive errno values for
 * open/ioctl/close failures. The rstats caller uses the usual negative-error
 * convention, so normalize errors before returning and publish counters only
 * after a completely successful request.
 */
static inline int
switch_status_publish_port_bytes(int ioctl_result, uint64_t received,
                                 uint64_t transmitted, uint64_t *rx,
                                 uint64_t *tx)
{
    if (!rx || !tx)
        return -EINVAL;

    if (ioctl_result != 0)
        return (ioctl_result > 0) ? -ioctl_result : ioctl_result;

    *rx = received;
    *tx = transmitted;
    return 0;
}

/*
 * The public WAN-PVID getter returns the VLAN ID on success and a negative
 * errno on device/open/ioctl/close failure. Do not turn a failed read into
 * the caller's plausible default VLAN ID.
 */
static inline int
switch_status_read_vlan_pvid(int ioctl_result, unsigned int pvid)
{
    if (ioctl_result != 0)
        return (ioctl_result > 0) ? -ioctl_result : ioctl_result;

    return (int)pvid;
}


/*
 * Link-change notifications are a hint to refresh the sampled WAN/LAN state.
 * A failed notification read must trigger a refresh, and any failed sample
 * keeps the refresh pending so a transient switch ioctl error cannot leave
 * detect_link with stale state until some unrelated future link transition.
 */
/* Resolve an explicit VLAN ID or a validated switch PVID fallback. */
static inline int
switch_status_resolve_vlan_vid(int configured_vid, int configured_valid,
                               int pvid_result, int pvid_valid,
                               int *resolved_vid)
{
    if (!resolved_vid)
        return -EINVAL;
    if (configured_valid) {
        *resolved_vid = configured_vid;
        return 0;
    }
    if (pvid_result < 0)
        return pvid_result;
    if (!pvid_valid)
        return -EINVAL;
    *resolved_vid = pvid_result;
    return 0;
}

/* A failed PVID sample is not a VLAN change and must not trigger a WAN restart. */
static inline int
switch_status_vlan_pvid_changed(int previous_result, int current_result)
{
    return (previous_result >= 0 && current_result >= 0 &&
            previous_result != current_result);
}

static inline int
switch_status_link_refresh_required(int initialized, int retry_pending,
                                    int changed_result,
                                    unsigned int link_changed)
{
    return (!initialized || retry_pending ||
            changed_result != 0 || link_changed != 0);
}

static inline int
switch_status_link_refresh_failed(int changed_result, int wan_result,
                                  int lan_result)
{
    return (changed_result != 0 || wan_result < 0 || lan_result != 0);
}

#endif /* SWITCH_STATUS_POLICY_H */
