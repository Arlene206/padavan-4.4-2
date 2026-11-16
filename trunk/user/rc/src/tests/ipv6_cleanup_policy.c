#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "ipv6_cleanup_policy.h"

static int route_status;
static int address_status;
static int neighbor_status;
static int route_calls;
static int address_calls;
static int neighbor_calls;
static char call_order[4];
static char neighbor_ifname[16];

static int
mock_clear_routes(void)
{
	call_order[route_calls + address_calls + neighbor_calls] = 'r';
	++route_calls;
	return route_status;
}

static int
mock_clear_addresses(void)
{
	call_order[route_calls + address_calls + neighbor_calls] = 'a';
	++address_calls;
	return address_status;
}

static int
mock_clear_neighbors(char *ifname)
{
	unsigned int length;

	call_order[route_calls + address_calls + neighbor_calls] = 'n';
	++neighbor_calls;
	if (!ifname)
		return EINVAL;
	length = (unsigned int)strlen(ifname);
	if (length >= sizeof(neighbor_ifname))
		return ENAMETOOLONG;
	memcpy(neighbor_ifname, ifname, length + 1);
	return neighbor_status;
}

static void
reset_case(int route, int address, int neighbor)
{
	route_status = route;
	address_status = address;
	neighbor_status = neighbor;
	route_calls = 0;
	address_calls = 0;
	neighbor_calls = 0;
	memset(call_order, 0, sizeof(call_order));
	memset(neighbor_ifname, 0, sizeof(neighbor_ifname));
}

static void
test_all_cleanup_steps_run_and_first_error_is_preserved(void)
{
	reset_case(EIO, EPERM, EACCES);
	assert(rc_ipv6_disable_cleanup_apply(mock_clear_routes,
		mock_clear_addresses, mock_clear_neighbors, "br0") == EIO);
	assert(route_calls == 1 && address_calls == 1 && neighbor_calls == 1);
	assert(strcmp(call_order, "ran") == 0);
	assert(strcmp(neighbor_ifname, "br0") == 0);

	reset_case(0, 0, EPERM);
	assert(rc_ipv6_disable_cleanup_apply(mock_clear_routes,
		mock_clear_addresses, mock_clear_neighbors, "br0") == EPERM);
	assert(strcmp(call_order, "ran") == 0);
}

static void
test_invalid_callback_fails_before_any_cleanup(void)
{
	reset_case(0, 0, 0);
	assert(rc_ipv6_disable_cleanup_apply(NULL, mock_clear_addresses,
		mock_clear_neighbors, "br0") == EINVAL);
	assert(route_calls == 0 && address_calls == 0 && neighbor_calls == 0);
	assert(rc_ipv6_disable_cleanup_apply(mock_clear_routes,
		NULL, mock_clear_neighbors, "br0") == EINVAL);
	assert(route_calls == 0 && address_calls == 0 && neighbor_calls == 0);
}

static void
test_invalid_bridge_is_reported_after_global_cleanup(void)
{
	reset_case(0, 0, 0);
	assert(rc_ipv6_disable_cleanup_apply(mock_clear_routes,
		mock_clear_addresses, mock_clear_neighbors, NULL) == EINVAL);
	assert(route_calls == 1 && address_calls == 1 && neighbor_calls == 1);
	assert(strcmp(call_order, "ran") == 0);
}

int
main(void)
{
	test_all_cleanup_steps_run_and_first_error_is_preserved();
	test_invalid_callback_fails_before_any_cleanup();
	test_invalid_bridge_is_reported_after_global_cleanup();
	puts("IPv6 global cleanup result propagation policy: PASS");
	return 0;
}
