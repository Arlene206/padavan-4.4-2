#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "dnsmasq_resolv_file_policy.h"

static void
test_success_ignores_stale_errno(void)
{
	assert(dnsmasq_resolv_file_error(0, EACCES) == 0);
}

static void
test_creation_failure_preserves_errno(void)
{
	assert(dnsmasq_resolv_file_error(1, EACCES) == EACCES);
	assert(dnsmasq_resolv_file_error(-1, ENOSPC) == ENOSPC);
}

static void
test_creation_failure_without_errno_fails_closed(void)
{
	assert(dnsmasq_resolv_file_error(1, 0) == EIO);
	assert(dnsmasq_resolv_file_error(-1, 0) == EIO);
	assert(dnsmasq_resolv_file_error(1, -1) == EIO);
}

int
main(void)
{
	test_success_ignores_stale_errno();
	test_creation_failure_preserves_errno();
	test_creation_failure_without_errno_fails_closed();
	puts("DNSMasq resolver-file creation status policy: PASS");
	return 0;
}
