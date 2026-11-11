#include <errno.h>
#include <stdio.h>

#include "gpio_btn_daemon_policy.h"

#define CHECK(condition) do { \
	if (!(condition)) { \
		fprintf(stderr, "FAIL %s:%d: %s\n", __FILE__, __LINE__, #condition); \
		return 1; \
	} \
} while (0)

static int
test_running_daemon_satisfies_start(void)
{
	CHECK(gpio_btn_start_policy_status(0, 1) == 0);
	CHECK(gpio_btn_start_policy_status(0, 2) == 0);
	return 0;
}

static int
test_missing_daemon_fails_start(void)
{
	CHECK(gpio_btn_start_policy_status(0, 0) == ESRCH);
	return 0;
}

static int
test_launcher_error_is_preserved(void)
{
	CHECK(gpio_btn_start_policy_status(EIO, 0) == EIO);
	CHECK(gpio_btn_start_policy_status(EBUSY, 1) == EBUSY);
	CHECK(gpio_btn_start_policy_status(-1, 0) == -1);
	return 0;
}

int
main(void)
{
	CHECK(test_running_daemon_satisfies_start() == 0);
	CHECK(test_missing_daemon_fails_start() == 0);
	CHECK(test_launcher_error_is_preserved() == 0);
	puts("GPIO button daemon startup postcondition policy: PASS");
	return 0;
}
