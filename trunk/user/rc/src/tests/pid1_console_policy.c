#include <assert.h>
#include <errno.h>
#include <string.h>
#include <termios.h>

#include "pid1_console_policy.h"

static struct termios input_attributes;
static struct termios output_attributes;
static int getter_status;
static int getter_errno;
static int getter_calls;
static int setter_status;
static int setter_errno;
static int setter_calls;
static int observed_fd;
static int observed_action;

static int
fake_getattr(int fd, struct termios *attributes)
{
	getter_calls++;
	observed_fd = fd;
	if (getter_status < 0) {
		errno = getter_errno;
		return -1;
	}
	*attributes = input_attributes;
	return 0;
}

static int
fake_setattr(int fd, int action, const struct termios *attributes)
{
	setter_calls++;
	observed_fd = fd;
	observed_action = action;
	output_attributes = *attributes;
	if (setter_status < 0) {
		errno = setter_errno;
		return -1;
	}
	return 0;
}

static void
reset_mocks(void)
{
	memset(&input_attributes, 0, sizeof(input_attributes));
	memset(&output_attributes, 0, sizeof(output_attributes));
	getter_status = 0;
	getter_errno = 0;
	getter_calls = 0;
	setter_status = 0;
	setter_errno = 0;
	setter_calls = 0;
	observed_fd = -1;
	observed_action = -1;
}



static int duplicate_calls;
static int duplicate_failure_target;
static int duplicate_errno;
static int duplicate_sources[3];
static int duplicate_targets[3];
static int claim_calls;
static int claim_status;
static int claim_errno;
static int foreground_calls;
static int foreground_status;
static int foreground_errno;
static int observed_request;
static int observed_argument;
static pid_t observed_group;

static int
fake_duplicate(int source_fd, int target_fd)
{
	if (duplicate_calls < 3) {
		duplicate_sources[duplicate_calls] = source_fd;
		duplicate_targets[duplicate_calls] = target_fd;
	}
	duplicate_calls++;
	if (target_fd == duplicate_failure_target) {
		errno = duplicate_errno;
		return -1;
	}
	return target_fd;
}

static int
fake_claim_terminal(int fd, unsigned long request, int argument)
{
	assert(fd == STDIN_FILENO);
	claim_calls++;
	observed_request = (int)request;
	observed_argument = argument;
	if (claim_status < 0) {
		errno = claim_errno;
		return -1;
	}
	return 0;
}

static int
fake_set_foreground(int fd, pid_t group)
{
	assert(fd == STDIN_FILENO);
	foreground_calls++;
	observed_group = group;
	if (foreground_status < 0) {
		errno = foreground_errno;
		return -1;
	}
	return 0;
}

static void
reset_attach_mocks(void)
{
	duplicate_calls = 0;
	duplicate_failure_target = -1;
	duplicate_errno = 0;
	memset(duplicate_sources, 0, sizeof(duplicate_sources));
	memset(duplicate_targets, 0, sizeof(duplicate_targets));
	claim_calls = 0;
	claim_status = 0;
	claim_errno = 0;
	foreground_calls = 0;
	foreground_status = 0;
	foreground_errno = 0;
	observed_request = 0;
	observed_argument = 0;
	observed_group = 0;
}

