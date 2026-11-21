#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "lan_kernel_name_policy.h"

static int hostname_status;
static int domain_status;
static int hostname_calls;
static int domain_calls;

static int
mock_sethostname(const char *name, size_t length)
{
	hostname_calls++;
	assert(strcmp(name, "router") == 0);
	assert(length == strlen("router"));
	return hostname_status;
}

static int
mock_setdomainname(const char *name, size_t length)
{
	domain_calls++;
	assert(strcmp(name, "home.arpa") == 0);
	assert(length == strlen("home.arpa"));
	return domain_status;
}

static const lan_kernel_name_ops_t mock_ops = {
	mock_sethostname,
	mock_setdomainname
};

static void
reset_case(int host_status, int domain_status_value)
{
	hostname_status = host_status;
	domain_status = domain_status_value;
	hostname_calls = 0;
	domain_calls = 0;
}

static void
test_success_updates_both_kernel_names(void)
{
	reset_case(0, 0);
	assert(lan_kernel_names_apply("router", "home.arpa", &mock_ops) == 0);
	assert(hostname_calls == 1);
	assert(domain_calls == 1);
}

static void
test_hostname_failure_does_not_skip_domain_update(void)
{
	reset_case(EPERM, EIO);
	assert(lan_kernel_names_apply("router", "home.arpa", &mock_ops) == EPERM);
	assert(hostname_calls == 1);
	assert(domain_calls == 1);
}

static void
test_domain_failure_is_reported_after_hostname_update(void)
{
	reset_case(0, ENAMETOOLONG);
	assert(lan_kernel_names_apply("router", "home.arpa", &mock_ops) == ENAMETOOLONG);
	assert(hostname_calls == 1);
	assert(domain_calls == 1);
}

static void
test_invalid_inputs_fail_before_system_calls(void)
{
	reset_case(0, 0);
	assert(lan_kernel_names_apply(NULL, "home.arpa", &mock_ops) == EINVAL);
	assert(lan_kernel_names_apply("router", NULL, &mock_ops) == EINVAL);
	assert(lan_kernel_names_apply("router", "home.arpa", NULL) == EINVAL);
	assert(hostname_calls == 0);
	assert(domain_calls == 0);
}

static void
test_missing_setter_fails_closed(void)
{
	const lan_kernel_name_ops_t incomplete = { mock_sethostname, NULL };

	reset_case(0, 0);
	assert(lan_kernel_names_apply("router", "home.arpa", &incomplete) == EINVAL);
	assert(hostname_calls == 0);
	assert(domain_calls == 0);
}

int
main(void)
{
	test_success_updates_both_kernel_names();
	test_hostname_failure_does_not_skip_domain_update();
	test_domain_failure_is_reported_after_hostname_update();
	test_invalid_inputs_fail_before_system_calls();
	test_missing_setter_fails_closed();
	puts("LAN kernel hostname/domain status regression: PASS");
	return 0;
}
