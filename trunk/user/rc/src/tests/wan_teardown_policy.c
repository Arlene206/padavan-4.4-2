#include <assert.h>
#include <stddef.h>
#include <string.h>
#include "wan_teardown_policy.h"

struct fake_context {
	const char *seen[3];
	int results[3];
	unsigned int calls;
};

static int
fake_ifdown(void *opaque, const char *ifname)
{
	struct fake_context *context = (struct fake_context *)opaque;
	assert(context->calls < 3);
	context->seen[context->calls] = ifname;
	return context->results[context->calls++];
}

static const struct wan_teardown_interface_ops fake_ops = {
	fake_ifdown
};

static void
test_wired_interfaces_both_succeed_without_usbnet(void)
{
	struct fake_context context = { { NULL, NULL, NULL }, { 0, 0, 0 }, 0 };
	assert(wan_teardown_ifdown_interfaces("eth2.1", "eth2.2", NULL,
					      &fake_ops, &context) == 0);
	assert(context.calls == 2);
	assert(strcmp(context.seen[0], "eth2.1") == 0);
	assert(strcmp(context.seen[1], "eth2.2") == 0);
}

static void
test_usbnet_interface_is_checked(void)
{
	struct fake_context context = { { NULL, NULL, NULL }, { 0, 0, 0 }, 0 };
	assert(wan_teardown_ifdown_interfaces("eth2.1", "eth2.2", "wwan0",
					      &fake_ops, &context) == 0);
	assert(context.calls == 3);
	assert(strcmp(context.seen[2], "wwan0") == 0);
}

static void
test_first_failure_does_not_skip_remaining_interfaces(void)
{
	struct fake_context context = { { NULL, NULL, NULL }, { -1, 0, 0 }, 0 };
	assert(wan_teardown_ifdown_interfaces("eth2.1", "eth2.2", "wwan0",
					      &fake_ops, &context) == -1);
	assert(context.calls == 3);
	assert(strcmp(context.seen[1], "eth2.2") == 0);
	assert(strcmp(context.seen[2], "wwan0") == 0);
}

static void
test_second_or_third_failure_is_reported(void)
{
	struct fake_context second = { { NULL, NULL, NULL }, { 0, 5, 0 }, 0 };
	struct fake_context third = { { NULL, NULL, NULL }, { 0, 0, -1 }, 0 };
	assert(wan_teardown_ifdown_interfaces("eth2.1", "eth2.2", "wwan0",
					      &fake_ops, &second) == -1);
	assert(second.calls == 3);
	assert(wan_teardown_ifdown_interfaces("eth2.1", "eth2.2", "wwan0",
					      &fake_ops, &third) == -1);
	assert(third.calls == 3);
}

static void
test_duplicate_names_are_attempted_once(void)
{
	struct fake_context first_third = { { NULL, NULL, NULL }, { 0, 0, 0 }, 0 };
	struct fake_context all_same = { { NULL, NULL, NULL }, { 0, 0, 0 }, 0 };
	assert(wan_teardown_ifdown_interfaces("wwan0", "eth2.2", "wwan0",
					      &fake_ops, &first_third) == 0);
	assert(first_third.calls == 2);
	assert(strcmp(first_third.seen[1], "eth2.2") == 0);
	assert(wan_teardown_ifdown_interfaces("wwan0", "wwan0", "wwan0",
					      &fake_ops, &all_same) == 0);
	assert(all_same.calls == 1);
}

static void
test_invalid_required_inputs_fail_without_side_effects(void)
{
	struct fake_context context = { { NULL, NULL, NULL }, { 0, 0, 0 }, 0 };
	assert(wan_teardown_ifdown_interfaces(NULL, "eth2.2", NULL,
					      &fake_ops, &context) == -1);
	assert(wan_teardown_ifdown_interfaces("", "eth2.2", NULL,
					      &fake_ops, &context) == -1);
	assert(wan_teardown_ifdown_interfaces("eth2.1", NULL, NULL,
					      &fake_ops, &context) == -1);
	assert(wan_teardown_ifdown_interfaces("eth2.1", "eth2.2", NULL,
					      NULL, &context) == -1);
	assert(context.calls == 0);
}

int
main(void)
{
	test_wired_interfaces_both_succeed_without_usbnet();
	test_usbnet_interface_is_checked();
	test_first_failure_does_not_skip_remaining_interfaces();
	test_second_or_third_failure_is_reported();
	test_duplicate_names_are_attempted_once();
	test_invalid_required_inputs_fail_without_side_effects();
	return 0;
}
