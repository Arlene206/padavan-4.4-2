#ifndef P910ND_SIGPIPE_POLICY_H
#define P910ND_SIGPIPE_POLICY_H

#include <signal.h>
#include <string.h>

/* Turn peer disconnects into EPIPE so the bidirectional stream can recover. */
static inline int
p910nd_install_sigpipe_policy(void)
{
	struct sigaction action;

	memset(&action, 0, sizeof(action));
	if (sigemptyset(&action.sa_mask) != 0)
		return -1;
	action.sa_handler = SIG_IGN;
	return sigaction(SIGPIPE, &action, NULL);
}

#endif
