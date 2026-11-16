#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "wireless_module_load_policy.h"

static int module_loaded;
static int load_result;
static int make_module_visible;
static int load_calls;
static int probe_calls;
static const char *expected_module;
static const char *expected_param;

static int
mock_module_load(const char *module_name, const char *module_param)
{
	assert(module_name != NULL);
	assert(strcmp(module_name, expected_module) == 0);
	assert((module_param == NULL && expected_param == NULL) ||
	       (module_param != NULL && expected_param != NULL &&
		strcmp(module_param, expected_param) == 0));
	load_calls++;
	if (make_module_visible)
		module_loaded = 1;
	return load_result;
}

static int
mock_is_module_loaded(const char *module_name)
{
	assert(module_name != NULL);
	assert(strcmp(module_name, expected_module) == 0);
	probe_calls++;
	return module_loaded;
}

static const rc_wireless_module_load_ops_t mock_ops = {
	mock_module_load,
	mock_is_module_loaded
};

static void
reset_case(int loaded, int result, int visible)
{
	module_loaded = loaded;
	load_result = result;
	make_module_visible = visible;
	load_calls = 0;
	probe_calls = 0;
	expected_module = "mt_7615e";
	expected_param = NULL;
}

static void
test_already_loaded_is_success(void)
{
	reset_case(1, 0, 0);
	assert(rc_wireless_module_load_verified("mt_7615e", &mock_ops) == 0);
	assert(load_calls == 1);
	assert(probe_calls == 1);
}

static void
test_newly_loaded_is_success(void)
{
	reset_case(0, 1, 1);
	assert(rc_wireless_module_load_verified("mt_7615e", &mock_ops) == 0);
	assert(load_calls == 1);
	assert(probe_calls == 1);
	assert(module_loaded == 1);
}

static void
test_failed_modprobe_is_not_success(void)
{
	reset_case(0, 0, 0);
	assert(rc_wireless_module_load_verified("mt_7615e", &mock_ops) == EIO);
	assert(load_calls == 1);
	assert(probe_calls == 1);
	assert(module_loaded == 0);
}

static void
test_visible_module_state_wins_over_loader_status(void)
{
	reset_case(0, -1, 1);
	assert(rc_wireless_module_load_verified("mt_7615e", &mock_ops) == 0);
	assert(load_calls == 1);
	assert(probe_calls == 1);
	assert(module_loaded == 1);
}

static void
test_module_parameter_is_forwarded(void)
{
	reset_case(0, 1, 1);
	expected_module = "xhci_hcd";
	expected_param = "usb3_disable=1";
	assert(rc_module_load_verified("xhci_hcd", "usb3_disable=1",
				       (const rc_module_load_ops_t *)&mock_ops) == 0);
	assert(load_calls == 1);
	assert(probe_calls == 1);
	assert(module_loaded == 1);
}

static void
test_invalid_inputs_fail_before_side_effects(void)
{
	rc_wireless_module_load_ops_t no_loader = {
		NULL, mock_is_module_loaded
	};
	rc_wireless_module_load_ops_t no_probe = {
		mock_module_load, NULL
	};

	reset_case(0, 0, 0);
	assert(rc_wireless_module_load_verified(NULL, &mock_ops) == EINVAL);
	assert(rc_wireless_module_load_verified("", &mock_ops) == EINVAL);
	assert(rc_wireless_module_load_verified("mt_7615e", NULL) == EINVAL);
	assert(rc_wireless_module_load_verified("mt_7615e", &no_loader) == EINVAL);
	assert(rc_wireless_module_load_verified("mt_7615e", &no_probe) == EINVAL);
	assert(load_calls == 0);
	assert(probe_calls == 0);
}


static const char *required_expected_names[3];
static int required_expected_visible[3];
static int required_load_calls;
static int required_probe_calls;

static int
mock_required_module_load(const char *module_name, const char *module_param)
{
	assert(required_load_calls < 3);
	assert(module_name != NULL);
	assert(strcmp(module_name, required_expected_names[required_load_calls]) == 0);
	assert(module_param == NULL);
	++required_load_calls;
	return 0;
}

static int
mock_required_module_probe(const char *module_name)
{
	assert(required_probe_calls < required_load_calls);
	assert(module_name != NULL);
	assert(strcmp(module_name, required_expected_names[required_probe_calls]) == 0);
	return required_expected_visible[required_probe_calls++];
}

static const rc_module_load_ops_t required_mock_ops = {
	mock_required_module_load,
	mock_required_module_probe
};

static void
test_required_module_set_attempts_all_and_keeps_first_failure(void)
{
	const rc_module_load_requirement_t requirements[] = {
		{ "nf_conntrack_pptp", NULL, 1 },
		{ "nf_nat_pptp", NULL, 1 },
		{ "fast_classifier", NULL, 0 }
	};

	required_expected_names[0] = "nf_conntrack_pptp";
	required_expected_names[1] = "nf_nat_pptp";
	required_expected_names[2] = "fast_classifier";
	required_expected_visible[0] = 0;
	required_expected_visible[1] = 1;
	required_expected_visible[2] = 0;
	required_load_calls = 0;
	required_probe_calls = 0;

	assert(rc_module_load_requirements_apply(requirements, 3,
						 &required_mock_ops) == EIO);
	assert(required_load_calls == 2);
	assert(required_probe_calls == 2);
}

static void
test_required_module_set_success_and_invalid_inputs(void)
{
	const rc_module_load_requirement_t requirements[] = {
		{ "nf_conntrack_sip", NULL, 1 },
		{ "nf_nat_sip", NULL, 1 }
	};

	required_expected_names[0] = "nf_conntrack_sip";
	required_expected_names[1] = "nf_nat_sip";
	required_expected_visible[0] = 1;
	required_expected_visible[1] = 1;
	required_load_calls = 0;
	required_probe_calls = 0;
	assert(rc_module_load_requirements_apply(requirements, 2,
						 &required_mock_ops) == 0);
	assert(required_load_calls == 2);
	assert(required_probe_calls == 2);
	assert(rc_module_load_requirements_apply(NULL, 1,
						 &required_mock_ops) == EINVAL);
	assert(rc_module_load_requirements_apply(requirements, 2, NULL) == EINVAL);
	assert(rc_module_load_requirements_apply(NULL, 0, &required_mock_ops) == 0);
	assert(required_load_calls == 2);
	assert(required_probe_calls == 2);
}

int
main(void)
{
	test_already_loaded_is_success();
	test_newly_loaded_is_success();
	test_failed_modprobe_is_not_success();
	test_visible_module_state_wins_over_loader_status();
	test_module_parameter_is_forwarded();
	test_invalid_inputs_fail_before_side_effects();
	test_required_module_set_attempts_all_and_keeps_first_failure();
	test_required_module_set_success_and_invalid_inputs();
	puts("RC module post-load verification and required-module set policy: PASS");
	return 0;
}
