#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>
#include "vpn_server_lease_policy.h"
#include "vpn_server_lease_lock.h"

static int
read_one(const unsigned char *bytes, size_t length, char *line, size_t line_size,
         size_t *line_length)
{
	FILE *fp = tmpfile();
	int result;
	assert(fp != NULL);
	assert(fwrite(bytes, 1, length, fp) == length);
	rewind(fp);
	result = vpn_server_lease_read_line(fp, line, line_size, line_length);
	fclose(fp);
	return result;
}

static FILE *
open_inventory(const char *contents)
{
	FILE *fp = tmpfile();
	assert(fp != NULL);
	if (contents && *contents)
		assert(fwrite(contents, 1, strlen(contents), fp) == strlen(contents));
	rewind(fp);
	return fp;
}

int
main(void)
{
	static const char valid[] = "ppp10 10.0.0.1 10.0.0.2 Alice Smith\n";
	static const char valid_two[] = "ppp10 10.0.0.1 10.0.0.2 Alice Smith\nppp11 10.0.0.1 10.0.0.3 Bob\n";
	static const char invalid_after_valid[] = "ppp10 10.0.0.1 10.0.0.2 Alice\nppp11 10.0.0.1\n";
	static const char empty_peer[] = "ppp10 10.0.0.1 10.0.0.2 \n";
	static const char no_newline[] = "ppp10 10.0.0.1 10.0.0.2 Alice";
	static const char too_few[] = "ppp10 10.0.0.1\n";
	static const char long_ifname[] = "ppp1234567890123 10.0.0.1 10.0.0.2 Alice\n";
	static const char long_peer[] = "ppp10 10.0.0.1 10.0.0.2 1234567890123456789012345678901234567890123456789012345678901234\n";
	static const char crlf[] = "ppp10 10.0.0.1 10.0.0.2 Alice\r\n";
	static const unsigned char embedded_nul[] = {
		'p','p','p','1','0',' ','1','0','.','0','.','0','.','1',' ',
		'1','0','.','0','.','0','.','2',' ','A','l','i','c','e',0,'\n'
	};
	char line[VPN_SERVER_LEASE_LINE_SIZE];
	char ifname[VPN_SERVER_LEASE_IFNAME_SIZE];
	char interfaces[2][VPN_SERVER_LEASE_IFNAME_SIZE];
	size_t line_length, interface_count;
	FILE *fp;

	assert(read_one((const unsigned char *)valid, sizeof(valid) - 1,
			line, sizeof(line), &line_length) == 1);
	assert(line_length == sizeof(valid) - 1);
	assert(vpn_server_lease_parse_line(line, line_length, ifname, sizeof(ifname)) == 0);
	assert(strcmp(ifname, "ppp10") == 0);
	assert(vpn_server_lease_should_keep(ifname, "ppp2"));
	assert(!vpn_server_lease_should_keep(ifname, "ppp10"));

	fp = open_inventory(valid_two);
	interface_count = 99;
	assert(vpn_server_lease_read_inventory(fp, interfaces, 2, &interface_count) == 0);
	assert(interface_count == 2);
	assert(strcmp(interfaces[0], "ppp10") == 0);
	assert(strcmp(interfaces[1], "ppp11") == 0);
	fclose(fp);

	/* A valid prefix followed by a malformed row must not publish a count. */
	fp = open_inventory(invalid_after_valid);
	interface_count = 99;
	assert(vpn_server_lease_read_inventory(fp, interfaces, 2, &interface_count) == -1);
	assert(interface_count == 99);
	fclose(fp);

	fp = open_inventory(valid_two);
	interface_count = 99;
	assert(vpn_server_lease_read_inventory(fp, interfaces, 1, &interface_count) == -1);
	assert(interface_count == 99);
	fclose(fp);

	fp = open_inventory("");
	interface_count = 99;
	assert(vpn_server_lease_read_inventory(fp, interfaces, 2, &interface_count) == 0);
	assert(interface_count == 0);
	fclose(fp);

	assert(read_one((const unsigned char *)no_newline, sizeof(no_newline) - 1,
			line, sizeof(line), &line_length) == -1);
	assert(read_one((const unsigned char *)too_few, sizeof(too_few) - 1,
			line, sizeof(line), &line_length) == 1);
	assert(vpn_server_lease_parse_line(line, line_length, ifname, sizeof(ifname)) == -1);
	assert(vpn_server_lease_parse_line(empty_peer, sizeof(empty_peer) - 1,
					  ifname, sizeof(ifname)) == -1);
	assert(vpn_server_lease_parse_line(long_ifname, sizeof(long_ifname) - 1,
					  ifname, sizeof(ifname)) == -1);
	assert(vpn_server_lease_parse_line(long_peer, sizeof(long_peer) - 1,
					  ifname, sizeof(ifname)) == -1);
	assert(vpn_server_lease_parse_line(crlf, sizeof(crlf) - 1,
					  ifname, sizeof(ifname)) == -1);
	assert(read_one(embedded_nul, sizeof(embedded_nul), line, sizeof(line),
			&line_length) == -1);

	{
		unsigned char oversized[VPN_SERVER_LEASE_LINE_SIZE + 20];
		memset(oversized, 'x', sizeof(oversized));
		oversized[sizeof(oversized) - 1] = '\n';
		assert(read_one(oversized, sizeof(oversized), line, sizeof(line),
				&line_length) == -1);
	}

	{
		char path[] = "/tmp/vpns-lock-test.XXXXXX";
		char hardlink[128], symlink_path[128];
		int fd, lock_fd;
		fd = mkstemp(path);
		assert(fd >= 0);
		assert(fchmod(fd, S_IRUSR | S_IWUSR) == 0);
		assert(close(fd) == 0);
		lock_fd = vpn_server_lease_lock_acquire(path);
		assert(lock_fd >= 0);
		assert(vpn_server_lease_lock_release(lock_fd) == 0);

		/* A second process must wait for the existing lease lock owner. */
		{
			int ready[2], flags, status;
			pid_t child;
			assert(pipe(ready) == 0);
			lock_fd = vpn_server_lease_lock_acquire(path);
			assert(lock_fd >= 0);
			child = fork();
			assert(child >= 0);
			if (child == 0) {
				int child_lock;
				close(ready[0]);
				(void)write(ready[1], "s", 1);
				child_lock = vpn_server_lease_lock_acquire(path);
				if (child_lock < 0)
					_exit(2);
				(void)write(ready[1], "l", 1);
				(void)vpn_server_lease_lock_release(child_lock);
				_exit(0);
			}
			close(ready[1]);
			assert(read(ready[0], line, 1) == 1 && line[0] == 's');
			usleep(100000);
			flags = fcntl(ready[0], F_GETFL, 0);
			assert(flags >= 0);
			assert(fcntl(ready[0], F_SETFL, flags | O_NONBLOCK) == 0);
			assert(read(ready[0], line, 1) == -1);
			assert(errno == EAGAIN || errno == EWOULDBLOCK);
			assert(vpn_server_lease_lock_release(lock_fd) == 0);
			assert(fcntl(ready[0], F_SETFL, flags) == 0);
			assert(read(ready[0], line, 1) == 1 && line[0] == 'l');
			assert(waitpid(child, &status, 0) == child);
			assert(WIFEXITED(status) && WEXITSTATUS(status) == 0);
			close(ready[0]);
		}

		snprintf(hardlink, sizeof(hardlink), "%s.hard", path);
		assert(link(path, hardlink) == 0);
		assert(vpn_server_lease_lock_acquire(path) == -1);
		unlink(hardlink);
		snprintf(symlink_path, sizeof(symlink_path), "%s.sym", path);
		assert(symlink(path, symlink_path) == 0);
		assert(vpn_server_lease_lock_acquire(symlink_path) == -1);
		unlink(symlink_path);
		unlink(path);
	}

	{
		char path[] = "/tmp/vpns-append-test.XXXXXX";
		int fd = mkstemp(path);
		assert(fd >= 0);
		assert(vpn_server_lease_append_record(fd, valid, sizeof(valid) - 1) == 0);
		assert(lseek(fd, 0, SEEK_SET) == 0);
		assert(read(fd, line, sizeof(line)) == (ssize_t)(sizeof(valid) - 1));
		assert(memcmp(line, valid, sizeof(valid) - 1) == 0);
		assert(close(fd) == 0);
		unlink(path);
	}
	{
		int fd = open("/dev/full", O_WRONLY | O_APPEND);
		if (fd >= 0) {
			assert(vpn_server_lease_append_record(fd, valid, sizeof(valid) - 1) == -1);
			close(fd);
		}
	}

	puts("VPN server lease inventory policy: PASS");
	return 0;
}
