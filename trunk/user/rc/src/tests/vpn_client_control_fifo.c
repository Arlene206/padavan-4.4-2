#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include "vpn_client_control_fifo.h"
#define CHECK(x) do { if (!(x)) { fprintf(stderr,"FAIL %s:%d: %s errno=%d\n",__FILE__,__LINE__,#x,errno); return -1; } } while (0)

static int
path_join(char *out, size_t size, const char *dir, const char *name)
{
	int n = snprintf(out, size, "%s/%s", dir, name);
	return n >= 0 && (size_t)n < size ? 0 : -1;
}

static int
test_orphaned_fifo_open_is_nonblocking(const char *fifo)
{
	pid_t child;
	int status;
	child = fork();
	CHECK(child >= 0);
	if (child == 0) {
		int fd;
		alarm(2);
		fd = vpn_client_control_fifo_open_writer(fifo);
		if (fd >= 0) { close(fd); _exit(1); }
		_exit(errno == ENXIO ? 0 : 2);
	}
	CHECK(waitpid(child, &status, 0) == child);
	CHECK(WIFEXITED(status) && WEXITSTATUS(status) == 0);
	CHECK(vpn_client_control_fifo_open_writer_wait(fifo, 3U, 1000U) < 0);
	CHECK(errno == ENXIO);
	return 0;
}

static int
test_delayed_reader_startup(const char *fifo)
{
	pid_t child;
	int fd, status;
	char buf[16];
	ssize_t n;
	child = fork();
	CHECK(child >= 0);
	if (child == 0) {
		int rd;
		usleep(50000);
		rd = open(fifo, O_RDONLY);
		if (rd < 0) _exit(2);
		n = read(rd, buf, sizeof(buf));
		close(rd);
		_exit(n == 6 && memcmp(buf, "c VPNC", 6) == 0 ? 0 : 3);
	}
	fd = vpn_client_control_fifo_open_writer_wait(fifo, 30U, 10000U);
	CHECK(fd >= 0);
	CHECK(vpn_client_control_fifo_write_fd(fd, "c VPNC", 6U) == 0);
	CHECK(close(fd) == 0);
	CHECK(waitpid(child, &status, 0) == child);
	CHECK(WIFEXITED(status) && WEXITSTATUS(status) == 0);
	return 0;
}

static int
test_fifo_type_and_symlink(const char *fifo, const char *link,
			   const char *regular)
{
	int fd;
	CHECK(symlink(fifo, link) == 0);
	fd = vpn_client_control_fifo_open_writer(link);
	CHECK(fd < 0 && errno == ELOOP);
	fd = open(regular, O_WRONLY | O_CREAT | O_TRUNC, 0600);
	CHECK(fd >= 0 && close(fd) == 0);
	fd = vpn_client_control_fifo_open_writer(regular);
	CHECK(fd < 0 && errno == EINVAL);
	CHECK(vpn_client_control_fifo_open_writer(NULL) < 0 && errno == EINVAL);
	return 0;
}

static int
test_retryable_errors(void)
{
	CHECK(vpn_client_control_fifo_open_retryable_error(ENOENT));
	CHECK(vpn_client_control_fifo_open_retryable_error(ENXIO));
	CHECK(vpn_client_control_fifo_open_retryable_error(EINTR));
	CHECK(!vpn_client_control_fifo_open_retryable_error(EACCES));
	CHECK(!vpn_client_control_fifo_open_retryable_error(ELOOP));
	CHECK(!vpn_client_control_fifo_open_retryable_error(EPIPE));
	CHECK(vpn_client_control_fifo_open_writer_wait("/missing/control/fifo",
						       2U, 1000U) < 0);
	CHECK(errno == ENOENT);
	CHECK(vpn_client_control_fifo_open_writer_wait("/tmp", 0U, 0U) < 0);
	CHECK(errno == EINVAL);
	return 0;
}

