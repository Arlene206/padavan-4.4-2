#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "httpd_auth_nvram_policy.h"

struct fake_nvram {
	const char *username;
	const char *password;
	int fail_read_at;
	int failure_errno;
	int begin_status;
	int end_status;
	int end_errno;
	int reads;
	int begins;
	int ends;
	char cache[128];
};

static int
fake_begin(void *opaque)
{
	struct fake_nvram *fake = (struct fake_nvram *)opaque;
	++fake->begins;
	if (fake->begin_status != 0) {
		errno = fake->failure_errno;
		return -1;
	}
	return 0;
}

static int
fake_get(void *opaque, const char *name, char **value)
{
	struct fake_nvram *fake = (struct fake_nvram *)opaque;
	const char *source;
	int index = fake->reads++;

	assert(index < 2);
	if (index == fake->fail_read_at) {
		errno = fake->failure_errno;
		return -1;
	}
	if (strcmp(name, "http_username") == 0)
		source = fake->username;
	else {
		assert(strcmp(name, "http_passwd") == 0);
		source = fake->password;
	}
	if (source == NULL) {
		*value = NULL;
		return 0;
	}
	assert(strlen(source) < sizeof(fake->cache));
	snprintf(fake->cache, sizeof(fake->cache), "%s", source);
	*value = fake->cache;
	return 1;
}

static int
fake_end(void *opaque)
{
	struct fake_nvram *fake = (struct fake_nvram *)opaque;
	++fake->ends;
	if (fake->end_status != 0) {
		errno = fake->end_errno;
		return -1;
	}
	return 0;
}

static int
snapshot(struct fake_nvram *fake, char *username, size_t username_size,
         char *password, size_t password_size)
{
	return httpd_auth_nvram_snapshot_credentials(
		"http_username", "http_passwd", username, username_size,
		password, password_size, fake_begin, fake_get, fake_end, fake);
}

static void
test_credentials_are_copied_before_cache_reuse(void)
{
	struct fake_nvram fake;
	char username[32], password[32];

	memset(&fake, 0, sizeof(fake));
	fake.fail_read_at = -1;
	fake.username = "admin";
	fake.password = "secret";
	assert(snapshot(&fake, username, sizeof(username),
			password, sizeof(password)) == 0);
	assert(strcmp(username, "admin") == 0);
	assert(strcmp(password, "secret") == 0);
	assert(fake.begins == 1 && fake.reads == 2 && fake.ends == 1);
}

static void
test_missing_keys_keep_legacy_empty_defaults(void)
{
	struct fake_nvram fake;
	char username[32] = "stale-user", password[32] = "stale-pass";

	memset(&fake, 0, sizeof(fake));
	fake.fail_read_at = -1;
	assert(snapshot(&fake, username, sizeof(username),
			password, sizeof(password)) == 0);
	assert(username[0] == '\0' && password[0] == '\0');
	assert(fake.reads == 2 && fake.ends == 1);
}

static void
test_username_read_error_invalidates_pair(void)
{
	struct fake_nvram fake;
	char username[32] = "stale-user", password[32] = "stale-pass";

	memset(&fake, 0, sizeof(fake));
	fake.fail_read_at = 0;
	fake.failure_errno = EACCES;
	fake.username = "admin";
	fake.password = "secret";
	assert(snapshot(&fake, username, sizeof(username),
			password, sizeof(password)) == EACCES);
	assert(username[0] == '\0' && password[0] == '\0');
	assert(fake.reads == 1 && fake.ends == 1);
}

static void
test_password_read_error_discards_username_snapshot(void)
{
	struct fake_nvram fake;
	char username[32] = "stale-user", password[32] = "stale-pass";

	memset(&fake, 0, sizeof(fake));
	fake.fail_read_at = 1;
	fake.failure_errno = EIO;
	fake.username = "admin";
	fake.password = "secret";
	assert(snapshot(&fake, username, sizeof(username),
			password, sizeof(password)) == EIO);
	assert(username[0] == '\0' && password[0] == '\0');
	assert(fake.reads == 2 && fake.ends == 1);
}

