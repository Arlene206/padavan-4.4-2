#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "samba_daemon_policy.h"

struct fake_daemon {
	const int *states;
	size_t state_count;
	size_t next_state;
	unsigned int slept_us;
};

static int
fake_is_running(void *context, const char *daemon)
{
	struct fake_daemon *fake = (struct fake_daemon *)context;

	assert(strcmp(daemon, "nmbd") == 0);
	if (fake->next_state < fake->state_count)
		return fake->states[fake->next_state++];
	fake->next_state++;
	return 0;
}

static void
fake_sleep_us(void *context, unsigned int microseconds)
{
	struct fake_daemon *fake = (struct fake_daemon *)context;

	fake->slept_us += microseconds;
}

static const struct samba_daemon_wait_ops fake_ops = {
	fake_is_running,
	fake_sleep_us
};

int
main(void)
{
	{
		const int states[] = { 0, 1, 1 };
		struct fake_daemon fake = { states, 3, 0, 0 };

		assert(samba_daemon_wait_for_stable_presence("nmbd", 3,
							     &fake_ops, &fake) == 0);
		assert(fake.next_state == 3);
		assert(fake.slept_us == 600000U);
	}

	{
		const int states[] = { 1, 0, 0 };
		struct fake_daemon fake = { states, 3, 0, 0 };

		errno = 0;
		assert(samba_daemon_wait_for_stable_presence("nmbd", 2,
							     &fake_ops, &fake) == -1);
		assert(errno == ETIMEDOUT);
		assert(fake.next_state == 3);
		assert(fake.slept_us == 700000U);
	}

	{
		struct fake_daemon fake = { NULL, 0, 0, 0 };

		errno = 0;
		assert(samba_daemon_wait_for_stable_presence("nmbd", 0,
							     &fake_ops, &fake) == -1);
		assert(errno == EINVAL);
		assert(fake.next_state == 0 && fake.slept_us == 0);
	}

	{
		struct fake_daemon fake = { NULL, 0, 0, 0 };

		errno = 0;
		assert(samba_daemon_wait_for_stable_presence("nmbd", 1,
							     NULL, &fake) == -1);
		assert(errno == EINVAL);
	}

	puts("Samba daemon bounded-presence policy: PASS");
	return 0;
}
