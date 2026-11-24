#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "../cmd-control-status.h"

#define CHECK(condition) do { \
	if (!(condition)) { \
		fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
		return 1; \
	} \
} while (0)

static int
test_success_responses(void)
{
	static const char ids[] = "OK 123 456";
	static const char listing[] = "OK\nNumL2TPTunnels 0\n";
	static const char exact[] = "OK";
	static const char delimiter[] = "OK ";

	CHECK(cmd_control_response_status(ids, sizeof(ids) - 1) == 0);
	CHECK(cmd_control_response_status(listing, sizeof(listing) - 1) == 0);
	CHECK(cmd_control_response_status(exact, sizeof(exact) - 1) == 0);
	CHECK(cmd_control_response_status(delimiter, sizeof(delimiter) - 1) == 0);
	return 0;
}

static int
test_error_responses(void)
{
	static const char unknown_peer[] = "ERR Unknown peer";
	static const char exact[] = "ERR";

	CHECK(cmd_control_response_status(unknown_peer,
					  sizeof(unknown_peer) - 1) == EIO);
	CHECK(cmd_control_response_status(exact, sizeof(exact) - 1) == EIO);
	return 0;
}

static int
test_malformed_and_truncated_responses(void)
{
	static const char bad_ok[] = "OKAY";
	static const char bad_error[] = "ERROR";
	static const char short_error[] = "ER";
	static const char short_ok[] = "O";

	CHECK(cmd_control_response_status(NULL, 0) == EPROTO);
	CHECK(cmd_control_response_status("", 0) == EPROTO);
	CHECK(cmd_control_response_status(short_ok, sizeof(short_ok) - 1) == EPROTO);
	CHECK(cmd_control_response_status(short_error, sizeof(short_error) - 1) == EPROTO);
	CHECK(cmd_control_response_status(bad_ok, sizeof(bad_ok) - 1) == EPROTO);
	CHECK(cmd_control_response_status(bad_error, sizeof(bad_error) - 1) == EPROTO);
	return 0;
}

int
main(void)
{
	CHECK(test_success_responses() == 0);
	CHECK(test_error_responses() == 0);
	CHECK(test_malformed_and_truncated_responses() == 0);
	puts("rp-l2tp control response status regression: PASS");
	return 0;
}
