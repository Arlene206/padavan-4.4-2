#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <sys/types.h>
#include "vpn_server_callback_policy.h"

static void
expect_invalid(const char *text)
{
	pid_t parsed = 0;

	errno = 0;
	assert(vpn_server_callback_pid_parse(text, &parsed) == -1);
	assert(errno != 0);
}

int
main(void)
{
	pid_t parsed = 0;
	char overflow[64];

	assert(vpn_server_callback_pid_parse("12345", &parsed) == 0);
	assert(parsed == (pid_t)12345);
	expect_invalid(NULL);
	expect_invalid("");
	expect_invalid("0");
	expect_invalid("1");
	expect_invalid("-1");
	expect_invalid("+2");
	expect_invalid("2x");
	expect_invalid(" 2");
	expect_invalid("2\n");
	expect_invalid("999999999999999999999999999999999999999999999");

	assert(vpn_server_callback_pids_match((pid_t)12345, (pid_t)12345,
					      (pid_t)12345));
	assert(!vpn_server_callback_pids_match((pid_t)12345, (pid_t)1,
					       (pid_t)12345));
	assert(!vpn_server_callback_pids_match((pid_t)12345, (pid_t)12346,
					       (pid_t)12345));
	assert(!vpn_server_callback_pids_match((pid_t)12345, (pid_t)12345,
					       (pid_t)12346));
	assert(!vpn_server_callback_pids_match((pid_t)1, (pid_t)1,
					       (pid_t)1));
	snprintf(overflow, sizeof(overflow), "%lu", ULONG_MAX);
	expect_invalid(overflow);

	puts("VPN server callback identity policy: PASS");
	return 0;
}
