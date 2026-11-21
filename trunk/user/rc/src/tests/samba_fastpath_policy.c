#include <assert.h>
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

	puts("Samba fastpath process-presence gate: PASS");
	return 0;
}
