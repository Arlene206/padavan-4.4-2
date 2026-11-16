#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>

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

static int radio_status[2];
static int ipv6_status;
static int radio_calls;
static int ipv6_calls;
static const char *radio_ifnames[2];
static int radio_enabled[2];
static int ipv6_enabled;

static int
mock_set_radio_registration(const char *ifname, int enabled)
{
	int index = radio_calls++;
	assert(index < 2);
	radio_ifnames[index] = ifname;
	radio_enabled[index] = enabled;
	return radio_status[index];
}

static int
mock_set_ipv6_nat(int enabled)
{
	++ipv6_calls;
	ipv6_enabled = enabled;
	return ipv6_status;
}

static void
reset_radio_case(int radio_2g, int radio_5g, int ipv6)
{
	radio_status[0] = radio_2g;
	radio_status[1] = radio_5g;
	ipv6_status = ipv6;
	radio_calls = 0;
	ipv6_calls = 0;
	radio_ifnames[0] = NULL;
	radio_ifnames[1] = NULL;
	radio_enabled[0] = -1;
	radio_enabled[1] = -1;
	ipv6_enabled = -1;
}

static void
test_system_wait_status_is_normalized(void)
{
	assert(rc_hwnat_command_status_policy(0, 0) == 0);
	assert(rc_hwnat_command_status_policy(7 << 8, 0) == EIO);
	assert(rc_hwnat_command_status_policy(SIGTERM, 0) == EIO);
	assert(rc_hwnat_command_status_policy(-1, ENOENT) == ENOENT);
	assert(rc_hwnat_command_status_policy(-1, 0) == EIO);
}

static void
test_radio_registration_and_ipv6_settings_are_applied(void)
{
	reset_radio_case(0, 0, 0);
	assert(rc_hwnat_configure_radio_policy(1, 1, 1, 0,
		"radio2", "radio5", mock_set_radio_registration,
		mock_set_ipv6_nat) == 0);
	assert(radio_calls == 2);
	assert(strcmp(radio_ifnames[0], "radio2") == 0);
	assert(strcmp(radio_ifnames[1], "radio5") == 0);
	assert(radio_enabled[0] == 1 && radio_enabled[1] == 1);
	assert(ipv6_calls == 1 && ipv6_enabled == 0);

	reset_radio_case(0, 0, 0);
	assert(rc_hwnat_configure_radio_policy(1, 1, 0, 1,
		"radio2", "radio5", mock_set_radio_registration,
		mock_set_ipv6_nat) == 0);
	assert(radio_calls == 2);
	assert(radio_enabled[0] == 0 && radio_enabled[1] == 0);
	assert(ipv6_calls == 1 && ipv6_enabled == 1);
}

static void
test_radio_failures_do_not_skip_independent_writes(void)
{
	reset_radio_case(EACCES, EPERM, EIO);
	assert(rc_hwnat_configure_radio_policy(1, 1, 1, 1,
		"radio2", "radio5", mock_set_radio_registration,
		mock_set_ipv6_nat) == EACCES);
	assert(radio_calls == 2 && ipv6_calls == 1);
	assert(radio_enabled[0] == 1 && radio_enabled[1] == 1);
	assert(ipv6_enabled == 1);

	reset_radio_case(0, -1, 0);
	assert(rc_hwnat_configure_radio_policy(1, 1, 0, 0,
		"radio2", "radio5", mock_set_radio_registration,
		mock_set_ipv6_nat) == EIO);
	assert(radio_calls == 2 && ipv6_calls == 1);
}

static void
test_disabled_or_unloaded_radio_policy_has_no_side_effects(void)
{
	reset_radio_case(0, 0, 0);
	assert(rc_hwnat_configure_radio_policy(0, 1, 1, 1,
		"radio2", "radio5", mock_set_radio_registration,
		mock_set_ipv6_nat) == 0);
	assert(radio_calls == 0 && ipv6_calls == 0);
	assert(rc_hwnat_configure_radio_policy(1, 0, 1, 1,
		"radio2", "radio5", mock_set_radio_registration,
		mock_set_ipv6_nat) == 0);
	assert(radio_calls == 0 && ipv6_calls == 0);
}

static void
test_missing_radio_writer_fails_before_side_effects(void)
{
	reset_radio_case(0, 0, 0);
	assert(rc_hwnat_configure_radio_policy(1, 1, 1, 1,
		"radio2", "radio5", NULL, mock_set_ipv6_nat) == EINVAL);
	assert(radio_calls == 0 && ipv6_calls == 0);
}

int
main(void)
{
	test_disabled_or_unloaded_does_not_touch_driver();
	test_missing_wan_vid_is_reported();
	test_successful_wan_vid_write_is_enabled();
	test_wan_vid_write_failure_is_reported();
	test_missing_setter_fails_before_side_effects();
	test_system_wait_status_is_normalized();
	test_radio_registration_and_ipv6_settings_are_applied();
	test_radio_failures_do_not_skip_independent_writes();
	test_disabled_or_unloaded_radio_policy_has_no_side_effects();
	test_missing_radio_writer_fails_before_side_effects();
	puts("HW NAT configuration, command-status and radio policy: PASS");
	return 0;
}
