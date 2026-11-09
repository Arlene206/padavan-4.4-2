#include <assert.h>
#include <stddef.h>
#include <string.h>
#include "wan_teardown_policy.h"

struct fake_context {
	const char *seen[2];
	int results[2];
	unsigned int calls;
};

static int
fake_ifdown(void *opaque, const char *ifname)
{
	struct fake_context *context = (struct fake_context *)opaque;
	assert(context->calls < 2);
	context->seen[context->calls] = ifname;
	return context->results[context->calls++];
}

static const struct wan_teardown_interface_ops fake_ops = {
	fake_ifdown
};

static void
test_distinct_interfaces_both_succeed(void)
{
	struct fake_context context = { { NULL, NULL }, { 0, 0 }, 0 };
	assert(wan_teardown_ifdown_interfaces("eth2.1", "eth2.2",
					      &fake_ops, &context) == 0);
	assert(context.calls == 2);
	assert(strcmp(context.seen[0], "eth2.1") == 0);
	assert(strcmp(context.seen[1], "eth2.2") == 0);
}

static void
test_first_failure_does_not_skip_second_interface(void)
{
	struct fake_context context = { { NULL, NULL }, { -1, 0 }, 0 };
	assert(wan_teardown_ifdown_interfaces("wwan0", "eth2.2",
					      &fake_ops, &context) == -1);
	assert(context.calls == 2);
	assert(strcmp(context.seen[1], "eth2.2") == 0);
}

static void
test_second_interface_failure_is_reported(void)
{
	struct fake_context context = { { NULL, NULL }, { 0, 5 }, 0 };
	assert(wan_teardown_ifdown_interfaces("eth2.1", "eth2.2",
					      &fake_ops, &context) == -1);
	assert(context.calls == 2);
}

static void
test_duplicate_interface_is_attempted_once(void)
{
	struct fake_context context = { { NULL, NULL }, { 0, 0 }, 0 };
	assert(wan_teardown_ifdown_interfaces("eth2.2", "eth2.2",
					      &fake_ops, &context) == 0);
	assert(context.calls == 1);
}

static void
test_invalid_inputs_fail_without_side_effects(void)
{
	struct fake_context context = { { NULL, NULL }, { 0, 0 }, 0 };
	assert(wan_teardown_ifdown_interfaces(NULL, "eth2.2",
					      &fake_ops, &context) == -1);
	assert(wan_teardown_ifdown_interfaces("", "eth2.2",
					      &fake_ops, &context) == -1);
	assert(wan_teardown_ifdown_interfaces("eth2.1", NULL,
					      &fake_ops, &context) == -1);
	assert(wan_teardown_ifdown_interfaces("eth2.1", "eth2.2",
					      NULL, &context) == -1);
	assert(context.calls == 0);
}

int
main(void)
{
	test_distinct_interfaces_both_succeed();
	test_first_failure_does_not_skip_second_interface();
	test_second_interface_failure_is_reported();
	test_duplicate_interface_is_attempted_once();
	test_invalid_inputs_fail_without_side_effects();
	return 0;
}
