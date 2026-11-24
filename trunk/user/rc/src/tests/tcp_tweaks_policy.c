#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <string.h>

#include "tcp_tweaks_policy.h"

struct fake_writer {
	size_t calls;
	int status[RC_TCP_TWEAK_COUNT];
};

static int
fake_write_tcp_tweak(void *context, const char *name, int value)
{
	struct fake_writer *writer = (struct fake_writer *)context;
	size_t call = writer->calls;
	int status;

	if (call >= RC_TCP_TWEAK_COUNT)
		return EOVERFLOW;

	assert(strcmp(name, rc_tcp_tweak_settings[call].name) == 0);
	assert(value == rc_tcp_tweak_settings[call].value);
	status = writer->status[call];
	writer->calls++;
	return status;
}

static void
test_success_attempts_every_tweak(void)
{
	struct fake_writer writer;

	memset(&writer, 0, sizeof(writer));
	assert(rc_tcp_tweaks_apply(fake_write_tcp_tweak, &writer) == 0);
	assert(writer.calls == RC_TCP_TWEAK_COUNT);
}

static void
test_failures_do_not_skip_later_tweaks_and_preserve_first_error(void)
{
	struct fake_writer writer;

	memset(&writer, 0, sizeof(writer));
	writer.status[1] = EACCES;
	writer.status[4] = ENOSPC;
	writer.status[7] = EIO;

	assert(rc_tcp_tweaks_apply(fake_write_tcp_tweak, &writer) == EACCES);
	assert(writer.calls == RC_TCP_TWEAK_COUNT);
}

static void
test_missing_writer_is_rejected(void)
{
	assert(rc_tcp_tweaks_apply(NULL, NULL) == EINVAL);
}

int
main(void)
{
	test_success_attempts_every_tweak();
	test_failures_do_not_skip_later_tweaks_and_preserve_first_error();
	test_missing_writer_is_rejected();
	return 0;
}
