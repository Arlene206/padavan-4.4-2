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

int
main(void)
{
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
	return 0;
}
