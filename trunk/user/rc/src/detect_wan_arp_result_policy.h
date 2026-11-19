#ifndef DETECT_WAN_ARP_RESULT_POLICY_H
#define DETECT_WAN_ARP_RESULT_POLICY_H
#include <errno.h>
#define DETECT_WAN_ARP_PROBE_ERROR (-1)
#define DETECT_WAN_ARP_PROBE_NO_REPLY 0
#define DETECT_WAN_ARP_PROBE_REPLY 1
/* Local socket/I/O failure is not evidence that the gateway did not answer. */
static inline int
detect_wan_arp_probe_result(int reply_received, int operation_error)
{
    if (operation_error != 0) {
        errno = operation_error > 0 ? operation_error : EIO;
        return DETECT_WAN_ARP_PROBE_ERROR;
    }
    return reply_received ? DETECT_WAN_ARP_PROBE_REPLY :
                            DETECT_WAN_ARP_PROBE_NO_REPLY;
}
/* Renewal needs three real no-reply probes, no local errors, and link-up. */
static inline int
detect_wan_arp_should_renew(int retries_exhausted, int probe_count,
                            int required_probes, int ap_mode, int link_up,
                            int probe_error)
{
    return retries_exhausted && required_probes > 0 &&
           probe_count == required_probes && !probe_error &&
           (ap_mode || link_up);
}
#endif /* DETECT_WAN_ARP_RESULT_POLICY_H */
