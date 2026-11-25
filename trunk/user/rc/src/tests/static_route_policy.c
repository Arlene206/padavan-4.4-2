#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>

#include "static_route_policy.h"

int
main(void)
{
	int metric, enabled = 17;

	/* Missing sr_enable_x keeps the legacy disabled setting. */
	assert(static_route_nvram_enabled_status(0, 0, NULL, 0, &enabled) == 0);
	assert(enabled == 0);
	assert(static_route_nvram_enabled_status(0, 0, NULL, 1, &enabled) == 0);
	assert(enabled == 1);
	assert(static_route_nvram_enabled_status(1, 0, "1", 0, &enabled) == 0);
	assert(enabled == 1);
	assert(static_route_nvram_enabled_status(1, 0, "0", 0, &enabled) == 0);
	assert(enabled == 0);
	assert(static_route_nvram_enabled_status(1, 0, "1x", 0, &enabled) == 0);
	assert(enabled == 0);
	assert(static_route_nvram_enabled_status(0, 0, "unexpected", 0, &enabled) == EIO);
	assert(static_route_nvram_enabled_status(2, 0, "1", 0, &enabled) == EIO);
	assert(static_route_nvram_enabled_status(1, 0, NULL, 0, &enabled) == EIO);
	assert(static_route_nvram_enabled_status(-1, EACCES, NULL, 0, &enabled) == EACCES);
	assert(enabled == 0);
	assert(static_route_nvram_enabled_status(-1, 0, NULL, 0, &enabled) == EIO);
	assert(static_route_nvram_enabled_status(1, 0, "1", 0, NULL) == EINVAL);

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

	assert(static_route_interface_is_safe(""));
	assert(static_route_interface_is_safe("LAN"));
	assert(static_route_interface_is_safe("MAN"));
	assert(static_route_interface_is_safe("WAN"));
	assert(!static_route_interface_is_safe(NULL));

	assert(static_route_ipv4_address_is_valid("192.168.1.1"));
	assert(static_route_ipv4_address_is_valid("0.0.0.0"));
	assert(static_route_ipv4_address_is_valid("255.255.255.255"));
	assert(!static_route_ipv4_address_is_valid(""));
	assert(!static_route_ipv4_address_is_valid(NULL));
	assert(!static_route_ipv4_address_is_valid("256.1.1.1"));
	assert(!static_route_ipv4_address_is_valid("1.2.3"));
	assert(!static_route_ipv4_address_is_valid("1.2.3.4.5"));
	assert(!static_route_ipv4_address_is_valid("01.2.3.4"));
	assert(!static_route_ipv4_address_is_valid("1.02.3.4"));
	assert(!static_route_ipv4_address_is_valid("000.0.0.0"));
	assert(!static_route_ipv4_address_is_valid("1.2.3.4:COMMIT"));
	assert(!static_route_ipv4_address_is_valid("1.2.3.4\\n"));
	assert(!static_route_interface_is_safe("WAN;DROP"));

	puts("static_route_policy: PASS");
	return 0;
}
