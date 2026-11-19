#include <errno.h>
#include <stdio.h>
#include "detect_wan_arp_result_policy.h"
#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
        return 1; \
    } \
} while (0)
static int
test_probe_result_keeps_local_errors_distinct(void)
{
    errno = 0;
    CHECK(detect_wan_arp_probe_result(1, 0) == DETECT_WAN_ARP_PROBE_REPLY);
    CHECK(detect_wan_arp_probe_result(0, 0) == DETECT_WAN_ARP_PROBE_NO_REPLY);
    errno = 0;
    CHECK(detect_wan_arp_probe_result(0, EACCES) == DETECT_WAN_ARP_PROBE_ERROR);
    CHECK(errno == EACCES);
    errno = 0;
    CHECK(detect_wan_arp_probe_result(1, EIO) == DETECT_WAN_ARP_PROBE_ERROR);
    CHECK(errno == EIO);
    errno = 0;
    CHECK(detect_wan_arp_probe_result(0, -1) == DETECT_WAN_ARP_PROBE_ERROR);
    CHECK(errno == EIO);
    return 0;
}
static int
test_dhcp_renew_requires_three_real_no_reply_probes(void)
{
    CHECK(detect_wan_arp_should_renew(1, 3, 3, 0, 1, 0));
    CHECK(detect_wan_arp_should_renew(1, 3, 3, 1, 0, 0));
    CHECK(!detect_wan_arp_should_renew(0, 3, 3, 0, 1, 0));
    CHECK(!detect_wan_arp_should_renew(1, 2, 3, 0, 1, 0));
    CHECK(!detect_wan_arp_should_renew(1, 4, 3, 0, 1, 0));
    CHECK(!detect_wan_arp_should_renew(1, 3, 3, 0, 0, 0));
    CHECK(!detect_wan_arp_should_renew(1, 3, 3, 0, 1, EIO));
    return 0;
}
int
main(void)
{
    CHECK(test_probe_result_keeps_local_errors_distinct() == 0);
    CHECK(test_dhcp_renew_requires_three_real_no_reply_probes() == 0);
    puts("WAN ARP probe local-error and DHCP-renewal policy: PASS");
    return 0;
}
