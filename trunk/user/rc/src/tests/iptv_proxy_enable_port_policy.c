#include <assert.h>
#include <errno.h>
#include <limits.h>

#include "iptv_proxy_enable_port_policy.h"

static void
check_port(int configured_port, int expected_status, int expected_enabled)
{
	int enabled = 1;
	int status;

	status = iptv_proxy_enable_port_status(configured_port, &enabled);
	assert(status == expected_status);
	assert(enabled == expected_enabled);
}

int
main(void)
{
	check_port(INT_MIN, ERANGE, 0);
	check_port(-1024, ERANGE, 0);
	check_port(-1, ERANGE, 0);
	check_port(0, 0, 0);
	check_port(1, 0, 0);
	check_port(1023, 0, 0);
	check_port(IPTV_PROXY_ENABLE_PORT_MIN, 0, 1);
	check_port(8080, 0, 1);
	check_port(IPTV_PROXY_ENABLE_PORT_MAX, 0, 1);
	check_port(IPTV_PROXY_ENABLE_PORT_MAX + 1, ERANGE, 0);
	check_port(INT_MAX, ERANGE, 0);
	assert(iptv_proxy_enable_port_status(8080, NULL) == EINVAL);

	return 0;
}
