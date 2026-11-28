#include <assert.h>
#include <errno.h>

#include "smp_affinity_policy.h"

static void
test_optional_missing_nodes_do_not_degrade_startup(void)
{
	assert(smp_affinity_record_status(0, ENOENT) == 0);
	assert(smp_affinity_record_status(ENOENT, 0) == 0);
	assert(smp_affinity_record_status(ENOENT, EIO) == EIO);
}

static void
test_real_write_errors_are_preserved(void)
{
	assert(smp_affinity_record_status(0, EIO) == EIO);
	assert(smp_affinity_record_status(0, EACCES) == EACCES);
	assert(smp_affinity_record_status(0, ENOSPC) == ENOSPC);
	assert(smp_affinity_record_status(EIO, 0) == EIO);
	assert(smp_affinity_record_status(EIO, ENOSPC) == EIO);
	assert(smp_affinity_record_status(ENOSPC, EACCES) == ENOSPC);
}

int
main(void)
{
	assert(smp_affinity_record_status(0, 0) == 0);
	test_optional_missing_nodes_do_not_degrade_startup();
	test_real_write_errors_are_preserved();
	return 0;
}
