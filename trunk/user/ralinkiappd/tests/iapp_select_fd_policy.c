#include "iapp_select_fd_policy.h"
#include "iapp_tcp_deadline_policy.h"

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


static void
test_tcp_deadline_fractional_remaining_time(void)
{
	struct timespec now = { 100, 250000000L };
	struct timespec deadline = { 105, 750000000L };
	struct timeval remaining;

	assert(iapp_tcp_deadline_remaining(&now, &deadline, &remaining) == 1);
	assert(remaining.tv_sec == 5);
	assert(remaining.tv_usec == 500000);
}

static void
test_tcp_deadline_rounds_sub_microsecond_up(void)
{
	struct timespec now = { 200, 999999999L };
	struct timespec deadline = { 201, 0 };
	struct timeval remaining;

	assert(iapp_tcp_deadline_remaining(&now, &deadline, &remaining) == 1);
	assert(remaining.tv_sec == 0);
	assert(remaining.tv_usec == 1);
}

static void
test_tcp_deadline_expiry_and_second_borrow(void)
{
	struct timespec now = { 300, 500000000L };
	struct timespec deadline = { 300, 500000000L };
	struct timeval remaining;

	assert(iapp_tcp_deadline_remaining(&now, &deadline, &remaining) == 0);
	assert(remaining.tv_sec == 0 && remaining.tv_usec == 0);
	now.tv_nsec++;
	assert(iapp_tcp_deadline_remaining(&now, &deadline, &remaining) == 0);

	now.tv_sec = 400;
	now.tv_nsec = 900000000L;
	deadline.tv_sec = 401;
	deadline.tv_nsec = 100000000L;
	assert(iapp_tcp_deadline_remaining(&now, &deadline, &remaining) == 1);
	assert(remaining.tv_sec == 0);
	assert(remaining.tv_usec == 200000);
}

static void
test_tcp_deadline_invalid_inputs(void)
{
	struct timespec now = { 1, -1 };
	struct timespec deadline = { 2, 0 };
	struct timeval remaining;

	assert(iapp_tcp_deadline_remaining(&now, &deadline, &remaining) == -1);
	now.tv_nsec = 0;
	deadline.tv_nsec = 1000000000L;
	assert(iapp_tcp_deadline_remaining(&now, &deadline, &remaining) == -1);
	deadline.tv_nsec = 0;
	assert(iapp_tcp_deadline_remaining(NULL, &deadline, &remaining) == -1);
	assert(iapp_tcp_deadline_remaining(&now, NULL, &remaining) == -1);
	assert(iapp_tcp_deadline_remaining(&now, &deadline, NULL) == -1);
}

int main(void)
{
	test_valid_descriptors();
	test_rejects_invalid_descriptors_without_mutation();
	test_rejects_invalid_arguments();
	test_tcp_deadline_fractional_remaining_time();
	test_tcp_deadline_rounds_sub_microsecond_up();
	test_tcp_deadline_expiry_and_second_borrow();
	test_tcp_deadline_invalid_inputs();
	puts("Ralink IAPP select FD_SETSIZE policy: PASS");
	puts("Ralink IAPP monotonic TCP deadline policy: PASS");
	return 0;
}
