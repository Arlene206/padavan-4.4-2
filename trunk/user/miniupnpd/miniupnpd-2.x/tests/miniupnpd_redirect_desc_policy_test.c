#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdio.h>

#include "../netfilter/miniupnpd_redirect_desc_policy.h"

static int allocation_calls;
static size_t observed_size;
static unsigned char storage[64];

static void *
fail_allocator(size_t size)
{
	allocation_calls++;
	observed_size = size;
	errno = ENOMEM;
	return NULL;
}

static void *
success_allocator(size_t size)
{
	allocation_calls++;
	observed_size = size;
	return storage;
}

static void
test_allocation_failure(void)
{
	allocation_calls = 0;
	errno = 0;
	assert(miniupnpd_redirect_desc_allocate(16, 8, fail_allocator) == NULL);
	assert(allocation_calls == 1);
	assert(observed_size == 24);
	assert(errno == ENOMEM);
}

static void
test_success(void)
{
	allocation_calls = 0;
	errno = 0;
	assert(miniupnpd_redirect_desc_allocate(16, 8, success_allocator) == storage);
	assert(allocation_calls == 1);
	assert(observed_size == 24);
}

static void
test_overflow_and_invalid_allocator(void)
{
	allocation_calls = 0;
	errno = 0;
	assert(miniupnpd_redirect_desc_allocate((size_t)-1, 1, fail_allocator) == NULL);
	assert(allocation_calls == 0);
	assert(errno == EOVERFLOW);
	assert(miniupnpd_redirect_desc_allocate(1, 1, NULL) == NULL);
	assert(errno == EINVAL);
	assert(miniupnpd_redirect_desc_allocate(1, 0, fail_allocator) == NULL);
	assert(errno == EINVAL);
	assert(allocation_calls == 0);
}

int
main(void)
{
	test_allocation_failure();
	test_success();
	test_overflow_and_invalid_allocator();
	puts("MiniUPnP redirect metadata allocation policy: PASS");
	return 0;
}
