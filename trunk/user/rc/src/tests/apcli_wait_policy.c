#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "apcli_wait_policy.h"

int
main(void)
{
	assert(rc_wan_apcli_wait_status(1) == 0);
	assert(rc_wan_apcli_wait_status(0) == ETIMEDOUT);

	puts("WAN AP-client association wait status: PASS");
	return 0;
}
