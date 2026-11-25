#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include <sys/select.h>

#include "wsd_select_fd_policy.h"

struct guarded_fd_set {
	fd_set set;
	unsigned char canary[16];
};

static void
test_valid_fd_boundaries_are_registered(void)
{
	fd_set set;
	int maxfd = -1;

	FD_ZERO(&set);
	assert(wsd_select_fd_add(&set, 0, &maxfd) == 0);
	assert(maxfd == 0);
	assert(FD_ISSET(0, &set));

	assert(wsd_select_fd_add(&set, FD_SETSIZE - 1, &maxfd) == 0);
	assert(maxfd == FD_SETSIZE - 1);
	assert(FD_ISSET(FD_SETSIZE - 1, &set));
}

static void
test_out_of_range_fd_is_rejected_before_fd_set(void)
{
	struct guarded_fd_set guarded;
	int maxfd = 7;
	int i;

	memset(&guarded, 0, sizeof(guarded));
	memset(guarded.canary, 0xa5, sizeof(guarded.canary));

	errno = 0;
	assert(wsd_select_fd_add(&guarded.set, FD_SETSIZE, &maxfd) == -1);
	assert(errno == EMFILE);
	assert(maxfd == 7);
	for (i = 0; i < (int)sizeof(guarded.canary); ++i)
		assert(guarded.canary[i] == 0xa5);

	errno = 0;
	assert(wsd_select_fd_add(&guarded.set, INT_MAX, &maxfd) == -1);
	assert(errno == EMFILE);
	assert(maxfd == 7);
	for (i = 0; i < (int)sizeof(guarded.canary); ++i)
		assert(guarded.canary[i] == 0xa5);
}

static void
test_invalid_arguments_are_rejected(void)
{
	fd_set set;
	int maxfd = -1;

	errno = 0;
	assert(wsd_select_fd_add(NULL, 0, &maxfd) == -1);
	assert(errno == EINVAL);
	errno = 0;
	assert(wsd_select_fd_add(&set, -1, &maxfd) == -1);
	assert(errno == EINVAL);
	errno = 0;
	assert(wsd_select_fd_add(&set, 0, NULL) == -1);
	assert(errno == EINVAL);
}

int
main(void)
{
	test_valid_fd_boundaries_are_registered();
	test_out_of_range_fd_is_rejected_before_fd_set();
	test_invalid_arguments_are_rejected();
	return 0;
}
