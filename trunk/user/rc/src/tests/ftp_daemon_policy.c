#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "ftp_daemon_policy.h"

struct fake_daemon {
	const int *states;
	size_t state_count;
	size_t next_state;
	unsigned int slept_us;
	int unknown_errno;
	int sleep_status;
	int sleep_errno;
};

static int
fake_is_running(void *context, const char *daemon)
{
	struct fake_daemon *fake = (struct fake_daemon *)context;

	assert(strcmp(daemon, "vsftpd") == 0);
	if (fake->next_state < fake->state_count) {
		int state = fake->states[fake->next_state++];
		if (state == PROCESS_PRESENCE_UNKNOWN)
			errno = fake->unknown_errno ? fake->unknown_errno : EIO;
		return state;
	}
	fake->next_state++;
	return PROCESS_PRESENCE_ABSENT;
}

static int
fake_sleep_us(void *context, unsigned int microseconds)
{
	struct fake_daemon *fake = (struct fake_daemon *)context;

	fake->slept_us += microseconds;
	if (fake->sleep_status != 0) {
		errno = fake->sleep_errno ? fake->sleep_errno : EIO;
		return -1;
	}
	return 0;
}

static const struct ftp_daemon_wait_ops fake_ops = {
	fake_is_running,
	fake_sleep_us
};

int
main(void)
{
	{
		const int states[] = { 0, 1, 1 };
		struct fake_daemon fake = { states, 3, 0, 0, 0, 0, 0 };

		assert(ftp_daemon_wait_for_stable_presence("vsftpd", 3,
							    &fake_ops, &fake) == 0);
		assert(fake.next_state == 3);
		assert(fake.slept_us == 600000U);
	}

	{
		const int states[] = { 1, 0, 0 };
		struct fake_daemon fake = { states, 3, 0, 0, 0, 0, 0 };

		errno = 0;
		assert(ftp_daemon_wait_for_stable_presence("vsftpd", 2,
							    &fake_ops, &fake) == -1);
		assert(errno == ETIMEDOUT);
		assert(fake.next_state == 3);
		assert(fake.slept_us == 700000U);
	}

	{
		struct fake_daemon fake = { NULL, 0, 0, 0, 0, 0, 0 };

		errno = 0;
		assert(ftp_daemon_wait_for_stable_presence("vsftpd", 0,
							    &fake_ops, &fake) == -1);
		assert(errno == EINVAL);
		assert(fake.next_state == 0 && fake.slept_us == 0);
	}

	{
		struct fake_daemon fake = { NULL, 0, 0, 0, 0, 0, 0 };

		errno = 0;
		assert(ftp_daemon_wait_for_stable_presence("vsftpd", 1,
							    NULL, &fake) == -1);
		assert(errno == EINVAL);
	}

	{
		const int states[] = { PROCESS_PRESENCE_UNKNOWN };
		struct fake_daemon fake = { states, 1, 0, 0, EACCES };
		errno = 0;
		assert(ftp_daemon_wait_for_stable_presence("vsftpd", 3, &fake_ops, &fake) == -1);
		assert(errno == EACCES && fake.next_state == 1 && fake.slept_us == 0);
	}
	{
		const int states[] = { PROCESS_PRESENCE_RUNNING, PROCESS_PRESENCE_UNKNOWN };
		struct fake_daemon fake = { states, 2, 0, 0, EACCES };
		errno = 0;
		assert(ftp_daemon_wait_for_stable_presence("vsftpd", 3, &fake_ops, &fake) == -1);
		assert(errno == EACCES && fake.next_state == 2 && fake.slept_us == 500000U);
	}
	{
		const int states[] = { (int)9 };
		struct fake_daemon fake = { states, 1, 0, 0, 0, 0, 0 };
		errno = 0;
		assert(ftp_daemon_wait_for_stable_presence("vsftpd", 1, &fake_ops, &fake) == -1);
		assert(errno == EIO);
	}

	{
		const int states[] = { PROCESS_PRESENCE_RUNNING };
		struct fake_daemon fake = { states, 1, 0, 0, 0, -1, EACCES };

		errno = 0;
		assert(ftp_daemon_wait_for_stable_presence(
			"vsftpd", 3, &fake_ops, &fake) == -1);
		assert(errno == EACCES);
		assert(fake.next_state == 1 && fake.slept_us == 500000U);
	}

	puts("vsftpd tri-state daemon-presence policy: PASS");
	return 0;
}
