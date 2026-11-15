#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "dnsmasq_reload_policy.h"

struct fake {
	enum process_presence states[8];
	int state_errors[8];
	int state_count;
	int state_index;
	pid_t signalled[8];
	int signals[8];
	int signal_count;
	int signal_status;
	int signal_errno;
};

static enum process_presence
fake_validate(void *context, const struct process_identity *identity)
{
	struct fake *fake = context;
	enum process_presence state;
	(void)identity;
	assert(fake->state_index < fake->state_count);
	state = fake->states[fake->state_index];
	if (state == PROCESS_PRESENCE_UNKNOWN)
		errno = fake->state_errors[fake->state_index];
	fake->state_index++;
	return state;
}

static int
fake_signal(void *context, pid_t pid, int signal_number)
{
	struct fake *fake = context;
	fake->signalled[fake->signal_count] = pid;
	fake->signals[fake->signal_count] = signal_number;
	fake->signal_count++;
	if (fake->signal_status != 0)
		errno = fake->signal_errno;
	return fake->signal_status;
}

static void
fake_init(struct fake *fake, const enum process_presence *states,
	  const int *errors, int count)
{
	int i;
	memset(fake, 0, sizeof(*fake));
	for (i = 0; i < count; i++) {
		fake->states[i] = states[i];
		if (errors)
			fake->state_errors[i] = errors[i];
	}
	fake->state_count = count;
}

int
main(void)
{
	struct process_identity identities[2];
	struct process_snapshot snapshot;
	struct fake fake;
	const enum process_presence valid[] = {
		PROCESS_PRESENCE_RUNNING, PROCESS_PRESENCE_RUNNING,
		PROCESS_PRESENCE_RUNNING, PROCESS_PRESENCE_RUNNING
	};
	const enum process_presence initial_unknown[] = {
		PROCESS_PRESENCE_RUNNING, PROCESS_PRESENCE_UNKNOWN
	};
	const enum process_presence signal_phase_unknown[] = {
		PROCESS_PRESENCE_RUNNING, PROCESS_PRESENCE_RUNNING,
		PROCESS_PRESENCE_RUNNING, PROCESS_PRESENCE_UNKNOWN
	};
	const enum process_presence vanished[] = {
		PROCESS_PRESENCE_RUNNING, PROCESS_PRESENCE_ABSENT,
		PROCESS_PRESENCE_RUNNING, PROCESS_PRESENCE_ABSENT
	};
	const int initial_errors[] = {0, EACCES};
	const int signal_errors[] = {0, 0, 0, EIO};
	int status;

	assert(dnsmasq_reload_notification_decision(PROCESS_PRESENCE_RUNNING) == 1);
	assert(dnsmasq_reload_notification_decision(PROCESS_PRESENCE_ABSENT) == 0);
	assert(dnsmasq_reload_notification_decision(PROCESS_PRESENCE_UNKNOWN) == -1);
	assert(dnsmasq_reload_post_signal_status(ESRCH, PROCESS_PRESENCE_ABSENT, 0) == 0);
	assert(dnsmasq_reload_post_signal_status(ESRCH, PROCESS_PRESENCE_RUNNING, 0) == ESRCH);
	assert(dnsmasq_reload_post_signal_status(ESRCH, PROCESS_PRESENCE_UNKNOWN, EACCES) == EACCES);
	assert(dnsmasq_reload_post_signal_status(ESRCH, PROCESS_PRESENCE_UNKNOWN, 0) == EIO);
	assert(dnsmasq_reload_post_signal_status(EPERM, PROCESS_PRESENCE_ABSENT, 0) == EPERM);
	assert(dnsmasq_reload_post_signal_status(ESRCH, (enum process_presence)7, 0) == EIO);

	memset(identities, 0, sizeof(identities));
	identities[0].pid = 501;
	identities[1].pid = 502;
	snapshot.state = PROCESS_PRESENCE_RUNNING;
	snapshot.items = identities;
	snapshot.count = 2;

	fake_init(&fake, valid, NULL, 4);
	assert(dnsmasq_reload_signal_snapshot(&snapshot, fake_validate, fake_signal, &fake) == 0);
	assert(fake.state_index == 4 && fake.signal_count == 2);
	assert(fake.signalled[0] == 501 && fake.signalled[1] == 502);
	assert(fake.signals[0] == SIGHUP && fake.signals[1] == SIGHUP);

	fake_init(&fake, initial_unknown, initial_errors, 2);
	status = dnsmasq_reload_signal_snapshot(&snapshot, fake_validate, fake_signal, &fake);
	assert(status == EACCES && fake.signal_count == 0);

	fake_init(&fake, signal_phase_unknown, signal_errors, 4);
	status = dnsmasq_reload_signal_snapshot(&snapshot, fake_validate, fake_signal, &fake);
	assert(status == EIO && fake.signal_count == 1 && fake.signalled[0] == 501);

	fake_init(&fake, vanished, NULL, 4);
	assert(dnsmasq_reload_signal_snapshot(&snapshot, fake_validate, fake_signal, &fake) == 0);
	assert(fake.signal_count == 1 && fake.signalled[0] == 501);

	snapshot.state = PROCESS_PRESENCE_ABSENT;
	snapshot.items = NULL;
	snapshot.count = 0;
	fake_init(&fake, valid, NULL, 4);
	assert(dnsmasq_reload_signal_snapshot(&snapshot, fake_validate, fake_signal, &fake) == ESRCH);
	assert(fake.state_index == 0 && fake.signal_count == 0);
	snapshot.state = PROCESS_PRESENCE_UNKNOWN;
	assert(dnsmasq_reload_signal_snapshot(&snapshot, fake_validate, fake_signal, &fake) == EIO);
	snapshot.state = PROCESS_PRESENCE_RUNNING;
	assert(dnsmasq_reload_signal_snapshot(&snapshot, NULL, fake_signal, &fake) == EINVAL);
	snapshot.items = NULL;
	assert(dnsmasq_reload_signal_snapshot(&snapshot, fake_validate, fake_signal, &fake) == EIO);

	snapshot.items = identities;
	snapshot.count = 2;
	fake_init(&fake, valid, NULL, 4);
	fake.signal_status = -1;
	fake.signal_errno = EPERM;
	assert(dnsmasq_reload_signal_snapshot(&snapshot, fake_validate, fake_signal, &fake) == EPERM);
	assert(fake.signal_count == 1);

	puts("DNSMasq cache reload identity and tri-state policy: PASS");
	return 0;
}
