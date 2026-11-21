#include <signal.h>
#include <stdio.h>

#include "../detect_wan_signal_policy.h"

#define CHECK(condition) do { \
	if (!(condition)) { \
		fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
		return 1; \
	} \
} while (0)

static int
test_sigterm_only_sets_sticky_stop_flag(void)
{
	volatile sig_atomic_t stopping = 0;

	detect_wan_record_signal(&stopping, SIGHUP);
	CHECK(stopping == 0);
	detect_wan_record_signal(&stopping, SIGUSR1);
	CHECK(stopping == 0);
	detect_wan_record_signal(&stopping, SIGTERM);
	CHECK(stopping == 1);
	detect_wan_record_signal(&stopping, SIGTERM);
	CHECK(stopping == 1);
	detect_wan_record_signal(NULL, SIGTERM);
	return 0;
}

int
main(void)
{
	CHECK(test_sigterm_only_sets_sticky_stop_flag() == 0);
	puts("WAN monitor signal deferral policy: PASS");
	return 0;
}
