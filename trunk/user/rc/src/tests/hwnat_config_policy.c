#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "hwnat_config_policy.h"

static int setter_status;
static int setter_calls;
static int last_wan_vid;

static int
mock_set_wan_vid(int wan_vid)
{
	++setter_calls;
	last_wan_vid = wan_vid;
	return setter_status;
}

static void
reset_case(int status)
{
	setter_status = status;
	setter_calls = 0;
	last_wan_vid = -1;
}

static void
test_disabled_or_unloaded_does_not_touch_driver(void)
{
	reset_case(0);
	assert(rc_hwnat_configure_policy(0, 1, 35, mock_set_wan_vid) == 0);
	assert(setter_calls == 0);

	assert(rc_hwnat_configure_policy(1, 0, 35, mock_set_wan_vid) == 0);
	assert(setter_calls == 0);
}

static void
test_missing_wan_vid_is_reported(void)
{
	reset_case(0);
	assert(rc_hwnat_configure_policy(1, 1, -1, mock_set_wan_vid) == EIO);
	assert(setter_calls == 0);

	/* A failed VLAN lookup is reported even when no HW NAT module was loaded. */
	assert(rc_hwnat_configure_policy(1, 0, -1, mock_set_wan_vid) == EIO);
	assert(setter_calls == 0);
}

static void
test_successful_wan_vid_write_is_enabled(void)
{
	reset_case(0);
	assert(rc_hwnat_configure_policy(1, 1, 35, mock_set_wan_vid) == 0);
	assert(setter_calls == 1);
	assert(last_wan_vid == 35);
}

static void
test_wan_vid_write_failure_is_reported(void)
{
	reset_case(EACCES);
	assert(rc_hwnat_configure_policy(1, 1, 35, mock_set_wan_vid) == EACCES);
	assert(setter_calls == 1);
	assert(last_wan_vid == 35);

	reset_case(-1);
	assert(rc_hwnat_configure_policy(1, 1, 35, mock_set_wan_vid) == EIO);
	assert(setter_calls == 1);
}

static void
test_missing_setter_fails_before_side_effects(void)
{
	reset_case(0);
	assert(rc_hwnat_configure_policy(1, 1, 35, NULL) == EINVAL);
	assert(setter_calls == 0);
}

int
main(void)
{
	test_disabled_or_unloaded_does_not_touch_driver();
	test_missing_wan_vid_is_reported();
	test_successful_wan_vid_write_is_enabled();
	test_wan_vid_write_failure_is_reported();
	test_missing_setter_fails_before_side_effects();
	puts("HW NAT WAN VID configuration status policy: PASS");
	return 0;
}
