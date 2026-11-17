#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "networkmap_daemon_policy.h"

struct fake_networkmap {
	const enum process_presence *states;
	size_t state_count;
	size_t next_state;
	int validation_error;
	int signal_error;
	int signal_count;
	pid_t signalled_pids[4];
	int signalled_signals[4];
};

static enum process_presence
fake_validate(void *context, const struct process_identity *identity)
{
	struct fake_networkmap *fake = (struct fake_networkmap *)context;
	assert(identity != NULL);
	assert(identity->pid > 0);
	if (fake->next_state >= fake->state_count) {
		errno = EIO;
		return PROCESS_PRESENCE_UNKNOWN;
	}
	if (fake->states[fake->next_state] == PROCESS_PRESENCE_UNKNOWN)
		errno = fake->validation_error ? fake->validation_error : EIO;
	return fake->states[fake->next_state++];
}

static int
fake_send_signal(void *context, pid_t pid, int sig)
{
	struct fake_networkmap *fake = (struct fake_networkmap *)context;
	if (fake->signal_error) {
		errno = fake->signal_error;
		return -1;
	}
	assert(fake->signal_count < 4);
	fake->signalled_pids[fake->signal_count] = pid;
	fake->signalled_signals[fake->signal_count] = sig;
	fake->signal_count++;
	return 0;
}

static void
fake_init(struct fake_networkmap *fake,
	  const enum process_presence *states, size_t state_count)
{
	memset(fake, 0, sizeof(*fake));
	fake->states = states;
	fake->state_count = state_count;
}

int
main(void)
{
	struct process_identity identities[2];
	struct process_snapshot snapshot;

	memset(identities, 0, sizeof(identities));
	identities[0].pid = 101;
	identities[1].pid = 202;
	memset(&snapshot, 0, sizeof(snapshot));
	snapshot.state = PROCESS_PRESENCE_RUNNING;
	snapshot.items = identities;
	snapshot.count = 2;

	assert(networkmap_start_allowed(PROCESS_PRESENCE_ABSENT));
	assert(!networkmap_start_allowed(PROCESS_PRESENCE_RUNNING));
	assert(!networkmap_start_allowed(PROCESS_PRESENCE_UNKNOWN));
	assert(!networkmap_start_allowed((enum process_presence)9));
	assert(networkmap_start_postcondition(0, PROCESS_PRESENCE_RUNNING) == 0);
	assert(networkmap_start_postcondition(0, PROCESS_PRESENCE_ABSENT) == -1);
	assert(networkmap_start_postcondition(0, PROCESS_PRESENCE_UNKNOWN) == -1);
	assert(networkmap_start_postcondition(0, (enum process_presence)9) == -1);
	assert(networkmap_start_postcondition(7, PROCESS_PRESENCE_RUNNING) == 7);
	assert(networkmap_start_postcondition(7, PROCESS_PRESENCE_ABSENT) == 7);


	assert(networkmap_action_for_presence(PROCESS_PRESENCE_ABSENT) ==
	       NETWORKMAP_DAEMON_START);
	assert(networkmap_action_for_presence(PROCESS_PRESENCE_RUNNING) ==
	       NETWORKMAP_DAEMON_REFRESH);
	assert(networkmap_action_for_presence(PROCESS_PRESENCE_UNKNOWN) ==
	       NETWORKMAP_DAEMON_ABORT);
	assert(networkmap_action_for_presence((enum process_presence)9) ==
	       NETWORKMAP_DAEMON_ABORT);

	{
		const enum process_presence states[] = {
			PROCESS_PRESENCE_RUNNING, PROCESS_PRESENCE_RUNNING,
			PROCESS_PRESENCE_RUNNING, PROCESS_PRESENCE_RUNNING
		};
		struct fake_networkmap fake;
		fake_init(&fake, states, 4);
		assert(networkmap_signal_snapshot(&snapshot, SIGUSR1, fake_validate,
						  fake_send_signal, &fake) == 0);
		assert(fake.next_state == 4 && fake.signal_count == 2);
		assert(fake.signalled_pids[0] == 101 &&
		       fake.signalled_pids[1] == 202);
		assert(fake.signalled_signals[0] == SIGUSR1 &&
		       fake.signalled_signals[1] == SIGUSR1);
	}

	{
		const enum process_presence states[] = {
			PROCESS_PRESENCE_RUNNING, PROCESS_PRESENCE_UNKNOWN
		};
		struct fake_networkmap fake;
		fake_init(&fake, states, 2);
		fake.validation_error = EACCES;
		errno = 0;
		assert(networkmap_signal_snapshot(&snapshot, SIGUSR1, fake_validate,
						  fake_send_signal, &fake) == -1);
		assert(errno == EACCES && fake.signal_count == 0);
	}

	{
		const enum process_presence states[] = {
			PROCESS_PRESENCE_RUNNING, PROCESS_PRESENCE_RUNNING,
			PROCESS_PRESENCE_RUNNING, PROCESS_PRESENCE_UNKNOWN
		};
		struct fake_networkmap fake;
		fake_init(&fake, states, 4);
		fake.validation_error = EACCES;
		errno = 0;
		assert(networkmap_signal_snapshot(&snapshot, SIGUSR1, fake_validate,
						  fake_send_signal, &fake) == -1);
		assert(errno == EACCES && fake.signal_count == 1);
		assert(fake.signalled_pids[0] == 101);
	}

	{
		const enum process_presence states[] = {
			PROCESS_PRESENCE_ABSENT, PROCESS_PRESENCE_ABSENT,
			PROCESS_PRESENCE_ABSENT, PROCESS_PRESENCE_ABSENT
		};
		struct fake_networkmap fake;
		fake_init(&fake, states, 4);
		assert(networkmap_signal_snapshot(&snapshot, SIGUSR1, fake_validate,
						  fake_send_signal, &fake) == 0);
		assert(fake.signal_count == 0);
	}

	{
		const enum process_presence states[] = {
			PROCESS_PRESENCE_RUNNING, PROCESS_PRESENCE_RUNNING,
			PROCESS_PRESENCE_RUNNING, PROCESS_PRESENCE_RUNNING
		};
		struct fake_networkmap fake;
		fake_init(&fake, states, 4);
		fake.signal_error = ESRCH;
		assert(networkmap_signal_snapshot(&snapshot, SIGUSR1, fake_validate,
						  fake_send_signal, &fake) == 0);
		assert(fake.signal_count == 0);
	}

	{
		const enum process_presence states[] = {
			PROCESS_PRESENCE_RUNNING, PROCESS_PRESENCE_RUNNING,
			PROCESS_PRESENCE_RUNNING, PROCESS_PRESENCE_RUNNING
		};
		struct fake_networkmap fake;
		fake_init(&fake, states, 4);
		fake.signal_error = EPERM;
		errno = 0;
		assert(networkmap_signal_snapshot(&snapshot, SIGUSR1, fake_validate,
						  fake_send_signal, &fake) == -1);
		assert(errno == EPERM && fake.signal_count == 0);
	}

	errno = 0;
	assert(networkmap_signal_snapshot(NULL, SIGUSR1, fake_validate,
					  fake_send_signal, NULL) == -1);
	assert(errno == EINVAL);
	puts("networkmap tri-state action, startup postcondition and identity-bound refresh policy: PASS");
	return 0;
}
