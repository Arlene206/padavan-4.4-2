#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "conntrack_flush_policy.h"
static int calls, status;
static const char *path_written, *value_written;
static int fake_write(const char *path, const char *value)
{
	calls++; path_written = path; value_written = value; return status;
}
static void reset(int result)
{
	calls = 0; status = result; path_written = NULL; value_written = NULL;
}
int main(void)
{
	reset(0);
	assert(rc_conntrack_flush_apply("192.0.2.7", fake_write) == 0);
	assert(calls == 1 && strcmp(path_written, "/proc/net/nf_conntrack") == 0);
	assert(strcmp(value_written, "192.0.2.7") == 0);
	reset(0);
	assert(rc_conntrack_flush_apply(NULL, fake_write) == 0);
	assert(calls == 1 && strcmp(value_written, "f") == 0);
	reset(EACCES);
	assert(rc_conntrack_flush_apply("198.51.100.9", fake_write) == EACCES);
	assert(calls == 1);
	reset(0);
	assert(rc_conntrack_flush_apply("192.0.2.7", NULL) == EINVAL);
	assert(calls == 0);
	puts("Conntrack flush result propagation policy: PASS");
	return 0;
}
