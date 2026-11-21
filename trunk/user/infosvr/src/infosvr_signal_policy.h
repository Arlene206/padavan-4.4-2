#ifndef INFOSVR_SIGNAL_POLICY_H
#define INFOSVR_SIGNAL_POLICY_H

#include <signal.h>

/* Expand to a single sig_atomic_t store in the signal handler. */
#define INFOSVR_SIGNAL_REQUEST_STOP(flag) ((flag) = 1)
#define INFOSVR_SIGNAL_STOP_PENDING(flag) ((flag) != 0)

/* Bound the check-to-block race around select() and recvfrom(). */
#define INFOSVR_SELECT_TIMEOUT_SECONDS 1
#define INFOSVR_SOCKET_TIMEOUT_SECONDS 1

#endif
