#ifndef VPN_CLIENT_CONTROL_FIFO_H
#define VPN_CLIENT_CONTROL_FIFO_H
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stddef.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

static inline int
vpn_client_control_fifo_open_writer(const char *path)
{
	struct stat st;
	int fd, saved_errno;
	if (!path || !path[0]) { errno = EINVAL; return -1; }
	fd = open(path, O_WRONLY | O_NONBLOCK | O_NOFOLLOW, 0600);
	if (fd < 0) return -1;
	if (fstat(fd, &st) != 0) {
		saved_errno = errno ? errno : EIO;
		close(fd); errno = saved_errno; return -1;
	}
	if (!S_ISFIFO(st.st_mode)) {
		close(fd); errno = EINVAL; return -1;
	}
	return fd;
}

static inline int
vpn_client_control_fifo_open_retryable_error(int error_number)
{
	return error_number == ENOENT || error_number == ENXIO ||
		error_number == EINTR;
}

/* Keep xl2tpd startup synchronization, but bound stale-FIFO waits. */
static inline int
vpn_client_control_fifo_open_writer_wait(const char *path,
					 unsigned int attempts,
					 unsigned int retry_delay_us)
{
	struct timespec delay;
	unsigned int attempt;
	int fd, saved_errno;
	if (!path || !path[0] || attempts == 0U) { errno = EINVAL; return -1; }
	for (attempt = 0; attempt < attempts; attempt++) {
		fd = vpn_client_control_fifo_open_writer(path);
		if (fd >= 0) return fd;
		saved_errno = errno ? errno : EIO;
		if (!vpn_client_control_fifo_open_retryable_error(saved_errno) ||
		    attempt + 1U == attempts) {
			errno = saved_errno; return -1;
		}
		delay.tv_sec = (time_t)(retry_delay_us / 1000000U);
		delay.tv_nsec = (long)(retry_delay_us % 1000000U) * 1000L;
		while (nanosleep(&delay, &delay) != 0)
			if (errno != EINTR) return -1;
	}
	errno = EIO;
	return -1;
}

/* A disappearing FIFO reader must not terminate rc through SIGPIPE. */
static inline int
vpn_client_control_fifo_write_fd(int fd, const void *buffer, size_t length)
{
	sigset_t set, old_set, pending;
	struct timespec no_wait;
	ssize_t written;
	int saved_errno = 0, was_pending;
	if (fd < 0 || !buffer || length == 0U) { errno = EINVAL; return -1; }
	if (length > (size_t)PIPE_BUF) { errno = EMSGSIZE; return -1; }
	if (sigemptyset(&set) != 0 || sigaddset(&set, SIGPIPE) != 0)
		return -1;
	if (sigprocmask(SIG_BLOCK, &set, &old_set) != 0) return -1;
	if (sigpending(&pending) != 0) {
		saved_errno = errno ? errno : EIO;
		goto restore;
	}
	was_pending = sigismember(&pending, SIGPIPE);
	if (was_pending < 0) {
		saved_errno = errno ? errno : EIO;
		goto restore;
	}
	do { written = write(fd, buffer, length); }
	while (written < 0 && errno == EINTR);
	if (written < 0) saved_errno = errno ? errno : EIO;
	else if ((size_t)written != length) saved_errno = EIO;
	if (saved_errno == EPIPE && !was_pending) {
		no_wait.tv_sec = 0; no_wait.tv_nsec = 0;
		while (sigtimedwait(&set, NULL, &no_wait) < 0 && errno == EINTR)
			;
	}
restore:
	if (sigprocmask(SIG_SETMASK, &old_set, NULL) != 0 && saved_errno == 0)
		saved_errno = errno ? errno : EIO;
	if (saved_errno) { errno = saved_errno; return -1; }
	return 0;
}
#endif
