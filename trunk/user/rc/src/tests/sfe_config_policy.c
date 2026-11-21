#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "sfe_config_policy.h"

static int write_status;
static int write_calls;
static int last_skip_value;

static int
mock_set_skip_to_bridge_ingress(int value)
{
	++write_calls;
	last_skip_value = value;
	return write_status;
}

static void
reset_case(int status)
{
	write_status = status;
	write_calls = 0;
	last_skip_value = -1;
}

static void
test_unconfigured_mode_does_not_write(void)
{
	reset_case(0);
	assert(rc_sfe_configure_policy(0, mock_set_skip_to_bridge_ingress) == 0);
	assert(rc_sfe_configure_policy(3, mock_set_skip_to_bridge_ingress) == 0);
	assert(write_calls == 0);
}

static void
test_mode_one_disables_skip_to_bridge_ingress(void)
{
	reset_case(0);
	assert(rc_sfe_configure_policy(1, mock_set_skip_to_bridge_ingress) == 0);
	assert(write_calls == 1);
	assert(last_skip_value == 0);
}

static void
test_mode_two_enables_skip_to_bridge_ingress(void)
{
	reset_case(0);
	assert(rc_sfe_configure_policy(2, mock_set_skip_to_bridge_ingress) == 0);
	assert(write_calls == 1);
	assert(last_skip_value == 1);
}

static void
test_failed_sfe_config_write_is_reported(void)
{
	reset_case(EACCES);
	assert(rc_sfe_configure_policy(1, mock_set_skip_to_bridge_ingress) == EACCES);
	assert(write_calls == 1);

	reset_case(-1);
	assert(rc_sfe_configure_policy(2, mock_set_skip_to_bridge_ingress) == EIO);
	assert(write_calls == 1);
}

static void
test_missing_writer_fails_before_side_effects(void)
{
	reset_case(0);
	assert(rc_sfe_configure_policy(1, NULL) == EINVAL);
	assert(write_calls == 0);
}

int
main(void)
{
	test_unconfigured_mode_does_not_write();
	test_mode_one_disables_skip_to_bridge_ingress();
	test_mode_two_enables_skip_to_bridge_ingress();
	test_failed_sfe_config_write_is_reported();
	test_missing_writer_fails_before_side_effects();
	puts("SFE sysfs configuration status policy: PASS");
	return 0;
}
