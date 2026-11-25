#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include "ftp_daemon_policy.h"
#include "service_daemon_wait_policy.h"

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

	assert(strcmp(daemon, "vsftpd") == 0 ||
	       strcmp(daemon, "minidlna") == 0 ||
	       strcmp(daemon, "mt-daapd") == 0);
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

static const struct service_daemon_wait_ops fake_ops = {
	fake_is_running,
	fake_sleep_us
};

int
main(void)
{
	int enabled = -1;

	assert(ftp_enable_value_status(EIO, 1, &enabled) == EIO);
	assert(enabled == 0);
	assert(ftp_enable_value_status(-1, 1, &enabled) == EIO);
	assert(enabled == 0);
	assert(ftp_enable_value_status(0, 2, &enabled) == EINVAL);
	assert(enabled == 0);
	assert(ftp_enable_value_status(0, 1, &enabled) == 0);
	assert(enabled == 1);
	assert(ftp_enable_value_status(0, 0, &enabled) == 0);
	assert(enabled == 0);
	assert(ftp_enable_value_status(0, 1, NULL) == EINVAL);

	assert(ftp_stop_should_reconcile_firewall(PROCESS_PRESENCE_ABSENT));
	assert(!ftp_stop_should_reconcile_firewall(PROCESS_PRESENCE_RUNNING));
	assert(!ftp_stop_should_reconcile_firewall(PROCESS_PRESENCE_UNKNOWN));
	assert(!ftp_stop_should_reconcile_firewall((enum process_presence)9));

	/* Explicit stop must also clear stale rules for absent -> absent. */
	assert(ftp_firewall_reconcile_needed(PROCESS_PRESENCE_ABSENT,
					     PROCESS_PRESENCE_ABSENT));
	assert(ftp_firewall_reconcile_needed(PROCESS_PRESENCE_RUNNING,
					     PROCESS_PRESENCE_ABSENT));
	assert(ftp_firewall_reconcile_needed(PROCESS_PRESENCE_ABSENT,
					     PROCESS_PRESENCE_RUNNING));
	assert(ftp_firewall_reconcile_needed(PROCESS_PRESENCE_UNKNOWN,
					     PROCESS_PRESENCE_UNKNOWN));
	assert(!ftp_firewall_reconcile_needed(PROCESS_PRESENCE_RUNNING,
					      PROCESS_PRESENCE_RUNNING));

	/* Storage-stop must clear stale FTP rules when no daemon was running. */
	assert(ftp_storage_stop_needs_firewall_refresh(0,
						     PROCESS_PRESENCE_ABSENT));
	assert(ftp_storage_stop_needs_firewall_refresh(1,
						     PROCESS_PRESENCE_ABSENT));
	/* Preserve refresh attempts for previously present/unknown daemons. */
	assert(ftp_storage_stop_needs_firewall_refresh(1,
						     PROCESS_PRESENCE_RUNNING));
	assert(ftp_storage_stop_needs_firewall_refresh(1,
						     PROCESS_PRESENCE_UNKNOWN));
	assert(ftp_storage_stop_needs_firewall_refresh(1, (enum process_presence)9));
	/* An absent-before / unknown-after observation must fail closed. */
	assert(!ftp_storage_stop_needs_firewall_refresh(0,
						      PROCESS_PRESENCE_RUNNING));
	assert(!ftp_storage_stop_needs_firewall_refresh(0,
						      PROCESS_PRESENCE_UNKNOWN));
	assert(!ftp_storage_stop_needs_firewall_refresh(0,
						      (enum process_presence)9));

	assert(!ftp_failed_start_may_leave_daemon(PROCESS_PRESENCE_ABSENT));
	assert(ftp_failed_start_may_leave_daemon(PROCESS_PRESENCE_RUNNING));
	assert(ftp_failed_start_may_leave_daemon(PROCESS_PRESENCE_UNKNOWN));
	assert(ftp_failed_start_may_leave_daemon((enum process_presence)9));

	{
		const int states[] = { 0, 1, 1 };
		struct fake_daemon fake = { states, 3, 0, 0, 0, 0, 0 };

		assert(service_daemon_wait_for_stable_presence("vsftpd", 3,
							    &fake_ops, &fake) == 0);
		assert(fake.next_state == 3);
		assert(fake.slept_us == 600000U);
	}
	{
		const int states[] = { PROCESS_PRESENCE_RUNNING,
				       PROCESS_PRESENCE_RUNNING };
		struct fake_daemon fake = { states, 2, 0, 0, 0, 0, 0 };

		assert(service_daemon_wait_for_stable_presence(
			"minidlna", 1, &fake_ops, &fake) == 0);
		assert(fake.next_state == 2 && fake.slept_us == 500000U);
	}
	{
		const int states[] = { PROCESS_PRESENCE_RUNNING,
				       PROCESS_PRESENCE_RUNNING };
		struct fake_daemon fake = { states, 2, 0, 0, 0, 0, 0 };

		assert(service_daemon_wait_for_stable_presence(
			"mt-daapd", 1, &fake_ops, &fake) == 0);
		assert(fake.next_state == 2 && fake.slept_us == 500000U);
	}

	{
		const int states[] = { 1, 0, 0 };
		struct fake_daemon fake = { states, 3, 0, 0, 0, 0, 0 };

		errno = 0;
		assert(service_daemon_wait_for_stable_presence("vsftpd", 2,
							    &fake_ops, &fake) == -1);
		assert(errno == ETIMEDOUT);
		assert(fake.next_state == 3);
		assert(fake.slept_us == 700000U);
	}

	{
		struct fake_daemon fake = { NULL, 0, 0, 0, 0, 0, 0 };

		errno = 0;
		assert(service_daemon_wait_for_stable_presence("vsftpd", 0,
							    &fake_ops, &fake) == -1);
		assert(errno == EINVAL);
		assert(fake.next_state == 0 && fake.slept_us == 0);
	}

	{
		struct fake_daemon fake = { NULL, 0, 0, 0, 0, 0, 0 };

		errno = 0;
		assert(service_daemon_wait_for_stable_presence("vsftpd", 1,
							    NULL, &fake) == -1);
		assert(errno == EINVAL);
	}

	{
		const int states[] = { PROCESS_PRESENCE_UNKNOWN };
		struct fake_daemon fake = { states, 1, 0, 0, EACCES, 0, 0 };
		errno = 0;
		assert(service_daemon_wait_for_stable_presence("vsftpd", 3, &fake_ops, &fake) == -1);
		assert(errno == EACCES && fake.next_state == 1 && fake.slept_us == 0);
	}
	{
		const int states[] = { PROCESS_PRESENCE_RUNNING, PROCESS_PRESENCE_UNKNOWN };
		struct fake_daemon fake = { states, 2, 0, 0, EACCES, 0, 0 };
		errno = 0;
		assert(service_daemon_wait_for_stable_presence("vsftpd", 3, &fake_ops, &fake) == -1);
		assert(errno == EACCES && fake.next_state == 2 && fake.slept_us == 500000U);
	}
	{
		const int states[] = { (int)9 };
		struct fake_daemon fake = { states, 1, 0, 0, 0, 0, 0 };
		errno = 0;
		assert(service_daemon_wait_for_stable_presence("vsftpd", 1, &fake_ops, &fake) == -1);
		assert(errno == EIO);
	}

	{
		const int states[] = { PROCESS_PRESENCE_RUNNING };
		struct fake_daemon fake = { states, 1, 0, 0, 0, -1, EACCES };

		errno = 0;
		assert(service_daemon_wait_for_stable_presence(
			"vsftpd", 3, &fake_ops, &fake) == -1);
		assert(errno == EACCES);
		assert(fake.next_state == 1 && fake.slept_us == 500000U);
	}

	puts("generic detached-daemon tri-state readiness policy: PASS");
	return 0;
}
