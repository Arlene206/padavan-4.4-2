#include <assert.h>
#include <errno.h>
#include "../ipv4_forward_policy.h"

typedef struct {
	int requested;
	int result;
	int calls;
} mock_forward_writer_t;

static int
mock_forward_write(void *context, int enabled)
{
	mock_forward_writer_t *mock = (mock_forward_writer_t *)context;
	mock->requested = enabled;
	mock->calls++;
	return mock->result;
}

int
main(void)
{
	mock_forward_writer_t mock;
	mock.requested = -1;
	mock.result = 0;
	mock.calls = 0;
	assert(rc_ipv4_forward_apply(0, mock_forward_write, &mock) == 0);
	assert(mock.requested == 0 && mock.calls == 1);
	assert(rc_ipv4_forward_apply(1, mock_forward_write, &mock) == 0);
	assert(mock.requested == 1 && mock.calls == 2);
	mock.result = EACCES;
	assert(rc_ipv4_forward_apply(0, mock_forward_write, &mock) == EACCES);
	assert(mock.requested == 0 && mock.calls == 3);
	mock.result = EROFS;
	assert(rc_ipv4_forward_apply(1, mock_forward_write, &mock) == EROFS);
	assert(mock.requested == 1 && mock.calls == 4);
	mock.result = -1;
	assert(rc_ipv4_forward_apply(0, mock_forward_write, &mock) == EIO);
	assert(rc_ipv4_forward_apply(0, NULL, &mock) == EINVAL);
	return 0;
}
