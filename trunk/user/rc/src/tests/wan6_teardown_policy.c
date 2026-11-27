#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "wan6_teardown_policy.h"

static void
assert_ifname(char *actual, const char *expected)
{
	if (expected == NULL) {
		assert(actual == NULL);
		return;
	}

	assert(actual != NULL);
	assert(strcmp(actual, expected) == 0);
}

static void
test_disabled_mode_cleans_last_live_native_interface(void)
{
	assert_ifname(wan6_teardown_select_ifname(
		"eth2.2", 1, 1, 0, 0, "eth2", "sit0"), "eth2.2");
}

static void
test_disabled_mode_cleans_last_live_sit_interface(void)
{
	assert_ifname(wan6_teardown_select_ifname(
		"sit0", 1, 1, 0, 1, "eth2", "sit0"), "sit0");
}

static void
test_disabled_mode_falls_back_to_orphaned_sit_device(void)
{
	assert_ifname(wan6_teardown_select_ifname(
		"", 0, 1, 0, 1, "eth2", "sit0"), "sit0");
}

static void
test_disabled_mode_skips_absent_runtime_interface(void)
{
	assert_ifname(wan6_teardown_select_ifname(
		"ppp0", 0, 1, 0, 0, "eth2", "sit0"), NULL);
}

static void
test_reconfiguration_prefers_previous_live_interface(void)
{
	/* NVRAM now says SIT, but the old live state is still on eth2. */
	assert_ifname(wan6_teardown_select_ifname(
		"eth2", 1, 0, 1, 0, "eth2", "sit0"), "eth2");
}

static void
test_current_sit_without_saved_state_uses_sit_device(void)
{
	assert_ifname(wan6_teardown_select_ifname(
		"", 0, 0, 1, 0, "eth2", "sit0"), "sit0");
}

static void
test_current_native_without_saved_state_uses_wan_device(void)
{
	assert_ifname(wan6_teardown_select_ifname(
		"", 0, 0, 0, 0, "eth2", "sit0"), "eth2");
}

static void
test_empty_native_request_name_has_no_fallback_interface(void)
{
	assert_ifname(wan6_teardown_select_ifname(
		"", 0, 0, 0, 0, "", "sit0"), NULL);
	assert_ifname(wan6_teardown_select_ifname(
		"", 0, 0, 0, 0, NULL, "sit0"), NULL);
}

static void
test_empty_request_still_cleans_previous_live_interface(void)
{
	assert_ifname(wan6_teardown_select_ifname(
		"eth2.2", 1, 0, 0, 0, "", "sit0"), "eth2.2");
}


static void
test_previous_interface_cleanup_with_null_request(void)
{
	assert(wan6_teardown_needs_previous_interface_cleanup(
		"eth2.2", NULL, "sit0") == 1);
}

static void
test_previous_interface_cleanup_with_empty_request(void)
{
	assert(wan6_teardown_needs_previous_interface_cleanup(
		"eth2.2", "", "sit0") == 1);
}

static void
test_same_requested_interface_needs_no_second_cleanup(void)
{
	assert(wan6_teardown_needs_previous_interface_cleanup(
		"eth2.2", "eth2.2", "sit0") == 0);
}

static void
test_sit_interface_is_not_treated_as_previous_native_interface(void)
{
	assert(wan6_teardown_needs_previous_interface_cleanup(
		"sit0", NULL, "sit0") == 0);
}

static void
test_missing_selected_interface_needs_no_previous_cleanup(void)
{
	assert(wan6_teardown_needs_previous_interface_cleanup(
		NULL, NULL, "sit0") == 0);
}

int
main(void)
{
	test_disabled_mode_cleans_last_live_native_interface();
	test_disabled_mode_cleans_last_live_sit_interface();
	test_disabled_mode_falls_back_to_orphaned_sit_device();
	test_disabled_mode_skips_absent_runtime_interface();
	test_reconfiguration_prefers_previous_live_interface();
	test_current_sit_without_saved_state_uses_sit_device();
	test_current_native_without_saved_state_uses_wan_device();
	test_empty_native_request_name_has_no_fallback_interface();
	test_empty_request_still_cleans_previous_live_interface();
	test_previous_interface_cleanup_with_null_request();
	test_previous_interface_cleanup_with_empty_request();
	test_same_requested_interface_needs_no_second_cleanup();
	test_sit_interface_is_not_treated_as_previous_native_interface();
	test_missing_selected_interface_needs_no_previous_cleanup();
	puts("WAN IPv6 teardown selection policy: PASS");
	return 0;
}
