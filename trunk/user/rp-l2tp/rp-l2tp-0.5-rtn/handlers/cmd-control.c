/***********************************************************************
*
* cmd-control.c
*
* Simple command-line program for sending commands to L2TP daemon
*
* Copyright (C) 2002 by Roaring Penguin Software Inc.
*
* This program is distributed under the terms of the GNU General Public
* License, Version 2, or (at your option) any later version.
*
* LIC: GPL
*
***********************************************************************/

static char const RCSID[] =
"$Id: cmd-control.c 3323 2011-09-21 18:45:48Z lly.dev $";

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/un.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>

#include "cmd-control-status.h"

#define CONTROL_IO_TIMEOUT_SECONDS 5
#define CONTROL_RESPONSE_PREFIX_SIZE 4

static int
write_all(int fd, const void *buffer, size_t length)
{
	const char *cursor = (const char *) buffer;

	while (length > 0) {
		ssize_t written = write(fd, cursor, length);

		if (written < 0) {
			if (errno == EINTR)
				continue;
			return -1;
		}
		if (written == 0) {
			errno = EIO;
			return -1;
		}
		cursor += written;
		length -= (size_t) written;
	}

	return 0;
}

/**********************************************************************
* %FUNCTION: send_cmd
* %ARGUMENTS:
*  cmd -- command to send to server
* %RETURNS:
*  file descriptor for channel to server
* %DESCRIPTION:
*  Sends a command and applies bounded socket I/O timeouts.
***********************************************************************/
static int
send_cmd(const char *cmd)
{
	struct sockaddr_un addr;
	struct timeval timeout;
	int fd;

	if (!cmd || !*cmd) {
		errno = EINVAL;
		return -1;
	}

	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_LOCAL;
	strncpy(addr.sun_path, "/var/run/l2tpctrl", sizeof(addr.sun_path) - 1);

	fd = socket(AF_LOCAL, SOCK_STREAM, 0);
	if (fd < 0) {
		perror("socket");
		return -1;
	}

	timeout.tv_sec = CONTROL_IO_TIMEOUT_SECONDS;
	timeout.tv_usec = 0;
	if (setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO,
		       &timeout, sizeof(timeout)) != 0 ||
	    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO,
		       &timeout, sizeof(timeout)) != 0) {
		perror("setsockopt");
		close(fd);
		return -1;
	}

	if (connect(fd, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("connect");
		close(fd);
		return -1;
	}

	if (write_all(fd, cmd, strlen(cmd)) != 0 ||
	    write_all(fd, "\n", 1) != 0) {
		perror("write");
		close(fd);
		return -1;
	}

	return fd;
}

int
main(int argc, char *argv[])
{
	int fd;
	int n;
	int response_status;
	size_t response_prefix_length = 0;
	char response_prefix[CONTROL_RESPONSE_PREFIX_SIZE];
	char buf[4096];

	if (argc != 2) {
		fprintf(stderr, "Usage: %s command\n", argv[0]);
		return 1;
	}

	fd = send_cmd(argv[1]);
	if (fd < 0)
		return 1;

	for (;;) {
		n = read(fd, buf, sizeof(buf));
		if (n < 0) {
			if (errno == EINTR)
				continue;
			perror("read");
			close(fd);
			return 1;
		}
		if (n == 0)
			break;

		if (response_prefix_length < sizeof(response_prefix)) {
			size_t remaining = sizeof(response_prefix) - response_prefix_length;
			size_t received = (size_t) n < remaining ? (size_t) n : remaining;

			memcpy(response_prefix + response_prefix_length, buf, received);
			response_prefix_length += received;
		}

		if (write_all(STDOUT_FILENO, buf, (size_t) n) != 0) {
			perror("stdout");
			close(fd);
			return 1;
		}
	}

	if (write_all(STDOUT_FILENO, "\n", 1) != 0) {
		perror("stdout");
		close(fd);
		return 1;
	}

	response_status = cmd_control_response_status(
		response_prefix, response_prefix_length);
	if (response_status != 0) {
		fprintf(stderr, "l2tp-control: daemon rejected or malformed the response: %s\n",
			strerror(response_status));
		close(fd);
		return 1;
	}

	if (close(fd) != 0) {
		perror("close");
		return 1;
	}

	return 0;
}
