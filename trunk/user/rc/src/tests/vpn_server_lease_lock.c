#define _GNU_SOURCE
#define APP_OPENVPN 1
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#ifndef SYS_GROUP_NOGROUP
#define SYS_GROUP_NOGROUP "nogroup"
#endif
#include "vpn_server_lease_lock.h"

static void
write_byte(int fd, char byte)
{
	ssize_t written;

	do {
		written = write(fd, &byte, 1);
	} while (written < 0 && errno == EINTR);
	assert(written == 1);
}

static char
read_byte(int fd)
{
	char byte;
	ssize_t received;

	do {
		received = read(fd, &byte, 1);
	} while (received < 0 && errno == EINTR);
	assert(received == 1);
	return byte;
}

int
main(void)
{
	char directory[] = "/tmp/vpns-shared-lease-lock.XXXXXX";
	char path[256], hardlink[256], symlink_path[256], target[256];
	struct stat st;
	struct timeval timeout;
	fd_set readfds;
	uid_t owner = geteuid();
	gid_t group = getegid();
	int lock_fd, pipefd[2], child_fd, ready, status;
	pid_t child;

	assert(vpn_server_lease_lock_acquire_path(NULL, owner, group, 1) == -1 &&
	       errno == EINVAL);
	assert(vpn_server_lease_lock_release(-1) == -1 && errno == EBADF);
	assert(mkdtemp(directory) != NULL);
	assert(snprintf(path, sizeof(path), "%s/lock", directory) > 0);
	assert(snprintf(hardlink, sizeof(hardlink), "%s/hard", directory) > 0);
	assert(snprintf(symlink_path, sizeof(symlink_path), "%s/sym", directory) > 0);
	assert(snprintf(target, sizeof(target), "%s/target", directory) > 0);

	lock_fd = vpn_server_lease_lock_acquire_path(path, owner, group, 1);
	assert(lock_fd >= 0 && fstat(lock_fd, &st) == 0);
	assert(S_ISREG(st.st_mode) && st.st_uid == owner && st.st_gid == group);
	assert(st.st_nlink == 1 && (st.st_mode & 07777) == 0620);

	/* A different process must block on the same stable inode until release. */
	assert(pipe(pipefd) == 0);
	child = fork();
	assert(child >= 0);
	if (child == 0) {
		close(pipefd[0]);
		close(lock_fd);
		write_byte(pipefd[1], 'W');
		child_fd = vpn_server_lease_lock_acquire_path(path, owner, group, 0);
		if (child_fd < 0)
			_exit(10);
		write_byte(pipefd[1], 'A');
		if (vpn_server_lease_lock_release(child_fd) != 0)
			_exit(11);
		_exit(0);
	}
	close(pipefd[1]);
	assert(read_byte(pipefd[0]) == 'W');
	FD_ZERO(&readfds);
	FD_SET(pipefd[0], &readfds);
	timeout.tv_sec = 0;
	timeout.tv_usec = 100000;
	ready = select(pipefd[0] + 1, &readfds, NULL, NULL, &timeout);
	assert(ready == 0);
	assert(vpn_server_lease_lock_release(lock_fd) == 0);
	assert(read_byte(pipefd[0]) == 'A');
	assert(waitpid(child, &status, 0) == child);
	assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);
	close(pipefd[0]);

	/* Reject exposed permissions, hard links and symlink substitution. */
	assert(chmod(path, 0644) == 0);
	assert(vpn_server_lease_lock_acquire_path(path, owner, group, 0) == -1 &&
	       errno == EPERM);
	assert(chmod(path, 0620) == 0);
	assert(link(path, hardlink) == 0);
	assert(vpn_server_lease_lock_acquire_path(path, owner, group, 0) == -1 &&
	       errno == EPERM);
	assert(unlink(hardlink) == 0);
	child_fd = open(target, O_WRONLY | O_CREAT | O_EXCL, 0600);
	assert(child_fd >= 0 && close(child_fd) == 0);
	assert(unlink(path) == 0 && symlink(target, symlink_path) == 0);
	assert(vpn_server_lease_lock_acquire_path(symlink_path, owner, group, 0) == -1 &&
	       errno == ELOOP);
	assert(unlink(symlink_path) == 0 && unlink(target) == 0);
	/* On root-capable hosts, exercise the real root:group permission
	 * boundary used by the OpenVPN nobody process. */
	if (geteuid() == (uid_t)0) {
		struct group *vpn_group = getgrnam(SYS_GROUP_NOGROUP);
		struct passwd *vpn_user = getpwnam("nobody");

		if (vpn_group && vpn_user) {
			char root_lock_path[256];
			int root_lock_fd, root_pipe[2], root_child_fd;
			int root_ready, root_status;
			pid_t root_child;

			assert(snprintf(root_lock_path, sizeof(root_lock_path),
					"%s/root-lock", directory) > 0);
			root_lock_fd = vpn_server_lease_lock_acquire_path(
				root_lock_path, (uid_t)0, vpn_group->gr_gid, 1);
			assert(root_lock_fd >= 0 && fstat(root_lock_fd, &st) == 0);
			assert(st.st_uid == (uid_t)0 && st.st_gid == vpn_group->gr_gid);
			assert((st.st_mode & 07777) == 0620);
			assert(chmod(directory, 0755) == 0);
			assert(pipe(root_pipe) == 0);
			root_child = fork();
			assert(root_child >= 0);
			if (root_child == 0) {
				close(root_pipe[0]);
				close(root_lock_fd);
				if (setgid(vpn_group->gr_gid) != 0 ||
				    setuid(vpn_user->pw_uid) != 0)
					_exit(12);
				write_byte(root_pipe[1], 'W');
				root_child_fd = vpn_server_lease_lock_acquire_path(
					root_lock_path, (uid_t)0, vpn_group->gr_gid, 0);
				if (root_child_fd < 0)
					_exit(13);
				write_byte(root_pipe[1], 'A');
				if (vpn_server_lease_lock_release(root_child_fd) != 0)
					_exit(14);
				_exit(0);
			}
			close(root_pipe[1]);
			assert(read_byte(root_pipe[0]) == 'W');
			FD_ZERO(&readfds);
			FD_SET(root_pipe[0], &readfds);
			timeout.tv_sec = 0;
			timeout.tv_usec = 100000;
			root_ready = select(root_pipe[0] + 1, &readfds, NULL, NULL,
					    &timeout);
			assert(root_ready == 0);
			assert(vpn_server_lease_lock_release(root_lock_fd) == 0);
			assert(read_byte(root_pipe[0]) == 'A');
			assert(waitpid(root_child, &root_status, 0) == root_child);
			assert(WIFEXITED(root_status) && WEXITSTATUS(root_status) == 0);
			close(root_pipe[0]);
			assert(chmod(directory, 0700) == 0);
			assert(unlink(root_lock_path) == 0);
		}
	}

	/* An attacker-created FIFO must fail immediately, never block startup. */
	assert(mkfifo(path, 0600) == 0);
	errno = 0;
	assert(vpn_server_lease_lock_acquire_path(path, owner, group, 0) == -1);
	assert(errno == ENXIO);
	assert(unlink(path) == 0);
	assert(rmdir(directory) == 0);

	puts("VPN server shared lease lock policy: PASS");
	return 0;
}
