#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>

#include "inadyn_daemon_policy.h"

struct fake_signal {
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
	struct fake_signal *fake = (struct fake_signal *)context;
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
fake_send_signal(void *context, pid_t pid, int sig)
{
	struct fake_signal *fake = (struct fake_signal *)context;
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
test_update_configuration_gate(void)
{
	assert(inadyn_update_should_run(1, 0) == 1);
	assert(inadyn_update_should_run(0, 0) == 0);
	assert(inadyn_update_should_run(1, 1) == 0);
	assert(inadyn_update_should_run(0, 1) == 0);
}

static void
test_presence_decisions(void)
{
	assert(inadyn_action_for_presence(PROCESS_PRESENCE_ABSENT) ==
	       INADYN_DAEMON_START);
	assert(inadyn_action_for_presence(PROCESS_PRESENCE_RUNNING) ==
	       INADYN_DAEMON_RELOAD);
	assert(inadyn_action_for_presence(PROCESS_PRESENCE_UNKNOWN) ==
	       INADYN_DAEMON_ABORT);
	assert(inadyn_action_for_presence((enum process_presence)7) ==
	       INADYN_DAEMON_ABORT);
	assert(inadyn_presence_error(PROCESS_PRESENCE_UNKNOWN, EACCES) == EACCES);
	assert(inadyn_presence_error(PROCESS_PRESENCE_UNKNOWN, 0) == EIO);
	assert(inadyn_presence_error((enum process_presence)7, 0) == EINVAL);
}

static void
test_config_preflight_requires_all_live_identities(void)
{
	struct process_identity identities[2];
	struct process_snapshot snapshot;
	struct fake_signal fake;
	const enum process_presence live[] = {
		PROCESS_PRESENCE_RUNNING, PROCESS_PRESENCE_RUNNING
	};
	const enum process_presence unknown[] = {
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
	fake.states = live;
	fake.state_count = 2;
	assert(inadyn_validate_snapshot(&snapshot, fake_validate, &fake) == 0);
	assert(fake.next_state == 2);

	memset(&fake, 0, sizeof(fake));
	fake.states = unknown;
	fake.state_count = 2;
	fake.validation_error = EACCES;
	errno = 0;
	assert(inadyn_validate_snapshot(&snapshot, fake_validate, &fake) == -1);
	assert(errno == EACCES && fake.next_state == 2);
}

static void
test_identity_bound_hup_signal(void)
{
	struct process_identity identities[2];
	struct process_snapshot snapshot;
	struct fake_signal fake;
	const enum process_presence live[] = {
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
	fake.states = live;
	fake.state_count = 4;
	assert(inadyn_signal_snapshot(&snapshot, SIGHUP, fake_validate,
				      fake_send_signal, &fake) == 0);
	assert(fake.next_state == 4 && fake.signal_count == 2);
	assert(fake.signalled_pids[0] == 101 && fake.signalled_pids[1] == 202);
	assert(fake.signalled_signals[0] == SIGHUP &&
	       fake.signalled_signals[1] == SIGHUP);

	memset(&fake, 0, sizeof(fake));
	fake.states = unknown_preflight;
	fake.state_count = 2;
	fake.validation_error = EACCES;
	errno = 0;
	assert(inadyn_signal_snapshot(&snapshot, SIGHUP, fake_validate,
				      fake_send_signal, &fake) == -1);
	assert(errno == EACCES && fake.signal_count == 0);

	memset(&fake, 0, sizeof(fake));
	fake.states = unknown_revalidate;
	fake.state_count = 4;
	fake.validation_error = EPERM;
	errno = 0;
	assert(inadyn_signal_snapshot(&snapshot, SIGHUP, fake_validate,
				      fake_send_signal, &fake) == -1);
	assert(errno == EPERM && fake.signal_count == 1);

	memset(&fake, 0, sizeof(fake));
	fake.states = live;
	fake.state_count = 4;
	fake.signal_error = ESRCH;
	assert(inadyn_signal_snapshot(&snapshot, SIGHUP, fake_validate,
				      fake_send_signal, &fake) == 0);
	assert(fake.signal_count == 0);

	memset(&fake, 0, sizeof(fake));
	fake.states = live;
	fake.state_count = 4;
	fake.signal_error = EPERM;
	errno = 0;
	assert(inadyn_signal_snapshot(&snapshot, SIGHUP, fake_validate,
				      fake_send_signal, &fake) == -1);
	assert(errno == EPERM && fake.signal_count == 0);

	errno = 0;
	assert(inadyn_signal_snapshot(&snapshot, SIGTERM, fake_validate,
				      fake_send_signal, &fake) == -1);
	assert(errno == EINVAL);
}

static void
test_identity_bound_usr1_signal(void)
{
	struct process_identity identity;
	struct process_snapshot snapshot;
	struct fake_signal fake;
	const enum process_presence live[] = {
		PROCESS_PRESENCE_RUNNING, PROCESS_PRESENCE_RUNNING
	};
	const enum process_presence unknown[] = { PROCESS_PRESENCE_UNKNOWN };

	memset(&identity, 0, sizeof(identity));
	identity.pid = 303;
	memset(&snapshot, 0, sizeof(snapshot));
	snapshot.state = PROCESS_PRESENCE_RUNNING;
	snapshot.items = &identity;
	snapshot.count = 1;

	memset(&fake, 0, sizeof(fake));
	fake.states = live;
	fake.state_count = sizeof(live) / sizeof(live[0]);
	assert(inadyn_signal_snapshot(&snapshot, SIGUSR1, fake_validate,
				      fake_send_signal, &fake) == 0);
	assert(fake.next_state == 2 && fake.signal_count == 1);
	assert(fake.signalled_pids[0] == identity.pid);
	assert(fake.signalled_signals[0] == SIGUSR1);

	memset(&fake, 0, sizeof(fake));
	fake.states = unknown;
	fake.state_count = sizeof(unknown) / sizeof(unknown[0]);
	fake.validation_error = EACCES;
	errno = 0;
	assert(inadyn_signal_snapshot(&snapshot, SIGUSR1, fake_validate,
				      fake_send_signal, &fake) == -1);
	assert(errno == EACCES && fake.signal_count == 0);
}

static void
test_start_postcondition_status(void)
{
	assert(inadyn_start_status(17, PROCESS_PRESENCE_RUNNING, 0) == 17);
	assert(inadyn_start_status(0, PROCESS_PRESENCE_RUNNING, 0) == 0);
	assert(inadyn_start_status(0, PROCESS_PRESENCE_UNKNOWN, EACCES) == EACCES);
	assert(inadyn_start_status(0, PROCESS_PRESENCE_UNKNOWN, 0) == EIO);
	assert(inadyn_start_status(0, PROCESS_PRESENCE_ABSENT, 0) == ETIMEDOUT);
	assert(inadyn_start_status(0, (enum process_presence)7, 0) == EINVAL);
	assert(errno == EINVAL);
}

int
main(void)
{
	test_update_configuration_gate();
	test_presence_decisions();
	test_start_postcondition_status();
	test_config_preflight_requires_all_live_identities();
	test_identity_bound_hup_signal();
	test_identity_bound_usr1_signal();
	puts("inadyn tri-state daemon and PID-bound HUP/USR1 policy: PASS");
	return 0;
}
