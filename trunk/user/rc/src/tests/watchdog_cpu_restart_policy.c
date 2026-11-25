#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include "watchdog_cpu_restart_policy.h"

static int module_visible;
static int load_result;
static int unload_result;
static int make_visible_on_load;
static int remove_on_unload;
static int load_calls;
static int unload_calls;
static int probe_calls;
static int probe_override;

static int
mock_load(const char *module_name, const char *module_param)
{
	assert(module_name != NULL);
	assert(strcmp(module_name, "rt_timer_wdg") == 0);
	assert(module_param == NULL);
	load_calls++;
	if (make_visible_on_load)
		module_visible = 1;
	return load_result;
}

static int
mock_unload(const char *module_name, int recurse_unload)
{
	assert(module_name != NULL);
	assert(strcmp(module_name, "rt_timer_wdg") == 0);
	assert(recurse_unload == 0);
	unload_calls++;
	if (remove_on_unload)
		module_visible = 0;
	return unload_result;
}

static int
mock_is_loaded(const char *module_name)
{
	assert(module_name != NULL);
	assert(strcmp(module_name, "rt_timer_wdg") == 0);
	probe_calls++;
	return probe_override == INT_MAX ? module_visible : probe_override;
}

static const rc_watchdog_cpu_restart_ops_t mock_ops = {
	mock_load,
	mock_unload,
	mock_is_loaded
};

static void
reset_case(int visible, int load_rc, int unload_rc,
	  int load_visible, int unload_absent)
{
	module_visible = visible;
	load_result = load_rc;
	unload_result = unload_rc;
	make_visible_on_load = load_visible;
	remove_on_unload = unload_absent;
	load_calls = 0;
	unload_calls = 0;
	probe_calls = 0;
	probe_override = INT_MAX;
}

static void
test_config_absent_default_disables_module(void)
{
	int should_load = -1;
	assert(rc_watchdog_cpu_restart_config_status(0, 0, &should_load) == 0);
	assert(should_load == 0);
}

static void
test_config_nonzero_enables_module(void)
{
	int should_load = 0;
	assert(rc_watchdog_cpu_restart_config_status(0, 1, &should_load) == 0);
	assert(should_load == 1);
	should_load = 0;
	assert(rc_watchdog_cpu_restart_config_status(0, -1, &should_load) == 0);
	assert(should_load == 1);
}

static void
test_config_read_failure_preserves_output_and_blocks_module_action(void)
{
	int should_load = 17;
	assert(rc_watchdog_cpu_restart_config_status(EIO, 0, &should_load) == EIO);
	assert(should_load == 17);
	assert(rc_watchdog_cpu_restart_config_status(EACCES, 1, &should_load) == EACCES);
	assert(should_load == 17);
	assert(rc_watchdog_cpu_restart_config_status(EINVAL, 0, &should_load) == EINVAL);
	assert(should_load == 17);
	assert(rc_watchdog_cpu_restart_config_status(0, 1, NULL) == EINVAL);
}

static void
test_enabled_module_already_loaded_is_success(void)
{
	reset_case(1, 0, 0, 0, 0);
	assert(rc_watchdog_cpu_restart_verified("rt_timer_wdg", 1, &mock_ops) == 0);
	assert(load_calls == 1 && unload_calls == 0 && probe_calls == 1);
}

static void
test_enabled_new_module_is_verified(void)
{
	reset_case(0, 1, 0, 1, 0);
	assert(rc_watchdog_cpu_restart_verified("rt_timer_wdg", 1, &mock_ops) == 0);
	assert(load_calls == 1 && unload_calls == 0 && probe_calls == 1);
	assert(module_visible == 1);
}

static void
test_failed_load_is_not_success(void)
{
	reset_case(0, 0, 0, 0, 0);
	assert(rc_watchdog_cpu_restart_verified("rt_timer_wdg", 1, &mock_ops) == EIO);
	assert(load_calls == 1 && unload_calls == 0 && probe_calls == 1);
	assert(module_visible == 0);
}

static void
test_visible_module_state_wins_over_loader_status(void)
{
	reset_case(0, 0, 0, 1, 0);
	assert(rc_watchdog_cpu_restart_verified("rt_timer_wdg", 1, &mock_ops) == 0);
	assert(load_calls == 1 && probe_calls == 1 && module_visible == 1);
}

