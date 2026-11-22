#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "openvpn_ipv6_protocol_policy.h"

int main(void)
{
	int protocol;
	int status;

	protocol = 2;
	status = openvpn_ipv6_protocol_apply_service_state(&protocol, EIO, 0, 0);
	assert(status == EIO && protocol == 2);

	protocol = 3;
	status = openvpn_ipv6_protocol_apply_service_state(&protocol, EACCES, 0, 0);
	assert(status == EACCES && protocol == 3);

	protocol = 2;
	status = openvpn_ipv6_protocol_apply_service_state(&protocol, 0, 0, 0);
	assert(status == 0 && protocol == 0);

	protocol = 3;
	status = openvpn_ipv6_protocol_apply_service_state(&protocol, 0, 0, 0);
	assert(status == 0 && protocol == 1);

	protocol = 3;
	status = openvpn_ipv6_protocol_apply_service_state(&protocol, 0, 4, 0);
	assert(status == 0 && protocol == 3);

	protocol = 1;
	status = openvpn_ipv6_protocol_apply_service_state(&protocol, EIO, 0, 0);
	assert(status == 0 && protocol == 1);

	assert(openvpn_ipv6_protocol_apply_service_state(NULL, 0, 0, 0) == EINVAL);
	puts("OpenVPN IPv6 protocol failure policy: PASS");
	return 0;
}
