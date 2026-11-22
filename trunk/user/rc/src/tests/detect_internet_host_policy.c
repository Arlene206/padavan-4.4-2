#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>

#include "detect_internet_host_policy.h"

struct fake_nvram {
	const char *names[8];
	const char *values[8];
	unsigned int count;
	unsigned int calls;
	char cache[128];
	const char *fail_name;
	int failure_status;
};

static int
fake_nvram_get(void *context, const char *param_name, char **value)
{
	struct fake_nvram *fake = (struct fake_nvram *)context;
	unsigned int i;

	if (!fake || !param_name || !value)
		return EINVAL;
	fake->calls++;
	if (fake->fail_name && strcmp(fake->fail_name, param_name) == 0) {
		errno = fake->failure_status;
		*value = NULL;
		return -1;
	}
	for (i = 0; i < fake->count; ++i) {
		if (strcmp(fake->names[i], param_name) == 0) {
			snprintf(fake->cache, sizeof(fake->cache), "%s",
				 fake->values[i]);
			*value = fake->cache;
			return 1;
		}
	}
	*value = NULL;
	return 0;
}

static void
test_endpoint_address_survives_port_cache_reuse(void)
{
	struct fake_nvram fake;
	char address[16];
	int port;

	memset(&fake, 0, sizeof(fake));
	fake.names[0] = "di_addr0";
	fake.values[0] = "192.0.2.25";
	fake.names[1] = "di_port0";
	fake.values[1] = "5353";
	fake.count = 2;

	assert(detect_internet_read_endpoint(fake_nvram_get, &fake,
					     "di_addr0", "di_port0",
					     address, sizeof(address),
					     &port) == 0);
	assert(fake.calls == 2);
	assert(strcmp(fake.cache, "5353") == 0);
	assert(strcmp(address, "192.0.2.25") == 0);
	assert(port == 5353);
}

static void
test_missing_endpoint_uses_legacy_empty_address_and_default_port(void)
{
	struct fake_nvram fake;
	char address[16] = "stale";
	int port = 999;

	memset(&fake, 0, sizeof(fake));
	assert(detect_internet_read_endpoint(fake_nvram_get, &fake,
					     "di_addr0", "di_port0",
					     address, sizeof(address),
					     &port) == 0);
	assert(fake.calls == 2);
	assert(address[0] == '\0');
	assert(port == 53);
}

static void
test_invalid_or_out_of_range_port_uses_legacy_default(void)
{
	struct fake_nvram fake;
	char address[16];
	int port;

	memset(&fake, 0, sizeof(fake));
	fake.names[0] = "di_addr0";
	fake.values[0] = "192.0.2.25";
	fake.names[1] = "di_port0";
	fake.values[1] = "not-a-port";
	fake.names[2] = "di_port1";
	fake.values[2] = "70000";
	fake.count = 3;

	assert(detect_internet_read_endpoint(fake_nvram_get, &fake,
					     "di_addr0", "di_port0",
					     address, sizeof(address),
					     &port) == 0);
	assert(port == 53);
	assert(detect_internet_read_int_setting(fake_nvram_get, &fake,
						"di_port1", 1, 65535,
						53, &port) == 0);
	assert(port == 53);
}

static void
test_address_backend_error_is_not_treated_as_missing_host(void)
{
	struct fake_nvram fake;
	char address[16] = "stale";
	int port = 999;

	memset(&fake, 0, sizeof(fake));
	fake.fail_name = "di_addr0";
	fake.failure_status = EACCES;
	assert(detect_internet_read_endpoint(fake_nvram_get, &fake,
					     "di_addr0", "di_port0",
					     address, sizeof(address),
					     &port) == EACCES);
	assert(fake.calls == 1);
	assert(address[0] == '\0');
	assert(port == 0);
}

