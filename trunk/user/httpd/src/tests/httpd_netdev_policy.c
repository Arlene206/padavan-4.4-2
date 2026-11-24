#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>

#include "../httpd_netdev_policy.h"

int main(void)
{
	uint64_t rx = 0x1122334455667788ULL;
	uint64_t tx = 0x8877665544332211ULL;

	assert(httpd_netdev_status_from_errno(ENODEV) == ENODEV);
	assert(httpd_netdev_status_from_errno(0) == EIO);

	assert(httpd_netdev_parse_counters(
		"100 2 3 4 5 6 7 8 200 9 10 11 12 13 14 15\n", &rx, &tx) == 0);
	assert(rx == 100 && tx == 200);

	assert(httpd_netdev_parse_counters(
		"0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0", &rx, &tx) == 0);
	assert(rx == 0 && tx == 0);

	assert(httpd_netdev_parse_counters(
		"18446744073709551615 0 0 0 0 0 0 0 18446744073709551614 0 0 0 0 0 0 0",
		&rx, &tx) == 0);
	assert(rx == UINT64_MAX && tx == UINT64_MAX - 1);

	assert(httpd_netdev_parse_counters(
		"100 2 3 4 5 6 7 8 200 9 10 11 12 13 14", &rx, &tx) == EPROTO);
	assert(httpd_netdev_parse_counters(
		"100 2 3 4 5 6 7 8 bad 9 10 11 12 13 14 15", &rx, &tx) == EPROTO);
	assert(httpd_netdev_parse_counters(
		"100x 2 3 4 5 6 7 8 200 9 10 11 12 13 14 15", &rx, &tx) == EPROTO);
	assert(httpd_netdev_parse_counters(
		"18446744073709551616 0 0 0 0 0 0 0 200 9 10 11 12 13 14 15",
		&rx, &tx) == ERANGE);
	assert(httpd_netdev_parse_counters(
		"100 2 3 4 5 6 7 8 200 9 10 11 12 13 14 15 999", &rx, &tx) == EPROTO);
	assert(httpd_netdev_parse_counters(NULL, &rx, &tx) == EINVAL);
	assert(httpd_netdev_parse_counters("0", NULL, &tx) == EINVAL);
	assert(httpd_netdev_parse_counters("0", &rx, NULL) == EINVAL);

	rx = 0x1122334455667788ULL;
	tx = 0x8877665544332211ULL;
	assert(httpd_netdev_parse_counters("0 0", &rx, &tx) == EPROTO);
	assert(rx == 0x1122334455667788ULL);
	assert(tx == 0x8877665544332211ULL);

	puts("HTTPD netdev counter parsing and fail-closed status policy: PASS");
	return 0;
}
