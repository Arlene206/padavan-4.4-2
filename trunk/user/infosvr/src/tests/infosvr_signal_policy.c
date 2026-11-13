#include <assert.h>
#include <signal.h>
#include <stdio.h>

#include "infosvr_signal_policy.h"

int
main(void)
{
	volatile sig_atomic_t stop_requested = 0;

	assert(!INFOSVR_SIGNAL_STOP_PENDING(stop_requested));
	INFOSVR_SIGNAL_REQUEST_STOP(stop_requested);
	assert(INFOSVR_SIGNAL_STOP_PENDING(stop_requested));
	INFOSVR_SIGNAL_REQUEST_STOP(stop_requested);
	assert(INFOSVR_SIGNAL_STOP_PENDING(stop_requested));
	assert(INFOSVR_SELECT_TIMEOUT_SECONDS == 1);
	assert(INFOSVR_SOCKET_TIMEOUT_SECONDS == 1);

	puts("infosvr SIGTERM signal deferral policy: PASS");
	return 0;
}
