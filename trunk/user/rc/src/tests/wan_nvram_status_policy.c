#include <assert.h>
#include <errno.h>

#include "wan_nvram_status_policy.h"

static void
test_success_ignores_stale_errno(void)
{
	assert(rc_wan_nvram_write_status(0, EACCES) == 0);
}

static void
test_failure_preserves_errno(void)
{
	assert(rc_wan_nvram_write_status(-1, EACCES) == EACCES);
}

static void
test_failure_without_errno_falls_back_to_eio(void)
{
	assert(rc_wan_nvram_write_status(-1, 0) == EIO);
}

static void
test_unexpected_nonzero_result_is_not_success(void)
{
	assert(rc_wan_nvram_write_status(1, 0) == EIO);
}

int
main(void)
{
	test_success_ignores_stale_errno();
	test_failure_preserves_errno();
	test_failure_without_errno_falls_back_to_eio();
	test_unexpected_nonzero_result_is_not_success();
	return 0;
}
