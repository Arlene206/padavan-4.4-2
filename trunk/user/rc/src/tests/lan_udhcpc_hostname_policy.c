#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include "lan_udhcpc_hostname_policy.h"

static void test_nvram_lookup_error_is_preserved(void)
{
	assert(lan_udhcpc_hostname_status(NULL, ENOENT, 0) == ENOENT);
	assert(lan_udhcpc_hostname_status(NULL, EIO, 0) == EIO);
}
static void test_missing_or_invalid_hostname_is_rejected(void)
{
	assert(lan_udhcpc_hostname_status(NULL, 0, 0) == EINVAL);
	assert(lan_udhcpc_hostname_status("", 0, 1) == EINVAL);
	assert(lan_udhcpc_hostname_status("bad hostname", 0, 0) == EINVAL);
}
static void test_valid_hostname_is_accepted(void)
{
	assert(lan_udhcpc_hostname_status("router", 0, 1) == 0);
}
int main(void)
{
	test_nvram_lookup_error_is_preserved();
	test_missing_or_invalid_hostname_is_rejected();
	test_valid_hostname_is_accepted();
	puts("LAN udhcpc hostname preflight regression: PASS");
	return 0;
}
