#include <assert.h>
#include <errno.h>
#include <stdio.h>

#include "detect_link_nvram_policy.h"

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
test_failed_publication_is_retried(void)
{
	struct fake_writer writer;
	int last_published = -1;

	writer.calls = 0;
	writer.result = -1;
	writer.last_status = -1;

	assert(detect_link_publish_wan_status(
		1, &last_published, fake_write_status, &writer) == -1);
	assert(writer.calls == 1);
	assert(writer.last_status == 1);
	assert(last_published == -1);

	writer.result = 0;
	assert(detect_link_publish_wan_status(
		1, &last_published, fake_write_status, &writer) == 0);
	assert(writer.calls == 2);
	assert(last_published == 1);

	assert(detect_link_publish_wan_status(
		1, &last_published, fake_write_status, &writer) == 0);
	assert(writer.calls == 2);
}

static void
test_changed_state_retries_without_replaying_side_effects(void)
{
	struct fake_writer writer;
	int last_published = 1;

	writer.calls = 0;
	writer.result = -1;
	writer.last_status = -1;

	assert(detect_link_publish_wan_status(
		0, &last_published, fake_write_status, &writer) == -1);
	assert(writer.calls == 1);
	assert(last_published == 1);

	writer.result = 0;
	assert(detect_link_publish_wan_status(
		0, &last_published, fake_write_status, &writer) == 0);
	assert(writer.calls == 2);
	assert(last_published == 0);
}

static void
test_wan_link_sample_stays_unknown_until_success(void)
{
	int known = 0;

	assert(detect_link_wan_sample_known(known, -1) == 0);
	assert(detect_link_wan_sample_known(known, -EIO) == 0);
	assert(detect_link_wan_sample_known(known, 0) == 1);
	assert(detect_link_wan_sample_known(known, 1) == 1);
	assert(detect_link_wan_sample_known(1, -1) == 1);
	assert(detect_link_wan_sample_known(1, EIO) == 1);
}

static void
test_ap_mode_cache_changes_only_after_successful_read(void)
{
	int cached_ap_mode = 1;

	assert(detect_link_apply_ap_mode_read(EIO, 0, &cached_ap_mode) == EIO);
	assert(cached_ap_mode == 1);
	assert(detect_link_apply_ap_mode_read(0, 0, &cached_ap_mode) == 0);
	assert(cached_ap_mode == 0);
	assert(detect_link_apply_ap_mode_read(0, 1, &cached_ap_mode) == 0);
	assert(cached_ap_mode == 1);
	assert(detect_link_apply_ap_mode_read(0, 1, NULL) == EINVAL);
}

static void
test_invalid_policy_arguments_fail_closed(void)
{
	struct fake_writer writer;

	writer.calls = 0;
	writer.result = 0;
	writer.last_status = -1;
	assert(detect_link_publish_wan_status(
		1, NULL, fake_write_status, &writer) == EINVAL);
	assert(detect_link_publish_wan_status(
		1, &writer.last_status, NULL, &writer) == EINVAL);
	assert(writer.calls == 0);
}

int
main(void)
{
	test_failed_publication_is_retried();
	test_changed_state_retries_without_replaying_side_effects();
	test_wan_link_sample_stays_unknown_until_success();
	test_ap_mode_cache_changes_only_after_successful_read();
	test_invalid_policy_arguments_fail_closed();
	puts("detect_link WAN NVRAM mirror retry policy: PASS");
	return 0;
}