static int
test_epipe_does_not_kill_process(void)
{
	struct sigaction old_action, action;
	sigset_t pending;
	int p[2], result;
	memset(&action, 0, sizeof(action));
	action.sa_handler = SIG_DFL;
	CHECK(sigemptyset(&action.sa_mask) == 0);
	CHECK(sigaction(SIGPIPE, &action, &old_action) == 0);
	CHECK(pipe(p) == 0 && close(p[0]) == 0);
	result = vpn_client_control_fifo_write_fd(p[1], "d VPNC", 6U);
	CHECK(result < 0 && errno == EPIPE);
	CHECK(sigpending(&pending) == 0 && sigismember(&pending, SIGPIPE) == 0);
	CHECK(close(p[1]) == 0);
	CHECK(sigaction(SIGPIPE, &old_action, NULL) == 0);
	return 0;
}

static int
test_preexisting_sigpipe_is_preserved(void)
{
	sigset_t set, old_set, pending;
	struct timespec zero;
	int p[2], result, saved_errno;
	CHECK(sigemptyset(&set) == 0 && sigaddset(&set, SIGPIPE) == 0);
	CHECK(sigprocmask(SIG_BLOCK, &set, &old_set) == 0);
	CHECK(raise(SIGPIPE) == 0);
	CHECK(sigpending(&pending) == 0 && sigismember(&pending, SIGPIPE) == 1);
	CHECK(pipe(p) == 0 && close(p[0]) == 0);
	result = vpn_client_control_fifo_write_fd(p[1], "d VPNC", 6U);
	saved_errno = errno;
	CHECK(result < 0 && saved_errno == EPIPE);
	CHECK(sigpending(&pending) == 0 && sigismember(&pending, SIGPIPE) == 1);
	zero.tv_sec = 0; zero.tv_nsec = 0;
	CHECK(sigtimedwait(&set, NULL, &zero) == SIGPIPE);
	CHECK(close(p[1]) == 0);
	CHECK(sigprocmask(SIG_SETMASK, &old_set, NULL) == 0);
	return 0;
}

static int
test_atomic_fifo_write_and_bounds(const char *fifo)
{
	int rd, wr;
	char buf[16], large[PIPE_BUF + 1];
	ssize_t n;
	rd = open(fifo, O_RDONLY | O_NONBLOCK);
	CHECK(rd >= 0);
	wr = vpn_client_control_fifo_open_writer_wait(fifo, 2U, 1000U);
	CHECK(wr >= 0);
	CHECK(vpn_client_control_fifo_write_fd(wr, "d VPNC", 6U) == 0);
	n = read(rd, buf, sizeof(buf));
	CHECK(n == 6 && memcmp(buf, "d VPNC", 6) == 0);
	CHECK(close(wr) == 0 && close(rd) == 0);
	CHECK(vpn_client_control_fifo_write_fd(-1, "x", 1U) < 0 && errno == EINVAL);
	CHECK(vpn_client_control_fifo_write_fd(1, NULL, 1U) < 0 && errno == EINVAL);
	CHECK(vpn_client_control_fifo_write_fd(1, "x", 0U) < 0 && errno == EINVAL);
	CHECK(vpn_client_control_fifo_write_fd(1, large, sizeof(large)) < 0 &&
	      errno == EMSGSIZE);
	return 0;
}

int
main(void)
{
	char dir[] = "/tmp/vpn-control.XXXXXX";
	char fifo[256], link[256], regular[256];
	CHECK(mkdtemp(dir) != NULL);
	CHECK(path_join(fifo,sizeof(fifo),dir,"control") == 0);
	CHECK(path_join(link,sizeof(link),dir,"link") == 0);
	CHECK(path_join(regular,sizeof(regular),dir,"regular") == 0);
	CHECK(mkfifo(fifo,0600) == 0);
	CHECK(test_orphaned_fifo_open_is_nonblocking(fifo) == 0);
	CHECK(test_delayed_reader_startup(fifo) == 0);
	CHECK(test_atomic_fifo_write_and_bounds(fifo) == 0);
	CHECK(test_fifo_type_and_symlink(fifo,link,regular) == 0);
	CHECK(test_retryable_errors() == 0);
	CHECK(test_epipe_does_not_kill_process() == 0);
	CHECK(test_preexisting_sigpipe_is_preserved() == 0);
	CHECK(unlink(link) == 0 && unlink(regular) == 0 && unlink(fifo) == 0);
	CHECK(rmdir(dir) == 0);
	puts("VPN client control FIFO bounded-open and SIGPIPE regression: PASS");
	return 0;
}
