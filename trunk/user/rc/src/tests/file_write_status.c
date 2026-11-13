#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "file_write_status.h"

static void
test_regular_file_write_and_close(void)
{
	char path[] = "/tmp/fput-string-status.XXXXXX";
	char buffer[32];
	int fd;
	FILE *stream;

	fd = mkstemp(path);
	assert(fd >= 0);
	stream = fdopen(fd, "w");
	assert(stream != NULL);
	assert(fput_string_stream(stream, "router-state\n") == 0);

	stream = fopen(path, "r");
	assert(stream != NULL);
	assert(fgets(buffer, sizeof(buffer), stream) != NULL);
	assert(strcmp(buffer, "router-state\n") == 0);
	assert(fclose(stream) == 0);
	assert(unlink(path) == 0);
}

static void
test_dev_full_reports_buffered_close_failure(void)
{
	FILE *stream;
	int status;

	stream = fopen("/dev/full", "w");
	if (!stream) {
		puts("/dev/full unavailable; close-failure case skipped");
		return;
	}

	errno = 0;
	status = fput_string_stream(stream, "proc-sys-write\n");
	assert(status != 0);
	assert(errno == status);
}

int
main(void)
{
	test_regular_file_write_and_close();
	test_dev_full_reports_buffered_close_failure();
	puts("shared file write/close status regression: PASS");
	return 0;
}
