#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "samba_daemon_policy.h"

struct fake_daemon {
	const enum process_presence *states;
	size_t state_count;
	size_t next_state;
	int state_errno;
	unsigned int slept_us;
	int sleep_status;
	int sleep_errno;
};

static enum process_presence
fake_get_presence(void *context, const char *daemon)
{
	struct fake_daemon *fake = (struct fake_daemon *)context;
	enum process_presence state;

	assert(strcmp(daemon, "nmbd") == 0);
	if (fake->next_state < fake->state_count)
		state = fake->states[fake->next_state++];
	else {
		fake->next_state++;
		state = PROCESS_PRESENCE_ABSENT;
	}
	if (state == PROCESS_PRESENCE_UNKNOWN)
		errno = fake->state_errno ? fake->state_errno : EIO;
	return state;
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

static const struct samba_daemon_wait_ops fake_ops = {
	fake_get_presence,
	fake_sleep_us
};

int
main(void)
{
	assert(!samba_failed_start_may_leave_daemon(PROCESS_PRESENCE_ABSENT));
	assert(samba_failed_start_may_leave_daemon(PROCESS_PRESENCE_RUNNING));
	assert(samba_failed_start_may_leave_daemon(PROCESS_PRESENCE_UNKNOWN));
	assert(samba_failed_start_may_leave_daemon((enum process_presence)9));

	{
		const enum process_presence states[] = {
			PROCESS_PRESENCE_ABSENT,
			PROCESS_PRESENCE_RUNNING,
			PROCESS_PRESENCE_RUNNING
		};
		struct fake_daemon fake = { states, 3, 0, 0, 0, 0, 0 };

		assert(samba_daemon_wait_for_stable_presence("nmbd", 3,
							     &fake_ops, &fake) == 0);
		assert(fake.next_state == 3);
		assert(fake.slept_us == 600000U);
	}

	{
		const enum process_presence states[] = {
			PROCESS_PRESENCE_RUNNING,
			PROCESS_PRESENCE_ABSENT,
			PROCESS_PRESENCE_ABSENT
		};
		struct fake_daemon fake = { states, 3, 0, 0, 0, 0, 0 };

		errno = 0;
		assert(samba_daemon_wait_for_stable_presence("nmbd", 2,
							     &fake_ops, &fake) == -1);
		assert(errno == ETIMEDOUT);
		assert(fake.next_state == 3);
		assert(fake.slept_us == 700000U);
	}

	{
		const enum process_presence states[] = {
			PROCESS_PRESENCE_UNKNOWN
		};
		struct fake_daemon fake = { states, 1, 0, EACCES, 0, 0, 0 };

		errno = 0;
		assert(samba_daemon_wait_for_stable_presence("nmbd", 2,
							     &fake_ops, &fake) == -1);
		assert(errno == EACCES);
		assert(fake.next_state == 1 && fake.slept_us == 0);
	}

	{
		const enum process_presence states[] = {
			PROCESS_PRESENCE_RUNNING,
			PROCESS_PRESENCE_UNKNOWN
		};
		struct fake_daemon fake = { states, 2, 0, EPERM, 0, 0, 0 };

		errno = 0;
		assert(samba_daemon_wait_for_stable_presence("nmbd", 2,
							     &fake_ops, &fake) == -1);
		assert(errno == EPERM);
		assert(fake.next_state == 2 && fake.slept_us == 500000U);
	}

	{
		const enum process_presence states[] = {
			(enum process_presence)9
		};
		struct fake_daemon fake = { states, 1, 0, 0, 0, 0, 0 };

		errno = 0;
		assert(samba_daemon_wait_for_stable_presence("nmbd", 1,
							     &fake_ops, &fake) == -1);
		assert(errno == EIO);
		assert(fake.next_state == 1 && fake.slept_us == 0);
	}

	{
		struct fake_daemon fake = { NULL, 0, 0, 0, 0, 0, 0 };

		errno = 0;
		assert(samba_daemon_wait_for_stable_presence("nmbd", 0,
							     &fake_ops, &fake) == -1);
		assert(errno == EINVAL);
		assert(fake.next_state == 0 && fake.slept_us == 0);
	}

	{
		struct fake_daemon fake = { NULL, 0, 0, 0, 0, 0, 0 };

		errno = 0;
		assert(samba_daemon_wait_for_stable_presence("nmbd", 1,
							     NULL, &fake) == -1);
		assert(errno == EINVAL);
	}


	{
		const enum process_presence states[] = { PROCESS_PRESENCE_RUNNING };
		struct fake_daemon fake = { states, 1, 0, 0, 0, -1, EACCES };

		errno = 0;
		assert(samba_daemon_wait_for_stable_presence("nmbd", 2,
							     &fake_ops, &fake) == -1);
		assert(errno == EACCES);
		assert(fake.next_state == 1 && fake.slept_us == 500000U);
	}

	puts("Samba tri-state daemon readiness policy: PASS");
	return 0;
}
