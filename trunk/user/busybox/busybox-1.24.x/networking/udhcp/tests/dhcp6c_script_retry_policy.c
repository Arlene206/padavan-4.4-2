#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "dhcp6c_script_retry_policy.h"
#include "dhcp_script_env_policy.h"

#define CHECK(condition) do { \
	if (!(condition)) { \
		fprintf(stderr, "%s:%d: check failed: %s\n", __FILE__, __LINE__, #condition); \
		return 1; \
	} \
} while (0)

static int publish_calls;
static int publish_fail_at;
static int unset_calls;
static const char *unset_entries[8];

static int fake_env_publish(char *entry)
{
	int call = publish_calls++;

	(void)entry;
	if (call == publish_fail_at) {
		if (call != 0)
			errno = E2BIG;
		return -1;
	}
	return 0;
}

static void fake_env_unset(const char *entry)
{
	if (unset_calls < (int)(sizeof(unset_entries) / sizeof(unset_entries[0])))
		unset_entries[unset_calls] = entry;
	unset_calls++;
}

static void reset_env_publish_fixture(int fail_at)
{
	publish_calls = 0;
	publish_fail_at = fail_at;
	unset_calls = 0;
	memset(unset_entries, 0, sizeof(unset_entries));
}

static int test_environment_publish_rolls_back_partial_set(void)
{
	char *envp[] = { "REASON=NBI", "interface=wan0", "new_dns=2001:4860::1", NULL };

	reset_env_publish_fixture(2);
	errno = 0;
	CHECK(dhcp6c_script_env_publish(envp, fake_env_publish, fake_env_unset) == -1);
	CHECK(errno == E2BIG);
	CHECK(publish_calls == 3);
	CHECK(unset_calls == 2);
	CHECK(strcmp(unset_entries[0], "interface=wan0") == 0);
	CHECK(strcmp(unset_entries[1], "REASON=NBI") == 0);
	return 0;
}

static int test_environment_publish_failure_without_errno_uses_enomem(void)
{
	char *envp[] = { "REASON=NBI", "interface=wan0", NULL };

	reset_env_publish_fixture(0);
	errno = 0;
	CHECK(dhcp6c_script_env_publish(envp, fake_env_publish, fake_env_unset) == -1);
	CHECK(errno == ENOMEM);
	CHECK(publish_calls == 1);
	CHECK(unset_calls == 0);
	return 0;
}

static int test_environment_publish_success_keeps_all_entries(void)
{
	char *envp[] = { "REASON=NBI", "interface=wan0", "new_dns=2001:4860::1", NULL };

	reset_env_publish_fixture(-1);
	CHECK(dhcp6c_script_env_publish(envp, fake_env_publish, fake_env_unset) == 0);
	CHECK(publish_calls == 3);
	CHECK(unset_calls == 0);
	return 0;
}

static int test_environment_publish_null_callback_is_rejected(void)
{
	char *envp[] = { "REASON=NBI", NULL };

	CHECK(dhcp6c_script_env_publish(envp, NULL, fake_env_unset) == -1);
	CHECK(errno == EINVAL);
	return 0;
}

static int test_dhcpv4_uses_shared_atomic_environment_publisher(void)
{
	char *envp[] = { "interface=eth0", "ip=192.0.2.10", "lease=3600", NULL };

	reset_env_publish_fixture(1);
	errno = 0;
	CHECK(dhcp_script_env_publish(envp, fake_env_publish, fake_env_unset) == -1);
	CHECK(errno == E2BIG);
	CHECK(publish_calls == 2);
	CHECK(unset_calls == 1);
	CHECK(strcmp(unset_entries[0], "interface=eth0") == 0);
	return 0;
}

static int test_shared_publisher_rejects_null_environment_and_unsetter(void)
{
	char *envp[] = { "interface=eth0", NULL };

	errno = 0;
	CHECK(dhcp_script_env_publish(NULL, fake_env_publish, fake_env_unset) == -1);
	CHECK(errno == EINVAL);
	errno = 0;
	CHECK(dhcp_script_env_publish(envp, fake_env_publish, NULL) == -1);
	CHECK(errno == EINVAL);
	return 0;
}

static int test_timer_remaining_clamps_expired_deadlines(void)
{
	CHECK(dhcp6_timer_remaining_ms(1500ULL, 1000ULL) == 500ULL);
	CHECK(dhcp6_timer_remaining_ms(1000ULL, 1000ULL) == 0ULL);
	CHECK(dhcp6_timer_remaining_ms(999ULL, 1000ULL) == 0ULL);
	CHECK(dhcp6_timer_remaining_ms(~0ULL, 1000ULL) == (~0ULL - 1000ULL));
	return 0;
}

static int test_success_does_not_retry(void)
{
	CHECK(!dhcp6c_script_retry_required(0));
	return 0;
}

static int test_positive_exit_status_retries(void)
{
	CHECK(dhcp6c_script_retry_required(1));
	CHECK(dhcp6c_script_retry_required(2));
	CHECK(dhcp6c_script_retry_required(255));
	return 0;
}

static int test_spawn_failure_retries(void)
{
	CHECK(dhcp6c_script_retry_required(-1));
	return 0;
}

static int test_retry_delay_is_bounded_and_nonzero(void)
{
	CHECK(DHCP6C_SCRIPT_RETRY_DELAY_MS == 5000ULL);
	CHECK(DHCP6C_SCRIPT_RETRY_DELAY_MS > 0);
	return 0;
}

static int
test_ipv6_source_address_storage_fits_kernel_sockaddr(void)
{
	struct sockaddr_storage from;
	struct sockaddr_in6 peer;

	CHECK(sizeof(from) >= sizeof(peer));
	CHECK(sizeof(from) >= sizeof(struct sockaddr_in6));
	return 0;
}

int main(void)
{
	if (test_dhcpv4_uses_shared_atomic_environment_publisher() != 0 ||
	    test_ipv6_source_address_storage_fits_kernel_sockaddr() != 0 ||
	    test_shared_publisher_rejects_null_environment_and_unsetter() != 0 ||
	    test_environment_publish_rolls_back_partial_set() != 0 ||
	    test_environment_publish_failure_without_errno_uses_enomem() != 0 ||
	    test_environment_publish_success_keeps_all_entries() != 0 ||
	    test_environment_publish_null_callback_is_rejected() != 0 ||
	    test_timer_remaining_clamps_expired_deadlines() != 0 ||
	    test_success_does_not_retry() != 0 ||
	    test_positive_exit_status_retries() != 0 ||
	    test_spawn_failure_retries() != 0 ||
	    test_retry_delay_is_bounded_and_nonzero() != 0)
		return 1;

	puts("DHCPv4/v6 callback environment and retry policy: PASS");
	return 0;
}
