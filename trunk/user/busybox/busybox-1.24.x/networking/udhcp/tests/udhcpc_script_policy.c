#include <stdio.h>

#include "udhcpc_script_policy.h"

#define CHECK(condition) do { \
	if (!(condition)) { \
		fprintf(stderr, "%s:%d: check failed: %s\n", __FILE__, __LINE__, #condition); \
		return 1; \
	} \
} while (0)

static int test_initial_acquisition_retries_selected_request(void)
{
	int timeout = 90;
	int packet_num = 3;
	unsigned already_waited_sec = 12;

	udhcpc_script_failure_prepare_retry(1, 4, 20, &timeout,
					   &already_waited_sec, &packet_num);
	CHECK(timeout == 4);
	CHECK(already_waited_sec == 0);
	CHECK(packet_num == 0);
	return 0;
}

static int test_failed_renewal_preserves_existing_lease_deadline(void)
{
	int timeout = 1800;
	int packet_num = 2;
	unsigned already_waited_sec = 300;

	udhcpc_script_failure_prepare_retry(0, 20, 25, &timeout,
					   &already_waited_sec, &packet_num);
	CHECK(timeout == 1800);
	CHECK(already_waited_sec == 325);
	CHECK(packet_num == 2);
	return 0;
}

static int test_script_runtime_cannot_extend_expired_lease(void)
{
	int timeout = 1800;
	int packet_num = 1;
	unsigned already_waited_sec = 1790;

	udhcpc_script_failure_prepare_retry(0, 20, 15, &timeout,
					   &already_waited_sec, &packet_num);
	CHECK(timeout == 0);
	CHECK(already_waited_sec == 0);
	CHECK(packet_num == 1);
	return 0;
}

static int test_exact_lease_deadline_is_expired(void)
{
	int timeout = 30;
	int packet_num = 0;
	unsigned already_waited_sec = 20;

	udhcpc_script_failure_prepare_retry(0, 20, 10, &timeout,
					   &already_waited_sec, &packet_num);
	CHECK(timeout == 0);
	CHECK(already_waited_sec == 0);
	return 0;
}

static int test_already_expired_lease_is_not_resurrected(void)
{
	int timeout = 5;
	int packet_num = 0;
	unsigned already_waited_sec = 6;

	udhcpc_script_failure_prepare_retry(0, 20, 0, &timeout,
					   &already_waited_sec, &packet_num);
	CHECK(timeout == 0);
	CHECK(already_waited_sec == 0);
	return 0;
}

int main(void)
{
	if (test_initial_acquisition_retries_selected_request() != 0 ||
	    test_failed_renewal_preserves_existing_lease_deadline() != 0 ||
	    test_script_runtime_cannot_extend_expired_lease() != 0 ||
	    test_exact_lease_deadline_is_expired() != 0 ||
	    test_already_expired_lease_is_not_resurrected() != 0)
		return 1;

	puts("udhcpc script failure retry policy: PASS");
	return 0;
}
