#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "wireless_module_load_policy.h"

static int module_loaded;
static int module_probe_result;
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
	if (make_module_visible) {
		module_loaded = 1;
		module_probe_result = 1;
	}
	return load_result;
}

static int
mock_is_module_loaded(const char *module_name)
{
	assert(module_name != NULL);
	assert(strcmp(module_name, expected_module) == 0);
	probe_calls++;
	return module_probe_result;
}

static const rc_wireless_module_load_ops_t mock_ops = {
	mock_module_load,
	mock_is_module_loaded
};

static void
reset_case(int loaded, int result, int visible)
{
	module_loaded = loaded;
	module_probe_result = loaded;
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
test_load_probe_error_is_reported(void)
{
	reset_case(0, 0, 0);
	module_probe_result = -EACCES;
	assert(rc_wireless_module_load_verified("mt_7615e", &mock_ops) == EACCES);
	assert(load_calls == 1);
	assert(probe_calls == 1);
	assert(module_loaded == 0);
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
test_required_module_set_probe_error_attempts_all_and_keeps_first_failure(void)
{
	const rc_module_load_requirement_t requirements[] = {
		{ "nf_conntrack_pptp", NULL, 1 },
		{ "nf_nat_pptp", NULL, 1 }
	};

	required_expected_names[0] = "nf_conntrack_pptp";
	required_expected_names[1] = "nf_nat_pptp";
	required_expected_visible[0] = -EPERM;
	required_expected_visible[1] = 1;
	required_load_calls = 0;
	required_probe_calls = 0;

	assert(rc_module_load_requirements_apply(requirements, 2,
						 &required_mock_ops) == EPERM);
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

#define UNLOAD_TEST_MODULES 3

static const char *unload_expected_names[UNLOAD_TEST_MODULES];
static int unload_module_loaded[UNLOAD_TEST_MODULES];
static int unload_remove_module[UNLOAD_TEST_MODULES];
static int unload_command_results[UNLOAD_TEST_MODULES];
static int unload_expected_count;
static int unload_calls;
static int unload_probe_calls;
static int unload_probe_error_call;
static int unload_probe_error;

static int
mock_module_unload(const char *module_name, int recurse_unload)
{
	int i;

	assert(recurse_unload == 0 || recurse_unload == 1);
	for (i = 0; i < unload_expected_count; ++i) {
		if (strcmp(module_name, unload_expected_names[i]) == 0)
			break;
	}
	assert(i < unload_expected_count);
	++unload_calls;
	if (unload_remove_module[i])
		unload_module_loaded[i] = 0;
	return unload_command_results[i];
}

static int
mock_module_unload_probe(const char *module_name)
{
	int i;

	for (i = 0; i < unload_expected_count; ++i) {
		if (strcmp(module_name, unload_expected_names[i]) == 0)
			break;
	}
	assert(i < unload_expected_count);
	++unload_probe_calls;
	if (unload_probe_error_call == unload_probe_calls)
		return -unload_probe_error;
	return unload_module_loaded[i];
}

static const rc_module_unload_ops_t mock_unload_ops = {
	mock_module_unload,
	mock_module_unload_probe
};

static void
reset_unload_case(const char *name, int loaded, int command_result,
		  int remove_module)
{
	unload_expected_names[0] = name;
	unload_module_loaded[0] = loaded;
	unload_command_results[0] = command_result;
	unload_remove_module[0] = remove_module;
	unload_expected_count = 1;
	unload_calls = 0;
	unload_probe_calls = 0;
	unload_probe_error_call = 0;
	unload_probe_error = 0;
}

static void
test_unload_absent_module_is_noop(void)
{
	reset_unload_case("nf_nat_pptp", 0, 0, 0);
	assert(rc_module_unload_verified("nf_nat_pptp", 1,
					 &mock_unload_ops) == 0);
	assert(unload_calls == 0);
	assert(unload_probe_calls == 1);
}

static void
test_unload_postcondition_wins_over_command_result(void)
{
	/* A nonzero command result is harmless if another actor removed it. */
	reset_unload_case("nf_nat_h323", 1, -1, 1);
	assert(rc_module_unload_verified("nf_nat_h323", 1,
					 &mock_unload_ops) == 0);
	assert(unload_calls == 1);
	assert(unload_probe_calls == 2);
	assert(unload_module_loaded[0] == 0);
}

static void
test_unload_busy_module_is_not_reported_as_removed(void)
{
	/* module_smart_unload() returns success for refcount > 0 but leaves it loaded. */
	reset_unload_case("nf_nat_sip", 1, 1, 0);
	assert(rc_module_unload_verified("nf_nat_sip", 1,
					 &mock_unload_ops) == EBUSY);
	assert(unload_calls == 1);
	assert(unload_probe_calls == 2);
	assert(unload_module_loaded[0] == 1);
}

static void
test_unload_invalid_inputs_have_no_side_effects(void)
{
	rc_module_unload_ops_t no_unloader = {
		NULL, mock_module_unload_probe
	};
	rc_module_unload_ops_t no_probe = {
		mock_module_unload, NULL
	};

	reset_unload_case("nf_nat_ftp", 1, 0, 1);
	assert(rc_module_unload_verified(NULL, 0, &mock_unload_ops) == EINVAL);
	assert(rc_module_unload_verified("", 0, &mock_unload_ops) == EINVAL);
	assert(rc_module_unload_verified("nf_nat_ftp", 0, NULL) == EINVAL);
	assert(rc_module_unload_verified("nf_nat_ftp", 0, &no_unloader) == EINVAL);
	assert(rc_module_unload_verified("nf_nat_ftp", 0, &no_probe) == EINVAL);
	assert(unload_calls == 0);
	assert(unload_probe_calls == 0);
}

static void
test_unload_requirement_set_attempts_all_and_keeps_first_failure(void)
{
	static const char *names[UNLOAD_TEST_MODULES] = {
		"nf_nat_pptp", "nf_nat_h323", "nf_nat_sip"
	};
	int i;
	int first_status = 0;

	unload_expected_count = UNLOAD_TEST_MODULES;
	unload_calls = 0;
	unload_probe_calls = 0;
	for (i = 0; i < UNLOAD_TEST_MODULES; ++i) {
		unload_expected_names[i] = names[i];
		unload_module_loaded[i] = 1;
		unload_command_results[i] = (i == 0) ? 1 : 0;
		unload_remove_module[i] = (i != 0);
	}
	for (i = 0; i < UNLOAD_TEST_MODULES; ++i) {
		int status = rc_module_unload_verified(names[i], 1,
						       &mock_unload_ops);
		if (first_status == 0 && status != 0)
			first_status = status;
	}
	assert(first_status == EBUSY);
	assert(unload_calls == UNLOAD_TEST_MODULES);
	assert(unload_probe_calls == UNLOAD_TEST_MODULES * 2);
	assert(unload_module_loaded[0] == 1);
	assert(unload_module_loaded[1] == 0);
	assert(unload_module_loaded[2] == 0);
}

static void
test_unload_initial_probe_error_fails_closed(void)
{
	reset_unload_case("nf_nat_pptp", 1, 0, 1);
	unload_probe_error_call = 1;
	unload_probe_error = EACCES;
	assert(rc_module_unload_verified("nf_nat_pptp", 1,
					 &mock_unload_ops) == EACCES);
	assert(unload_calls == 0);
	assert(unload_probe_calls == 1);
	assert(unload_module_loaded[0] == 1);
}

static void
test_unload_postcondition_probe_error_is_reported(void)
{
	reset_unload_case("nf_nat_h323", 1, 0, 1);
	unload_probe_error_call = 2;
	unload_probe_error = EIO;
	assert(rc_module_unload_verified("nf_nat_h323", 1,
					 &mock_unload_ops) == EIO);
	assert(unload_calls == 1);
	assert(unload_probe_calls == 2);
	assert(unload_module_loaded[0] == 0);
}

int
main(void)
{
	test_already_loaded_is_success();
	test_newly_loaded_is_success();
	test_failed_modprobe_is_not_success();
	test_visible_module_state_wins_over_loader_status();
	test_load_probe_error_is_reported();
	test_module_parameter_is_forwarded();
	test_invalid_inputs_fail_before_side_effects();
	test_required_module_set_attempts_all_and_keeps_first_failure();
	test_required_module_set_probe_error_attempts_all_and_keeps_first_failure();
	test_required_module_set_success_and_invalid_inputs();
	test_unload_absent_module_is_noop();
	test_unload_postcondition_wins_over_command_result();
	test_unload_busy_module_is_not_reported_as_removed();
	test_unload_invalid_inputs_have_no_side_effects();
	test_unload_requirement_set_attempts_all_and_keeps_first_failure();
	test_unload_initial_probe_error_fails_closed();
	test_unload_postcondition_probe_error_is_reported();
	puts("RC module load/unload postcondition policies: PASS");
	return 0;
}
