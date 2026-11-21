#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "wan6_identity_policy.h"

struct fake_identity_backend {
	int set_status[4];
	int set_calls;
	int stop_status;
	int stop_calls;
	int unit[4];
	char names[4][16];
	char values[4][32];
};

static int
fake_set_value(void *opaque, int unit, const char *param_name,
		const char *value)
{
	struct fake_identity_backend *fake = (struct fake_identity_backend *)opaque;
	int call = fake->set_calls++;

	assert(call >= 0 && call < 4);
	fake->unit[call] = unit;
	snprintf(fake->names[call], sizeof(fake->names[call]), "%s", param_name);
	snprintf(fake->values[call], sizeof(fake->values[call]), "%s", value);
	return fake->set_status[call];
}

static int
fake_stop_sit(void *opaque)
{
	struct fake_identity_backend *fake = (struct fake_identity_backend *)opaque;

	++fake->stop_calls;
	return fake->stop_status;
}

static void
test_sit_identity_commit_success(void)
{
	struct fake_identity_backend fake;
	int rollback_status = -1;

	memset(&fake, 0, sizeof(fake));
	assert(wan6_publish_sit_identity(fake_set_value, fake_stop_sit,
		&fake, 2, "sit0", &rollback_status) == 0);
	assert(rollback_status == 0);
	assert(fake.set_calls == 1 && fake.stop_calls == 0);
	assert(fake.unit[0] == 2);
	assert(strcmp(fake.names[0], "ifname6") == 0);
	assert(strcmp(fake.values[0], "sit0") == 0);
}

static void
test_sit_identity_failure_removes_tunnel_and_clears_partial_identity(void)
{
	struct fake_identity_backend fake;
	int rollback_status = 0;

	memset(&fake, 0, sizeof(fake));
	fake.set_status[0] = EIO;
	assert(wan6_publish_sit_identity(fake_set_value, fake_stop_sit,
		&fake, 1, "sit0", &rollback_status) == EIO);
	assert(rollback_status == 0);
	assert(fake.set_calls == 2 && fake.stop_calls == 1);
	assert(strcmp(fake.names[1], "ifname6") == 0);
	assert(strcmp(fake.values[1], "") == 0);
}

static void
test_sit_identity_failure_retains_state_when_tunnel_removal_fails(void)
{
	struct fake_identity_backend fake;
	int rollback_status = 0;

	memset(&fake, 0, sizeof(fake));
	fake.set_status[0] = ENOSPC;
	fake.stop_status = EBUSY;
	assert(wan6_publish_sit_identity(fake_set_value, fake_stop_sit,
		&fake, 1, "sit0", &rollback_status) == ENOSPC);
	assert(rollback_status == EBUSY);
	assert(fake.set_calls == 1 && fake.stop_calls == 1);
}

static void
test_sit_identity_failure_reports_metadata_clear_failure(void)
{
	struct fake_identity_backend fake;
	int rollback_status = 0;

	memset(&fake, 0, sizeof(fake));
	fake.set_status[0] = EIO;
	fake.set_status[1] = EPERM;
	assert(wan6_publish_sit_identity(fake_set_value, fake_stop_sit,
		&fake, 1, "sit0", &rollback_status) == EIO);
	assert(rollback_status == EPERM);
	assert(fake.set_calls == 2 && fake.stop_calls == 1);
}

static void
test_native_identity_is_retained_when_kernel_rollback_incomplete(void)
{
	struct fake_identity_backend fake;

	memset(&fake, 0, sizeof(fake));
	assert(wan6_clear_native_identity_after_rollback(
		fake_set_value, &fake, 2, EBUSY) == EBUSY);
	assert(fake.set_calls == 0);
}

static void
test_native_identity_is_cleared_after_complete_kernel_rollback(void)
{
	struct fake_identity_backend fake;

	memset(&fake, 0, sizeof(fake));
	assert(wan6_clear_native_identity_after_rollback(
		fake_set_value, &fake, 1, 0) == 0);
	assert(fake.set_calls == 1);
	assert(fake.unit[0] == 1);
	assert(strcmp(fake.names[0], "ifname6") == 0);
	assert(strcmp(fake.values[0], "") == 0);
}

static void
test_native_identity_clear_failure_is_reported(void)
{
	struct fake_identity_backend fake;

	memset(&fake, 0, sizeof(fake));
	fake.set_status[0] = ENOSPC;
	assert(wan6_clear_native_identity_after_rollback(
		fake_set_value, &fake, 1, 0) == ENOSPC);
	assert(fake.set_calls == 1);
	assert(strcmp(fake.names[0], "ifname6") == 0);
}

static void
test_metadata_clear_attempts_both_fields_and_preserves_first_error(void)
{
	struct fake_identity_backend fake;

	memset(&fake, 0, sizeof(fake));
	fake.set_status[0] = EACCES;
	assert(wan6_clear_runtime_metadata(fake_set_value, &fake, 3, 1) == EACCES);
	assert(fake.set_calls == 2);
	assert(strcmp(fake.names[0], "dns6") == 0);
	assert(strcmp(fake.values[0], "") == 0);
	assert(strcmp(fake.names[1], "ifname6") == 0);
	assert(strcmp(fake.values[1], "") == 0);
}

static void
test_metadata_clear_can_preserve_dns_when_requested(void)
{
	struct fake_identity_backend fake;

	memset(&fake, 0, sizeof(fake));
	assert(wan6_clear_runtime_metadata(fake_set_value, &fake, 0, 0) == 0);
	assert(fake.set_calls == 1);
	assert(strcmp(fake.names[0], "ifname6") == 0);
}

static void
test_metadata_clear_rejects_invalid_arguments_before_side_effects(void)
{
	struct fake_identity_backend fake;

	memset(&fake, 0, sizeof(fake));
	assert(wan6_clear_runtime_metadata(NULL, &fake, 0, 1) == EINVAL);
	assert(wan6_clear_runtime_metadata(fake_set_value, &fake, -1, 1) == EINVAL);
	assert(wan6_publish_sit_identity(NULL, fake_stop_sit, &fake,
		0, "sit0", NULL) == EINVAL);
	assert(wan6_publish_sit_identity(fake_set_value, NULL, &fake,
		0, "sit0", NULL) == EINVAL);
	assert(wan6_publish_sit_identity(fake_set_value, fake_stop_sit, &fake,
		0, "", NULL) == EINVAL);
	assert(fake.set_calls == 0 && fake.stop_calls == 0);
}

int
main(void)
{
	test_sit_identity_commit_success();
	test_sit_identity_failure_removes_tunnel_and_clears_partial_identity();
	test_sit_identity_failure_retains_state_when_tunnel_removal_fails();
	test_sit_identity_failure_reports_metadata_clear_failure();
	test_native_identity_is_retained_when_kernel_rollback_incomplete();
	test_native_identity_is_cleared_after_complete_kernel_rollback();
	test_native_identity_clear_failure_is_reported();
	test_metadata_clear_attempts_both_fields_and_preserves_first_error();
	test_metadata_clear_can_preserve_dns_when_requested();
	test_metadata_clear_rejects_invalid_arguments_before_side_effects();
	puts("WAN IPv6 identity publication policy: PASS");
	return 0;
}