static void
test_port_backend_error_invalidates_entire_endpoint(void)
{
	struct fake_nvram fake;
	char address[16] = "stale";
	int port = 999;

	memset(&fake, 0, sizeof(fake));
	fake.names[0] = "di_addr0";
	fake.values[0] = "192.0.2.25";
	fake.count = 1;
	fake.fail_name = "di_port0";
	fake.failure_status = EIO;

	assert(detect_internet_read_endpoint(fake_nvram_get, &fake,
					     "di_addr0", "di_port0",
					     address, sizeof(address),
					     &port) == EIO);
	assert(fake.calls == 2);
	assert(address[0] == '\0');
	assert(port == 0);
}

static void
test_oversized_address_fails_without_truncation(void)
{
	struct fake_nvram fake;
	char address[16];
	int port;

	memset(&fake, 0, sizeof(fake));
	fake.names[0] = "di_addr0";
	fake.values[0] = "192.0.2.123456789";
	fake.names[1] = "di_port0";
	fake.values[1] = "53";
	fake.count = 2;
	assert(detect_internet_read_endpoint(fake_nvram_get, &fake,
					     "di_addr0", "di_port0",
					     address, sizeof(address),
					     &port) == ENAMETOOLONG);
	assert(fake.calls == 1);
	assert(address[0] == '\0');
	assert(port == 0);
}

static void
test_runtime_integer_backend_error_is_not_replaced_by_default(void)
{
	struct fake_nvram fake;
	int timeout = 99;

	memset(&fake, 0, sizeof(fake));
	fake.fail_name = "di_timeout";
	fake.failure_status = EACCES;
	assert(detect_internet_read_int_setting(fake_nvram_get, &fake,
						"di_timeout", 1, 10, 3,
						&timeout) == EACCES);
	assert(fake.calls == 1);
	assert(timeout == 3);
}

static void
test_poll_mode_uses_legacy_range_fallback(void)
{
	struct fake_nvram fake;
	int poll_mode;

	memset(&fake, 0, sizeof(fake));
	fake.names[0] = "di_poll_mode";
	fake.values[0] = "2";
	fake.count = 1;
	assert(detect_internet_read_int_setting(fake_nvram_get, &fake,
						"di_poll_mode", 0, 1, 0,
						&poll_mode) == 0);
	assert(poll_mode == 0);
}

static void
test_select_fdset_bounds_before_fd_set(void)
{
	assert(detect_internet_select_fd_status(0, FD_SETSIZE) == 0);
	assert(detect_internet_select_fd_status(FD_SETSIZE - 1, FD_SETSIZE) == 0);
	assert(detect_internet_select_fd_status(FD_SETSIZE, FD_SETSIZE) == EOVERFLOW);
	assert(detect_internet_select_fd_status(FD_SETSIZE + 1, FD_SETSIZE) == EOVERFLOW);
	assert(detect_internet_select_fd_status(-1, FD_SETSIZE) == EINVAL);
	assert(detect_internet_select_fd_status(0, 0) == EINVAL);
}

static void
test_invalid_policy_arguments_fail_closed(void)
{
	struct fake_nvram fake;
	char address[16];
	int port;

	memset(&fake, 0, sizeof(fake));
	assert(detect_internet_read_endpoint(NULL, &fake, "di_addr0",
					     "di_port0", address,
					     sizeof(address), &port) == EINVAL);
	assert(detect_internet_read_endpoint(fake_nvram_get, &fake, "di_addr0",
					     "di_port0", NULL, 16,
					     &port) == EINVAL);
	assert(detect_internet_read_int_setting(fake_nvram_get, &fake,
						"di_poll_mode", 0, 1, 0,
						NULL) == EINVAL);
	assert(fake.calls == 0);
}

int
main(void)
{
	test_endpoint_address_survives_port_cache_reuse();
	test_missing_endpoint_uses_legacy_empty_address_and_default_port();
	test_invalid_or_out_of_range_port_uses_legacy_default();
	test_address_backend_error_is_not_treated_as_missing_host();
	test_port_backend_error_invalidates_entire_endpoint();
	test_oversized_address_fails_without_truncation();
	test_runtime_integer_backend_error_is_not_replaced_by_default();
	test_poll_mode_uses_legacy_range_fallback();
	test_select_fdset_bounds_before_fd_set();
	test_invalid_policy_arguments_fail_closed();
	puts("detect_internet checked endpoint settings policy: PASS");
	return 0;
}
