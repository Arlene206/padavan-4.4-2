#include <assert.h>
#include <errno.h>
#include <stddef.h>

#include "libc_gai_policy.h"

struct fake_operations {
	int write_calls;
	int unlink_calls;
	int write_status;
	int unlink_status;
};

static int
fake_write(void *context)
{
	struct fake_operations *operations = (struct fake_operations *)context;
	operations->write_calls++;
	return operations->write_status;
}

static int
fake_unlink(void *context)
{
	struct fake_operations *operations = (struct fake_operations *)context;
	operations->unlink_calls++;
	return operations->unlink_status;
}

static void
test_enable_writes_and_propagates_write_failure(void)
{
	struct fake_operations operations = { 0, 0, EACCES, 0 };

	assert(rc_libc_gai_apply(1, fake_write, fake_unlink, &operations) == EACCES);
	assert(operations.write_calls == 1);
	assert(operations.unlink_calls == 0);
}

static void
test_disable_treats_missing_override_as_success(void)
{
	struct fake_operations operations = { 0, 0, 0, ENOENT };

	assert(rc_libc_gai_apply(0, fake_write, fake_unlink, &operations) == 0);
	assert(operations.write_calls == 0);
	assert(operations.unlink_calls == 1);
}

static void
test_disable_propagates_other_unlink_failures(void)
{
	struct fake_operations operations = { 0, 0, 0, EACCES };

	assert(rc_libc_gai_apply(0, fake_write, fake_unlink, &operations) == EACCES);
	assert(operations.write_calls == 0);
	assert(operations.unlink_calls == 1);
}

static void
test_missing_required_operation_is_rejected(void)
{
	assert(rc_libc_gai_apply(1, NULL, fake_unlink, NULL) == EINVAL);
	assert(rc_libc_gai_apply(0, fake_write, NULL, NULL) == EINVAL);
}

int
main(void)
{
	test_enable_writes_and_propagates_write_failure();
	test_disable_treats_missing_override_as_success();
	test_disable_propagates_other_unlink_failures();
	test_missing_required_operation_is_rejected();
	return 0;
}
