#include <stdio.h>
#include <stddef.h>

#include "../cmd-request-policy.h"

#define CHECK(condition) do { \
	if (!(condition)) { \
		fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
		return 1; \
	} \
} while (0)

static int
test_valid_delimited_requests(void)
{
	CHECK(cmd_control_request_is_valid(1, 1, 512, 1));
	CHECK(cmd_control_request_is_valid(1, 2, 512, 1));
	CHECK(cmd_control_request_is_valid(1, 511, 512, 1));
	return 0;
}

static int
test_incomplete_requests_are_rejected(void)
{
	CHECK(!cmd_control_request_is_valid(0, 1, 512, 1));
	CHECK(!cmd_control_request_is_valid(0, 511, 512, 1));
	CHECK(!cmd_control_request_is_valid(1, 1, 512, 0));
	CHECK(!cmd_control_request_is_valid(1, 511, 512, 0));
	return 0;
}

static int
test_empty_and_capacity_sized_requests_are_rejected(void)
{
	CHECK(!cmd_control_request_is_valid(1, 0, 512, 0));
	CHECK(!cmd_control_request_is_valid(1, 0, 512, 1));
	CHECK(!cmd_control_request_is_valid(1, 512, 512, 1));
	CHECK(!cmd_control_request_is_valid(1, 513, 512, 1));
	CHECK(!cmd_control_request_is_valid(1, 1, 0, 1));
	return 0;
}

int
main(void)
{
	CHECK(test_valid_delimited_requests() == 0);
	CHECK(test_incomplete_requests_are_rejected() == 0);
	CHECK(test_empty_and_capacity_sized_requests_are_rejected() == 0);
	puts("rp-l2tp control request framing regression: PASS");
	return 0;
}
