#define _POSIX_C_SOURCE 200809L

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static int
set_reuseaddr(int fd)
{
	int enabled = 1;

	return setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &enabled, sizeof(enabled));
}

int
main(void)
{
	int listener = -1, client = -1, accepted = -1, replacement = -1;
	struct sockaddr_in address;
	socklen_t address_length = sizeof(address);
	int status = 1;

	listener = socket(AF_INET, SOCK_STREAM, 0);
	if (listener < 0 || set_reuseaddr(listener) < 0) {
		perror("create reusable IAPP listener");
		goto out;
	}
	memset(&address, 0, sizeof(address));
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	address.sin_port = 0;
	if (bind(listener, (struct sockaddr *)&address, sizeof(address)) < 0 ||
	    getsockname(listener, (struct sockaddr *)&address, &address_length) < 0 ||
	    listen(listener, 1) < 0) {
		perror("bind/listen reusable IAPP listener");
		goto out;
	}

	client = socket(AF_INET, SOCK_STREAM, 0);
	if (client < 0 || connect(client, (struct sockaddr *)&address, sizeof(address)) < 0) {
		perror("connect local IAPP peer");
		goto out;
	}
	accepted = accept(listener, NULL, NULL);
	if (accepted < 0) {
		perror("accept local IAPP peer");
		goto out;
	}

	/* Make the server the active closer, leaving its endpoint in TIME_WAIT. */
	if (close(accepted) < 0) {
		perror("close accepted IAPP peer");
		accepted = -1;
		goto out;
	}
	accepted = -1;
	if (close(client) < 0) {
		perror("close IAPP client");
		client = -1;
		goto out;
	}
	client = -1;
	if (close(listener) < 0) {
		perror("close IAPP listener");
		listener = -1;
		goto out;
	}
	listener = -1;

	/* SO_REUSEADDR on both the old and replacement listeners permits restart. */
	replacement = socket(AF_INET, SOCK_STREAM, 0);
	if (replacement < 0 || set_reuseaddr(replacement) < 0 ||
	    bind(replacement, (struct sockaddr *)&address, sizeof(address)) < 0) {
		perror("rebind IAPP listener during TIME_WAIT");
		goto out;
	}
	status = 0;
	puts("IAPP TCP SO_REUSEADDR TIME_WAIT restart regression: PASS");

out:
	if (accepted >= 0)
		close(accepted);
	if (client >= 0)
		close(client);
	if (listener >= 0)
		close(listener);
	if (replacement >= 0)
		close(replacement);
	return status;
}
