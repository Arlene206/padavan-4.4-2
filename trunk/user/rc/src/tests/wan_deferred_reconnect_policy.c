#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include "wan_deferred_reconnect_policy.h"

struct fake_publisher {
	char name[32];
	long value;
	int result;
	unsigned int calls;
};

static int
fake_publish(void *opaque, const char *name, long value)
{
	struct fake_publisher *publisher = (struct fake_publisher *)opaque;
	assert(publisher != NULL);
	assert(name != NULL);
	assert(strlen(name) < sizeof(publisher->name));
	strcpy(publisher->name, name);
	publisher->value = value;
	publisher->calls++;
	return publisher->result;
}

static void
test_clear_publishes_zero(void)
{
	struct fake_publisher publisher = { "", -1, 0, 0 };
	assert(wan_deferred_reconnect_clear(fake_publish, &publisher) == 0);
	assert(publisher.calls == 1);
	assert(strcmp(publisher.name, "deferred_wanup_t") == 0);
	assert(publisher.value == 0);
}

static void
test_clear_preserves_nvram_failure(void)
{
	struct fake_publisher publisher = { "", -1, ENOSPC, 0 };
	assert(wan_deferred_reconnect_clear(fake_publish, &publisher) == ENOSPC);
	assert(publisher.calls == 1);
	assert(publisher.value == 0);
}

static void
test_schedule_publishes_valid_deadline(void)
{
	struct fake_publisher publisher = { "", -1, 0, 0 };
	assert(wan_deferred_reconnect_schedule(100, 30, fake_publish,
					       &publisher) == 0);
	assert(publisher.calls == 1);
	assert(strcmp(publisher.name, "deferred_wanup_t") == 0);
	assert(publisher.value == 130);
}

static void
test_schedule_propagates_nvram_failure(void)
{
	struct fake_publisher publisher = { "", -1, EIO, 0 };
	assert(wan_deferred_reconnect_schedule(100, 30, fake_publish,
					       &publisher) == EIO);
	assert(publisher.calls == 1);
	assert(publisher.value == 130);
}

static void
test_schedule_rejects_invalid_and_overflowing_deadlines(void)
{
	struct fake_publisher publisher = { "", -1, 0, 0 };
	assert(wan_deferred_reconnect_schedule(-1, 10, fake_publish,
					       &publisher) == EINVAL);
	assert(wan_deferred_reconnect_schedule(10, 0, fake_publish,
					       &publisher) == EINVAL);
	assert(wan_deferred_reconnect_schedule(10, -1, fake_publish,
					       &publisher) == EINVAL);
	assert(wan_deferred_reconnect_schedule((long)INT_MAX - 1, 2,
					       fake_publish, &publisher) == EOVERFLOW);
#if LONG_MAX > INT_MAX
	assert(wan_deferred_reconnect_schedule(10, (long)INT_MAX + 1,
					       fake_publish, &publisher) == EOVERFLOW);
	assert(wan_deferred_reconnect_publish(fake_publish, &publisher,
					      (long)INT_MAX + 1) == EOVERFLOW);
#endif
	assert(publisher.calls == 0);
}

static void
test_missing_publisher_is_rejected(void)
{
	assert(wan_deferred_reconnect_clear(NULL, NULL) == EINVAL);
	assert(wan_deferred_reconnect_schedule(10, 5, NULL, NULL) == EINVAL);
}

int
main(void)
{
	test_clear_publishes_zero();
	test_clear_preserves_nvram_failure();
	test_schedule_publishes_valid_deadline();
	test_schedule_propagates_nvram_failure();
	test_schedule_rejects_invalid_and_overflowing_deadlines();
	test_missing_publisher_is_rejected();
	return 0;
}
