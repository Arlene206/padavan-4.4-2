#include <assert.h>
#include <errno.h>
#include "smp_affinity_policy.h"
int
main(void)
{
	assert(smp_affinity_record_status(0, 0) == 0);
	/* Optional IRQ/queue nodes are absent on some supported topologies. */
	assert(smp_affinity_record_status(0, ENOENT) == 0);
	assert(smp_affinity_record_status(ENOENT, 0) == 0);
	assert(smp_affinity_record_status(ENOENT, EIO) == EIO);
	/* Real write failures are retained and the first one is deterministic. */
	assert(smp_affinity_record_status(0, EIO) == EIO);
	assert(smp_affinity_record_status(0, EACCES) == EACCES);
	assert(smp_affinity_record_status(0, ENOSPC) == ENOSPC);
	assert(smp_affinity_record_status(EIO, 0) == EIO);
	assert(smp_affinity_record_status(EIO, ENOSPC) == EIO);
	assert(smp_affinity_record_status(ENOSPC, EACCES) == ENOSPC);
	return 0;
}
