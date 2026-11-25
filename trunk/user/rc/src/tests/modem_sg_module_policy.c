#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "modem_sg_module_policy.h"

static void
test_loaded_module_is_ready(void)
{
	assert(modem_sg_module_ready_status(1) == 0);
}

static void
test_absent_module_is_not_ready(void)
{
	assert(modem_sg_module_ready_status(0) == -1);
}

static void
test_zerocd_success_is_accepted(void)
{
	assert(modem_sg_zerocd_command_status(0) == 0);
}

static void
test_zerocd_failure_is_reported(void)
{
	assert(modem_sg_zerocd_command_status(1) == -1);
	assert(modem_sg_zerocd_command_status(127) == -1);
	assert(modem_sg_zerocd_command_status(-1) == -1);
}

static void
test_unknown_module_state_fails_closed(void)
{
	assert(modem_sg_module_ready_status(-EIO) == -1);
}

static void
test_missing_zero_cd_setting_preserves_legacy_default(void)
{
	int use_sr_mod = 1;

	assert(modem_sg_zero_cd_mode_status(0, 0, NULL, &use_sr_mod) == 0);
	assert(use_sr_mod == 0);
}

static void
test_present_zero_cd_setting_selects_configured_mode(void)
{
	int use_sr_mod = -1;

	assert(modem_sg_zero_cd_mode_status(1, 0, "1", &use_sr_mod) == 0);
	assert(use_sr_mod == 1);
	assert(modem_sg_zero_cd_mode_status(1, 0, "0", &use_sr_mod) == 0);
	assert(use_sr_mod == 0);
	assert(modem_sg_zero_cd_mode_status(1, 0, "  -2  ", &use_sr_mod) == 0);
	assert(use_sr_mod == 1);
}

static void
test_zero_cd_backend_read_failure_fails_closed(void)
{
	int use_sr_mod = 1;

	assert(modem_sg_zero_cd_mode_status(-1, EIO, NULL, &use_sr_mod) == EIO);
	assert(use_sr_mod == 0);
	assert(modem_sg_zero_cd_mode_status(-1, 0, NULL, &use_sr_mod) == EIO);
	assert(use_sr_mod == 0);
}

static void
test_zero_cd_malformed_value_fails_closed(void)
{
	int use_sr_mod = 1;

	assert(modem_sg_zero_cd_mode_status(1, 0, "", &use_sr_mod) == 0);
	assert(use_sr_mod == 0);
	assert(modem_sg_zero_cd_mode_status(1, 0, "   ", &use_sr_mod) == 0);
	assert(use_sr_mod == 0);
	assert(modem_sg_zero_cd_mode_status(1, 0, "not-a-number", &use_sr_mod) == EINVAL);
	assert(use_sr_mod == 0);
	assert(modem_sg_zero_cd_mode_status(1, 0, "1junk", &use_sr_mod) == EINVAL);
	assert(use_sr_mod == 0);
	assert(modem_sg_zero_cd_mode_status(
		1, 0, "999999999999999999999999999999999", &use_sr_mod) == ERANGE);
	assert(use_sr_mod == 0);
	assert(modem_sg_zero_cd_mode_status(1, 0, NULL, &use_sr_mod) == EIO);
	assert(use_sr_mod == 0);
	assert(modem_sg_zero_cd_mode_status(2, 0, "1", &use_sr_mod) == EIO);
	assert(use_sr_mod == 0);
	assert(modem_sg_zero_cd_mode_status(1, 0, "1", NULL) == EINVAL);
}

int
main(void)
{
	test_loaded_module_is_ready();
	test_absent_module_is_not_ready();
	test_unknown_module_state_fails_closed();
	test_zerocd_success_is_accepted();
	test_zerocd_failure_is_reported();
	test_missing_zero_cd_setting_preserves_legacy_default();
	test_present_zero_cd_setting_selects_configured_mode();
	test_zero_cd_backend_read_failure_fails_closed();
	test_zero_cd_malformed_value_fails_closed();

	puts("USB SCSI generic/ZeroCD command status policy: PASS");
	return 0;
}