static void
test_missing_errno_and_unknown_read_status_fail_closed(void)
{
	struct fake_nvram fake;
	char username[32], password[32];

	memset(&fake, 0, sizeof(fake));
	fake.fail_read_at = 0;
	fake.failure_errno = 0;
	assert(snapshot(&fake, username, sizeof(username),
			password, sizeof(password)) == EIO);
	assert(username[0] == '\0' && password[0] == '\0');
	assert(httpd_auth_nvram_read_status(2, 0) == EIO);
}

static void
test_transaction_begin_failure_does_not_read_or_end(void)
{
	struct fake_nvram fake;
	char username[32] = "stale-user", password[32] = "stale-pass";

	memset(&fake, 0, sizeof(fake));
	fake.begin_status = -1;
	fake.failure_errno = EBUSY;
	assert(snapshot(&fake, username, sizeof(username),
			password, sizeof(password)) == EBUSY);
	assert(username[0] == '\0' && password[0] == '\0');
	assert(fake.begins == 1 && fake.reads == 0 && fake.ends == 0);
}

static void
test_transaction_end_failure_invalidates_pair(void)
{
	struct fake_nvram fake;
	char username[32], password[32];

	memset(&fake, 0, sizeof(fake));
	fake.fail_read_at = -1;
	fake.username = "admin";
	fake.password = "secret";
	fake.end_status = -1;
	fake.end_errno = EIO;
	assert(snapshot(&fake, username, sizeof(username),
			password, sizeof(password)) == EIO);
	assert(username[0] == '\0' && password[0] == '\0');
	assert(fake.begins == 1 && fake.reads == 2 && fake.ends == 1);
}

static void
test_primary_read_error_wins_over_end_error(void)
{
	struct fake_nvram fake;
	char username[32], password[32];

	memset(&fake, 0, sizeof(fake));
	fake.fail_read_at = 0;
	fake.failure_errno = EACCES;
	fake.end_status = -1;
	fake.end_errno = EIO;
	assert(snapshot(&fake, username, sizeof(username),
			password, sizeof(password)) == EACCES);
	assert(username[0] == '\0' && password[0] == '\0');
	assert(fake.ends == 1);
}

static void
test_oversized_credential_invalidates_pair(void)
{
	struct fake_nvram fake;
	char username[5], password[32];

	memset(&fake, 0, sizeof(fake));
	fake.fail_read_at = -1;
	fake.username = "admin";
	fake.password = "secret";
	assert(snapshot(&fake, username, sizeof(username),
			password, sizeof(password)) == ENAMETOOLONG);
	assert(username[0] == '\0' && password[0] == '\0');
	assert(fake.ends == 1);
}

static void
test_invalid_arguments_fail_closed(void)
{
	struct fake_nvram fake;
	char username[32] = "stale-user", password[32] = "stale-pass";

	memset(&fake, 0, sizeof(fake));
	assert(httpd_auth_nvram_snapshot_credentials(
		NULL, "http_passwd", username, sizeof(username),
		password, sizeof(password), fake_begin, fake_get, fake_end, &fake) == EINVAL);
	assert(username[0] == '\0' && password[0] == '\0');
	assert(fake.begins == 0 && fake.reads == 0 && fake.ends == 0);
}

int
main(void)
{
	test_credentials_are_copied_before_cache_reuse();
	test_missing_keys_keep_legacy_empty_defaults();
	test_username_read_error_invalidates_pair();
	test_password_read_error_discards_username_snapshot();
	test_missing_errno_and_unknown_read_status_fail_closed();
	test_transaction_begin_failure_does_not_read_or_end();
	test_transaction_end_failure_invalidates_pair();
	test_primary_read_error_wins_over_end_error();
	test_oversized_credential_invalidates_pair();
	test_invalid_arguments_fail_closed();
	puts("HTTPD checked credential snapshot regression: PASS");
	return 0;
}
