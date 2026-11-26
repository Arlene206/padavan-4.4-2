#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "../dns-forwarder-1.2.1/hev-dns-forwarder-policy.h"

static void
test_udp_readiness_drain_policy(void)
{
	assert(HEV_DNS_FORWARDER_UDP_DRAIN_PACKET ==
	       hev_dns_forwarder_udp_drain_action(0, 0));
	assert(HEV_DNS_FORWARDER_UDP_DRAIN_PACKET ==
	       hev_dns_forwarder_udp_drain_action(512, 0));
	assert(HEV_DNS_FORWARDER_UDP_DRAIN_RETRY ==
	       hev_dns_forwarder_udp_drain_action(-1, EINTR));
	assert(HEV_DNS_FORWARDER_UDP_DRAIN_STOP ==
	       hev_dns_forwarder_udp_drain_action(-1, EAGAIN));
	assert(HEV_DNS_FORWARDER_UDP_DRAIN_STOP ==
	       hev_dns_forwarder_udp_drain_action(-1, EWOULDBLOCK));
	assert(HEV_DNS_FORWARDER_UDP_DRAIN_ERROR ==
	       hev_dns_forwarder_udp_drain_action(-1, EBADF));
}

int
main (void)
{
	unsigned short port = 4321;

	test_udp_readiness_drain_policy();

	assert(0 == hev_dns_forwarder_parse_port("1", &port));
	assert(1 == port);
	assert(0 == hev_dns_forwarder_parse_port("53", &port));
	assert(53 == port);
	assert(0 == hev_dns_forwarder_parse_port("00053", &port));
	assert(53 == port);
	assert(0 == hev_dns_forwarder_parse_port("65535", &port));
	assert(65535 == port);

	port = 4321;
	assert(EINVAL == hev_dns_forwarder_parse_port(NULL, &port));
	assert(4321 == port);
	assert(EINVAL == hev_dns_forwarder_parse_port("", &port));
	assert(EINVAL == hev_dns_forwarder_parse_port("0", &port));
	assert(EINVAL == hev_dns_forwarder_parse_port("65536", &port));
	assert(EINVAL == hev_dns_forwarder_parse_port("-1", &port));
	assert(EINVAL == hev_dns_forwarder_parse_port("+53", &port));
	assert(EINVAL == hev_dns_forwarder_parse_port("53 ", &port));
	assert(EINVAL == hev_dns_forwarder_parse_port("53x", &port));
	assert(EINVAL == hev_dns_forwarder_parse_port(" 53", &port));
	assert(ERANGE == hev_dns_forwarder_parse_port(
		"999999999999999999999999999999999999999999999999999999999999",
		&port));
	assert(4321 == port);

	puts("DNS Forwarder upstream port parser: PASS");
	return 0;
}
