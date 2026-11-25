#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include "dhcp6c_ia_policy.h"

struct fake_target {
	int id;
	int installed;
	int failures_left;
	int attempts;
};

static int
reconcile(struct fake_target *targets, size_t count, int server_interface)
{
	size_t i;

	for (i = 0; i < count; ++i) {
		if (!dhcp6c_ia_pd_target_needs_apply(
		    targets[i].id == server_interface, targets[i].installed))
			continue;
		++targets[i].attempts;
		if (targets[i].failures_left > 0) {
			--targets[i].failures_left;
			return EIO;
		}
		targets[i].installed = 1;
	}
	return 0;
}

static void
test_partial_pd_failure_is_retried_without_reapplying_successes(void)
{
	struct fake_target targets[] = {
		{ 1, 0, 0, 0 }, /* DHCP-facing interface: never assign here */
		{ 2, 0, 0, 0 },
		{ 3, 0, 1, 0 }, /* transient kernel failure on first attempt */
		{ 4, 0, 0, 0 }
	};
	int status;

	status = reconcile(targets, sizeof(targets) / sizeof(targets[0]), 1);
	assert(status == EIO);
	assert(!targets[0].installed && targets[0].attempts == 0);
	assert(targets[1].installed && targets[1].attempts == 1);
	assert(!targets[2].installed && targets[2].attempts == 1);
	assert(!targets[3].installed && targets[3].attempts == 0);

	status = reconcile(targets, sizeof(targets) / sizeof(targets[0]), 1);
	assert(status == 0);
	assert(targets[0].attempts == 0);
	assert(targets[1].installed && targets[1].attempts == 1);
	assert(targets[2].installed && targets[2].attempts == 2);
	assert(targets[3].installed && targets[3].attempts == 1);

	assert(reconcile(targets, sizeof(targets) / sizeof(targets[0]), 1) == 0);
	assert(targets[1].attempts == 1);
	assert(targets[2].attempts == 2);
	assert(targets[3].attempts == 1);
}

static void
test_first_error_and_retry_gate(void)
{
	int status = 0;
	status = dhcp6c_ia_first_error(status, EIO);
	status = dhcp6c_ia_first_error(status, EINVAL);
	assert(status == EIO);
	assert(dhcp6c_ia_retry_required(status));
	assert(!dhcp6c_ia_retry_required(0));
	assert(dhcp6c_ia_first_error(0, 0) == 0);
}

int
main(void)
{
	test_partial_pd_failure_is_retried_without_reapplying_successes();
	test_first_error_and_retry_gate();
	puts("DHCPv6 IA delegated-prefix retry policy: PASS");
	return 0;
}
