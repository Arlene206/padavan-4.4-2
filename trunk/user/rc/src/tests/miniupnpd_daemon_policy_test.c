#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>

#include "miniupnpd_daemon_policy.h"


struct miniupnpd_fake_signal {
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
miniupnpd_fake_validate(void *context, const struct process_identity *identity)
{
	struct miniupnpd_fake_signal *fake = (struct miniupnpd_fake_signal *)context;
	assert(identity != NULL && identity->pid > 0);
	if (fake->next_state >= fake->state_count) {
		errno = EIO;
		return PROCESS_PRESENCE_UNKNOWN;
	}
	if (fake->states[fake->next_state] == PROCESS_PRESENCE_UNKNOWN)
		errno = fake->validation_error ? fake->validation_error : EIO;
	return fake->states[fake->next_state++];
}

static int
miniupnpd_fake_send(void *context, pid_t pid, int signal_number)
{
	struct miniupnpd_fake_signal *fake = (struct miniupnpd_fake_signal *)context;
	if (fake->signal_error) {
		errno = fake->signal_error;
		return -1;
	}
	assert(fake->signal_count < 4);
	fake->signalled_pids[fake->signal_count] = pid;
	fake->signalled_signals[fake->signal_count] = signal_number;
	fake->signal_count++;
	return 0;
}

static void
miniupnpd_test_presence_policy(void)
{
	assert(miniupnpd_start_allowed(PROCESS_PRESENCE_ABSENT));
	assert(!miniupnpd_start_allowed(PROCESS_PRESENCE_RUNNING));
	assert(!miniupnpd_start_allowed(PROCESS_PRESENCE_UNKNOWN));
	assert(!miniupnpd_start_allowed((enum process_presence)7));
	assert(miniupnpd_action_for_presence(PROCESS_PRESENCE_ABSENT) ==
	       MINIUPNPD_DAEMON_START);
	assert(miniupnpd_action_for_presence(PROCESS_PRESENCE_RUNNING) ==
	       MINIUPNPD_DAEMON_NOTIFY);
	assert(miniupnpd_action_for_presence(PROCESS_PRESENCE_UNKNOWN) ==
	       MINIUPNPD_DAEMON_ABORT);
	assert(miniupnpd_action_for_presence((enum process_presence)7) ==
	       MINIUPNPD_DAEMON_ABORT);
	assert(miniupnpd_presence_error(PROCESS_PRESENCE_RUNNING, 0) == 0);
	assert(miniupnpd_presence_error(PROCESS_PRESENCE_ABSENT, 0) == 0);
	assert(miniupnpd_presence_error(PROCESS_PRESENCE_UNKNOWN, EACCES) == EACCES);
	assert(miniupnpd_presence_error(PROCESS_PRESENCE_UNKNOWN, 0) == EIO);
	assert(miniupnpd_presence_error((enum process_presence)7, 0) == EINVAL);
}

static void
miniupnpd_test_identity_bound_signal(void)
{
	struct process_identity identities[2];
	struct process_snapshot snapshot;
	struct miniupnpd_fake_signal fake;
	const enum process_presence running[] = {
		PROCESS_PRESENCE_RUNNING, PROCESS_PRESENCE_RUNNING,
		PROCESS_PRESENCE_RUNNING, PROCESS_PRESENCE_RUNNING
	};
	const enum process_presence unknown_preflight[] = {
		PROCESS_PRESENCE_RUNNING, PROCESS_PRESENCE_UNKNOWN
	};
	const enum process_presence unknown_revalidate[] = {
		PROCESS_PRESENCE_RUNNING, PROCESS_PRESENCE_RUNNING,
		PROCESS_PRESENCE_RUNNING, PROCESS_PRESENCE_UNKNOWN
	};

	memset(identities, 0, sizeof(identities));
	identities[0].pid = 101;
	identities[1].pid = 202;
	memset(&snapshot, 0, sizeof(snapshot));
	snapshot.state = PROCESS_PRESENCE_RUNNING;
	snapshot.items = identities;
	snapshot.count = 2;

	memset(&fake, 0, sizeof(fake));
	fake.states = running;
	fake.state_count = 4;
	assert(miniupnpd_signal_snapshot(&snapshot, SIGUSR1,
					 miniupnpd_fake_validate,
					 miniupnpd_fake_send, &fake) == 0);
	assert(fake.next_state == 4 && fake.signal_count == 2);
	assert(fake.signalled_pids[0] == 101 && fake.signalled_pids[1] == 202);
	assert(fake.signalled_signals[0] == SIGUSR1 &&
	       fake.signalled_signals[1] == SIGUSR1);

	memset(&fake, 0, sizeof(fake));
	fake.states = unknown_preflight;
	fake.state_count = 2;
	fake.validation_error = EACCES;
	errno = 0;
	assert(miniupnpd_signal_snapshot(&snapshot, SIGUSR1,
					 miniupnpd_fake_validate,
					 miniupnpd_fake_send, &fake) == -1);
	assert(errno == EACCES && fake.signal_count == 0);

	memset(&fake, 0, sizeof(fake));
	fake.states = unknown_revalidate;
	fake.state_count = 4;
	fake.validation_error = EPERM;
	errno = 0;
	assert(miniupnpd_signal_snapshot(&snapshot, SIGUSR1,
					 miniupnpd_fake_validate,
					 miniupnpd_fake_send, &fake) == -1);
	assert(errno == EPERM && fake.signal_count == 1);

	memset(&fake, 0, sizeof(fake));
	fake.states = running;
	fake.state_count = 4;
	fake.signal_error = ESRCH;
	assert(miniupnpd_signal_snapshot(&snapshot, SIGUSR1,
					 miniupnpd_fake_validate,
					 miniupnpd_fake_send, &fake) == 0);
	assert(fake.signal_count == 0);

	memset(&fake, 0, sizeof(fake));
	fake.states = running;
	fake.state_count = 4;
	fake.signal_error = EPERM;
	errno = 0;
	assert(miniupnpd_signal_snapshot(&snapshot, SIGUSR1,
					 miniupnpd_fake_validate,
					 miniupnpd_fake_send, &fake) == -1);
	assert(errno == EPERM && fake.signal_count == 0);

	errno = 0;
	assert(miniupnpd_signal_snapshot(&snapshot, SIGTERM,
					 miniupnpd_fake_validate,
					 miniupnpd_fake_send, &fake) == -1);
	assert(errno == EINVAL);
}

static void
miniupnpd_test_update_desired_state(void)
{
	assert(!miniupnpd_update_should_stop(1, 1, 0));
	assert(miniupnpd_update_should_stop(0, 1, 0));
	assert(miniupnpd_update_should_stop(1, 0, 0));
	assert(miniupnpd_update_should_stop(1, 1, 1));
	assert(miniupnpd_update_should_stop(0, 0, 0));
}

int
main(void)
{
	miniupnpd_test_presence_policy();
	miniupnpd_test_identity_bound_signal();
	miniupnpd_test_update_desired_state();

	assert(miniupnpd_start_policy_status(EIO, 0) == EIO);
	assert(miniupnpd_start_policy_status(EIO, 1) == EIO);
	assert(miniupnpd_start_policy_status(0, 0) == ESRCH);
	assert(miniupnpd_start_policy_status(0, 1) == 0);
	assert(miniupnpd_start_policy_status(0, 7) == 0);

	assert(miniupnpd_stop_policy_status(EIO, 0) == EIO);
	assert(miniupnpd_stop_policy_status(EIO, 1) == EIO);
	assert(miniupnpd_stop_policy_status(0, 1) == EBUSY);
	assert(miniupnpd_stop_policy_status(0, 0) == 0);
	assert(miniupnpd_stop_policy_status(-1, 0) == -1);

	puts("miniupnpd daemon lifecycle policy: PASS");
	return 0;
}
