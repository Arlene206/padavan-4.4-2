#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../trunk/user/p910nd/p910nd_sigpipe_policy.h"

int
main(void)
{
	int sockets[2];
	char received[2];
	const char payload[2] = {'o', 'k'};
	const char byte = 'x';
	struct sigaction action;

	assert(signal(SIGPIPE, SIG_DFL) != SIG_ERR);
	assert(socketpair(AF_UNIX, SOCK_STREAM, 0, sockets) == 0);
	assert(write(sockets[0], payload, sizeof(payload)) == (ssize_t)sizeof(payload));
	assert(read(sockets[1], received, sizeof(received)) == (ssize_t)sizeof(received));
	assert(memcmp(payload, received, sizeof(payload)) == 0);

	assert(p910nd_install_sigpipe_policy() == 0);
	assert(sigaction(SIGPIPE, NULL, &action) == 0);
	assert(action.sa_handler == SIG_IGN);

	assert(close(sockets[1]) == 0);
	errno = 0;
	assert(write(sockets[0], &byte, sizeof(byte)) == -1);
	assert(errno == EPIPE);
	assert(close(sockets[0]) == 0);
	return 0;
}
