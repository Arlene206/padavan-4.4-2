#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "udhcpc_script_link_policy.h"

static int create_status;
static int create_errno;
static int create_calls;
static int read_status;
static int read_errno;
static int read_calls;
static const char *read_target;
static const char *expected_path;
static const char *expected_create_target;

static int
mock_create(const char *target, const char *link_path)
{
	create_calls++;
	assert(expected_create_target != NULL);
	assert(expected_path != NULL);
	assert(strcmp(target, expected_create_target) == 0);
	assert(strcmp(link_path, expected_path) == 0);
	if (create_status == 0)
		return 0;
	errno = create_errno;
	return -1;
}

static ssize_t
mock_read(const char *link_path, char *buffer, size_t buffer_size)
{
	size_t length;

	read_calls++;
	assert(expected_path != NULL);
	assert(strcmp(link_path, expected_path) == 0);
	if (read_status < 0) {
		errno = read_errno;
		return -1;
	}
	assert(read_target != NULL);
	length = strlen(read_target);
	assert(length <= buffer_size);
	memcpy(buffer, read_target, length);
	return (ssize_t)length;
}

static const rc_udhcpc_script_link_ops_t mock_ops = {
	mock_create,
	mock_read
};

static void
reset_case(void)
{
	create_status = 0;
	create_errno = 0;
	create_calls = 0;
	read_status = 0;
	read_errno = 0;
	read_calls = 0;
	read_target = "/sbin/rc";
	expected_path = "/tmp/udhcpc_lan.script";
	expected_create_target = "/sbin/rc";
}

static void
test_new_link_is_created_without_readback(void)
{
	reset_case();
	assert(rc_udhcpc_script_link_ensure(expected_path,
					    expected_create_target,
					    &mock_ops) == 0);
	assert(create_calls == 1);
	assert(read_calls == 0);
}

static void
test_existing_exact_link_is_idempotent(void)
{
	reset_case();
	create_status = -1;
	create_errno = EEXIST;
	assert(rc_udhcpc_script_link_ensure(expected_path,
					    expected_create_target,
					    &mock_ops) == 0);
	assert(create_calls == 1);
	assert(read_calls == 1);
}

static void
test_existing_wrong_link_is_not_replaced(void)
{
	reset_case();
	create_status = -1;
	create_errno = EEXIST;
	read_target = "/tmp/attacker-controlled-script";
	assert(rc_udhcpc_script_link_ensure(expected_path,
					    expected_create_target,
					    &mock_ops) == EEXIST);
	assert(create_calls == 1);
	assert(read_calls == 1);
}

static void
test_existing_non_symlink_is_rejected(void)
{
	reset_case();
	create_status = -1;
	create_errno = EEXIST;
	read_status = -1;
	read_errno = EINVAL;
	assert(rc_udhcpc_script_link_ensure(expected_path,
					    expected_create_target,
					    &mock_ops) == EINVAL);
	assert(read_calls == 1);
}

static void
test_creation_error_does_not_attempt_readback(void)
{
	reset_case();
	create_status = -1;
	create_errno = EACCES;
	assert(rc_udhcpc_script_link_ensure(expected_path,
					    expected_create_target,
					    &mock_ops) == EACCES);
	assert(read_calls == 0);
}

static void
test_readback_error_is_propagated(void)
{
	reset_case();
	create_status = -1;
	create_errno = EEXIST;
	read_status = -1;
	read_errno = EIO;
	assert(rc_udhcpc_script_link_ensure(expected_path,
					    expected_create_target,
					    &mock_ops) == EIO);
}

static void
test_invalid_arguments_fail_before_side_effects(void)
{
	reset_case();
	assert(rc_udhcpc_script_link_ensure(NULL, expected_create_target,
					    &mock_ops) == EINVAL);
	assert(rc_udhcpc_script_link_ensure(expected_path, NULL,
					    &mock_ops) == EINVAL);
	assert(rc_udhcpc_script_link_ensure(expected_path,
					    expected_create_target, NULL) == EINVAL);
	assert(create_calls == 0);
	assert(read_calls == 0);
}

static void
test_overlong_target_fails_before_side_effects(void)
{
	char target[RC_UDHCPC_SCRIPT_LINK_TARGET_MAX + 1];

	memset(target, 'x', sizeof(target) - 1);
	target[sizeof(target) - 1] = '\0';
	reset_case();
	assert(rc_udhcpc_script_link_ensure(expected_path, target,
					    &mock_ops) == ENAMETOOLONG);
	assert(create_calls == 0);
	assert(read_calls == 0);
}

int
main(void)
{
	test_new_link_is_created_without_readback();
	test_existing_exact_link_is_idempotent();
	test_existing_wrong_link_is_not_replaced();
	test_existing_non_symlink_is_rejected();
	test_creation_error_does_not_attempt_readback();
	test_readback_error_is_propagated();
	test_invalid_arguments_fail_before_side_effects();
	test_overlong_target_fails_before_side_effects();
	puts("LAN udhcpc event-handler symlink safety and idempotency: PASS");
	return 0;
}
