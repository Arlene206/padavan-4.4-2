#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include "passwd_temp_file_policy.h"

static void
test_temp_file_is_exclusive_and_private(void)
{
	char first_path[] = "/tmp/.padavan-passwd-test.XXXXXX";
	char second_path[] = "/tmp/.padavan-passwd-test.XXXXXX";
	struct stat st;
	int first_fd, second_fd;
	first_fd = passwd_temp_file_open(first_path, 0600);
	assert(first_fd >= 0);
	assert(fstat(first_fd, &st) == 0);
	assert(S_ISREG(st.st_mode));
	assert((st.st_mode & 0777) == 0600);
	assert(st.st_nlink == 1);
	assert(close(first_fd) == 0);
	second_fd = passwd_temp_file_open(second_path, 0600);
	assert(second_fd >= 0);
	assert(strcmp(first_path, second_path) != 0);
	assert(fstat(second_fd, &st) == 0);
	assert((st.st_mode & 0777) == 0600);
	assert(close(second_fd) == 0);
	assert(unlink(first_path) == 0);
	assert(unlink(second_path) == 0);
}
static void
test_invalid_template_fails_closed(void)
{
	errno = 0;
	assert(passwd_temp_file_open(NULL, 0600) == -1);
	assert(errno == EINVAL);
}
static void
test_requested_mode_is_applied(void)
{
	char path[] = "/tmp/.padavan-passwd-test-mode.XXXXXX";
	struct stat st;
	int fd = passwd_temp_file_open(path, 0640);
	assert(fd >= 0);
	assert(fstat(fd, &st) == 0);
	assert((st.st_mode & 0777) == 0640);
	assert(close(fd) == 0);
	assert(unlink(path) == 0);
}
int
main(void)
{
	test_temp_file_is_exclusive_and_private();
	test_invalid_template_fails_closed();
	test_requested_mode_is_applied();
	puts("Secure password temporary file policy: PASS");
	return 0;
}
