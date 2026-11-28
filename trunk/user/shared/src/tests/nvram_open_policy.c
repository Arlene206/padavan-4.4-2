#include <assert.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "nvram_open_policy.h"

static void
test_device_open_flags_preserve_read_write(void)
{
	int flags = nvram_device_open_flags();

	assert((flags & O_ACCMODE) == O_RDWR);
	assert((flags & O_CLOEXEC) != 0);
}

static void
test_open_descriptor_is_close_on_exec(void)
{
	int fd, descriptor_flags;

	fd = open("/dev/null", nvram_device_open_flags());
	assert(fd >= 0);
	descriptor_flags = fcntl(fd, F_GETFD);
	assert(descriptor_flags >= 0);
	assert((descriptor_flags & FD_CLOEXEC) != 0);
	assert(close(fd) == 0);
}

int
main(void)
{
	test_device_open_flags_preserve_read_write();
	test_open_descriptor_is_close_on_exec();
	puts("NVRAM device descriptor close-on-exec policy: PASS");
	return 0;
}
