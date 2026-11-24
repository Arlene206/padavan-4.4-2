#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "samba_fastpath_policy.h"

int
main(void)
{
	assert(samba_fastpath_should_enable(1, PROCESS_PRESENCE_RUNNING));
	assert(!samba_fastpath_should_enable(1, PROCESS_PRESENCE_ABSENT));
	assert(!samba_fastpath_should_enable(1, PROCESS_PRESENCE_UNKNOWN));
	assert(!samba_fastpath_should_enable(1, (enum process_presence)9));
	assert(!samba_fastpath_should_enable(0, PROCESS_PRESENCE_RUNNING));
	assert(!samba_fastpath_should_enable(-1, PROCESS_PRESENCE_RUNNING));

	assert(samba_fastpath_stop_result(0, 0) == 0);
	/* No proc hook means this kernel has no SMB fastpath state to clear. */
	assert(samba_fastpath_stop_result(0, ENOENT) == 0);
	assert(samba_fastpath_stop_result(0, EIO) == -1);
	assert(samba_fastpath_stop_result(0, EACCES) == -1);
	assert(samba_fastpath_stop_result(-1, EIO) == -1);
	assert(samba_fastpath_stop_result(-2, EIO) == -2);
	assert(samba_fastpath_stop_result(7, EIO) == 7);

	puts("Samba fastpath process-presence and stop-state policy: PASS");
	return 0;
}
