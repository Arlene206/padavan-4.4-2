#include <stdio.h>

#include "dhcp6c_script_retry_policy.h"

#define CHECK(condition) do { \
	if (!(condition)) { \
		fprintf(stderr, "%s:%d: check failed: %s\n", __FILE__, __LINE__, #condition); \
		return 1; \
	} \
} while (0)

static int test_success_does_not_retry(void)
{
	CHECK(!dhcp6c_script_retry_required(0));
	return 0;
}

static int test_positive_exit_status_retries(void)
{
	CHECK(dhcp6c_script_retry_required(1));
	CHECK(dhcp6c_script_retry_required(2));
	CHECK(dhcp6c_script_retry_required(255));
	return 0;
}

static int test_spawn_failure_retries(void)
{
	CHECK(dhcp6c_script_retry_required(-1));
	return 0;
}

static int test_retry_delay_is_bounded_and_nonzero(void)
{
	CHECK(DHCP6C_SCRIPT_RETRY_DELAY_MS == 5000ULL);
	CHECK(DHCP6C_SCRIPT_RETRY_DELAY_MS > 0);
	return 0;
}

int main(void)
{
	if (test_success_does_not_retry() != 0 ||
	    test_positive_exit_status_retries() != 0 ||
	    test_spawn_failure_retries() != 0 ||
	    test_retry_delay_is_bounded_and_nonzero() != 0)
		return 1;

	puts("DHCPv6 configuration-script retry policy: PASS");
	return 0;
}
