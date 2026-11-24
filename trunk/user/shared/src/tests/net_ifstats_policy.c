#include "net_ifstats_policy.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void
expect_bad_interface(const char *ifname, int expected)
{
	char path[64];

	assert(net_ifstats_build_path(path, sizeof(path), ifname, "rx_bytes") == expected);
	assert(path[0] == '\0');
}

static void
expect_bad_counter(const char *text, int expected)
{
	uint64_t value = 123;

	assert(net_ifstats_parse_counter(text, &value) == expected);
	assert(value == 123);
}

static void
test_counter_delta(void)
{
	uint64_t delta = 99;

	assert(net_ifstats_counter_delta(100U, "40", &delta) == 0);
	assert(delta == 60U);
	assert(net_ifstats_counter_delta(10U, "40", &delta) == 0);
	assert(delta == 0U);
	/* Empty NVRAM baselines historically meant zero. */
	assert(net_ifstats_counter_delta(100U, "", &delta) == 0);
	assert(delta == 100U);
	assert(net_ifstats_counter_delta(100U, "40junk", &delta) == EINVAL);
	assert(delta == 0U);
	assert(net_ifstats_counter_delta(100U, "-1", &delta) == EINVAL);
	assert(delta == 0U);
	assert(net_ifstats_counter_delta(100U, "18446744073709551616", &delta) == ERANGE);
	assert(delta == 0U);
	assert(net_ifstats_counter_delta(100U, NULL, &delta) == EINVAL);
	assert(delta == 0U);
	assert(net_ifstats_counter_delta(100U, "40", NULL) == EINVAL);
}

static void
test_counter_pair_result(void)
{
	uint64_t rx = 0, tx = 34;

	/* A valid zero is a successful sample, not an I/O error. */
	assert(net_ifstats_pair_result(0, 0, &rx, &tx) == 0);
	assert(rx == 0 && tx == 34);

	rx = 12;
	tx = 34;
	assert(net_ifstats_pair_result(EIO, 0, &rx, &tx) == EIO);
	assert(rx == 0 && tx == 0);

	rx = 12;
	tx = 34;
	assert(net_ifstats_pair_result(0, ENOENT, &rx, &tx) == ENOENT);
	assert(rx == 0 && tx == 0);

	rx = 12;
	tx = 34;
	assert(net_ifstats_pair_result(EACCES, ENOMEM, &rx, &tx) == EACCES);
	assert(rx == 0 && tx == 0);

	tx = 34;
	assert(net_ifstats_pair_result(0, 0, NULL, &tx) == EINVAL);
	assert(tx == 0);
	rx = 12;
	assert(net_ifstats_pair_result(0, 0, &rx, &rx) == EINVAL);
	assert(rx == 0);
}

int
main(void)
{
	char path[64];
	char small_path[8];
	char long_ifname[IFNAMSIZ + 1];
	uint64_t value;

	assert(net_ifstats_build_path(path, sizeof(path), "eth0", "rx_bytes") == 0);
	assert(strcmp(path, "/sys/class/net/eth0/statistics/rx_bytes") == 0);
	assert(net_ifstats_build_path(path, sizeof(path), "ra0.1", "tx_bytes") == 0);
	assert(strcmp(path, "/sys/class/net/ra0.1/statistics/tx_bytes") == 0);
	assert(net_ifstats_build_path(path, sizeof(path), "wwan-1", "rx_bytes") == 0);

	expect_bad_interface(NULL, EINVAL);
	expect_bad_interface("", EINVAL);
	expect_bad_interface(".", EINVAL);
	expect_bad_interface("..", EINVAL);
	expect_bad_interface("../../etc/passwd", EINVAL);
	expect_bad_interface("eth0/../../proc", EINVAL);
	expect_bad_interface("eth0\\..\\proc", EINVAL);
	expect_bad_interface("eth 0", EINVAL);
	expect_bad_interface("eth0\n", EINVAL);
	expect_bad_interface("eth0$HOME", EINVAL);

	memset(long_ifname, 'a', IFNAMSIZ);
	long_ifname[IFNAMSIZ] = '\0';
	expect_bad_interface(long_ifname, ENAMETOOLONG);
	assert(net_ifstats_build_path(small_path, sizeof(small_path),
				      "eth0", "rx_bytes") == ENAMETOOLONG);
	assert(small_path[0] == '\0');
	assert(net_ifstats_build_path(NULL, sizeof(path), "eth0", "rx_bytes") == EINVAL);
	assert(net_ifstats_build_path(path, 0, "eth0", "rx_bytes") == EINVAL);
	assert(net_ifstats_build_path(path, sizeof(path), "eth0", "tx_packets") == EINVAL);

	test_counter_pair_result();
	test_counter_delta();
	assert(net_ifstats_parse_counter("0\n", &value) == 0 && value == 0);
	assert(net_ifstats_parse_counter("18446744073709551615\n", &value) == 0);
	assert(value == UINT64_MAX);
	assert(net_ifstats_parse_counter("4294967296\n", &value) == 0);
	assert(value == 4294967296ULL);
	assert(net_ifstats_parse_counter("123 \t\n", &value) == 0 && value == 123);

	expect_bad_counter(NULL, EINVAL);
	expect_bad_counter("", EINVAL);
	expect_bad_counter(" 1\n", EINVAL);
	expect_bad_counter("+1\n", EINVAL);
	expect_bad_counter("-1\n", EINVAL);
	expect_bad_counter("1x\n", EINVAL);
	expect_bad_counter("1\n2\n", EINVAL);
	expect_bad_counter("18446744073709551616\n", ERANGE);
	puts("sysfs WAN interface counter path and decimal parser policy: PASS");
	return 0;
}
