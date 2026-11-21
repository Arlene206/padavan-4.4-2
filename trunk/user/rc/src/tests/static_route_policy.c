#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>

#include "static_route_policy.h"

int
main(void)
{
	int metric;

	assert(static_route_entry_count_is_safe(0));
	assert(static_route_entry_count_is_safe(64));
	assert(static_route_entry_count_is_safe(128));
	assert(!static_route_entry_count_is_safe(-1));
	assert(!static_route_entry_count_is_safe(129));
	assert(STATIC_ROUTE_MAX_SNAPSHOT_SIZE == 7552);
	assert(STATIC_ROUTE_MAX_SNAPSHOT_SIZE < 8192);

	assert(static_route_metric_parse("", &metric) == 0 && metric == 0);
	assert(static_route_metric_parse("0", &metric) == 0 && metric == 0);
	assert(static_route_metric_parse("123", &metric) == 0 && metric == 123);
	assert(static_route_metric_parse("2147483646", &metric) == 0 &&
	       metric == INT_MAX - 1);
	assert(static_route_metric_parse("-1", &metric) == EINVAL);
	assert(static_route_metric_parse("+1", &metric) == EINVAL);
	assert(static_route_metric_parse("12x", &metric) == EINVAL);
	assert(static_route_metric_parse("2147483647", &metric) == ERANGE);
	assert(static_route_metric_parse("999999999999", &metric) == ERANGE);
	assert(static_route_metric_parse("1", NULL) == EINVAL);

	assert(static_route_ipv4_field_is_safe(""));
	assert(static_route_ipv4_field_is_safe("192.168.1.1"));
	assert(static_route_ipv4_field_is_safe("255.255.255.255"));
	assert(!static_route_ipv4_field_is_safe(NULL));
	assert(!static_route_ipv4_field_is_safe("1234567890123456"));
	assert(!static_route_ipv4_field_is_safe("192.168.1.1:COMMIT"));
	assert(!static_route_ipv4_field_is_safe("192.168.1.1\n"));

	assert(static_route_interface_is_safe(""));
	assert(static_route_interface_is_safe("LAN"));
	assert(static_route_interface_is_safe("MAN"));
	assert(static_route_interface_is_safe("WAN"));
	assert(!static_route_interface_is_safe(NULL));
	assert(!static_route_interface_is_safe("WAN;DROP"));

	puts("static_route_policy: PASS");
	return 0;
}
