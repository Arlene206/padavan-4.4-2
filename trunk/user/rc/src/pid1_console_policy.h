#ifndef PID1_CONSOLE_POLICY_H
#define PID1_CONSOLE_POLICY_H

#include <errno.h>
#include <termios.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <unistd.h>

typedef int (*pid1_console_getattr_fn)(int, struct termios *);
typedef int (*pid1_console_setattr_fn)(int, int, const struct termios *);

/* A failed read must never feed uninitialized termios state to tcsetattr. */
static inline int
pid1_console_configure(int fd, pid1_console_getattr_fn get_attributes,
		       pid1_console_setattr_fn set_attributes)
{
	struct termios tty;

	if (!get_attributes || !set_attributes)
		return EINVAL;

	memset(&tty, 0, sizeof(tty));
	errno = 0;
	if (get_attributes(fd, &tty) < 0)
		return errno > 0 ? errno : EIO;

	/* set control chars */
	tty.c_cc[VINTR]  = 3;	/* C-c */
	tty.c_cc[VQUIT]  = 28;	/* C-\\ */
	tty.c_cc[VERASE] = 127;	/* C-? */
	tty.c_cc[VKILL]  = 21;	/* C-u */
	tty.c_cc[VEOF]   = 4;	/* C-d */
	tty.c_cc[VSTART] = 17;	/* C-q */
	tty.c_cc[VSTOP]  = 19;	/* C-s */
	tty.c_cc[VSUSP]  = 26;	/* C-z */

	/* use line discipline 0 */
	tty.c_line = 0;

	/* added CRTSCTS to fix Debian bug 528560 */
	tty.c_cflag &= CBAUD | CBAUDEX | CSIZE | CSTOPB | PARENB | PARODD | CRTSCTS;
	tty.c_cflag |= CREAD | HUPCL | CLOCAL;

	/* input modes */
	tty.c_iflag = ICRNL | IXON | IXOFF;

	/* output modes */
	tty.c_oflag = OPOST | ONLCR;

	/* local modes */
	tty.c_lflag = ISIG | ICANON | ECHO | ECHOE | ECHOK | ECHOCTL | ECHOKE | IEXTEN;

	errno = 0;
	if (set_attributes(fd, TCSANOW, &tty) < 0)
		return errno > 0 ? errno : EIO;
	return 0;
}


typedef int (*pid1_console_dup_fn)(int, int);
typedef int (*pid1_console_tty_ioctl_fn)(int, unsigned long, int);
typedef int (*pid1_console_foreground_fn)(int, pid_t);

/* Build a usable stdio/controlling-terminal context or fail before shell exec. */
static inline int
pid1_console_attach(int fd, pid_t foreground_group, int terminal_available,
		    pid1_console_dup_fn duplicate_fd,
		    pid1_console_tty_ioctl_fn claim_terminal,
		    pid1_console_foreground_fn set_foreground)
{
	int descriptors[] = { STDIN_FILENO, STDOUT_FILENO, STDERR_FILENO };
	int i;

	if (fd < 0 || !duplicate_fd ||
	    (terminal_available && (!claim_terminal || !set_foreground)))
		return EINVAL;

	for (i = 0; i < (int)(sizeof(descriptors) / sizeof(descriptors[0])); i++) {
		errno = 0;
		if (duplicate_fd(fd, descriptors[i]) != descriptors[i])
			return errno > 0 ? errno : EIO;
	}

	/* A /dev/null fallback supplies stdio only, never a controlling tty. */
	if (!terminal_available)
		return 0;

	errno = 0;
	if (claim_terminal(STDIN_FILENO, TIOCSCTTY, 1) < 0)
		return errno > 0 ? errno : EIO;

	errno = 0;
	if (set_foreground(STDIN_FILENO, foreground_group) < 0)
		return errno > 0 ? errno : EIO;

	return 0;
}

#define PID1_CONSOLE_RETRY_FAST_EXIT_SECONDS 2L
#define PID1_CONSOLE_RETRY_DELAY_SECONDS 1U

/* Avoid a tight fork/exec loop when the recovery shell cannot stay alive. */
static inline unsigned int
pid1_console_retry_delay_seconds(long runtime_seconds)
{
	return runtime_seconds < PID1_CONSOLE_RETRY_FAST_EXIT_SECONDS ?
		PID1_CONSOLE_RETRY_DELAY_SECONDS : 0U;
}

#endif /* PID1_CONSOLE_POLICY_H */