static void
test_disabled_module_unload_is_verified(void)
{
	reset_case(1, 0, 1, 0, 1);
	assert(rc_watchdog_cpu_restart_verified("rt_timer_wdg", 0, &mock_ops) == 0);
	assert(load_calls == 0 && unload_calls == 1 && probe_calls == 1);
	assert(module_visible == 0);
}

static void
test_disabled_already_absent_is_success(void)
{
	reset_case(0, 0, 0, 0, 0);
	assert(rc_watchdog_cpu_restart_verified("rt_timer_wdg", 0, &mock_ops) == 0);
	assert(load_calls == 0 && unload_calls == 1 && probe_calls == 1);
}

static void
test_busy_module_prevents_false_disable_success(void)
{
	reset_case(1, 0, 1, 0, 0);
	assert(rc_watchdog_cpu_restart_verified("rt_timer_wdg", 0, &mock_ops) == EBUSY);
	assert(load_calls == 0 && unload_calls == 1 && probe_calls == 1);
	assert(module_visible == 1);
}

static void
test_absent_postcondition_wins_over_unloader_status(void)
{
	reset_case(1, 0, 0, 0, 1);
	assert(rc_watchdog_cpu_restart_verified("rt_timer_wdg", 0, &mock_ops) == 0);
	assert(unload_calls == 1 && probe_calls == 1 && module_visible == 0);
}

static void
test_unknown_module_state_is_not_treated_as_absent(void)
{
	reset_case(0, 0, 0, 1, 0);
	probe_override = -EACCES;
	assert(rc_watchdog_cpu_restart_verified("rt_timer_wdg", 1, &mock_ops) == EACCES);
	assert(load_calls == 1 && unload_calls == 0 && probe_calls == 1);

	reset_case(1, 0, 0, 0, 1);
	probe_override = -EACCES;
	assert(rc_watchdog_cpu_restart_verified("rt_timer_wdg", 0, &mock_ops) == EACCES);
	assert(load_calls == 0 && unload_calls == 1 && probe_calls == 1);
}

static void
test_invalid_probe_state_fails_closed(void)
{
	reset_case(1, 0, 0, 0, 0);
	probe_override = 2;
	assert(rc_watchdog_cpu_restart_verified("rt_timer_wdg", 1, &mock_ops) == EIO);
	assert(load_calls == 1 && probe_calls == 1);
}

static void
test_invalid_inputs_fail_before_side_effects(void)
{
	rc_watchdog_cpu_restart_ops_t no_load = {
		NULL, mock_unload, mock_is_loaded
	};
	rc_watchdog_cpu_restart_ops_t no_unload = {
		mock_load, NULL, mock_is_loaded
	};
	rc_watchdog_cpu_restart_ops_t no_probe = {
		mock_load, mock_unload, NULL
	};

	reset_case(0, 0, 0, 0, 0);
	assert(rc_watchdog_cpu_restart_verified(NULL, 1, &mock_ops) == EINVAL);
	assert(rc_watchdog_cpu_restart_verified("", 1, &mock_ops) == EINVAL);
	assert(rc_watchdog_cpu_restart_verified("rt_timer_wdg", 1, NULL) == EINVAL);
	assert(rc_watchdog_cpu_restart_verified("rt_timer_wdg", 1, &no_load) == EINVAL);
	assert(rc_watchdog_cpu_restart_verified("rt_timer_wdg", 0, &no_unload) == EINVAL);
	assert(rc_watchdog_cpu_restart_verified("rt_timer_wdg", 1, &no_probe) == EINVAL);
	assert(load_calls == 0 && unload_calls == 0 && probe_calls == 0);
}

int
main(void)
{
	test_config_absent_default_disables_module();
	test_config_nonzero_enables_module();
	test_config_read_failure_preserves_output_and_blocks_module_action();
	test_enabled_module_already_loaded_is_success();
	test_enabled_new_module_is_verified();
	test_failed_load_is_not_success();
	test_visible_module_state_wins_over_loader_status();
	test_disabled_module_unload_is_verified();
	test_disabled_already_absent_is_success();
	test_busy_module_prevents_false_disable_success();
	test_absent_postcondition_wins_over_unloader_status();
	test_unknown_module_state_is_not_treated_as_absent();
	test_invalid_probe_state_fails_closed();
	test_invalid_inputs_fail_before_side_effects();
	puts("RC watchdog module restart postcondition policy: PASS");
	return 0;
}
