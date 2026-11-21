#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

int
main(void)
{
	char path[] = "/tmp/firewall-snapshot-fd.XXXXXX";
	char command[64], buffer[32];
	struct stat snapshot_stat;
	int fd, fd_flags, status, length;
	ssize_t amount;

	fd = mkstemp(path);
	assert(fd >= 0);
	assert(fchmod(fd, S_IRUSR | S_IWUSR) == 0);
	assert(fstat(fd, &snapshot_stat) == 0);
	assert((snapshot_stat.st_mode & 0777) == 0600);

	/* The writer must retain this inode even if its pathname disappears. */
	assert(unlink(path) == 0);
	assert(access(path, F_OK) != 0 && errno == ENOENT);

	/* Model a close-on-exec descriptor and require an explicit inheritance gate. */
	fd_flags = fcntl(fd, F_GETFD, 0);
	assert(fd_flags >= 0);
	assert(fcntl(fd, F_SETFD, fd_flags | FD_CLOEXEC) == 0);
	fd_flags = fcntl(fd, F_GETFD, 0);
	assert(fd_flags >= 0 && (fd_flags & FD_CLOEXEC));
	assert(fcntl(fd, F_SETFD, fd_flags & ~FD_CLOEXEC) == 0);

	length = snprintf(command, sizeof(command), "printf snapshot-data >&%d", fd);
	assert(length > 0 && (size_t)length < sizeof(command));
	status = system(command);
	assert(status != -1 && WIFEXITED(status) && WEXITSTATUS(status) == 0);
	assert(fstat(fd, &snapshot_stat) == 0);
	assert(snapshot_stat.st_size == 13);
	assert(lseek(fd, 0, SEEK_SET) == 0);
	amount = read(fd, buffer, sizeof(buffer) - 1);
	assert(amount == 13);
	buffer[amount] = '\0';
	assert(strcmp(buffer, "snapshot-data") == 0);

	assert(close(fd) == 0);
	puts("firewall snapshot descriptor writer inheritance: PASS");
	return 0;
}