static void
test_attach_requires_complete_standard_io_and_terminal_setup(void)
{
	reset_attach_mocks();
	assert(pid1_console_attach(17, 42, 1, fake_duplicate,
				  fake_claim_terminal, fake_set_foreground) == 0);
	assert(duplicate_calls == 3);
	assert(duplicate_sources[0] == 17 && duplicate_targets[0] == STDIN_FILENO);
	assert(duplicate_sources[1] == 17 && duplicate_targets[1] == STDOUT_FILENO);
	assert(duplicate_sources[2] == 17 && duplicate_targets[2] == STDERR_FILENO);
	assert(claim_calls == 1 && observed_request == TIOCSCTTY && observed_argument == 1);
	assert(foreground_calls == 1 && observed_group == 42);

	reset_attach_mocks();
	duplicate_failure_target = STDOUT_FILENO;
	duplicate_errno = EACCES;
	assert(pid1_console_attach(17, 42, 1, fake_duplicate,
				  fake_claim_terminal, fake_set_foreground) == EACCES);
	assert(duplicate_calls == 2 && claim_calls == 0 && foreground_calls == 0);

	reset_attach_mocks();
	duplicate_failure_target = STDIN_FILENO;
	assert(pid1_console_attach(17, 42, 1, fake_duplicate,
				  fake_claim_terminal, fake_set_foreground) == EIO);
	assert(duplicate_calls == 1 && claim_calls == 0 && foreground_calls == 0);

	reset_attach_mocks();
	claim_status = -1;
	claim_errno = EBUSY;
	assert(pid1_console_attach(17, 42, 1, fake_duplicate,
				  fake_claim_terminal, fake_set_foreground) == EBUSY);
	assert(duplicate_calls == 3 && claim_calls == 1 && foreground_calls == 0);

	reset_attach_mocks();
	foreground_status = -1;
	foreground_errno = 0;
	assert(pid1_console_attach(17, 42, 1, fake_duplicate,
				  fake_claim_terminal, fake_set_foreground) == EIO);
	assert(duplicate_calls == 3 && claim_calls == 1 && foreground_calls == 1);

	reset_attach_mocks();
	assert(pid1_console_attach(0, 42, 0, fake_duplicate, NULL, NULL) == 0);
	assert(duplicate_calls == 3 && claim_calls == 0 && foreground_calls == 0);

	assert(pid1_console_attach(-1, 42, 0, fake_duplicate, NULL, NULL) == EINVAL);
	assert(pid1_console_attach(0, 42, 1, fake_duplicate, NULL, NULL) == EINVAL);
}

int
main(void)
{
	tcflag_t retained_flags;

	reset_mocks();
	assert(pid1_console_configure(4, NULL, fake_setattr) == EINVAL);
	assert(pid1_console_configure(4, fake_getattr, NULL) == EINVAL);
	assert(getter_calls == 0 && setter_calls == 0);

	reset_mocks();
	getter_status = -1;
	getter_errno = EACCES;
	assert(pid1_console_configure(7, fake_getattr, fake_setattr) == EACCES);
	assert(getter_calls == 1 && setter_calls == 0);
	getter_errno = 0;
	assert(pid1_console_configure(7, fake_getattr, fake_setattr) == EIO);
	assert(getter_calls == 2 && setter_calls == 0);

	reset_mocks();
	setter_status = -1;
	setter_errno = EBUSY;
	assert(pid1_console_configure(8, fake_getattr, fake_setattr) == EBUSY);
	assert(getter_calls == 1 && setter_calls == 1);
	setter_errno = 0;
	assert(pid1_console_configure(8, fake_getattr, fake_setattr) == EIO);
	assert(getter_calls == 2 && setter_calls == 2);

	reset_mocks();
	input_attributes.c_cflag = CBAUD | CSIZE | CSTOPB | PARENB | PARODD;
	retained_flags = input_attributes.c_cflag &
		(CBAUD | CBAUDEX | CSIZE | CSTOPB | PARENB | PARODD | CRTSCTS);
	input_attributes.c_iflag = BRKINT;
	input_attributes.c_oflag = OPOST | OCRNL;
	input_attributes.c_lflag = ICANON;
	input_attributes.c_cc[VINTR] = 99;
	assert(pid1_console_configure(9, fake_getattr, fake_setattr) == 0);
	assert(getter_calls == 1 && setter_calls == 1);
	assert(observed_fd == 9 && observed_action == TCSANOW);
	assert(output_attributes.c_cc[VINTR] == 3);
	assert(output_attributes.c_cc[VQUIT] == 28);
	assert(output_attributes.c_cc[VERASE] == 127);
	assert(output_attributes.c_cc[VKILL] == 21);
	assert(output_attributes.c_cc[VEOF] == 4);
	assert(output_attributes.c_cc[VSTART] == 17);
	assert(output_attributes.c_cc[VSTOP] == 19);
	assert(output_attributes.c_cc[VSUSP] == 26);
	assert(output_attributes.c_line == 0);
	assert(output_attributes.c_cflag == (retained_flags | CREAD | HUPCL | CLOCAL));
	assert(output_attributes.c_iflag == (ICRNL | IXON | IXOFF));
	assert(output_attributes.c_oflag == (OPOST | ONLCR));
	assert(output_attributes.c_lflag ==
		(ISIG | ICANON | ECHO | ECHOE | ECHOK | ECHOCTL | ECHOKE | IEXTEN));
	test_attach_requires_complete_standard_io_and_terminal_setup();
	return 0;
}
