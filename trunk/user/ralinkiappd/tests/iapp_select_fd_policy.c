#include "iapp_select_fd_policy.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

struct guarded_fd_set {
	unsigned long before;
	fd_set set;
	unsigned long after;
};

static void test_valid_descriptors(void)
{
	struct guarded_fd_set guarded;
	int max_fd = -1;

	memset(&guarded, 0, sizeof(guarded));
	guarded.before = 0x13579bdfUL;
	guarded.after = 0xfdb97531UL;
	FD_ZERO(&guarded.set);

	assert(iapp_select_fd_add(&guarded.set, 0, &max_fd) == 0);
	assert(max_fd == 0);
	assert(FD_ISSET(0, &guarded.set));
	assert(iapp_select_fd_add(&guarded.set, FD_SETSIZE - 1, &max_fd) == 0);
	assert(max_fd == FD_SETSIZE - 1);
	assert(FD_ISSET(FD_SETSIZE - 1, &guarded.set));
	assert(guarded.before == 0x13579bdfUL);
	assert(guarded.after == 0xfdb97531UL);
}

static void expect_rejected_descriptor(struct guarded_fd_set *guarded,
				       int fd, int expected_errno)
{
	int max_fd = 17;

	errno = 0;
	assert(iapp_select_fd_add(&guarded->set, fd, &max_fd) == -1);
	assert(errno == expected_errno);
	assert(max_fd == 17);
	assert(!FD_ISSET(0, &guarded->set));
	assert(guarded->before == 0x13579bdfUL);
	assert(guarded->after == 0xfdb97531UL);
}

static void test_rejects_invalid_descriptors_without_mutation(void)
{
	struct guarded_fd_set guarded;

	memset(&guarded, 0, sizeof(guarded));
	guarded.before = 0x13579bdfUL;
	guarded.after = 0xfdb97531UL;
	FD_ZERO(&guarded.set);

	expect_rejected_descriptor(&guarded, -1, EBADF);
	expect_rejected_descriptor(&guarded, FD_SETSIZE, EINVAL);
	expect_rejected_descriptor(&guarded, INT_MAX, EINVAL);
}

static void test_rejects_invalid_arguments(void)
{
	fd_set set;
	int max_fd = -1;

	FD_ZERO(&set);
	errno = 0;
	assert(iapp_select_fd_add(NULL, 0, &max_fd) == -1);
	assert(errno == EINVAL);
	errno = 0;
	assert(iapp_select_fd_add(&set, 0, NULL) == -1);
	assert(errno == EINVAL);
	assert(!FD_ISSET(0, &set));
	assert(max_fd == -1);
}

int main(void)
{
	test_valid_descriptors();
	test_rejects_invalid_descriptors_without_mutation();
	test_rejects_invalid_arguments();
	puts("Ralink IAPP select FD_SETSIZE policy: PASS");
	return 0;
}
