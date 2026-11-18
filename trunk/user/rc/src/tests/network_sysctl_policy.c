#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include "network_sysctl_policy.h"

static int mock_status[5];
static unsigned int mock_calls;
static int mock_operation(void *context, unsigned int operation)
{
	(void)context;
	assert(operation == mock_calls);
	assert(operation < sizeof(mock_status) / sizeof(mock_status[0]));
	return mock_status[mock_calls++];
}
static void reset_mock(void)
{
	unsigned int i;
	mock_calls = 0;
	for (i = 0; i < sizeof(mock_status) / sizeof(mock_status[0]); ++i)
		mock_status[i] = 0;
}
static void test_all_attempted_first_error(void)
{
	reset_mock();
	mock_status[0] = EIO; mock_status[1] = EPERM; mock_status[3] = EACCES;
	assert(rc_network_sysctl_apply(4, mock_operation, NULL) == EIO);
	assert(mock_calls == 4);
}
static void test_later_failure(void)
{
	reset_mock(); mock_status[2] = EROFS;
	assert(rc_network_sysctl_apply(4, mock_operation, NULL) == EROFS);
	assert(mock_calls == 4);
}
static void test_negative_status(void)
{
	reset_mock(); mock_status[0] = -1; mock_status[1] = EIO;
	assert(rc_network_sysctl_apply(2, mock_operation, NULL) == -1);
	assert(mock_calls == 2);
}
static void test_success_and_empty(void)
{
	reset_mock();
	assert(rc_network_sysctl_apply(3, mock_operation, NULL) == 0);
	assert(mock_calls == 3);
	reset_mock();
	assert(rc_network_sysctl_apply(0, NULL, NULL) == 0);
	assert(mock_calls == 0);
}
static void test_configured_syncookies_gate(void)
{
	assert(rc_network_sysctl_tcp_syncookies_allows_wan(0, EIO));
	assert(rc_network_sysctl_tcp_syncookies_allows_wan(0, -1));
	assert(rc_network_sysctl_tcp_syncookies_allows_wan(1, 0));
	assert(!rc_network_sysctl_tcp_syncookies_allows_wan(1, EIO));
	assert(!rc_network_sysctl_tcp_syncookies_allows_wan(1, -1));
}
static void test_invalid_callback(void)
{
	reset_mock();
	assert(rc_network_sysctl_apply(1, NULL, NULL) == EINVAL);
	assert(mock_calls == 0);
}
int main(void)
{
	test_all_attempted_first_error(); test_later_failure(); test_negative_status();
	test_success_and_empty(); test_configured_syncookies_gate(); test_invalid_callback();
	puts("Network sysctl attempt-all/first-error policy: PASS");
	return 0;
}
