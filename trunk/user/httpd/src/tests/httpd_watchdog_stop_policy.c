#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>

#include "httpd_watchdog_stop_policy.h"

struct mock {
	enum process_presence states[8];
	int state_count;
	int state_index;
	int signal_result;
	int signal_errno;
	int signal_calls;
	int last_signal;
	int sleep_calls;
};

static enum process_presence
mock_presence(void *opaque)
{
	struct mock *mock = (struct mock *)opaque;
	int index = mock->state_index++;

	if (index >= mock->state_count)
		index = mock->state_count - 1;
	if (mock->states[index] == PROCESS_PRESENCE_UNKNOWN)
		errno = EACCES;
	return mock->states[index];
}

static int
mock_signal(void *opaque, int signal_number)
{
	struct mock *mock = (struct mock *)opaque;

	mock->signal_calls++;
	mock->last_signal = signal_number;
	if (mock->signal_result != 0)
		errno = mock->signal_errno;
	return mock->signal_result;
}

static void
mock_sleep(void *opaque)
{
	struct mock *mock = (struct mock *)opaque;

	mock->sleep_calls++;
}

static const struct httpd_watchdog_stop_ops mock_ops = {
	mock_presence,
	mock_signal,
	mock_sleep
};

static void
mock_init(struct mock *mock, const enum process_presence *states, int count)
{
	memset(mock, 0, sizeof(*mock));
	memcpy(mock->states, states, (size_t)count * sizeof(states[0]));
	mock->state_count = count;
}

int
main(void)
{
	struct mock mock;
	const enum process_presence absent[] = {
		PROCESS_PRESENCE_ABSENT
	};
	const enum process_presence unknown[] = {
		PROCESS_PRESENCE_UNKNOWN
	};
	const enum process_presence stopped[] = {
		PROCESS_PRESENCE_RUNNING, PROCESS_PRESENCE_ABSENT
	};
	const enum process_presence delayed[] = {
		PROCESS_PRESENCE_RUNNING, PROCESS_PRESENCE_RUNNING,
		PROCESS_PRESENCE_ABSENT
	};
	const enum process_presence post_unknown[] = {
		PROCESS_PRESENCE_RUNNING, PROCESS_PRESENCE_UNKNOWN
	};
	const enum process_presence timeout[] = {
		PROCESS_PRESENCE_RUNNING, PROCESS_PRESENCE_RUNNING,
		PROCESS_PRESENCE_RUNNING, PROCESS_PRESENCE_RUNNING,
		PROCESS_PRESENCE_RUNNING, PROCESS_PRESENCE_RUNNING
	};

	mock_init(&mock, absent, 1);
	assert(httpd_watchdog_stop_for_restore(&mock_ops, &mock) == 0);
	assert(mock.signal_calls == 0 && mock.state_index == 1);

	mock_init(&mock, unknown, 1);
	assert(httpd_watchdog_stop_for_restore(&mock_ops, &mock) == -1);
	assert(errno == EACCES && mock.signal_calls == 0);

	mock_init(&mock, stopped, 2);
	assert(httpd_watchdog_stop_for_restore(&mock_ops, &mock) == 0);
	assert(mock.signal_calls == 1 && mock.last_signal == SIGTERM);
	assert(mock.sleep_calls == 0);

	mock_init(&mock, delayed, 3);
	assert(httpd_watchdog_stop_for_restore(&mock_ops, &mock) == 0);
	assert(mock.signal_calls == 1 && mock.sleep_calls == 1);

	mock_init(&mock, stopped, 2);
	mock.signal_result = -1;
	mock.signal_errno = ESRCH;
	assert(httpd_watchdog_stop_for_restore(&mock_ops, &mock) == 0);
	assert(mock.signal_calls == 1);

	mock_init(&mock, stopped, 2);
	mock.signal_result = -1;
	mock.signal_errno = EPERM;
	assert(httpd_watchdog_stop_for_restore(&mock_ops, &mock) == -1);
	assert(errno == EPERM && mock.state_index == 1);

	mock_init(&mock, post_unknown, 2);
	assert(httpd_watchdog_stop_for_restore(&mock_ops, &mock) == -1);
	assert(errno == EACCES && mock.signal_calls == 1);
	assert(mock.sleep_calls == 0);

	mock_init(&mock, timeout, 6);
	assert(httpd_watchdog_stop_for_restore(&mock_ops, &mock) == -1);
	assert(errno == ETIMEDOUT && mock.state_index == 6);
	assert(mock.signal_calls == 1 && mock.sleep_calls == 4);

	assert(httpd_watchdog_stop_for_restore(NULL, NULL) == -1);
	assert(errno == EINVAL);

	puts("HTTPD watchdog identity-stop policy regression: PASS");
	return 0;
}
