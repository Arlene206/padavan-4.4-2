#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "ftp_account_auth_policy.h"

struct fake_nvram {
	int locked;
	int begin_calls;
	int get_calls;
	int end_calls;
	int unlocked_gets;
	int fail_begin;
	int fail_end;
	int username0_present;
	int username1_present;
	int password0_present;
	int password1_present;
	int writer_attempt_on_username0;
	int writer_blocked;
	char count[16];
	char username0[ROUTER_ACCOUNT_NV_NAME_SIZE];
	char username1[ROUTER_ACCOUNT_NV_NAME_SIZE];
	char password0[128];
	char password1[128];
};

static void
fake_nvram_init(struct fake_nvram *fake)
{
	memset(fake, 0, sizeof(*fake));
	strcpy(fake->count, "2");
	fake->username0_present = 1;
	fake->username1_present = 1;
	fake->password0_present = 1;
	fake->password1_present = 1;
	strcpy(fake->username0, "alice");
	strcpy(fake->username1, "bob");
	strcpy(fake->password0, "alice-secret");
	strcpy(fake->password1, "bob-secret");
}

static int
fake_nvram_begin(void *context)
{
	struct fake_nvram *fake = (struct fake_nvram *)context;
	++fake->begin_calls;
	if (fake->fail_begin) {
		errno = EBUSY;
		return -1;
	}
	if (fake->locked) {
		errno = EDEADLK;
		return -1;
	}
	fake->locked = 1;
	return 0;
}

static int
fake_nvram_get(void *context, const char *name, char **value)
{
	struct fake_nvram *fake = (struct fake_nvram *)context;
	char key[24];
	int i;

	++fake->get_calls;
	if (!fake->locked) {
		++fake->unlocked_gets;
		errno = EPERM;
		return -1;
	}
	if (strcmp(name, "acc_num") == 0) {
		*value = fake->count;
		return 1;
	}
	if (fake->writer_attempt_on_username0 &&
	    strcmp(name, "acc_username0") == 0) {
		fake->writer_attempt_on_username0 = 0;
		if (fake->locked)
			++fake->writer_blocked;
		else
			strcpy(fake->password0, "mixed-generation-secret");
	}
	for (i = 0; i < 2; ++i) {
		(void)snprintf(key, sizeof(key), "acc_username%d", i);
		if (strcmp(name, key) == 0) {
			if (i == 0 && fake->username0_present) {
				*value = fake->username0;
				return 1;
			}
			if (i == 1 && fake->username1_present) {
				*value = fake->username1;
				return 1;
			}
			return 0;
		}
		(void)snprintf(key, sizeof(key), "acc_password%d", i);
		if (strcmp(name, key) == 0) {
			if (i == 0 && fake->password0_present) {
				*value = fake->password0;
				return 1;
			}
			if (i == 1 && fake->password1_present) {
				*value = fake->password1;
				return 1;
			}
			return 0;
		}
	}
	return 0;
}

static int
fake_nvram_end(void *context)
{
	struct fake_nvram *fake = (struct fake_nvram *)context;
	++fake->end_calls;
	fake->locked = 0;
	if (fake->fail_end) {
		errno = EIO;
		return -1;
	}
	return 0;
}

static const struct vsftpd_account_auth_nvram_ops fake_ops = {
	fake_nvram_begin,
	fake_nvram_get,
	fake_nvram_end
};

static void
test_credentials_are_read_under_one_transaction(void)
{
	struct fake_nvram fake;
	char password[128];

	fake_nvram_init(&fake);
	fake.writer_attempt_on_username0 = 1;
	strcpy(password, "stale-buffer");
	assert(vsftpd_account_auth_password_read(
		&fake_ops, &fake, "alice", 3, password, sizeof(password)) == 1);
	assert(strcmp(password, "alice-secret") == 0);
	assert(fake.begin_calls == 1 && fake.end_calls == 1);
	assert(fake.get_calls == 4);
	assert(fake.writer_blocked == 1);
	assert(fake.unlocked_gets == 0 && !fake.locked);
}

static void
test_absent_account_returns_no_password(void)
{
	struct fake_nvram fake;
	char password[128];

	fake_nvram_init(&fake);
	strcpy(password, "stale-buffer");
	assert(vsftpd_account_auth_password_read(
		&fake_ops, &fake, "carol", 3, password, sizeof(password)) == 0);
	assert(password[0] == '\0');
	assert(fake.begin_calls == 1 && fake.end_calls == 1);
}

static void
test_incomplete_or_ambiguous_inventory_fails_closed(void)
{
	struct fake_nvram fake;
	char password[128];

	fake_nvram_init(&fake);
	fake.username1_present = 0;
	assert(vsftpd_account_auth_password_read(
		&fake_ops, &fake, "alice", 3, password, sizeof(password)) == -1);
	assert(password[0] == '\0');

	fake_nvram_init(&fake);
	strcpy(fake.username1, "alice");
	assert(vsftpd_account_auth_password_read(
		&fake_ops, &fake, "alice", 3, password, sizeof(password)) == -1);
	assert(password[0] == '\0');
}

static void
test_missing_or_empty_password_fails_closed(void)
{
	struct fake_nvram fake;
	char password[128];

	fake_nvram_init(&fake);
	fake.password0_present = 0;
	assert(vsftpd_account_auth_password_read(
		&fake_ops, &fake, "alice", 3, password, sizeof(password)) == -1);
	assert(password[0] == '\0');

	fake_nvram_init(&fake);
	fake.password0[0] = '\0';
	assert(vsftpd_account_auth_password_read(
		&fake_ops, &fake, "alice", 3, password, sizeof(password)) == -1);
	assert(password[0] == '\0');
}

static void
test_transaction_errors_discard_copied_credentials(void)
{
	struct fake_nvram fake;
	char password[128];

	fake_nvram_init(&fake);
	fake.fail_begin = 1;
	strcpy(password, "stale-buffer");
	assert(vsftpd_account_auth_password_read(
		&fake_ops, &fake, "alice", 3, password, sizeof(password)) == -1);
	assert(password[0] == '\0');
	assert(fake.get_calls == 0 && fake.end_calls == 0);

	fake_nvram_init(&fake);
	fake.fail_end = 1;
	strcpy(password, "stale-buffer");
	assert(vsftpd_account_auth_password_read(
		&fake_ops, &fake, "alice", 3, password, sizeof(password)) == -1);
	assert(password[0] == '\0');
	assert(fake.end_calls == 1 && !fake.locked);
}

static void
test_malformed_and_overflowing_account_count_fails_closed(void)
{
	struct fake_nvram fake;
	char password[128];

	fake_nvram_init(&fake);
	strcpy(fake.count, "2junk");
	assert(vsftpd_account_auth_password_read(
		&fake_ops, &fake, "alice", 3, password, sizeof(password)) == -1);

	fake_nvram_init(&fake);
	strcpy(fake.count, "4");
	assert(vsftpd_account_auth_password_read(
		&fake_ops, &fake, "alice", 3, password, sizeof(password)) == -1);
}

int
main(void)
{
	test_credentials_are_read_under_one_transaction();
	test_absent_account_returns_no_password();
	test_incomplete_or_ambiguous_inventory_fails_closed();
	test_missing_or_empty_password_fails_closed();
	test_transaction_errors_discard_copied_credentials();
	test_malformed_and_overflowing_account_count_fails_closed();
	puts("vsftpd credential snapshot policy: PASS");
	return 0;
}
