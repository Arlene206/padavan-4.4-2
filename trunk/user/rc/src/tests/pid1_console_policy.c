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
	return 0;
}
