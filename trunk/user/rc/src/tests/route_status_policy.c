#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include "../route_status_policy.h"

static int
apply_results(const int *results, size_t count, size_t *attempts)
{
	int status = 0;
	size_t i;
	for (i = 0; i < count; ++i) {
		++*attempts;
		status = route_status_accumulate(status, results[i]);
	}
	return status;
}

static void
test_static_mtu_failure_is_not_masked_by_later_success(void)
{
	const int startup_steps[] = { EINVAL, 0, 0 };
	size_t attempts = 0;

	assert(apply_results(startup_steps,
			     sizeof(startup_steps) / sizeof(startup_steps[0]),
			     &attempts) == EINVAL);
	assert(attempts == sizeof(startup_steps) / sizeof(startup_steps[0]));
}

static void
test_wan_post_script_failure_is_reported_and_later_steps_continue(void)
{
	const int startup_steps[] = { 0, EIO, 0 };
	size_t attempts = 0;

	/* Routes succeed, the custom post-WAN hook fails, and later startup still runs. */
	assert(apply_results(startup_steps,
			     sizeof(startup_steps) / sizeof(startup_steps[0]),
			     &attempts) == EIO);
	assert(attempts == sizeof(startup_steps) / sizeof(startup_steps[0]));
}

static void
test_wan_post_down_script_failure_is_reported_and_later_steps_continue(void)
{
	const int teardown_steps[] = { 0, EIO, 0 };
	size_t attempts = 0;

	/* Earlier cleanup succeeds, the custom down hook fails, later detection runs. */
	assert(apply_results(teardown_steps,
			     sizeof(teardown_steps) / sizeof(teardown_steps[0]),
			     &attempts) == EIO);
	assert(attempts == sizeof(teardown_steps) / sizeof(teardown_steps[0]));
}

int
main(void)
{
	test_static_mtu_failure_is_not_masked_by_later_success();
	test_wan_post_script_failure_is_reported_and_later_steps_continue();
	test_wan_post_down_script_failure_is_reported_and_later_steps_continue();
	const int first_error[] = { 0, EIO, 0, ENETUNREACH };
	const int negative_error[] = { 0, -1, 0, EPERM };
	const int no_errors[] = { 0, 0, 0 };
	size_t attempts = 0;
	assert(apply_results(first_error, sizeof(first_error) / sizeof(first_error[0]), &attempts) == EIO);
	assert(attempts == sizeof(first_error) / sizeof(first_error[0]));
	attempts = 0;
	assert(apply_results(negative_error, sizeof(negative_error) / sizeof(negative_error[0]), &attempts) == -1);
	assert(attempts == sizeof(negative_error) / sizeof(negative_error[0]));
	attempts = 0;
	assert(apply_results(no_errors, sizeof(no_errors) / sizeof(no_errors[0]), &attempts) == 0);
	assert(attempts == sizeof(no_errors) / sizeof(no_errors[0]));
	assert(route_status_accumulate(EIO, 0) == EIO);
	assert(route_status_accumulate(EIO, EPERM) == EIO);
	assert(route_status_finish(0, 0) == 0);
	assert(route_status_finish(EIO, 0) == EIO);
	assert(route_status_finish(0, -1) == -1);
	assert(route_status_finish(EIO, -1) == -1);
	assert(route_status_finish(route_status_accumulate(EIO, ENETUNREACH), 0) == EIO);
	assert(route_status_idempotent_add(0) == 0);
	assert(route_status_idempotent_add(EEXIST) == 0);
	assert(route_status_idempotent_add(EIO) == EIO);
	assert(route_status_idempotent_add(ENETUNREACH) == ENETUNREACH);
	assert(route_status_accumulate(EIO, route_status_idempotent_add(EEXIST)) == EIO);
	assert(route_status_demand_ready(0) == 1);
	assert(route_status_demand_ready(EIO) == 0);
	assert(route_status_demand_ready(ENETUNREACH) == 0);
	assert(route_status_demand_ready(-1) == 0);
	assert(route_status_demand_ready(route_status_idempotent_add(EEXIST)) == 1);
	return 0;
}
