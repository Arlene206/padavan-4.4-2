#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "detect_internet_nvram_policy.h"

struct fake_writer {
	int calls;
	int result;
	int last_status;
};

static int
fake_write_status(void *context, int status)
{
	struct fake_writer *writer = (struct fake_writer *)context;

	if (!writer)
		return EINVAL;
	writer->calls++;
	writer->last_status = status;
	return writer->result;
}

static void
test_failed_unknown_state_publication_is_retried(void)
{
	struct fake_writer writer;
	int last_published = -1;

	writer.calls = 0;
	writer.result = EIO;
	writer.last_status = -1;

	assert(detect_internet_publish_status(
		2, &last_published, fake_write_status, &writer) == EIO);
	assert(writer.calls == 1);
	assert(writer.last_status == 2);
	assert(last_published == -1);

	writer.result = 0;
	assert(detect_internet_publish_status(
		2, &last_published, fake_write_status, &writer) == 0);
	assert(writer.calls == 2);
	assert(last_published == 2);

	assert(detect_internet_publish_status(
		2, &last_published, fake_write_status, &writer) == 0);
	assert(writer.calls == 2);
}

static void
test_changed_connectivity_state_keeps_previous_cache_on_failure(void)
{
	struct fake_writer writer;
	int last_published = 1;

	writer.calls = 0;
	writer.result = EIO;
	writer.last_status = -1;

	assert(detect_internet_publish_status(
		0, &last_published, fake_write_status, &writer) == EIO);
	assert(writer.calls == 1);
	assert(last_published == 1);

	writer.result = 0;
	assert(detect_internet_publish_status(
		0, &last_published, fake_write_status, &writer) == 0);
	assert(writer.calls == 2);
	assert(last_published == 0);
}

static void
test_invalid_policy_arguments_fail_closed(void)
{
	struct fake_writer writer;

	writer.calls = 0;
	writer.result = 0;
	writer.last_status = -1;
	assert(detect_internet_publish_status(
		0, NULL, fake_write_status, &writer) == EINVAL);
	assert(detect_internet_publish_status(
		0, &writer.last_status, NULL, &writer) == EINVAL);
	assert(writer.calls == 0);
}

int
main(void)
{
	test_failed_unknown_state_publication_is_retried();
	test_changed_connectivity_state_keeps_previous_cache_on_failure();
	test_invalid_policy_arguments_fail_closed();
	puts("detect_internet NVRAM mirror retry policy: PASS");
	return 0;
}
